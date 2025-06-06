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

#include "osm/OsmObjectHelper.h"

#include <string>

#include "osmium/osm/object.hpp"

#include "config/Constants.h"
#include "osm/ChangeAction.h"
#include "util/Types.h"

namespace cnst = olu::config::constants;

constexpr std::string_view WHITESPACE = " \t\n\r\f\v";

// _________________________________________________________________________________________________
olu::id_t olu::osm::OsmObjectHelper::parseIdFromUri(const std::string_view &uri) {
    if (uri.empty()) {
        throw OsmObjectHelperException("Can not parse id from empty uri.");
    }

    // Scan from the end, skipping '>' and '"' (because they can be part of the uri formatting)
    // and collecting digits
    size_t i = uri.size();
    while (i > 0 && (uri[i - 1] == '>' || uri[i - 1] == '\"')) {
        --i;
    }

    const size_t end = i;
    while (i > 0 && std::isdigit(uri[i - 1])) {
        --i;
    }

    if (end == i) {
        const std::string msg = "Can not parse id from uri: " + std::string(uri);
        throw OsmObjectHelperException(msg.c_str());
    }

    id_t result = 0;
    const auto idString = uri.substr(i, end - i);

    if (auto [_, ec] = std::from_chars(idString.data(), idString.data() + idString.size(), result);
        ec == std::errc()) {
        return result;
    }

    const std::string msg = "Can not convert id: " + std::string(idString) + " from uri: "
                            + std::string(uri) + " to long type.";
    throw OsmObjectHelperException(msg.c_str());
}

//__________________________________________________________________________________________________
olu::osm::OsmObjectType
olu::osm::OsmObjectHelper::parseOsmTypeFromUri(const std::string_view &uri) {
    if (uri.empty()) {
        const std::string msg = "Cannot parse type from empty uri.";
        throw OsmObjectHelperException(msg.c_str());
    }

    if (uri.starts_with(cnst::NAMESPACE_IRI_OSM_NODE)) {
        return OsmObjectType::NODE;
    }

    if (uri.starts_with(cnst::NAMESPACE_IRI_OSM_WAY)) {
        return OsmObjectType::WAY;
    }

    if (uri.starts_with(cnst::NAMESPACE_IRI_OSM_REL)) {
        return OsmObjectType::RELATION;
    }

    const std::string msg = "Cant extract osm type from uri: " + std::string(uri);
    throw OsmObjectHelperException(msg.c_str());
}

// _________________________________________________________________________________________________
olu::lon_lat_t
olu::osm::OsmObjectHelper::parseLonLatFromWktPoint(const std::string_view &wktPoint) {
    if (wktPoint.empty()) {
        const std::string msg = "Cannot parse type from empty WKT point.";
        throw OsmObjectHelperException(msg.c_str());
    }

    // Find position of brackets
    const auto start = wktPoint.find('(');
    const auto end = wktPoint.find(')');
    if (start == std::string_view::npos || end == std::string_view::npos) {
        const std::string msg = "WKT point is not correctly formatted: " + std::string(wktPoint);
        throw OsmObjectHelperException(msg.c_str());
    }

    // Extract the coordinates from the WKT point string
    const std::string_view coords_view = wktPoint.substr(start + 1, end - start - 1);

    // Find the end of the longitude part (the first whitespace).
    const auto lon_end = coords_view.find_first_of(WHITESPACE);
    if (lon_end == std::string_view::npos) {
        const std::string msg = "Cannot parse lon/lat from WKT point (no separator found): "
                                + std::string(wktPoint);
        throw OsmObjectHelperException(msg.c_str());
    }

    // Find the start of the latitude part (the first non-whitespace after the longitude).
    const auto lat_start = coords_view.find_first_not_of(WHITESPACE, lon_end);
    if (lat_start == std::string_view::npos) {
        const std::string msg = "Cannot parse lat from WKT point (missing second coordinate): "
                                + std::string(wktPoint);
        throw OsmObjectHelperException(msg.c_str());
    }

    // Extract the longitude and latitude as string_views.
    const std::string_view lon_sv = coords_view.substr(0, lon_end);
    const std::string_view lat_sv = coords_view.substr(lat_start);

    if (lon_sv.empty() || lat_sv.empty()) {
        const std::string msg = "Cannot parse lat from WKT point (coordinate is empty): "
                                + std::string(wktPoint);
        throw OsmObjectHelperException(msg.c_str());
    }

    return {std::string(lon_sv), std::string(lat_sv)};}

// _________________________________________________________________________________________________
olu::osm::ChangeAction
olu::osm::OsmObjectHelper::getChangeAction(const osmium::OSMObject &osmObject) {
    if (osmObject.deleted()) { return ChangeAction::DELETE; }
    if (osmObject.version() == 1) { return ChangeAction::CREATE; }
    return ChangeAction::MODIFY;
}
