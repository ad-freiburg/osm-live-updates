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
    if (osmTag == cnst::NAMESPACE_OSM_NODE) {
        optionalPredicates = "VALUES ?pred { "+ cnst::PREFIXED_GEO_AS_WKT +" }";
    } else if (osmTag == cnst::NAMESPACE_OSM_WAY) {
        optionalPredicates = "VALUES ?pred { " + cnst::PREFIXED_WAY_MEMBER_ID + " "
                                               + cnst::PREFIXED_WAY_MEMBER_POS + " "
                                               + cnst::PREFIXED_GEO_AS_WKT + " }";
    } else if (osmTag == cnst::NAMESPACE_OSM_REL) {
        optionalPredicates = "VALUES ?pred { " + cnst::PREFIXED_REL_MEMBER_ID + " "
                                               + cnst::PREFIXED_REL_MEMBER_POS + " "
                                               + cnst::PREFIXED_REL_MEMBER_ROLE + " "
                                               + cnst::PREFIXED_GEO_AS_WKT + " }";
    } else {
        const std::string msg = "Unknown osmTag: " + osmTag;
        throw QueryWriterException(msg.c_str());
    }

    std::ostringstream oss;
    oss << "DELETE { ";
    oss << wrapWithGraphOptional(
        getTripleClause(cnst::QUERY_VAR_VAL, "?p1", "?o1") +
        getTripleClause("?o1", "?pred", "?o2"));
    oss << "} WHERE { ";
    oss << wrapWithGraphOptional(
        getValuesClause(osmTag, ids) +
        getTripleClause(cnst::QUERY_VAR_VAL, "?p1", "?o1") +
        "FILTER (! STRSTARTS(STR(?p1), STR(" + cnst::NAMESPACE_OGC + ":))) . "
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
        getTripleClause( cnst::QUERY_VAR_VAL, "?p", "?o"));
    oss << "} WHERE { ";
    oss << wrapWithGraphOptional(
        getValuesClause(osmTag, ids) +
        getTripleClause(cnst::QUERY_VAR_VAL, "?p", "?o") +
        "FILTER (STRSTARTS(STR(?p),STR(" + cnst::NAMESPACE_OSM_META + ":)) || "
                 "STRSTARTS(STR(?p),STR(" + cnst::NAMESPACE_OSM_KEY + ":)) || "
                 "STRSTARTS(STR(?p),STR(" + cnst::PREFIXED_OSM2RDF_FACTS + "))) . }");
    return oss.str();
}


// _________________________________________________________________________________________________
std::string
olu::sparql::QueryWriter::writeDeleteQueryForGeometry(const std::set<id_t> &ids,
                                                      const std::string &osmTag) const {
    std::ostringstream oss;
    oss << "DELETE { ";
    oss << wrapWithGraphOptional(
        getTripleClause(cnst::QUERY_VAR_VAL, cnst::PREFIXED_OSM2RDF_GEOM_OBB, "?o1") +
        getTripleClause(cnst::QUERY_VAR_VAL, cnst::PREFIXED_OSM2RDF_GEOM_ENVELOPE, "?o2") +
        getTripleClause(cnst::QUERY_VAR_VAL, cnst::PREFIXED_OSM2RDF_GEOM_CONVEX_HULL, "?o3") +
        getTripleClause(cnst::QUERY_VAR_VAL, cnst::PREFIXED_OSM2RDF_LENGTH, "?o4") +
        getTripleClause(cnst::QUERY_VAR_VAL, cnst::PREFIXED_OSM2RDF_AREA, "?o5") +
        getTripleClause("?geom", cnst::PREFIXED_GEO_AS_WKT, "?o6") +
        getTripleClause("?cent", cnst::PREFIXED_GEO_AS_WKT, "?o7"));
    oss << "} WHERE { ";
    oss << wrapWithGraphOptional(
        getValuesClause(osmTag, ids) +
        wrapWithOptional(getTripleClause(cnst::QUERY_VAR_VAL, cnst::PREFIXED_OSM2RDF_GEOM_OBB, "?o1")) +
        wrapWithOptional(getTripleClause(cnst::QUERY_VAR_VAL, cnst::PREFIXED_OSM2RDF_GEOM_ENVELOPE, "?o2")) +
        wrapWithOptional(getTripleClause(cnst::QUERY_VAR_VAL, cnst::PREFIXED_OSM2RDF_GEOM_CONVEX_HULL, "?o3")) +
        wrapWithOptional(getTripleClause(cnst::QUERY_VAR_VAL, cnst::PREFIXED_OSM2RDF_LENGTH, "?o4")) +
        wrapWithOptional(getTripleClause(cnst::QUERY_VAR_VAL, cnst::PREFIXED_OSM2RDF_AREA, "?o5")) +
        wrapWithOptional(getTripleClause(cnst::QUERY_VAR_VAL, cnst::PREFIXED_GEO_HAS_GEOMETRY, "?geom") +
                         getTripleClause("?geom", cnst::PREFIXED_GEO_AS_WKT, "?o6")) +
        wrapWithOptional(getTripleClause(cnst::QUERY_VAR_VAL, cnst::PREFIXED_GEO_HAS_CENTROID, "?geom") +
                         getTripleClause("?geom", cnst::PREFIXED_GEO_AS_WKT, "?o7")));
    oss << " }";
    return oss.str();
}

