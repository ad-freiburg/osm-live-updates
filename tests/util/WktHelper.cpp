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

#include "util/WktHelper.h"
#include "gtest/gtest.h"

TEST(WktHelper, createDummyNodeFromPoint) {
    {
        std::string pointAsWkt = "POINT(13.5690032 42.7957187)";
        int nodeId = 1;

        auto dummyNode = olu::osm::WktHelper::createNodeFromPoint(nodeId, pointAsWkt);
        ASSERT_EQ(dummyNode,
                  "<node id=\"1\" lat=\"42.7957187\" lon=\"13.5690032\"/>");
    }
    {
        std::string pointAsWkt = "POINT(42.7957187)";
        int nodeId = 1;

        ASSERT_THROW(
                olu::osm::WktHelper::createNodeFromPoint(nodeId, pointAsWkt),
                olu::osm::WktHelperException);
    }
    {
        std::string pointAsWkt = "POINT(13.5690032,42.7957187)";
        int nodeId = 1;

        ASSERT_THROW(
                olu::osm::WktHelper::createNodeFromPoint(nodeId, pointAsWkt),
                olu::osm::WktHelperException);
    }
    {
        std::string pointAsWkt = "(13.5690032 42.7957187)";
        int nodeId = 1;

        ASSERT_THROW(
                olu::osm::WktHelper::createNodeFromPoint(nodeId, pointAsWkt),
                olu::osm::WktHelperException);
    }
    {
        std::string pointAsWkt = "13.5690032 42.7957187";
        int nodeId = 1;

        ASSERT_THROW(
                olu::osm::WktHelper::createNodeFromPoint(nodeId, pointAsWkt),
                olu::osm::WktHelperException);
    }
}