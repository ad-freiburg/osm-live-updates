// Copyright 2024, University of Freiburg
// Authors: Nicolas von Trott <nicolasvontrott@gmail.com>.

// This file is part of osm-live-updates.
//
// osm-live-updates is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// osm-live-updates is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with osm-live-updates.  If not, see <https://www.gnu.org/licenses/>.

#include "osm/OsmChangeHandler.h"

#include <string>
#include <iostream>
#include <fstream>
#include <set>

#include "osmium/visitor.hpp"
#include "osmium/builder/osm_object_builder.hpp"
#include "osmium/io/pbf_output.hpp"
#include "osmium/io/writer.hpp"
#include "osmium/osm/entity_bits.hpp"
#include "osmium/osm/object_comparisons.hpp"
#include "osm2rdf/util/Time.h"
#include "osm2rdf/util/ProgressBar.h"

#include "config/Constants.h"
#include "osm/NodeHandler.h"
#include "osm/Osm2ttl.h"
#include "osm/OsmDataFetcherSparql.h"
#include "sparql/QueryWriter.h"
#include "util/XmlHelper.h"
#include "util/TtlHelper.h"
#include "util/BatchHelper.h"

namespace cnst = olu::config::constants;

// _________________________________________________________________________________________________
olu::osm::OsmChangeHandler::OsmChangeHandler(const config::Config &config, OsmDataFetcher &odf,
                                             StatisticsHandler &stats) :
    _config(config),
    _sparql(config),
    _queryWriter(config),
    _odf(&odf),
    _stats(&stats),
    _nodeHandler(config, odf, stats),
    _wayHandler(config, odf, stats),
    _relationHandler(config, odf, stats),
    _referencesHandler(_config, odf, _nodeHandler, _wayHandler, _relationHandler) {
    createTmpFiles();
}

