//
// Created by Nicolas von Trott on 22.11.24.
//

#include "gtest/gtest.h"
#include "osm/Node.h"

namespace olu::osm {
    TEST(Node, initNodeFromPoint) {
        {
            ASSERT_NO_THROW(Node (1, "POINT(13.5690032 42.7957187)"));
        } {
            ASSERT_THROW(Node(1, "POINT(13.5690032)"), NodeException);
        }
    }

    TEST(Node, getNodeXml) {
        {
            const Node node(1, "POINT(13.5690032 42.7957187)");
            ASSERT_EQ(node.getXml(),
                      "<node id=\"1\" lat=\"42.7957187\" lon=\"13.5690032\"/>");
        }
    }
}
