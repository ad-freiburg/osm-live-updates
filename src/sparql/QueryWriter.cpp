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

#include <string>
#include <vector>
#include <sstream>

namespace cnst = olu::config::constants;

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
        optionalPredicates = "VALUES ?pred { "+ cnst::OSM_2_RDF_GEO_AS_WKT +" }";
    } else if (osmTag == "osmway") {
        optionalPredicates = "VALUES ?pred { " + cnst::OSM_2_RDF_WAY_MEMBER_ID + " " + cnst::OSM_2_RDF_WAY_MEMBER_POS + " " + cnst::OSM_2_RDF_GEO_AS_WKT + " }";
    } else if (osmTag == "osmrel") {
        optionalPredicates = "VALUES ?pred { " + cnst::OSM_2_RDF_RELATION_MEMBER_ID + " " + cnst::OSM_2_RDF_RELATION_MEMBER_POS + " " + cnst::OSM_2_RDF_RELATION_MEMBER_ROLE + " " + cnst::OSM_2_RDF_GEO_AS_WKT + " }";
    } else {
        const std::string msg = "Unknown osmTag: " + osmTag;
        throw QueryWriterException(msg.c_str());
    }

    std::ostringstream oss;
    oss << "DELETE { ";
    oss << wrapWithGraphOptional(
        getTripleClause("?" + cnst::QUERY_VARIABLE_VALUE, "?p1", "?o1") +
        getTripleClause("?o1", "?pred", "?o2"));
    oss << "} WHERE { ";
    oss << wrapWithGraphOptional(
        getValuesClause(osmTag + ":", ids) +
        getTripleClause("?" + cnst::QUERY_VARIABLE_VALUE, "?p1", "?o1") +
        "FILTER (! STRSTARTS(STR(?p1), STR(ogc:))) . "
        "OPTIONAL {" + optionalPredicates +
        getTripleClause("?o1", "?pred", "?o2") + "}" );
    oss << " }";
    return oss.str();
}

// _________________________________________________________________________________________________
std::string
olu::sparql::QueryWriter::writeDeleteQueryForMetaAndTags(const std::set<id_t> &ids,
                                                         const std::string &osmTag) const {
    std::ostringstream oss;
    oss << "DELETE { ";
    oss << wrapWithGraphOptional(
        getTripleClause("?" + cnst::QUERY_VARIABLE_VALUE, "?p", "?o"));
    oss << "} WHERE { ";
    oss << wrapWithGraphOptional(
        getValuesClause(osmTag + ":", ids) +
        getTripleClause("?" + cnst::QUERY_VARIABLE_VALUE, "?p", "?o") +
        "FILTER (STRSTARTS(STR(?p),STR(osmmeta:)) || STRSTARTS(STR(?p),STR(osmkey:)) || STRSTARTS(STR(?p),STR(osm2rdf:facts))) .  }");
    return oss.str();
}


// _________________________________________________________________________________________________
std::string
olu::sparql::QueryWriter::writeDeleteQueryForGeometry(const std::set<id_t> &ids,
                                                      const std::string &osmTag) const {
    std::ostringstream oss;
    oss << "DELETE { ";
    oss << wrapWithGraphOptional(
        getTripleClause("?" + cnst::QUERY_VARIABLE_VALUE, "osm2rdfgeom:obb", "?o1") +
        getTripleClause("?" + cnst::QUERY_VARIABLE_VALUE, "osm2rdfgeom:envelope", "?o2") +
        getTripleClause("?" + cnst::QUERY_VARIABLE_VALUE, "osm2rdfgeom:convex_hull", "?o3") +
        getTripleClause("?" + cnst::QUERY_VARIABLE_VALUE, "osm2rdf:length", "?o4") +
        getTripleClause("?" + cnst::QUERY_VARIABLE_VALUE, "osm2rdf:area", "?o5") +
        getTripleClause("?geom", "geo:asWKT", "?o6") +
        getTripleClause("?cent", "geo:asWKT", "?o7"));
    oss << "} WHERE { ";
    oss << wrapWithGraphOptional(
        getValuesClause(osmTag + ":", ids) +
        "OPTIONAL { ?value osm2rdfgeom:obb ?o1 . } "
        "OPTIONAL { ?value osm2rdfgeom:envelope ?o2 . } "
        "OPTIONAL { ?value osm2rdfgeom:convex_hull ?o3 . } "
        "OPTIONAL { ?value osm2rdf:length ?o4 . } "
        "OPTIONAL { ?value osm2rdf:area ?o5 . } "
        "OPTIONAL { ?value geo:hasGeometry ?geom . ?geom geo:asWKT ?o6 . } "
        "OPTIONAL { ?value geo:hasCentroid ?cent . ?cent geo:asWKT ?o7 . } ");
    oss << " }";
    return oss.str();
}

