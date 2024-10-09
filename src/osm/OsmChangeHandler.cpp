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
#include "util/WktHelper.h"

#include <boost/property_tree/ptree.hpp>
#include <string>
#include <iostream>
#include <osm2rdf/util/ProgressBar.h>
#include <set>

/// The maximum number of triples that can be in a query to the QLever endpoint.
const static inline int MAX_TRIPLE_COUNT_PER_QUERY = 64;

namespace olu::osm {

    void OsmChangeHandler::handleChange(const std::string &pathToOsmChangeFile,
                                        const bool &deleteChangeFile,
                                        const bool &showProgress) {
        // Read the file in a property tree
        boost::property_tree::ptree osmChangeElement;
        try {
            if (pathToOsmChangeFile.ends_with(config::constants::GZIP_EXTENSION)) {
                auto decompressed = olu::util::Decompressor::readGzip(
                        pathToOsmChangeFile);
                olu::util::XmlReader::populatePTreeFromString(
                        decompressed,
                        osmChangeElement);
            } else {
                olu::util::XmlReader::populatePTreeFromFile(pathToOsmChangeFile,
                                                            osmChangeElement);
            }
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            throw OsmChangeHandlerException(
                    "Exception while trying to read the change file in a property tree");
        }

        // Prepare a progress bar for the progress indicator
        auto maxCount = countElements(osmChangeElement);
        osm2rdf::util::ProgressBar progressBar(maxCount, showProgress);
        size_t entryCount = 0;

        // Loop over all changesets in the change file ('modify', 'delete' or 'create')
        auto changesets = osmChangeElement.get_child(config::constants::OSM_CHANGE_TAG);
        for (const auto &changeset : changesets) {
            if (changeset.first == config::constants::MODIFY_TAG) {
                // Loop over each element ('node', 'way' or 'relation') to be modified
                for (const auto &element : changeset.second) {
                    _stats.add(ChangesetStat { element.first, MODIFY });

                    try {
                        handleModify(element.first, element.second);
                    } catch (std::exception &e) {
                        std::cerr << "Could not handle modification of element with tag "
                                  << element.first
                                  << " and id "
                                  << util::XmlReader::readAttribute(
                                          olu::config::constants::ID_ATTRIBUTE, element.second)
                                  << std::endl;

                        throw OsmChangeHandlerException(
                                "Exception while trying to modify element");
                    }

                    progressBar.update(entryCount++);
                }
            } else if (changeset.first == config::constants::CREATE_TAG) {
                // Loop over each element ('node', 'way' or 'relation') to be created
                for (const auto &element : changeset.second) {
                    _stats.add(ChangesetStat { element.first, INSERT });

                    try {
                        handleInsert(element.first, element.second);
                    } catch (std::exception &e) {
                        std::cerr << "Could not handle insertion of element with tag "
                                  << element.first
                                  << " and id "
                                  << util::XmlReader::readAttribute(
                                          olu::config::constants::ID_ATTRIBUTE, element.second)
                                  << std::endl;

                        throw OsmChangeHandlerException(
                                "Exception while trying to insert element");
                    }

                    // Fix: The cache needs to be cleared after each update, otherwise the
                    // sparql endpoint can send outdated data
                    _sparql.clearCache();
                    progressBar.update(entryCount++);
                }
            } else if (changeset.first == config::constants::DELETE_TAG) {
                // Loop over each element ('node', 'way' or 'relation') to be deleted
                for (const auto &element : changeset.second) {
                    _stats.add(ChangesetStat { element.first, DELETE });

                    try {
                        handleDelete(element.first, element.second);
                    } catch (std::exception &e) {
                        std::cerr << "Could not handle deletion of element with tag "
                                  << element.first
                                  << " and id "
                                  << util::XmlReader::readAttribute(
                                          olu::config::constants::ID_ATTRIBUTE, element.second)
                                  << std::endl;

                        throw OsmChangeHandlerException(
                                "Exception while trying to delete element");
                    }

                    // Fix: The cache needs to be cleared after each update, otherwise the
                    // sparql endpoint can send outdated data
                    _sparql.clearCache();
                    progressBar.update(entryCount++);
                }
            }
        }

        if (deleteChangeFile) {
            try {
                std::filesystem::remove(pathToOsmChangeFile);
            } catch (std::exception &e) {
                std::cerr << e.what() << std::endl;
                std::string message = "Exception while trying to delete the change file at path: "
                        + pathToOsmChangeFile;
                throw OsmChangeHandlerException(message.c_str());
            }
        }

        progressBar.done();

        _stats.printResults();
    }

