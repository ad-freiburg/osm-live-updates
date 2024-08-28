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

#include "util/URLHelper.h"
#include "config/Constants.h"

#include <stdexcept>
#include <boost/asio/connect.hpp>
#include <iostream>
#include <iomanip>

namespace constants = olu::config::constants;

namespace olu::util {

// _________________________________________________________________________________________________
std::string URLHelper::buildUrl(std::vector<std::string> &pathSegments) {
    std::string url;
    for( const auto& segment : pathSegments ) {
        if(&segment == &pathSegments.back() ) {
            url.append(segment);
        } else {
            url.append(segment + "/");
        }
    }

    return url;
}

// _________________________________________________________________________________________________
std::string URLHelper::formatSequenceNumberForUrl(std::string &sequenceNumber) {
    if (sequenceNumber.length() < 1)
        throw std::invalid_argument(constants::EXCEPTION_MSG_SEQUENCE_NUMBER_IS_EMPTY);

    if (sequenceNumber.length() > 9)
        throw std::invalid_argument(constants::EXCEPTION_MSG_SEQUENCE_NUMBER_IS_TOO_LONG);

    while (sequenceNumber.length() < 9)
        sequenceNumber.insert(0, "0");

    std::string part1 = sequenceNumber.substr(0, 3);
    std::string part2 = sequenceNumber.substr(3, 3);
    std::string part3 = sequenceNumber.substr(6, 3);
    return part1 + "/" + part2 + "/" + part3;
}

// _________________________________________________________________________________________________
std::string URLHelper::encodeForUrlQuery(const std::string &value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (char c : value) {
        // Keep alphanumeric characters and other allowed characters
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }
        // Any other characters are percent-encoded
        escaped << '%' << std::setw(2) << int((unsigned char)c);
    }

    return escaped.str();
}

} // namespace olu::util