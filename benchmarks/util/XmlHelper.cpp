
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

#include "util/XmlHelper.h"

#include <fstream>

// _________________________________________________________________________________________________
static void getNodeDummy(benchmark::State& state) {
    constexpr auto nodeId = 123456789;
    const auto nodeLocation = olu::lon_lat_t{"13.5690032", "42.7957187"};

    for (auto _ : state) {
        auto parsedId = olu::util::XmlHelper::getNodeDummy(nodeId, nodeLocation);
    }
}
BENCHMARK(getNodeDummy);

// _________________________________________________________________________________________________
static void getAndWriteNodeDummy(benchmark::State& state) {
    constexpr auto nodeId = 123456789;
    const auto nodeLocation = olu::lon_lat_t{"13.5690032", "42.7957187"};

    std::ofstream outFile("benchmark.txt");
    for (auto _ : state) {
        auto dummy = olu::util::XmlHelper::getNodeDummy(nodeId, nodeLocation);
        outFile.write(dummy.data(), dummy.size());
    }
    outFile.close();
}
BENCHMARK(getAndWriteNodeDummy);

// _________________________________________________________________________________________________
static void getWayDummy(benchmark::State& state) {
    constexpr auto wayId = 123456789;
    const auto members = olu::member_ids_t{123456789, 987654321, 1122334455,
                                           5566778899, 1029384756, 1234567890, 34567890};

    std::ofstream outFile("benchmark.txt");
    for (auto _ : state) {
        auto parsedId = olu::util::XmlHelper::getWayDummy(wayId, members, false);
    }
}
BENCHMARK(getWayDummy);

// _________________________________________________________________________________________________
static void getAndWriteWayDummy(benchmark::State& state) {
    constexpr auto wayId = 123456789;
    const auto members = olu::member_ids_t{123456789, 987654321, 1122334455,
                                           5566778899, 1029384756, 1234567890, 34567890};

    std::ofstream outFile("benchmark.txt");
    for (auto _ : state) {
        auto dummy = olu::util::XmlHelper::getWayDummy(wayId, members, false);
        outFile.write(dummy.data(), dummy.size());
    }
    outFile.close();
}
BENCHMARK(getAndWriteWayDummy);

// _________________________________________________________________________________________________
static void getRelationDummy(benchmark::State& state) {
    constexpr auto relationId = 123456789;
    const olu::osm::relation_members_t members = {
        olu::osm::RelationMember{123456789, olu::osm::OsmObjectType::NODE, std::string("member1")},
        olu::osm::RelationMember{234523458, olu::osm::OsmObjectType::WAY, std::string("member2")},
        olu::osm::RelationMember{234523487, olu::osm::OsmObjectType::RELATION, std::string("member2")},
        olu::osm::RelationMember{345634563, olu::osm::OsmObjectType::NODE, std::string("member3")},
        olu::osm::RelationMember{456745674, olu::osm::OsmObjectType::WAY, std::string("member4")},
        olu::osm::RelationMember{567856789, olu::osm::OsmObjectType::RELATION, std::string("member5")},
        olu::osm::RelationMember{678967890, olu::osm::OsmObjectType::NODE, std::string("member6")},
        olu::osm::RelationMember{789078901, olu::osm::OsmObjectType::WAY, std::string("member7")},
        olu::osm::RelationMember{890189012, olu::osm::OsmObjectType::RELATION, std::string("member8")},
        olu::osm::RelationMember{901290123, olu::osm::OsmObjectType::NODE, std::string("member9")},
    };

    for (auto _ : state) {
        auto dummy = olu::util::XmlHelper::getRelationDummy(relationId, "type", members);
    }
}
BENCHMARK(getRelationDummy);

// _________________________________________________________________________________________________
static void getAndWriteRelationDummy(benchmark::State& state) {
    constexpr auto relationId = 123456789;
    const olu::osm::relation_members_t members = {
        olu::osm::RelationMember{123456789, olu::osm::OsmObjectType::NODE, std::string("member1")},
        olu::osm::RelationMember{234523458, olu::osm::OsmObjectType::WAY, std::string("member2")},
        olu::osm::RelationMember{234523487, olu::osm::OsmObjectType::RELATION, std::string("member2")},
        olu::osm::RelationMember{345634563, olu::osm::OsmObjectType::NODE, std::string("member3")},
        olu::osm::RelationMember{456745674, olu::osm::OsmObjectType::WAY, std::string("member4")},
        olu::osm::RelationMember{567856789, olu::osm::OsmObjectType::RELATION, std::string("member5")},
        olu::osm::RelationMember{678967890, olu::osm::OsmObjectType::NODE, std::string("member6")},
        olu::osm::RelationMember{789078901, olu::osm::OsmObjectType::WAY, std::string("member7")},
        olu::osm::RelationMember{890189012, olu::osm::OsmObjectType::RELATION, std::string("member8")},
        olu::osm::RelationMember{901290123, olu::osm::OsmObjectType::NODE, std::string("member9")},
    };

    std::ofstream outFile("benchmark.txt");

    for (auto _ : state) {
        auto dummy = olu::util::XmlHelper::getRelationDummy(relationId, "type", members);
        outFile.write(dummy.data(), dummy.size());
    }

    outFile.close();
}
BENCHMARK(getAndWriteRelationDummy);