    void olu::osm::OsmChangeHandler::handleInsert(const std::string& elementTag,
                                                  const boost::property_tree::ptree &element) {
        std::vector<std::string> osmElements;
        try {
            osmElements = getOsmElementsForInsert(elementTag, element);
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            throw OsmChangeHandlerException(
                    "Exception while trying to get elements for insertion");
        }

        std::vector<std::string> ttl;
        try {
            ttl = _osm2ttl.convert(osmElements);
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            throw OsmChangeHandlerException(
                    "Exception while trying to convert osm element to ttl");
        }

        try {
            createAndRunInsertQuery(ttl);
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            throw OsmChangeHandlerException(
                    "Exception while trying to create and run insert query");
        }
    }

    void OsmChangeHandler::handleDelete(const std::string& elementTag,
                                        const boost::property_tree::ptree &element) {
        std::string query;
        if (elementTag == config::constants::NODE_TAG) {
            auto id = olu::osm::OsmDataFetcher::getIdFor(element);
            query = sparql::QueryWriter::writeNodeDeleteQuery(id);
            _sparql.setPrefixes(config::constants::PREFIXES_FOR_NODE_DELETE_QUERY);
        } else if (elementTag == config::constants::WAY_TAG) {
            auto id = olu::osm::OsmDataFetcher::getIdFor(element);
            query = sparql::QueryWriter::writeWayDeleteQuery(id);
            _sparql.setPrefixes(config::constants::PREFIXES_FOR_WAY_DELETE_QUERY);
        } else if (elementTag == config::constants::RELATION_TAG) {
            auto id = olu::osm::OsmDataFetcher::getIdFor(element);
            query = sparql::QueryWriter::writeRelationDeleteQuery(id);
            _sparql.setPrefixes(config::constants::PREFIXES_FOR_RELATION_DELETE_QUERY);

            handleRelationMemberDeletion(id);
        } else {
            std::string msg = "Could not determine osm type for element: "
                              + util::XmlReader::readTree(element);
            throw OsmChangeHandlerException(msg.c_str());
        }

        _sparql.setQuery(query);
        _sparql.setMethod(util::POST);

        try {
            _sparql.runQuery();
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            throw OsmChangeHandlerException(
                    "Exception while trying to run sparql query for deletion");
        }
    }

    void
    OsmChangeHandler::handleRelationMemberDeletion(const long long &relationId) {
        auto memberSubjects = _odf.fetchSubjectsOfRelationMembers(relationId);

        std::vector<std::vector<std::string>> memberSubjectsBatches;
        for (auto it = memberSubjects.cbegin(), e = memberSubjects.cend(); it != memberSubjects.cend(); it = e) {
            e = it + std::min<std::size_t>(memberSubjects.cend() - it, MAX_TRIPLE_COUNT_PER_QUERY);
            memberSubjectsBatches.emplace_back(it, e);
        }

        for (auto & batch : memberSubjectsBatches) {
            auto query = sparql::QueryWriter::writeDeleteQuery(batch);
            _sparql.setPrefixes(config::constants::PREFIXES_FOR_RELATION_DELETE_QUERY);
            _sparql.setQuery(query);
            _sparql.setMethod(util::POST);
            try {
                _sparql.runQuery();
            } catch (std::exception &e) {
                std::cerr << e.what() << std::endl;
                throw OsmChangeHandlerException(
                        "Exception while trying to run sparql query for deletion");
            }
        }
    }

    void OsmChangeHandler::handleModify(const std::string& elementTag,
                                        const boost::property_tree::ptree &element) {
        handleDelete(elementTag, element);
        handleInsert(elementTag, element);
    }

    std::vector<std::string>
    OsmChangeHandler::getOsmElementsForInsert(const std::string &elementTag,
                                              const boost::property_tree::ptree &element) {
        std::vector<std::string> osmElements;
        osmElements.push_back(config::constants::OSM_XML_NODE_START);
        if (elementTag == config::constants::WAY_TAG) {
            auto referencedNodeIds = getIdsOfReferencedNodes(element);

            std::vector<std::string> nodeReferenceElements;
            try {
                nodeReferenceElements = createDummyNodes(referencedNodeIds);
            } catch (std::exception &e) {
                std::cerr << e.what() << std::endl;
                throw OsmChangeHandlerException(
                        "Exception while trying to create node references for way");
            }

            osmElements.insert(
                osmElements.end(),
                std::make_move_iterator(nodeReferenceElements.begin()),
                std::make_move_iterator(nodeReferenceElements.end())
            );
        }
        osmElements.push_back(olu::util::XmlReader::readTree(element, {elementTag}, 0));
        osmElements.push_back(config::constants::OSM_XML_NODE_END);
        return osmElements;
    }

    size_t OsmChangeHandler::countElements(const boost::property_tree::ptree &osmChangeElement) {
        size_t count = 0;
        // Loop over all change elements in the change file ('modify', 'delete' or 'create')
        for (const auto &child : osmChangeElement.get_child(
                config::constants::OSM_CHANGE_TAG)) {
            if (child.first == config::constants::MODIFY_TAG ||
                child.first == config::constants::CREATE_TAG ||
                child.first == config::constants::DELETE_TAG) {
                count += child.second.size();
            }
        }
        return count;
    }

