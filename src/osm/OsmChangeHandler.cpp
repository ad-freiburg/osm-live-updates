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
#include "osm/OsmChangeHandlerException.h"
#include "osm/OsmDataFetcher.h"
#include "osm/Osm2ttl.h"
#include "util/XmlReader.h"
#include "config/Constants.h"
#include "sparql/SparqlWrapper.h"
#include "sparql/QueryWriter.h"

#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include <string>

namespace olu::osm {
    OsmChangeHandler::OsmChangeHandler(std::string &sparqlEndpointUri) {
        _osm2ttl = Osm2ttl();
//        _sparql.setEndpointUri(sparqlEndpointUri);
    }

    void OsmChangeHandler::handleChange(const std::string &pathToOsmChangeFile) {
        std::ifstream ifs (pathToOsmChangeFile);
        std::string fileContent( (std::istreambuf_iterator<char>(ifs) ),
                                 (std::istreambuf_iterator<char>()) );

        boost::property_tree::ptree osmChange;
        olu::util::XmlReader::populatePTreeFromString(fileContent, osmChange);

        for (const auto &child : osmChange.get_child(config::constants::OSM_CHANGE_TAG)) {
            boost::property_tree::ptree childTree = child.second;
            if (child.first == config::constants::MODIFY_TAG) {
                for (const auto &element : childTree) {
//                    handleModify(element.first, element.second);
                }
            } else if (child.first == config::constants::CREATE_TAG) {
                for (const auto &element : childTree) {
                    boost::property_tree::ptree elementTree = element.second;
                    handleInsert(element.first, element.second);
                }
            } else if (child.first == config::constants::DELETE_TAG) {
                for (const auto &element : childTree) {
//                    handleDelete(element.first, element.second);
                }
            }
        }
    }

    void olu::osm::OsmChangeHandler::handleInsert(const std::string& elementTag,
                                                  const boost::property_tree::ptree &element) {
        // Elements without a tag are not converted to ttl
        if (olu::util::XmlReader::readTagOfChildren("", element).empty()) {
            return;
        }

        auto osmElements = getOsmElementsForInsert(elementTag, element);
        for (std::string i: osmElements)
            std::cout << i << ' ';
        auto ttl = _osm2ttl.convert(osmElements);
        auto query = sparql::QueryWriter::writeInsertQuery(ttl);
        _sparql.setQuery(query);
        _sparql.setMethod(util::POST);
        _sparql.runQuery();
    }

    void OsmChangeHandler::handleDelete(const std::string& elementTag,
                                        const boost::property_tree::ptree &element) {
        auto subject = olu::sparql::QueryWriter::getSubjectFor(elementTag, element);
        auto query = sparql::QueryWriter::writeDeleteQuery(subject);
        _sparql.setQuery(query);
        _sparql.setMethod(util::POST);
        _sparql.runQuery();
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
            auto nodeReferenceElements = olu::OsmDataFetcher::fetchNodeReferencesForWay(element);
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

} // namespace olu::osm
