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

TEST(XmlReader, readAttribute) {
    {
        // Todo: Read path from environment
        std::string path = "/app/tests/data/";
        std::ifstream xmlFile (path + "node.osm");
        std::string content( (std::istreambuf_iterator<char>(xmlFile) ),
                             (std::istreambuf_iterator<char>()) );

        pt::ptree tree;
        olu::util::XmlReader::populatePTreeFromString(content, tree);

        auto attribute = olu::util::XmlReader::readAttribute<std::string>(
                "osm.node.<xmlattr>.id",
                tree);
        ASSERT_EQ(attribute, "1");

        tree.clear();
    }

    {
        // Todo: Read path from environment
        std::string path = "/app/tests/data/";
        std::ifstream xmlFile (path + "node.osm");
        std::string content( (std::istreambuf_iterator<char>(xmlFile) ),
                             (std::istreambuf_iterator<char>()) );

        pt::ptree tree;
        olu::util::XmlReader::populatePTreeFromString(content, tree);

        EXPECT_ANY_THROW(olu::util::XmlReader::readAttribute<std::string>("osm.node.<xmlattr>.notExisting", tree));

        tree.clear();
    }
}


TEST(XmlReader, readTagOfChildren) {
    {
        // Todo: Read path from environment
        std::string path = "/app/tests/data/";
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