    std::vector<std::string>
    OsmChangeHandler::getPrefixesFromConvertedData(std::vector<std::string> ttl) {
        std::vector<std::string> prefixes;
        std::copy_if (
                ttl.begin(),
                ttl.end(),
                std::back_inserter(prefixes),
                [](const std::string& triple){
                    return triple.starts_with("@prefix");
                } );

        // Transform prefixes to correct format
        for (auto & prefix : prefixes) {
            // TODO: Replace with regex
            prefix = "PREFIX " + prefix.substr(8, prefix.length() - 10);
        }

        return prefixes;
    }

    std::vector<std::string>
    OsmChangeHandler::getTriplesFromConvertedData(std::vector<std::string> ttl) {
        std::vector<std::string> triples;
        std::copy_if (
                ttl.begin(),
                ttl.end(),
                std::back_inserter(triples),
                [](const std::string& triple){
                    return !triple.starts_with("@prefix");
                } );
        return triples;
    }

    std::vector<long long>
    OsmChangeHandler::getIdsOfReferencedNodes(const boost::property_tree::ptree &way) {
        std::vector<long long> referencedNodes;
        std::set<long long> visitedNodes;

        for (const auto &child : way.get_child("")) {
            if (child.first != config::constants::NODE_REFERENCE_TAG) {
                continue;
            }

            std::string idAsString;
            try {
                idAsString = util::XmlReader::readAttribute(
                        config::constants::NODE_REFERENCE_ATTRIBUTE,
                        child.second);
            } catch (std::exception &e) {
                std::cerr << e.what() << std::endl;
                std::string msg = "Exception while trying to read id of node reference: "
                        + util::XmlReader::readTree(child.second);
                throw OsmChangeHandlerException(msg.c_str());
            }

            long long id;
            try {
                id = std::stol(idAsString);
            } catch(std::exception &e) {
                std::cerr << e.what() << std::endl;
                std::string msg = "Exception while trying to convert id string: "
                        + idAsString + " to long";
                throw OsmChangeHandlerException(msg.c_str());
            }

            if (!visitedNodes.contains(id)) {
                visitedNodes.insert(id);
            }
        }

        std::vector<long long> nodeIds(visitedNodes.begin(), visitedNodes.end());
        return nodeIds;
    }

    std::vector<std::string>
    OsmChangeHandler::createDummyNodes(const std::vector<long long>& nodeIds) {
        // QLever has a maximum number of triples it can handle in one query, so we have to
        // divide the triples in batches
        std::vector<std::vector<long long>> nodeIdsBatches;
        for (auto it = nodeIds.cbegin(), e = nodeIds.cend(); it != nodeIds.cend(); it = e) {
            e = it + std::min<std::size_t>(nodeIds.cend() - it, MAX_TRIPLE_COUNT_PER_QUERY);
            nodeIdsBatches.emplace_back(it, e);
        }

        std::vector<std::string> pointsAsWkt;
        for (auto & nodeIdBatch : nodeIdsBatches) {
            auto pointsBatchResult = _odf.fetchNodeLocationsAsWkt(nodeIdBatch);
            pointsAsWkt.insert( pointsAsWkt.end(), pointsBatchResult.begin(), pointsBatchResult.end() );
        }

        std::vector<std::string> dummyNodes;
        for (auto it = nodeIds.begin(); it != nodeIds.end(); ++it) {
            long index = std::distance(nodeIds.begin(), it);
            auto dummyNode = olu::osm::WktHelper::createDummyNodeFromPoint(
                    nodeIds.at(index),
                    pointsAsWkt.at(index));
            dummyNodes.emplace_back(dummyNode);
        }

        return dummyNodes;
    }

    void OsmChangeHandler::createAndRunInsertQuery(const std::vector<std::string>& ttl) {
        auto prefixes = getPrefixesFromConvertedData(ttl);
        auto triples = getTriplesFromConvertedData(ttl);

        // QLever has a maximum number of triples it can handle in one query, so we have to
        // divide the triples in batches
        std::vector<std::vector<std::string>> triplesBatches;
        for (auto it = triples.cbegin(), e = triples.cend(); it != triples.cend(); it = e) {
            e = it + std::min<std::size_t>(triples.cend() - it, MAX_TRIPLE_COUNT_PER_QUERY);
            triplesBatches.emplace_back(it, e);
        }

        // Create a sparql query for each batch and send it to the sparql endpoint.
        for (auto & batch : triplesBatches) {
            auto query = sparql::QueryWriter::writeInsertQuery(batch);
            _sparql.setPrefixes(prefixes);
            _sparql.setQuery(query);
            _sparql.setMethod(util::POST);

            try {
                _sparql.runQuery();
            } catch (std::exception &e) {
                std::cerr << e.what() << std::endl;
                throw OsmChangeHandlerException(
                        "Exception while trying to run sparql query for insertion");
            }
        }
    }
} // namespace olu::osm