// _________________________________________________________________________________________________
std::string
olu::sparql::QueryWriter::writeQueryForNodeLocations(const std::set<id_t> &nodeIds) const {
    std::ostringstream oss;
    oss << "SELECT " + cnst::QUERY_VAR_VAL + " " + cnst::QUERY_VAR_LOC + " ";
    oss << getFromClauseOptional();
    oss << "WHERE { ";
    oss << getValuesClause(cnst::PREFIXED_OSM2RDF_GEOM_NODE_, "", nodeIds);
    oss << getTripleClause(cnst::QUERY_VAR_VAL, cnst::PREFIXED_GEO_AS_WKT, cnst::QUERY_VAR_LOC);
    oss << "}";
    return oss.str();
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeQueryForLatestNodeTimestamp() const {
    std::ostringstream oss;
    oss << "SELECT " + cnst::QUERY_VAR_TIMESTAMP + " ";
    oss << getFromClauseOptional();
    oss << "WHERE { ";
    oss << getTripleClause("?node", cnst::PREFIXED_RDF_TYPE, cnst::PREFIXED_OSM_NODE);
    oss << getTripleClause("?node", cnst::PREFIXED_OSM_META_TIMESTAMP, cnst::QUERY_VAR_TIMESTAMP);
    oss << "} ORDER BY DESC(" + cnst::QUERY_VAR_TIMESTAMP + ") LIMIT 1";
    return oss.str();
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeQueryForRelations(const std::set<id_t> & relationIds) const {
    std::ostringstream oss;
    oss << "SELECT " + cnst::QUERY_VAR_VAL + " " + cnst::QUERY_VAR_TYPE + " "
          "(GROUP_CONCAT(" + cnst::QUERY_VAR_MEMBER_ID + "; separator=\";\") AS " + cnst::QUERY_VAR_MEMBER_IDS + ") "
          "(GROUP_CONCAT(" + cnst::QUERY_VAR_MEMBER_ROLE + "; separator=\";\") AS " + cnst::QUERY_VAR_MEMBER_ROLES + ") "
          "(GROUP_CONCAT(" + cnst::QUERY_VAR_MEMBER_POS + "; separator=\";\") AS " + cnst::QUERY_VAR_MEMBER_POSS + ") ";
    oss << getFromClauseOptional();
    oss << "WHERE { ";
    oss << getValuesClause(cnst::NAMESPACE_OSM_REL, relationIds);
    oss << getTripleClause(cnst::QUERY_VAR_VAL, cnst::PREFIXED_OSM_KEY_TYPE, cnst::QUERY_VAR_TYPE);
    oss << getTripleClause(cnst::QUERY_VAR_VAL, cnst::PREFIXED_REL_MEMBER, cnst::QUERY_VAR_MEMBER);
    oss << getTripleClause(cnst::QUERY_VAR_MEMBER, cnst::PREFIXED_REL_MEMBER_ID, cnst::QUERY_VAR_MEMBER_ID);
    oss << getTripleClause(cnst::QUERY_VAR_MEMBER, cnst::PREFIXED_REL_MEMBER_ROLE, cnst::QUERY_VAR_MEMBER_ROLE);
    oss << getTripleClause(cnst::QUERY_VAR_MEMBER, cnst::PREFIXED_REL_MEMBER_POS, cnst::QUERY_VAR_MEMBER_POS);
    oss << "} GROUP BY " + cnst::QUERY_VAR_VAL + " " + cnst::QUERY_VAR_TYPE;
    return oss.str();
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeQueryForWaysMembers(const std::set<id_t> &wayIds) const {
    std::ostringstream oss;
    oss << "SELECT " + cnst::QUERY_VAR_VAL + " "
          "(GROUP_CONCAT(?nodeUri; separator=\";\") AS " + cnst::QUERY_VAR_MEMBER_IDS + ") "
          "(GROUP_CONCAT(?nodePos; separator=\";\") AS " + cnst::QUERY_VAR_MEMBER_POSS + ") ";
    oss << getFromClauseOptional();
    oss << "WHERE { ";
    oss << getValuesClause(cnst::NAMESPACE_OSM_WAY, wayIds);
    oss << getTripleClause(cnst::QUERY_VAR_VAL, cnst::PREFIXED_WAY_MEMBER, "?node");
    oss << getTripleClause("?node", cnst::PREFIXED_WAY_MEMBER_ID, "?nodeUri");
    oss << getTripleClause("?node", cnst::PREFIXED_WAY_MEMBER_POS, "?nodePos");
    oss << "} GROUP BY " + cnst::QUERY_VAR_VAL;
    return oss.str();
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeQueryForRelsMembers(const std::set<id_t> &relIds) const {
    std::ostringstream oss;
    oss << "SELECT " + cnst::QUERY_VAR_VAL + " "
          "(GROUP_CONCAT(?memberUri; separator=\";\") AS " + cnst::QUERY_VAR_MEMBER_IDS + ") "
          "(GROUP_CONCAT(?memberPos; separator=\";\") AS " + cnst::QUERY_VAR_MEMBER_POSS + ") "
          "(GROUP_CONCAT(?memberRole; separator=\";\") AS " + cnst::QUERY_VAR_MEMBER_ROLES + ") ";
    oss << getFromClauseOptional();
    oss << "WHERE { ";
    oss << getValuesClause(cnst::NAMESPACE_OSM_REL, relIds);
    oss << getTripleClause(cnst::QUERY_VAR_VAL, cnst::PREFIXED_REL_MEMBER, "?member");
    oss << getTripleClause("?member", cnst::PREFIXED_REL_MEMBER_ID, "?memberUri");
    oss << getTripleClause("?member", cnst::PREFIXED_REL_MEMBER_POS, "?memberPos");
    oss << getTripleClause("?member", cnst::PREFIXED_REL_MEMBER_ROLE, "?memberRole");
    oss << "} GROUP BY " + cnst::QUERY_VAR_VAL;
    return oss.str();
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeQueryForReferencedNodes(const std::set<id_t> &wayIds) const {
    std::ostringstream oss;
    oss << "SELECT ?node ";
    oss << getFromClauseOptional();
    oss << "WHERE { ";
    oss << getValuesClause(cnst::NAMESPACE_OSM_WAY, wayIds);
    oss << getTripleClause(cnst::QUERY_VAR_VAL, cnst::PREFIXED_WAY_MEMBER, cnst::QUERY_VAR_MEMBER);
    oss << getTripleClause(cnst::QUERY_VAR_MEMBER, cnst::PREFIXED_WAY_MEMBER_ID, "?node");
    oss << "} GROUP BY ?node";
    return oss.str();
}

// _________________________________________________________________________________________________
std::string olu::sparql::QueryWriter::writeQueryForRelationMemberIds(const std::set<id_t> &relIds) const {
    std::ostringstream oss;
    oss << "SELECT ?p ";
    oss << getFromClauseOptional();
    oss << "WHERE { ";
    oss << getValuesClause(cnst::NAMESPACE_OSM_REL, relIds);
    oss << getTripleClause(cnst::QUERY_VAR_VAL, cnst::PREFIXED_REL_MEMBER, "?o");
    oss << getTripleClause("?o", cnst::PREFIXED_REL_MEMBER_ID, "?p");
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
    oss << getValuesClause(cnst::NAMESPACE_OSM_NODE, nodeIds);
    oss << getTripleClause("?s", cnst::PREFIXED_WAY_MEMBER_ID, cnst::QUERY_VAR_VAL);
    oss << getTripleClause("?way", cnst::PREFIXED_WAY_MEMBER, "?s");
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
    oss << getValuesClause(cnst::NAMESPACE_OSM_NODE, nodeIds);
    oss << getTripleClause("?s", cnst::PREFIXED_REL_MEMBER, "?o");
    oss << getTripleClause("?o", cnst::PREFIXED_REL_MEMBER_ID, cnst::QUERY_VAR_VAL);
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
    oss << getValuesClause(cnst::NAMESPACE_OSM_WAY, wayIds);
    oss << getTripleClause("?s", cnst::PREFIXED_REL_MEMBER, "?o");
    oss << getTripleClause("?o", cnst::PREFIXED_REL_MEMBER_ID, cnst::QUERY_VAR_VAL);
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
    oss << getValuesClause(cnst::NAMESPACE_OSM_REL, relationIds);
    oss << getTripleClause("?s", cnst::PREFIXED_REL_MEMBER, "?o");
    oss << getTripleClause("?o", cnst::PREFIXED_REL_MEMBER_ID, cnst::QUERY_VAR_VAL);
    oss << "} ";
    oss << "GROUP BY ?s";
    return oss.str();
}

std::string olu::sparql::QueryWriter::writeQueryForTagsAndMetaInfo(const std::string &subject) const {
    std::ostringstream oss;
    oss << "SELECT ";
    oss << cnst::QUERY_VAR_KEY + " ";
    oss << cnst::QUERY_VAR_VAL + " ";
    oss << cnst::QUERY_VAR_TIMESTAMP + " ";
    oss << cnst::QUERY_VAR_VERSION + " ";
    oss << cnst::QUERY_VAR_CHANGESET + " ";
    oss << getFromClauseOptional();
    oss << "WHERE { { ";
    oss << getTripleClause(subject, cnst::QUERY_VAR_KEY, cnst::QUERY_VAR_VAL);
    oss << "FILTER REGEX(STR(" + cnst::QUERY_VAR_KEY + "), STR(" + cnst::NAMESPACE_OSM_KEY + ":)) } ";
    oss << wrapWithUnion(getTripleClause(subject, cnst::PREFIXED_OSM_META_TIMESTAMP, cnst::QUERY_VAR_TIMESTAMP));
    oss << wrapWithUnion(getTripleClause(subject, cnst::PREFIXED_OSM_META_VERSION, cnst::QUERY_VAR_VERSION));
    oss << wrapWithUnion(getTripleClause(subject, cnst::PREFIXED_OSM_META_CHANGESET, cnst::QUERY_VAR_CHANGESET));
    oss << " }";

    return oss.str();
}

std::string olu::sparql::QueryWriter::getFromClauseOptional() const {
    return  _config.graphUri.empty() ? "" : "FROM <" +_config.graphUri + "> ";
}

std::string olu::sparql::QueryWriter::getValuesClause(const std::string &osmTag,
                                                      const std::set<id_t> &objectIds) {
    return getValuesClause(osmTag, ":", objectIds);
}

std::string olu::sparql::QueryWriter::getValuesClause(const std::string &osmTag,
                                                      const std::string &delimiter,
                                                      const std::set<id_t> &objectIds) {
    std::ostringstream oss;
    oss << "VALUES " + cnst::QUERY_VAR_VAL +" { ";
    for (const auto & objectId : objectIds) {
        oss << osmTag;
        oss << delimiter;
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

std::string olu::sparql::QueryWriter::wrapWithOptional(const std::string& clause) {
    return "OPTIONAL { " + clause + " } ";
}

std::string olu::sparql::QueryWriter::getTripleClause(const std::string& subject,
                                                      const std::string& predicate,
                                                      const std::string& object) {
    return subject + " " + predicate + " " + object + " . ";
}
