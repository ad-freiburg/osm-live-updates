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

#include <iostream>
#include <string>

#include "osmium/osm/object.hpp"
#include <boost/tokenizer.hpp>

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
    auto [_, ec] = std::from_chars(idString.data(), idString.data() + idString.size(), result);
    if (ec == std::errc()) {
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
    const std::string_view coordsView = wktPoint.substr(start + 1, end - start - 1);

    // Find the end of the longitude part (the first whitespace).
    const auto lonEnd = coordsView.find_first_of(WHITESPACE);
    if (lonEnd == std::string_view::npos) {
        const std::string msg = "Cannot parse lon/lat from WKT point (no separator found): "
                                + std::string(wktPoint);
        throw OsmObjectHelperException(msg.c_str());
    }

    // Find the start of the latitude part (the first non-whitespace after the longitude).
    const auto latStart = coordsView.find_first_not_of(WHITESPACE, lonEnd);
    if (latStart == std::string_view::npos) {
        const std::string msg = "Cannot parse lat from WKT point (missing second coordinate): "
                                + std::string(wktPoint);
        throw OsmObjectHelperException(msg.c_str());
    }

    // Extract the longitude and latitude as string_views.
    const std::string_view lonView = coordsView.substr(0, lonEnd);
    const std::string_view latView = coordsView.substr(latStart);

    if (lonView.empty() || latView.empty()) {
        const std::string msg = "Cannot parse lat from WKT point (coordinate is empty): "
                                + std::string(wktPoint);
        throw OsmObjectHelperException(msg.c_str());
    }

    return {std::string(lonView), std::string(latView)};}

// _________________________________________________________________________________________________
// Temporary struct to hold way member information for sorting
struct WayMemberInfo {
    int position;
    std::string_view uriView;

    // Make it comparable for sorting
    auto operator<=>(const WayMemberInfo&) const = default;
};

// _________________________________________________________________________________________________
olu::member_ids_t
olu::osm::OsmObjectHelper::parseWayMemberList(const std::string_view &uriList,
                                              const std::string_view &positionList) {
    if (uriList.empty() || positionList.empty()) {
        throw OsmObjectHelperException("Cannot parse way member list from empty strings.");
    }

    std::vector<WayMemberInfo> tempMembers;
    tempMembers.reserve(std::ranges::count(uriList, ';') + 1);

    size_t uriPos = 0;
    size_t posPos = 0;
    while (uriPos < uriList.size()) {
        const auto nextUriDelim = uriList.find(';', uriPos);
        const auto nextPosDelim = positionList.find(';', posPos);

        const auto uriToken = uriList.substr(uriPos, nextUriDelim - uriPos);
        const auto posToken = positionList.substr(posPos, nextPosDelim - posPos);

        if (uriToken.empty() || posToken.empty()) {
            throw OsmObjectHelperException("Invalid uri or position list,"
                                           " when parsing way member list.");
        }

        int currentPos;
        auto [ptr, ec] = std::from_chars(posToken.data(),
                                                        posToken.data() + posToken.size(),
                                                        currentPos);
        if (ec != std::errc()) {
            throw OsmObjectHelperException("Invalid character in position list,"
                                           " when parsing way member list.");
        }

        tempMembers.emplace_back(currentPos, uriToken);

        // Break if we've processed the last token.
        if (nextUriDelim == std::string_view::npos) break;

        uriPos = nextUriDelim + 1;
        posPos = nextPosDelim + 1;
    }

    // sort the members by their position
    std::ranges::sort(tempMembers, {}, &WayMemberInfo::position);

    member_ids_t members;
    members.reserve(tempMembers.size());

    for (const auto&[position, uri_view] : tempMembers) {
        members.emplace_back(parseIdFromUri(uri_view));
    }

    return members;
}

// _________________________________________________________________________________________________
// Temporary struct to hold relation member information for sorting
struct RelationMemberInfo {
    int position;
    std::string_view uriView;
    std::string_view role;

