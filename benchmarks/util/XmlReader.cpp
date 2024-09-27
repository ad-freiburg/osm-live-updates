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

#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include "benchmark/benchmark.h"
#include "config/Config.h"
#include "util/XmlReader.h"
#include "config/Constants.h"

// ---------------------------------------------------------------------------
static void Read_Attribute(benchmark::State& state) {
    std::string path = "/src/tests/data/";
    std::ifstream xmlFile (path + "node.osm");
    std::string content( (std::istreambuf_iterator<char>(xmlFile) ),
                         (std::istreambuf_iterator<char>()) );

    pt::ptree tree;
    olu::util::XmlReader::populatePTreeFromString(content, tree);

    for (auto _ : state) {
        std::string attribute = olu::util::XmlReader::readAttribute(
                olu::config::constants::ATTRIBUTE_PATH_FOR_NODE_ID,
                tree);
    }
}
BENCHMARK(Read_Attribute);

// ---------------------------------------------------------------------------
static void Populate_PTree_From_String_Node(benchmark::State& state) {
    std::string path = "/src/tests/data/";
    std::ifstream xmlFile (path + "node.osm");
    std::string content( (std::istreambuf_iterator<char>(xmlFile) ),
                         (std::istreambuf_iterator<char>()) );

    for (auto _ : state) {
        pt::ptree tree;
        olu::util::XmlReader::populatePTreeFromString(content, tree);
    }
}
BENCHMARK(Populate_PTree_From_String_Node);

// ---------------------------------------------------------------------------
static void Populate_PTree_From_String_Change_File(benchmark::State& state) {
    std::string path = "/src/tests/data/";
    std::ifstream xmlFile (path + "427.osc");
    std::string content( (std::istreambuf_iterator<char>(xmlFile) ),
                         (std::istreambuf_iterator<char>()) );

    for (auto _ : state) {
        pt::ptree tree;
        olu::util::XmlReader::populatePTreeFromString(content, tree);
    }
}
BENCHMARK(Populate_PTree_From_String_Change_File);

// ---------------------------------------------------------------------------
static void Read_Tag_Of_Children(benchmark::State& state) {
    std::string path = "/src/tests/data/";
    std::ifstream xmlFile (path + "node.osm");
    std::string content( (std::istreambuf_iterator<char>(xmlFile) ),
                         (std::istreambuf_iterator<char>()) );

    pt::ptree tree;
    olu::util::XmlReader::populatePTreeFromString(content, tree);

    for (auto _ : state) {
        auto childrenTags = olu::util::XmlReader::readTagOfChildren(
                olu::config::constants::OSM_TAG,
                tree,
                false);
    }
}
BENCHMARK(Read_Tag_Of_Children);