// _________________________________________________________________________________________________
void olu::osm::OsmChangeHandler::run() {
    _stats->startTimeProcessingChangeFiles();
    osmium::io::Reader nodeReader{cnst::PATH_TO_CHANGE_FILE,
        osmium::osm_entity_bits::node,
        osmium::io::read_meta::no};
    // Loop over the osm objects in the change file one time and store each objects id in the
    // corresponding set
    // (_createdNodes/Ways/Relations, _modifiedNodes/Ways/Relations,
    // _deletedNodes/Ways/Relations).
    osmium::apply(nodeReader, _nodeHandler);
    // Check for modified nodes if the location has changed.
    // If so, the node is added to the _modifiedNodesWithChangedLocation set, otherwise to the
    // _modifiedNodes set
    _stats->startTimeCheckingNodeLocations();
    _nodeHandler.checkNodesForLocationChange();
    _stats->endTimeCheckingNodeLocations();
    nodeReader.close();

    osmium::io::Reader wayReader{ cnst::PATH_TO_CHANGE_FILE,
        osmium::osm_entity_bits::way,
        osmium::io::read_meta::no};
    osmium::apply(wayReader, _wayHandler);
    // Check for modified ways if the members have changed.
    // If so, the way is added to the _modifiedWaysWithChangedMembers set, otherwise to the
    // _modifiedWays set
    _stats->startTimeCheckingWayMembers();
    _wayHandler.checkWaysForMemberChange(_nodeHandler.getModifiedNodesWithChangedLocation());
    _stats->endTimeCheckingWayMembers();
    wayReader.close();

    osmium::io::Reader relationReader{ cnst::PATH_TO_CHANGE_FILE,
        osmium::osm_entity_bits::relation,
        osmium::io::read_meta::no};
    osmium::apply(relationReader, _relationHandler);
    // Check for modified relations if the members have changed.
    // If so, the relation is added to the _modifiedRelationsWithChangedMembers set, otherwise
    // to the _modifiedRelations set
    _stats->startTimeCheckingRelationMembers();
    _relationHandler.checkRelationsForMemberChange(
        _nodeHandler.getModifiedNodesWithChangedLocation(),
        _wayHandler.getModifiedWaysWithChangedMembers());
    _stats->endTimeCheckingRelationMembers();
    relationReader.close();

    if (_nodeHandler.empty() && _wayHandler.empty() && _relationHandler.empty()) {
        throw OsmChangeHandlerException("Change file is empty.");
    }

    _stats->endTimeProcessingChangeFiles();
    _stats->printCurrentStep("Fetch IDs of objects that need to be updated...");
    // Fetch the ids of all ways and relations that need to be updated, meaning they reference an
    // OSM object that changed their geometry because of elements in the change file.
    _stats->startTimeFetchingObjectsToUpdateGeo();
    getIdsOfWaysToUpdateGeo();
    getIdsOfRelationsToUpdateGeo();
    _stats->endTimeFetchingObjectsToUpdateGeo();

    _stats->printCurrentStep("Fetch references...");
    // Loop over the ways and relations a second time to store the ids of the referenced
    // elements.
    // We will need to retrieve them later from the endpoint (if they are not already
    // in the change file) for osm2rdf to calculate the geometries.
    _stats->startTimeFetchingReferences();
    osmium::io::Reader referencesReader{ cnst::PATH_TO_CHANGE_FILE,
        osmium::osm_entity_bits::way | osmium::osm_entity_bits::relation,
        osmium::io::read_meta::no};
    osmium::apply(referencesReader, _referencesHandler);
    referencesReader.close();

    // Fetch the ids of all nodes and ways that are referenced by relations which are not in the
    // change file.
    std::set relationIds(_referencesHandler.getReferencedRelations());
    relationIds.insert(_relationsToUpdateGeometry.begin(),
                        _relationsToUpdateGeometry.end());
    _referencesHandler.getReferencesForRelations(relationIds);

    // Fetch the ids of all nodes that are referenced by ways which are not in the change file
    std::set wayIds(_referencesHandler.getReferencedWays());
    wayIds.insert(_waysToUpdateGeometry.begin(), _waysToUpdateGeometry.end());
    _referencesHandler.getReferencesForWays(wayIds);
    _stats->endTimeFetchingReferences();

    // Create the dummy objects for the nodes, ways and relations that are referenced by
    // elements in the change file,
    // as well as the ways and relations for which the geometry needs to be updated.
    _stats->startTimeCreatingDummys();
    createDummyElements();
    _stats->endTimeCreatingDummys();

    try {
        _stats->startTimeOsm2RdfConversion();
        Osm2ttl(_config).convert();
        _stats->endTimeOsm2RdfConversion();
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        throw OsmChangeHandlerException("Exception while trying to convert osm element to"
                                        " ttl");
    }

    // Delete and insert elements from database
    _stats->startTimeDeletingTriples();
    deleteTriplesFromDatabase();
    _stats->endTimeDeletingTriples();

    _stats->startTimeInsertingTriples();
    insertTriplesToDatabase();
    _stats->endTimeInsertingTriples();
}

// _________________________________________________________________________________________________
void olu::osm::OsmChangeHandler::createTmpFiles() {
    initTmpFile(cnst::PATH_TO_NODE_FILE);
    initTmpFile(cnst::PATH_TO_WAY_FILE);
    initTmpFile(cnst::PATH_TO_RELATION_FILE);
}

// _________________________________________________________________________________________________
void olu::osm::OsmChangeHandler::initTmpFile(const std::string& filepath) {
    std::ofstream file(filepath, std::ios::trunc);
    file << "<osm version=\"0.6\">" << std::endl;
    file.close();
}

// _________________________________________________________________________________________________
void olu::osm::OsmChangeHandler::finalizeTmpFile(const std::string& filepath) {
    std::ofstream file(filepath, std::ios::app);
    file << "</osm>" << std::endl;
    file.close();
}

// _________________________________________________________________________________________________
void olu::osm::OsmChangeHandler::addToTmpFile(const std::string& element,
                                              const std::string& elementTag) {
    std::ofstream outputFile;
    if (elementTag == cnst::XML_TAG_NODE) {
        outputFile.open (cnst::PATH_TO_NODE_FILE, std::ios::app);
    } else if (elementTag == cnst::XML_TAG_WAY) {
        outputFile.open (cnst::PATH_TO_WAY_FILE, std::ios::app);
    } else if (elementTag == cnst::XML_TAG_REL) {
        outputFile.open (cnst::PATH_TO_RELATION_FILE, std::ios::app);
    }

    outputFile << element << std::endl;

    outputFile.close();
}

