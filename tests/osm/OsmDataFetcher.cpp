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

#include "osm/OsmDataFetcher.h"
#include "config/Constants.h"
#include "gtest/gtest.h"
#include "sparql/SparqlWrapper.h"

namespace olu {
    TEST(OsmDataFetcher, fetchLatestSequenceNumber) {
        auto config = config::Config();
        auto sw = sparql::SparqlWrapper(config);
        auto osmDataFetcher = olu::osm::OsmDataFetcher(config);
        auto response = osmDataFetcher.fetchLatestSequenceNumber();

        ASSERT_TRUE(response.length() > 0);
    }

    TEST(OsmDataFetcher, fetchNode) {
        std::string nodeId = "1";
        auto response = olu::osm::OsmDataFetcher::fetchNode(nodeId);

        ASSERT_TRUE(response.length() > 0);
    }
}