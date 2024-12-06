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

#include "osm/OsmDataFetcher.h"

#include "benchmark/benchmark.h"
#include "config/Constants.h"
#include "sparql/QueryWriter.h"

namespace cnst = olu::config::constants;

// ---------------------------------------------------------------------------
static void Set_Prefixes(benchmark::State& state) {
    auto config((olu::config::Config()));
    auto sparqlWrapper = olu::sparql::SparqlWrapper(config);

    for (auto _ : state) {
        sparqlWrapper.setPrefixes(olu::config::constants::PREFIXES_FOR_WAY_DELETE_QUERY);
    }
}
BENCHMARK(Set_Prefixes);

// ---------------------------------------------------------------------------
static void Run_Query_For_Node_Location(benchmark::State& state) {
    auto config((olu::config::Config()));
    config.sparqlEndpointUri = cnst::QLEVER_LOCAL_HOST_URI;

    auto sparqlWrapper = olu::sparql::SparqlWrapper(config);
    olu::sparql::QueryWriter qw{config};
    std::string query = qw.writeQueryForNodeLocations({1});

    for (auto _ : state) {
        sparqlWrapper.setQuery(query);
        sparqlWrapper.setPrefixes(olu::config::constants::PREFIXES_FOR_NODE_LOCATION);

        auto response = sparqlWrapper.runQuery();
    }
}
BENCHMARK(Run_Query_For_Node_Location);

// // ---------------------------------------------------------------------------
// static void Run_Query_For_Node_Deletion(benchmark::State& state) {
//     auto config((olu::config::Config()));
//     config.sparqlEndpointUri = cnst::QLEVER_LOCAL_HOST_URI;
//
//     auto sparqlWrapper = olu::sparql::SparqlWrapper(config);
//     // auto query = olu::sparql::QueryWriter::writeDeleteQuery({"osmnode:1"}, TODO);
//
//     for (auto _ : state) {
//         sparqlWrapper.setPrefixes(cnst::PREFIXES_FOR_NODE_DELETE_QUERY);
//         sparqlWrapper.setQuery(query);
//
//         auto response = sparqlWrapper.runQuery();
//     }
// }
// BENCHMARK(Run_Query_For_Node_Deletion);

// ---------------------------------------------------------------------------
static void Clear_Cache(benchmark::State& state) {
    auto config((olu::config::Config()));
    config.sparqlEndpointUri = cnst::QLEVER_LOCAL_HOST_URI;

    for (auto _ : state) {
        auto sparqlWrapper = olu::sparql::SparqlWrapper(config);
        sparqlWrapper.clearCache();
    }
}
BENCHMARK(Clear_Cache);
