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

#ifndef NODEHANDLER_H
#define NODEHANDLER_H

#include <map>
#include <set>

#include "osmium/handler.hpp"

#include "OsmDataFetcher.h"

namespace olu::osm {

    class NodeHandler: public osmium::handler::Handler {
    public:
        explicit NodeHandler(const config::Config &config): _config(config), _odf(config) {}

        // Iterator for osmium::apply
        void node(const osmium::Node& node);

        /**
         * Checks if the location of the given nodes from the change file has changed. If so, the
         * node is added to the _modifiedNodesWithChangedLocation set, otherwise to the
         * _modifiedNodes set
         */
        void checkNodesForLocationChange();

        [[nodiscard]] std::set<id_t> getCreatedNodes() const { return _createdNodes; }
        [[nodiscard]] std::set<id_t> getModifiedNodes() const { return _modifiedNodes; }
        [[nodiscard]] std::set<id_t> getDeletedNodes() const { return _deletedNodes; }
        [[nodiscard]] std::set<id_t> getModifiedNodesWithChangedLocation() const {
            return _modifiedNodesWithChangedLocation;}
        [[nodiscard]] std::set<id_t> getAllNodes() const {
            std::set<id_t> allNodes;
            allNodes.insert(_createdNodes.begin(), _createdNodes.end());
            allNodes.insert(_modifiedNodes.begin(), _modifiedNodes.end());
            allNodes.insert(_modifiedNodesWithChangedLocation.begin(),
                            _modifiedNodesWithChangedLocation.end());
            allNodes.insert(_deletedNodes.begin(), _deletedNodes.end());
            return allNodes;
        }
        [[nodiscard]] size_t getNumOfNodes() const {
            return _createdNodes.size() +
                   _modifiedNodes.size() +
                   _modifiedNodesWithChangedLocation.size() +
                   _deletedNodes.size();
        }

        /**
         * Prints the number of created, modified and deleted nodes to the console.
         */
        void printNodeStatistics() const;

        /**
         * @return True if the change file contains no nodes.
         */
        bool empty() const {
            return _createdNodes.empty() &&
                   _modifiedNodes.empty() &&
                   _modifiedNodesWithChangedLocation.empty() &&
                   _deletedNodes.empty();
        }

        /**
         * @Returns TRUE if the node with the given ID is contained in a `create`, `modify` or
         * 'delete' changeset in the changeFile.
         *
         * @warning All nodes inside the ChangeFile have to be processed BEFORE using this function.
         * Therefore, the earliest time this function can be called is inside the loop over the osm
         * elements inside `storeIdsOfElementsInChangeFile()` after the first way has occured.
         */
        [[nodiscard]] bool nodeInChangeFile(const id_t &nodeId) const {
            return _modifiedNodes.contains(nodeId) ||
                   _modifiedNodesWithChangedLocation.contains(nodeId) ||
                   _createdNodes.contains(nodeId) ||
                   _deletedNodes.contains(nodeId);
        }

    private:
        config::Config _config;
        OsmDataFetcher _odf;

        // Nodes that are in a delete-changeset in the change file.
        std::set<id_t> _deletedNodes;
        // Nodes that are in a create-changeset in the change file.
        std::set<id_t> _createdNodes;


        std::map<id_t, osmium::Location> _modifiedNodesBuffer;
        // Nodes that are in a modify-changeset in the change file that don't change their location.
        std::set<id_t> _modifiedNodes;
        // Nodes that where modified in the changeset and have a location that has changed.
        std::set<id_t> _modifiedNodesWithChangedLocation;
    };

}


#endif //NODEHANDLER_H
