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

namespace olu::config::constants {
    const static inline std::string OSM_NODE_BASE_URL =
        "https://www.openstreetmap.org/api/0.6/node";

    // HTML
    const static inline std::string HTML_KEY_CONTENT_TYPE = "Content-Type";
    const static inline std::string HTML_VALUE_CONTENT_TYPE_SPARQL_QUERY =
            "application/sparql-query";

    const static inline std::string HTML_KEY_ACCEPT = "Accept";
    const static inline std::string HTML_VALUE_ACCEPT_SPARQL_RESULT_XML =
            "application/sparql-results+xml";

    // File extensions
    const static inline std::string OSM_FILE_EXTENSION = ".osm";
    const static inline std::string OSM_CHANGE_FILE_EXTENSION = ".osc";
    const static inline std::string GZIP_EXTENSION = ".gz";
    const static inline std::string BZIP2_EXTENSION = ".bz2";
    const static inline std::string RDF_TURTLE_EXTENSION = ".ttl";
    const static inline std::string TXT_EXTENSION = ".txt";

    // File names
    const static inline std::string OSM_DIFF_STATE_FILE = "state";
    const static inline std::string OSM_2_RDF_INPUT_FILE = "tmp_input";
    const static inline std::string OSM_2_RDF_OUTPUT_FILE = "tmp_output";

    const static inline std::string DIFF_CACHE_FILE = "cache_for_seq_";

    // Exception Messages
    const static inline char *const EXCEPTION_MSG_SEQUENCE_NUMBER_IS_INVALID =
            "Sequence number is invalid.";

    // Osm2rdf
    const static inline std::string PATH_TO_INPUT_FILE = "/input/input.osm";
    const static inline std::string PATH_TO_OUTPUT_FILE = "/output/output.ttl";
    const static inline std::string PATH_TO_SCRATCH_DIRECTORY = "/scratch/";

    // OsmChangeHandler
    const static inline std::string XML_ATTRIBUTE_TAG = "<xmlattr>";
    const static inline std::string NODE_TAG = "node";
    const static inline std::string NODE_REFERENCE_TAG = "nd";
    const static inline std::string WAY_TAG = "way";
    const static inline std::string RELATION_TAG = "relation";
    const static inline std::string OSM_TAG = "osm";
    const static inline std::string OSM_CHANGE_TAG = "osmChange";

    const static inline std::string MODIFY_TAG = "modify";
    const static inline std::string DELETE_TAG = "delete";
    const static inline std::string CREATE_TAG = "create";

    const static inline std::string OSM_XML_NODE_START = "<osm version=\"0.6\">";
    const static inline std::string OSM_XML_NODE_END = "</osm>";

    const static inline std::string NODE_SUBJECT = "osmnode";
    const static inline std::string WAY_SUBJECT = "osmway";
    const static inline std::string RELATION_SUBJECT = "osmrel";

    const static inline std::string LOCATION_AS_WKT_PREDICATE = "geo:hasGeometry/geo:asWKT";
    const static inline std::string RDF_TYPE_PREDICATE = "rdf:type";
    const static inline std::string OSM_META_TIMESTAMP_PREDICATE = "osmmeta:timestamp";

    const static inline std::string OSM_NODE_OBJECT = "osm:node";

    const static inline std::string NODE_REFERENCE_ATTRIBUTE = XML_ATTRIBUTE_TAG + "." + "ref";
    const static inline std::string ID_ATTRIBUTE = XML_ATTRIBUTE_TAG + "." + "id";

    const static inline std::string ATTRIBUTE_PATH_FOR_NODE_ID =
            OSM_TAG + "." + NODE_TAG + "." + ID_ATTRIBUTE;
    const static inline std::string ATTRIBUTE_PATH_FOR_WAY_ID =
            OSM_TAG + "." + WAY_TAG + "." + ID_ATTRIBUTE;
    const static inline std::string ATTRIBUTE_PATH_FOR_RELATION_ID =
            OSM_TAG + "." + RELATION_TAG + "." + ID_ATTRIBUTE;

    // SPARQL
    const static inline std::string DEFAULT_PREFIXES =
        "PREFIX ohmnode: <https://www.openhistoricalmap.org/node/>\n"
        "PREFIX osmrel: <https://www.openstreetmap.org/relation/>\n"
        "PREFIX osmnode: <https://www.openstreetmap.org/node/>\n"
        "PREFIX osmkey: <https://www.openstreetmap.org/wiki/Key:>\n"
        "PREFIX osmway: <https://www.openstreetmap.org/way/>\n"
        "PREFIX osmmeta: <https://www.openstreetmap.org/meta/>\n"
        "PREFIX osm: <https://www.openstreetmap.org/>\n"
        "PREFIX osm2rdfmeta: <https://osm2rdf.cs.uni-freiburg.de/rdf/meta#>\n"
        "PREFIX ohmrel: <https://www.openhistoricalmap.org/relation/>\n"
        "PREFIX osmt: <https://www.openstreetmap.org/wiki/Key:>\n"
        "PREFIX osm2rdfmember: <https://osm2rdf.cs.uni-freiburg.de/rdf/member#>\n"
        "PREFIX osm2rdfkey: <https://osm2rdf.cs.uni-freiburg.de/rdf/key#>\n"
        "PREFIX osm2rdfgeom: <https://osm2rdf.cs.uni-freiburg.de/rdf/geom#>\n"
        "PREFIX ohmway: <https://www.openhistoricalmap.org/way/>\n"
        "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>\n"
        "PREFIX xsd: <http://www.w3.org/2001/XMLSchema#>\n"
        "PREFIX ohm: <https://www.openhistoricalmap.org/>\n"
        "PREFIX wd: <http://www.wikidata.org/entity/>\n"
        "PREFIX osm2rdf: <https://osm2rdf.cs.uni-freiburg.de/rdf#>\n"
        "PREFIX ogc: <http://www.opengis.net/rdf#>\n"
        "PREFIX geo: <http://www.opengis.net/ont/geosparql#>\n";

    const static inline std::vector<std::string> PREFIXES_FOR_NODE_LOCATION{
            "PREFIX osmnode: <https://www.openstreetmap.org/node/>",
            "PREFIX geo: <http://www.opengis.net/ont/geosparql#>"
    };

    const static inline std::vector<std::string> PREFIXES_FOR_LATEST_NODE_TIMESTAMP {
            "PREFIX osmmeta: <https://www.openstreetmap.org/meta/>",
            "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>",
            "PREFIX osm: <https://www.openstreetmap.org/>"
    };

    const static inline std::vector<std::string> PREFIXES_FOR_DELETE_QUERY {
        "PREFIX osmrel: <https://www.openstreetmap.org/relation/>",
        "PREFIX osmnode: <https://www.openstreetmap.org/node/>",
        "PREFIX osmway: <https://www.openstreetmap.org/way/>"
    };

    // Qlever
    const static inline std::string PATH_TO_SPARQL_RESULT =
            "sparql.results.result.binding.literal";


} // namespace olu::config::constants

#endif //OSM_LIVE_UPDATES_CONSTANTS_H
