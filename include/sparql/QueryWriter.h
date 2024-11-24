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
         * @returns A SPARQL query that deletes all triples for the given node ids.
         */
        static std::string writeNodesDeleteQuery(const std::set<long long> &nodeIds);

        /**
        * @returns A SPARQL query that deletes all triples for the given way ids.
        */
        static std::string writeWaysDeleteQuery(const std::set<long long int> &wayIds);

        /**
        * @returns A SPARQL query that deletes all triples for the given relation ids.
        */
        static std::string writeRelationsDeleteQuery(const std::set<long long int> &relationIds);

        /**
        * @returns A SPARQL query for the locations of the nodes with the given ID in WKT format
        */
        static std::string writeQueryForNodeLocations(const std::set<long long int> &nodeIds);

        /**
         * @returns A SPARQL query for the latest timestamp of any node in the database
         */
        static std::string writeQueryForLatestNodeTimestamp();

        /**
        * @returns A SPARQL query for the subject of all members of the given relation
        */
        static std::string writeQueryForRelationMembers(const long long &relationId);

        /**
        * @returns A SPARQL query for the subject of all members of the given relation
        */
        static std::string writeQueryForWayMembers(const long long &wayId);

        /**
         * @returns A SPARQL query for all nodes that are referenced by the given way
         */
        static std::string writeQueryForWaysMembers(const std::set<long long> &wayIds);

        /**
         * @returns A SPARQL query for all members of the given relations
         */
        static std::string writeQueryForRelationMembers(const std::set<long long> &relIds);

        /**
        * @returns A SPARQL query for all ways that reference the given nodes
        */
        static std::string writeQueryForWaysReferencingNodes(const std::set<long long> &nodeIds);

        /**
        * @returns A SPARQL query for relations that reference the given nodes
        */
        static std::string writeQueryForRelationsReferencingNodes(const std::set<long long> &nodeIds);

        /**
        * @returns A SPARQL query for relations that reference the given ways
        */
        static std::string writeQueryForRelationsReferencingWays(const std::set<long long> &wayIds);

        /**
        * @returns A SPARQL query for relations that reference the given relations
        */
        static std::string writeQueryForRelationsReferencingRelations(const std::set<long long> &relationIds);
    };
} // namespace olu::sparql

#endif //OSM_LIVE_UPDATES_QUERYWRITER_H
