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

#include "sparql/QueryWriter.h"
#include "sparql/QueryWriterException.h"
#include "config/Constants.h"
#include "util/XmlReader.h"

#include <string>
#include <vector>
#include <iostream>

//TODO: Check for valid triples

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeInsertQuery(std::vector<std::string>& triples) {
    std::string query;
    query = olu::config::constants::PREFIXES;
    query += "INSERT DATA {\n";

    for (auto & element : triples) {
        query += element + "\n";
    }

    query += "}";
    return query;
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeDeleteQuery(std::string& subject) {
    std::string query;
    query = olu::config::constants::PREFIXES;
    query += "DELETE { ?s ?p ?o } WHERE {\n"
            + subject +
            " ?p ?o .\n"
            "}";
    return query;
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::getSubjectFor(const std::string &elementTag,
                                                    const boost::property_tree::ptree &element) {
    std::string identifier;
    if (elementTag == config::constants::NODE_TAG) {
        identifier = olu::util::XmlReader::readAttribute(
                olu::config::constants::ID_ATTRIBUTE,
                element);
        return config::constants::NODE_SUBJECT + ":" + identifier;
    }

    if (elementTag == config::constants::WAY_TAG) {
        identifier = olu::util::XmlReader::readAttribute(
                olu::config::constants::ID_ATTRIBUTE,
                element);
        return config::constants::WAY_SUBJECT + ":" + identifier;
    }

    if (elementTag == config::constants::RELATION_TAG) {
        identifier = olu::util::XmlReader::readAttribute(
                olu::config::constants::ID_ATTRIBUTE,
                element);
        return config::constants::RELATION_SUBJECT + ":" + identifier;
    }

    throw QueryWriterException(
            ("Could not determine subject for element: " +
                    util::XmlReader::readTree(element)).c_str());
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeQueryForNodeLocation(const std::string &nodeId) {
    auto query = olu::config::constants::PREFIXES;
    query += "SELECT { ?o } WHERE {\n"
             + config::constants::NODE_SUBJECT + ":" + nodeId + " "
             + config::constants::LOCATION_AS_WKT_PREDICATE + " "
             "?o .\n"
             "}";
    return query;
}
