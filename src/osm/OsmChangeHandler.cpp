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
#include "util/XmlReader.h"
#include "util/Decompressor.h"
#include "config/Constants.h"
#include "sparql/QueryWriter.h"
#include "util/OsmObjectHelper.h"
#include "util/TtlHelper.h"
#include "osm2rdf/util/Time.h"

#include <boost/property_tree/ptree.hpp>
#include <string>
#include <iostream>
#include <set>
#include <regex>
#include <sys/stat.h>

#include "osm2rdf/util/ProgressBar.h"

// The maximum number of values that should be in a query to the QLever endpoint.
static inline constexpr int MAX_VALUES_PER_QUERY = 1024;

namespace cnst = olu::config::constants;

void doInBatches(std::set<olu::id_t>& set, const long elementsPerBatch,
                 std::function<void(std::set<olu::id_t>)> func) {
    std::vector vector(set.begin(), set.end());
    std::vector<std::set<olu::id_t> > vectorBatches;
    for (auto it = vector.cbegin(), e = vector.cend(); it != vector.cend(); it = e) {
        e = it + std::min<std::size_t>(vector.end() - it, elementsPerBatch);
        vectorBatches.emplace_back(it, e);
    }

    for (const auto &vectorBatch: vectorBatches) {
        func(vectorBatch);
    }
}

namespace olu::osm {
    OsmChangeHandler::OsmChangeHandler(const config::Config &config) : _config(config),
                                                                       _sparql(config),
                                                                       _queryWriter(config),
                                                                       _odf(config) {
        try {
            const auto decompressed = util::Decompressor::readGzip(cnst::PATH_TO_CHANGE_FILE);
            util::XmlReader::populatePTreeFromString(decompressed, _osmChangeElement);
            _osmChangeElement = _osmChangeElement.get_child(cnst::XML_TAG_OSM_CHANGE);
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            throw OsmChangeHandlerException(
                "Exception while trying to read the change file in a property tree");
        }

        createTmpFiles();
    }

    void OsmChangeHandler::run() {
        // Store the ids of all elements that where deleted, modified or created and the ids of
        // objects where the geometry needs to be updated
        storeIdsOfElementsInChangeFile();
        processElementsInChangeFile();
        getIdsOfWaysToUpdateGeo();
        getIdsOfRelationsToUpdateGeo();

        std::cout
        << osm2rdf::util::currentTimeFormatted()
        << "Fetch references..."
        << std::endl;
        // Get the ids of all referenced objects
//        getReferencedRelations(); Skipped atm because osm2rdf does not calculate the geometry for
//                                  relations that reference other relations
        getReferencesForRelations();
        getReferencesForWays();

        // Create dummy objects for the referenced osm objects
        createDummyElements();

        // Convert osm objects to triples
        try {
            Osm2ttl(_config).convert();
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            throw OsmChangeHandlerException(
                "Exception while trying to convert osm element to ttl");
        }

        // Delete and insert elements from database
        deleteTriplesFromDatabase();
        insertTriplesToDatabase();

        const auto modifiedNodesCount = _modifiedNodes.size() +
                                                   _modifiedNodesWithChangedLocation.size();
        const auto modifiedWaysCount = _modifiedWays.size() +
                                                   _modifiedWaysWithChangedMembers.size();
        const auto modifiedRelCount = _modifiedRelations.size() +
                                                  _modifiedRelsWithChangedMembers.size();
        std::cout << osm2rdf::util::currentTimeFormatted() << "nodes created: "
            << _createdNodes.size() << " modified: " << modifiedNodesCount << " deleted: "
            << _deletedNodes.size() << std::endl;
        std::cout << osm2rdf::util::currentTimeFormatted() << "ways created: "
            << _createdWays.size() << " modified: " << modifiedWaysCount << " deleted: "
            << _deletedWays.size() << std::endl;
        std::cout << osm2rdf::util::currentTimeFormatted() << "relations created: "
            << _createdRelations.size() << " modified: " << modifiedRelCount
            << " deleted: " << _deletedRelations.size() << std::endl;
        std::cout << osm2rdf::util::currentTimeFormatted() << "updated geometries for "
            << _waysToUpdateGeometry.size() << " ways and " << _relationsToUpdateGeometry.size()
            << " relations" << std::endl;
    }

