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
            std::cout << "Start handling change file at path: " << _config.pathToOsmChangeFile << std::endl;
            _och.handleChange(_config.pathToOsmChangeFile, false);
        } else {
            _latestState = _odf.fetchLatestDatabaseState();
            auto sequenceNumber = decideStartSequenceNumber();
            std::cout << "Start at sequence number: " << sequenceNumber << std::endl;
            std::cout << "Latest sequence number: " << _latestState.sequenceNumber << std::endl;
            while (sequenceNumber <= _latestState.sequenceNumber) {
                auto pathToOsmChangeFile = _odf.fetchDiffWithSequenceNumber(sequenceNumber);
                std::cout << "Handling diff with sequence number: " << sequenceNumber << std::endl;
                _och.handleChange(pathToOsmChangeFile, true);
                sequenceNumber++;
            }
        }

        std::cout << "DONE" << std::endl;
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
