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

#include "osm/OsmDatabaseState.h"
#include "osm/Node.h"
#include "osm/Relation.h"
#include "osm/Way.h"
#include "sparql/SparqlWrapper.h"
#include "util/Types.h"

#include <set>
#include <string>

namespace olu::osm {

    /**
     * Deals with the retrieval of osm data needed for the update process.
     *
     * Osm change files are fetched from the server, the URL of which must be provided by the user.
     * The remaining data is collected from the SPARQL endpoint.
     */
    class OsmDataFetcher {
    public:
        explicit OsmDataFetcher(const config::Config &config)
            : _config(config), _sparqlWrapper(config) { }

        // Fetch from SERVER -----------------------------------------------------------------------
        /**
         * Fetches the database state (sequence number and timestamp) for the given sequence number
         * from the server
         *
         * @param sequenceNumber the sequence number to fetch the database state for
         * @return The database state for the provided sequence number
         */
        [[nodiscard]] OsmDatabaseState fetchDatabaseState(int sequenceNumber) const;

        /**
         * Fetches the database state (sequence number and timestamp) of the latest diff from the
         * server
         *
         * @return The latest database state on the server
         */
        [[nodiscard]] OsmDatabaseState fetchLatestDatabaseState() const;

        /**
         * Fetches the .osc change file from the server, writes it to a file and returns the path to
         * the file. The file might be compressed with gzip.
         *
         * @param sequenceNumber The sequence number to fetch the change file for
         * @return The path to the location of the fetched .osm Change file
         */
        std::string fetchChangeFile(int &sequenceNumber) ;

        /**
         * Fetches the 'nearest' database state for the given timestamp from the server, meaning the
         * first state which timestamp is before the given timestamp.
         *
         * @param timeStamp Timestamp to fetch the `nearest` database state for
         * @return The 'nearest' database state for the given timestamp
         */
        [[nodiscard]] OsmDatabaseState
        fetchDatabaseStateForTimestamp(const std::string& timeStamp) const;

        /**
         * Sends a query to the sparql endpoint to get the location of the nodes with the given ids
         *
         * @warning It is not guaranteed that the SPARQL endpoint returns a location for each node
         * ID, therefore the returned vector can have fewer elements than the given set of node ids
         *
         * @param nodeIds The ids of the nodes to fetch location for
         * @return A vector containing node objects with the location and id
         */
        std::vector<Node> fetchNodes(const std::set<id_t> &nodeIds);

        /**
         * @return A vector containing a pair of the member's uri and role for all members of the
         * given relation.
         */
        std::vector<Relation>
        fetchRelations(const std::set<id_t> &relationIds);

        /**
         * Sends a query to the sparql endpoint to get the ids of all nodes that are referenced
         * in the given way
         *
         * @return The subjects of all members
         */
        std::vector<Way> fetchWays(const std::set<id_t> &wayIds);

        /**
          * Sends a query to the sparql endpoint to get the ids of all nodes that are referenced
          * in the given way
          *
          * @return The subjects of all members
          */
        std::vector<id_t> fetchWaysMembers(const std::set<id_t> &wayIds);

        /**
         * @return The ids of all nodes and ways that are referenced by the given relations
         */
        std::pair<std::vector<id_t>, std::vector<id_t>>
        fetchRelationMembers(const std::set<id_t> &relIds);

        /**
         * Sends a query to the sparql endpoint to the latest timestamp of any node in the database
         *
         * @return The latest timestamp of any node
         */
        std::string fetchLatestTimestampOfAnyNode();

        /**
         * @return The ids of all ways that reference the given nodes.
         */
        std::vector<id_t> fetchWaysReferencingNodes(const std::set<id_t> &nodeIds);

        /**
         * @return The ids of all relations that reference the given nodes.
         */
        std::vector<id_t> fetchRelationsReferencingNodes(const std::set<id_t> &nodeIds);

        /**
         * @return The ids of all relations that reference the given ways.
         */
        std::vector<id_t> fetchRelationsReferencingWays(const std::set<id_t> &wayIds);

        /**
         * @return The ids of all relations that reference the given relations.
         */
        std::vector<id_t> fetchRelationsReferencingRelations(const std::set<id_t> &relationIds);

    private:
        config::Config _config;
        sparql::SparqlWrapper _sparqlWrapper;

        /**
         * Extracts the database state from a state file. A state file contains a sequence number
         * and a timestamp and describes the state for an osm change file.
         *
         * @param stateFile The state file to extract the database state from.
         * @return The database state described by the state file
         */
        static OsmDatabaseState extractStateFromStateFile(const std::string& stateFile);

        boost::property_tree::ptree runQuery(const std::string &query,
                                             const std::vector<std::string> &prefixes);
    };

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

} // namespace olu

#endif //OSM_LIVE_UPDATES_OSMDATAFETCHER_H
