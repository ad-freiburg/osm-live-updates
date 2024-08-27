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

#include "util/XmlWriter.h"
#include "util/XmlReader.h"
#include "config/Constants.h"
#include "gtest/gtest.h"

#include <fstream>

// _________________________________________________________________________________________________
TEST(XmlWriter, addAttributeToElement) {
    {
    // Todo: Read path from environment
    std::string path = "/src/tests/data/";
    std::ifstream file1 (path + "empty_node.osm");
    std::string content( (std::istreambuf_iterator<char>(file1) ),
                         (std::istreambuf_iterator<char>()) );

    std::ifstream file2 (path + "empty_node_with_added_tag.osm");
    std::string correctNodeElement( (std::istreambuf_iterator<char>(file2) ),
                                    (std::istreambuf_iterator<char>()) );

    pt::ptree tree;
    olu::util::XmlReader::populatePTreeFromString(content, tree);
    std::string elementPath = olu::config::constants::NODE_TAG + "." + "tag";
    olu::util::XmlWriter::addAttributeToElement(tree, elementPath, "value", "key");
    std::string nodeElement = olu::util::XmlReader::readTree(tree);
    ASSERT_EQ(nodeElement, correctNodeElement);

    tree.clear();
    }
}



//
