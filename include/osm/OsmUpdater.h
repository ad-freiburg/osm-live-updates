//
// Created by Nicolas von Trott on 11.09.24.
//

#ifndef OSM_LIVE_UPDATES_OSMUPDATER_H
#define OSM_LIVE_UPDATES_OSMUPDATER_H

#include <string>
#include "config/Config.h"
#include "OsmDatabaseState.h"
#include "OsmDataFetcher.h"
#include "OsmChangeHandler.h"

namespace olu::osm {
    class OsmUpdater {
    public:
        explicit OsmUpdater(config::Config& config);

        void run(int fromSequenceNumber);

    private:
        config::Config& _config;
        OsmDataFetcher _odf;
        OsmChangeHandler _och;
        OsmDatabaseState _latestState;
    };
} // namespace olu::osm

#endif //OSM_LIVE_UPDATES_OSMUPDATER_H
