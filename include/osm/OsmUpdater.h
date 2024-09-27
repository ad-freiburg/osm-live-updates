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

    /**
     * Manages the update process.
     *
     * This class is responsible for managing the OSM change files for the update process.
     * Depending on the user input, a single OSM change file is processed or, if a directory is
     * available, all outstanding change files are used.
     */
    class OsmUpdater {
    public:
        explicit OsmUpdater(config::Config& config);

        /// Starts the update process.
        void run();
    private:
        config::Config& _config;
        OsmDataFetcher _odf;
        OsmChangeHandler _och;
        OsmDatabaseState _latestState;

        /**
         * Decides which sequence number to start from.
         *
         * There are three ways to choose the sequence
         * number:
         *  1.  The user can define the sequence number
         *  2.  The user can define a timestamp for which the 'nearest' sequence number will be
         *      determined
         *  3.  If the user has not specified a sequence number or timestamp, the node with the
         *      most recent timestamp is used as a reference point to determine the ‘nearest’
         *      sequence number
         * The 'nearest' sequence number is the first sequence number whose timestamp is earlier
         * than the timestamp given by the user or latest node.
         *
         * @return The sequence number to start from
         */
        [[nodiscard]] int decideStartSequenceNumber();
    };

} // namespace olu::osm

#endif //OSM_LIVE_UPDATES_OSMUPDATER_H
