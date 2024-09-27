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
#include "util/XmlReader.h"
#include "config/Constants.h"
#include "osm/OsmChangeHandler.h"

// ---------------------------------------------------------------------------
static void Count_Elements(benchmark::State& state) {
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
BENCHMARK(Count_Elements);