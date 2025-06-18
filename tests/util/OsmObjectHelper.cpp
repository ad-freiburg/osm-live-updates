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

#include "osm/OsmObjectHelper.h"

#include <osmium/builder/osm_object_builder.hpp>

#include "gtest/gtest.h"

// _________________________________________________________________________________________________
TEST(OsmObjectHelper, parseIdFromUri) {
    {
        constexpr std::string_view uri = "https://www.openstreetmap.org/node/123456789";
        constexpr olu::id_t expectedId = 123456789;
        const olu::id_t parsedId = olu::osm::OsmObjectHelper::parseIdFromUri(uri);

        EXPECT_EQ(expectedId, parsedId);
    }
    {
        constexpr std::string_view uri = "<https://www.openstreetmap.org/way/987654321>";
        constexpr olu::id_t expectedId = 987654321;
        const olu::id_t parsedId = olu::osm::OsmObjectHelper::parseIdFromUri(uri);

        EXPECT_EQ(expectedId, parsedId);
    }
    {
        constexpr std::string_view uri = "\"https://www.openstreetmap.org/relation/135792468\"";
        constexpr olu::id_t expectedId = 135792468;
        const olu::id_t parsedId = olu::osm::OsmObjectHelper::parseIdFromUri(uri);

        EXPECT_EQ(expectedId, parsedId);
    }
}

// _________________________________________________________________________________________________
TEST(OsmObjectHelper, parseIdFromUriInvalid) {
    {
        constexpr std::string_view uri = "";

        EXPECT_THROW(olu::osm::OsmObjectHelper::parseIdFromUri(uri),
                     olu::osm::OsmObjectHelperException);
    }
    {
        constexpr std::string_view uri = "invalid-uri";

        EXPECT_THROW(olu::osm::OsmObjectHelper::parseIdFromUri(uri),
                     olu::osm::OsmObjectHelperException);
    }
    {
        constexpr std::string_view uri = "<https://www.openstreetmap.org/node/abc>";

        EXPECT_THROW(olu::osm::OsmObjectHelper::parseIdFromUri(uri),
                     olu::osm::OsmObjectHelperException);
    }
    {
        constexpr std::string_view uri = "https://www.openstreetmap.org/way/-123456/";

        EXPECT_THROW(olu::osm::OsmObjectHelper::parseIdFromUri(uri),
                     olu::osm::OsmObjectHelperException);
    }
}

// _________________________________________________________________________________________________
TEST(OsmObjectHelper, parseOsmTypeFromUri) {
    {
        const std::string uri = "https://www.openstreetmap.org/node/123456789";
        constexpr auto expectedType = olu::osm::OsmObjectType::NODE;
        const auto parsedType = olu::osm::OsmObjectHelper::parseOsmTypeFromUri(uri);

        EXPECT_EQ(expectedType, parsedType);
    }
    {
        const std::string uri = "https://www.openstreetmap.org/way/987654321";
        constexpr auto expectedType = olu::osm::OsmObjectType::WAY;
        const auto parsedType = olu::osm::OsmObjectHelper::parseOsmTypeFromUri(uri);

        EXPECT_EQ(expectedType, parsedType);
    }
    {
        const std::string uri = "https://www.openstreetmap.org/relation/135792468";
        constexpr auto expectedType = olu::osm::OsmObjectType::RELATION;
        const auto parsedType = olu::osm::OsmObjectHelper::parseOsmTypeFromUri(uri);

        EXPECT_EQ(expectedType, parsedType);
    }
}

// _________________________________________________________________________________________________
TEST(OsmObjectHelper, parseOsmTypeFromUriInvalid) {
    {
        const std::string uri = "";

        EXPECT_THROW(olu::osm::OsmObjectHelper::parseOsmTypeFromUri(uri),
                     olu::osm::OsmObjectHelperException);
    }
    {
        const std::string uri = "https://www.openstreetmap.org/invalid/123456789";

        EXPECT_THROW(olu::osm::OsmObjectHelper::parseOsmTypeFromUri(uri),
                     olu::osm::OsmObjectHelperException);
    }
    {
        const std::string uri = "<https://www.openstreetmap.org/node/abc>";

        EXPECT_THROW(olu::osm::OsmObjectHelper::parseOsmTypeFromUri(uri),
                     olu::osm::OsmObjectHelperException);
    }
    {
        const std::string uri = "\"https://www.openstreetmap.org/way/-123456\"";

        EXPECT_THROW(olu::osm::OsmObjectHelper::parseOsmTypeFromUri(uri),
                     olu::osm::OsmObjectHelperException);
    }
}