// _________________________________________________________________________________________________
void olu::osm::OsmChangeHandler::getIdsOfWaysToUpdateGeo() {
    if (!_nodeHandler.getModifiedNodesWithChangedLocation().empty()) {
        util::BatchHelper::doInBatches(
            _nodeHandler.getModifiedNodesWithChangedLocation(),
            _config.batchSize,
            [this](const std::set<id_t> &batch) {
                for (const auto &wayId: _odf->fetchWaysReferencingNodes(batch)) {
                    if (!_wayHandler.wayInChangeFile(wayId)) {
                        _waysToUpdateGeometry.insert(wayId);
                        _stats->countWayToUpdateGeometry();
                    }
                }
            });
    }
}

// _________________________________________________________________________________________________
void olu::osm::OsmChangeHandler::getIdsOfRelationsToUpdateGeo() {
    // Get ids of relations that reference a modified node
    if (!_nodeHandler.getModifiedNodesWithChangedLocation().empty()) {
        util::BatchHelper::doInBatches(
            _nodeHandler.getModifiedNodesWithChangedLocation(),
            _config.batchSize,
            [this](const std::set<id_t>& batch) {
                for (const auto &relId: _odf->fetchRelationsReferencingNodes(batch)) {
                    if (!_relationHandler.relationInChangeFile(relId)) {
                        _relationsToUpdateGeometry.insert(relId);
                        _stats->countRelationToUpdateGeometry();
                    }
                }
            });
    }

    // Get ids of relations that reference a way with changed geometry
    std::set<id_t> updatedWays;
    for (const auto &wayId: _wayHandler.getModifiedWaysWithChangedMembers()) {
        updatedWays.insert(wayId);
    }
    updatedWays.insert(_waysToUpdateGeometry.begin(), _waysToUpdateGeometry.end());

    if (!updatedWays.empty()) {
        util::BatchHelper::doInBatches(
            updatedWays,
            _config.batchSize,
            [this](const std::set<id_t>& batch) {
                for (const auto &relId: _odf->fetchRelationsReferencingWays(batch)) {
                    if (!_relationHandler.relationInChangeFile(relId) &&
                        !_relationsToUpdateGeometry.contains(relId)) {
                        _relationsToUpdateGeometry.insert(relId);
                        _stats->countRelationToUpdateGeometry();
                    }
                }
            });
    }

    // Get ids of relations that reference a modified relation.
    // Skip this because osm2rdf does not calculate geometries for relations that reference
    // other relations
//        if (!_relationHandler.getModifiedAreas().empty()) {
//            util::BatchHelper::doInBatches(
//                    _relationHandler.getModifiedAreas(),
//            MAX_TRIPLE_COUNT_PER_QUERY,
//            [this](const std::set<id_t>& batch) {
//                auto relationIds = _odf.fetchRelationsReferencingRelations(batch);
//                for (const auto &relId: relationIds) {
//                    if (!_relationHandler.getModifiedAreas().contains(relId)) {
//                        _relationsToUpdateGeometry.insert(relId);
//                    }
//                }
//            });
//        }
}

// _________________________________________________________________________________________________
void olu::osm::OsmChangeHandler::getReferencedRelations() {
    if (!_relationsToUpdateGeometry.empty()) {
        util::BatchHelper::doInBatches(
                _relationsToUpdateGeometry,
            _config.batchSize,
            [this](const std::set<id_t>& batch) {
                for (const auto &relId: _odf->fetchRelationsReferencingRelations(batch)) {
                    if (!_relationsToUpdateGeometry.contains(relId) &&
                        !_relationHandler.getCreatedRelations().contains(relId) &&
                        !_relationHandler.getModifiedAreas().contains(relId)) {
                        _referencesHandler.getReferencedRelations().insert(relId);
                    }
                }
            });
    }
}

