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
std::string olu::sparql::SparqlWrapper::sendQuery() {
    if (_config->sparqlOutput == config::SparqlOutput::DEBUG_FILE) {
        writeQueryToFileOutput(false);
    }

    // Set the accept-value depending on whether we are using QLever or not.
    // QLever endpoints will return metadata with the results, while SPARQL results will only
    // include the actual data.
    const auto acceptValue = _config->isQLever
                                 ? cnst::HTML_VALUE_ACCEPT_QLEVER_RESULT_JSON
                                 : cnst::HTML_VALUE_ACCEPT_SPARQL_RESULT_JSON;

    // Format and encode query
    const std::string query = _prefixes + _query;
    const std::string encodedQuery = util::URLHelper::encodeForUrlQuery(query);

    auto request = util::HttpRequest(util::POST, _config->sparqlEndpointUri);
    request.addHeader(cnst::HTML_KEY_CONTENT_TYPE, cnst::HTML_VALUE_CONTENT_TYPE);
    request.addHeader(cnst::HTML_KEY_ACCEPT, acceptValue);
    // We need to set this otherwise libcurl will wait 1 sec before sending the request
    request.addHeader("Expect", "");

    std::string body = "query=" + encodedQuery;
    body += _config->accessToken.empty() ? "" : "&access-token=" + _config->accessToken;
    request.addBody(body);

    std::string response;
    try {
        response = request.perform();
    } catch(std::exception &e) {
        std::cerr << e.what() << std::endl;
        const std::string msg = "Exception while sending `POST` request to the sparql endpoint with"
                                " body: " + query.substr(0, 100);
        throw SparqlWrapperException(msg.c_str());
    }

    // Clear query and prefixes for the next request
    _query = ""; _prefixes = "";
    return response;
}

// _________________________________________________________________________________________________
std::string olu::sparql::SparqlWrapper::sendUpdate(const UpdateOperation &updateOp) {
    if (_config->sparqlOutput == config::SparqlOutput::DEBUG_FILE ||
        _config->sparqlOutput == config::SparqlOutput::FILE) {
        writeQueryToFileOutput(updateOp == UpdateOperation::INSERT);
    }

    std::string url = _config->sparqlEndpointUriForUpdates;
    if (updateOp == UpdateOperation::INSERT) {
        // For INSERT operations, we use the Graph store HTTP protocol, where the graph URI is
        // specified as a parameter in the body.
        url += _config->graphUri.empty() ? "?default" : "?graph=" + util::URLHelper::encodeForUrlQuery(_config->graphUri);
    }

    auto request = util::HttpRequest(util::POST,  url);

    // Set the accept-value depending on whether we are using QLever or not.
    // QLever endpoints will return metadata with the results, while SPARQL results will only
    // include the actual data.
    const auto acceptValue = _config->isQLever
                                 ? cnst::HTML_VALUE_ACCEPT_QLEVER_RESULT_JSON
                                 : cnst::HTML_VALUE_ACCEPT_SPARQL_RESULT_JSON;
    request.addHeader(cnst::HTML_KEY_ACCEPT, acceptValue);
    // We need to set this otherwise libcurl will wait 1 sec before sending the request
    request.addHeader("Expect", "");

    // Set an authorization header if an access token is provided by user
    if (!_config->accessToken.empty()) {
        request.addHeader(cnst::HTML_KEY_AUTHORIZATION, "Bearer " + _config->accessToken);
    }

    std::string body = _prefixes + _query;
    switch (updateOp) {
        case UpdateOperation::INSERT:
            // We use Graph store HTTP protocol for INSERT operations (POST with data)
            request.addHeader(cnst::HTML_KEY_CONTENT_TYPE, cnst::HTML_VALUE_CONTENT_TYPE_TURTLE);
            request.addBody(body);

            auto filename = cnst::getPathToOluTmpDir(_config->tmpDir).append("insert_").append(std::to_string(_insertOpCount)).append(".txt");
            _insertOpCount += 1;
            std::ofstream file(filename, std::ios::trunc);
            file << body << std::endl;
            file.close();

            break;
        case UpdateOperation::DELETE:
            request.addHeader(cnst::HTML_KEY_CONTENT_TYPE, cnst::HTML_VALUE_CONTENT_TYPE);
            body = "update=" + util::URLHelper::encodeForUrlQuery(body);
            request.addBody(body);
            break;
    }

    std::string response;
    try {
        if (_config->sparqlOutput == config::SparqlOutput::ENDPOINT) {
            response = request.perform();
        }
    } catch(std::exception &e) {
        std::cerr << e.what() << std::endl;
        const std::string msg = "Exception while sending `POST` request to the sparql endpoint";
        throw SparqlWrapperException(msg.c_str());
    }

    // Clear query and prefixes for the next request
    _query = ""; _prefixes = "";
    return response;
}

// _________________________________________________________________________________________________
std::string olu::sparql::SparqlWrapper::runUpdate(const UpdateOperation &updateOp) {
    return sendUpdate(updateOp);
}

// _________________________________________________________________________________________________
std::string olu::sparql::SparqlWrapper::runQuery() {
    const auto response = sendQuery();

    if (response.empty()) {
        throw SparqlWrapperException("Empty response from SPARQL endpoint");
    }

    return response;
}

// _________________________________________________________________________________________________
void olu::sparql::SparqlWrapper::writeQueryToFileOutput(const bool &isInsertOperation) const {
    std::ofstream outputFile;
    outputFile.open (_config->sparqlOutputFile, std::ios_base::app);

    // For insert operations, we use the graph store protocol which sends the triples as body to the
    // SPARQL endpoint.
    // However, when we write this operation to a file, we must wrap the query in an
    // INSERT DATA { ... } clause, so it still makes sense as a SPARQL update operation.
    if (isInsertOperation) {
        outputFile << _prefixes << " INSERT DATA { " << _query << "}" << std::endl;
    } else {
        outputFile << _prefixes << _query << std::endl;
    }
    outputFile.close();
}

// _________________________________________________________________________________________________
void olu::sparql::SparqlWrapper::clearCache() const {
    auto request = util::HttpRequest(util::HttpMethod::POST, _config->sparqlEndpointUri);
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