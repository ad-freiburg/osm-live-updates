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

#ifndef OSM_LIVE_UPDATES_CONSTANTS_H
#define OSM_LIVE_UPDATES_CONSTANTS_H

#include <string>
#include <vector>
#include <sstream>

#include "Config.h"

namespace olu::config::constants {
    // Exception Messages --------------------------------------------------------------------------
    const static inline auto EXCEPTION_MSG_SEQUENCE_NUMBER_IS_INVALID = "Sequence number is invalid.";

    // HTML ----------------------------------------------------------------------------------------
    const static inline std::string HTML_KEY_CONTENT_TYPE = "Content-Type";
    const static inline std::string HTML_VALUE_CONTENT_TYPE = "application/x-www-form-urlencoded";
    const static inline std::string HTML_VALUE_CONTENT_TYPE_TURTLE = "text/turtle";

    const static inline std::string HTML_KEY_AUTHORIZATION = "Authorization";
    const static inline std::string HTML_KEY_ACCEPT = "Accept";
    const static inline std::string HTML_VALUE_ACCEPT_SPARQL_RESULT_JSON =
            "application/sparql-results+json";
    const static inline std::string HTML_VALUE_ACCEPT_QLEVER_RESULT_JSON =
            "application/qlever-results+json";

    // File extensions -----------------------------------------------------------------------------
    const static inline std::string OSM_CHANGE_FILE_EXTENSION = ".osc";
    const static inline std::string GZIP_EXTENSION = ".gz";
    const static inline std::string OSM_EXTENSION = ".osm";
    const static inline std::string TURTLE_FILE_EXTENSION = ".ttl";
    const static inline std::string TEXT_FILE_EXTENSION = ".txt";

    // Directory paths -----------------------------------------------------------------------------
    const static inline std::string PATH_TO_TEMP_DIR = "tmp/";
    const static inline std::string PATH_TO_DUMMY_DIR = PATH_TO_TEMP_DIR  + "dummy/";
    const static inline std::string PATH_TO_CHANGE_FILE_DIR = PATH_TO_TEMP_DIR + "changes/";
    const static inline std::string PATH_TO_SCRATCH_DIRECTORY = "osm2rdfScratch/";

    // File paths ----------------------------------------------------------------------------------
    const static inline std::string PATH_TO_CHANGE_FILE = PATH_TO_TEMP_DIR + "changes" + OSM_CHANGE_FILE_EXTENSION + GZIP_EXTENSION;
    const static inline std::string PATH_TO_INPUT_FILE = PATH_TO_TEMP_DIR + "input" + OSM_EXTENSION;
    const static inline std::string PATH_TO_OUTPUT_FILE = PATH_TO_TEMP_DIR + "output" + TURTLE_FILE_EXTENSION;
    const static inline std::string PATH_TO_OSM2RDF_INFO_OUTPUT_FILE = PATH_TO_TEMP_DIR + "osm2rdf_info" + TEXT_FILE_EXTENSION;
    const static inline std::string PATH_TO_OSM2RDF_INFO_OUTPUT_FILE_DEBUG = "osm2rdf_info" + TEXT_FILE_EXTENSION;
    const static inline std::string PATH_TO_STATE_FILE = "state" + TEXT_FILE_EXTENSION;

    // XML -----------------------------------------------------------------------------------------
    const static inline std::string XML_TAG_ATTR = "<xmlattr>";
    const static inline std::string XML_TAG_NODE = "node";
    const static inline std::string XML_TAG_NODE_REF = "nd";
    const static inline std::string XML_TAG_WAY = "way";
    const static inline std::string XML_TAG_REL = "relation";
    const static inline std::string XML_TAG_OSM = "osm";
    const static inline std::string XML_TAG_OSM_CHANGE = "osmChange";
    const static inline std::string XML_TAG_REF = "ref";
    const static inline std::string XML_TAG_MEMBER = "member";
    const static inline std::string XML_TAG_TYPE = "type";
    const static inline std::string XML_TAG_ROLE = "role";
    const static inline std::string XML_TAG_ID = "id";
    const static inline std::string XML_TAG_LAT = "lat";
    const static inline std::string XML_TAG_LON = "lon";
    const static inline std::string XML_TAG_MODIFY = "modify";
    const static inline std::string XML_TAG_DELETE = "delete";
    const static inline std::string XML_TAG_CREATE = "create";
    const static inline std::string XML_TAG_SPARQL = "sparql";
    const static inline std::string KEY_RESULTS = "results";
    const static inline std::string KEY_BINDINGS = "bindings";
    const static inline std::string KEY_QLEVER_SELECTED = "selected";
    const static inline std::string KEY_QLEVER_RESULTS = "res";
    const static inline std::string KEY_QLEVER_TIME = "time";
    const static inline std::string KEY_QLEVER_TOTAL = "total";
    const static inline std::string KEY_QLEVER_COMPUTE_RESULT = "computeResult";
    const static inline std::string KEY_QLEVER_DELTA_TRIPLES = "delta-triples";
    const static inline std::string KEY_QLEVER_DIFFERENCE = "difference";
    const static inline std::string KEY_QLEVER_DELETED = "deleted";
    const static inline std::string KEY_QLEVER_INSERTED = "inserted";
    const static inline std::string KEY_VALUE = "value";

