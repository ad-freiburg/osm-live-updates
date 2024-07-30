#include <iostream>
#include "../include/sparql/QueryWriter.h"
#include "../include/osm-live-updates/OsmDataFetcher.h"

#include <string>
#include <vector>

int main() {
    auto osmDataFetcher = olu::OsmDataFetcher(OsmDiffGranularity::MINUTE);
    std::string seqNumber = osmDataFetcher.fetchLatestSequenceNumber();
    osmDataFetcher.fetchDiffWithSequenceNumber(seqNumber);

    return 0;
}