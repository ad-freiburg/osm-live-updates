//
// Created by Nicolas von Trott on 22.11.24.
//

#include "gtest/gtest.h"
#include "osm/Node.h"

namespace olu::osm {
    TEST(Node, initNodeFromPoint) {
        {
            ASSERT_NO_THROW(Node (1, "POINT(13.5690032 42.7957187)"));
        }
        {
            ASSERT_THROW(Node(1, "POINT(13.5690032)"), NodeException);
        }
        {
            const osmium::Location location(42.7957187, 13.5690032);
            ASSERT_NO_THROW(Node(1, location));
        }
    }

    TEST(Node, getNodeXml) {
        {
            const Node node(1, "POINT(13.5690032 42.7957187)");
            ASSERT_EQ(node.getXml(),
                      "<node id=\"1\" lat=\"42.7957187\" lon=\"13.5690032\"/>");
        }
        {
            const osmium::Location location(42.7957187, 13.5690032);
            const Node node(1, location);
            ASSERT_EQ(node.getXml(),
                      "<node id=\"1\" lat=\"13.5690032\" lon=\"42.7957187\"/>");
        }
    }
}
