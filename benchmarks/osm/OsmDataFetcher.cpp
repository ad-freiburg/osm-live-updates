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

// ---------------------------------------------------------------------------
static void Fetch_Latest_Timestamp_Of_Any_Node(benchmark::State& state) {
    auto config((olu::config::Config()));
    config.sparqlEndpointUri = "http://host.docker.internal:7007/osm-planet/";

    auto odf = olu::osm::OsmDataFetcher(config);

    for (auto _ : state) {
        odf.fetchLatestTimestampOfAnyNode();
    }
}
BENCHMARK(Fetch_Latest_Timestamp_Of_Any_Node);

// ---------------------------------------------------------------------------
static void Fetch_Node_Location_As_Wkt(benchmark::State& state) {
    auto config((olu::config::Config()));
    config.sparqlEndpointUri = "http://host.docker.internal:7007/osm-planet/";

    auto odf = olu::osm::OsmDataFetcher(config);

    for (auto _ : state) {
        odf.fetchNodeLocationAsWkt(2186958084);
    }
}
BENCHMARK(Fetch_Node_Location_As_Wkt);
