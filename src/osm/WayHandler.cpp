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

#include "osm/WayHandler.h"

#include <iostream>

#include "osmium/osm/way.hpp"
#include "osm2rdf/util/Time.h"

#include "osm/OsmObjectHelper.h"

// _________________________________________________________________________________________________
void olu::osm::WayHandler::printWayStatistics() const {
    std::cout << osm2rdf::util::currentTimeFormatted()
            << "ways created: " << _createdWays.size()
            << " modified: " << _modifiedWays.size() + _modifiedWaysWithChangedMembers.size()
            << " deleted: " << _deletedWays.size()
            << std::endl;
}

// _________________________________________________________________________________________________
void olu::osm::WayHandler::way(const osmium::Way &way) {
    switch (OsmObjectHelper::getChangeAction(way)) {
        case ChangeAction::CREATE:
            _createdWays.insert(way.id());
            break;
        case ChangeAction::DELETE:
            _deletedWays.insert(way.id());
            break;
        case ChangeAction::MODIFY:
            std::vector<id_t> nodeRefs;
            for (const auto &nodeRef: way.nodes()) {
                nodeRefs.push_back(nodeRef.ref());
            }

            _modifiedWaysBuffer.emplace(way.id(), nodeRefs);
            break;
    }
}

// _________________________________________________________________________________________________
void olu::osm::WayHandler::checkWaysForMemberChange(
    const std::set<id_t> &modifiedNodesWithChangedLocation) {
    for (const auto &[wayId, nodeRefs] : _modifiedWaysBuffer) {
        // We have to check if a node reference of the way has its location changed, and if so,
        // the ways geometry has to be updated nevertheless
        bool hasModifiedNode = false;
        for (const auto &nodeId : nodeRefs) {
            if (modifiedNodesWithChangedLocation.contains(nodeId)) {
                _modifiedWaysWithChangedMembers.insert(wayId);
                hasModifiedNode = true;
                break;
            }
        }

        if (hasModifiedNode) {
            continue;
        }

        const auto &membersEndpoint =  _odf->fetchWaysMembersSorted({wayId});
        if (membersEndpoint.empty()) {
            _createdWays.insert(wayId);
            continue;
        }

        for (const auto &[wayId, nodeRefsEndpoint]: membersEndpoint) {
            if (OsmObjectHelper::areWayMemberEqual(nodeRefs, nodeRefsEndpoint)) {
                _modifiedWays.insert(wayId);
            } else {
                _modifiedWaysWithChangedMembers.insert(wayId);
            }
        }
    }

    _modifiedWaysBuffer.clear();
}
