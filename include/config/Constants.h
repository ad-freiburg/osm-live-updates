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
    // Exception Messages
    const static inline char *const EXCEPTION_MSG_SEQUENCE_NUMBER_IS_INVALID =
        "Sequence number is invalid.";

    // HTML
    const static inline std::string HTML_KEY_CONTENT_TYPE = "Content-Type";
    const static inline std::string HTML_VALUE_CONTENT_TYPE = "application/x-www-form-urlencoded";

    const static inline std::string HTML_KEY_ACCEPT = "Accept";
    const static inline std::string HTML_VALUE_ACCEPT_SPARQL_RESULT_XML =
            "application/sparql-results+xml";
    const static inline std::string HTML_VALUE_ACCEPT_SPARQL_RESULT_JSON =
            "application/sparql-results+json";

    // File extensions
    const static inline std::string OSM_CHANGE_FILE_EXTENSION = ".osc";
    const static inline std::string GZIP_EXTENSION = ".gz";

    // File names
    const static inline std::string PATH_TO_TEMP_DIR = "tmp/";
    const static inline std::string PATH_TO_CHANGE_FILE_DIR = PATH_TO_TEMP_DIR + "changes/";
    const static inline std::string CHANGE_FILE_NAME = "changes.osc.gz";
    const static inline std::string PATH_TO_CHANGE_FILE = PATH_TO_TEMP_DIR + CHANGE_FILE_NAME;
    const static inline std::string PATH_TO_NODE_FILE = PATH_TO_TEMP_DIR + "nodes.osm";
    const static inline std::string PATH_TO_WAY_FILE = PATH_TO_TEMP_DIR + "ways.osm";
    const static inline std::string PATH_TO_RELATION_FILE = PATH_TO_TEMP_DIR + "relations.osm";

    // Osm2rdf
    const static inline std::string PATH_TO_INPUT_FILE = PATH_TO_TEMP_DIR + "input.osm";
    const static inline std::string PATH_TO_OUTPUT_FILE = PATH_TO_TEMP_DIR + "output.ttl";
    const static inline std::string PATH_TO_SCRATCH_DIRECTORY = "osm2rdfScratch/";

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

    const static inline std::string NODE_REFERENCE_ATTRIBUTE = XML_ATTRIBUTE_TAG + "." + "ref";
    const static inline std::string ID_ATTRIBUTE = XML_ATTRIBUTE_TAG + "." + "id";

    // SPARQL
    const static inline std::string OSM_WAY_URI = "https://www.openstreetmap.org/way/";
    const static inline std::string OSM_NODE_URI = "https://www.openstreetmap.org/node/";
    const static inline std::string OSM_REL_URI = "https://www.openstreetmap.org/relation/";
    const static inline std::string OSM_GEOM_NODE_URI = "https://osm2rdf.cs.uni-freiburg.de/rdf/geom#osm_node_";
    const static inline std::string OSM_TAG_KEY = "https://www.openstreetmap.org/wiki/Key:";

    const static inline std::vector<std::string> DEFAULT_PREFIXES{
        "PREFIX ohmnode: <https://www.openhistoricalmap.org/node/>"
        "PREFIX osmrel: <https://www.openstreetmap.org/relation/>"
        "PREFIX osmnode: <https://www.openstreetmap.org/node/>"
        "PREFIX osmkey: <https://www.openstreetmap.org/wiki/Key:>"
        "PREFIX osmway: <https://www.openstreetmap.org/way/>"
        "PREFIX osmmeta: <https://www.openstreetmap.org/meta/>"
        "PREFIX osm: <https://www.openstreetmap.org/>"
        "PREFIX osm2rdfmeta: <https://osm2rdf.cs.uni-freiburg.de/rdf/meta#>"
        "PREFIX ohmrel: <https://www.openhistoricalmap.org/relation/>"
        "PREFIX osmt: <https://www.openstreetmap.org/wiki/Key:>"
        "PREFIX osm2rdfmember: <https://osm2rdf.cs.uni-freiburg.de/rdf/member#>"
        "PREFIX osm2rdfkey: <https://osm2rdf.cs.uni-freiburg.de/rdf/key#>"
        "PREFIX osm2rdfgeom: <https://osm2rdf.cs.uni-freiburg.de/rdf/geom#>"
        "PREFIX ohmway: <https://www.openhistoricalmap.org/way/>"
        "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>"
        "PREFIX xsd: <http://www.w3.org/2001/XMLSchema#>"
        "PREFIX ohm: <https://www.openhistoricalmap.org/>"
        "PREFIX wd: <http://www.wikidata.org/entity/>"
        "PREFIX osm2rdf: <https://osm2rdf.cs.uni-freiburg.de/rdf#>"
        "PREFIX ogc: <http://www.opengis.net/rdf#>"
        "PREFIX geo: <http://www.opengis.net/ont/geosparql#>"};

    const static inline std::vector<std::string> PREFIXES_FOR_NODE_LOCATION{
            "PREFIX osmnode: <https://www.openstreetmap.org/node/>",
            "PREFIX geo: <http://www.opengis.net/ont/geosparql#>",
            "PREFIX osm2rdfgeom: <https://osm2rdf.cs.uni-freiburg.de/rdf/geom#>"
    };

    const static inline std::vector<std::string> PREFIXES_FOR_RELATION_MEMBERS{
            "PREFIX osmrel: <https://www.openstreetmap.org/relation/>",
            "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>",
            "PREFIX osm2rdfmember: <https://osm2rdf.cs.uni-freiburg.de/rdf/member#>",
            "PREFIX osm: <https://www.openstreetmap.org/>",
            "PREFIX osmkey: <https://www.openstreetmap.org/wiki/Key:>"
    };

    const static inline std::vector<std::string> PREFIXES_FOR_WAY_MEMBERS{
            "PREFIX osmway: <https://www.openstreetmap.org/way/>",
            "PREFIX osm2rdfmember: <https://osm2rdf.cs.uni-freiburg.de/rdf/member#>"
    };

    const static inline std::vector<std::string> PREFIXES_FOR_WAY_TAGS{
            "PREFIX osmway: <https://www.openstreetmap.org/way/>",
            "PREFIX osmmeta: <https://www.openstreetmap.org/meta/>"
    };

    const static inline std::vector<std::string> PREFIXES_FOR_RELATION_TAGS{
            "PREFIX osmrel: <https://www.openstreetmap.org/relation/>",
            "PREFIX osmmeta: <https://www.openstreetmap.org/meta/>"
    };

    const static inline std::vector<std::string> PREFIXES_FOR_LATEST_NODE_TIMESTAMP {
            "PREFIX osmmeta: <https://www.openstreetmap.org/meta/>",
            "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>",
            "PREFIX osm: <https://www.openstreetmap.org/>"
    };

    const static inline std::vector<std::string> PREFIXES_FOR_NODE_DELETE_QUERY {
        "PREFIX osmnode: <https://www.openstreetmap.org/node/>",
        "PREFIX ogc: <http://www.opengis.net/rdf#>"
    };

    const static inline std::vector<std::string> PREFIXES_FOR_WAY_DELETE_QUERY {
            "PREFIX osmway: <https://www.openstreetmap.org/way/>",
            "PREFIX ogc: <http://www.opengis.net/rdf#>"
    };

    const static inline std::vector<std::string> PREFIXES_FOR_RELATION_DELETE_QUERY {
            "PREFIX osmrel: <https://www.openstreetmap.org/relation/>",
            "PREFIX ogc: <http://www.opengis.net/rdf#>"
    };

    const static inline std::vector<std::string> PREFIXES_FOR_WAYS_REFERENCING_NODE {
            "PREFIX osmway: <https://www.openstreetmap.org/way/>",
            "PREFIX osmnode: <https://www.openstreetmap.org/node/>"
    };

    const static inline std::vector<std::string> PREFIXES_FOR_RELATIONS_REFERENCING_NODE {
            "PREFIX osm2rdfmember: <https://osm2rdf.cs.uni-freiburg.de/rdf/member#>",
            "PREFIX osmrel: <https://www.openstreetmap.org/relation/>",
            "PREFIX osmnode: <https://www.openstreetmap.org/node/>"
    };

    const static inline std::vector<std::string> PREFIXES_FOR_RELATIONS_REFERENCING_WAY {
            "PREFIX osm2rdfmember: <https://osm2rdf.cs.uni-freiburg.de/rdf/member#>",
            "PREFIX osmrel: <https://www.openstreetmap.org/relation/>",
            "PREFIX osmway: <https://www.openstreetmap.org/way/>"
    };

    const static inline std::vector<std::string> PREFIXES_FOR_RELATIONS_REFERENCING_RELATIONS {
            "PREFIX osm2rdfmember: <https://osm2rdf.cs.uni-freiburg.de/rdf/member#>",
            "PREFIX osmrel: <https://www.openstreetmap.org/relation/>",
    };

    // Qlever
    const static inline std::string HEADER = "Configuration for OLU";

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

    const static inline std::string SPARQL_OUTPUT_INFO = "Update Output:";
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

} // namespace olu::config::constants

#endif //OSM_LIVE_UPDATES_CONSTANTS_H