// _________________________________________________________________________________________________
void olu::osm::OsmChangeHandler::createDummyElements() {
    const std::size_t count = _referencesHandler.getReferencedNodes().size()
        + _referencesHandler.getReferencedWays().size()
        + _waysToUpdateGeometry.size()
        + _referencesHandler.getReferencedRelations().size()
        + _relationsToUpdateGeometry.size();

    if (count == 0) {
        return;
    }

    _stats->printCurrentStep("Create referenced objects...");
    osm2rdf::util::ProgressBar createProgress(count, _config.showProgress);
    size_t counter = 0;
    createProgress.update(counter);

    createDummyNodes(createProgress, counter);
    createDummyWays(createProgress, counter);
    createDummyRelations(createProgress, counter);

    createProgress.done();
}


// _________________________________________________________________________________________________
void olu::osm::OsmChangeHandler::createDummyNodes(osm2rdf::util::ProgressBar &progress,
                                                  size_t &counter) {
    util::BatchHelper::doInBatches(
        _referencesHandler.getReferencedNodes(),
        _config.batchSize,
        [this, &counter, progress](std::set<id_t> const& batch) mutable {
            progress.update(counter += batch.size());
            for (auto const& node: _odf->fetchNodes(batch)) {
                // Add the dummy node to the buffer if it is not already in the change file
                if (!_nodeHandler.nodeInChangeFile(node.getId())) {
                    addToTmpFile(node.getXml(), cnst::XML_TAG_NODE);
                    _stats->countDummyNode();
                }
            }
        });

    finalizeTmpFile(cnst::PATH_TO_NODE_FILE);
}

// _________________________________________________________________________________________________
void olu::osm::OsmChangeHandler::createDummyWays(osm2rdf::util::ProgressBar &progress,
                                                 size_t &counter) {
    std::set<id_t> wayIds;
    for (const auto &wayId: _referencesHandler.getReferencedWays()) {
        wayIds.insert(wayId);
    }
    wayIds.insert(_waysToUpdateGeometry.begin(), _waysToUpdateGeometry.end());

    util::BatchHelper::doInBatches(
        wayIds,
        _config.batchSize,
        [this, progress, &counter](std::set<id_t> const& batch) mutable {
            progress.update(counter += batch.size());
            for (auto& way: _odf->fetchWays(batch)) {
                // The ways for which the geometry does not need to be updated are already in
                // the tmp file
                if (_wayHandler.getModifiedWays().contains(way.getId())) {
                    continue;
                }

                if (_waysToUpdateGeometry.contains(way.getId())) {
                    _odf->fetchWayInfos(way);
                }

                // Add the dummy way to the buffer if it is not already in the change file
                if (!_wayHandler.wayInChangeFile(way.getId())) {
                    addToTmpFile(way.getXml(), cnst::XML_TAG_WAY);
                    _stats->countDummyWay();
                }
            }
        });

    finalizeTmpFile(cnst::PATH_TO_WAY_FILE);
}

// _________________________________________________________________________________________________
void olu::osm::OsmChangeHandler::createDummyRelations(osm2rdf::util::ProgressBar &progress,
                                                      size_t &counter) {
    std::set<id_t> relations;
    for (const auto &relId: _referencesHandler.getReferencedRelations()) {
        relations.insert(relId);
    }
    relations.insert(_relationsToUpdateGeometry.begin(),
        _relationsToUpdateGeometry.end());

    util::BatchHelper::doInBatches(
        relations,
        _config.batchSize,
        [this, &counter, progress](std::set<id_t> const& batch) mutable {
            progress.update(counter += batch.size());
            for (auto& rel: _odf->fetchRelations(batch)) {
                if (_relationsToUpdateGeometry.contains(rel.getId())) {
                    _odf->fetchRelationInfos(rel);
                }

                // Add the dummy relation to the buffer if it is not already in the change file
                if (!_relationHandler.relationInChangeFile(rel.getId())) {
                    addToTmpFile(rel.getXml(), cnst::XML_TAG_REL);
                    _stats->countDummyRelation();
                }
            }
        });

    finalizeTmpFile(cnst::PATH_TO_RELATION_FILE);
}