// _________________________________________________________________________________________________
TEST(OsmObjectHelper, parseLonLatFromWktPoint) {
    {
        constexpr std::string_view wktPoint = "POINT (8.6296398 53.1494628)";
        olu::lon_lat_t expected = {"8.6296398", "53.1494628"};
        const auto result = olu::osm::OsmObjectHelper::parseLonLatFromWktPoint(wktPoint);

        EXPECT_EQ(expected, result);
    }
    {
        constexpr std::string_view wktPoint = "POINT (10.1234567  54.9876543)";
        olu::lon_lat_t expected = {"10.1234567", "54.9876543"};
        const auto result = olu::osm::OsmObjectHelper::parseLonLatFromWktPoint(wktPoint);

        EXPECT_EQ(expected, result);
    }
    {
        constexpr std::string_view wktPoint = "POINT (10.1567  54.543)";
        olu::lon_lat_t expected = {"10.1567", "54.543"};
        const auto result = olu::osm::OsmObjectHelper::parseLonLatFromWktPoint(wktPoint);

        EXPECT_EQ(expected, result);
    }
    {
        constexpr std::string_view wktPoint = "POINT(10.1567  54.543)";
        olu::lon_lat_t expected = {"10.1567", "54.543"};
        const auto result = olu::osm::OsmObjectHelper::parseLonLatFromWktPoint(wktPoint);

        EXPECT_EQ(expected, result);
    }
    {
        constexpr std::string_view wktPoint = "POINT    (10.1567  54.543)";
        olu::lon_lat_t expected = {"10.1567", "54.543"};
        const auto result = olu::osm::OsmObjectHelper::parseLonLatFromWktPoint(wktPoint);

        EXPECT_EQ(expected, result);
    }
    {
    constexpr std::string_view wktPoint = "POINT (10 54)";
        olu::lon_lat_t expected = {"10", "54"};
        const auto result = olu::osm::OsmObjectHelper::parseLonLatFromWktPoint(wktPoint);

        EXPECT_EQ(expected, result);
    }
}

// _________________________________________________________________________________________________
TEST(OsmObjectHelper, parseLonLatFromWktPointInvalid) {
    {
        constexpr std::string_view wktPoint = "";

        EXPECT_THROW(olu::osm::OsmObjectHelper::parseLonLatFromWktPoint(wktPoint),
                     olu::osm::OsmObjectHelperException);
    }
    {
        constexpr std::string_view wktPoint = "POINT ()";

        EXPECT_THROW(olu::osm::OsmObjectHelper::parseLonLatFromWktPoint(wktPoint),
                     olu::osm::OsmObjectHelperException);
    }
    {
        constexpr std::string_view wktPoint = "POINT (8.6296398)";

        EXPECT_THROW(olu::osm::OsmObjectHelper::parseLonLatFromWktPoint(wktPoint),
                     olu::osm::OsmObjectHelperException);
    }
    {
        constexpr std::string_view wktPoint = "POINT 8.6296398 53.1494628)";

        EXPECT_THROW(olu::osm::OsmObjectHelper::parseLonLatFromWktPoint(wktPoint),
                     olu::osm::OsmObjectHelperException);
    }
    {
        constexpr std::string_view wktPoint = "POINT (8.6296398 53.1494628";

        EXPECT_THROW(olu::osm::OsmObjectHelper::parseLonLatFromWktPoint(wktPoint),
                     olu::osm::OsmObjectHelperException);
    }
    {
        constexpr std::string_view wktPoint = "POINT 8.6296398 53.1494628";

        EXPECT_THROW(olu::osm::OsmObjectHelper::parseLonLatFromWktPoint(wktPoint),
                     olu::osm::OsmObjectHelperException);
    }
}

