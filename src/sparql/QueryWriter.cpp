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
#include <sstream>

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeInsertQuery(const std::vector<std::string>& triples) const {
    std::ostringstream tripleClause;
    for (const auto & element : triples) {
        tripleClause << element;
        tripleClause << " . ";
    }

    std::ostringstream oss;
    oss << "INSERT DATA ";
    oss << "{ ";
    oss << wrapWithGraphOptional(tripleClause.str());
    oss << "}";
    return oss.str();
}

// _________________________________________________________________________________________________
std::string
olu::sparql::QueryWriter::writeDeleteQuery(const std::set<id_t> &ids, const std::string &osmTag) const {
    std::string optionalPredicates;
    if (osmTag == "osmnode") {
        optionalPredicates = "VALUES ?pred { geo:asWKT }";
    } else if (osmTag == "osmway") {
        optionalPredicates = "VALUES ?pred { osmway:member_id osmway:member_pos geo:asWKT }";
    } else if (osmTag == "osmrel") {
        optionalPredicates = "VALUES ?pred { osmrel:member_id osmrel:member_role osmrel:member_pos geo:asWKT }";
    } else {
        const std::string msg = "Unknown osmTag: " + osmTag;
        throw QueryWriterException(msg.c_str());
    }

    std::ostringstream oss;
    oss << "DELETE { ";
    oss << wrapWithGraphOptional("?val ?p1 ?o1 . ?o1 ?pred ?o2 . ");
    oss << "} WHERE { ";
    oss << getValuesClause(osmTag + ":", ids);
    oss << wrapWithGraphOptional(
        "?val ?p1 ?o1 FILTER (! STRSTARTS(?p1, ogc:)) . "
        "OPTIONAL {" + optionalPredicates + " ?o1 ?pred ?o2. }");
    oss << " }";
    return oss.str();
}

