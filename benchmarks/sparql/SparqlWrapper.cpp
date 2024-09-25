//
// Created by Nicolas von Trott on 25.09.24.
//

#include "osm/Osm2ttl.h"
#include "osm/OsmDataFetcher.h"

#include "benchmark/benchmark.h"
#include "config/Constants.h"
#include "sparql/QueryWriter.h"

// ---------------------------------------------------------------------------
static void Run_Query_For_Node_Location(benchmark::State& state) {
    auto config((olu::config::Config()));
    config.sparqlEndpointUri = "http://host.docker.internal:7007/osm-planet/";

    auto sparqlWrapper = olu::sparql::SparqlWrapper(config);
    auto query = olu::sparql::QueryWriter::writeQueryForNodeLocation(1);
    sparqlWrapper.setMethod(olu::util::GET);
    sparqlWrapper.setQuery(query);
    sparqlWrapper.setPrefixes(olu::config::constants::PREFIXES_FOR_NODE_LOCATION);

    for (auto _ : state) {
        auto response = sparqlWrapper.runQuery();
    }
}
BENCHMARK(Run_Query_For_Node_Location);

// ---------------------------------------------------------------------------
static void Run_Query_For_Node_Deletion(benchmark::State& state) {
    auto config((olu::config::Config()));
    config.sparqlEndpointUri = "http://host.docker.internal:7007/osm-planet/";

    auto sparqlWrapper = olu::sparql::SparqlWrapper(config);
    auto query = olu::sparql::QueryWriter::writeDeleteQuery("osmnode:1");
    sparqlWrapper.setPrefixes(olu::config::constants::PREFIXES_FOR_DELETE_QUERY);
    sparqlWrapper.setQuery(query);
    sparqlWrapper.setMethod(olu::util::POST);

    for (auto _ : state) {
        auto response = sparqlWrapper.runQuery();
    }
}
BENCHMARK(Run_Query_For_Node_Deletion);

BENCHMARK_MAIN();