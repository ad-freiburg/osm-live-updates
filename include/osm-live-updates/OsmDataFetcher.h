//
// Created by Nicolas von Trott on 26.07.24.
//

#ifndef OSM_LIVE_UPDATES_OSMDATAFETCHER_H
#define OSM_LIVE_UPDATES_OSMDATAFETCHER_H

namespace olu {

class OsmDataFetcher {
public:
    static void fetchDiffWithSequenceNumber(int& sequenceNumber);
};

} // namespace olu

#endif //OSM_LIVE_UPDATES_OSMDATAFETCHER_H
