//
// Created by Nicolas von Trott on 26.07.24.
//

#ifndef OSM_LIVE_UPDATES_OSMDATAFETCHER_H
#define OSM_LIVE_UPDATES_OSMDATAFETCHER_H

#include <string>

namespace olu {

class OsmDataFetcher {
public:
    // Fetches the sequence number of the latest diff from the osm server and returns it
    static std::string fetchLatestSequenceNumber();
    static void fetchDiffWithSequenceNumber(int& sequenceNumber);
};

} // namespace olu

#endif //OSM_LIVE_UPDATES_OSMDATAFETCHER_H
