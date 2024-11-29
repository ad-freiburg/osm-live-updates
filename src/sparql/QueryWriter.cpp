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

#include <string>
#include <vector>

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeInsertQuery(const std::vector<std::string>& triples) {
    std::string query;
    query += "INSERT DATA { ";

    for (const auto & element : triples) {
        query += element + " . ";
    }

    query += "}";
    return query;
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeDeleteQuery(const std::vector<std::string>& subjects) {
    std::string query;
    query += "DELETE WHERE { ";

    for (size_t i = 0; i < subjects.size(); ++i) {
        query += subjects[i] + " ?p" + std::to_string(i) + " ?o" + std::to_string(i) + " . ";
    }

    query += "}";
    return query;
}

// _________________________________________________________________________________________________
std::string
olu::sparql::QueryWriter::writeQueryForNodeLocations(const std::set<id_t> &nodeIds) {
    std::string query;
    query += "SELECT ?nodeGeo ?location WHERE { VALUES (?nodeGeo) { ";

    for (const auto & nodeId : nodeIds) {
        query += "(osm2rdfgeom:osm_node_" + std::to_string(nodeId) + ") ";
    }

    query += "} ?nodeGeo geo:asWKT ?location . }";
    return query;
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeQueryForLatestNodeTimestamp() {
    std::string query = "SELECT ?p WHERE { ?s rdf:type osm:node . ?s osmmeta:timestamp ?p . } "
                        "ORDER BY DESC(?p) LIMIT 1";
    return query;
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeQueryForRelations(const std::set<id_t> & relationIds) {
    std::string query = "SELECT ?rel ?id ?role ?key WHERE { VALUES (?rel) { ";

    for (const auto & relId : relationIds) {
        query += "(osmrel:" + std::to_string(relId)+ ") ";
    }

    query += "?rel osmkey:type ?key . "
             "?rel osmrel:member ?o . "
             "?o osm2rdfmember:id ?id . "
             "?o osm2rdfmember:role ?role . "
             "}  ORDER BY ?rel";
    return query;
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeQueryForWaysMembers(const std::set<id_t> &wayIds) {
    std::string query;
    query += "SELECT ?way ?node WHERE { VALUES (?way) { ";

    for (const auto & wayId : wayIds) {
        query += "(osmway:" + std::to_string(wayId)+ ") ";
    }

    query += "} ?way osmway:node ?member . ?member osmway:node ?node . }";
    return query;
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeQueryForReferencedNodes(const std::set<id_t> &wayIds) {
    std::string query;
    query += "SELECT ?node WHERE { VALUES (?way) { ";

    for (const auto & wayId : wayIds) {
        query += "(osmway:" + std::to_string(wayId)+ ") ";
    }

    query += "} ?way osmway:node ?member . ?member osmway:node ?node . } GROUP BY ?node";
    return query;
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeQueryForRelationMembers(const std::set<id_t> &relIds) {
    std::string query;
    query += "SELECT ?p WHERE { VALUES (?rel) { ";

    for (const auto & relId : relIds) {
        query += "(osmrel:" + std::to_string(relId)+ ") ";
    }

    query += "} ?rel osmrel:member ?o . ?o osm2rdfmember:id ?p . } GROUP BY ?p";
    return query;
}

// _________________________________________________________________________________________________
std::string
olu::sparql::QueryWriter::writeQueryForWaysReferencingNodes(const std::set<id_t> &nodeIds) {
    std::string query;
    query += "SELECT ?way WHERE { VALUES (?node) { ";

    for (const auto & nodeId : nodeIds) {
        query += "(osmnode:" + std::to_string(nodeId) + ") ";
    }

    query += "} ?identifier osmway:node ?node . ?way osmway:node ?identifier . } GROUP BY ?way";
    return query;
}

// _________________________________________________________________________________________________
std::string
olu::sparql::QueryWriter::writeQueryForRelationsReferencingNodes(const std::set<id_t> &nodeIds) {
    std::string query;
    query += "SELECT ?s WHERE { VALUES (?node) { ";

    for (const auto & nodeId : nodeIds) {
        query += "(osmnode:" + std::to_string(nodeId) + ") ";
    }

    query += "} ?s osmrel:member ?o . ?o osm2rdfmember:id ?node . } GROUP BY ?s";
    return query;
}

// _________________________________________________________________________________________________
std::string
olu::sparql::QueryWriter::writeQueryForRelationsReferencingWays(const std::set<id_t> &wayIds) {
    std::string query;
    query += "SELECT ?s WHERE { VALUES (?way) { ";

    for (const auto & wayId : wayIds) {
        query += "(osmway:" + std::to_string(wayId) + ") ";
    }

    query += "} ?s osmrel:member ?o . ?o osm2rdfmember:id ?way . } GROUP BY ?s";
    return query;
}

// _________________________________________________________________________________________________
std::string
olu::sparql::QueryWriter::writeQueryForRelationsReferencingRelations(const std::set<id_t> &relationIds) {
    std::string query;
    query += "SELECT ?s WHERE { ";

    size_t c = 0;
    bool isFirst = true;
    for (const auto & relId : relationIds) {
        if (!isFirst) {
            query += "UNION ";
        }

        query += "{ ?s osmrel:member ?o" + std::to_string(c) + " . ?o" + std::to_string(c) + " osm2rdfmember:id osmrel:" + std::to_string(relId) + " . } ";
        isFirst = false;
        c++;
    }

    query += "} GROUP BY ?s";
    return query;
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeNodesDeleteQuery(const std::set<id_t> &nodeIds) {
    std::vector<std::string> subjects;
    for (const auto & nodeId : nodeIds) {
        subjects.push_back("osmnode:" + std::to_string(nodeId));
        subjects.push_back("osm2rdfgeom:osm_node_" + std::to_string(nodeId));
    }

    return writeDeleteQuery(subjects);
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeWaysDeleteQuery(const std::set<id_t> &wayIds) {
    std::vector<std::string> subjects;
    for (const auto & wayId : wayIds) {
        subjects.push_back("osmway:" + std::to_string(wayId));
        subjects.push_back("osm2rdf:way_" + std::to_string(wayId));
        subjects.push_back("osm2rdfgeom:osm_wayarea_" + std::to_string(wayId));
    }

    return writeDeleteQuery(subjects);
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeRelationsDeleteQuery(const std::set<id_t> &relationIds) {
    std::vector<std::string> subjects;
    for (const auto & relationId : relationIds) {
        subjects.push_back("osmrel:" + std::to_string(relationId));
        subjects.push_back("osm2rdfgeom:osm_relarea_" + std::to_string(relationId));
    }

    return writeDeleteQuery(subjects);
}
