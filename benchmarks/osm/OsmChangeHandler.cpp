//
// Created by Nicolas von Trott on 25.09.24.
//

#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include "benchmark/benchmark.h"
#include "util/XmlReader.h"
#include "config/Constants.h"
#include "osm/OsmChangeHandler.h"


// ---------------------------------------------------------------------------
static void countElements(benchmark::State& state) {
    std::string path = "/src/tests/data/";
    std::ifstream xmlFile (path + "427.osc");
    std::string content( (std::istreambuf_iterator<char>(xmlFile) ),
                         (std::istreambuf_iterator<char>()) );

    pt::ptree osmChangeElement;
    olu::util::XmlReader::populatePTreeFromString(content, osmChangeElement);

    for (auto _ : state) {
        olu::osm::OsmChangeHandler::countElements(osmChangeElement);
    }
}
BENCHMARK(countElements);