// _________________________________________________________________________________________________
TEST(OsmObjectHelper, parseWayMemberList) {
    {
        constexpr std::string_view uriList = "https://www.openstreetmap.org/node/1;"
                                             "https://www.openstreetmap.org/way/2;"
                                             "https://www.openstreetmap.org/relation/3";
        constexpr std::string_view positionList = "0;1;2";

        const auto memberList = olu::osm::OsmObjectHelper::parseWayMemberList(uriList, positionList);

        ASSERT_EQ(memberList.size(), 3);
        EXPECT_EQ(memberList[0], 1);
        EXPECT_EQ(memberList[1], 2);
        EXPECT_EQ(memberList[2], 3);
    }
    {
        constexpr std::string_view uriList = "https://www.openstreetmap.org/node/4;"
                                             "https://www.openstreetmap.org/way/5";
        constexpr std::string_view positionList = "1;0";

        const auto memberList = olu::osm::OsmObjectHelper::parseWayMemberList(uriList, positionList);

        ASSERT_EQ(memberList.size(), 2);
        EXPECT_EQ(memberList[0], 5);
        EXPECT_EQ(memberList[1], 4);
    }
    {
        constexpr std::string_view uriList = "https://www.openstreetmap.org/node/4";
        constexpr std::string_view positionList = "1";

        const auto memberList = olu::osm::OsmObjectHelper::parseWayMemberList(uriList, positionList);

        ASSERT_EQ(memberList.size(), 1);
        EXPECT_EQ(memberList[0], 4);
    }
}

// _________________________________________________________________________________________________
TEST(OsmObjectHelper, parseWayMemberListInvalid) {
    // Empty URI list
    {
        constexpr std::string_view uriList = "";
        std::string_view positionList = "0;1;2";

        EXPECT_THROW(olu::osm::OsmObjectHelper::parseWayMemberList(uriList, positionList),
                     olu::osm::OsmObjectHelperException);
    }
    // Empty position list
    {
        constexpr std::string_view uriList = "https://www.openstreetmap.org/node/1;"
                                             "https://www.openstreetmap.org/way/2;"
                                             "https://www.openstreetmap.org/relation/3";
        std::string_view positionList = "";

        EXPECT_THROW(olu::osm::OsmObjectHelper::parseWayMemberList(uriList, positionList),
                     olu::osm::OsmObjectHelperException);
    }
    // Missing uri in the uri list
    {
        constexpr std::string_view uriList = "https://www.openstreetmap.org/node/1;"
                                             ";"
                                             "https://www.openstreetmap.org/relation/3";
        constexpr std::string_view positionList = "0;1;2";

        EXPECT_THROW(olu::osm::OsmObjectHelper::parseWayMemberList(uriList, positionList),
                     olu::osm::OsmObjectHelperException);
    }
    // Missing position in the position list
    {
        constexpr std::string_view uriList = "https://www.openstreetmap.org/node/1;"
                                             "https://www.openstreetmap.org/way/2;"
                                             "https://www.openstreetmap.org/relation/3";
        constexpr std::string_view positionList = "0;1;";

        EXPECT_THROW(olu::osm::OsmObjectHelper::parseWayMemberList(uriList, positionList),
                     olu::osm::OsmObjectHelperException);
    }
}

