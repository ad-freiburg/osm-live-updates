//
// Created by Nicolas von Trott on 11.09.24.
//

#include <iostream>
#include "osm/OsmUpdater.h"

namespace olu::osm {
    OsmUpdater::OsmUpdater(config::Config &config)
    : _config(config), _odf(OsmDataFetcher(config)), _och(OsmChangeHandler(config)) {
        _latestState = _odf.fetchLatestDatabaseState();
    }

    void OsmUpdater::run(int fromSequenceNumber) {
        auto sequenceNumber = fromSequenceNumber;
        while (sequenceNumber <= _latestState.sequenceNumber) {
            auto pathToOsmChangeFile = _odf.fetchDiffWithSequenceNumber(fromSequenceNumber);
            _och.handleChange(pathToOsmChangeFile);
            sequenceNumber++;
        }
    }
}
