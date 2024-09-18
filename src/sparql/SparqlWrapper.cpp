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
            _prefixes += prefix + " ";
        }
    }

    // _____________________________________________________________________________________________
    std::string SparqlWrapper::runQuery() {
        handleFileOutput();

        // Format and encode query for url
        std::string query = _prefixes + _query;
        std::string encodedQuery = util::URLHelper::encodeForUrlQuery(query);

        std::string response;
        if (_httpMethod == util::POST) {
            auto request = util::HttpRequest(_httpMethod, _config.sparqlEndpointUri);
            request.addHeader(olu::config::constants::HTML_KEY_CONTENT_TYPE,
                              olu::config::constants::HTML_VALUE_CONTENT_TYPE);
            std::string body = "query=" + encodedQuery;
            request.addBody(body);
            response = request.perform();
        } else if (_httpMethod == util::GET) {
            std::string url = _config.sparqlEndpointUri + "?query=" + encodedQuery;
            auto request = util::HttpRequest(_httpMethod, url);
            request.addHeader(olu::config::constants::HTML_KEY_ACCEPT,
                              olu::config::constants::HTML_VALUE_ACCEPT_SPARQL_RESULT_XML);
            response = request.perform();
        }

        _query = ""; _prefixes = "";
        return response;
    }

    void SparqlWrapper::clearOutputFile() const {
        if (_config.writeSparqlQueriesToFile) {
            std::ofstream outputFile;
            outputFile.open (_config.pathToSparqlQueryOutput, std::ios_base::trunc);
            outputFile << "";
            outputFile.close();
        }
    }

    void SparqlWrapper::handleFileOutput() {
        if (_config.writeSparqlQueriesToFile) {
            std::ofstream outputFile;
            outputFile.open (_config.pathToSparqlQueryOutput, std::ios_base::app);
            outputFile << _prefixes << _query << std::endl;
            outputFile.close();
        }
    }

    void SparqlWrapper::clearCache() const {
        std::string url = _config.sparqlEndpointUri;
        auto request = util::HttpRequest(util::HttpMethod::POST, url);
        request.addHeader(olu::config::constants::HTML_KEY_CONTENT_TYPE,
                          olu::config::constants::HTML_VALUE_CONTENT_TYPE);
        request.addBody("cmd=clear-cache");
        auto response = request.perform();
    }

} // namespace olu::sparql