// _________________________________________________________________________________________________
void olu::osm::OsmChangeHandler::runUpdateQuery(const std::string &query,
                                                const std::vector<std::string> &prefixes) {
    _stats->countUpdateQuery();
    _sparql.setQuery(query);
    _sparql.setPrefixes(prefixes);

    std::string response;
    try {
        response = _sparql.runUpdate();
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        const std::string msg = "Exception while trying to run sparql update query: "
                                + query.substr(0, std::min<int>(query.size(), 100))
                                + " ...";
        throw OsmChangeHandlerException(msg.c_str());
    }

    if (_config.isQLever) {
        // Update responses are in "[]" so remove them before parsing
        response = response.substr(1, response.size() - 2);
        for (auto doc = _parser.iterate(response);
             auto field: doc.get_object()) {
            if (field.error()) {
                std::cerr << field.error() << std::endl;
                throw OsmDataFetcherException("Error while parsing QLever update response.");
            }

            if (const std::string_view key = field.escaped_key(); key == cnst::KEY_QLEVER_TIME) {
                _stats->logQleverTimingInfoUpdate(field.value().get_object());
            }
        }
    }
}

// _________________________________________________________________________________________________
void olu::osm::OsmChangeHandler::deleteNodesFromDatabase(osm2rdf::util::ProgressBar &progress,
                                                         size_t &counter) {
    util::BatchHelper::doInBatches(
        _nodeHandler.getAllNodes(),
        _config.batchSize,
        [this, progress, &counter](std::set<id_t> const &batch) mutable {
            runUpdateQuery(_queryWriter.writeDeleteQuery(batch, cnst::NAMESPACE_OSM_NODE),
                           cnst::PREFIXES_FOR_NODE_DELETE_QUERY);
            progress.update(counter += batch.size());
        });
}

// _________________________________________________________________________________________________
void olu::osm::OsmChangeHandler::deleteWaysFromDatabase(osm2rdf::util::ProgressBar &progress,
                                                        size_t &counter) {
    std::set<id_t> waysToDelete;
    for (const auto &wayId: _wayHandler.getDeletedWays()) {
        waysToDelete.insert(wayId);
    }
    for (const auto &wayId: _wayHandler.getModifiedWaysWithChangedMembers()) {
        waysToDelete.insert(wayId);
    }
    for (const auto &wayId: _wayHandler.getCreatedWays()) {
        waysToDelete.insert(wayId);
    }

    util::BatchHelper::doInBatches(
        waysToDelete,
        _config.batchSize,
        [this, &counter, progress](std::set<id_t> const &batch) mutable {
            runUpdateQuery(_queryWriter.writeDeleteQuery(batch, cnst::NAMESPACE_OSM_WAY),
                           cnst::PREFIXES_FOR_WAY_DELETE_QUERY);
            progress.update(counter += batch.size());
        });
}

// _________________________________________________________________________________________________
void olu::osm::OsmChangeHandler::deleteWaysMetaDataAndTags(osm2rdf::util::ProgressBar &progress,
                                                           size_t &counter) {
    util::BatchHelper::doInBatches(
        _wayHandler.getModifiedWays(),
        _config.batchSize,
        [this, &counter, progress](std::set<id_t> const &batch) mutable {
            runUpdateQuery(_queryWriter.writeDeleteQueryForMetaAndTags(batch, cnst::NAMESPACE_OSM_WAY),
                           cnst::PREFIXES_FOR_WAY_DELETE_META_AND_TAGS_QUERY);
            progress.update(counter += batch.size());
        });
}

// _________________________________________________________________________________________________
void olu::osm::OsmChangeHandler::deleteRelationsMetaDataAndTags(osm2rdf::util::ProgressBar &progress,
                                                                size_t &counter) {
    util::BatchHelper::doInBatches(
        _relationHandler.getModifiedRelations(),
        _config.batchSize,
        [this, &counter, progress](std::set<id_t> const &batch) mutable {
            runUpdateQuery(_queryWriter.writeDeleteQueryForMetaAndTags(batch, cnst::NAMESPACE_OSM_REL),
                           cnst::PREFIXES_FOR_RELATION_DELETE_META_AND_TAGS_QUERY);
            progress.update(counter += batch.size());
        });
}

