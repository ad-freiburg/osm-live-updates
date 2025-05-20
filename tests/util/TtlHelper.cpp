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

#include "util/TtlHelper.h"
#include "config/Constants.h"
#include "gtest/gtest.h"

#include <stdexcept>

namespace constants = olu::config::constants;

namespace olu::util {
    const std::string tripleString0 = "osmnode:1 osmmeta:timestamp \"2024-07-07T19:48:37\"^^xsd:dateTime .";
    const std::string tripleString1 = "osmnode:1 osmkey:tower:type \"communication\" .";
    const std::string tripleString2 = "osmnode:1 osmkey:tower:construction \"lattice\" .";
    const std::string tripleString3 = "osmnode:1 osmkey:note \"This is the very first node on OpenStreetMap.\" .";
    const std::string tripleString4 = "osmnode:1 osmkey:name \"Monte Piselli - San Giacomo\" .";
    const std::string tripleString5 = "osmnode:1 osmkey:frequency \"105.5 MHz\" .";
    const std::string tripleString6 = "osmnode:1 osmkey:description \"Radio Subasio\" .";
    const std::string tripleString7 = "osmnode:1 osmkey:communication:radio \"fm\" .";
    const std::string tripleString8 = "osmnode:1 osmkey:man_made \"mast\" .";
    const std::string tripleString9 = "osmnode:1 osmkey:communication:microwave \"yes\" .";
    const std::string tripleString10 = "osmnode:1 osm2rdf:facts \"9\"^^xsd:integer .";
    const std::string tripleString11 = "osmnode:1 geo:hasGeometry osm2rdfgeom:osm_node_1 .";
    const std::string tripleString12 = "osm2rdfgeom:osm_node_1 geo:asWKT \"POINT(13.5690032 42.7957187)\"^^geo:wktLiteral .";
    const std::string tripleString13 = "osmnode:1 osm2rdfgeom:convex_hull \"POLYGON((13.5690032 42.7957187,13.5690032 42.7957187,13.5690032 42.7957187,13.5690032 42.7957187,13.5690032 42.7957187))\"^^geo:wktLiteral .";
    const std::string tripleString14 = "osmnode:1 osm2rdfgeom:envelope \"POLYGON((13.5690032 42.7957187,13.5690032 42.7957187,13.5690032 42.7957187,13.5690032 42.7957187,13.5690032 42.7957187))\"^^geo:wktLiteral .";
    const std::string tripleString15 = "osmnode:1 osm2rdfgeom:obb \"POLYGON((13.5690032 42.7957187,13.5690032 42.7957187,13.5690032 42.7957187,13.5690032 42.7957187,13.5690032 42.7957187))\"^^geo:wktLiteral .";

