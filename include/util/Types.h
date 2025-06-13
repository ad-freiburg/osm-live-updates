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

#ifndef OSM_LIVE_UPDATES_TYPES_H
#define OSM_LIVE_UPDATES_TYPES_H

#include <chrono>
#include <cstdint>
#include <string>
#include <tuple>
#include <vector>

namespace olu {

    typedef int64_t id_t;
    typedef uint32_t changeset_id_t;
    typedef uint32_t version_t;
    typedef std::vector<id_t> member_ids_t;

    typedef std::string wktPoint_t;
    typedef std::pair<std::string, std::string> lon_lat_t;

    typedef std::tuple<std::string, std::string, std::string> triple_t;
    typedef std::pair<std::string, std::string> key_value_t;

    typedef std::chrono::time_point<std::chrono::system_clock> time_point_t;
}

#endif //OSM_LIVE_UPDATES_TYPES_H
