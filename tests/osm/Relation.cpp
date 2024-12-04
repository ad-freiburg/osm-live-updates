//
// Created by Nicolas von Trott on 24.11.24.
//

#include "gtest/gtest.h"
#include "osm/Relation.h"

namespace olu::osm {
    TEST(Relation, initRelationFromPoint) {
        {
            ASSERT_NO_THROW(Relation(1));
        }
    }

    TEST(Relation, getRelationXml) { {
            Relation relation(1);
            relation.addMember({1, "node", "member"});
            relation.addMember({1, "way", "member"});
            relation.addMember({1, "relation", "member"});
            ASSERT_EQ(relation.getXml(),
                      "<relation id=\"1\">"
                      "<member type=\"node\" ref=\"1\" role=\"member\"/>"
                      "<member type=\"way\" ref=\"1\" role=\"member\"/>"
                      "<member type=\"relation\" ref=\"1\" role=\"member\"/>"
                      "<tag k=\"type\" v=\"\"/>"
                      "</relation>"
            );
        } {
            Relation relation(1);
            relation.addTag("key", "value");
            ASSERT_EQ(relation.getXml(),
                      "<relation id=\"1\">"
                      "<tag k=\"type\" v=\"\"/>"
                      "<tag k=\"key\" v=\"value\"/>"
                      "</relation>"
            );
        } {
            Relation relation(1);
            relation.addMember({1, "node", "member"});
            relation.addMember({1, "way", "member"});
            relation.addMember({1, "relation", "member"});
            relation.addTag("key", "value");
            ASSERT_EQ(relation.getXml(),
                      "<relation id=\"1\">"
                      "<member type=\"node\" ref=\"1\" role=\"member\"/>"
                      "<member type=\"way\" ref=\"1\" role=\"member\"/>"
                      "<member type=\"relation\" ref=\"1\" role=\"member\"/>"
                      "<tag k=\"type\" v=\"\"/>"
                      "<tag k=\"key\" v=\"value\"/>"
                      "</relation>"
            );
        } {
            Relation relation(1);
            relation.setTimestamp("2024-09-19T09:02:41");
            relation.addMember({1, "node", "member"});
            relation.addMember({1, "way", "member"});
            relation.addMember({1, "relation", "member"});
            relation.addTag("key", "value");
            ASSERT_EQ(relation.getXml(),
                      "<relation id=\"1\" timestamp=\"2024-09-19T09:02:41Z\">"
                      "<member type=\"node\" ref=\"1\" role=\"member\"/>"
                      "<member type=\"way\" ref=\"1\" role=\"member\"/>"
                      "<member type=\"relation\" ref=\"1\" role=\"member\"/>"
                      "<tag k=\"type\" v=\"\"/>"
                      "<tag k=\"key\" v=\"value\"/>"
                      "</relation>"
            );
        }
    }
}
