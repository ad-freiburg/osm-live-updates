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

#include <string>
#include <vector>

#include "config/Config.h"

namespace olu::sparql {

    /**
     * Wrapper class that handles communication with a SPARQL endpoint.
     * To successfully send a request to the SPARQL endpoint,
     * the prefixes, query and method must be set with the
     * corresponding functions. Then the request can be sent with `runQuery`.
     *
     * This class will currently only work with QLever SPARQL endpoints.
     *
     * If the `writeSparqlQueriesToFile` flag is set, all SPARQL queries sent to the endpoint will
     * be stored in a .txt file located at the path which is specified in the config
     * (`pathToSparqlQueryOutput`)
     */
    class SparqlWrapper {
    public:
        explicit SparqlWrapper(config::Config  config): _config(std::move(config)) { }

        /**
         * Sets the query to send to the SPARQL endpoint. The prefixes must be set
         * with `setPrefixes`.
         */
        void setQuery(const std::string& query);

        /**
         * Sets the prefixes for the query to send to the SPARQL endpoint.
         */
        void setPrefixes(const std::vector<std::string> &prefixes);

        /**
         * Sends a request to clear the cache of the SPARQL endpoint.
         */
        void clearCache() const;

        /**
         * Sends a POST request with the encoded prefixes and query as body to the SPARQL endpoint.
         *
         * @return The response from the SPARQL endpoint.
         */
        std::string runQuery();

        /**
         * Sends a POST request with the encoded prefixes and the update query as body to the SPARQL
         * endpoint.
         *
         * @return The response from the SPARQL endpoint.
         */
        std::string runUpdate();
    private:
        config::Config _config;
        std::string _query;
        std::string _prefixes;

        void writeQueryToFileOutput() const;

        /**
         * Sends a HTTP request to the sparql endpoint.
         */
        std::string send(bool isUpdate);
    };

    /**
     * Exception that can appear inside the `SparqlWrapper` class.
     */
    class SparqlWrapperException final : public std::exception {
        std::string message;

    public:
        explicit SparqlWrapperException(const char* msg) : message(msg) { }

        [[nodiscard]] const char* what() const noexcept override {
            return message.c_str();
        }
    };

} // namespace olu::sparql

#endif //OSM_LIVE_UPDATES_SPARQLWRAPPER_H
