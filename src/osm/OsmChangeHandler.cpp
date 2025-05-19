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
#include "osm/NodeHandler.h"
#include "util/XmlReader.h"
#include "config/Constants.h"
#include "sparql/QueryWriter.h"
#include "util/OsmObjectHelper.h"
#include "util/TtlHelper.h"
#include "osm2rdf/util/Time.h"
#include "util/BatchHelper.h"
#include "osm2rdf/util/ProgressBar.h"

#include <string>
#include <iostream>
#include <set>
#include <regex>
#include <osmium/visitor.hpp>
#include <osmium/builder/osm_object_builder.hpp>
#include <osmium/io/pbf_output.hpp>
#include <osmium/io/writer.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/osm/object_comparisons.hpp>

namespace cnst = olu::config::constants;

namespace olu::osm {
    OsmChangeHandler::OsmChangeHandler(const config::Config &config) :
        _config(config),
        _sparql(config),
        _queryWriter(config),
        _odf(config),
        _nodeHandler(config),
        _wayHandler(config),
        _relationHandler(config),
        _referencesHandler(
            _config, _odf, _nodeHandler, _wayHandler, _relationHandler) {
        createTmpFiles();
    }

    void OsmChangeHandler::run() {
        osmium::io::Reader nodeReader{ cnst::PATH_TO_CHANGE_FILE,
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
        _nodeHandler.checkNodesForLocationChange();
        nodeReader.close();

        osmium::io::Reader wayReader{ cnst::PATH_TO_CHANGE_FILE,
            osmium::osm_entity_bits::way,
            osmium::io::read_meta::no};
        osmium::apply(wayReader, _wayHandler);
        // Check for modified ways if the members have changed.
        // If so, the way is added to the _modifiedWaysWithChangedMembers set, otherwise to the
        // _modifiedWays set
        _wayHandler.checkWaysForMemberChange(_nodeHandler.getModifiedNodesWithChangedLocation());
        wayReader.close();

        osmium::io::Reader relationReader{ cnst::PATH_TO_CHANGE_FILE,
            osmium::osm_entity_bits::relation,
            osmium::io::read_meta::no};
        osmium::apply(relationReader, _relationHandler);
        // Check for modified relations if the members have changed.
        // If so, the relation is added to the _modifiedRelationsWithChangedMembers set, otherwise
        // to the _modifiedRelations set
        _relationHandler.checkRelationsForMemberChange(
            _nodeHandler.getModifiedNodesWithChangedLocation(),
            _wayHandler.getModifiedWaysWithChangedMembers());
        relationReader.close();

        if (_nodeHandler.empty() && _wayHandler.empty() && _relationHandler.empty()) {
            throw OsmChangeHandlerException("Change file is empty.");
        }

        // Loop over the ways and relations a second time to store the ids of the referenced
        // elements.
        // We will need to retrieve them later from the endpoint (if they are not already
        // in the change file) for osm2rdf to calculate the geometries.
        osmium::io::Reader referencesReader{ cnst::PATH_TO_CHANGE_FILE,
            osmium::osm_entity_bits::way | osmium::osm_entity_bits::relation,
            osmium::io::read_meta::no};
        osmium::apply(referencesReader, _referencesHandler);
        referencesReader.close();

        getIdsOfWaysToUpdateGeo();
        getIdsOfRelationsToUpdateGeo();

        std::cout
        << osm2rdf::util::currentTimeFormatted()
        << "Fetch references..."
        << std::endl;
        // Fetch the ids of all nodes and ways that are referenced by relations which are not in the
        // change file.
        // We will later create placeholder objects for them (for nodes, which hold the locations,
        // and for ways and relations, which hold the members) so that osm2rdf can calculate the
        // geometries.
        std::set relationIds(_referencesHandler.getReferencedRelations());
        relationIds.insert(_relationsToUpdateGeometry.begin(), _relationsToUpdateGeometry.end());
        _referencesHandler.getReferencesForRelations(relationIds);

        // Fetch the ids of all nodes that are referenced by ways which are not in the change file
        std::set wayIds(_referencesHandler.getReferencedWays());
        wayIds.insert(_waysToUpdateGeometry.begin(), _waysToUpdateGeometry.end());
        _referencesHandler.getReferencesForWays(wayIds);

        // Create the dummy objects for the nodes, ways and relations that are referenced by
        // elements in the change file,
        // as well as the ways and relations for which the geometry needs to be updated.
        createDummyElements();

        try {
            Osm2ttl(_config).convert();
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            throw OsmChangeHandlerException("Exception while trying to convert osm element to ttl");
        }

        // Delete and insert elements from database
        deleteTriplesFromDatabase();
        insertTriplesToDatabase();
        
        _nodeHandler.printNodeStatistics();
        _wayHandler.printWayStatistics();
        _relationHandler.printRelationStatistics();
        
        std::cout << osm2rdf::util::currentTimeFormatted() << "updated geometries for "
            << _waysToUpdateGeometry.size() << " ways and " << _relationsToUpdateGeometry.size()
            << " relations" << std::endl;
    }

    void OsmChangeHandler::createTmpFiles() {
        initTmpFile(cnst::PATH_TO_NODE_FILE);
        initTmpFile(cnst::PATH_TO_WAY_FILE);
        initTmpFile(cnst::PATH_TO_RELATION_FILE);
    }

    void OsmChangeHandler::initTmpFile(const std::string& filepath) {
        std::ofstream file(filepath, std::ios::trunc);
        file << "<osm version=\"0.6\">" << std::endl;
        file.close();
    }

    void OsmChangeHandler::finalizeTmpFile(const std::string& filepath) {
        std::ofstream file(filepath, std::ios::app);
        file << "</osm>" << std::endl;
        file.close();
    }

    void OsmChangeHandler::addToTmpFile(const std::string& element,
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

    void OsmChangeHandler::getIdsOfWaysToUpdateGeo() {
        if (!_nodeHandler.getModifiedNodesWithChangedLocation().empty()) {
            util::BatchHelper::doInBatches(
                _nodeHandler.getModifiedNodesWithChangedLocation(),
                _config.maxValuesPerQuery,
                [this](const std::set<id_t> &batch) {
                    for (const auto &wayId: _odf.fetchWaysReferencingNodes(batch)) {
                        if (!_wayHandler.wayInChangeFile(wayId)) {
                            _waysToUpdateGeometry.insert(wayId);
                        }
                    }
                });
        }
    }

    void OsmChangeHandler::getIdsOfRelationsToUpdateGeo() {
        // Get ids of relations that reference a modified node
        if (!_nodeHandler.getModifiedNodesWithChangedLocation().empty()) {
            util::BatchHelper::doInBatches(
                _nodeHandler.getModifiedNodesWithChangedLocation(),
                _config.maxValuesPerQuery,
                [this](const std::set<id_t>& batch) {
                    for (const auto &relId: _odf.fetchRelationsReferencingNodes(batch)) {
                        if (!_relationHandler.relationInChangeFile(relId)) {
                            _relationsToUpdateGeometry.insert(relId);
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
                _config.maxValuesPerQuery,
                [this](const std::set<id_t>& batch) {
                    for (const auto &relId: _odf.fetchRelationsReferencingWays(batch)) {
                        if (!_relationHandler.relationInChangeFile(relId)) {
                            _relationsToUpdateGeometry.insert(relId);
                        }
                    }
                });
        }

        // Get ids of relations that reference a modified relation. Skip this because
        // osm2rdf does not calculate geometries for relations that reference other relations
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

    void OsmChangeHandler::getReferencedRelations() {
        if (!_relationsToUpdateGeometry.empty()) {
            util::BatchHelper::doInBatches(
                    _relationsToUpdateGeometry,
                _config.maxValuesPerQuery,
                [this](const std::set<id_t>& batch) {
                    auto relationIds = _odf.fetchRelationsReferencingRelations(batch);
                    for (const auto &relId: relationIds) {
                        if (!_relationsToUpdateGeometry.contains(relId) &&
                            !_relationHandler.getCreatedRelations().contains(relId) &&
                            !_relationHandler.getModifiedAreas().contains(relId)) {
                            _referencesHandler.getReferencedRelations().insert(relId);
                        }
                    }
                });
        }
    }

    void OsmChangeHandler::createDummyElements() {
        const std::size_t count = _referencesHandler.getReferencedNodes().size()
            + _referencesHandler.getReferencedWays().size()
            + _waysToUpdateGeometry.size()
            + _referencesHandler.getReferencedRelations().size()
            + _relationsToUpdateGeometry.size();

        if (count == 0) {
            return;
        }

        std::cout
        << osm2rdf::util::currentTimeFormatted()
        << "Create referenced objects..."
        << std::endl;
        osm2rdf::util::ProgressBar createProgress(count, _config.showProgress);
        size_t counter = 0;
        createProgress.update(counter);

        createDummyNodes(createProgress, counter);
        createDummyWays(createProgress, counter);
        createDummyRelations(createProgress, counter);

        createProgress.done();
    }


    void OsmChangeHandler::createDummyNodes(osm2rdf::util::ProgressBar &progress, size_t &counter) {
        util::BatchHelper::doInBatches(
            _referencesHandler.getReferencedNodes(),
            _config.maxValuesPerQuery,
            [this, &counter, progress](std::set<id_t> const& batch) mutable {
                progress.update(counter += batch.size());
                for (auto const& node: _odf.fetchNodes(batch)) {
                    // Add the dummy node to the buffer if it is not already in the change file
                    if (!_nodeHandler.nodeInChangeFile(node.getId())) {
                        addToTmpFile(node.getXml(), cnst::XML_TAG_NODE);
                    }
                }
            });

        finalizeTmpFile(cnst::PATH_TO_NODE_FILE);
    }

    void OsmChangeHandler::createDummyWays(osm2rdf::util::ProgressBar &progress, size_t &counter) {
        std::set<id_t> wayIds;
        for (const auto &wayId: _referencesHandler.getReferencedWays()) {
            wayIds.insert(wayId);
        }
        wayIds.insert(_waysToUpdateGeometry.begin(), _waysToUpdateGeometry.end());

        util::BatchHelper::doInBatches(
            wayIds,
            _config.maxValuesPerQuery,
            [this, progress, &counter](std::set<id_t> const& batch) mutable {
                progress.update(counter += batch.size());
                for (auto& way: _odf.fetchWays(batch)) {
                    // The ways for which the geometry does not need to be updated are already in
                    // the tmp file
                    if (_wayHandler.getModifiedWays().contains(way.getId())) {
                        continue;
                    }

                    if (_waysToUpdateGeometry.contains(way.getId())) {
                        _odf.fetchWayInfos(way);
                    }

                    // Add the dummy way to the buffer if it is not already in the change file
                    if (!_wayHandler.wayInChangeFile(way.getId())) {
                        addToTmpFile(way.getXml(), cnst::XML_TAG_WAY);
                    }
                }
            });

        finalizeTmpFile(cnst::PATH_TO_WAY_FILE);
    }

    void OsmChangeHandler::createDummyRelations(osm2rdf::util::ProgressBar &progress,
                                                size_t &counter) {
        std::set<id_t> relations;
        for (const auto &relId: _referencesHandler.getReferencedRelations()) {
            relations.insert(relId);
        }
        relations.insert(_relationsToUpdateGeometry.begin(),
            _relationsToUpdateGeometry.end());

        util::BatchHelper::doInBatches(
            relations,
            _config.maxValuesPerQuery,
            [this, &counter, progress](std::set<id_t> const& batch) mutable {
                progress.update(counter += batch.size());
                for (auto& rel: _odf.fetchRelations(batch)) {
                    if (_relationsToUpdateGeometry.contains(rel.getId())) {
                        _odf.fetchRelationInfos(rel);
                    }

                    // Add the dummy relation to the buffer if it is not already in the change file
                    if (_relationHandler.relationInChangeFile(rel.getId())) {
                        addToTmpFile(rel.getXml(), cnst::XML_TAG_REL);
                    }
                }
            });

        finalizeTmpFile(cnst::PATH_TO_RELATION_FILE);
    }

    void
    OsmChangeHandler::runUpdateQuery(const std::string &query,
                                     const std::vector<std::string> &prefixes) {
        _sparql.setQuery(query);
        _sparql.setPrefixes(prefixes);
        try {
            _sparql.runUpdate();
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            const std::string msg = "Exception while trying to run sparql update query: "
                                    + query.substr(0, std::min<int>(query.size(), 100))
                                    + " ...";
            throw OsmChangeHandlerException(msg.c_str());
        }
    }

    void OsmChangeHandler::deleteNodesFromDatabase(osm2rdf::util::ProgressBar &progress,
                                                   size_t &counter) {
        util::BatchHelper::doInBatches(
            _nodeHandler.getAllNodes(),
            _config.maxValuesPerQuery,
            [this, progress, &counter](std::set<id_t> const &batch) mutable {
                runUpdateQuery(_queryWriter.writeDeleteQuery(batch, cnst::NAMESPACE_OSM_NODE),
                               cnst::PREFIXES_FOR_NODE_DELETE_QUERY);
                progress.update(counter += batch.size());
            });
    }

    void OsmChangeHandler::deleteWaysFromDatabase(osm2rdf::util::ProgressBar &progress,
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
            _config.maxValuesPerQuery,
            [this, &counter, progress](std::set<id_t> const &batch) mutable {
                runUpdateQuery(_queryWriter.writeDeleteQuery(batch, cnst::NAMESPACE_OSM_WAY),
                               cnst::PREFIXES_FOR_WAY_DELETE_QUERY);
                progress.update(counter += batch.size());
            });
    }

    void OsmChangeHandler::deleteWaysMetaDataAndTags(osm2rdf::util::ProgressBar &progress,
                                                    size_t &counter) {
        util::BatchHelper::doInBatches(
            _wayHandler.getModifiedWays(),
            _config.maxValuesPerQuery,
            [this, &counter, progress](std::set<id_t> const &batch) mutable {
                runUpdateQuery(_queryWriter.writeDeleteQueryForMetaAndTags(batch, cnst::NAMESPACE_OSM_WAY),
                               cnst::PREFIXES_FOR_WAY_DELETE_META_AND_TAGS_QUERY);
                progress.update(counter += batch.size());
            });
    }

    void OsmChangeHandler::deleteRelationsMetaDataAndTags(osm2rdf::util::ProgressBar &progress,
                                                          size_t &counter) {
        util::BatchHelper::doInBatches(
            _relationHandler.getModifiedRelations(),
            _config.maxValuesPerQuery,
            [this, &counter, progress](std::set<id_t> const &batch) mutable {
                runUpdateQuery(_queryWriter.writeDeleteQueryForMetaAndTags(batch, cnst::NAMESPACE_OSM_REL),
                               cnst::PREFIXES_FOR_RELATION_DELETE_META_AND_TAGS_QUERY);
                progress.update(counter += batch.size());
            });
    }

    void OsmChangeHandler::deleteWaysGeometry(osm2rdf::util::ProgressBar &progress,
                                                size_t &counter) {
        util::BatchHelper::doInBatches(
            _waysToUpdateGeometry,
            _config.maxValuesPerQuery,
            [this, &counter, progress](std::set<id_t> const &batch) mutable {
                runUpdateQuery(_queryWriter.writeDeleteQueryForGeometry(batch, cnst::NAMESPACE_OSM_WAY),
                               cnst::PREFIXES_FOR_WAY_DELETE_GEOMETRY_QUERY);
                progress.update(counter += batch.size());
            });
    }

    void OsmChangeHandler::deleteRelationsFromDatabase(osm2rdf::util::ProgressBar &progress,
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
            _config.maxValuesPerQuery,
            [this, &counter, progress](std::set<id_t> const &batch) mutable {
                runUpdateQuery(_queryWriter.writeDeleteQuery(batch, cnst::NAMESPACE_OSM_REL),
                               cnst::PREFIXES_FOR_RELATION_DELETE_QUERY);
                progress.update(counter += batch.size());
            });
    }

    void OsmChangeHandler::deleteRelationsGeometry(osm2rdf::util::ProgressBar &progress,
                                                   size_t &counter) {
        util::BatchHelper::doInBatches(
            _relationsToUpdateGeometry,
            _config.maxValuesPerQuery,
            [this, &counter, progress](std::set<id_t> const &batch) mutable {
                runUpdateQuery(_queryWriter.writeDeleteQueryForGeometry(batch, cnst::NAMESPACE_OSM_REL),
                               cnst::PREFIXES_FOR_RELATION_DELETE_GEOMETRY_QUERY);
                progress.update(counter += batch.size());
            });
    }

    void OsmChangeHandler::deleteTriplesFromDatabase() {
        const std::size_t count = _nodeHandler.getDeletedNodes().size() + _nodeHandler.getModifiedNodes().size()
            + _nodeHandler.getModifiedNodesWithChangedLocation().size()
            + _wayHandler.getDeletedWays().size() + _wayHandler.getModifiedWays().size() + _wayHandler.getModifiedWaysWithChangedMembers().size()
            + _waysToUpdateGeometry.size()
            + _relationHandler.getDeletedRelations().size() + _relationHandler.getModifiedRelations().size()
            + _relationHandler.getModifiedRelationsWithChangedMembers().size()
            + _relationsToUpdateGeometry.size();

        if (count == 0) {
            std::cout
            << osm2rdf::util::currentTimeFormatted()
            << "No elements to delete..."
            << std::endl;
            return;
        }

        std::cout
        << osm2rdf::util::currentTimeFormatted()
        << "Deleting elements from database..."
        << std::endl;
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


    void OsmChangeHandler::insertTriplesToDatabase() {
        auto triples = filterRelevantTriples();

        if (triples.empty()) {
            std::cout
            << osm2rdf::util::currentTimeFormatted()
            << "No triples to insert into database..."
            << std::endl;
            return;
        }

        std::cout
        << osm2rdf::util::currentTimeFormatted()
        << "Inserting triples into database..."
        << std::endl;
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

            if (tripleBatch.size() == _config.maxValuesPerQuery || i == triples.size() - 1) {
                runUpdateQuery(_queryWriter.writeInsertQuery(tripleBatch), cnst::DEFAULT_PREFIXES);
                tripleBatch.clear();

                if (i == triples.size() - 1) {
                    insertProgress.done();
                } else {
                    insertProgress.update(counter += _config.maxValuesPerQuery);
                }
            }
        }
    }

    std::vector<triple_t> OsmChangeHandler::filterRelevantTriples() {
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
        // current link object, for example member nodes or geometries
        std::string currentLink;

        // Loop over each triple that osm2rdf outputs
        std::string line;
        std::ifstream osm2rdfOutput;
        osm2rdfOutput.open(cnst::PATH_TO_OUTPUT_FILE);
        while (std::getline(osm2rdfOutput, line)) {
            if (line.starts_with("@")) {
                continue;
            }

            auto [sub, pre, obj] = util::TtlHelper::getTriple(line);

            // Decode tag values
            if (pre.starts_with(cnst::NAMESPACE_OSM_KEY)) {
                obj = util::XmlReader::xmlDecode(obj);
            }

            // Check if there is currently a link set
            if (!currentLink.empty() && currentLink == sub) {
                relevantTriples.emplace_back(sub, pre, obj);
                continue;
            }

            // Check for relevant nodes
            if (util::TtlHelper::isRelevantNamespace(sub, cnst::XML_TAG_NODE)) {
                if (nodesToInsert.contains(util::TtlHelper::getIdFromSubject(sub, cnst::XML_TAG_NODE))) {

                    relevantTriples.emplace_back(sub, pre, obj);

                    if (util::TtlHelper::hasRelevantObject(pre, cnst::XML_TAG_NODE)) {
                        currentLink = obj;
                    }
                }

                continue;
            }

            // Check for relevant ways
            if (util::TtlHelper::isRelevantNamespace(sub, cnst::XML_TAG_WAY)) {
                auto wayId = util::TtlHelper::getIdFromSubject(sub, cnst::XML_TAG_WAY);

                if (waysToInsert.contains(wayId)) {
                    relevantTriples.emplace_back(sub, pre, obj);

                    if (util::TtlHelper::hasRelevantObject(pre, cnst::XML_TAG_WAY)) {
                        currentLink = obj;
                    }
                }

                // Check for relevant meta data and tag values
                if (_wayHandler.getModifiedWays().contains(wayId)) {
                    if (pre.starts_with(cnst::NAMESPACE_OSM_KEY) ||
                        pre.starts_with(cnst::NAMESPACE_OSM_META) ||
                        pre.starts_with(cnst::PREFIXED_OSM2RDF_FACTS)) {
                        relevantTriples.emplace_back(sub, pre, obj);
                    }
                }

                if (_waysToUpdateGeometry.contains(wayId)) {
                    if (pre.starts_with(cnst::NAMESPACE_OSM2RDF_GEOM) ||
                        pre.starts_with(config::constants::PREFIXED_OSM2RDF_LENGTH)) {
                        relevantTriples.emplace_back(sub, pre, obj);
                    }

                    if (util::TtlHelper::hasRelevantObject(pre, cnst::XML_TAG_WAY)) {
                        currentLink = obj;
                    }
                }

                continue;
            }

            // Check for relevant relations
            if (util::TtlHelper::isRelevantNamespace(sub, cnst::XML_TAG_REL)) {
                auto relId = util::TtlHelper::getIdFromSubject(sub, cnst::XML_TAG_REL);
                if (relationsToInsert.contains(relId)) {
                    relevantTriples.emplace_back(sub, pre, obj);

                    if (util::TtlHelper::hasRelevantObject(pre, cnst::XML_TAG_REL)) {
                        currentLink = obj;
                    }
                }

                // Check for relevant meta data and tag values
                if (_relationHandler.getModifiedRelations().contains(relId)) {
                    if (pre.starts_with(cnst::NAMESPACE_OSM_KEY) ||
                        pre.starts_with(cnst::NAMESPACE_OSM_META) ||
                        pre.starts_with(cnst::PREFIXED_OSM2RDF_FACTS)) {
                        relevantTriples.emplace_back(sub, pre, obj);
                        }
                }

                if (_relationsToUpdateGeometry.contains(relId)) {
                    if (pre.starts_with(cnst::NAMESPACE_OSM2RDF_GEOM) ||
                        pre.starts_with(cnst::PREFIXED_OSM2RDF_LENGTH) ||
                        pre.starts_with(cnst::PREFIXED_OSM2RDF_AREA)) {
                        relevantTriples.emplace_back(sub, pre, obj);
                    }

                    if (util::TtlHelper::hasRelevantObject(pre, cnst::XML_TAG_REL)) {
                        currentLink = obj;
                    }
                }
            }
        }

        osm2rdfOutput.close();
        return relevantTriples;
    }

} // namespace olu::osm
