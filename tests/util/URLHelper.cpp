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

namespace olu::util {

// _________________________________________________________________________________________________
TEST(URLHelper, formatSequenceNumber) {
    {
        std::string seqNumber = "6177383";
        std::string formattedSeqNumber = URLHelper::formatSequenceNumberForUrl(seqNumber);
        ASSERT_EQ(formattedSeqNumber, "006/177/383");
    }
    {
        std::string seqNumber = "116177383";
        std::string formattedSeqNumber = URLHelper::formatSequenceNumberForUrl(seqNumber);
        ASSERT_EQ(formattedSeqNumber, "116/177/383");
    }
    {
        EXPECT_THROW({
             try {
                 std::string seqNumber = "1234567890";
                 std::string formattedSeqNumber = URLHelper::formatSequenceNumberForUrl(seqNumber);
             } catch(const std::invalid_argument& e) {
                 EXPECT_STREQ(constants::EXCEPTION_MSG_SEQUENCE_NUMBER_IS_TOO_LONG, e.what());
                 throw;
             }}, std::invalid_argument);
    }
    {
        EXPECT_THROW({
             try {
                 std::string seqNumber;
                 std::string formattedSeqNumber = URLHelper::formatSequenceNumberForUrl(seqNumber);
             } catch(const std::invalid_argument& e) {
                 EXPECT_STREQ(constants::EXCEPTION_MSG_SEQUENCE_NUMBER_IS_EMPTY, e.what());
                 throw;
             }}, std::invalid_argument);
    }
}

// _________________________________________________________________________________________________
TEST(URLHelper, buildUrl) {
    {
        std::vector<std::string> pathSegments;
        pathSegments.emplace_back(constants::OSM_REPLICATION_BASE_URL);
        pathSegments.emplace_back("state.txt");
        std::string url = URLHelper::buildUrl(pathSegments);
        ASSERT_EQ(url, constants::OSM_REPLICATION_BASE_URL + "/" + "state.txt");
    }
    {
        std::vector<std::string> pathSegments;
        std::string url = URLHelper::buildUrl(pathSegments);
        ASSERT_EQ(url, "");
    }
}


} // namespace olu::util

