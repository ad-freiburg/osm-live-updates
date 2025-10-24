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

#ifndef OSMOBJECT_H
#define OSMOBJECT_H

namespace olu::osm {
    /**
     * Possible types of osm objects in a change file.
     *
     * NODE_TAGGED and NODE_UNTAGGED are only used internally to distinguish between nodes if
     * osm2rdf was run with the option to use a separate IRI for untagged nodes.
     *
     */
    enum class OsmObjectType {
        NODE, NODE_TAGGED, NODE_UNTAGGED, WAY, RELATION
    };
}

#endif //OSMOBJECT_H
