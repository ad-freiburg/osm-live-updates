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

#ifndef OSM_LIVE_UPDATES_WAY_H
#define OSM_LIVE_UPDATES_WAY_H
#include <utility>
#include "string"

#include "util/Types.h"
#include "osmium/osm/location.hpp"

namespace olu::osm {

    class Way {
    public:
        explicit Way(u_id id, WayMembers members): id(id), members(members) {};

        std::string get_xml();

        WayMembers get_members() { return members; };
        [[nodiscard]] u_id get_id() const { return id; };
    protected:
        u_id id;
        WayMembers members;
    };

    /**
     * Exception that can appear inside the `Node` class.
     */
    class WayException : public std::exception {
    private:
        std::string message;
    public:
        explicit WayException(const char* msg) : message(msg) { }

        [[nodiscard]] const char* what() const noexcept override {
            return message.c_str();
        }
    };

}

#endif //OSM_LIVE_UPDATES_WAY_H
