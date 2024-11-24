//
// Created by Nicolas von Trott on 22.11.24.
//

#include "gtest/gtest.h"
#include "osm/Node.h"

namespace olu::osm {

    TEST(Node, initNodeFromPoint) {
    {
        Node node(1, "POINT(13.5690032 42.7957187)");

        ASSERT_EQ(node.get_xml(),
        "<node id=\"1\" lat=\"42.7957187\" lon=\"13.5690032\"/>");
    }
    {
    ASSERT_THROW(
            Node node(1, "POINT(135690032 42.7957187)"),
            NodeException);
    }
    {

    ASSERT_THROW(
            Node node(1, "POINT(13.5690032)"),
            NodeException);
    }
    {
    ASSERT_THROW(
            Node node(1, "POINT(13.5690032, 42.7957187)"),
            NodeException);
    }
    {
    ASSERT_THROW(
            Node node(1, "POINT(13.5690032 42)"),
            NodeException);
    }
    }

}