    const static inline std::string XML_TAG_NAME = "name";
    const static inline std::string KEY_URI = "uri";
    const static inline std::string XML_TAG_LITERAL = "literal";
    const static inline std::string XML_TAG_BINDING = "binding";
    const static inline std::string XML_TAG_KEY = "k";
    const static inline std::string XML_TAG_VALUE = "v";
    const static inline std::string XML_TAG_TAG = "tag";

    static std::string MakeXMLPath(const std::vector<std::string> &segments) {
        if (segments.empty()) {
            return {};
        }

        std::ostringstream oss;
        auto it = segments.begin();
        oss << *it++;

        for (; it != segments.end(); ++it) {
            oss << '.' << *it;
        }

        return oss.str();
    }
    const static inline std::string XML_PATH_ATTR_NODE_REF = MakeXMLPath({XML_TAG_ATTR, XML_TAG_REF});
    const static inline std::string XML_PATH_ATTR_ID = MakeXMLPath({XML_TAG_ATTR, XML_TAG_ID});
    const static inline std::string XML_PATH_ATTR_LAT = MakeXMLPath({XML_TAG_ATTR, XML_TAG_LAT});
    const static inline std::string XML_PATH_ATTR_LON = MakeXMLPath({XML_TAG_ATTR, XML_TAG_LON});
    const static inline std::string XML_PATH_ATTR_TYPE = MakeXMLPath({XML_TAG_ATTR, XML_TAG_TYPE});
    const static inline std::string XML_PATH_ATTR_ROLE = MakeXMLPath({XML_TAG_ATTR, XML_TAG_ROLE});
    const static inline std::string XML_PATH_ATTR_KEY = MakeXMLPath({XML_TAG_ATTR, XML_TAG_KEY});
    const static inline std::string XML_PATH_ATTR_VALUE = MakeXMLPath({XML_TAG_ATTR, XML_TAG_VALUE});
    const static inline std::string XML_PATH_ATTR_NAME = MakeXMLPath({XML_TAG_ATTR, XML_TAG_NAME});
    const static inline std::string XML_PATH_SPARQL_RESULTS = MakeXMLPath({XML_TAG_SPARQL, KEY_RESULTS});
    const static inline std::string XML_PATH_BINDING_URI = MakeXMLPath({XML_TAG_BINDING, KEY_URI});

    // OSM2RDF -------------------------------------------------------------------------------------
    /// Namespaces and prefixes --------------------------------------------------------------------
    static std::string MakePrefixDecl(const std::string &prefix, const std::string &iri) {
            return "PREFIX " + prefix + ": <" + iri + ">";
    }

    const static inline std::string NAMESPACE_OSM = "osm";
    const static inline std::string NAMESPACE_IRI_OSM = "https://www.openstreetmap.org/";
    const static inline std::string PREFIX_DECL_OSM = MakePrefixDecl(NAMESPACE_OSM, NAMESPACE_IRI_OSM);

    const static inline std::string NAMESPACE_OSM_NODE = "osmnode";
    const static inline std::string NAMESPACE_IRI_OSM_NODE = "https://www.openstreetmap.org/node/";
    const static inline std::string PREFIX_DECL_OSM_NODE = MakePrefixDecl(NAMESPACE_OSM_NODE, NAMESPACE_IRI_OSM_NODE);

    const static inline std::string NAMESPACE_OSM_WAY = "osmway";
    const static inline std::string NAMESPACE_IRI_OSM_WAY = "https://www.openstreetmap.org/way/";
    const static inline std::string PREFIX_DECL_OSM_WAY = MakePrefixDecl(NAMESPACE_OSM_WAY, NAMESPACE_IRI_OSM_WAY);

