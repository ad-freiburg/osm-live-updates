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

#ifndef OSM_LIVE_UPDATES_OSMOBJECTHELPER_H
#define OSM_LIVE_UPDATES_OSMOBJECTHELPER_H

#include <string>

#include "osmium/osm/object.hpp"

#include "osm/OsmObjectType.h"
#include "osm/ChangeAction.h"
#include "util/Types.h"

namespace olu::osm {
    class OsmObjectHelper {
    public:
        /**
         * Parses the id from an uri like "https://www.openstreetmap.org/node/1" or
         * "https://osm2rdf.cs.uni-freiburg.de/rdf/geom#osm_node_10916447545".
         *
         * The uri has to end with the id number.
         * There is no validation of the input,
         * so make sure it is in the correct format.
         *
         * @param uri The uri to extract the id from.
         * @return The extracted id as an id_t type.
         */
        static id_t parseIdFromUri(const std::string_view &uri);

        /**
         * Parses the osm object type from an uri like "https://www.openstreetmap.org/node/1" or
         * "https://www.openstreetmap.org/way/1".
         *
         * The uri has to start with an osm object namespace
         * (NAMESPACE_IRI_OSM_NODE, NAMESPACE_IRI_OSM_WAY, or NAMESPACE_IRI_OSM_RELATION).
         *
         * @param uri The uri to extract the osm object type from.
         * @return The extracted osm object type as an OsmObjectType enum value.
         */
        static OsmObjectType parseOsmTypeFromUri(const std::string_view& uri);

        /**
         * Parses a WKT point string and returns the latitude and longitude as a pair.
         * The WKT point string should be in the format "POINT(lon lat)".
         * The longitude and latitude will not be cast to double, but returned as string.
         *
         * @param wktPoint The WKT point string to parse.
         * @return A pair containing the longitude and latitude as string.
         */
        static lon_lat_t parseLonLatFromWktPoint(const std::string_view &wktPoint);

        /**
         * To check whether an object has been created or modified, we check if the version is
         * equal to 1. This is, however, not always correct, as it can happen that we merge several
         * change files. An object created in one file may have been modified in a later one but is
         * still missing on the sparql endpoint, e.g., it is equal to a create operation for the
         * endpoint.
         *
         * @param osmObject The osm object to get the action for.
         * @return The action (create, modify or delete) that was performed on the given osm element
         * in the change file
         */
        static ChangeAction getChangeAction(const osmium::OSMObject &osmObject);
    };

    /**
     * Exception that can appear inside the `WktHelper` class.
     */
    class OsmObjectHelperException final : public std::exception {
        std::string message;

    public:
        explicit OsmObjectHelperException(const char* msg) : message(msg) { }

        [[nodiscard]] const char* what() const noexcept override {
            return message.c_str();
        }
    };
}
#endif //OSM_LIVE_UPDATES_OSMOBJECTHELPER_H
