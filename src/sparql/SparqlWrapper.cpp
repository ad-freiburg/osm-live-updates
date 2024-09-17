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

#include "sparql/SparqlWrapper.h"
#include "util/URLHelper.h"
#include "util/HttpRequest.h"
#include "config/Constants.h"

#include <string>
#include <fstream>
#include <iostream>

namespace olu::sparql {

    // _____________________________________________________________________________________________
    void SparqlWrapper::setMethod(const util::HttpMethod httpMethod) {
        _httpMethod = httpMethod;
    }

    // _____________________________________________________________________________________________
    void SparqlWrapper::setQuery(const std::string &query) {
        _query = query;
    }
    void SparqlWrapper::setPrefixes(const std::vector<std::string> &prefixes) {
        for (const auto & prefix : prefixes) {
            _prefixes += prefix + "\n";
        }
    }

    // _____________________________________________________________________________________________
    std::string SparqlWrapper::runQuery() {
        handleFileOutput();
        // Format and encode query for url
        std::string query = _prefixes + "\n" + _query;
        std::string encodedQuery = util::URLHelper::encodeForUrlQuery(query);
        std::string url = _config.sparqlEndpointUri + "?query=" + encodedQuery;

        auto request = util::HttpRequest(_httpMethod, url);
        std::string response;
        if (_httpMethod == util::POST) {
            request.addHeader(olu::config::constants::HTML_KEY_CONTENT_TYPE,
                              olu::config::constants::HTML_VALUE_CONTENT_TYPE_SPARQL_QUERY);
        } else if (_httpMethod == util::GET) {
            request.addHeader(olu::config::constants::HTML_KEY_ACCEPT,
                              olu::config::constants::HTML_VALUE_ACCEPT_SPARQL_RESULT_XML);
        }

        response = request.perform();
        _query = ""; _prefixes = "";

        return response;
    }

    void SparqlWrapper::handleFileOutput() {
        if (_config.writeSparqlQueriesToFile) {
            std::ofstream outputFile;
            outputFile.open (_config.pathToSparqlQueryOutput, std::ios_base::app);
            outputFile << _query << "\n";
            outputFile.close();
        }
    }

    void SparqlWrapper::clearCache() const {
        std::string url = _config.sparqlEndpointUri;
        auto request = util::HttpRequest(util::HttpMethod::POST, url);
        request.addHeader("Content-Type", "application/x-www-form-urlencoded;charset=UTF-8");
        request.addHeader("Accept", "application/qlever-results+json");
        request.addBody("cmd=clear-cache");
        auto response = request.perform();
    }

} // namespace olu::sparql