    const static inline std::string NAMESPACE_OSM_REL = "osmrel";
    const static inline std::string NAMESPACE_IRI_OSM_REL = "https://www.openstreetmap.org/relation/";
    const static inline std::string PREFIX_DECL_OSM_REL = MakePrefixDecl(NAMESPACE_OSM_REL, NAMESPACE_IRI_OSM_REL);

    const static inline std::string NAMESPACE_OSM_KEY = "osmkey";
    const static inline std::string NAMESPACE_IRI_OSM_KEY = "https://www.openstreetmap.org/wiki/Key:";
    const static inline std::string PREFIX_DECL_OSM_KEY = MakePrefixDecl(NAMESPACE_OSM_KEY, NAMESPACE_IRI_OSM_KEY);

    const static inline std::string NAMESPACE_OSM_META = "osmmeta";
    const static inline std::string NAMESPACE_IRI_OSM_META = "https://www.openstreetmap.org/meta/";
    const static inline std::string PREFIX_DECL_OSM_META = MakePrefixDecl(NAMESPACE_OSM_META, NAMESPACE_IRI_OSM_META);

    const static inline std::string NAMESPACE_OSM2RDF = "osm2rdf";
    const static inline std::string NAMESPACE_IRI_OSM2RDF = "https://osm2rdf.cs.uni-freiburg.de/rdf#";
    const static inline std::string PREFIX_DECL_OSM2RDF = MakePrefixDecl(NAMESPACE_OSM2RDF, NAMESPACE_IRI_OSM2RDF);

    const static inline std::string NAMESPACE_OSM2RDF_META = "osm2rdfmeta";
    const static inline std::string NAMESPACE_IRI_OSM2RDF_META = "https://osm2rdf.cs.uni-freiburg.de/rdf/meta#";
    const static inline std::string PREFIX_DECL_OSM2RDF_META = MakePrefixDecl(NAMESPACE_OSM2RDF_META, NAMESPACE_IRI_OSM2RDF_META);

    const static inline std::string NAMESPACE_OSM2RDF_MEMBER = "osm2rdfmember";
    const static inline std::string NAMESPACE_IRI_OSM2RDF_MEMBER = "https://osm2rdf.cs.uni-freiburg.de/rdf/member#";
    const static inline std::string PREFIX_DECL_OSM2RDF_MEMBER = MakePrefixDecl(NAMESPACE_OSM2RDF_MEMBER, NAMESPACE_IRI_OSM2RDF_MEMBER);

    const static inline std::string NAMESPACE_OSM2RDF_KEY = "osm2rdfkey";
    const static inline std::string NAMESPACE_IRI_OSM2RDF_KEY = "https://osm2rdf.cs.uni-freiburg.de/rdf/key#";
    const static inline std::string PREFIX_DECL_OSM2RDF_KEY = MakePrefixDecl(NAMESPACE_OSM2RDF_KEY, NAMESPACE_IRI_OSM2RDF_KEY);

    const static inline std::string NAMESPACE_OSM2RDF_GEOM = "osm2rdfgeom";
    const static inline std::string NAMESPACE_IRI_OSM2RDF_GEOM = "https://osm2rdf.cs.uni-freiburg.de/rdf/geom#";
    const static inline std::string PREFIX_DECL_OSM2RDF_GEOM = MakePrefixDecl(NAMESPACE_OSM2RDF_GEOM, NAMESPACE_IRI_OSM2RDF_GEOM);

    const static inline std::string NAMESPACE_OSM2RDF_GENID = "genid";
    const static inline std::string NAMESPACE_IRI_OSM2RDF_GENID = "http://osm2rdf.cs.uni-freiburg.de/.well-known/genid/";
    const static inline std::string PREFIX_DECL_OSM2RDF_GENID = MakePrefixDecl(NAMESPACE_OSM2RDF_GENID, NAMESPACE_IRI_OSM2RDF_GENID);

    const static inline std::string NAMESPACE_RDF = "rdf";
    const static inline std::string NAMESPACE_IRI_RDF = "http://www.w3.org/1999/02/22-rdf-syntax-ns#";
    const static inline std::string PREFIX_DECL_RDF = MakePrefixDecl(NAMESPACE_RDF, NAMESPACE_IRI_RDF);

