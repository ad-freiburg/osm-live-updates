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

#include <stdexcept>
#include <iostream>
#include <iomanip>

#include "boost/regex.hpp"

#include "config/Constants.h"

static inline constexpr int MIN_SEQ_NUMBER = 0;
static inline constexpr int MAX_SEQ_NUMBER = 999999999;

static inline constexpr int FORMATTED_SEQ_NUMBER_LENGTH = 9;
static inline constexpr int SEGMENT_LENGTH = 3;

static inline constexpr int START_POS_FIRST_SEGMENT = 0;
static inline constexpr int START_POS_SECOND_SEGMENT = 3;
static inline constexpr int START_POS_THIRD_SEGMENT = 6;

namespace constants = olu::config::constants;

// _________________________________________________________________________________________________
std::string olu::util::URLHelper::buildUrl(const std::vector<std::string> &pathSegments) {
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
std::string olu::util::URLHelper::formatSequenceNumberForUrl(const int &sequenceNumber) {
    if (sequenceNumber < MIN_SEQ_NUMBER || sequenceNumber > MAX_SEQ_NUMBER) {
        throw std::invalid_argument(constants::EXCEPTION_MSG_SEQUENCE_NUMBER_IS_INVALID);
    }

    std::string sequenceNumberStr = std::to_string(sequenceNumber);
    while (sequenceNumberStr.length() < FORMATTED_SEQ_NUMBER_LENGTH) {
        sequenceNumberStr.insert(0, "0");
    }

    // Format sequence number for a file path which looks like XXX/XXX/XXX
    const std::string firstSegment = sequenceNumberStr.substr(START_POS_FIRST_SEGMENT,
                                                              SEGMENT_LENGTH);
    const std::string secondSegment = sequenceNumberStr.substr(START_POS_SECOND_SEGMENT,
                                                               SEGMENT_LENGTH);
    const std::string thirdSegment = sequenceNumberStr.substr(START_POS_THIRD_SEGMENT,
                                                              SEGMENT_LENGTH);
    return firstSegment + "/" + secondSegment + "/" + thirdSegment;
}

// _________________________________________________________________________________________________
std::string olu::util::URLHelper::encodeForUrlQuery(const std::string &value) {
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
        escaped << '%' << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
    }

    return escaped.str();
}

// _________________________________________________________________________________________________
bool olu::util::URLHelper::isValidUri(const std::string &uri) {
    const boost::regex regex(R"(((\w+:\/\/)[-a-zA-Z0-9:@;?&=\/%\+\.\*!'\(\),\$_\{\}\^~\[\]`#|]+))");
    return regex_match (uri, regex);
}
