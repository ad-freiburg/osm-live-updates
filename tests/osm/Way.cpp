//
// Created by Nicolas von Trott on 24.11.24.
//

#include "gtest/gtest.h"
#include "osm/Way.h"

namespace olu::osm {
    TEST(Way, initWayFromPoint) {
        {
            ASSERT_NO_THROW(Way(1));
        }
    }

    TEST(Way, getWayXml) { {
            Way way(1);
            way.addMember(1);
            way.addMember(2);
            way.addMember(3);
            ASSERT_EQ(way.getXml(),
                      "<way id=\"1\">"
                      "<nd ref=\"1\"/>"
                      "<nd ref=\"2\"/>"
                      "<nd ref=\"3\"/>"
                      "</way>"
            );
        } {
            Way way(1);
            way.addTag("key", "value");
            ASSERT_EQ(way.getXml(),
                      "<way id=\"1\">"
                      "<tag k=\"key\" v=\"value\"/>"
                      "</way>"
            );
        } {
            Way way(1);
            way.addMember(1);
            way.addMember(2);
            way.addMember(3);
            way.addTag("key", "value");
            ASSERT_EQ(way.getXml(),
                      "<way id=\"1\">"
                      "<nd ref=\"1\"/>"
                      "<nd ref=\"2\"/>"
                      "<nd ref=\"3\"/>"
                      "<tag k=\"key\" v=\"value\"/>"
                      "</way>"
            );
        } {
            Way way(1);
            way.setTimestamp("2024-09-19T09:02:41");
            way.addMember(1);
            way.addMember(2);
            way.addMember(3);
            way.addTag("key", "value");
            ASSERT_EQ(way.getXml(),
                      "<way id=\"1\" timestamp=\"2024-09-19T09:02:41Z\">"
                      "<nd ref=\"1\"/>"
                      "<nd ref=\"2\"/>"
                      "<nd ref=\"3\"/>"
                      "<tag k=\"key\" v=\"value\"/>"
                      "</way>"
            );
        }
    }
}