    const static inline std::string NAMESPACE_XSD = "xsd";
    const static inline std::string NAMESPACE_IRI_XSD = "http://www.w3.org/2001/XMLSchema#";
    const static inline std::string PREFIX_DECL_XSD = MakePrefixDecl(NAMESPACE_XSD, NAMESPACE_IRI_XSD);

    const static inline std::string NAMESPACE_WD = "wd";
    const static inline std::string NAMESPACE_IRI_WD = "http://www.wikidata.org/entity/";
    const static inline std::string PREFIX_DECL_WD = MakePrefixDecl(NAMESPACE_WD, NAMESPACE_IRI_WD);

    const static inline std::string NAMESPACE_OGC = "ogc";
    const static inline std::string NAMESPACE_IRI_OGC = "http://www.opengis.net/rdf#";
    const static inline std::string PREFIX_DECL_OGC = MakePrefixDecl(NAMESPACE_OGC, NAMESPACE_IRI_OGC);

    const static inline std::string NAMESPACE_GEO = "geo";
    const static inline std::string NAMESPACE_IRI_GEO = "http://www.opengis.net/ont/geosparql#";
    const static inline std::string PREFIX_DECL_GEO = MakePrefixDecl(NAMESPACE_GEO, NAMESPACE_IRI_GEO);

    /// Names --------------------------------------------------------------------------------------
    const static inline std::string NAME_MEMBER = "member";
    const static inline std::string NAME_MEMBER_ID = "member_id";
    const static inline std::string NAME_MEMBER_POS = "member_pos";
    const static inline std::string NAME_MEMBER_ROLE = "member_role";
    const static inline std::string NAME_MEMBER_IDS = "member_ids";
    const static inline std::string NAME_MEMBER_POSS = "member_poss";
    const static inline std::string NAME_MEMBER_ROLES = "member_roles";

    const static inline std::string NAME_HAS_GEOMETRY = "hasGeometry";
    const static inline std::string NAME_HAS_CENTROID = "hasCentroid";
    const static inline std::string NAME_AS_WKT = "asWKT";

    const static inline std::string NAME_FACTS = "facts";
    const static inline std::string NAME_AREA = "area";
    const static inline std::string NAME_LENGTH = "length";
    const static inline std::string NAME_OBB = "obb";
    const static inline std::string NAME_ENVELOPE = "envelope";
    const static inline std::string NAME_CONVEX_HULL = "convex_hull";
    const static inline std::string NAME_TYPE = "type";
    const static inline std::string NAME_KEY = "key";
    const static inline std::string NAME_VALUE = "value";
    const static inline std::string NAME_URI = "uri";
    const static inline std::string NAME_LOCATION = "location";
    const static inline std::string NAME_TIMESTAMP = "timestamp";
    const static inline std::string NAME_VERSION = "version";
    const static inline std::string NAME_CHANGESET = "changeset";
    const static inline std::string NAME_OSM_NODE_ = "osm_node_";
    const static inline std::string NAME_NODE = "node";
    const static inline std::string NAME_NODES = "nodes";
    const static inline std::string NAME_WAY = "way";
    const static inline std::string NAME_REL = "rel";
    const static inline std::string NAME_INFO = "info";
    const static inline std::string NAME_OPTION = "option";
    const static inline std::string NAME_SEQUENCE_NUMBER = "sequenceNumber";
    const static inline std::string NAME_DATE_MODIFIED = "dateModified";
    const static inline std::string NAME_UPDATES_COMPLETE_UNTIL = "updatesCompleteUntil";

    /// Prefixed names -----------------------------------------------------------------------------
    static std::string MakePrefixedName(const std::string &prefix, const std::string &name) {
        return prefix + ":" + name;
    }
    const static inline std::string PREFIXED_WAY_MEMBER = MakePrefixedName(NAMESPACE_OSM_WAY, NAME_MEMBER);
    const static inline std::string PREFIXED_WAY_MEMBER_ID = MakePrefixedName(NAMESPACE_OSM_WAY, NAME_MEMBER_ID);
    const static inline std::string PREFIXED_WAY_MEMBER_POS = MakePrefixedName(NAMESPACE_OSM_WAY, NAME_MEMBER_POS);

