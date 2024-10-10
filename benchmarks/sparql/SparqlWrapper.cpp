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

#include <boost/property_tree/ptree.hpp>
#include "osm/Osm2ttl.h"
#include "osm/OsmDataFetcher.h"

#include "benchmark/benchmark.h"
#include "config/Constants.h"
#include "sparql/QueryWriter.h"
#include "util/XmlReader.h"
#include "osm/OsmChangeHandler.h"

// ---------------------------------------------------------------------------
static void Set_Prefixes(benchmark::State& state) {
    auto config((olu::config::Config()));
    config.sparqlEndpointUri = "http://host.docker.internal:7007/osm-planet/";
    auto sparqlWrapper = olu::sparql::SparqlWrapper(config);

    for (auto _ : state) {
        sparqlWrapper.setPrefixes(olu::config::constants::PREFIXES_FOR_WAY_DELETE_QUERY);
    }
}
BENCHMARK(Set_Prefixes);

// ---------------------------------------------------------------------------
static void Set_Query(benchmark::State& state) {
    auto config((olu::config::Config()));
    config.sparqlEndpointUri = "http://host.docker.internal:7007/osm-planet/";

    std::string path = "/src/tests/data/";
    std::ifstream xmlFile (path + "node.osm");
    std::string content( (std::istreambuf_iterator<char>(xmlFile) ),
                         (std::istreambuf_iterator<char>()) );

    pt::ptree tree;
    olu::util::XmlReader::populatePTreeFromString(content, tree);
    auto nodeElement = tree.get_child("osm.node");

    auto och = olu::osm::OsmChangeHandler(config);
    auto osmElements = och.getOsmElementsForInsert("node", nodeElement);

    auto osm2ttl = olu::osm::Osm2ttl();
    auto ttl = osm2ttl.convert(osmElements);

    auto prefixes = olu::osm::OsmChangeHandler::getPrefixesFromConvertedData(ttl);
    auto triples = olu::osm::OsmChangeHandler::getTriplesFromConvertedData(ttl, "node", nodeElement);

    auto sparqlWrapper = olu::sparql::SparqlWrapper(config);
    auto query = olu::sparql::QueryWriter::writeInsertQuery(triples);

    for (auto _ : state) {
        sparqlWrapper.setQuery(query);
    }
}
BENCHMARK(Set_Query);

// ---------------------------------------------------------------------------
static void Set_Method(benchmark::State& state) {
    auto config((olu::config::Config()));
    config.sparqlEndpointUri = "http://host.docker.internal:7007/osm-planet/";
    auto sparqlWrapper = olu::sparql::SparqlWrapper(config);

    for (auto _ : state) {
        sparqlWrapper.setMethod(olu::util::POST);
    }
}
BENCHMARK(Set_Method);

// ---------------------------------------------------------------------------
static void Run_Query_For_Node_Insertion(benchmark::State& state) {
    auto config((olu::config::Config()));
    config.sparqlEndpointUri = "http://host.docker.internal:7007/osm-planet/";

    std::string path = "/src/tests/data/";
    std::ifstream xmlFile (path + "node.osm");
    std::string content( (std::istreambuf_iterator<char>(xmlFile) ),
                         (std::istreambuf_iterator<char>()) );

    pt::ptree tree;
    olu::util::XmlReader::populatePTreeFromString(content, tree);
    auto nodeElement = tree.get_child("osm.node");

    auto och = olu::osm::OsmChangeHandler(config);
    auto osmElements = och.getOsmElementsForInsert("node", nodeElement);

    auto osm2ttl = olu::osm::Osm2ttl();
    auto ttl = osm2ttl.convert(osmElements);

    auto prefixes = olu::osm::OsmChangeHandler::getPrefixesFromConvertedData(ttl);
    auto triples = olu::osm::OsmChangeHandler::getTriplesFromConvertedData(ttl, "node", nodeElement);

    auto sparqlWrapper = olu::sparql::SparqlWrapper(config);
    auto query = olu::sparql::QueryWriter::writeInsertQuery(triples);

    for (auto _ : state) {
        sparqlWrapper.setPrefixes(olu::config::constants::PREFIXES_FOR_WAY_DELETE_QUERY);
        sparqlWrapper.setQuery(query);
        sparqlWrapper.setMethod(olu::util::POST);

        auto response = sparqlWrapper.runQuery();
    }
}
BENCHMARK(Run_Query_For_Node_Insertion);

// ---------------------------------------------------------------------------
static void Run_Query_For_Node_Location(benchmark::State& state) {
    auto config((olu::config::Config()));
    config.sparqlEndpointUri = "http://host.docker.internal:7007/osm-planet/";

    auto sparqlWrapper = olu::sparql::SparqlWrapper(config);
    auto query = olu::sparql::QueryWriter::writeQueryForNodeLocation(1);

    for (auto _ : state) {
        sparqlWrapper.setMethod(olu::util::GET);
        sparqlWrapper.setQuery(query);
        sparqlWrapper.setPrefixes(olu::config::constants::PREFIXES_FOR_NODE_LOCATION);

        auto response = sparqlWrapper.runQuery();
    }
}
BENCHMARK(Run_Query_For_Node_Location);

//// ---------------------------------------------------------------------------
//static void Run_Query_For_Node_Deletion(benchmark::State& state) {
//    auto config((olu::config::Config()));
//    config.sparqlEndpointUri = "http://host.docker.internal:7007/osm-planet/";
//
//    auto sparqlWrapper = olu::sparql::SparqlWrapper(config);
//    auto query = olu::sparql::QueryWriter::writeDeleteQuery("osmnode:1");
//
//    for (auto _ : state) {
//        sparqlWrapper.setPrefixes(olu::config::constants::PREFIXES_FOR_DELETE_QUERY);
//        sparqlWrapper.setQuery(query);
//        sparqlWrapper.setMethod(olu::util::POST);
//
//        auto response = sparqlWrapper.runQuery();
//    }
//}
//BENCHMARK(Run_Query_For_Node_Deletion);

// ---------------------------------------------------------------------------
static void Clear_Cache(benchmark::State& state) {
    auto config((olu::config::Config()));
    config.sparqlEndpointUri = "http://host.docker.internal:7007/osm-planet/";

    for (auto _ : state) {
        auto sparqlWrapper = olu::sparql::SparqlWrapper(config);
        sparqlWrapper.clearCache();
    }
}
BENCHMARK(Clear_Cache);
