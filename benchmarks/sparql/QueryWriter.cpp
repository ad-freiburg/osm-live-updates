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

#include "sparql/QueryWriter.h"

#include "benchmark/benchmark.h"

// _________________________________________________________________________________________________
static void writeQueryForRelations(benchmark::State& state) {
    const std::set<olu::id_t> relIds = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50};
    const auto qw = olu::sparql::QueryWriter(olu::config::Config());

    for (auto _ : state) {
        auto encoded = qw.writeQueryForRelations(relIds);
    }
}
BENCHMARK(writeQueryForRelations);

// _________________________________________________________________________________________________
static void writeInsertQuery(benchmark::State& state) {
    const std::vector<std::string> relIds = {"a b c", "d e f", "g h i", "j k l", "m n o", "p q r", "s t u", "v w x", "y z a1", "b2 c3 d4", "e5 f6 g7", "h8 i9 j0", "k1 l2 m3", "n4 o5 p6", "q7 r8 s9", "t0 u1 v2", "w3 x4 y5", "z6 a7 b8", "c9 d0 e1","a b c", "d e f", "g h i", "j k l", "m n o", "p q r", "s t u", "v w x", "y z a1", "b2 c3 d4", "e5 f6 g7", "h8 i9 j0", "k1 l2 m3", "n4 o5 p6", "q7 r8 s9", "t0 u1 v2", "w3 x4 y5", "z6 a7 b8", "c9 d0 e1","a b c", "d e f", "g h i", "j k l", "m n o", "p q r", "s t u", "v w x", "y z a1", "b2 c3 d4", "e5 f6 g7", "h8 i9 j0", "k1 l2 m3", "n4 o5 p6", "q7 r8 s9", "t0 u1 v2", "w3 x4 y5", "z6 a7 b8", "c9 d0 e1","a b c", "d e f", "g h i", "j k l", "m n o", "p q r", "s t u", "v w x", "y z a1", "b2 c3 d4", "e5 f6 g7", "h8 i9 j0", "k1 l2 m3", "n4 o5 p6", "q7 r8 s9", "t0 u1 v2", "w3 x4 y5", "z6 a7 b8", "c9 d0 e1","a b c", "d e f", "g h i", "j k l", "m n o", "p q r", "s t u", "v w x", "y z a1", "b2 c3 d4", "e5 f6 g7", "h8 i9 j0", "k1 l2 m3", "n4 o5 p6", "q7 r8 s9", "t0 u1 v2", "w3 x4 y5", "z6 a7 b8", "c9 d0 e1","a b c", "d e f", "g h i", "j k l", "m n o", "p q r", "s t u", "v w x", "y z a1", "b2 c3 d4", "e5 f6 g7", "h8 i9 j0", "k1 l2 m3", "n4 o5 p6", "q7 r8 s9", "t0 u1 v2", "w3 x4 y5", "z6 a7 b8", "c9 d0 e1","a b c", "d e f", "g h i", "j k l", "m n o", "p q r", "s t u", "v w x", "y z a1", "b2 c3 d4", "e5 f6 g7", "h8 i9 j0", "k1 l2 m3", "n4 o5 p6", "q7 r8 s9", "t0 u1 v2", "w3 x4 y5", "z6 a7 b8", "c9 d0 e1"};
    const auto qw = olu::sparql::QueryWriter(olu::config::Config());

    for (auto _ : state) {
        auto encoded = qw.writeInsertQuery(relIds);
    }
}
BENCHMARK(writeInsertQuery);