    const static inline std::string PREFIXED_REL_MEMBER = MakePrefixedName(NAMESPACE_OSM_REL, NAME_MEMBER);
    const static inline std::string PREFIXED_REL_MEMBER_ID = MakePrefixedName(NAMESPACE_OSM_REL, NAME_MEMBER_ID);
    const static inline std::string PREFIXED_REL_MEMBER_POS = MakePrefixedName(NAMESPACE_OSM_REL, NAME_MEMBER_POS);
    const static inline std::string PREFIXED_REL_MEMBER_ROLE = MakePrefixedName(NAMESPACE_OSM_REL, NAME_MEMBER_ROLE);

    const static inline std::string PREFIXED_GEO_HAS_GEOMETRY = MakePrefixedName(NAMESPACE_GEO, NAME_HAS_GEOMETRY);
    const static inline std::string PREFIXED_GEO_HAS_CENTROID = MakePrefixedName(NAMESPACE_GEO, NAME_HAS_CENTROID);
    const static inline std::string PREFIXED_GEO_AS_WKT = MakePrefixedName(NAMESPACE_GEO, NAME_AS_WKT);

    const static inline std::string PREFIXED_OSM2RDF_FACTS = MakePrefixedName(NAMESPACE_OSM2RDF, NAME_FACTS);
    const static inline std::string PREFIXED_OSM2RDF_AREA = MakePrefixedName(NAMESPACE_OSM2RDF, NAME_AREA);
    const static inline std::string PREFIXED_OSM2RDF_LENGTH = MakePrefixedName(NAMESPACE_OSM2RDF, NAME_LENGTH);
    const static inline std::string PREFIXED_OSM2RDF_GEOM_OBB = MakePrefixedName(NAMESPACE_OSM2RDF_GEOM, NAME_OBB);
    const static inline std::string PREFIXED_OSM2RDF_GEOM_ENVELOPE = MakePrefixedName(NAMESPACE_OSM2RDF_GEOM, NAME_ENVELOPE);
    const static inline std::string PREFIXED_OSM2RDF_GEOM_CONVEX_HULL = MakePrefixedName(NAMESPACE_OSM2RDF_GEOM, NAME_CONVEX_HULL);
    const static inline std::string PREFIXED_OSM2RDF_GEOM_NODE_ = MakePrefixedName(NAMESPACE_OSM2RDF_GEOM, NAME_OSM_NODE_);

    const static inline std::string PREFIXED_OSM_NODE = MakePrefixedName(NAMESPACE_OSM, NAME_NODE);
    const static inline std::string PREFIXED_OSM_KEY_TYPE = MakePrefixedName(NAMESPACE_OSM_KEY, NAME_TYPE);
    const static inline std::string PREFIXED_OSM_META_TIMESTAMP = MakePrefixedName(NAMESPACE_OSM_META, NAME_TIMESTAMP);
    const static inline std::string PREFIXED_OSM_META_VERSION = MakePrefixedName(NAMESPACE_OSM_META, NAME_VERSION);
    const static inline std::string PREFIXED_OSM_META_CHANGESET = MakePrefixedName(NAMESPACE_OSM_META, NAME_CHANGESET);

    const static inline std::string PREFIXED_RDF_TYPE = MakePrefixedName(NAMESPACE_RDF, NAME_TYPE);

    const static inline std::string PREFIXED_OSM2RDF_META_OPTION = MakePrefixedName(NAMESPACE_OSM2RDF_META, NAME_OPTION);
    const static inline std::string PREFIXED_OSM2RDF_META_INFO = MakePrefixedName(NAMESPACE_OSM2RDF_META, NAME_INFO);
    const static inline std::string PREFIXED_OSM2RDF_META_VERSION = MakePrefixedName(NAMESPACE_OSM2RDF_META, NAME_VERSION);
    const static inline std::string PREFIXED_OSM2RDF_META_UPDATES_COMPLETE_UNTIL = MakePrefixedName(NAMESPACE_OSM2RDF_META, NAME_UPDATES_COMPLETE_UNTIL);
    const static inline std::string PREFIXED_OSM2RDF_META_DATE_MODIFIED = MakePrefixedName(NAMESPACE_OSM2RDF_META, NAME_DATE_MODIFIED);

