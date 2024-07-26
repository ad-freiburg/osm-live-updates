#include <iostream>
#include "../include/sparql/QueryWriter.h"
#include "../include/osm-live-updates/OsmDataFetcher.h"

#include <string>
#include <vector>

int main() {
    olu::OsmDataFetcher::fetchLatestSequenceNumber();

    return 0;
}