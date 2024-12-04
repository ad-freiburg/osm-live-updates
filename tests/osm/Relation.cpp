//
// Created by Nicolas von Trott on 24.11.24.
//

#include "gtest/gtest.h"
#include "osm/Relation.h"

namespace olu::osm {
    TEST(Relation, createRelationFromReferences) {
        Relation relation(1);
        relation.setType("multipolygon");
        relation.addMember({1, "node", "member"});
        relation.addMember({2, "way", "outer"});
        relation.addMember({3, "relation", "inner"});

        ASSERT_EQ(relation.getXml(),
                  "<relation id=\"1\">"
                  "<member type=\"node\" ref=\"1\" role=\"member\"/>"
                  "<member type=\"way\" ref=\"2\" role=\"outer\"/>"
                  "<member type=\"relation\" ref=\"3\" role=\"inner\"/>"
                  "<tag k=\"type\" v=\"multipolygon\"/>"
                  "</relation>"
        );
    }
}