    /// Prefix declarations ------------------------------------------------------------------------
    const static inline std::vector DEFAULT_PREFIXES {
            PREFIX_DECL_OSM, PREFIX_DECL_OSM_NODE, PREFIX_DECL_OSM_WAY, PREFIX_DECL_OSM_REL, PREFIX_DECL_OSM_KEY, PREFIX_DECL_OSM_META,
            PREFIX_DECL_OSM2RDF, PREFIX_DECL_OSM2RDF_KEY, PREFIX_DECL_OSM2RDF_GEOM, PREFIX_DECL_OSM2RDF_META, PREFIX_DECL_OSM2RDF_GENID, PREFIX_DECL_OSM2RDF_MEMBER,
            PREFIX_DECL_WD, PREFIX_DECL_GEO, PREFIX_DECL_OGC, PREFIX_DECL_RDF, PREFIX_DECL_XSD};

    const static inline std::vector PREFIXES_FOR_NODE_LOCATION {
            PREFIX_DECL_OSM_NODE, PREFIX_DECL_GEO, PREFIX_DECL_OSM2RDF_GEOM};

    const static inline std::vector PREFIXES_FOR_RELATION_MEMBERS{
            PREFIX_DECL_OSM_REL, PREFIX_DECL_RDF, PREFIX_DECL_OSM2RDF_MEMBER, PREFIX_DECL_OSM, PREFIX_DECL_OSM_KEY};

    const static inline std::vector PREFIXES_FOR_WAY_MEMBERS{
            PREFIX_DECL_OSM_WAY, PREFIX_DECL_OSM2RDF_MEMBER};

    const static inline std::vector PREFIXES_FOR_WAY_TAGS_AND_META_INFO{
            PREFIX_DECL_OSM_WAY, PREFIX_DECL_OSM_META, PREFIX_DECL_OSM_KEY};

    const static inline std::vector PREFIXES_FOR_RELATION_TAGS_AND_META_INFO{
            PREFIX_DECL_OSM_REL, PREFIX_DECL_OSM_META, PREFIX_DECL_OSM_KEY};

    const static inline std::vector PREFIXES_FOR_LATEST_NODE_TIMESTAMP {
            PREFIX_DECL_OSM_META, PREFIX_DECL_OSM, PREFIX_DECL_RDF};

    const static inline std::vector PREFIXES_FOR_NODE_DELETE_QUERY {
            PREFIX_DECL_OSM_NODE, PREFIX_DECL_OGC, PREFIX_DECL_GEO};

    const static inline std::vector PREFIXES_FOR_WAY_DELETE_QUERY {
            PREFIX_DECL_OSM_WAY, PREFIX_DECL_OGC, PREFIX_DECL_GEO};

    const static inline std::vector PREFIXES_FOR_WAY_DELETE_META_AND_TAGS_QUERY {
            PREFIX_DECL_OSM_WAY, PREFIX_DECL_OSM_META, PREFIX_DECL_OSM2RDF, PREFIX_DECL_OSM_KEY};

    const static inline std::vector PREFIXES_FOR_RELATION_DELETE_META_AND_TAGS_QUERY {
            PREFIX_DECL_OSM_REL, PREFIX_DECL_OSM_META, PREFIX_DECL_OSM2RDF, PREFIX_DECL_OSM_KEY};

    const static inline std::vector PREFIXES_FOR_WAY_DELETE_GEOMETRY_QUERY {
            PREFIX_DECL_OSM_WAY, PREFIX_DECL_GEO, PREFIX_DECL_OSM2RDF, PREFIX_DECL_OSM2RDF_GEOM};

    const static inline std::vector PREFIXES_FOR_RELATION_DELETE_GEOMETRY_QUERY {
            PREFIX_DECL_OSM_REL, PREFIX_DECL_GEO, PREFIX_DECL_OSM2RDF, PREFIX_DECL_OSM2RDF_GEOM};

    const static inline std::vector PREFIXES_FOR_RELATION_DELETE_QUERY {
            PREFIX_DECL_OSM_REL, PREFIX_DECL_OGC, PREFIX_DECL_GEO};

    const static inline std::vector PREFIXES_FOR_WAYS_REFERENCING_NODE {
            PREFIX_DECL_OSM_WAY, PREFIX_DECL_OSM_NODE};

    const static inline std::vector PREFIXES_FOR_RELATIONS_REFERENCING_NODE {
            PREFIX_DECL_OSM2RDF_MEMBER, PREFIX_DECL_OSM_REL, PREFIX_DECL_OSM_NODE};

    const static inline std::vector PREFIXES_FOR_RELATIONS_REFERENCING_WAY {
            PREFIX_DECL_OSM2RDF_MEMBER, PREFIX_DECL_OSM_REL, PREFIX_DECL_OSM_WAY};

