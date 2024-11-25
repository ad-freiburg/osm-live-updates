//
// Created by Nicolas von Trott on 11.09.24.
//

#include <iostream>
#include "osm/OsmUpdater.h"
#include "osm2rdf/util/Time.h"

namespace olu::osm {
    OsmUpdater::OsmUpdater(config::Config &config)
    : _config(config), _odf(OsmDataFetcher(config)) { }

    void OsmUpdater::run() {
        // If the user has provided the path to an osm change file, only this change file will be
        // processed
        if (!(_config.pathToOsmChangeFile.empty())) {
            std::cout
            << osm2rdf::util::currentTimeFormatted()
            << "Start handling change file: "
            << std::endl;

            auto och(OsmChangeHandler(_config, _config.pathToOsmChangeFile));
            och.run();
        } else {
            _latestState = _odf.fetchLatestDatabaseState();
            auto sequenceNumber = decideStartSequenceNumber();

            std::cout
            << osm2rdf::util::currentTimeFormatted()
            << "Start at sequence number: "
            << sequenceNumber
            << std::endl;

            std::cout
            << osm2rdf::util::currentTimeFormatted()
            << "Latest sequence number: "
            << _latestState.sequenceNumber
            << std::endl;

            while (sequenceNumber <= _latestState.sequenceNumber) {
                auto pathToOsmChangeFile = _odf.fetchChangeFile(sequenceNumber);
                std::cout
                << osm2rdf::util::currentTimeFormatted()
                << "Handling change file for sequence number: "
                << sequenceNumber
                << std::endl;

                auto och(OsmChangeHandler(_config, pathToOsmChangeFile));
                och.run();
                sequenceNumber++;

                std::filesystem::remove(pathToOsmChangeFile);
            }
        }

        std::cout
        << osm2rdf::util::currentTimeFormatted()
        << "DONE"
        << std::endl;
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

        auto databaseState = _odf.fetchDatabaseStateForTimestamp(timestamp);
        return databaseState.sequenceNumber;
    }
}
