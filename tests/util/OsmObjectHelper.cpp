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

TEST(OsmObjectHelper, createWayFromReferences) {
    ASSERT_EQ(olu::osm::OsmObjectHelper::createWayFromReferences(1, {1, 2, 3}),
              "<way id=\"1\">"
              "<nd ref=\"1\"/>"
              "<nd ref=\"2\"/>"
              "<nd ref=\"3\"/>"
              "<tag k=\"type\" v=\"tmp\"/>"
              "</way>"
    );
}
TEST(OsmObjectHelper, createRelationFromReferences) {
    std::vector<std::pair<std::string, std::string>> members = {
            std::pair("https://www.openstreetmap.org/node/1", "member"),
            std::pair("https://www.openstreetmap.org/way/2", "outer"),
            std::pair("https://www.openstreetmap.org/relation/3", "inner")};

    ASSERT_EQ(olu::osm::OsmObjectHelper::createRelationFromReferences(
            1,
            std::pair("multipolygon" , members)),
              "<relation id=\"1\">"
              "<member type=\"node\" ref=\"1\" role=\"member\"/>"
              "<member type=\"way\" ref=\"2\" role=\"outer\"/>"
              "<member type=\"relation\" ref=\"3\" role=\"inner\"/>"
              "<tag k=\"type\" v=\"multipolygon\"/>"
              "</relation>"
    );
}