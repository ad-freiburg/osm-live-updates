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
#include "config/Constants.h"
#include "util/XmlReader.h"

#include <string>
#include <vector>
#include <iostream>

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeInsertQuery(const std::vector<std::string>& triples) {
    std::string query;
    query += "INSERT DATA { ";

    for (const auto & element : triples) {
        query += element + " ";
    }

    query += "}";
    return query;
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeDeleteQuery(const std::string& subject) {
    std::string query;
    // Todo: This does not work with the current implementation of sparql updates
//    query += "DELETE { ?s ?p ?o } WHERE { "
//             + subject +
//             " ?p ?o . }";
    query += "DELETE { " + subject + "?p ?o } WHERE { "
            + subject +
            " ?p ?o . }";
    return query;
}

// _________________________________________________________________________________________________
std::string
olu::sparql::QueryWriter::getSubjectFor(const std::string &elementTag,
                                        const boost::property_tree::ptree &element) {
    std::string identifier;
    try {
        identifier = olu::util::XmlReader::readAttribute(
                olu::config::constants::ID_ATTRIBUTE,
                element);
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::string msg = "Could not identifier of element: " + util::XmlReader::readTree(element);
        throw QueryWriterException(msg.c_str());
    }

    if (elementTag == config::constants::NODE_TAG) {
        return "osmnode:" + identifier;
    }

    if (elementTag == config::constants::WAY_TAG) {
        return config::constants::WAY_SUBJECT + ":" + identifier;
    }

    if (elementTag == config::constants::RELATION_TAG) {
        return config::constants::RELATION_SUBJECT + ":" + identifier;
    }

    std::string msg = "Could not determine subject for element: "
            + util::XmlReader::readTree(element);
    throw QueryWriterException(msg.c_str());
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeQueryForNodeLocation(const long long &nodeId) {
    // TODO: Find out why this does not always work
//    std::string query = "SELECT ?o WHERE { osmnode:" + std::to_string(nodeId) + " geo:hasGeometry/geo:asWKT ?o . }";
    std::string query = "SELECT ?o WHERE { osm2rdfgeom:osm_node_" + std::to_string(nodeId) + " geo:asWKT ?o . }";
    return query;
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeQueryForLatestNodeTimestamp() {
    std::string query = "SELECT ?p WHERE { ?s rdf:type osm:node . ?s osmmeta:timestamp ?p . } "
                        "ORDER BY DESC(?p) LIMIT 1";
    return query;
}
