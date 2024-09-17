//
// Created by Nicolas von Trott on 11.09.24.
//

#include <iostream>
#include "osm/OsmUpdater.h"

namespace olu::osm {
    OsmUpdater::OsmUpdater(config::Config &config)
    : _config(config), _odf(OsmDataFetcher(config)), _och(OsmChangeHandler(config)) { }

    void OsmUpdater::run() {
        if (!(_config.pathToOsmChangeFile.empty())) {
            _och.handleChange(_config.pathToOsmChangeFile);
        } else {
            _latestState = _odf.fetchLatestDatabaseState();
            auto sequenceNumber = decideStartSequenceNumber();
            while (sequenceNumber <= _latestState.sequenceNumber) {
                auto pathToOsmChangeFile = _odf.fetchDiffWithSequenceNumber(sequenceNumber);
                _och.handleChange(pathToOsmChangeFile);
                sequenceNumber++;
            }
        }
    }

    int OsmUpdater::decideStartSequenceNumber() {
        if (_config.sequenceNumber > 0) {
            return _config.sequenceNumber;
        }

        std::string timestamp;
        if (_config.timestamp.empty()) {
            timestamp = _odf.fetchLatestTimestampOfAnyNode();
        } else {
            timestamp = _config.timestamp;
        }

        return _odf.fetchNearestSequenceNumberForTimestamp(timestamp);
    }
}