    void
    OsmChangeHandler::checkNodesForLocationChange(std::set<id_t> &nodeIds,
                                                  const std::vector<osmium::Location> &nodeLocs) {
        std::vector<Node> nodeLocsFromEndpoint;
        doInBatches(
            nodeIds,
            MAX_VALUES_PER_QUERY,
            [this, &nodeLocsFromEndpoint](std::set<id_t> const& batch) mutable {
                std::vector<Node> nodes = _odf.fetchNodes(batch);
                nodeLocsFromEndpoint.insert(nodeLocsFromEndpoint.end(),
                    nodes.begin(), nodes.end());
            });

        // Check if the number of nodes from the endpoint is equal to the number of modified nodes
        // in the change file. If this happens we do not know which nodes are missing, so we update
        // the references for all of them.
        if (nodeLocsFromEndpoint.size() != nodeLocs.size() ||
            nodeLocsFromEndpoint.size() != nodeIds.size()) {
            std::cout << osm2rdf::util::currentTimeFormatted() ;
            std::cout << "WARNING: Some modified nodes do not exist in the endpoint. "
                         "This indicates an inconsistency in the database." << std::endl;
            std::ranges::copy(nodeIds,
                              std::inserter(_modifiedNodesWithChangedLocation,
                                                  _modifiedNodesWithChangedLocation.end()));
            return;
        }

        size_t index = 0;
        for (auto nodeId : nodeIds) {
            if (nodeLocs[index] != nodeLocsFromEndpoint[index].getLocation()) {
                _modifiedNodesWithChangedLocation.insert(nodeId);
            } else {
                _modifiedNodes.insert(nodeId);
            }

            index += 1;
        }
    }

    void OsmChangeHandler::checkWaysForMemberChange(
        const std::vector<std::pair<id_t, member_ids_t>>& waysWithMembers) {
        for (const auto &[wayId, nodeRefs] : waysWithMembers) {
            // We have to check if a node reference of the way has its location changed and if so,
            // the ways geometry has to be updated nevertheless
            bool hasModifiedNode = false;
            for (const auto &nodeId : nodeRefs) {
                if (_modifiedNodesWithChangedLocation.contains(nodeId)) {
                    _modifiedWaysWithChangedMembers.insert(wayId);
                    hasModifiedNode = true;
                    break;
                }
            }

            if (hasModifiedNode) {
                continue;
            }

            for (const auto &[wayId, nodeRefsEndpoint]: _odf.fetchWaysMembersSorted({wayId})) {
                if (OsmObjectHelper::areWayMemberEqual(nodeRefs, nodeRefsEndpoint)) {
                    _modifiedWays.insert(wayId);
                } else {
                    _modifiedWaysWithChangedMembers.insert(wayId);
                }
            }
        }
    }
    
