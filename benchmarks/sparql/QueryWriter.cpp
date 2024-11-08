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
#include "sparql/QueryWriter.h"
#include "benchmark/benchmark.h"
#include "config/Constants.h"
#include "util/XmlReader.h"
#include "osm/Osm2ttl.h"

// ---------------------------------------------------------------------------
static void Write_Query_For_Latest_Node_Timestamp(benchmark::State& state) {
    for (auto _ : state) {
        olu::sparql::QueryWriter::writeQueryForLatestNodeTimestamp();
    }
}
BENCHMARK(Write_Query_For_Latest_Node_Timestamp);

// ---------------------------------------------------------------------------
static void Write_Query_For_Node_Location(benchmark::State& state) {
    for (auto _ : state) {
        olu::sparql::QueryWriter::writeQueryForNodeLocation(123456789);
    }
}
BENCHMARK(Write_Query_For_Node_Location);

// ---------------------------------------------------------------------------
static void Write_Delete_Query(benchmark::State& state) {
    for (auto _ : state) {
        olu::sparql::QueryWriter::writeNodeDeleteQuery(1);
    }
}
BENCHMARK(Write_Delete_Query);

// ---------------------------------------------------------------------------
static void Write_Insert_Query_Node(benchmark::State& state) {
    std::string path = "/src/tests/data/";

    std::ifstream ifs(path + "node.osm");
    std::string nodeElement((std::istreambuf_iterator<char>(ifs)),
                            (std::istreambuf_iterator<char>()));
    std::vector<std::string> elements;
    elements.push_back(nodeElement);

    auto osm2ttl = olu::osm::Osm2ttl();
    auto output = osm2ttl.convert();

    std::ifstream ifs2(output);
    std::string data((std::istreambuf_iterator<char>(ifs2)),
                     (std::istreambuf_iterator<char>()));

    std::vector<std::string> triplets;
    for (std::string line; std::getline(ifs2, line); )
    {
        triplets.push_back(line);
    }

    for (auto _ : state) {
        olu::sparql::QueryWriter::writeInsertQuery(triplets);
    }
}
BENCHMARK(Write_Insert_Query_Node);

// ---------------------------------------------------------------------------
static void Write_Insert_Query_Way(benchmark::State& state) {
    std::string path = "/src/tests/data/";

    std::ifstream ifs(path + "wayWithReferences.osm");
    std::string nodeElement((std::istreambuf_iterator<char>(ifs)),
                            (std::istreambuf_iterator<char>()));
    std::vector<std::string> elements;
    elements.push_back(nodeElement);

    auto osm2ttl = olu::osm::Osm2ttl();
    auto output = osm2ttl.convert();

    std::ifstream ifs2(output);
    std::string data((std::istreambuf_iterator<char>(ifs2)),
                     (std::istreambuf_iterator<char>()));

    std::vector<std::string> triplets;
    for (std::string line; std::getline(ifs2, line); )
    {
        triplets.push_back(line);
    }

    for (auto _ : state) {
        olu::sparql::QueryWriter::writeInsertQuery(triplets);
    }

    for (auto _ : state) {
        olu::sparql::QueryWriter::writeInsertQuery(triplets);
    }
}
BENCHMARK(Write_Insert_Query_Way);