//
// Created by Nicolas von Trott on 20.05.25.
//

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