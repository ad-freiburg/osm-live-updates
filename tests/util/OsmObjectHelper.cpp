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

#include "util/OsmObjectHelper.h"
#include "gtest/gtest.h"

TEST(OsmObjectHelper, createDummyNodeFromPoint) {
    {
        std::string pointAsWkt = "POINT(13.5690032 42.7957187)";
        int nodeId = 1;

        auto dummyNode = olu::osm::OsmObjectHelper::createNodeFromPoint(nodeId, pointAsWkt);
        ASSERT_EQ(dummyNode,
                  "<node id=\"1\" lat=\"42.7957187\" lon=\"13.5690032\"/>");
    }
    {
        std::string pointAsWkt = "POINT(42.7957187)";
        int nodeId = 1;

        ASSERT_THROW(
                olu::osm::OsmObjectHelper::createNodeFromPoint(nodeId, pointAsWkt),
                olu::osm::WktHelperException);
    }
    {
        std::string pointAsWkt = "POINT(13.5690032,42.7957187)";
        int nodeId = 1;

        ASSERT_THROW(
                olu::osm::OsmObjectHelper::createNodeFromPoint(nodeId, pointAsWkt),
                olu::osm::WktHelperException);
    }
    {
        std::string pointAsWkt = "(13.5690032 42.7957187)";
        int nodeId = 1;

        ASSERT_THROW(
                olu::osm::OsmObjectHelper::createNodeFromPoint(nodeId, pointAsWkt),
                olu::osm::WktHelperException);
    }
    {
        std::string pointAsWkt = "13.5690032 42.7957187";
        int nodeId = 1;

        ASSERT_THROW(
                olu::osm::OsmObjectHelper::createNodeFromPoint(nodeId, pointAsWkt),
                olu::osm::WktHelperException);
    }
}

TEST(OsmObjectHelper, createWayFromReferences) {
    ASSERT_EQ(olu::osm::OsmObjectHelper::createWayFromReferences(1, {1, 2, 3}),
              "<way id=\"1\">"
              "<nd ref=\"1\"/>"
              "<nd ref=\"2\"/>"
              "<nd ref=\"3\"/>"
              "</way>"
    );
}
TEST(OsmObjectHelper, createRelationFromReferences) {
    ASSERT_EQ(olu::osm::OsmObjectHelper::createRelationFromReferences(
            1,
            { std::pair("https://www.openstreetmap.org/node/1", "member"),
              std::pair("https://www.openstreetmap.org/way/2", "outer"),
              std::pair("https://www.openstreetmap.org/relation/3", "inner") }),
              "<relation id=\"1\">"
              "<member type=\"node\" ref=\"1\" role=\"member\"/>"
              "<member type=\"way\" ref=\"2\" role=\"outer\"/>"
              "<member type=\"relation\" ref=\"3\" role=\"inner\"/>"
              "</relation>"
    );
}