// _________________________________________________________________________________________________
std::string
olu::sparql::QueryWriter::writeQueryForNodeLocations(const std::set<id_t> &nodeIds) const {
    std::ostringstream oss;
    oss << "SELECT ?" + cnst::QUERY_VARIABLE_VALUE + " ?" + cnst::QUERY_VARIABLE_LOCATION + " ";
    oss << getFromClauseOptional();
    oss << "WHERE { ";
    oss << getValuesClause("osm2rdfgeom:osm_node_", nodeIds);
    oss << getTripleClause("?" + cnst::QUERY_VARIABLE_VALUE, cnst::OSM_2_RDF_GEO_AS_WKT, "?" + cnst::QUERY_VARIABLE_LOCATION);
    oss << "}";
    return oss.str();
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeQueryForLatestNodeTimestamp() const {
    std::ostringstream oss;
    oss << "SELECT ?" + cnst::QUERY_VARIABLE_TIMESTAMP + " ";
    oss << getFromClauseOptional();
    oss << "WHERE { ";
    oss << getTripleClause("?node", "rdf:type", "osm:node");
    oss << getTripleClause("?node", "osmmeta:timestamp", "?" + cnst::QUERY_VARIABLE_TIMESTAMP);
    oss << "} ORDER BY DESC(?" + cnst::QUERY_VARIABLE_TIMESTAMP + ") LIMIT 1";
    return oss.str();
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeQueryForRelations(const std::set<id_t> & relationIds) const {
    std::ostringstream oss;
    oss << "SELECT ?" + cnst::QUERY_VARIABLE_VALUE + " ?" + cnst::QUERY_VARIABLE_TYPE + " "
          "(GROUP_CONCAT(?memberUri; separator=\";\") AS ?" + cnst::QUERY_VARIABLE_MEMBER_URIS + ") "
          "(GROUP_CONCAT(?memberRole; separator=\";\") AS ?" + cnst::QUERY_VARIABLE_MEMBER_ROLES + ") "
          "(GROUP_CONCAT(?memberPos; separator=\";\") AS ?" + cnst::QUERY_VARIABLE_MEMBER_POSITIONS + ") ";
    oss << getFromClauseOptional();
    oss << "WHERE { ";
    oss << getValuesClause("osmrel:", relationIds);
    oss << getTripleClause("?" + cnst::QUERY_VARIABLE_VALUE, "osmkey:type", "?type");
    oss << getTripleClause("?" + cnst::QUERY_VARIABLE_VALUE, cnst::OSM_2_RDF_RELATION_MEMBER, "?member");
    oss << getTripleClause("?member", cnst::OSM_2_RDF_RELATION_MEMBER_ID, "?memberUri");
    oss << getTripleClause("?member", cnst::OSM_2_RDF_RELATION_MEMBER_ROLE, "?memberRole");
    oss << getTripleClause("?member", cnst::OSM_2_RDF_RELATION_MEMBER_POS, "?memberPos");
    oss << "} GROUP BY ?" + cnst::QUERY_VARIABLE_VALUE + " ?" + cnst::QUERY_VARIABLE_TYPE;
    return oss.str();
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeQueryForWaysMembers(const std::set<id_t> &wayIds) const {
    std::ostringstream oss;
    oss << "SELECT ?" + cnst::QUERY_VARIABLE_VALUE + " "
          "(GROUP_CONCAT(?nodeUri; separator=\";\") AS ?" + cnst::QUERY_VARIABLE_MEMBER_URIS + ") "
          "(GROUP_CONCAT(?nodePos; separator=\";\") AS ?" + cnst::QUERY_VARIABLE_MEMBER_POSITIONS + ") ";
    oss << getFromClauseOptional();
    oss << "WHERE { ";
    oss << getValuesClause("osmway:", wayIds);
    oss << getTripleClause("?" + cnst::QUERY_VARIABLE_VALUE, cnst::OSM_2_RDF_WAY_MEMBER, "?node");
    oss << getTripleClause("?node", cnst::OSM_2_RDF_WAY_MEMBER_ID, "?nodeUri");
    oss << getTripleClause("?node", cnst::OSM_2_RDF_WAY_MEMBER_POS, "?nodePos");
    oss << "} GROUP BY ?" + cnst::QUERY_VARIABLE_VALUE;
    return oss.str();
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeQueryForReferencedNodes(const std::set<id_t> &wayIds) const {
    std::ostringstream oss;
    oss << "SELECT ?node ";
    oss << getFromClauseOptional();
    oss << "WHERE { ";
    oss << getValuesClause("osmway:", wayIds);
    oss << getTripleClause("?" + cnst::QUERY_VARIABLE_VALUE, cnst::OSM_2_RDF_WAY_MEMBER, "?member");
    oss << getTripleClause("?member", cnst::OSM_2_RDF_WAY_MEMBER_ID, "?node");
    oss << "} GROUP BY ?node";
    return oss.str();
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeQueryForRelationMembers(const std::set<id_t> &relIds) const {
    std::ostringstream oss;
    oss << "SELECT ?p ";
    oss << getFromClauseOptional();
    oss << "WHERE { ";
    oss << getValuesClause("osmrel:", relIds);
    oss << getTripleClause("?" + cnst::QUERY_VARIABLE_VALUE, cnst::OSM_2_RDF_RELATION_MEMBER, "?o");
    oss << getTripleClause("?o", cnst::OSM_2_RDF_RELATION_MEMBER_ID, "?p");
    oss << "} GROUP BY ?p";
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
    oss << getTripleClause("?s", cnst::OSM_2_RDF_WAY_MEMBER_ID, "?" + cnst::QUERY_VARIABLE_VALUE);
    oss << getTripleClause("?way", cnst::OSM_2_RDF_WAY_MEMBER, "?s");
    oss << "} GROUP BY ?way";
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
    oss << getTripleClause("?s", cnst::OSM_2_RDF_RELATION_MEMBER, "?o");
    oss << getTripleClause("?o", cnst::OSM_2_RDF_RELATION_MEMBER_ID, "?" + cnst::QUERY_VARIABLE_VALUE);
    oss << "} GROUP BY ?s";
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
    oss << getTripleClause("?s", cnst::OSM_2_RDF_RELATION_MEMBER, "?o");
    oss << getTripleClause("?o", cnst::OSM_2_RDF_RELATION_MEMBER_ID, "?" + cnst::QUERY_VARIABLE_VALUE);
    oss << "} GROUP BY ?s";
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
    oss << getTripleClause("?s", cnst::OSM_2_RDF_RELATION_MEMBER, "?o");
    oss << getTripleClause("?o", cnst::OSM_2_RDF_RELATION_MEMBER_ID, "?" + cnst::QUERY_VARIABLE_VALUE);
    oss << "} ";
    oss << "GROUP BY ?s";
    return oss.str();
}

std::string olu::sparql::QueryWriter::writeQueryForTagsAndMetaInfo(const std::string &subject) const {
    std::ostringstream oss;
    oss << "SELECT ";
    oss << "?" + cnst::QUERY_VARIABLE_KEY + " ";
    oss << "?" + cnst::QUERY_VARIABLE_VALUE + " ";
    oss << "?" + cnst::QUERY_VARIABLE_TIMESTAMP + " ";
    oss << "?" + cnst::QUERY_VARIABLE_VERSION + " ";
    oss << "?" + cnst::QUERY_VARIABLE_CHANGESET + " ";
    oss << getFromClauseOptional();
    oss << "WHERE { { ";
    oss << getTripleClause(subject, "?" + cnst::QUERY_VARIABLE_KEY, "?" + cnst::QUERY_VARIABLE_VALUE);
    oss << "FILTER REGEX(STR(?" + cnst::QUERY_VARIABLE_KEY + "), STR(osmkey:)) } ";
    oss << wrapWithUnion(getTripleClause(subject, "osmmeta:timestamp", "?" + cnst::QUERY_VARIABLE_TIMESTAMP));
    oss << wrapWithUnion(getTripleClause(subject, "osmmeta:version", "?" + cnst::QUERY_VARIABLE_VERSION));
    oss << wrapWithUnion(getTripleClause(subject, "osmmeta:changeset", "?" + cnst::QUERY_VARIABLE_CHANGESET));
    oss << " }";

    return oss.str();
}

std::string olu::sparql::QueryWriter::getFromClauseOptional() const {
    return  _config.graphUri.empty() ? "" : "FROM <" +_config.graphUri + "> ";
}

std::string olu::sparql::QueryWriter::getValuesClause(const std::string &osmTag,
                                                      const std::set<id_t> &objectIds) {
    std::ostringstream oss;
    oss << "VALUES ?"+ cnst::QUERY_VARIABLE_VALUE +" { ";
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

std::string olu::sparql::QueryWriter::wrapWithUnion(const std::string& clause) {
    return "UNION { " + clause + " } ";
}

std::string olu::sparql::QueryWriter::getTripleClause(const std::string& subject,
                                                      const std::string& predicate,
                                                      const std::string& object) {
    return subject + " " + predicate + " " + object + " . ";
}
