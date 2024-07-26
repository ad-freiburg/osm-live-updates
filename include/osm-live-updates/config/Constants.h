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

namespace olu::config::constants {
    // URLs
    const static inline std::string OSM_REPLICATION_BASE_URL =
        "https://planet.openstreetmap.org/replication";
    const static inline std::string OSM_NODE_BASE_URL =
        "https://www.openstreetmap.org/api/0.6/node";

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

    // Exception Messages
    const static inline char *const EXCEPTION_MSG_SEQUENCE_NUMBER_IS_EMPTY =
            "Sequence number is empty.";
    const static inline char *const EXCEPTION_MSG_SEQUENCE_NUMBER_IS_TOO_LONG =
            "Sequence number is too long, it can be up to 9 Digits.";


    // SPARQL
    const static inline std::string PREFIXES =
        "PREFIX ohmnode: <https://www.openhistoricalmap.org/node/>\n"
        "PREFIX osmrel: <https://www.openstreetmap.org/relation/>\n"
        "PREFIX osmnode: <https://www.openstreetmap.org/node/>\n"
        "PREFIX osmkey: <https://www.openstreetmap.org/wiki/Key:>\n"
        "PREFIX osmway: <https://www.openstreetmap.org/way/>\n"
        "PREFIX osmmeta: <https://www.openstreetmap.org/meta/>\n"
        "PREFIX osm: <https://www.openstreetmap.org/>\n"
        "PREFIX osm2rdfmeta: <https://osm2rdf.cs.uni-freiburg.de/rdf/meta#>\n"
        "PREFIX ohmrel: <https://www.openhistoricalmap.org/relation/>\n"
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
} // namespace olu::config::constants

#endif //OSM_LIVE_UPDATES_CONSTANTS_H
