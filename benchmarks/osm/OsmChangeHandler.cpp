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
#include "config/Constants.h"
#include "osm/OsmChangeHandler.h"

// ---------------------------------------------------------------------------
static void Handle_Change_Delete_Node(benchmark::State& state) {
    std::string path = "/src/tests/data/delete_node.osc";

    auto config((olu::config::Config()));
    config.sparqlEndpointUri = "http://host.docker.internal:7007/osm-planet/";

    // auto och = olu::osm::OsmChangeHandler(config);
    //
    // for (auto _ : state) {
    //     och.run();
    // }
}
BENCHMARK(Handle_Change_Delete_Node);

// ---------------------------------------------------------------------------
static void Handle_Change_Insert_Node(benchmark::State& state) {
    std::string path = "/src/tests/data/insert_node.osc";

    auto config((olu::config::Config()));
    config.sparqlEndpointUri = "http://host.docker.internal:7007/osm-planet/";

    // auto och = olu::osm::OsmChangeHandler(config);
    //
    // for (auto _ : state) {
    //     och.run();
    // }
}
BENCHMARK(Handle_Change_Insert_Node);

// ---------------------------------------------------------------------------
static void Handle_Change_Modify_Node(benchmark::State& state) {
    std::string path = "/src/tests/data/modify_node.osc";

    auto config((olu::config::Config()));
    config.sparqlEndpointUri = "http://host.docker.internal:7007/osm-planet/";

    // auto och = olu::osm::OsmChangeHandler(config);
    //
    // for (auto _ : state) {
    //     och.run();
    // }
}
BENCHMARK(Handle_Change_Modify_Node);