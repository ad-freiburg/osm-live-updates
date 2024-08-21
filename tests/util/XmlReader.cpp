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

#include "util/XmlReader.h"
#include "config/Constants.h"
#include "gtest/gtest.h"

#include <fstream>

// _________________________________________________________________________________________________
TEST(XmlReader, readNodeElement) {
    {
        // Todo: Read path from environment
        std::string path = "/src/tests/data/";
        std::ifstream xmlFile (path + "node.osm");
        std::string content( (std::istreambuf_iterator<char>(xmlFile) ),
                             (std::istreambuf_iterator<char>()) );

        std::string nodeElement = olu::util::XmlReader::readNodeElement(content);
        std::string correctNodeElement = R"(<node id="1" visible="true" version="37" changeset="153676518" timestamp="2024-07-07T19:48:37Z" user="tyr_asd" uid="115612" lat="42.7957187" lon="13.5690032"><tag k="communication:microwave" v="yes"/><tag k="communication:radio" v="fm"/><tag k="description" v="Radio Subasio"/><tag k="frequency" v="105.5 MHz"/><tag k="man_made" v="mast"/><tag k="name" v="Monte Piselli - San Giacomo"/><tag k="note" v="This is the very first node on OpenStreetMap."/><tag k="tower:construction" v="lattice"/><tag k="tower:type" v="communication"/></node>)";
        ASSERT_EQ(nodeElement, correctNodeElement);
    }
}

TEST(XmlReader, readAttribute) {
    {
        // Todo: Read path from environment
        std::string path = "/src/tests/data/";
        std::ifstream xmlFile (path + "node.osm");
        std::string content( (std::istreambuf_iterator<char>(xmlFile) ),
                             (std::istreambuf_iterator<char>()) );

        pt::ptree tree;
        olu::util::XmlReader::populatePTreeFromString(content, tree);

        std::string attribute = olu::util::XmlReader::readAttribute(
                olu::config::constants::ATTRIBUTE_PATH_FOR_NODE_ID,
                tree);
        ASSERT_EQ(attribute, "1");

        tree.clear();
    }

    {
        // Todo: Read path from environment
        std::string path = "/src/tests/data/";
        std::ifstream xmlFile (path + "node.osm");
        std::string content( (std::istreambuf_iterator<char>(xmlFile) ),
                             (std::istreambuf_iterator<char>()) );

        pt::ptree tree;
        olu::util::XmlReader::populatePTreeFromString(content, tree);

        std::string attribute = olu::util::XmlReader::readAttribute(
                "osm.node.<xmlattr>.notExisting",
                tree);

        ASSERT_EQ(attribute, "");

        tree.clear();
    }

    {
        // Todo: Read path from environment
        std::string path = "/src/tests/data/";
        std::ifstream xmlFile (path + "node.osm");
        std::string content( (std::istreambuf_iterator<char>(xmlFile) ),
                             (std::istreambuf_iterator<char>()) );

        pt::ptree tree;
        olu::util::XmlReader::populatePTreeFromString(content, tree);

        std::string attribute = olu::util::XmlReader::readAttribute(
                olu::config::constants::OSM_TAG + "." +
                olu::config::constants::NODE_TAG + "." +
                olu::config::constants::XML_ATTRIBUTE_TAG + ".notExisting",
                tree);

        ASSERT_EQ(attribute, "");

        tree.clear();
    }
}


TEST(XmlReader, readTagOfChildren) {
    {
        // Todo: Read path from environment
        std::string path = "/src/tests/data/";
        std::ifstream xmlFile (path + "node.osm");
        std::string content( (std::istreambuf_iterator<char>(xmlFile) ),
                             (std::istreambuf_iterator<char>()) );

        pt::ptree tree;
        olu::util::XmlReader::populatePTreeFromString(content, tree);

        auto childrenTags = olu::util::XmlReader::readTagOfChildren(
                olu::config::constants::OSM_TAG,
                tree,
                false);

        ASSERT_EQ(childrenTags.size(), 2);
        ASSERT_EQ(childrenTags.at(0), olu::config::constants::XML_ATTRIBUTE_TAG);
        ASSERT_EQ(childrenTags.at(1), olu::config::constants::NODE_TAG);

        auto childrenTags2 = olu::util::XmlReader::readTagOfChildren(
                olu::config::constants::OSM_TAG,
                tree,
                true);

        ASSERT_EQ(childrenTags2.size(), 1);
        ASSERT_EQ(childrenTags2.at(0), olu::config::constants::NODE_TAG);

        tree.clear();
    }
}