    const static inline std::vector PREFIXES_FOR_RELATIONS_REFERENCING_RELATIONS {
            PREFIX_DECL_OSM2RDF_MEMBER, PREFIX_DECL_OSM_REL};

    const static inline std::vector PREFIXES_FOR_OSM2RDF_VERSION {
            PREFIX_DECL_OSM2RDF_META
    };

    const static inline std::vector PREFIXES_FOR_OSM2RDF_OPTIONS {
            PREFIX_DECL_OSM2RDF_META
    };

    const static inline std::vector PREFIXES_FOR_METADATA_TRIPLES {
        PREFIX_DECL_OSM2RDF_META, PREFIX_DECL_XSD
    };

    // Query variables -----------------------------------------------------------------------------
    static std::string MakeQueryVar(const std::string &name) {
            return "?" + name;
    }
    const static inline std::string QUERY_VAR_TYPE = MakeQueryVar(NAME_TYPE);
    const static inline std::string QUERY_VAR_KEY = MakeQueryVar(NAME_KEY);
    const static inline std::string QUERY_VAR_VAL = MakeQueryVar(NAME_VALUE);
    const static inline std::string QUERY_VAR_LOC = MakeQueryVar(NAME_LOCATION);
    const static inline std::string QUERY_VAR_TIMESTAMP = MakeQueryVar(NAME_TIMESTAMP);
    const static inline std::string QUERY_VAR_VERSION = MakeQueryVar(NAME_VERSION);
    const static inline std::string QUERY_VAR_CHANGESET = MakeQueryVar(NAME_CHANGESET);
    const static inline std::string QUERY_VAR_NODE = MakeQueryVar(NAME_NODE);
    const static inline std::string QUERY_VAR_WAY = MakeQueryVar(NAME_WAY);
    const static inline std::string QUERY_VAR_REL = MakeQueryVar(NAME_REL);
    const static inline std::string QUERY_VAR_MEMBER = MakeQueryVar(NAME_MEMBER);
    const static inline std::string QUERY_VAR_MEMBER_ID = MakeQueryVar(NAME_MEMBER_ID);
    const static inline std::string QUERY_VAR_MEMBER_ROLE = MakeQueryVar(NAME_MEMBER_ROLE);
    const static inline std::string QUERY_VAR_MEMBER_POS = MakeQueryVar(NAME_MEMBER_POS);
    const static inline std::string QUERY_VAR_MEMBER_IDS = MakeQueryVar(NAME_MEMBER_IDS);
    const static inline std::string QUERY_VAR_MEMBER_ROLES = MakeQueryVar(NAME_MEMBER_ROLES);
    const static inline std::string QUERY_VAR_MEMBER_POSS = MakeQueryVar(NAME_MEMBER_POSS);
    const static inline std::string QUERY_VAR_OPTION = MakeQueryVar(NAME_OPTION);
    const static inline std::string QUERY_VAR_SEQUENCE_NUMBER = MakeQueryVar(NAME_SEQUENCE_NUMBER);


    // Triple patterns --------------------------------------------------------------------------
    const static inline std::string IRI_XSD_DATE_TIME = "<" + NAMESPACE_IRI_XSD + "dateTime>";
    const static inline std::string IRI_XSD_INT = "<" + NAMESPACE_IRI_XSD + "integer>";

        // Options and output --------------------------------------------------------------------------
    const static inline std::string HELP_OPTION_SHORT = "h";
    const static inline std::string HELP_OPTION_LONG = "help";
    const static inline std::string HELP_OPTION_HELP = "Display help information.";

    const static inline std::string SPARQL_ENDPOINT_URI_INFO = "SPARQL endpoint URI:";

    const static inline std::string SPARQL_GRAPH_URI_INFO = "SPARQL graph URI:";
    const static inline std::string SPARQL_GRAPH_URI_OPTION_SHORT = "g";
    const static inline std::string SPARQL_GRAPH_URI_OPTION_LONG = "graph";
    const static inline std::string SPARQL_GRAPH_URI_OPTION_HELP =
           "The URI of the graph that you want to update.";

    const static inline std::string SPARQL_ACCESS_TOKEN_INFO = "Access token for SPARQL endpoint:";
    const static inline std::string SPARQL_ACCESS_TOKEN_OPTION_SHORT = "a";
    const static inline std::string SPARQL_ACCESS_TOKEN_OPTION_LONG = "access-token";
    const static inline std::string SPARQL_ACCESS_TOKEN_OPTION_HELP =
          "The access token for the SPARQL endpoint";

