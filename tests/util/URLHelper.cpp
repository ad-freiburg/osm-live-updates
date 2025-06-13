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
#include "gtest/gtest.h"

#include <stdexcept>

namespace constants = olu::config::constants;

// _________________________________________________________________________________________________
TEST(URLHelper, formatSequenceNumber) { {
        int seqNumber = 6177383;
        std::string formattedSeqNumber =
                olu::util::URLHelper::formatSequenceNumberForUrl(seqNumber);
        ASSERT_EQ(formattedSeqNumber, "006/177/383");
    } {
        int seqNumber = 116177383;
        std::string formattedSeqNumber =
                olu::util::URLHelper::formatSequenceNumberForUrl(seqNumber);
        ASSERT_EQ(formattedSeqNumber, "116/177/383");
    } {
        EXPECT_THROW({
                     try {
                     int seqNumber = 1234567890;
                     std::string formattedSeqNumber = olu::util::URLHelper::
                     formatSequenceNumberForUrl(seqNumber);
                     } catch(const std::invalid_argument& e) {
                     EXPECT_STREQ(constants::EXCEPTION_MSG_SEQUENCE_NUMBER_IS_INVALID, e.what());
                     throw;
                     }}, std::invalid_argument);
    } {
        EXPECT_THROW({
                     try {
                     int seqNumber = -1 ;
                     std::string formattedSeqNumber = olu::util::URLHelper::
                     formatSequenceNumberForUrl(seqNumber);
                     } catch(const std::invalid_argument& e) {
                     EXPECT_STREQ(constants::EXCEPTION_MSG_SEQUENCE_NUMBER_IS_INVALID, e.what());
                     throw;
                     }}, std::invalid_argument);
    }
}

// _________________________________________________________________________________________________
TEST(URLHelper, buildUrl) { {
        std::vector<std::string> pathSegments;
        pathSegments.emplace_back("https://www.openstreetmap.org/api/0.6/node");
        pathSegments.emplace_back("state.txt");
        const std::string url = olu::util::URLHelper::buildUrl(pathSegments);
        ASSERT_EQ(url, "https://www.openstreetmap.org/api/0.6/node/state.txt");
    } {
        const std::vector<std::string> pathSegments;
        const std::string url = olu::util::URLHelper::buildUrl(pathSegments);
        ASSERT_EQ(url, "");
    }
}
// _________________________________________________________________________________________________
TEST(URLHelper, encodeForUrlQuery) {
    {
        const std::string input = "https://www.openstreetmap.org/api/0.6/node/123456789";
        const std::string encoded = olu::util::URLHelper::encodeForUrlQuery(input);
        ASSERT_EQ(encoded, "https%3A%2F%2Fwww.openstreetmap.org%2Fapi%2F0.6%2Fnode%2F123456789");
    }
    {
        const std::string input = "Hello World!";
        const std::string encoded = olu::util::URLHelper::encodeForUrlQuery(input);
        ASSERT_EQ(encoded, "Hello%20World%21");
    }
    {
        const std::string input = "Special characters: & ? = #";
        const std::string encoded = olu::util::URLHelper::encodeForUrlQuery(input);
        ASSERT_EQ(encoded, "Special%20characters%3A%20%26%20%3F%20%3D%20%23");
    }
    {
        const std::string input = "";
        const std::string encoded = olu::util::URLHelper::encodeForUrlQuery(input);
        ASSERT_EQ(encoded, "");
    }
}