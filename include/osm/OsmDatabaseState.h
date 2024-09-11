//
// Created by Nicolas von Trott on 11.09.24.
//

#ifndef OSM_LIVE_UPDATES_OSMDATABASESTATE_H
#define OSM_LIVE_UPDATES_OSMDATABASESTATE_H

#include <string>

namespace olu::osm {

    struct OsmDatabaseState {
        std::string timeStamp;
        int sequenceNumber;
    };

} // namespace olu::osm

#endif //OSM_LIVE_UPDATES_OSMDATABASESTATE_H