// _________________________________________________________________________________________________
void olu::osm::OsmChangeHandler::deleteWaysGeometry(osm2rdf::util::ProgressBar &progress,
                                                    size_t &counter) {
    util::BatchHelper::doInBatches(
        _waysToUpdateGeometry,
        _config.batchSize,
        [this, &counter, progress](std::set<id_t> const &batch) mutable {
            runUpdateQuery(_queryWriter.writeDeleteQueryForGeometry(batch, cnst::NAMESPACE_OSM_WAY),
                           cnst::PREFIXES_FOR_WAY_DELETE_GEOMETRY_QUERY);
            progress.update(counter += batch.size());
        });
}

// _________________________________________________________________________________________________
void olu::osm::OsmChangeHandler::deleteRelationsFromDatabase(osm2rdf::util::ProgressBar &progress,
                                                             size_t &counter) {
    std::set<id_t> relationsToDelete;
    for (const auto &wayId: _relationHandler.getDeletedRelations()) {
        relationsToDelete.insert(wayId);
    }
    for (const auto &wayId: _relationHandler.getModifiedRelationsWithChangedMembers()) {
        relationsToDelete.insert(wayId);
    }
    for (const auto &wayId: _relationHandler.getCreatedRelations()) {
        relationsToDelete.insert(wayId);
    }

    util::BatchHelper::doInBatches(
        relationsToDelete,
        _config.batchSize,
        [this, &counter, progress](std::set<id_t> const &batch) mutable {
            runUpdateQuery(_queryWriter.writeDeleteQuery(batch, cnst::NAMESPACE_OSM_REL),
                           cnst::PREFIXES_FOR_RELATION_DELETE_QUERY);
            progress.update(counter += batch.size());
        });
}

// _________________________________________________________________________________________________
void olu::osm::OsmChangeHandler::deleteRelationsGeometry(osm2rdf::util::ProgressBar &progress,
                                                         size_t &counter) {
    util::BatchHelper::doInBatches(
        _relationsToUpdateGeometry,
        _config.batchSize,
        [this, &counter, progress](std::set<id_t> const &batch) mutable {
            runUpdateQuery(_queryWriter.writeDeleteQueryForGeometry(batch, cnst::NAMESPACE_OSM_REL),
                           cnst::PREFIXES_FOR_RELATION_DELETE_GEOMETRY_QUERY);
            progress.update(counter += batch.size());
        });
}

// _________________________________________________________________________________________________
void olu::osm::OsmChangeHandler::deleteTriplesFromDatabase() {
    const std::size_t count = _nodeHandler.getNumOfNodes()
        + _wayHandler.getNumOfWays()
        + _waysToUpdateGeometry.size()
        + _relationHandler.getNumOfRelations()
        + _relationsToUpdateGeometry.size();

    if (count == 0) {
        _stats->printCurrentStep("No elements to delete...");
        return;
    }

    _stats->printCurrentStep("Deleting elements from database...");
    osm2rdf::util::ProgressBar deleteProgress(count, _config.showProgress);
    size_t counter = 0;
    deleteProgress.update(counter);

    deleteNodesFromDatabase(deleteProgress, counter);
    deleteWaysFromDatabase(deleteProgress, counter);
    deleteWaysMetaDataAndTags(deleteProgress,counter);
    deleteWaysGeometry(deleteProgress, counter);
    deleteRelationsFromDatabase(deleteProgress, counter);
    deleteRelationsMetaDataAndTags(deleteProgress,counter);
    deleteRelationsGeometry(deleteProgress, counter);

    deleteProgress.done();
}


// _________________________________________________________________________________________________
void olu::osm::OsmChangeHandler::insertTriplesToDatabase() {
    _stats->startTimeFilteringTriples();
    auto triples = filterRelevantTriples();
    _stats->endTimeFilteringTriples();

    if (triples.empty()) {
        _stats->printCurrentStep("No triples to insert into database...");
        return;
    }

    _stats->printCurrentStep("Inserting triples into database...");
    osm2rdf::util::ProgressBar insertProgress(triples.size(), _config.showProgress);
    size_t counter = 0;
    insertProgress.update(counter);

    std::vector<std::string> tripleBatch;
    for (size_t i = 0; i < triples.size(); ++i) {
        auto [s, p, o] = triples[i];
        std::ostringstream triple;
        if (o.starts_with("_")) {
            triple << s;
            triple << " ";
            triple << p;
            triple << "[ ";

            while (true) {
                i++;
                if (auto [next_s, next_p, next_o] = triples[i]; next_s.starts_with("_")) {
                    triple << next_p;
                    triple << " ";
                    triple << next_o;
                    triple << "; ";
                } else {
                    i--;
                    break;
                }
            }

            triple << " ]";
        } else {
            triple << s;
            triple << " ";
            triple << p;
            triple << " ";
            triple << o;
        }

        tripleBatch.emplace_back(triple.str());

        if (tripleBatch.size() == _config.batchSize || i == triples.size() - 1) {
            runUpdateQuery(_queryWriter.writeInsertQuery(tripleBatch), cnst::DEFAULT_PREFIXES);
            tripleBatch.clear();

            if (i == triples.size() - 1) {
                insertProgress.done();
            } else {
                insertProgress.update(counter += _config.batchSize);
            }
        }
    }
}

