// Copyright 2024, University of Freiburg
// Authors: Nicolas von Trott <nicolasvontrott@gmail.com>.

// This file is part of osm-live-updates.
//
// osm-live-updates is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// osm-live-updates is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with osm-live-updates.  If not, see <https://www.gnu.org/licenses/>.

#ifndef OSM_LIVE_UPDATES_OSMUPDATER_H
#define OSM_LIVE_UPDATES_OSMUPDATER_H


#include <sparql/QueryWriter.h>

#include "OsmChangeHandler.h"
#include "config/Config.h"
#include "osm/StatisticsHandler.h"
#include "osm/OsmDataFetcher.h"
#include "osm/OsmReplicationServerHelper.h"

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
        StatisticsHandler _stats;
        OsmReplicationServerHelper _repServer;
        std::unique_ptr<OsmDataFetcher> _odf;
        sparql::QueryWriter _queryWriter;

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
        void decideStartSequenceNumber();

        /**
        * Download all change files from the given sequence number to the `latest` one, and store
        * them in the /changes directory.
        */
        void fetchChangeFiles();

        /**
        * Uses osmium to merge all change files in the /changes directory into a single one.
        */
        void mergeChangeFiles(const std::string &pathToChangeFileDir);

        /**
        * Delete all files in the /changes dir
        */
        static void clearChangesDir();

        /**
        * Delete /tmp dir
        */
        static void deleteTmpDir();

        /**
         * Compares the version of osm2rdf on the SPARQL endpoint with the one used in this program
         * and logs a warning if they differ.
         */
        void checkOsm2RdfVersions() const;

        /**
         * Inserts "dateModified", "updatesCompleteUntil" and, if used, "replicationServer"
         * metadata triples to the SPARQL endpoint.
         *
         * dateModified: The date when the last update was made to the database. For each run of olu
         * one dateModified triple is inserted.
         *
         * updatesCompleteUntil: The sequence number until which the updates are complete. There is
         * always only one updatesCompleteUntil triple in the database, which is updated with each
         * run of olu.
         *
         * replicationServer: The replication server URI used to fetch the change files for the last
         * update.
         */
        void insertMetadataTriples(OsmChangeHandler &och);

        /**
         * Applies the user-specified bounding box or polygon to the change files
         * by using the 'extract' option from the 'osmium-tool'
         */
        void applyBoundaries() const;

        /**
         * Reads the osm2rdf options from the SPARQL endpoint and stores them in the config object.
         *
         * Also checks if a separate IRI is used for untagged nodes and sets the corresponding
         * value in the config object.
         */
        void readOsm2RdfOptionsFromEndpoint();
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