// _________________________________________________________________________________________________
std::string
olu::sparql::QueryWriter::writeQueryForNodeLocations(const std::set<id_t> &nodeIds) const {
    std::ostringstream oss;
    oss << "SELECT ?nodeGeo ?location ";
    oss << getFromClauseOptional();
    oss << "WHERE { ";
    oss << getValuesClause("osm2rdfgeom:osm_node_", nodeIds);
    oss << "?val geo:asWKT ?location . }";
    return oss.str();
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeQueryForLatestNodeTimestamp() const {
    std::ostringstream oss;
    oss << "SELECT ?p ";
    oss << getFromClauseOptional();
    oss << "WHERE { ?s rdf:type osm:node . ?s osmmeta:timestamp ?p . } ORDER BY DESC(?p) LIMIT 1";
    return oss.str();
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeQueryForRelations(const std::set<id_t> & relationIds) const {
    std::ostringstream oss;
    oss << "SELECT ?val ?type"
          "(GROUP_CONCAT(?memberUri; separator=\";\") AS ?memberUris) "
          "(GROUP_CONCAT(?memberRole; separator=\";\") AS ?memberRoles) "
          "(GROUP_CONCAT(?memberPos; separator=\";\") AS ?memberPositions) ";
    oss << getFromClauseOptional();
    oss << "WHERE { ";
    oss << getValuesClause("osmrel:", relationIds);
    oss << "?val osmkey:type ?type . "
          "?val osmrel:member ?o . "
          "?o osm2rdfmember:id ?memberUri . "
          "?o osm2rdfmember:role ?memberRole . "
          "?o osm2rdfmember:pos ?memberPos . "
          "} GROUP BY ?val ?type";
    return oss.str();
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeQueryForWaysMembers(const std::set<id_t> &wayIds) const {
    std::ostringstream oss;
    oss << "SELECT ?val "
          "(GROUP_CONCAT(?nodeUri; separator=\";\") AS ?nodeUris) "
          "(GROUP_CONCAT(?nodePos; separator=\";\") AS ?nodePositions) ";
    oss << getFromClauseOptional();
    oss << "WHERE { ";
    oss << getValuesClause("osmway:", wayIds);
    oss << "?val osmway:node ?member . "
          "?member osmway:node ?nodeUri . "
          "?member osm2rdfmember:pos ?nodePos "
          "} GROUP BY ?val";
    return oss.str();
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeQueryForReferencedNodes(const std::set<id_t> &wayIds) const {
    std::ostringstream oss;
    oss << "SELECT ?node ";
    oss << getFromClauseOptional();
    oss << "WHERE { ";
    oss << getValuesClause("osmway:", wayIds);
    oss << "?val osmway:node ?member . ?member osmway:node ?node . } GROUP BY ?node";
    return oss.str();
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeQueryForRelationMembers(const std::set<id_t> &relIds) const {
    std::ostringstream oss;
    oss << "SELECT ?p ";
    oss << getFromClauseOptional();
    oss << "WHERE { ";
    oss << getValuesClause("osmrel:", relIds);
    oss << "?val osmrel:member ?o . ?o osm2rdfmember:id ?p . } GROUP BY ?p";
    return oss.str();
}

// _________________________________________________________________________________________________
std::string
olu::sparql::QueryWriter::writeQueryForWaysReferencingNodes(const std::set<id_t> &nodeIds) const {
    std::ostringstream oss;
    oss << "SELECT ?way ";
    oss << getFromClauseOptional();
    oss << "WHERE { ";
    oss << getValuesClause("osmnode:", nodeIds);
    oss << "?identifier osmway:node ?val . ?way osmway:node ?identifier . } GROUP BY ?way";
    return oss.str();
}

// _________________________________________________________________________________________________
std::string
olu::sparql::QueryWriter::writeQueryForRelationsReferencingNodes(const std::set<id_t> &nodeIds) const {
    std::ostringstream oss;
    oss << "SELECT ?s ";
    oss << getFromClauseOptional();
    oss << "WHERE { ";
    oss << getValuesClause("osmnode:", nodeIds);
    oss << "?s osmrel:member ?o . ?o osm2rdfmember:id ?val . } GROUP BY ?s";
    return oss.str();
}

// _________________________________________________________________________________________________
std::string
olu::sparql::QueryWriter::writeQueryForRelationsReferencingWays(const std::set<id_t> &wayIds) const {
    std::ostringstream oss;
    oss << "SELECT ?s ";
    oss << getFromClauseOptional();
    oss << "WHERE { ";
    oss << getValuesClause("osmway:", wayIds);
    oss << "?s osmrel:member ?o . ?o osm2rdfmember:id ?val . } GROUP BY ?s";
    return oss.str();
}

// _________________________________________________________________________________________________
std::string
olu::sparql::QueryWriter::writeQueryForRelationsReferencingRelations(const std::set<id_t> &relationIds) const {
    std::ostringstream oss;
    oss << "SELECT ?s ";
    oss << getFromClauseOptional();
    oss << "WHERE { ";
    oss << getValuesClause("osmrel:", relationIds);
    oss << "?s osmrel:member ?o . "
          "?o osm2rdfmember:id ?val . } "
          "GROUP BY ?s";
    return oss.str();
}

std::string olu::sparql::QueryWriter::writeQueryForTagsAndTimestamp(const std::string &subject) const {
    std::ostringstream oss;
    oss << "SELECT ?key ?value ?time ";
    oss << getFromClauseOptional();
    oss << "WHERE { { ";
    oss << subject;
    oss << " ?key ?value . "
          "FILTER regex(str(?key), \"https://www.openstreetmap.org/wiki/Key:\") } "
          "UNION { ";
    oss << subject;
    oss << " osmmeta:timestamp ?time } }";

    return oss.str();
}

std::string olu::sparql::QueryWriter::getFromClauseOptional() const {
    return  _config.graphUri.empty() ? "" : "FROM <" +_config.graphUri + "> ";
}

std::string olu::sparql::QueryWriter::getValuesClause(const std::string &osmTag,
                                                      const std::set<id_t> &objectIds) {
    std::ostringstream oss;
    oss << "VALUES ?val { ";
    for (const auto & objectId : objectIds) {
        oss << osmTag;
        oss << std::to_string(objectId);
        oss << " ";
    }
    oss << "} ";

    return  oss.str();
}

std::string olu::sparql::QueryWriter::wrapWithGraphOptional(const std::string& clause) const {
    return _config.graphUri.empty() ? clause : "GRAPH <" + _config.graphUri + "> { " + clause + " } ";
}
