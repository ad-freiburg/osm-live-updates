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
    OsmChangeHandler::OsmChangeHandler(std::string &sparqlEndpointUri):
        _sparql(sparql::SparqlWrapper(sparqlEndpointUri))  {
        _osm2ttl = Osm2ttl();
    }

    void olu::osm::OsmChangeHandler::handleInsert(const boost::property_tree::ptree &element) {
        // Check if the element has at least one tag. If not return without doing anything.
        auto childrenTags = olu::util::XmlReader::readTagOfChildren(
                config::constants::OSM_TAG,
                element);
        if (childrenTags.empty()) {
            return;
        }
        auto elementTag = childrenTags.front();

        std::vector<std::string> osmElements;
        if (elementTag == config::constants::WAY_TAG) {
            osmElements = olu::OsmDataFetcher::fetchNodeReferencesForWay(element);
        }

        osmElements.push_back(olu::util::XmlReader::writeXmlElementToString(element));
        auto ttl = _osm2ttl.convert(osmElements);
        auto query = sparql::QueryWriter::writeInsertQuery(ttl);
        _sparql.setQuery(query);
        _sparql.setMethod(util::POST);
        _sparql.runQuery();
    }

    void OsmChangeHandler::handleDelete(const boost::property_tree::ptree &element) {
        auto subject = getSubjectFor(element);
        auto query = sparql::QueryWriter::writeDeleteQuery(subject);
        _sparql.setQuery(query);
        _sparql.setMethod(util::POST);
        _sparql.runQuery();
    }

    void OsmChangeHandler::handleModify(const boost::property_tree::ptree &element) {
        handleDelete(element);
        handleInsert(element);
    }

    std::string OsmChangeHandler::getSubjectFor(const boost::property_tree::ptree &element) {
        auto childrenTags = olu::util::XmlReader::readTagOfChildren(
                config::constants::OSM_TAG,
                element);
        if (childrenTags.size() != 1) {
            std::cerr << "Element should not contain more or less than one children" << std::endl;
        }

        auto elementTag = childrenTags.front();

        std::string identifier;
        if (elementTag == config::constants::NODE_TAG) {
            identifier = olu::util::XmlReader::readAttribute(
                    olu::config::constants::ATTRIBUTE_PATH_FOR_NODE_ID,
                    element);
            return config::constants::NODE_SUBJECT + ":" + identifier;
        }

        if (elementTag == config::constants::WAY_TAG) {
            identifier = olu::util::XmlReader::readAttribute(
                    olu::config::constants::ATTRIBUTE_PATH_FOR_WAY_ID,
                    element);
            return config::constants::WAY_SUBJECT + ":" + identifier;
        }

        if (elementTag == config::constants::RELATION_TAG) {
            identifier = olu::util::XmlReader::readAttribute(
                    olu::config::constants::ATTRIBUTE_PATH_FOR_RELATION_ID,
                    element);
            return config::constants::RELATION_SUBJECT + ":" + identifier;
        }

        throw OsmChangeHandlerException(
                ("Could not determine subject for element: " +
                util::XmlReader::writeXmlElementToString(element)).c_str());
    }

} // namespace olu::osm
