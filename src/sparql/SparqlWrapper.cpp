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

#include <string>
#include <fstream>
#include <iostream>

#include "simdjson/padded_string.h"

#include "util/URLHelper.h"
#include "util/HttpRequest.h"
#include "config/Constants.h"

namespace cnst = olu::config::constants;

// _________________________________________________________________________________________________
void olu::sparql::SparqlWrapper::setQuery(const std::string &query) {
    _query = query;
}

// _________________________________________________________________________________________________
void olu::sparql::SparqlWrapper::setPrefixes(const std::vector<std::string> &prefixes) {
    for (const auto & prefix : prefixes) {
        _prefixes += prefix + " ";
    }
}

// _________________________________________________________________________________________________
std::string olu::sparql::SparqlWrapper::send(const std::string& acceptValue, const bool isUpdate) {
    if (_config.sparqlOutput == config::SparqlOutput::DEBUG_FILE ||
        (_config.sparqlOutput == config::SparqlOutput::FILE && isUpdate)) {
        writeQueryToFileOutput();
    }

    // Format and encode query
    const std::string query = _prefixes + _query;
    const std::string encodedQuery = util::URLHelper::encodeForUrlQuery(query);

    const auto endpointUri = isUpdate
                                 ? _config.sparqlEndpointUriForUpdates
                                 : _config.sparqlEndpointUri;
    auto request = util::HttpRequest(util::POST, endpointUri);
    request.addHeader(cnst::HTML_KEY_CONTENT_TYPE, cnst::HTML_VALUE_CONTENT_TYPE);
    request.addHeader(cnst::HTML_KEY_ACCEPT, acceptValue);
    // We need to set this otherwise libcurl will wait 1 sec before sending the request
    request.addHeader("Expect", "");

    std::string body = (isUpdate ? "update=" : "query=") + encodedQuery;
    body += _config.accessToken.empty() ? "" : "&access-token=" + _config.accessToken;
    request.addBody(body);

    std::string response;
    try {
        if (!isUpdate || _config.sparqlOutput == config::SparqlOutput::ENDPOINT) {
            response = request.perform();
        }
    } catch(std::exception &e) {
        std::cerr << e.what() << std::endl;
        const std::string msg = "Exception while sending `POST` request to the sparql endpoint with"
                                " body: " + query;
        throw SparqlWrapperException(msg.c_str());
    }

    // Clear query and prefixes for the next request
    _query = ""; _prefixes = "";
    return response;
}

// _________________________________________________________________________________________________
void olu::sparql::SparqlWrapper::runUpdate() {
    send(cnst::HTML_VALUE_ACCEPT_SPARQL_RESULT_JSON, true);
}

// _________________________________________________________________________________________________
std::string olu::sparql::SparqlWrapper::runQuery() {
    const auto response = send(cnst::HTML_VALUE_ACCEPT_SPARQL_RESULT_JSON, false);

    if (response.empty()) {
        throw SparqlWrapperException("Empty response from SPARQL endpoint");
    }

    return response;
}

// _________________________________________________________________________________________________
void olu::sparql::SparqlWrapper::writeQueryToFileOutput() const {
    std::ofstream outputFile;
    outputFile.open (_config.sparqlOutputFile, std::ios_base::app);
    outputFile << _prefixes << _query << std::endl;
    outputFile.close();
}

// _________________________________________________________________________________________________
void olu::sparql::SparqlWrapper::clearCache() const {
    auto request = util::HttpRequest(util::HttpMethod::POST, _config.sparqlEndpointUri);
    request.addHeader(cnst::HTML_KEY_CONTENT_TYPE, cnst::HTML_VALUE_CONTENT_TYPE);
    request.addBody("cmd=clear-cache");

    try {
        request.perform();
    } catch(std::exception &e) {
        std::cerr << e.what() << std::endl;
        const std::string msg = "Exception while sending request to clear cache ot the endpoint";
        throw SparqlWrapperException(msg.c_str());
    }
}