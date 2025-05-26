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

#include "config/Constants.h"
#include "osm/ChangeAction.h"
#include "util/Types.h"

namespace cnst = olu::config::constants;

// _________________________________________________________________________________________________
olu::id_t olu::osm::OsmObjectHelper::parseIdFromUri(const std::string_view &uri) {
    std::vector<char> id;
    // Read characters from the end of the uri until the first non digit is reached
    for (auto it = uri.rbegin(); it != uri.rend(); ++it) {
        if (std::isdigit(*it)) {
            id.push_back(*it);
        } else {
            break;
        }
    }

    try {
        return std::stoll(std::string(id.rbegin(), id.rend()));
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        const std::string msg = "Cant extract id from uri: " + std::string(uri);
        throw OsmObjectHelperException(msg.c_str());
    }
}

//__________________________________________________________________________________________________
olu::osm::OsmObjectType olu::osm::OsmObjectHelper::parseOsmTypeFromUri(const std::string& uri) {
    if (uri.starts_with(cnst::NAMESPACE_IRI_OSM_NODE)) {
        return OsmObjectType::NODE;
    }

    if (uri.starts_with(cnst::NAMESPACE_IRI_OSM_WAY)) {
        return OsmObjectType::WAY;
    }

    if (uri.starts_with(cnst::NAMESPACE_IRI_OSM_REL)) {
        return OsmObjectType::RELATION;
    }

    const std::string msg = "Cant extract osm type from uri: " + uri;
    throw OsmObjectHelperException(msg.c_str());
}

// _________________________________________________________________________________________________
bool olu::osm::OsmObjectHelper::areWayMemberEqual(member_ids_t member1, member_ids_t member2) {
    return member1.size() == member2.size() &&
           std::equal(member1.begin(), member1.end(), member2.begin());
}

// _________________________________________________________________________________________________
olu::osm::ChangeAction
olu::osm::OsmObjectHelper::getChangeAction(const osmium::OSMObject &osmObject) {
    if (osmObject.deleted()) { return ChangeAction::DELETE; }
    if (osmObject.version() == 1) { return ChangeAction::CREATE; }
    return ChangeAction::MODIFY;
}