    void OsmChangeHandler::checkRelsForMemberChange(
        const std::vector<std::pair<id_t, rel_members_t>>& relsWithMembers) {

        for (const auto &[relId, relMembers] : relsWithMembers) {
            bool hasModifiedMember = false;
            for (const auto &member : relMembers) {
                if (member.osmTag == cnst::XML_TAG_NODE) {
                    if (_modifiedNodesWithChangedLocation.contains(member.id)) {
                        _modifiedRelsWithChangedMembers.insert(relId);
                        hasModifiedMember = true;
                        break;
                    }
                } else if (member.osmTag == cnst::XML_TAG_WAY) {
                    if (_modifiedWaysWithChangedMembers.contains(member.id)) {
                        _modifiedRelsWithChangedMembers.insert(relId);
                        hasModifiedMember = true;
                        break;
                    }
                } else if (member.osmTag == cnst::XML_TAG_REL) {
                    // At the moment all relations that have a relation as member are handled
                    // as if their geometry has changed. This is not ideal, but to be sure that the
                    // geometry hasn't changed we would have to check for all members that are relations
                    // if their geometry has changed, but this is not known at this point.
                    _modifiedRelsWithChangedMembers.insert(relId);
                    hasModifiedMember = true;
                    break;
                }
            }

            if (hasModifiedMember) {
                continue;
            }

            for (const auto &[relId, memberEndpoint]: _odf.fetchRelsMembersSorted({relId})) {
                if (OsmObjectHelper::areRelMemberEqual(relMembers, memberEndpoint)) {
                    _modifiedRelations.insert(relId);
                } else {
                    _modifiedRelsWithChangedMembers.insert(relId);
                }
            }
        }
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

    void OsmChangeHandler::storeIdsOfElementsInChangeFile() {
        std::set<id_t> modifiedNodes;
        std::vector<osmium::Location> modifiedNodeLocations;

        std::vector<std::pair<id_t, member_ids_t>> modifiedWaysWithMembers;
        std::vector<std::pair<id_t, rel_members_t>> modifiedRelsWithMembers;

        for (const auto &[changesetTag, changesetElement]: _osmChangeElement) {
            if (changesetTag == cnst::XML_TAG_ATTR) { continue; }

            for (const auto &[elementTag, element]: changesetElement) {
                auto id = getIdFor(element);

                if (changesetTag == cnst::XML_TAG_MODIFY) {
                    if (elementTag == cnst::XML_TAG_NODE) {
                        modifiedNodes.emplace(id);
                        modifiedNodeLocations.emplace_back(getLocationFor(element));
                    } else if (elementTag == cnst::XML_TAG_WAY) {
                        modifiedWaysWithMembers.emplace_back(id, getMemberForWay(element));
                    } else if (elementTag == cnst::XML_TAG_REL) {
                        modifiedRelsWithMembers.emplace_back(id, getMemberForRel(element));

                        if (OsmObjectHelper::isMultipolygon(element)) {
                            _modifiedAreas.insert(id);
                        }
                    }
                } else if (changesetTag == cnst::XML_TAG_CREATE) {
                    if (elementTag == cnst::XML_TAG_NODE) {
                        _createdNodes.insert(id);
                    } else if (elementTag == cnst::XML_TAG_WAY) {
                        _createdWays.insert(id);
                    } else if (elementTag == cnst::XML_TAG_REL) {
                        _createdRelations.insert(id);
                    }
                } else if (changesetTag == cnst::XML_TAG_DELETE) {
                    if (elementTag == cnst::XML_TAG_NODE) {
                        _deletedNodes.insert(id);
                    } else if (elementTag == cnst::XML_TAG_WAY) {
                        _deletedWays.insert(id);
                    } else if (elementTag == cnst::XML_TAG_REL) {
                        _deletedRelations.insert(id);
                    }
                }
            }
        }

        // Check if the location of the modified nodes has changed and store them in the responding
        // sets (_modifiedNodes or _modifiedNodesWithChangedLocation)
        checkNodesForLocationChange(modifiedNodes, modifiedNodeLocations);

        // Check if the members of modified ways and rels have changed and store them in the
        // responding sets (_modifiedWays/Rels or _modifiedWays/RelsWithChangedMembers)
        checkWaysForMemberChange(modifiedWaysWithMembers);
        // std::cout << (_modifiedWaysWithChangedMembers.size() * 100 / (_modifiedWaysWithChangedMembers.size() + _modifiedWays.size())) << "% of ways have a changed member list" << std::endl;
        checkRelsForMemberChange(modifiedRelsWithMembers);
        // std::cout << (_modifiedRelsWithChangedMembers.size() * 100 / (_modifiedRelsWithChangedMembers.size() + _modifiedRelations.size())) << "% of relations have a changed member list" << std::endl;

        if (_createdNodes.empty() && _modifiedNodes.empty() &&
            _modifiedNodesWithChangedLocation.empty() && _deletedNodes.empty() &&
            _createdWays.empty() && _modifiedWays.empty() && _modifiedWaysWithChangedMembers.empty() &&
            _deletedWays.empty() && _createdRelations.empty() && _modifiedRelations.empty() &&
            _modifiedRelsWithChangedMembers.empty() && _deletedRelations.empty()) {
            throw OsmChangeHandlerException("Change file is empty.");
        }
    }

    void OsmChangeHandler::processElementsInChangeFile() {
        for (auto &[changesetTag, changesetElement]: _osmChangeElement) {
            if (changesetTag == cnst::XML_TAG_MODIFY || changesetTag == cnst::XML_TAG_CREATE) {
                for (auto &[elementTag, element]: changesetElement) {
                    if (elementTag == cnst::XML_TAG_WAY || elementTag == cnst::XML_TAG_REL) {
                        storeIdsOfReferencedElements(element);
                        util::XmlReader::sanitizeXmlTags(element);
                    }

                    addToTmpFile(util::XmlReader::readTree(element, elementTag), elementTag);
                }
            }
        }

        // We do not need to keep the tree in memory after we are done here
        _osmChangeElement.clear();
    }

    void OsmChangeHandler::getIdsOfWaysToUpdateGeo() {
        if (!_modifiedNodesWithChangedLocation.empty()) {
            doInBatches(
                _modifiedNodesWithChangedLocation,
                MAX_VALUES_PER_QUERY,
                [this](const std::set<id_t> &batch) {
                    for (const auto &wayId: _odf.fetchWaysReferencingNodes(batch)) {
                        if (!wayInChangeFile(wayId)) {
                            _waysToUpdateGeometry.insert(wayId);
                        }
                    }
                });
        }
    }

    void OsmChangeHandler::getIdsOfRelationsToUpdateGeo() {
        // Get ids of relations that reference a modified node
        if (!_modifiedNodesWithChangedLocation.empty()) {
            doInBatches(
                _modifiedNodesWithChangedLocation,
                MAX_VALUES_PER_QUERY,
                [this](const std::set<id_t>& batch) {
                    for (const auto &relId: _odf.fetchRelationsReferencingNodes(batch)) {
                        if (!relationInChangeFile(relId)) {
                            _relationsToUpdateGeometry.insert(relId);
                        }
                    }
                });
        }

        // Get ids of relations that reference a way with changed geometry
        std::set<id_t> updatedWays;
        updatedWays.insert(_modifiedWaysWithChangedMembers.begin(),
                            _modifiedWaysWithChangedMembers.end());
        updatedWays.insert(_waysToUpdateGeometry.begin(), _waysToUpdateGeometry.end());
        if (!updatedWays.empty()) {
            doInBatches(
                updatedWays,
                MAX_VALUES_PER_QUERY,
                [this](const std::set<id_t>& batch) {
                    for (const auto &relId: _odf.fetchRelationsReferencingWays(batch)) {
                        if (!relationInChangeFile(relId)) {
                            _relationsToUpdateGeometry.insert(relId);
                        }
                    }
                });
        }

        // Get ids of relations that reference a modified relation. Skip this because
        // osm2rdf does not calculate geometries for relations that reference other relations
//        if (!_modifiedAreas.empty()) {
//            doInBatches(
//                    _modifiedAreas,
//            MAX_TRIPLE_COUNT_PER_QUERY,
//            [this](const std::set<id_t>& batch) {
//                auto relationIds = _odf.fetchRelationsReferencingRelations(batch);
//                for (const auto &relId: relationIds) {
//                    if (!_modifiedAreas.contains(relId)) {
//                        _relationsToUpdateGeometry.insert(relId);
//                    }
//                }
//            });
//        }
    }

    void OsmChangeHandler::getReferencedRelations() {
        if (!_relationsToUpdateGeometry.empty()) {
            doInBatches(
                    _relationsToUpdateGeometry,
                MAX_VALUES_PER_QUERY,
                [this](const std::set<id_t>& batch) {
                    auto relationIds = _odf.fetchRelationsReferencingRelations(batch);
                    for (const auto &relId: relationIds) {
                        if (!_relationsToUpdateGeometry.contains(relId) &&
                            !_createdRelations.contains(relId) &&
                            !_modifiedAreas.contains(relId)) {
                            _referencedRelations.insert(relId);
                        }
                    }
                });
        }
    }

    void OsmChangeHandler::getReferencesForRelations() {
        std::set<id_t> relations;
        relations.insert(_referencedRelations.begin(), _referencedRelations.end());
        relations.insert(_relationsToUpdateGeometry.begin(), _relationsToUpdateGeometry.end());
        if (!relations.empty()) {
            doInBatches(
                    relations,
                    MAX_VALUES_PER_QUERY,
                    [this](const std::set<id_t>& batch) {
                    auto [nodeIds, wayIds] = _odf.fetchRelationMembers(batch);
                    for (const auto &wayId: wayIds) {
                        if (!_waysToUpdateGeometry.contains(wayId) &&
                            !_createdWays.contains(wayId) &&
                            !_modifiedWaysWithChangedMembers.contains(wayId)) {
                            _referencedWays.insert(wayId);
                        }
                    }
                    for (const auto &nodeId: nodeIds) {
                        if (!nodeInChangeFile(nodeId)) {
                            _referencedNodes.insert(nodeId);
                        }
                    }
                });
        }
    }

    void OsmChangeHandler::getReferencesForWays() {
        std::set<id_t> waysToFetchNodesFor;
        waysToFetchNodesFor.insert(_referencedWays.begin(), _referencedWays.end());
        waysToFetchNodesFor.insert(_waysToUpdateGeometry.begin(), _waysToUpdateGeometry.end());
        if (!waysToFetchNodesFor.empty()) {
            doInBatches(
            waysToFetchNodesFor,
            MAX_VALUES_PER_QUERY,
            [this](const std::set<id_t>& batch) {
                for (const auto &nodeId: _odf.fetchWaysMembers(batch)) {
                    if (!nodeInChangeFile(nodeId)) {
                        _referencedNodes.insert(nodeId);
                    }
                }
            });
        }
    }

    void OsmChangeHandler::createDummyElements() {
        const std::size_t count = _referencedNodes.size() + _referencedWays.size()
            + _waysToUpdateGeometry.size() + _referencedRelations.size()
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
        doInBatches(
            _referencedNodes,
            MAX_VALUES_PER_QUERY,
            [this, &counter, progress](std::set<id_t> const& batch) mutable {
                progress.update(counter += batch.size());
                for (auto const& node: _odf.fetchNodes(batch)) {
                    addToTmpFile(node.getXml(), cnst::XML_TAG_NODE);
                }
            });

        finalizeTmpFile(cnst::PATH_TO_NODE_FILE);
    }



    void OsmChangeHandler::createDummyWays(osm2rdf::util::ProgressBar &progress, size_t &counter) {
        std::set<id_t> wayIds;
        wayIds.insert(_referencedWays.begin(), _referencedWays.end());
        wayIds.insert(_waysToUpdateGeometry.begin(), _waysToUpdateGeometry.end());

        doInBatches(
            wayIds,
            MAX_VALUES_PER_QUERY,
            [this, progress, &counter](std::set<id_t> const& batch) mutable {
                progress.update(counter += batch.size());
                for (auto& way: _odf.fetchWays(batch)) {
                    // The ways for which the geometry does not need to be updated are already in
                    // the tmp file
                    if (_modifiedWays.contains(way.getId())) {
                        continue;
                    }

                    if (_waysToUpdateGeometry.contains(way.getId())) {
                        _odf.fetchWayInfos(way);
                    }

                    addToTmpFile(way.getXml(), cnst::XML_TAG_WAY);
                }
            });

        finalizeTmpFile(cnst::PATH_TO_WAY_FILE);
    }

    void OsmChangeHandler::createDummyRelations(osm2rdf::util::ProgressBar &progress, size_t &counter) {
        std::set<id_t> relations;
        relations.insert(_referencedRelations.begin(), _referencedRelations.end());
        relations.insert(_relationsToUpdateGeometry.begin(), _relationsToUpdateGeometry.end());

        doInBatches(
            relations,
            MAX_VALUES_PER_QUERY,
            [this, &counter, progress](std::set<id_t> const& batch) mutable {
                progress.update(counter += batch.size());
                for (auto& rel: _odf.fetchRelations(batch)) {
                    if (_relationsToUpdateGeometry.contains(rel.getId())) {
                        _odf.fetchRelationInfos(rel);
                    }

                    addToTmpFile(rel.getXml(), cnst::XML_TAG_REL);
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
        std::set<id_t> nodesToDelete;
        nodesToDelete.insert(_deletedNodes.begin(), _deletedNodes.end());
        nodesToDelete.insert(_modifiedNodes.begin(), _modifiedNodes.end());
        nodesToDelete.insert(_modifiedNodesWithChangedLocation.begin(),
                             _modifiedNodesWithChangedLocation.end());
        nodesToDelete.insert(_createdNodes.begin(), _createdNodes.end());

        doInBatches(
            nodesToDelete,
            MAX_VALUES_PER_QUERY,
            [this, progress, &counter](std::set<id_t> const &batch) mutable {
                runUpdateQuery(_queryWriter.writeDeleteQuery(batch, cnst::NAMESPACE_OSM_NODE),
                               cnst::PREFIXES_FOR_NODE_DELETE_QUERY);
                progress.update(counter += batch.size());
            });
    }

    void OsmChangeHandler::deleteWaysFromDatabase(osm2rdf::util::ProgressBar &progress,
                                                  size_t &counter) {
        std::set<id_t> waysToDelete;
        waysToDelete.insert(_deletedWays.begin(), _deletedWays.end());
        waysToDelete.insert(_modifiedWaysWithChangedMembers.begin(), _modifiedWaysWithChangedMembers.end());
        waysToDelete.insert(_createdWays.begin(), _createdWays.end());

        doInBatches(
            waysToDelete,
            MAX_VALUES_PER_QUERY,
            [this, &counter, progress](std::set<id_t> const &batch) mutable {
                runUpdateQuery(_queryWriter.writeDeleteQuery(batch, cnst::NAMESPACE_OSM_WAY),
                               cnst::PREFIXES_FOR_WAY_DELETE_QUERY);
                progress.update(counter += batch.size());
            });
    }

    void OsmChangeHandler::deleteWaysMetaDataAndTags(osm2rdf::util::ProgressBar &progress,
                                                    size_t &counter) {
        doInBatches(
            _modifiedWays,
            MAX_VALUES_PER_QUERY,
            [this, &counter, progress](std::set<id_t> const &batch) mutable {
                runUpdateQuery(_queryWriter.writeDeleteQueryForMetaAndTags(batch, cnst::NAMESPACE_OSM_WAY),
                               cnst::PREFIXES_FOR_WAY_DELETE_META_AND_TAGS_QUERY);
                progress.update(counter += batch.size());
            });
    }

    void OsmChangeHandler::deleteRelationsMetaDataAndTags(osm2rdf::util::ProgressBar &progress,
                                                          size_t &counter) {
        doInBatches(
            _modifiedRelations,
            MAX_VALUES_PER_QUERY,
            [this, &counter, progress](std::set<id_t> const &batch) mutable {
                runUpdateQuery(_queryWriter.writeDeleteQueryForMetaAndTags(batch, cnst::NAMESPACE_OSM_REL),
                               cnst::PREFIXES_FOR_RELATION_DELETE_META_AND_TAGS_QUERY);
                progress.update(counter += batch.size());
            });
    }

    void OsmChangeHandler::deleteWaysGeometry(osm2rdf::util::ProgressBar &progress,
                                                size_t &counter) {
        doInBatches(
            _waysToUpdateGeometry,
            MAX_VALUES_PER_QUERY,
            [this, &counter, progress](std::set<id_t> const &batch) mutable {
                runUpdateQuery(_queryWriter.writeDeleteQueryForGeometry(batch, cnst::NAMESPACE_OSM_WAY),
                               cnst::PREFIXES_FOR_WAY_DELETE_GEOMETRY_QUERY);
                progress.update(counter += batch.size());
            });
    }

    void OsmChangeHandler::deleteRelationsFromDatabase(osm2rdf::util::ProgressBar &progress,
                                                       size_t &counter) {
        std::set<id_t> relationsToDelete;
        relationsToDelete.insert(_deletedRelations.begin(), _deletedRelations.end());
        relationsToDelete.insert(_modifiedRelsWithChangedMembers.begin(), _modifiedRelsWithChangedMembers.end());
        relationsToDelete.insert(_createdRelations.begin(), _createdRelations.end());

        doInBatches(
            relationsToDelete,
            MAX_VALUES_PER_QUERY,
            [this, &counter, progress](std::set<id_t> const &batch) mutable {
                runUpdateQuery(_queryWriter.writeDeleteQuery(batch, cnst::NAMESPACE_OSM_REL),
                               cnst::PREFIXES_FOR_RELATION_DELETE_QUERY);
                progress.update(counter += batch.size());
            });
    }

    void OsmChangeHandler::deleteRelationsGeometry(osm2rdf::util::ProgressBar &progress,
                                                   size_t &counter) {
        doInBatches(
            _relationsToUpdateGeometry,
            MAX_VALUES_PER_QUERY,
            [this, &counter, progress](std::set<id_t> const &batch) mutable {
                runUpdateQuery(_queryWriter.writeDeleteQueryForGeometry(batch, cnst::NAMESPACE_OSM_REL),
                               cnst::PREFIXES_FOR_RELATION_DELETE_GEOMETRY_QUERY);
                progress.update(counter += batch.size());
            });
    }

    void OsmChangeHandler::deleteTriplesFromDatabase() {
        const std::size_t count = _deletedNodes.size() + _modifiedNodes.size()
            + _modifiedNodesWithChangedLocation.size()
            + _deletedWays.size() + _modifiedWays.size() + _modifiedWaysWithChangedMembers.size()
            + _waysToUpdateGeometry.size()
            + _deletedRelations.size() + _modifiedRelations.size()
            + _modifiedRelsWithChangedMembers.size()
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

            if (tripleBatch.size() == MAX_VALUES_PER_QUERY || i == triples.size() - 1) {
                runUpdateQuery(_queryWriter.writeInsertQuery(tripleBatch), cnst::DEFAULT_PREFIXES);
                tripleBatch.clear();

                if (i == triples.size() - 1) {
                    insertProgress.done();
                } else {
                    insertProgress.update(counter += MAX_VALUES_PER_QUERY);
                }
            }
        }
    }

    std::vector<Triple> OsmChangeHandler::filterRelevantTriples() {
        std::set<id_t> nodesToInsert;
        nodesToInsert.insert(_createdNodes.begin(), _createdNodes.end());
        nodesToInsert.insert(_modifiedNodes.begin(), _modifiedNodes.end());
        nodesToInsert.insert(_modifiedNodesWithChangedLocation.begin(),
                             _modifiedNodesWithChangedLocation.end());

        std::set<id_t> waysToInsert;
        waysToInsert.insert(_createdWays.begin(), _createdWays.end());
        waysToInsert.insert(_modifiedWaysWithChangedMembers.begin(), _modifiedWaysWithChangedMembers.end());

        std::set<id_t> relationsToInsert;
        relationsToInsert.insert(_createdRelations.begin(), _createdRelations.end());
        relationsToInsert.insert(_modifiedRelsWithChangedMembers.begin(), _modifiedRelsWithChangedMembers.end());

        // Triples that should be inserted into the database
        std::vector<Triple> relevantTriples;
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
                if (_modifiedWays.contains(wayId)) {
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
                if (_modifiedRelations.contains(relId)) {
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

    void
    OsmChangeHandler::storeIdsOfReferencedElements(const boost::property_tree::ptree& relElement) {
        for (const auto &[memberTag, element] : relElement.get_child("")) {
            if (memberTag != cnst::XML_TAG_MEMBER && memberTag != cnst::XML_TAG_NODE_REF) {
                continue;
            }

            std::string type;
            if (memberTag == cnst::XML_TAG_NODE_REF) {
                type = cnst::XML_TAG_NODE;
            } else {
                type = util::XmlReader::readAttribute<std::string>(cnst::XML_PATH_ATTR_TYPE, element);
            }

            auto refIdAsString = util::XmlReader::readAttribute<std::string>(cnst::XML_PATH_ATTR_NODE_REF,
                                                                             element);

            id_t id;
            try {
                id = std::stoll(refIdAsString);
            } catch(std::exception &e) {
                std::cerr << e.what() << std::endl;
                const std::string msg = "Exception while trying to convert id string: "
                                        + refIdAsString + " to long";
                throw OsmChangeHandlerException(msg.c_str());
            }

            if (type == cnst::XML_TAG_NODE) {
                if (!nodeInChangeFile(id)) {
                    _referencedNodes.insert(id);
                }
            } else if (type == cnst::XML_TAG_WAY) {
                if (!wayInChangeFile(id)) {
                    _referencedWays.insert(id);
                }
            } else if (type == cnst::XML_TAG_REL) {
                if (!relationInChangeFile(id)) {
                    _referencedRelations.insert(id);
                }
            } else {
                const std::string msg = "Cant handle member type: " + type + " for element: "
                                        + util::XmlReader::readTree(relElement);
                throw OsmChangeHandlerException(msg.c_str());
            }
        }
    }

    id_t OsmChangeHandler::getIdFor(const boost::property_tree::ptree &element) {
        try {
            return util::XmlReader::readAttribute<id_t>(config::constants::XML_PATH_ATTR_ID,element);
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            const std::string msg = "Could not extract identifier from element: "
                                    + util::XmlReader::readTree(element);
            throw OsmDataFetcherException(msg.c_str());
        }
    }

    osmium::Location OsmChangeHandler::getLocationFor(const boost::property_tree::ptree &element) {
        double lat;
        double lon;
        try {
            lat = util::XmlReader::readAttribute<double>(
                config::constants::XML_PATH_ATTR_LAT, element);
            lon = util::XmlReader::readAttribute<double>(
                config::constants::XML_PATH_ATTR_LON, element);
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            const std::string msg = "Could not extract lat or lon from element: "
                                    + util::XmlReader::readTree(element);
            throw OsmDataFetcherException(msg.c_str());
        }

        return {lon, lat};
    }

    member_ids_t OsmChangeHandler::getMemberForWay(const boost::property_tree::ptree &element) {
        member_ids_t wayMember;

        for (const auto &child : element) {
            if (child.first == cnst::XML_TAG_NODE_REF) {
                wayMember.emplace_back(
                    util::XmlReader::readAttribute<id_t>(cnst::XML_PATH_ATTR_NODE_REF,
                                                         child.second));
            }
        }

        return wayMember;
    }

    rel_members_t OsmChangeHandler::getMemberForRel(const boost::property_tree::ptree &element) {
        rel_members_t relMember;

        for (const auto &child : element) {
            if (child.first == cnst::XML_TAG_MEMBER) {
                auto type = util::XmlReader::readAttribute<std::string>(cnst::XML_PATH_ATTR_TYPE,
                                                                        child.second);
                auto refId = util::XmlReader::readAttribute<id_t>(cnst::XML_PATH_ATTR_NODE_REF,
                                                                  child.second);
                auto role = util::XmlReader::readAttribute<std::string>(cnst::XML_PATH_ATTR_ROLE,
                                                                        child.second);
                // If a role for a relation is empty, osm2rdf uses the placeholder "member"
                if (role.empty()) {
                    role = "member";
                }

                relMember.emplace_back(refId, type, role);
            }
        }

        return relMember;
    }

} // namespace olu::osm
