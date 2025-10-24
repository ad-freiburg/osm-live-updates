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

#ifndef OSM_LIVE_UPDATES_OSMDATAFETCHER_H
#define OSM_LIVE_UPDATES_OSMDATAFETCHER_H

#include <map>
#include <set>
#include <string>

#include "OsmDatabaseState.h"
#include "config/Constants.h"
#include "osm/Node.h"
#include "osm/Relation.h"
#include "osm/Way.h"
#include "util/Types.h"

namespace olu::osm {
    /**
     * Exception that can appear inside the `OsmDataFetcher` class.
     */
    class OsmDataFetcherException final : public std::exception {
        std::string message;
    public:
        explicit OsmDataFetcherException(const char* msg) : message(msg) { }

        [[nodiscard]] const char* what() const noexcept override {
            return message.c_str();
        }
    };

    /**
     * Base Class for the retrieval of osm data from the SPARQL endpoint.
     */
    class OsmDataFetcher {
    public:
        virtual ~OsmDataFetcher() = default;

        /**
         * Sends a query to the sparql endpoint to get the location of the nodes with the given ids
         *
         * @warning It is not guaranteed that the SPARQL endpoint returns a location for each node
         * ID. Therefore,
         * the returned vector can have fewer elements than the given set of node ids
         *
         * @param nodeIds The ids of the nodes to fetch location for
         * @return A vector containing node objects with the location and id
         */
        virtual std::vector<Node> fetchNodes(const std::set<id_t> &nodeIds){return {};}

        /**
         * Fetches the locations for the given node ids and writes the nodes to a file in the osm
         * XML format.
         *
         * @param filePath The path to the file where the nodes should be written
         * @param nodeIds The ids of the nodes to fetch
         */
        virtual void
        fetchAndWriteNodesToFile(const std::string &filePath, const std::set<id_t> &nodeIds){}

        /**
         * Fetches the members for the given relations and writes the relation to a file in the osm
         * XML format.
         *
         * @param filePath The path to the file where the relations should be written
         * @param relationIds The ids of the relations to fetch
         * @return The number of relations that were written to the file, e.g., the number of
         * relations for which the SPARQL endpoint returned the members.
         */
        virtual size_t
        fetchAndWriteRelationsToFile(const std::string &filePath,
                                     const std::set<id_t> &relationIds) { return {}; }

        /**
         * Fetches the members for the given ways and writes the way to a file in the osm
         * XML format.
         *
         * @param filePath The path to the file where the ways should be written
         * @param wayIds The ids of the ways to fetch
         * @return The number of ways that were written to the file, e.g., the number of ways for
         * which the SPARQL endpoint returned the members.
         */
        virtual size_t
        fetchAndWriteWaysToFile(const std::string &filePath,
                                const std::set<id_t> &wayIds){return{};}

        /**
          * Sends a query to the sparql endpoint to get the ids of all nodes that are referenced
          * in the given way
          *
          * @return The subjects of all members
          */
        virtual member_ids_t fetchWaysMembers(const std::set<id_t> &wayIds){return {};}

        /**
         * @return The ids of all nodes and ways that are referenced by the given relations
         */
        virtual std::pair<std::vector<id_t>, std::vector<id_t>>
        fetchRelationMembers(const std::set<id_t> &relIds){return {};}

        /**
         * Sends a query to the sparql endpoint to fetch the latest timestamp for the predicate
         * 'osmmeta:timestamp', which is the latest timestamp of all OSM objects in the database.
         *
         * @return The latest timestamp for the predicate 'osmmeta:timestamp'
         */
        virtual std::string fetchLatestTimestamp(){return {};}

        /**
         * @return The ids of all ways that reference the given nodes.
         */
        virtual std::vector<id_t> fetchWaysReferencingNodes(const std::set<id_t> &nodeIds){return {};}

        /**
         * @return The ids of all relations that reference the given nodes.
         */
        virtual std::vector<id_t> fetchRelationsReferencingNodes(const std::set<id_t> &nodeIds){return {};}

        /**
         * @return The ids of all relations that reference the given ways.
         */
        virtual std::vector<id_t> fetchRelationsReferencingWays(const std::set<id_t> &wayIds){return {};}

        /**
         * @return The ids of all relations that reference the given relations.
         */
        virtual std::vector<id_t> fetchRelationsReferencingRelations(const std::set<id_t> &relationIds){return {};}

        /**
        * Fetches the osm2rdf version of the initial dump from the SPARQL endpoint.
        *
        * @return The osm2rdf version for the initial dump or an empty string,
        * if no or multiple versions are found.
        */
        virtual std::string fetchOsm2RdfVersion() { return{}; }

        /**
         * Fetches the osm2rdf-options used to create the initial dump from the SPARQL endpoint.
         *
         * @return A map containing the osm2rdf-options, where the key is the option name and the
         * value is the option value.
         */
        virtual std::map<std::string, std::string> fetchOsm2RdfOptions() { return {}; }

        /**
         * Fetches the 'osm2rdfmeta:updatesCompleteUntil' metadata triple from the SPARQL endpoint
         * and returns the database state until which the updates are complete.
         *
         * @return The database state until which the updates are complete, or 0 if not found.
         */
        virtual OsmDatabaseState fetchUpdatesCompleteUntil() { return {}; }

        /**
         * Fetches the 'osm2rdfmeta:replicationServer' metadata triple from the SPARQL endpoint
         * and returns the replication server URI.
         *
         * @return The replication server URI, or an empty string if not found.
         */
        virtual std::string fetchReplicationServer() { return {}; }
    };
} // namespace olu

#endif //OSM_LIVE_UPDATES_OSMDATAFETCHER_H
