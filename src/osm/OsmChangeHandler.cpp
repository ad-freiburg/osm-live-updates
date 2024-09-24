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

#include <boost/property_tree/ptree.hpp>
#include <string>
#include <iostream>
#include <osm2rdf/util/ProgressBar.h>

const static inline int MAX_TRIPLE_COUNT_PER_QUERY = 64;

namespace olu::osm {

    OsmChangeHandler::OsmChangeHandler(config::Config& config)
    : _config(config), _sparql(config), _osm2ttl(), _odf(OsmDataFetcher(config)) { }

    void OsmChangeHandler::handleChange(const std::string &pathToOsmChangeFile,
                                        const bool &deleteChangeFile) {
        boost::property_tree::ptree osmChangeElement;
        if (pathToOsmChangeFile.ends_with(config::constants::GZIP_EXTENSION)) {
            auto decompressed = olu::util::Decompressor::readGzip(pathToOsmChangeFile);
            olu::util::XmlReader::populatePTreeFromString(decompressed, osmChangeElement);
        } else {
            olu::util::XmlReader::populatePTreeFromFile(pathToOsmChangeFile,
                                                        osmChangeElement);
        }

        auto maxCount = countElements(osmChangeElement);
        osm2rdf::util::ProgressBar progressBar(maxCount, true);
        size_t entryCount = 0;
        // Loop over all change elements in the change file ('modify', 'delete' or 'create')
        for (const auto &child : osmChangeElement.get_child(
                config::constants::OSM_CHANGE_TAG)) {

            if (child.first == config::constants::MODIFY_TAG) {
                // Loop over each element ('node', 'way' or 'relation') to be modified
                for (const auto &element : child.second) {
                    handleModify(element.first, element.second);
                    progressBar.update(entryCount++);
                }
            } else if (child.first == config::constants::CREATE_TAG) {
                // Loop over each element ('node', 'way' or 'relation') to be created
                for (const auto &element : child.second) {
                    handleInsert(element.first, element.second);

                    // Fix: The cache needs to be cleared after each update, otherwise the
                    // sparql endpoint can send outdated data
                    _sparql.clearCache();
                    progressBar.update(entryCount++);
                }
            } else if (child.first == config::constants::DELETE_TAG) {
                // Loop over each element ('node', 'way' or 'relation') to be deleted
                for (const auto &element : child.second) {
                    handleDelete(element.first, element.second);

                    // Fix: The cache needs to be cleared after each update, otherwise the
                    // sparql endpoint can send outdated data
                    _sparql.clearCache();
                    progressBar.update(entryCount++);
                }
            }
        }

        if (deleteChangeFile) {
            std::filesystem::remove(pathToOsmChangeFile);
        }

        progressBar.done();
    }



    void olu::osm::OsmChangeHandler::handleInsert(const std::string& elementTag,
                                                  const boost::property_tree::ptree &element) {
        std::string result;
        std::vector<std::string> osmElements;
        try {
            // Convert the osmElements in xml format to rdf turtle format
            osmElements = getOsmElementsForInsert(elementTag, element);
            auto ttl = _osm2ttl.convert(osmElements);
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
                prefix = "PREFIX " + prefix.substr(8, prefix.length() - 10);
            }

            std::vector<std::string> triples;
            std::copy_if (
                    ttl.begin(),
                    ttl.end(),
                    std::back_inserter(triples),
                    [](const std::string& triple){
                        return !triple.starts_with("@prefix");
                    } );

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
                _sparql.runQuery();
            }
        } catch (std::exception &e) {
            std::cerr << "Could not handle insertion of element with tag "
                << elementTag
                << " and id "
                << util::XmlReader::readAttribute(
                        olu::config::constants::ID_ATTRIBUTE, element)
                << std::endl;
            return;
        }
    }

    void OsmChangeHandler::handleDelete(const std::string& elementTag,
                                        const boost::property_tree::ptree &element) {
        auto subject = olu::sparql::QueryWriter::getSubjectFor(elementTag, element);
        auto query = sparql::QueryWriter::writeDeleteQuery(subject);
        _sparql.setPrefixes(config::constants::PREFIXES_FOR_DELETE_QUERY);
        _sparql.setQuery(query);
        _sparql.setMethod(util::POST);
        auto result = _sparql.runQuery();
    }

    void OsmChangeHandler::handleModify(const std::string& elementTag,
                                        const boost::property_tree::ptree &element) {
        handleDelete(elementTag, element);
        handleInsert(elementTag, element);
    }

    std::vector<std::string> OsmChangeHandler::getOsmElementsForInsert(
            const std::string &elementTag,
            const boost::property_tree::ptree &element) {
        std::vector<std::string> osmElements;
        osmElements.push_back(config::constants::OSM_XML_NODE_START);
        if (elementTag == config::constants::WAY_TAG) {
            auto nodeReferenceElements = _odf.fetchNodeReferencesForWay(element);
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
} // namespace olu::osm