    const static inline std::string SPARQL_UPDATE_PATH_INFO = "SPARQL endpoint URI for updates:";
    const static inline std::string SPARQL_UPDATE_PATH_OPTION_SHORT = "u";
    const static inline std::string SPARQL_UPDATE_PATH_OPTION_LONG = "endpoint-uri-updates";
    const static inline std::string SPARQL_UPDATE_PATH_OPTION_HELP =
         "Specify a different URI for SPARQL updates.";

    const static inline std::string SPARQL_OUTPUT_INFO = "SPARQL update output is set to file:";
    const static inline std::string SPARQL_OUTPUT_OPTION_SHORT = "o";
    const static inline std::string SPARQL_OUTPUT_OPTION_LONG = "sparql-output";
    const static inline std::string SPARQL_OUTPUT_OPTION_HELP =
        "Specify if SPARQL updates should be written to a file instead of sending them to the endpoint.";

    const static inline std::string SPARQL_OUTPUT_FORMAT_INFO = "Output format:";
    const static inline std::string SPARQL_OUTPUT_FORMAT_OPTION_SHORT = "d";
    const static inline std::string SPARQL_OUTPUT_FORMAT_OPTION_LONG = "debug";
    const static inline std::string SPARQL_OUTPUT_FORMAT_OPTION_HELP =
        "If set, all SPARQL queries are written to the output file.";

    const static inline std::string PATH_TO_INPUT_INFO = "Input:";
    const static inline std::string PATH_TO_INPUT_OPTION_SHORT = "i";
        const static inline std::string PATH_TO_INPUT_OPTION_LONG = "input";
    const static inline std::string PATH_TO_INPUT_OPTION_HELP =
            "The path to the directory with the OsmChange files.";

    const static inline std::string OSM_CHANGE_FILE_DIRECTORY_URI_INFO =
            "URI of OsmChange file server:";
    const static inline std::string OSM_CHANGE_FILE_SERVER_URI_OPTION_SHORT = "f";
    const static inline std::string OSM_CHANGE_FILE_SERVER_URI_OPTION_LONG = "file-server";
    const static inline std::string OSM_CHANGE_FILE_SERVER_URI_OPTION_HELP =
            "The URI of the server with the OsmChange files.";

    const static inline std::string SEQUENCE_NUMBER_INFO = "Starting sequence number:";
    const static inline std::string SEQUENCE_NUMBER_OPTION_SHORT = "s";
    const static inline std::string SEQUENCE_NUMBER_OPTION_LONG = "sequence-number";
    const static inline std::string SEQUENCE_NUMBER_OPTION_HELP =
            "The sequence number to start the update process from.";

    const static inline std::string TIME_STAMP_INFO = "Starting timestamp:";
    const static inline std::string TIME_STAMP_OPTION_SHORT = "t";
    const static inline std::string TIME_STAMP_OPTION_LONG = "timestamp";
    const static inline std::string TIME_STAMP_OPTION_HELP =
            "The time stamp to start the update process from.";

    const static inline std::string BATCH_SIZE_INFO = "Number of values per batch:";
    const static inline std::string BATCH_SIZE_OPTION_SHORT = "";
    const static inline std::string BATCH_SIZE_OPTION_LONG = "batch-size";
    const static inline std::string BATCH_SIZE_OPTION_HELP =
        "The number of values or triples that should be sent in one batch to the SPARQL endpoint. "
        "Default is " + std::to_string(Config::DEFAULT_BATCH_SIZE) + ".";

    const static inline std::string QLEVER_ENDPOINT_INFO = "Working with a QLever endpoint";
    const static inline std::string QLEVER_ENDPOINT_OPTION_SHORT = "";
    const static inline std::string QLEVER_ENDPOINT_OPTION_LONG = "qlever";
    const static inline std::string QLEVER_ENDPOINT_OPTION_HELP =
        "Specify if the SPARQL endpoint is QLever. More metadata will be added to the output.";

    const static inline std::string STATISTICS_INFO = "";
    const static inline std::string STATISTICS_OPTION_SHORT = "";
    const static inline std::string STATISTICS_OPTION_LONG = "statistics";
    const static inline std::string STATISTICS_OPTION_HELP =
        "Specify if detailed statistics should be added to the output.";

} // namespace olu::config::constants

#endif //OSM_LIVE_UPDATES_CONSTANTS_H