// _________________________________________________________________________________________________
TEST(OsmObjectHelper, parseRelationMemberList) {
    {
        constexpr std::string_view uriList = "https://www.openstreetmap.org/node/1;"
                                             "https://www.openstreetmap.org/way/2;"
                                             "https://www.openstreetmap.org/relation/3";
        constexpr std::string_view rolesList = "role1;role2;role3";
        constexpr std::string_view positionList = "0;1;2";

        const auto memberList = olu::osm::OsmObjectHelper::parseRelationMemberList(uriList, rolesList, positionList);

        ASSERT_EQ(memberList.size(), 3);
        EXPECT_EQ(memberList[0].id, 1);
        EXPECT_EQ(memberList[0].type, olu::osm::OsmObjectType::NODE);
        EXPECT_EQ(memberList[0].role, "role1");

        EXPECT_EQ(memberList[1].id, 2);
        EXPECT_EQ(memberList[1].type, olu::osm::OsmObjectType::WAY);
        EXPECT_EQ(memberList[1].role, "role2");

        EXPECT_EQ(memberList[2].id, 3);
        EXPECT_EQ(memberList[2].type, olu::osm::OsmObjectType::RELATION);
        EXPECT_EQ(memberList[2].role, "role3");
    }
    {
        constexpr std::string_view uriList = "https://www.openstreetmap.org/node/4;"
                                             "https://www.openstreetmap.org/way/5";
        constexpr std::string_view rolesList = "roleA;roleB";
        constexpr std::string_view positionList = "1;0";

        const auto memberList = olu::osm::OsmObjectHelper::parseRelationMemberList(uriList, rolesList, positionList);

        ASSERT_EQ(memberList.size(), 2);
        EXPECT_EQ(memberList[0].id, 5);
        EXPECT_EQ(memberList[0].type, olu::osm::OsmObjectType::WAY);
        EXPECT_EQ(memberList[0].role, "roleB");

        EXPECT_EQ(memberList[1].id, 4);
        EXPECT_EQ(memberList[1].type, olu::osm::OsmObjectType::NODE);
        EXPECT_EQ(memberList[1].role, "roleA");
    }
    {
        constexpr std::string_view uriList = "https://www.openstreetmap.org/node/6";
        constexpr std::string_view rolesList = "roleC";
        constexpr std::string_view positionList = "0";

        const auto memberList = olu::osm::OsmObjectHelper::parseRelationMemberList(uriList, rolesList, positionList);

        ASSERT_EQ(memberList.size(), 1);
        EXPECT_EQ(memberList[0].id, 6);
        EXPECT_EQ(memberList[0].type, olu::osm::OsmObjectType::NODE);
        EXPECT_EQ(memberList[0].role, "roleC");
    }
    // Test with more members that are not ordered
    {
        constexpr std::string_view uriList = "https://www.openstreetmap.org/node/7;"
                                             "https://www.openstreetmap.org/way/8;"
                                             "https://www.openstreetmap.org/relation/9";
        constexpr std::string_view rolesList = "roleX;roleY;roleZ";
        constexpr std::string_view positionList = "2;0;1";

        const auto memberList = olu::osm::OsmObjectHelper::parseRelationMemberList(uriList, rolesList, positionList);

        ASSERT_EQ(memberList.size(), 3);
        EXPECT_EQ(memberList[0].id, 8);
        EXPECT_EQ(memberList[0].type, olu::osm::OsmObjectType::WAY);
        EXPECT_EQ(memberList[0].role, "roleY");

        EXPECT_EQ(memberList[1].id, 9);
        EXPECT_EQ(memberList[1].type, olu::osm::OsmObjectType::RELATION);
        EXPECT_EQ(memberList[1].role, "roleZ");

        EXPECT_EQ(memberList[2].id, 7);
        EXPECT_EQ(memberList[2].type, olu::osm::OsmObjectType::NODE);
        EXPECT_EQ(memberList[2].role, "roleX");
    }
}

// _________________________________________________________________________________________________
TEST(OsmObjectHelper, parseRelationMemberListInvalid) {
    // Empty URI list
    {
        constexpr std::string_view uriList = "";
        constexpr std::string_view rolesList = "role1;role2;role3";
        constexpr std::string_view positionList = "0;1;2";

        EXPECT_THROW(olu::osm::OsmObjectHelper::parseRelationMemberList(uriList, rolesList, positionList),
                     olu::osm::OsmObjectHelperException);
    }
    // Empty roles list
    {
        constexpr std::string_view uriList = "https://www.openstreetmap.org/node/1;"
                                             "https://www.openstreetmap.org/way/2;"
                                             "https://www.openstreetmap.org/relation/3";
        constexpr std::string_view rolesList = "";
        constexpr std::string_view positionList = "0;1;2";

        EXPECT_THROW(olu::osm::OsmObjectHelper::parseRelationMemberList(uriList, rolesList, positionList),
                     olu::osm::OsmObjectHelperException);
    }
    // Empty position list
    {
        constexpr std::string_view uriList = "https://www.openstreetmap.org/node/1;"
                                             "https://www.openstreetmap.org/way/2;"
                                             "https://www.openstreetmap.org/relation/3";
        constexpr std::string_view rolesList = "role1;role2;role3";
        constexpr std::string_view positionList = "";

        EXPECT_THROW(olu::osm::OsmObjectHelper::parseRelationMemberList(uriList, rolesList, positionList),
                     olu::osm::OsmObjectHelperException);
    }
    // Missing uri in the uri list
    {
        constexpr std::string_view uriList = "https://www.openstreetmap.org/node/1;"
                                             ";"
                                             "https://www.openstreetmap.org/relation/3";
        constexpr std::string_view rolesList = "role1;role2;role3";
        constexpr std::string_view positionList = "0;1;2";

        EXPECT_THROW(olu::osm::OsmObjectHelper::parseRelationMemberList(uriList, rolesList, positionList),
                     olu::osm::OsmObjectHelperException);
    }
    // Missing role in the roles list
    {
        constexpr std::string_view uriList = "https://www.openstreetmap.org/node/1;"
                                             "https://www.openstreetmap.org/way/2;"
                                             "https://www.openstreetmap.org/relation/3";
        constexpr std::string_view rolesList = "role1;role2;";
        constexpr std::string_view positionList = "0;1;2";

        EXPECT_THROW(olu::osm::OsmObjectHelper::parseRelationMemberList(uriList, rolesList, positionList),
                     olu::osm::OsmObjectHelperException);
    }
    // Missing position in the position list
    {
        constexpr std::string_view uriList = "https://www.openstreetmap.org/node/1;"
                                             "https://www.openstreetmap.org/way/2;"
                                             "https://www.openstreetmap.org/relation/3";
        constexpr std::string_view rolesList = "role1;role2;role3";
        constexpr std::string_view positionList = "0;1;";

        EXPECT_THROW(olu::osm::OsmObjectHelper::parseRelationMemberList(uriList, rolesList, positionList),
                     olu::osm::OsmObjectHelperException);
    }
}

