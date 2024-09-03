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

#ifndef OSM_LIVE_UPDATES_SPARQLWRAPPER_H
#define OSM_LIVE_UPDATES_SPARQLWRAPPER_H

#include "util/HttpMethod.h"
#include "config/Config.h"

#include <string>
#include <vector>

namespace olu::sparql {

class SparqlWrapper {
public:
    // Class that handles the connection to a sparql endpoint
    explicit SparqlWrapper(const olu::config::Config& config) { _config = config; };

    // Sets the HTTP Method for the query. Typically, `SELECT` queries should be performed with
    // `GET` and update queries (`DELETE`, `INSERT`) with `POST`
    void setMethod(util::HttpMethod method);

    // Sets the current query
    void setQuery(const std::string& query);

    // Sets the prefixes for the current query
    void setPrefixes(const std::string& prefixes);

    // Sends the current query to the sparql endpoint and returns the response from the endpoint.
    // Make sure that the correct HTTP method is set before running the query (POST for database
    // updates and GET for queries with select)
    std::string runQuery();
private:
    olu::config::Config _config;
    util::HttpMethod _httpMethod = util::POST;
    std::string _query;
    std::string _prefixes;

    void handleFileOutput();
};

} // namespace olu::sparql

#endif //OSM_LIVE_UPDATES_SPARQLWRAPPER_H
