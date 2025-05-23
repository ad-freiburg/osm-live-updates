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

namespace olu::config {

enum SparqlOutput {
    ENDPOINT = 0,
    FILE = 1,
    DEBUG_FILE = 2,
};

struct Config {
    // The uri of the SPARQL endpoint for queries
    std::string sparqlEndpointUri;
    // The uri to the SPARQL endpoint to update. If not specified by user, this will be the same as
    // the endpoint for queries
    std::string sparqlEndpointUriForUpdates;

    // User specified local directory for change files.
    std::string changeFileDir;
    // User specified uri for server to download change files.
    std::string changeFileDirUri;

    // Uri of the SPARQL graph. Optional.
    std::string graphUri;
    // Access token for the SPARQL endpoint. Optional.
    std::string accessToken;

    // User specified sequence number from command line.
    int sequenceNumber = -1;
    // User specified timestamp from command line
    std::string timestamp;

    // Specifies if osm2rdf should be run with the option to mask blank nodes
    bool noBlankNodes = false;

    // Specifies whether a progress bar should be shown
    bool showProgress = true;

    size_t maxValuesPerQuery = 1024;

    // Specifies what happens with the sparql output
    // - ENDPOINT: The sparql updates are sent to the sparql endpoint
    // - FILE: The sparql updates are written to a file
    // - DEBUG: All sparql queries and updates are written to a file
    SparqlOutput sparqlOutput = ENDPOINT;
    std::filesystem::path sparqlOutputFile;

    // Generate the information string containing the current settings.
    [[nodiscard]] std::string getInfo(std::string_view prefix) const;

    // Parse provided commandline arguments into config object.
    void fromArgs(int argc, char** argv);
};

}

#endif //OSM_LIVE_UPDATES_CONFIG_H