// _________________________________________________________________________________________________
TEST(OsmObjectHelper, getChangeAction) {
    {
        osmium::memory::Buffer buffer{1024 * 10};

        {
            osmium::builder::NodeBuilder builder{buffer};
            builder.set_id(1)
                .set_visible(true)
                .set_version(1)
                .set_deleted(true);
        }

        const auto& node = buffer.get<osmium::Node>(buffer.commit());
        EXPECT_EQ(olu::osm::OsmObjectHelper::getChangeAction(node), olu::osm::ChangeAction::DELETE);
    }
    {
        osmium::memory::Buffer buffer{1024 * 10};

        {
            osmium::builder::NodeBuilder builder{buffer};
            builder.set_id(1)
                .set_visible(true)
                .set_version(1);
        }

        const auto& node = buffer.get<osmium::Node>(buffer.commit());
        EXPECT_EQ(olu::osm::OsmObjectHelper::getChangeAction(node), olu::osm::ChangeAction::CREATE);
    }
    {
        osmium::memory::Buffer buffer{1024 * 10};

        {
            osmium::builder::NodeBuilder builder{buffer};
            builder.set_id(1)
                .set_visible(true)
                .set_version(2);
        }

        const auto& node = buffer.get<osmium::Node>(buffer.commit());
        EXPECT_EQ(olu::osm::OsmObjectHelper::getChangeAction(node), olu::osm::ChangeAction::MODIFY);
    }
}

// _________________________________________________________________________________________________
TEST(OsmObjectHelper, parseOsm2RdfOption) {
    {
        constexpr std::string_view optionIRI = "<https://osm2rdf.cs.uni-freiburg.de/rdf/meta#add-way-metadata>";
        EXPECT_EQ(olu::osm::OsmObjectHelper::parseOsm2rdfOptionName(optionIRI), "add-way-metadata");
    }
    {
        constexpr std::string_view optionIRI = "https://osm2rdf.cs.uni-freiburg.de/rdf/meta#add-relation-metadata>";
        EXPECT_EQ(olu::osm::OsmObjectHelper::parseOsm2rdfOptionName(optionIRI), "add-relation-metadata");
    }
    {
        constexpr std::string_view optionIRI = "<https://osm2rdf.cs.uni-freiburg.de/rdf/meta#add-node-metadata";
        EXPECT_EQ(olu::osm::OsmObjectHelper::parseOsm2rdfOptionName(optionIRI), "add-node-metadata");
    }
    {
        constexpr std::string_view optionIRI = "https://osm2rdf.cs.uni-freiburg.de/rdf/meta#add-node-metadata";
        EXPECT_EQ(olu::osm::OsmObjectHelper::parseOsm2rdfOptionName(optionIRI), "add-node-metadata");
    }
    {
        constexpr std::string_view optionIRI = "<https://osm2rdf.cs.uni-freiburg.de/rdf/meta#>";
        EXPECT_THROW(olu::osm::OsmObjectHelper::parseOsm2rdfOptionName(optionIRI),
                     olu::osm::OsmObjectHelperException);
    }
    {
        constexpr std::string_view optionIRI = "<https://www.openstreetmap.org/node/1>";
        EXPECT_THROW(olu::osm::OsmObjectHelper::parseOsm2rdfOptionName(optionIRI),
                     olu::osm::OsmObjectHelperException);
    }
    {
        constexpr std::string_view optionIRI = "";
        EXPECT_THROW(olu::osm::OsmObjectHelper::parseOsm2rdfOptionName(optionIRI),
                     olu::osm::OsmObjectHelperException);
    }
}