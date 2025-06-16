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

#ifndef OSM_LIVE_UPDATES_OSMDATABASESTATE_H
#define OSM_LIVE_UPDATES_OSMDATABASESTATE_H

#include <string>

#include "util/Logger.h"

namespace olu::osm {

    struct OsmDatabaseState {
        std::string timeStamp;
        int sequenceNumber;
    };

inline std::string to_string(const OsmDatabaseState& state) {
    std::ostringstream oss;
    oss.imbue(util::commaLocale);

    if (state.timeStamp.empty()) {
        oss << "(Sequence number: " << state.sequenceNumber << ")";
        return oss.str();
    }

    std::string timestampFormatted = state.timeStamp;
    std::erase_if(timestampFormatted, [](const char c) {
        return c == '\\';
    });

    oss << "(Sequence number: " << state.sequenceNumber
        << ", Timestamp: " << timestampFormatted << ")";
    return oss.str();
}

} // namespace olu::osm

#endif //OSM_LIVE_UPDATES_OSMDATABASESTATE_H
