//
// Created by Nicolas von Trott on 25.09.24.
//

#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include "benchmark/benchmark.h"
#include "config/Config.h"
#include "util/XmlReader.h"
#include "config/Constants.h"

// ---------------------------------------------------------------------------
static void readAttribute(benchmark::State& state) {
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
BENCHMARK(readAttribute);

// ---------------------------------------------------------------------------
static void populatePTreeFromString(benchmark::State& state) {
    std::string path = "/src/tests/data/";
    std::ifstream xmlFile (path + "node.osm");
    std::string content( (std::istreambuf_iterator<char>(xmlFile) ),
                         (std::istreambuf_iterator<char>()) );

    for (auto _ : state) {
        pt::ptree tree;
        olu::util::XmlReader::populatePTreeFromString(content, tree);
    }
}
BENCHMARK(populatePTreeFromString);

// ---------------------------------------------------------------------------
static void populatePTreeFromStringLarge(benchmark::State& state) {
    std::string path = "/src/tests/data/";
    std::ifstream xmlFile (path + "427.osc");
    std::string content( (std::istreambuf_iterator<char>(xmlFile) ),
                         (std::istreambuf_iterator<char>()) );

    for (auto _ : state) {
        pt::ptree tree;
        olu::util::XmlReader::populatePTreeFromString(content, tree);
    }
}
BENCHMARK(populatePTreeFromStringLarge);

// ---------------------------------------------------------------------------
static void readTagOfChildren(benchmark::State& state) {
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
BENCHMARK(readTagOfChildren);



