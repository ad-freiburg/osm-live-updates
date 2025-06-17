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

#ifndef RELATIONMEMBER_H
#define RELATIONMEMBER_H

#include <string>

#include "osmium/osm/item_type.hpp"

#include "osm/OsmObjectType.h"
#include "util/Types.h"

namespace olu::osm {
    /**
     * Exception that can appear inside the `Node` class.
     */
    class RelationMemberException final : public std::exception {
        std::string message;

    public:
        explicit RelationMemberException(const char *msg) : message(msg) { }

        [[nodiscard]] const char *what() const noexcept override {
            return message.c_str();
        }
    };

    class RelationMember {
    public:
        id_t id;
        OsmObjectType type;
        std::string role;

        explicit RelationMember(const id_t &id, const OsmObjectType &memberType,
                                const std::string &role): id(id), type(memberType),
                                                   role(role) {}

        explicit RelationMember(const id_t &id, const OsmObjectType &memberType,
                                const std::string_view &role): id(id), type(memberType),
                                           role(role) {}

        explicit RelationMember(const id_t &id, const std::string &memberNamespace,
                                const std::string &role);

        explicit RelationMember(const std::string &memberUri, const std::string &role);

        explicit RelationMember(const id_t &id, const osmium::item_type &memberType,
                                const std::string &role);

        static bool areRelMemberEqual(std::vector<RelationMember> member1,
                                      std::vector<RelationMember> member2);
    };

    typedef std::vector<RelationMember> relation_members_t;
}

#endif //RELATIONMEMBER_H
