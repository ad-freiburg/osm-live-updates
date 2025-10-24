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

#ifndef OSM_LIVE_UPDATES_CONFIG_H
#define OSM_LIVE_UPDATES_CONFIG_H

#include <string>
#include <filesystem>
#include <map>
#include <thread>

namespace olu::config {

enum SparqlOutput {
    ENDPOINT = 0,
    FILE = 1,
    DEBUG_FILE = 2,
};

struct Config {
    static constexpr u_int16_t DEFAULT_WKT_PRECISION = 7;
    static constexpr u_int16_t DEFAULT_PERCENTAGE_PRECISION = 1;
    static constexpr u_int32_t DEFAULT_BATCH_SIZE = 1 << 18;

    // The uri of the SPARQL endpoint for queries
    std::string sparqlEndpointUri;
    // The uri to the SPARQL endpoint to update. If not specified by user, this will be the same as
    // the endpoint for queries
    std::string sparqlEndpointUriForUpdates;

    // User specified local directory for change files.
    std::string changeFileDir;
    // User specified uri for server to download change files.
    std::string replicationServerUri;

    // Uri of the SPARQL graph. Optional.
    std::string graphUri;
    // Access token for the SPARQL endpoint. Optional.
    std::string accessToken;

    // User specified sequence number from command line.
    int sequenceNumber = -1;
    // User specified sequence number up to which the change files should be processed.
    int maxSequenceNumber = std::numeric_limits<int>::min();

    // User specified timestamp from the command line
    std::string timestamp;

    // User specified bounding box in the form of "LEFT, BOTTOM, RIGHT, TOP"
    std::string bbox;
    // User specified path to a polygon file
    std::string pathToPolygonFile;

    // Strategy that is used by the osmium extract command to extract the changes. Possible values
    // are: smart, complete_ways and simple. The default is "smart", because this is the one
    // Geofabrik uses for their country extracts.
    // See: https://docs.osmcode.org/osmium/latest/osmium-extract.html for an explanation of the
    // strategies.
    std::string extractStrategy = "smart";

    int numThreads = std::thread::hardware_concurrency();

    // Specifies whether a progress bar should be shown
    bool showProgress = true;

    // Option that can be used if the SPARQL endpoint is QLever.
    bool isQLever = false;

    // Option to enable detailed statistics output.
    bool showDetailedStatistics = false;

    // The number of values or triples that should be sent in one batch to the SPARQL endpoint
    size_t batchSize = DEFAULT_BATCH_SIZE;

    // Directory to store temporary files
    std::filesystem::path tmpDir = std::filesystem::temp_directory_path();

    // Specifies what happens with the sparql output
    // - ENDPOINT: The sparql updates are sent to the sparql endpoint
    // - FILE: The sparql updates are written to a file
    // - DEBUG: All sparql queries and updates are written to a file
    SparqlOutput sparqlOutput = ENDPOINT;
    std::filesystem::path sparqlOutputFile;

    // User can specify a file to which the response of the SPARQL endpoint will be written.
    std::filesystem::path sparqlResponseFile;

    // Stores the osm2rdf options that will be fetched from the SPARQL endpoint before
    // we convert the OSM data to RDF triples.
    std::map<std::string, std::string> osm2rdfOptions;

    // True, if the osm2rdf dump was created with a separate prefix for untagged nodes.
    std::string separatePrefixForUntaggedNodes;

    // Generate the information string containing the current settings.
    void printInfo() const;

    // Parse provided commandline arguments into config object.
    void fromArgs(int argc, char** argv);
};

}

#endif //OSM_LIVE_UPDATES_CONFIG_H
