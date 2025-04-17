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

    std::ostringstream ss;
    ss << "INSERT DATA ";
    ss << "{ ";
    ss << wrapWithGraphOptional(tripleClause.str());
    ss << "}";
    return ss.str();
}

// _________________________________________________________________________________________________
std::string
olu::sparql::QueryWriter::writeDeleteQuery(const std::set<id_t> &ids, const std::string &osmTag) const {
    std::ostringstream ss;
    ss << "DELETE { ";
    ss << wrapWithGraphOptional("?s ?p1 ?o1 . ?o1 ?p2 ?o2 . ");
    ss << "WHERE { ";
    ss << getValuesClause(osmTag + ":", ids);
    ss << wrapWithGraphOptional("?val ?p1 ?o1 FILTER (! STRSTARTS(?p1, ogc:)) . OPTIONAL { ?o1 ?p2 ?o2. }"); ss << " }";
    return ss.str();
}

// _________________________________________________________________________________________________
std::string
olu::sparql::QueryWriter::writeQueryForNodeLocations(const std::set<id_t> &nodeIds) const {
    std::ostringstream ss;
    ss << "SELECT ?nodeGeo ?location ";
    ss << getFromClauseOptional();
    ss << "WHERE { ";
    ss << getValuesClause("osm2rdfgeom:osm_node_", nodeIds);
    ss << "?val geo:asWKT ?location . }";
    return ss.str();
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeQueryForLatestNodeTimestamp() const {
    std::ostringstream ss;
    ss << "SELECT ?p ";
    ss << getFromClauseOptional();
    ss << "WHERE { ?s rdf:type osm:node . ?s osmmeta:timestamp ?p . } ORDER BY DESC(?p) LIMIT 1";
    return ss.str();
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeQueryForRelations(const std::set<id_t> & relationIds) const {
    std::ostringstream ss;
    ss << "SELECT ?val ?type"
          "(GROUP_CONCAT(?memberUri; separator=\";\") AS ?memberUris) "
          "(GROUP_CONCAT(?memberRole; separator=\";\") AS ?memberRoles) "
          "(GROUP_CONCAT(?memberPos; separator=\";\") AS ?memberPositions) ";
    ss << getFromClauseOptional();
    ss << "WHERE { ";
    ss << getValuesClause("osmrel:", relationIds);
    ss << "?val osmkey:type ?type . "
          "?val osmrel:member ?o . "
          "?o osm2rdfmember:id ?memberUri . "
          "?o osm2rdfmember:role ?memberRole . "
          "?o osm2rdfmember:pos ?memberPos . "
          "} GROUP BY ?val ?type";
    return ss.str();
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeQueryForWaysMembers(const std::set<id_t> &wayIds) const {
    std::ostringstream ss;
    ss << "SELECT ?val "
          "(GROUP_CONCAT(?nodeUri; separator=\";\") AS ?nodeUris) "
          "(GROUP_CONCAT(?nodePos; separator=\";\") AS ?nodePositions) ";
    ss << getFromClauseOptional();
    ss << "WHERE { ";
    ss << getValuesClause("osmway:", wayIds);
    ss << "?val osmway:node ?member . "
          "?member osmway:node ?nodeUri . "
          "?member osm2rdfmember:pos ?nodePos "
          "} GROUP BY ?val";
    return ss.str();
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeQueryForReferencedNodes(const std::set<id_t> &wayIds) const {
    std::ostringstream ss;
    ss << "SELECT ?node ";
    ss << getFromClauseOptional();
    ss << "WHERE { ";
    ss << getValuesClause("osmway:", wayIds);
    ss << "?val osmway:node ?member . ?member osmway:node ?node . } GROUP BY ?node";
    return ss.str();
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeQueryForRelationMembers(const std::set<id_t> &relIds) const {
    std::ostringstream ss;
    ss << "SELECT ?p ";
    ss << getFromClauseOptional();
    ss << "WHERE { ";
    ss << getValuesClause("osmrel:", relIds);
    ss << "?val osmrel:member ?o . ?o osm2rdfmember:id ?p . } GROUP BY ?p";
    return ss.str();
}

// _________________________________________________________________________________________________
std::string
olu::sparql::QueryWriter::writeQueryForWaysReferencingNodes(const std::set<id_t> &nodeIds) const {
    std::ostringstream ss;
    ss << "SELECT ?way ";
    ss << getFromClauseOptional();
    ss << "WHERE { ";
    ss << getValuesClause("osmnode:", nodeIds);
    ss << "?identifier osmway:node ?val . ?way osmway:node ?identifier . } GROUP BY ?way";
    return ss.str();
}

// _________________________________________________________________________________________________
std::string
olu::sparql::QueryWriter::writeQueryForRelationsReferencingNodes(const std::set<id_t> &nodeIds) const {
    std::ostringstream ss;
    ss << "SELECT ?s ";
    ss << getFromClauseOptional();
    ss << "WHERE { ";
    ss << getValuesClause("osmnode:", nodeIds);
    ss << "?s osmrel:member ?o . ?o osm2rdfmember:id ?val . } GROUP BY ?s";
    return ss.str();
}

// _________________________________________________________________________________________________
std::string
olu::sparql::QueryWriter::writeQueryForRelationsReferencingWays(const std::set<id_t> &wayIds) const {
    std::ostringstream ss;
    ss << "SELECT ?s ";
    ss << getFromClauseOptional();
    ss << "WHERE { ";
    ss << getValuesClause("osmway:", wayIds);
    ss << "?s osmrel:member ?o . ?o osm2rdfmember:id ?val . } GROUP BY ?s";
    return ss.str();
}

// _________________________________________________________________________________________________
std::string
olu::sparql::QueryWriter::writeQueryForRelationsReferencingRelations(const std::set<id_t> &relationIds) const {
    std::ostringstream ss;
    ss << "SELECT ?s ";
    ss << getFromClauseOptional();
    ss << "WHERE { ";
    ss << getValuesClause("osmrel:", relationIds);
    ss << "?s osmrel:member ?o . "
          "?o osm2rdfmember:id ?val . } "
          "GROUP BY ?s";
    return ss.str();
}

std::string olu::sparql::QueryWriter::writeQueryForTagsAndTimestamp(const std::string &subject) const {
    std::ostringstream ss;
    ss << "SELECT ?key ?value ?time ";
    ss << getFromClauseOptional();
    ss << "WHERE { { ";
    ss << subject;
    ss << " ?key ?value . "
          "FILTER regex(str(?key), \"https://www.openstreetmap.org/wiki/Key:\") } "
          "UNION { ";
    ss << subject;
    ss << " osmmeta:timestamp ?time } }";

    return ss.str();
}

std::string olu::sparql::QueryWriter::getFromClauseOptional() const {
    return  _config.graphUri.empty() ? "" : "FROM <" +_config.graphUri + "> ";
}

std::string olu::sparql::QueryWriter::getValuesClause(const std::string &osmTag,
                                                      const std::set<id_t> &ids) {
    std::ostringstream ss;
    ss << "VALUES ?val { ";
    for (const auto & id : ids) {
        ss << osmTag;
        ss << std::to_string(id);
        ss << " ";
    }
    ss << "} ";

    return  ss.str();
}

std::string olu::sparql::QueryWriter::wrapWithGraphOptional(const std::string& clause) const {
    return _config.graphUri.empty() ? clause : "GRAPH <" + _config.graphUri + "> { " + clause + " } ";
}
