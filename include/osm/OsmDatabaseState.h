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

#include <regex>
#include <string>

#include "util/Logger.h"

inline std::string formatTimestamp(const std::string& timestamp) {
    std::string timestampFormatted = timestamp;
    std::erase_if(timestampFormatted, [](const char c) {
        return c == '\\';
    });
    return timestampFormatted;
}

namespace olu::osm {
    struct OsmDatabaseState {
        std::string timeStamp;
        int sequenceNumber = -1;

        // Compare database states based on the timestamp as the sequence number can vary
        // depending on which replication server is used (e.g., minute diffs will have another
        // numbering as daily diffs, but the timestamp can still be used to compare them).
        bool operator<(const OsmDatabaseState& other) const {
            return formatTimestamp(timeStamp) < formatTimestamp(other.timeStamp);
        }

        bool operator>(const OsmDatabaseState& other) const {
            return formatTimestamp(timeStamp) > formatTimestamp(other.timeStamp);
        }

        bool operator>=(const OsmDatabaseState& other) const {
            return formatTimestamp(timeStamp) >= formatTimestamp(other.timeStamp);
        }

        bool operator<=(const OsmDatabaseState& other) const {
            return formatTimestamp(timeStamp) <= formatTimestamp(other.timeStamp);
        }

        bool operator==(const OsmDatabaseState& other) const {
            return formatTimestamp(timeStamp) == formatTimestamp(other.timeStamp);
        }
    };

    inline std::string to_string(const OsmDatabaseState& state) {
        std::ostringstream oss;
        oss.imbue(util::commaLocale);

        if (state.timeStamp.empty()) {
            oss << "(Sequence number: " << state.sequenceNumber << ")";
            return oss.str();
        }

        oss << "(Sequence number: " << state.sequenceNumber
            << ", Timestamp: " << formatTimestamp(state.timeStamp) << ")";
        return oss.str();
    }

    inline OsmDatabaseState from_string(const std::string& str) {
        OsmDatabaseState state;

        const std::regex regex(R"(\(Sequence number: ([\d,]+), Timestamp: (.+)\))");
        if (std::smatch match; std::regex_search(str, match, regex) && match.size() == 3) {
            std::string seqNumStr = match[1].str();
            // Remove the thousand separator commas from the sequence number string
            std::erase(seqNumStr, ',');
            state.sequenceNumber = std::stoi(seqNumStr);
            state.timeStamp = match[2].str();
        } else {
            throw std::runtime_error("Failed to parse OsmDatabaseState from string: " + str);
        }

        return state;
    }

} // namespace olu::osm

#endif //OSM_LIVE_UPDATES_OSMDATABASESTATE_H
