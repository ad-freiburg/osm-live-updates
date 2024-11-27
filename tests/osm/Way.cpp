//
// Created by Nicolas von Trott on 24.11.24.
//

#include "gtest/gtest.h"
#include "osm/Way.h"

namespace olu::osm {
    TEST(Way, createWayFromReferences) {
        Way way(1);
        way.addMember(1);
        way.addMember(2);
        way.addMember(3);
        ASSERT_EQ(way.getXml(),
                  "<way id=\"1\">"
                  "<nd ref=\"1\"/>"
                  "<nd ref=\"2\"/>"
                  "<nd ref=\"3\"/>"
                  "<tag k=\"type\" v=\"tmp\"/>"
                  "</way>"
        );
    }
}