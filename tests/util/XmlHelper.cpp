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

#include "util/XmlHelper.h"

#include "gtest/gtest.h"

// _________________________________________________________________________________________________
TEST(XmlHelper, getNodeDummy) {
    constexpr olu::id_t nodeId = 123456789;
    const olu::lon_lat_t lonLat = {"13.5690032", "42.7957187"};
    const std::string expectedXml = "<node id=\"123456789\" lat=\"42.7957187\" lon=\"13.5690032\"/>";

    const std::string xml = olu::util::XmlHelper::getNodeDummy(nodeId, lonLat);
    EXPECT_EQ(expectedXml, xml);
}

// _________________________________________________________________________________________________
TEST(XmlHelper, getWayDummy) {
    constexpr olu::id_t wayId = 987654321;
    const olu::member_ids_t memberIds = {1, 2, 3};
    const std::string expectedXml = "<way id=\"987654321\">"
                                    "<nd ref=\"1\"/><nd ref=\"2\"/>"
                                    "<nd ref=\"3\"/>"
                                    "<tag k=\"K\" v=\"V\"/>"
                                    "</way>";

    const std::string xml = olu::util::XmlHelper::getWayDummy(wayId, memberIds);
    EXPECT_EQ(expectedXml, xml);
}

// _________________________________________________________________________________________________
TEST(XmlHelper, getRelationDummy) {
    constexpr olu::id_t relationId = 135792468;
    const olu::osm::RelationMember member1(1, olu::osm::OsmObjectType::NODE, std::string("role1"));
    const olu::osm::RelationMember member2(2, olu::osm::OsmObjectType::WAY, std::string("role2"));
    const olu::osm::RelationMember member3(3, olu::osm::OsmObjectType::RELATION,
                                           std::string("role3"));
    const std::vector members = {member1, member2, member3};
    const std::string expectedXml = "<relation id=\"135792468\">"
                                    "<member type=\"node\" ref=\"1\" role=\"role1\"/>"
                                    "<member type=\"way\" ref=\"2\" role=\"role2\"/>"
                                    "<member type=\"relation\" ref=\"3\" role=\"role3\"/>"
                                    "<tag k=\"type\" v=\"relationType\"/>"
                                    "</relation>";

    const std::string xml = olu::util::XmlHelper::getRelationDummy(
        relationId, "relationType", members);
    EXPECT_EQ(expectedXml, xml);
}