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

#include "osm/NodeHandler.h"

#include <iostream>
#include <ranges>

#include "osmium/osm/node.hpp"
#include "osm2rdf/util/Time.h"

#include "osm/Node.h"
#include "osm/OsmObjectHelper.h"
#include "util/BatchHelper.h"

// _________________________________________________________________________________________________
void olu::osm::NodeHandler::node(const osmium::Node& node) {
    switch (OsmObjectHelper::getChangeAction(node)) {
        case ChangeAction::CREATE:
            _createdNodes.insert(node.id());
            break;
        case ChangeAction::MODIFY:
            _modifiedNodesBuffer.emplace(node.id(), node.location());
            break;
        case ChangeAction::DELETE:
            _deletedNodes.insert(node.id());
            break;
    }
}

// _________________________________________________________________________________________________
void olu::osm::NodeHandler::printNodeStatistics() const {
    std::cout << osm2rdf::util::currentTimeFormatted()
    << "nodes created: " << _createdNodes.size()
    << " modified: " << _modifiedNodes.size() + _modifiedNodesWithChangedLocation.size()
    << " deleted: " << _deletedNodes.size()
    << std::endl;
}

// _________________________________________________________________________________________________
void olu::osm::NodeHandler::checkNodesForLocationChange() {
    auto keysView = std::views::keys(_modifiedNodesBuffer);
    const auto nodeIds = std::set(keysView.begin(), keysView.end());

    std::map<id_t, osmium::Location> remoteNodes;
    util::BatchHelper::doInBatches(
        nodeIds,
        _config.batchSize,
        [this, &remoteNodes](std::set<id_t> const& batch) mutable {
            for (const auto& node : _odf->fetchNodes(batch)) {
                remoteNodes.emplace(node.getId(), node.getLocation());
            }
    });

    for (const auto&[localId, localLocation] : _modifiedNodesBuffer) {
        if (const auto remoteNode = remoteNodes.find(localId); remoteNode != remoteNodes.end()) {
            if (localLocation == remoteNode->second) {
                _modifiedNodes.insert(localId);
            } else {
                _modifiedNodesWithChangedLocation.insert(localId);
            }
        } else {
            // If we cannot find the node location on the endpoint, we assume that the node has
            // to be created. (See OsmObjectHelper::getChangeAction for an explanation why this
            // could happen even if the node is in a modify-changeset)
            _createdNodes.insert(localId);
        }
    }

    _modifiedNodesBuffer.clear();
}