// _________________________________________________________________________________________________
std::vector<olu::triple_t> olu::osm::OsmChangeHandler::filterRelevantTriples() const {
    // Get the ids of all nodes, ways and relations for which the triples should be inserted
    // into the database
    std::set<id_t> nodesToInsert;
    for (const auto &nodeId: _nodeHandler.getCreatedNodes()) {
        nodesToInsert.insert(nodeId);
    }
    for (const auto &nodeId: _nodeHandler.getModifiedNodes()) {
        nodesToInsert.insert(nodeId);
    }
    for (const auto &nodeId: _nodeHandler.getModifiedNodesWithChangedLocation()) {
        nodesToInsert.insert(nodeId);
    }

    std::set<id_t> waysToInsert;
    for (const auto &wayId: _wayHandler.getCreatedWays()) {
        waysToInsert.insert(wayId);
    }
    for (const auto &wayId: _wayHandler.getModifiedWaysWithChangedMembers()) {
        waysToInsert.insert(wayId);
    }

    std::set<id_t> relationsToInsert;
    for (const auto &relId: _relationHandler.getCreatedRelations()) {
        relationsToInsert.insert(relId);
    }
    for (const auto &relId: _relationHandler.getModifiedRelationsWithChangedMembers()) {
        relationsToInsert.insert(relId);
    }

    // Triples that should be inserted into the database
    std::vector<triple_t> relevantTriples;
    // current link object, for example, member nodes or geometries (can also be blank nodes)
    std::string currentLink;

    // Loop over each triple that osm2rdf outputs
    std::string line;
    std::ifstream osm2rdfOutput;
    osm2rdfOutput.open(cnst::PATH_TO_OUTPUT_FILE);
    while (std::getline(osm2rdfOutput, line)) {
        // Filer out prefixes at the start of the document
        if (line.starts_with("@")) { continue; }
        _stats->countTriple();

        auto triple = util::TtlHelper::parseTriple(line);
        const auto& [subject, predicate, object] = triple;

        // Check if there is currently a link set
        if (!currentLink.empty() && currentLink == subject) {
            relevantTriples.emplace_back(subject, predicate, util::XmlHelper::xmlDecode(object));
            continue;
        }

        // Check all triples that are in the "osmnode" namespace.
        if (util::TtlHelper::isInNamespaceForOsmObject(subject, OsmObjectType::NODE)) {
            filterNodeTriple(triple, nodesToInsert, relevantTriples, currentLink);
            continue;
        }

        // Check all triples that are in the "osmway" namespace.
        if (util::TtlHelper::isInNamespaceForOsmObject(subject, OsmObjectType::WAY)) {
            filterWayTriple(triple, waysToInsert, relevantTriples, currentLink);
            continue;
        }

        // Check all triples that are in the "osmrel" namespace.
        if (util::TtlHelper::isInNamespaceForOsmObject(subject, OsmObjectType::RELATION)) {
            filterRelationTriple(triple, relationsToInsert, relevantTriples, currentLink);
        }
    }

    osm2rdfOutput.close();
    _stats->setNumberOfTriplesToInsert(relevantTriples.size());
    return relevantTriples;
}

