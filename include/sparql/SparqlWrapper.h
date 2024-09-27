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

    /**
     * Wrapper class that handles communication with a SPARQL endpoint. In order to successfully
     * send a request to the SPARQL endpoint, the prefixes, query and method must be set with the
     * corresponding functions. Then the request can be sent with `runQuery`.
     *
     * This class will currently only work with QLever SPARQL endpoints.
     *
     * If the `writeSparqlQueriesToFile` flag is set, all SPARQL queries that were send to the
     * endpoint will be stored in a .txt file located at the path which is specified in the config
     * (`pathToSparqlQueryOutput`)
     */
    class SparqlWrapper {
    public:
        explicit SparqlWrapper(const olu::config::Config& config) {
            _config = config; clearOutputFile();
        };

        /**
         * Sets the HTTP Method for the request to the SPARQL endpoint.
         * Choose `GET' for `SELECT' queries and `POST' for update queries (`DELETE', `INSERT').
         *
         * @param method The HTTP method for the request to the sparql endpoint
         */
        void setMethod(util::HttpMethod method);

        /**
         * @param query The query to send to the SPARQL endpoint. The prefixes must be set
         * separately
         */
        void setQuery(const std::string& query);

        /**
         * @param prefixes The prefixes to send to the SPARQL endpoint.
         */
        void setPrefixes(const std::vector<std::string> &prefixes);

        /**
         * Sends a request to clear the cache of the SPARQL endpoint.
         */
        void clearCache() const;

        /**
         * Sends a request with the current prefixes and query to the SPARQL endpoint.
         *
         * @return The response from the SPARQL endpoint.
         */
        std::string runQuery();
    private:
        olu::config::Config _config;
        util::HttpMethod _httpMethod = util::POST;
        std::string _query;
        std::string _prefixes;

        void clearOutputFile() const;

        /**
         * Writes the prefixes and query to the output file if the `writeSparqlQueriesToFile` flag
         * is set
         */
        void handleFileOutput();
    };

    /**
     * Exception that can appear inside the `SparqlWrapper` class.
     */
    class SparqlWrapperException : public std::exception {
    private:
        std::string message;

    public:
        explicit SparqlWrapperException(const char* msg) : message(msg) { }

        [[nodiscard]] const char* what() const noexcept override {
            return message.c_str();
        }
    };

} // namespace olu::sparql

#endif //OSM_LIVE_UPDATES_SPARQLWRAPPER_H
