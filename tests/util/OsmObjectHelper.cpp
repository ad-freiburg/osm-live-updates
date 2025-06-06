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