    // _________________________________________________________________________________________________
    TEST(TtlHelper, parseTriple) {
        {
            auto [subject, predicate, object] = TtlHelper::parseTriple(tripleString0);
            ASSERT_EQ(subject, "osmnode:1");
            ASSERT_EQ(predicate, "osmmeta:timestamp");
            ASSERT_EQ(object, "\"2024-07-07T19:48:37\"^^xsd:dateTime");
        }
        {
            auto [subject, predicate, object] = TtlHelper::parseTriple(tripleString1);
            ASSERT_EQ(subject, "osmnode:1");
            ASSERT_EQ(predicate, "osmkey:tower:type");
            ASSERT_EQ(object, "\"communication\"");
        }
        {
            auto [subject, predicate, object] = TtlHelper::parseTriple(tripleString2);
            ASSERT_EQ(subject, "osmnode:1");
            ASSERT_EQ(predicate, "osmkey:tower:construction");
            ASSERT_EQ(object, "\"lattice\"");
        }
        {
            auto [subject, predicate, object] = TtlHelper::parseTriple(tripleString3);
            ASSERT_EQ(subject, "osmnode:1");
            ASSERT_EQ(predicate, "osmkey:note");
            ASSERT_EQ(object, "\"This is the very first node on OpenStreetMap.\"");
        }
        {
            auto [subject, predicate, object] = TtlHelper::parseTriple(tripleString4);
            ASSERT_EQ(subject, "osmnode:1");
            ASSERT_EQ(predicate, "osmkey:name");
            ASSERT_EQ(object, "\"Monte Piselli - San Giacomo\"");
        }
        {
            auto [subject, predicate, object] = TtlHelper::parseTriple(tripleString5);
            ASSERT_EQ(subject, "osmnode:1");
            ASSERT_EQ(predicate, "osmkey:frequency");
            ASSERT_EQ(object, "\"105.5 MHz\"");
        }
        {
            auto [subject, predicate, object] = TtlHelper::parseTriple(tripleString6);
            ASSERT_EQ(subject, "osmnode:1");
            ASSERT_EQ(predicate, "osmkey:description");
            ASSERT_EQ(object, "\"Radio Subasio\"");
        }
        {
            auto [subject, predicate, object] = TtlHelper::parseTriple(tripleString7);
            ASSERT_EQ(subject, "osmnode:1");
            ASSERT_EQ(predicate, "osmkey:communication:radio");
            ASSERT_EQ(object, "\"fm\"");
        }
        {
            auto [subject, predicate, object] = TtlHelper::parseTriple(tripleString8);
            ASSERT_EQ(subject, "osmnode:1");
            ASSERT_EQ(predicate, "osmkey:man_made");
            ASSERT_EQ(object, "\"mast\"");
        }
        {
            auto [subject, predicate, object] = TtlHelper::parseTriple(tripleString9);
            ASSERT_EQ(subject, "osmnode:1");
            ASSERT_EQ(predicate, "osmkey:communication:microwave");
            ASSERT_EQ(object, "\"yes\"");
        }
        {
            auto [subject, predicate, object] = TtlHelper::parseTriple(tripleString10);
            ASSERT_EQ(subject, "osmnode:1");
            ASSERT_EQ(predicate, "osm2rdf:facts");
            ASSERT_EQ(object, "\"9\"^^xsd:integer");
        }
        {
            auto [subject, predicate, object] = TtlHelper::parseTriple(tripleString11);
            ASSERT_EQ(subject, "osmnode:1");
            ASSERT_EQ(predicate, "geo:hasGeometry");
            ASSERT_EQ(object, "osm2rdfgeom:osm_node_1");
        }
        {
            auto [subject, predicate, object] = TtlHelper::parseTriple(tripleString12);
            ASSERT_EQ(subject, "osm2rdfgeom:osm_node_1");
            ASSERT_EQ(predicate, "geo:asWKT");
            ASSERT_EQ(object, "\"POINT(13.5690032 42.7957187)\"^^geo:wktLiteral");
        }
        {
            auto [subject, predicate, object] = TtlHelper::parseTriple(tripleString13);
            ASSERT_EQ(subject, "osmnode:1");
            ASSERT_EQ(predicate, "osm2rdfgeom:convex_hull");
            ASSERT_EQ(object, "\"POLYGON((13.5690032 42.7957187,13.5690032 42.7957187,13.5690032 42.7957187,13.5690032 42.7957187,13.5690032 42.7957187))\"^^geo:wktLiteral");
        }
        {
            auto [subject, predicate, object] = TtlHelper::parseTriple(tripleString14);
            ASSERT_EQ(subject, "osmnode:1");
            ASSERT_EQ(predicate, "osm2rdfgeom:envelope");
            ASSERT_EQ(object, "\"POLYGON((13.5690032 42.7957187,13.5690032 42.7957187,13.5690032 42.7957187,13.5690032 42.7957187,13.5690032 42.7957187))\"^^geo:wktLiteral");
        }
        {
            auto [subject, predicate, object] = TtlHelper::parseTriple(tripleString15);
            ASSERT_EQ(subject, "osmnode:1");
            ASSERT_EQ(predicate, "osm2rdfgeom:obb");
            ASSERT_EQ(object, "\"POLYGON((13.5690032 42.7957187,13.5690032 42.7957187,13.5690032 42.7957187,13.5690032 42.7957187,13.5690032 42.7957187))\"^^geo:wktLiteral");
        }
    }

    // _________________________________________________________________________________________________
    TEST(TtlHelper, getIdFromSubject) {
        {
            const std::string subject = "osmnode:1";
            ASSERT_EQ(TtlHelper::parseId(subject), 1);
        }
        {
            const std::string subject = "osmnode:123";
            ASSERT_EQ(TtlHelper::parseId(subject), 123);
        }
        {
            const std::string subject = "osm2rdfgeom:osm_node_1";
            ASSERT_EQ(TtlHelper::parseId(subject), 1);
        }
        {
            const std::string subject = "osmway:1";
            ASSERT_EQ(TtlHelper::parseId(subject), 1);
        }
        {
            const std::string subject = "osmrel:1";
            ASSERT_EQ(TtlHelper::parseId(subject), 1);
        }
    }
}