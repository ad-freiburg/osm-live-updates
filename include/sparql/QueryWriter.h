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
#include <set>

namespace olu::sparql {

    /**
     *  Convenience class for some functions that return SPARQL queries.
     */
    class QueryWriter {
    public:
        /**
         * @returns A SPARQL query that inserts a list of triples in to the database
         */
        static std::string writeInsertQuery(const std::vector<std::string>& triples);

        /**
         * @returns A SPARQL query that deletes all given triples
         */
        static std::string writeDeleteQuery(const std::vector<std::string>& subjects);

        /**
         * @returns A SPARQL query that deletes all triples for a given node element in the database
         */
        static std::string writeNodeDeleteQuery(const long long &nodeId);

        /**
         * @returns A SPARQL query that deletes all triples for the given node ids.
         * @example For one node with id 1:
         *
         */
        static std::string writeNodesDeleteQuery(const std::set<long long> &nodeIds);


        /**
         * @returns A SPARQL query that deletes all triples for a given way element in the database
         */
        static std::string writeWayDeleteQuery(const long long &wayId);

        /**
         * @returns A SPARQL query that deletes all triples for a given relation element in the
         * database
         */
        static std::string writeRelationDeleteQuery(const long long &relationId);

        /**
        * @returns A SPARQL query for the location of the node with the given ID in WKT format
        */
        static std::string writeQueryForNodeLocation(const long long &nodeId);

        /**
        * @returns A SPARQL query for the locations of the nodes with the given ID in WKT format
        */
        static std::string writeQueryForNodeLocations(const std::vector<long long> &nodeIds);

        /**
         * @returns A SPARQL query for the latest timestamp of any node in the database
         */
        static std::string writeQueryForLatestNodeTimestamp();

        /**
        * @returns A SPARQL query for the subject of all members of the given relation
        */
        static std::string writeQueryForRelationMembers(const long long &relationId);

        /**
        * @returns A SPARQL query for all ways in which the given node is a member
        */
        static std::string writeQueryForWaysReferencingNodes(const std::set<long long> &nodeIds);
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
