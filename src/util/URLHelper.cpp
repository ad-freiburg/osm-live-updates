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

const static inline int MIN_SEQ_NUMBER = 0;
const static inline int MAX_SEQ_NUMBER = 999999999;

const static inline int FORMATTED_SEQ_NUMBER_LENGTH = 9;
const static inline int SEGMENT_LENGTH = 3;

const static inline int START_POS_FIRST_SEGMENT = 0;
const static inline int START_POS_SECOND_SEGMENT = 3;
const static inline int START_POS_THIRD_SEGMENT = 6;

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
std::string URLHelper::formatSequenceNumberForUrl(int &sequenceNumber) {
    if (sequenceNumber < MIN_SEQ_NUMBER || sequenceNumber > MAX_SEQ_NUMBER) {
        throw std::invalid_argument(constants::EXCEPTION_MSG_SEQUENCE_NUMBER_IS_INVALID);
    }

    std::string sequenceNumberStr = std::to_string(sequenceNumber);
    while (sequenceNumberStr.length() < FORMATTED_SEQ_NUMBER_LENGTH) {
        sequenceNumberStr.insert(0, "0");
    }

    // Format sequence number for file path which looks like XXX/XXX/XXX
    std::string firstSegment = sequenceNumberStr.substr(START_POS_FIRST_SEGMENT, SEGMENT_LENGTH);
    std::string secondSegment = sequenceNumberStr.substr(START_POS_SECOND_SEGMENT, SEGMENT_LENGTH);
    std::string thirdSegment = sequenceNumberStr.substr(START_POS_THIRD_SEGMENT, SEGMENT_LENGTH);
    return firstSegment + "/" + secondSegment + "/" + thirdSegment;
}

// _________________________________________________________________________________________________
std::string URLHelper::encodeForUrlQuery(const std::string &value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (const char c : value) {
        // Keep alphanumeric characters and other allowed characters
        if (isalnum(c) != 0 || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }
        // Any other characters are percent-encoded
        escaped << '%' << std::setw(2) << int((unsigned char)c);
    }

    return escaped.str();
}

} // namespace olu::util