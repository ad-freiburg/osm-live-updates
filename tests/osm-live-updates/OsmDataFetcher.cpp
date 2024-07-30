//
// Created by Nicolas von Trott on 30.07.24.
//

#include "osm-live-updates/OsmDataFetcher.h"
#include "config/Constants.h"
#include "gtest/gtest.h"

namespace olu {
    TEST(OsmDataFetcher, fetchLatestSequenceNumber) {
        auto osmDataFetcher = OsmDataFetcher(OsmDiffGranularity::MINUTE);
        osmDataFetcher.fetchLatestSequenceNumber();

        // Todo: How to test? The latest sequence number will be different every minute/day/hour
    }
}