// _________________________________________________________________________________________________
void olu::osm::OsmChangeHandler::filterNodeTriple(const triple_t &nodeTriple,
                                                  const std::set<id_t> &nodesToInsert,
                                                  std::vector<triple_t> &relevantTriples,
                                                  std::string &currentLink) {
    if (const auto& [subject, predicate, object] = nodeTriple;
        nodesToInsert.contains(util::TtlHelper::parseId(subject))) {
        // Decode tag values
        relevantTriples.emplace_back(subject, predicate, util::XmlHelper::xmlDecode(object));

        if (util::TtlHelper::hasRelevantObject(predicate, OsmObjectType::NODE)) {
            currentLink = object;
        }
    }
}

// _________________________________________________________________________________________________
void olu::osm::OsmChangeHandler::filterWayTriple(const triple_t &wayTriple,
                                                 const std::set<id_t> &waysToInsert,
                                                 std::vector<triple_t> &relevantTriples,
                                                 std::string &currentLink) const {
    const auto& [subject, predicate, object] = wayTriple;
    const auto wayId = util::TtlHelper::parseId(subject);

    if (waysToInsert.contains(wayId)) {
        relevantTriples.emplace_back(subject, predicate, util::XmlHelper::xmlDecode(object));

        // Check if the object links to a relevant triple for the geometry of the
        // relation
        // (For example, "osmway:member" links to the object which describes
        // the member)
        if (util::TtlHelper::hasRelevantObject(predicate, OsmObjectType::WAY)) {
            currentLink = object;
        }
    }

    // We only update the triples for the metadata and tags of the ways that are in the
    // modifiedWays set.
    if (_wayHandler.getModifiedWays().contains(wayId)) {
        if (util::TtlHelper::isMetadataOrTagPredicate(predicate, OsmObjectType::WAY)) {
            relevantTriples.emplace_back(subject, predicate, util::XmlHelper::xmlDecode(object));
        }
    }

    // We only update the triples that describe the geometry of the ways that are in the
    // _waysToUpdateGeometry set.
    if (_waysToUpdateGeometry.contains(wayId)) {
        if (util::TtlHelper::isGeometryPredicate(predicate, OsmObjectType::WAY)) {
            relevantTriples.emplace_back(subject, predicate, util::XmlHelper::xmlDecode(object));
        }

        // Check if the object links to a relevant triple for the geometry of the
        // relation (For example, "osm2rdfgeom:osm_node_1")
        if (util::TtlHelper::hasRelevantObject(predicate, OsmObjectType::WAY)) {
            currentLink = object;
        }
    }
}

// _________________________________________________________________________________________________
void olu::osm::OsmChangeHandler::filterRelationTriple(const triple_t &relationTriple,
                                                 const std::set<id_t> &relationsToInsert,
                                                 std::vector<triple_t> &relevantTriples,
                                                 std::string &currentLink) const {
    const auto& [subject, predicate, object] = relationTriple;
    const auto relId = util::TtlHelper::parseId(subject);

    if (relationsToInsert.contains(relId)) {
        relevantTriples.emplace_back(subject, predicate, object);

        // (For example, "osmrel:member" links to the object which describes
        // the member)
        if (util::TtlHelper::hasRelevantObject(predicate, OsmObjectType::RELATION)) {
            currentLink = object;
        }
    }

    // We only update the triples for the metadata and tags of the relations that are
    // in the modifiedRelations set.
    if (_relationHandler.getModifiedRelations().contains(relId)) {
        if (util::TtlHelper::isMetadataOrTagPredicate(predicate, OsmObjectType::RELATION)) {
            relevantTriples.emplace_back(subject, predicate, util::XmlHelper::xmlDecode(object));
        }
    }

    // We only update the triples that describe the geometry of the relations that are
    // in the _relationsToUpdateGeometry set.
    if (_relationsToUpdateGeometry.contains(relId)) {
        if (util::TtlHelper::isGeometryPredicate(predicate, OsmObjectType::RELATION)) {
            relevantTriples.emplace_back(subject, predicate, util::XmlHelper::xmlDecode(object));
        }

        // Check if the object links to a relevant triple for the geometry of the
        // relation (For example, "osm2rdfgeom:osm_node_1")
        if (util::TtlHelper::hasRelevantObject(predicate, OsmObjectType::RELATION)) {
            currentLink = object;
        }
    }
}