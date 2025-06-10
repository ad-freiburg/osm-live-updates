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
#include "util/TtlHelper.h"

#include <config/Constants.h>

// ---------------------------------------------------------------------------
static void Parse_Triple(benchmark::State& state) {
    const std::string triple = "osmnode:1 osmkey:tower:construction \"lattice\" .";

    for (auto _ : state) {
        auto components = olu::util::TtlHelper::parseTriple(triple);
    }
}
BENCHMARK(Parse_Triple);

// ---------------------------------------------------------------------------
static void Get_Id_From_Node_Subject(benchmark::State& state) {
    const std::string subject = "osmnode:1";

    for (auto _ : state) {
        auto components = olu::util::TtlHelper::parseId(subject);
    }
}
BENCHMARK(Get_Id_From_Node_Subject);
// ---------------------------------------------------------------------------
static void Get_Id_From_Way_Subject(benchmark::State& state) {
    const std::string subject = "osmway:1";

    for (auto _ : state) {
        auto components = olu::util::TtlHelper::parseId(subject);
    }
}
BENCHMARK(Get_Id_From_Way_Subject);
// ---------------------------------------------------------------------------
static void Get_Id_From_Relation_Subject(benchmark::State& state) {
    const std::string subject = "osmrel:1";

    for (auto _ : state) {
        auto components = olu::util::TtlHelper::parseId(subject);
    }
}
BENCHMARK(Get_Id_From_Relation_Subject);