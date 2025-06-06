// Copyright 2025, University of Freiburg
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

#include "benchmark/benchmark.h"

#include "osm/OsmObjectHelper.h"

// ---------------------------------------------------------------------------
static void parseIdFromUri(benchmark::State& state) {
    constexpr std::string_view uri = "https://www.openstreetmap.org/node/123456789";

     for (auto _ : state) {
        auto parsedId = olu::osm::OsmObjectHelper::parseIdFromUri(uri);
     }
}
BENCHMARK(parseIdFromUri);

// ---------------------------------------------------------------------------
static void parseOsmTypeFromUriNode(benchmark::State& state) {
    constexpr std::string_view uri = "https://www.openstreetmap.org/node/123456789";

    for (auto _ : state) {
        auto parsedUri = olu::osm::OsmObjectHelper::parseOsmTypeFromUri(uri);
    }
}
BENCHMARK(parseOsmTypeFromUriNode);

// ---------------------------------------------------------------------------
static void parseOsmTypeFromUriWay(benchmark::State& state) {
    constexpr std::string_view uri = "https://www.openstreetmap.org/way/123456789";

    for (auto _ : state) {
        auto parsedUri = olu::osm::OsmObjectHelper::parseOsmTypeFromUri(uri);
    }
}
BENCHMARK(parseOsmTypeFromUriWay);

// ---------------------------------------------------------------------------
static void parseOsmTypeFromUriRelation(benchmark::State& state) {
    constexpr std::string_view uri = "https://www.openstreetmap.org/relation/123456789";

    for (auto _ : state) {
        auto parsedUri = olu::osm::OsmObjectHelper::parseOsmTypeFromUri(uri);
    }
}
BENCHMARK(parseOsmTypeFromUriRelation);

// ---------------------------------------------------------------------------
static void parseLonLatFromWktPoint(benchmark::State& state) {
    constexpr std::string_view wktPoint = "POINT (8.6296398 53.1494628)";

    for (auto _ : state) {
        auto [lon, lat] = olu::osm::OsmObjectHelper::parseLonLatFromWktPoint(wktPoint);
    }
}
BENCHMARK(parseLonLatFromWktPoint);