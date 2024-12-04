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
#include "util/XmlReader.h"

#include <string>
#include <fstream>
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace cnst = olu::config::constants;
namespace olu::sparql {
    // _____________________________________________________________________________________________
    void SparqlWrapper::setQuery(const std::string &query) {
        _query = query;
    }

    // _____________________________________________________________________________________________
    void SparqlWrapper::setPrefixes(const std::vector<std::string> &prefixes) {
        for (const auto & prefix : prefixes) {
            _prefixes += prefix + " ";
        }
    }

    std::string
    SparqlWrapper::send(const std::string& acceptValue, const std::string &endpointUri,
                        bool isUpdate) {
        handleFileOutput();

        // Format and encode query
        std::string query = _prefixes + _query;
        std::string encodedQuery = util::URLHelper::encodeForUrlQuery(query);

        std::string response;
        auto request = util::HttpRequest(util::POST, endpointUri);
        request.addHeader(cnst::HTML_KEY_CONTENT_TYPE, cnst::HTML_VALUE_CONTENT_TYPE);
        request.addHeader(cnst::HTML_KEY_ACCEPT, acceptValue);
        // We need to set this otherwise libcurl will wait 1 sec before sending the request
        request.addHeader("Expect", "");

        std::string body = (isUpdate ? "update=" : "query=") + encodedQuery;
        request.addBody(body);
        try {
            response = request.perform();
        } catch(std::exception &e) {
            std::cerr << e.what() << std::endl;
            std::string msg =
                    "Exception while sending `POST` request to the sparql endpoint with body: "
                    + query;
            throw SparqlWrapperException(msg.c_str());
        }

        // Clear query and prefixes for next request
        _query = ""; _prefixes = "";
        return response;
    }


    // _____________________________________________________________________________________________
    void SparqlWrapper::runUpdate() {
        auto endpointUri = _config.sparqlEndpointUri + _config.pathForSparqlUpdates;
        auto response = send(cnst::HTML_VALUE_ACCEPT_SPARQL_RESULT_JSON, endpointUri, true);

        if (response.empty()) {
            return;
        }

        boost::property_tree::ptree pt;
        std::istringstream json_stream(response);
        try {
            read_json(json_stream, pt);
        } catch(std::exception &e) {
            std::string msg = "Could not interpret response from SPARQL endpoint: " + response;
            throw SparqlWrapperException(msg.c_str());
        }

        if (pt.get<std::string>("status") == "ERROR") {
            auto exception = pt.get<std::string>("exception");
            std::string msg = "SPARQL endpoint returned status ERROR with exception: " + exception;
            throw SparqlWrapperException(msg.c_str());
        }
    }

    // _____________________________________________________________________________________________
    boost::property_tree::ptree SparqlWrapper::runQuery() {
        auto response = send(cnst::HTML_VALUE_ACCEPT_SPARQL_RESULT_XML,
                            _config.sparqlEndpointUri, false);

        if (response.empty()) {
            throw SparqlWrapperException("Empty response from SPARQL endpoint");
        }

        boost::property_tree::ptree pt;
        std::istringstream json_stream(response);
        // The QLever endpoint will return the content in json if an exception occurred, so we check
        // If we can parse the response as json. If that fails the response is in xml format and we
        // can carry on
        try {
            read_json(json_stream, pt);
        } catch(std::exception &e) {
            // If the json parsing failed that means we got a valid response from the endpoint
            boost::property_tree::ptree responseAsTree;
            util::XmlReader::populatePTreeFromString(response, responseAsTree);
            return responseAsTree;
        }

        if (pt.get<std::string>("status") == "ERROR") {
            auto exception = pt.get<std::string>("exception");
            std::string msg = "SPARQL endpoint returned status ERROR with exception: " + exception;
            throw SparqlWrapperException(msg.c_str());
        }

        std::string msg = "Could not interpret response from SPARQL endpoint: " + response;
        throw SparqlWrapperException(msg.c_str());
    }

    void SparqlWrapper::clearOutputFile() const {
        if (_config.writeSparqlQueriesToFile) {
            std::ofstream outputFile;
            outputFile.open (_config.pathToSparqlQueryOutput, std::ios_base::trunc);
            outputFile << "";
            outputFile.close();
        }
    }

    void SparqlWrapper::handleFileOutput() const {
        if (_config.writeSparqlQueriesToFile) {
            std::ofstream outputFile;
            outputFile.open (_config.pathToSparqlQueryOutput, std::ios_base::app);
            outputFile << _prefixes << _query << std::endl;
            outputFile.close();
        }
    }

    void SparqlWrapper::clearCache() const {
        auto request = util::HttpRequest(util::HttpMethod::POST, _config.sparqlEndpointUri);
        request.addHeader(cnst::HTML_KEY_CONTENT_TYPE, cnst::HTML_VALUE_CONTENT_TYPE);
        request.addBody("cmd=clear-cache");

        try {
            request.perform();
        } catch(std::exception &e) {
            std::cerr << e.what() << std::endl;
            std::string msg =
                    "Exception while sending request to clear cache ot the sparql endpoint";
            throw SparqlWrapperException(msg.c_str());
        }
    }

} // namespace olu::sparql
