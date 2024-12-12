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

#ifndef OSM_LIVE_UPDATES_URLHELPER_H
#define OSM_LIVE_UPDATES_URLHELPER_H

#include <string>
#include <vector>

namespace olu::util {

class URLHelper {
public:
    // Builds an url from a list of strings by concatenating them with a '/'
    static std::string buildUrl(std::vector<std::string> &pathSegments);

    // Formats a sequence number for use in an url
    // For example: The sequence number 6177383 would be returned as 006/177/383
    // @throw 'std::invalid_argument' if sequence number is empty or too long
    static std::string formatSequenceNumberForUrl(int &sequenceNumber);

    // Url encodes the given string
    static std::string encodeForUrlQuery(const std::string& value);

    static bool isValidUri(const std::string& uri);
};

} // namespace olu::util

#endif //OSM_LIVE_UPDATES_URLHELPER_H