    // Make it comparable for sorting
    auto operator<=>(const RelationMemberInfo&) const = default;
};

// _________________________________________________________________________________________________
olu::osm::relation_members_t
olu::osm::OsmObjectHelper::parseRelationMemberList(const std::string_view &uriList,
                                                   const std::string_view &rolesList,
                                                   const std::string_view &positionList) {
    if (uriList.empty() || rolesList.empty() || positionList.empty()) {
        throw OsmObjectHelperException("Cannot parse way member list from empty strings.");
    }

    std::vector<RelationMemberInfo> tempMembers;
    tempMembers.reserve(std::ranges::count(uriList, ';') + 1);

    size_t uriPos = 0;
    size_t rolePos = 0;
    size_t posPos = 0;
    while (uriPos < uriList.size()) {
        const auto nextUriDelim = uriList.find(';', uriPos);
        const auto nextRoleDelim = rolesList.find(';', rolePos);
        const auto nextPosDelim = positionList.find(';', posPos);

        const auto uriToken = uriList.substr(uriPos, nextUriDelim - uriPos);
        const auto roleToken = rolesList.substr(rolePos, nextRoleDelim - rolePos);
        const auto posToken = positionList.substr(posPos, nextPosDelim - posPos);

        if (uriToken.empty() || roleToken.empty() || posToken.empty()) {
            throw OsmObjectHelperException("Invalid uri or position list,"
                                           " when parsing way member list.");
        }

        int currentPos;
        auto [ptr, ec] = std::from_chars(posToken.data(),
                                                        posToken.data() + posToken.size(),
                                                        currentPos);
        if (ec != std::errc()) {
            throw OsmObjectHelperException("Invalid character in position list,"
                                           " when parsing way member list.");
        }

        tempMembers.emplace_back(currentPos, uriToken, roleToken);

        // Break if we've processed the last token.
        if (nextUriDelim == std::string_view::npos) break;

        uriPos = nextUriDelim + 1;
        rolePos = nextRoleDelim + 1;
        posPos = nextPosDelim + 1;
    }

    // sort the members by their position
    std::ranges::sort(tempMembers, {}, &RelationMemberInfo::position);

    relation_members_t members;
    members.reserve(tempMembers.size());

    for (const auto&[position, uri_view, role_view] : tempMembers) {
        const auto memberId = parseIdFromUri(uri_view);
        const auto memberType = parseOsmTypeFromUri(uri_view);
        members.emplace_back(memberId, memberType, role_view);
    }

    return members;
}

// _________________________________________________________________________________________________
olu::osm::ChangeAction
olu::osm::OsmObjectHelper::getChangeAction(const osmium::OSMObject &osmObject) {
    if (osmObject.deleted()) { return ChangeAction::DELETE; }
    if (osmObject.version() == 1) { return ChangeAction::CREATE; }
    return ChangeAction::MODIFY;
}

// _________________________________________________________________________________________________
std::string olu::osm::OsmObjectHelper::parseOsm2rdfOptionName(std::string_view optionIRI) {
    if (optionIRI.empty()) {
       throw OsmObjectHelperException("Cannot parse option name from empty string.");
    }

    if (optionIRI.starts_with('<')) {
        optionIRI = optionIRI.substr(1, optionIRI.size() - 1);
    }

    if (optionIRI.ends_with('>')) {
        optionIRI = optionIRI.substr(0, optionIRI.size() - 1);
    }

    if (!optionIRI.starts_with(cnst::NAMESPACE_IRI_OSM2RDF_META)) {
        const std::string msg = "Invalid osm2rdf option IRI: " + std::string(optionIRI);
        throw OsmObjectHelperException(msg.c_str());
    }

    auto optionName = std::string(optionIRI.substr(cnst::NAMESPACE_IRI_OSM2RDF_META.size()));
    if (optionName.empty()) {
        throw OsmObjectHelperException("Empty osm2rdf option name.");
    }

    return optionName;
}