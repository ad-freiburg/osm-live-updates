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

#include <set>
#include <string>

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
         * @return A vector containing a pair of the member's uri and role for all members of the
         * given relation.
         */
        virtual std::vector<Relation> fetchRelations(const std::set<id_t> &relationIds){return {};}

        /**
         * Fetches tags and timestamp for the given relation
         */
        virtual void fetchRelationInfos(Relation &relation){}

        /**
         * Sends a query to the sparql endpoint to get the ids of all nodes that are referenced
         * in the given way
         *
         * @return The subjects of all members
         */
        virtual std::vector<Way> fetchWays(const std::set<id_t> &wayIds){return {};}

        /**
         * Fetches tags and timestamp for the given way
         */
        virtual void fetchWayInfos(Way &way){}

        /**
          * Sends a query to the sparql endpoint to get the ids of all nodes that are referenced
          * in the given way
          *
          * @return The subjects of all members
          */
        virtual member_ids_t fetchWaysMembers(const std::set<id_t> &wayIds){return {};}

        /**
          * Sends a query to the sparql endpoint to get the ids of all nodes that are referenced
          * in the given way
          *
          * @return The subjects of all members
          */
        virtual std::vector<std::pair<id_t, member_ids_t>>
        fetchWaysMembersSorted(const std::set<id_t> &wayIds){return {};}

        /**
          * Sends a query to the sparql endpoint to get the members of the given relations
          *
          * @return The subjects of all members
          */
        virtual std::vector<std::pair<id_t, std::vector<RelationMember>>>
        fetchRelsMembersSorted(const std::set<id_t> &relIds){return {};}

        /**
         * @return The ids of all nodes and ways that are referenced by the given relations
         */
        virtual std::pair<std::vector<id_t>, std::vector<id_t>>
        fetchRelationMembers(const std::set<id_t> &relIds){return {};}

        /**
         * Sends a query to the sparql endpoint to the latest timestamp of any node in the database
         *
         * @return The latest timestamp of any node
         */
        virtual std::string fetchLatestTimestampOfAnyNode(){return {};}

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
    };
} // namespace olu

#endif //OSM_LIVE_UPDATES_OSMDATAFETCHER_H
