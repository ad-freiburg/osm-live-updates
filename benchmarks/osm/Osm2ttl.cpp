//
// Created by Nicolas von Trott on 25.09.24.
//

#include "osm/Osm2ttl.h"

#include "benchmark/benchmark.h"

// ---------------------------------------------------------------------------
static void Convert_Node(benchmark::State& state) {
    std::string path = "/src/tests/data/";

    std::ifstream ifs(path + "node.osm");
    std::string nodeElement((std::istreambuf_iterator<char>(ifs)),
                            (std::istreambuf_iterator<char>()));
    std::vector<std::string> elements;
    elements.push_back(nodeElement);

    for (auto _ : state) {
        auto osm2rdf = olu::osm::Osm2ttl();
        auto result = osm2rdf.convert(elements);
    }
}
BENCHMARK(Convert_Node);

// ---------------------------------------------------------------------------
static void Convert_Way(benchmark::State& state) {
    std::string path = "/src/tests/data/";

    std::ifstream ifs(path + "wayWithReferences.osm");
    std::string nodeElement((std::istreambuf_iterator<char>(ifs)),
                            (std::istreambuf_iterator<char>()));
    std::vector<std::string> elements;
    elements.push_back(nodeElement);

    for (auto _ : state) {
        auto osm2rdf = olu::osm::Osm2ttl();
        auto result = osm2rdf.convert(elements);
    }
}
BENCHMARK(Convert_Way);
