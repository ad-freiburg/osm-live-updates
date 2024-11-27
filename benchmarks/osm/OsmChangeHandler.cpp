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


//// ---------------------------------------------------------------------------
//static void Create_And_Run_Insert_Query_Node(benchmark::State& state) {
//    auto config((olu::config::Config()));
//    config.sparqlEndpointUri = "http://host.docker.internal:7007/osm-planet/";
//
//    std::string path = "/src/tests/data/";
//    std::ifstream xmlFile (path + "node.osm");
//    std::string content( (std::istreambuf_iterator<char>(xmlFile) ),
//                         (std::istreambuf_iterator<char>()) );
//
//    pt::ptree tree;
//    olu::util::XmlReader::populatePTreeFromString(content, tree);
//    auto nodeElement = tree.get_child("osm.node");
//
//    auto och = olu::osm::OsmChangeHandler(config, std::string());
//    auto osmElements = och.getOsmElementsForInsert("node", nodeElement);
//
//    auto osm2ttl = olu::osm::Osm2ttl();
//    auto output = osm2ttl.convert();
//
//    std::ifstream ifs(output);
//    std::string data((std::istreambuf_iterator<char>(ifs)),
//                     (std::istreambuf_iterator<char>()));
//
//    std::vector<std::string> triplets;
//    for (std::string line; std::getline(ifs, line); )
//    {
//        triplets.push_back(line);
//    }
//
//    for (auto _ : state) {
//        och.createAndRunInsertQuery(triplets, "node");
//    }
//}
//BENCHMARK(Create_And_Run_Insert_Query_Node);

// ---------------------------------------------------------------------------
//static void Get_Triples_From_Converted_Data(benchmark::State& state) {
//    auto config((olu::config::Config()));
//    config.sparqlEndpointUri = "http://host.docker.internal:7007/osm-planet/";
//
//    std::string path = "/src/tests/data/";
//    std::ifstream xmlFile (path + "node.osm");
//    std::string content( (std::istreambuf_iterator<char>(xmlFile) ),
//                         (std::istreambuf_iterator<char>()) );
//
//    pt::ptree tree;
//    olu::util::XmlReader::populatePTreeFromString(content, tree);
//    auto nodeElement = tree.get_child("osm.node");
//
//    auto och = olu::osm::OsmChangeHandler(config, std::string());
//    auto osmElements = och.getOsmElementsForInsert("node", nodeElement);
//
//    auto osm2ttl = olu::osm::Osm2ttl();
//    auto output = osm2ttl.convert();
//
//    std::ifstream ifs(output);
//    std::string data((std::istreambuf_iterator<char>(ifs)),
//                     (std::istreambuf_iterator<char>()));
//
//    std::vector<std::string> triplets;
//    for (std::string line; std::getline(ifs, line); )
//    {
//        triplets.push_back(line);
//    }
//    for (auto _ : state) {
//        olu::osm::OsmChangeHandler::getTriplesFromConvertedData(triplets, "node");
//    }
//}
//BENCHMARK(Get_Triples_From_Converted_Data);

// ---------------------------------------------------------------------------
//static void Get_Prefixes_From_Converted_Data(benchmark::State& state) {
//    auto config((olu::config::Config()));
//    config.sparqlEndpointUri = "http://host.docker.internal:7007/osm-planet/";
//
//    std::string path = "/src/tests/data/";
//    std::ifstream xmlFile (path + "node.osm");
//    std::string content( (std::istreambuf_iterator<char>(xmlFile) ),
//                         (std::istreambuf_iterator<char>()) );
//
//    pt::ptree tree;
//    olu::util::XmlReader::populatePTreeFromString(content, tree);
//    auto nodeElement = tree.get_child("osm.node");
//
//    auto och = olu::osm::OsmChangeHandler(config, std::string());
//    auto osmElements = och.getOsmElementsForInsert("node", nodeElement);
//
//    auto osm2ttl = olu::osm::Osm2ttl();
//    auto output = osm2ttl.convert();
//
//    std::ifstream ifs(output);
//    std::string data((std::istreambuf_iterator<char>(ifs)),
//                     (std::istreambuf_iterator<char>()));
//
//    std::vector<std::string> triplets;
//    for (std::string line; std::getline(ifs, line); )
//    {
//        triplets.push_back(line);
//    }
//
//    for (auto _ : state) {
//        olu::osm::OsmChangeHandler::getPrefixesFromConvertedData(triplets);
//    }
//}
//BENCHMARK(Get_Prefixes_From_Converted_Data);

// ---------------------------------------------------------------------------
//static void Get_Osm_Elements_For_Insert_Node(benchmark::State& state) {
//    auto config((olu::config::Config()));
//    config.sparqlEndpointUri = "http://host.docker.internal:7007/osm-planet/";
//
//    std::string path = "/src/tests/data/";
//    std::ifstream xmlFile (path + "node.osm");
//    std::string content( (std::istreambuf_iterator<char>(xmlFile) ),
//                         (std::istreambuf_iterator<char>()) );
//
//    pt::ptree tree;
//    olu::util::XmlReader::populatePTreeFromString(content, tree);
//    auto nodeElement = tree.get_child("osm.node");
//
//    auto och = olu::osm::OsmChangeHandler(config, std::string());
//    for (auto _ : state) {
//        och.getOsmElementsForInsert("node", nodeElement);
//    }
//}
//BENCHMARK(Get_Osm_Elements_For_Insert_Node);

// ---------------------------------------------------------------------------
static void Handle_Change_Delete_Node(benchmark::State& state) {
    std::string path = "/src/tests/data/delete_node.osc";

    auto config((olu::config::Config()));
    config.sparqlEndpointUri = "http://host.docker.internal:7007/osm-planet/";

    auto och = olu::osm::OsmChangeHandler(config, std::string());

    for (auto _ : state) {
        och.run();
    }
}
BENCHMARK(Handle_Change_Delete_Node);

// ---------------------------------------------------------------------------
static void Handle_Change_Insert_Node(benchmark::State& state) {
    std::string path = "/src/tests/data/insert_node.osc";

    auto config((olu::config::Config()));
    config.sparqlEndpointUri = "http://host.docker.internal:7007/osm-planet/";

    auto och = olu::osm::OsmChangeHandler(config, std::string());

    for (auto _ : state) {
        och.run();
    }
}
BENCHMARK(Handle_Change_Insert_Node);

// ---------------------------------------------------------------------------
static void Handle_Change_Modify_Node(benchmark::State& state) {
    std::string path = "/src/tests/data/modify_node.osc";

    auto config((olu::config::Config()));
    config.sparqlEndpointUri = "http://host.docker.internal:7007/osm-planet/";

    auto och = olu::osm::OsmChangeHandler(config, std::string());

    for (auto _ : state) {
        och.run();
    }
}
BENCHMARK(Handle_Change_Modify_Node);