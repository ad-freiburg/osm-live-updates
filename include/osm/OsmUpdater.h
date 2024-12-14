//
// Created by Nicolas von Trott on 11.09.24.
//

#ifndef OSM_LIVE_UPDATES_OSMUPDATER_H
#define OSM_LIVE_UPDATES_OSMUPDATER_H

#include <iostream>

#include "config/Config.h"
#include "osm/OsmDatabaseState.h"
#include "osm/OsmDataFetcher.h"

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
        explicit OsmUpdater(const config::Config &config);

        /// Starts the update process.
        void run();
    private:
        config::Config _config;
        OsmDataFetcher _odf;
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

        /**
        * Download all change files from the given seqence number to the `latest` one, and store
        * them in the /changes directory.
        */
        void fetchChangeFiles(int sequenceNumber);

        /**
        * Uses osmium to merge all change files in the /changes directory into a single one.
        */
        static void mergeChangeFiles(const std::string &pathToChangeFileDir);

        /**
        * Delete all files in the /changes dir
        */
        static void clearChangesDir();

        /**
        * Delete /tmp dir
        */
        static void deleteTmpDir();
    };

    /**
    * Exception that can appear inside the `OsmUpdater` class.
    */
    class OsmUpdaterException final : public std::exception {
        std::string message;
    public:
        explicit OsmUpdaterException(const char* msg) : message(msg) { }

        [[nodiscard]] const char* what() const noexcept override {
            return message.c_str();
        }
    };

} // namespace olu::osm

#endif //OSM_LIVE_UPDATES_OSMUPDATER_H
