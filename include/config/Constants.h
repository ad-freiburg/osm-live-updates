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
    const static inline std::string QLEVER_LOCAL_HOST_URI =
            "http://host.docker.internal:7007/osm-planet/";

    const static inline std::string QLEVER_OSM_PLANET_URI =
            "https://qlever.cs.uni-freiburg.de/api/osm-planet";

    const static inline std::string OSM_NODE_BASE_URL =
        "https://www.openstreetmap.org/api/0.6/node";

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

    const static inline std::string DIFF_CACHE_FILE = "cache_for_seq_";

    // Exception Messages
    const static inline char *const EXCEPTION_MSG_SEQUENCE_NUMBER_IS_INVALID =
            "Sequence number is invalid.";

    const static inline std::string PATH_TO_NODE_FILE = "nodes.osm";
    const static inline std::string PATH_TO_WAY_FILE = "ways.osm";
    const static inline std::string PATH_TO_RELATION_FILE = "relations.osm";
    const static inline std::string PATH_TO_TRIPLES_FILE = "triples.ttl";

    // Osm2rdf
    const static inline std::string PATH_TO_INPUT_FILE = "input.osm";
    const static inline std::string PATH_TO_OUTPUT_FILE = "output.ttl";
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
            "PREFIX osmway: <https://www.openstreetmap.org/way/>"
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
        "PREFIX osmnode: <https://www.openstreetmap.org/node/>"
    };

    const static inline std::vector<std::string> PREFIXES_FOR_WAY_DELETE_QUERY {
            "PREFIX osmway: <https://www.openstreetmap.org/way/>"
    };

    const static inline std::vector<std::string> PREFIXES_FOR_RELATION_DELETE_QUERY {
            "PREFIX osmrel: <https://www.openstreetmap.org/relation/>"
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
    const static inline std::string SPARQL_ENDPOINT_URI_OPTION_SHORT = "u";
    const static inline std::string SPARQL_ENDPOINT_URI_OPTION_LONG = "sparql-uri";
    const static inline std::string SPARQL_ENDPOINT_URI_OPTION_HELP =
            "The URI of the SPARQL endpoint that you want to update.";

    const static inline std::string SPARQL_UPDATE_PATH_INFO = "SPARQL update path:";
    const static inline std::string SPARQL_UPDATE_PATH_OPTION_SHORT = "p";
    const static inline std::string SPARQL_UPDATE_PATH_OPTION_LONG = "sparql-update-path";
    const static inline std::string SPARQL_UPDATE_PATH_OPTION_HELP =
            "The path for update request on the SPARQL endpoint. Empty by default.";

    const static inline std::string PATH_TO_OSM_CHANGE_FILE_INFO = "Path to osm change file:";
    const static inline std::string PATH_TO_OSM_CHANGE_FILE_OPTION_SHORT = "f";
    const static inline std::string PATH_TO_OSM_CHANGE_FILE_OPTION_LONG = "change-file-path";
    const static inline std::string PATH_TO_OSM_CHANGE_FILE_OPTION_HELP =
            "The path to the osm change file that you want to be applied to the SPARQL endpoint.";

    const static inline std::string OSM_CHANGE_FILE_DIRECTORY_URI_INFO =
            "URI of osm change file directory:";
    const static inline std::string OSM_CHANGE_FILE_DIRECTORY_URI_OPTION_SHORT = "d";
    const static inline std::string OSM_CHANGE_FILE_DIRECTORY_URI_OPTION_LONG = "change-file-dir";
    const static inline std::string OSM_CHANGE_FILE_DIRECTORY_URI_OPTION_HELP =
            "The URI of the directory where the osm change files can be found.";

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

    const static inline std::string NO_TIMESTAMP_OR_SEQUENCE_NUMBER_INFO =
            "As you have not entered a sequence number or timestamp, the programme will determine the starting point for you.";

} // namespace olu::config::constants

#endif //OSM_LIVE_UPDATES_CONSTANTS_H
