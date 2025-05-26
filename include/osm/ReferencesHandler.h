// Copyright 2025, University of Freiburg
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

#ifndef REFERENCESHANDLER_H
#define REFERENCESHANDLER_H

#include "osmium/handler.hpp"

#include "osm/NodeHandler.h"
#include "osm/WayHandler.h"
#include "osm/RelationHandler.h"

namespace olu::osm {
    class ReferencesHandler: public osmium::handler::Handler {
    public:
        explicit ReferencesHandler(config::Config &config,
                                   OsmDataFetcher &odf,
                                   NodeHandler &nodeHandler,
                                   WayHandler &wayHandler,
                                   RelationHandler &relationHandler):
            _config(config),
            _odf(odf),
            _nodeHandler(nodeHandler),
            _wayHandler(wayHandler),
            _relationHandler(relationHandler) {}

        // Iterators for osmium::apply
        void way(const osmium::Way& way);
        void relation(const osmium::Relation& relation);

        /**
         * Fetches the ids of all nodes and ways that are referenced by the relation with the given
         * ids and stores them in the corresponding set (_referencedNodes, _referencedWays)
         *
         * @param relationIds The ids of the relations for which the referenced nodes and ways
         * should be fetched
         */
        void getReferencesForRelations(const std::set<id_t> &relationIds);

        /**
         * Fetches the ids of all nodes that are referenced by the way with the given
         * ids and stores them in the corresponding set (_referencedNodes)
         *
         * @param wayIds The ids of the ways for which the referenced nodes should be fetched
         */
        void getReferencesForWays(const std::set<id_t> &wayIds);

        [[nodiscard]] std::set<id_t> getReferencedNodes() const {
            return _referencedNodes;
        }
        [[nodiscard]] std::set<id_t> getReferencedWays() const {
            return _referencedWays;
        }
        [[nodiscard]] std::set<id_t> getReferencedRelations() const {
            return _referencedRelations;
        }

    private:
        config::Config& _config;
        OsmDataFetcher& _odf;
        NodeHandler& _nodeHandler;
        WayHandler& _wayHandler;
        RelationHandler& _relationHandler;

        // Nodes that are referenced by a way or relation that are NOT present in the change file,
        // meaning they have to be fetched from the database
        std::set<id_t> _referencedNodes;

        // Ways that are referenced by a relation that are NOT present in the change file,
        // meaning they have to be fetched from the database
        std::set<id_t> _referencedWays;

        // Relations that are referenced by a relation that are NOT present in the change file,
        // meaning they have to be fetched from the database
        std::set<id_t> _referencedRelations;
    };

    /**
     * Exception that can appear inside the `ReferencesHandler` class.
     */
    class ReferencesHandlerException final : public std::exception {
        std::string message;
    public:
        explicit ReferencesHandlerException(const char* msg) : message(msg) { }

        [[nodiscard]] const char* what() const noexcept override {
            return message.c_str();
        }
    };
}


#endif //REFERENCESHANDLER_H
