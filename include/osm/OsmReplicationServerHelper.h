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

#ifndef OSMREPLICATIONSERVERHELPER_H
#define OSMREPLICATIONSERVERHELPER_H

#include <string>

#include "config/Config.h"
#include "osm/OsmDatabaseState.h"

namespace olu::osm {
    /**
     *  Deals with the retrieval of osm change files from the replication server that is specified
     *  by the user.
     */
    class OsmReplicationServerHelper {
    public:
        explicit OsmReplicationServerHelper(config::Config config): _config(std::move(config)) { }

        /**
         * Fetches the database state (sequence number and timestamp) for the given sequence number
         * from the server
         *
         * @param sequenceNumber the sequence number to fetch the database state for
         * @return The database state for the provided sequence number
         */
        [[nodiscard]] OsmDatabaseState fetchDatabaseStateForSeqNumber(int sequenceNumber) const;

        /**
         * Fetches the database state (sequence number and timestamp) of the latest diff from the
         * server
         *
         * @return The latest database state on the server
         */
        [[nodiscard]] OsmDatabaseState fetchLatestDatabaseState() const;

        /**
         * Fetches the .osc change file from the server, writes it to a file and returns the path to
         * the file. The file might be compressed with gzip.
         *
         * @param sequenceNumber The sequence number to fetch the change file for
         * @return The path to the location of the fetched .osm Change file
         */
        std::string fetchChangeFile(int &sequenceNumber) ;

        /**
         * Fetches the 'nearest' database state for the given timestamp from the server, meaning the
         * first state which timestamp is before the given timestamp.
         *
         * @param timeStamp Timestamp to fetch the `nearest` database state for
         * @return The 'nearest' database state for the given timestamp
         */
        [[nodiscard]] OsmDatabaseState
        fetchDatabaseStateForTimestamp(const std::string& timeStamp) const;
    private:
        config::Config _config;

        /**
         * Sends a HTTP request to the replication server and tries to extract a data base state
         * from the returned state file.
         *
         * @param stateFilePath The path of the state file on the replication server
         * @return The database state for the state file at the provided file path
         */
        OsmDatabaseState fetchDatabaseStateFromUrl(const std::string& stateFilePath) const;

        /**
         * Extracts the database state from a state file. A state file contains a sequence number
         * and a timestamp and describes the state for an osm change file.
         *
         * @param stateFile The state file to extract the database state from.
         * @return The database state described by the state file
         */
        static OsmDatabaseState extractStateFromStateFile(const std::string& stateFile);
    };

    /**
     * Exception that can appear inside the `OsmReplicationServerHelper` class.
     */
    class OsmReplicationServerHelperException final : public std::exception {
        std::string message;
    public:
        explicit OsmReplicationServerHelperException(const char* msg) : message(msg) { }

        [[nodiscard]] const char* what() const noexcept override {
            return message.c_str();
        }
    };

}

#endif //OSMREPLICATIONSERVERHELPER_H
