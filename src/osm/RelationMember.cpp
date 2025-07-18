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

#include "osm/RelationMember.h"

#include <iostream>

#include "config/Constants.h"
#include "osm/OsmObjectHelper.h"

namespace cnst = olu::config::constants;

// _________________________________________________________________________________________________
olu::osm::RelationMember::RelationMember(const id_t &id, const std::string &memberNamespace,
                                         const std::string &role) {
    this->id = id;
    this->role = role;

    if (memberNamespace == config::constants::NAMESPACE_OSM_NODE) {
        this->type = OsmObjectType::NODE;
    } else if (memberNamespace == config::constants::NAMESPACE_OSM_WAY) {
        this->type = OsmObjectType::WAY;
    } else if (memberNamespace == config::constants::NAMESPACE_OSM_REL) {
        this->type = OsmObjectType::RELATION;
    } else {
        const std::string msg = "Cannot initialize relation member from unknown namespace:"
                                " " + memberNamespace;
        throw RelationMemberException(msg.c_str());
    }
}

// _________________________________________________________________________________________________
olu::osm::RelationMember::RelationMember(const std::string &memberUri, const std::string &role) {
    this->role = role;

    try {
        this->id = OsmObjectHelper::parseIdFromUri(memberUri);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        const std::string msg = "Cannot extract relation id from uri: " + memberUri;
        throw RelationMemberException(msg.c_str());
    }

    if (memberUri.starts_with(config::constants::NAMESPACE_IRI_OSM_NODE)) {
        this->type = OsmObjectType::NODE;
    } else if (memberUri.starts_with(config::constants::NAMESPACE_IRI_OSM_WAY)) {
        this->type = OsmObjectType::WAY;
    } else if (memberUri.starts_with(config::constants::NAMESPACE_IRI_OSM_REL)) {
        this->type = OsmObjectType::RELATION;
    } else {
        const std::string msg = "Cannot initialize relation member from unknown namespace:"
                                " " + memberUri;
        throw RelationMemberException(msg.c_str());
    }
}

// _________________________________________________________________________________________________
olu::osm::RelationMember::RelationMember(const id_t &id, const osmium::item_type &memberType,
                                         const std::string &role) {
    this->id = id;
    this->role = role;

    switch (memberType) {
        case osmium::item_type::node:
            this->type = OsmObjectType::NODE;
        break;
        case osmium::item_type::way:
            this->type = OsmObjectType::WAY;
        break;
        case osmium::item_type::relation:
            this->type = OsmObjectType::RELATION;
        break;
        default:
            const std::string msg = "Cannot initialize relation member from unknown osmium "
                                    "item type";
        throw RelationMemberException(msg.c_str());
    }
}

// _________________________________________________________________________________________________
bool olu::osm::RelationMember::areRelMemberEqual(std::vector<RelationMember> member1,
                                                 std::vector<RelationMember> member2) {
    return member1.size() == member2.size() &&
           std::equal(member1.begin(), member1.end(), member2.begin(),
                      [](const RelationMember &a, const RelationMember &b) {
                          return a.id == b.id &&
                                 a.type == b.type &&
                                 a.role == b.role;
                      });
}
