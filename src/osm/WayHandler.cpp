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

#include "osmium/osm/way.hpp"

#include "osm/OsmObjectHelper.h"

// _________________________________________________________________________________________________
void olu::osm::WayHandler::way(const osmium::Way &way) {
    switch (OsmObjectHelper::getChangeAction(way)) {
        case ChangeAction::CREATE:
            _createdWays.insert(way.id());
            _stats->countCreatedWay();
            break;
        case ChangeAction::DELETE:
            _deletedWays.insert(way.id());
            _stats->countDeletedWay();
            break;
        case ChangeAction::MODIFY:
            _modifiedWays.insert(way.id());
            _stats->countModifiedWay();
            break;
    }
}
