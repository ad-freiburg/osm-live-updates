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

#ifndef OSM_LIVE_UPDATES_QUERYWRITER_H
#define OSM_LIVE_UPDATES_QUERYWRITER_H

#include <string>
#include <vector>
#include <boost/property_tree/ptree.hpp>

namespace olu::sparql {

    /**
     *  Convenience class for some functions that return SPARQL queries.
     */
    class QueryWriter {
    public:
        /**
         * Returns the elements (`node`, `way` or `relation`) subject formatted for a SPARQL query.
         *
         * Example: For a node element with id 1787 the function would return 'osmnode:1787'
         *
         * @param elementTag The name of the element (`node`, `way` or `relation`)
         * @param element The osm element
         * @return The subject of the element formatted for a SPARQL query
         */
        static std::string getSubjectFor(const std::string& elementTag,
                                         const boost::property_tree::ptree &element);

        /**
         * @returns A SPARQL query that inserts a list of triples in to the database
         */
        static std::string writeInsertQuery(const std::vector<std::string>& triples);

        /**
         * @returns A SPARQL query that deletes all triples for a given subject in the database
         */
        static std::string writeDeleteQuery(const std::string& subject);

        /**
        * @returns A SPARQL query for the location of the node with the given ID in WKT format
        */
        static std::string writeQueryForNodeLocation(const long long &nodeId);

        /**
         * @returns A SPARQL query for the latest timestamp of any node in the database
         */
        static std::string writeQueryForLatestNodeTimestamp();
    };

    /**
     * Exception that can appear inside the `QueryWriter` class.
     */
    class QueryWriterException : public std::exception {
    private:
        std::string message;

    public:
        explicit QueryWriterException(const char* msg) : message(msg) { }

        [[nodiscard]] const char* what() const noexcept override {
            return message.c_str();
        }
    };

} // namespace olu::sparql

#endif //OSM_LIVE_UPDATES_QUERYWRITER_H
