// Copyright 2025, University of Freiburg
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

#include "osm/OsmReplicationServerHelper.h"

#include <vector>
#include <iostream>
#include <fstream>
#include <regex>

#include "omp.h"

#include "config/Constants.h"
#include "util/Exceptions.h"
#include "util/URLHelper.h"
#include "util/HttpRequest.h"
#include "util/Logger.h"
#include "util/Time.h"

namespace cnst = olu::config::constants;

inline constexpr int BATCH_SIZE = 10;

// _________________________________________________________________________________________________
olu::osm::OsmDatabaseState
olu::osm::OsmReplicationServerHelper::fetchDatabaseStateFromUrl(
    const std::string &stateFilePath) const {
    const std::string url = util::URLHelper::buildUrl({
        _config.replicationServerUri,
        stateFilePath});

    std::string response;
    try {
        // Fetch state file from replication server
        response = util::HttpRequest(util::GET, url).perform();
    } catch (const std::exception& e) {
        if (std::string(e.what()).find("404") != std::string::npos) {
            std::cerr << "The state file was not found on the replication server. Perhaps it is "
                         "too long in the past?" << std::endl;
        }

        const std::string msg = "Exception while trying to fetch state file from url: " + url;
        throw OsmReplicationServerHelperException(msg.c_str());
    }

    return extractStateFromStateFile(response);
}

// _________________________________________________________________________________________________
olu::osm::OsmDatabaseState
olu::osm::OsmReplicationServerHelper::fetchDatabaseStateForSeqNumber(const int sequenceNumber) const {
    const auto stateFileName =
            util::URLHelper::formatSequenceNumberForUrl(sequenceNumber) + "." +
            cnst::PATH_TO_STATE_FILE;
    return fetchDatabaseStateFromUrl(stateFileName);
}

// _________________________________________________________________________________________________
olu::osm::OsmDatabaseState olu::osm::OsmReplicationServerHelper::fetchLatestDatabaseState() const {
    return fetchDatabaseStateFromUrl(cnst::PATH_TO_STATE_FILE);
}

// _________________________________________________________________________________________________
void olu::osm::OsmReplicationServerHelper::fetchChangeFile(int &sequenceNumber) {
    std::string diffFilename = util::URLHelper::formatSequenceNumberForUrl(sequenceNumber)
                               + cnst::OSM_CHANGE_FILE_EXTENSION
                               + cnst::GZIP_EXTENSION;
    std::string url = util::URLHelper::buildUrl({
        _config.replicationServerUri,
        diffFilename});

    // Get change file from server and write to a cache file.
    std::string response;
    try {
        response = util::HttpRequest(util::GET, url).perform();
    } catch (const std::exception& e) {
        if (std::string(e.what()).find("404") != std::string::npos) {
            std::cerr << "The change file is not found on the replication server" << std::endl;
        }

        const std::string msg = "Exception while trying to fetch change file for sequence "
                                "number " + sequenceNumber;
        throw OsmReplicationServerHelperException(msg.c_str());
    }

    std::string fileName = cnst::PATH_TO_CHANGE_FILE_DIR
                           + std::to_string(sequenceNumber)
                           + cnst::OSM_CHANGE_FILE_EXTENSION
                           + cnst::GZIP_EXTENSION;

    std::ofstream outputFile;
    outputFile.open(fileName);
    outputFile << response;
    outputFile.close();
}

// _________________________________________________________________________________________________
void olu::osm::OsmReplicationServerHelper::fetchDatabaseStateForTimestamp(
    const std::string &timeStamp) const {
    // We have found a state file at which we have to start if the timestamp from the state
    // file is further in the past than the latest timestamp from the sparql endpoint.
    // (We can lexicographically compare timestamps because they are ISO-formatted
    // "YYYY-MM-DDTHH:MM:SS")
    if (_stats->getLatestDatabaseState().timeStamp <= timeStamp) {
        const std::string msg = "The latest database state on the reapplication server (" + _stats->
                                getLatestDatabaseState().timeStamp + ") is before or "
                                "equal to the timestamp: " + timeStamp;
        throw util::DatabaseUpToDateException(msg.c_str());
    }

    util::Logger::log(util::LogEvent::INFO,
                      "Find matching database state on replication server...");
    // Fetch database states in batches of BATCH_SIZE until we find a state file that has a matching
    // timestamp.
    auto toSeqNum = _stats->getLatestDatabaseState().sequenceNumber;

    // If the osm planet replication server is used, we can make an educated guess for the sequence
    // number based on the timestamp, since the sequences are generated with a granularity of
    // minutes, hours or days.
    if (const auto guessedSeqNum = makeEducatedGuessForSequenceNumber(timeStamp, toSeqNum);
        guessedSeqNum > 0) {
        // Fetch the database states for the guessed sequence number and the one before and after
        // it
        for (const auto databaseStates = fetchDatabaseStatesForSequenceNumbers(
                 guessedSeqNum - 1, guessedSeqNum + 1);
             const auto &fetchedState: databaseStates) {
            if (fetchedState <= OsmDatabaseState(timeStamp)) {
                _stats->setStartDatabaseState(fetchedState);
                util::Logger::log(util::LogEvent::INFO,
                                  "Matching database state on replication server is: "
                                  + olu::osm::to_string(fetchedState));
                return;
            }
        }
    }

    std::cout << "Educated guess for sequence number did not work" << std::endl;

    // If another replication server is used or the educated guess fails, we have to find a matching
    // database state by brute-force iteration through the sequence numbers.
    while (toSeqNum > 0) {
        const auto fromSeqNum = std::max(toSeqNum - BATCH_SIZE, 0);

        for (auto databaseStates = fetchDatabaseStatesForSequenceNumbers(fromSeqNum, toSeqNum);
             const auto& fetchedState : databaseStates) {
            if (fetchedState.timeStamp <= timeStamp) {
                _stats->setStartDatabaseState(fetchedState);
                util::Logger::log(util::LogEvent::INFO,
                                  "Matching database state on replication server is: "
                                  + olu::osm::to_string(fetchedState));
                return;
            }
        }

        toSeqNum = std::max(fromSeqNum, 0);
    }

    const std::string msg = "Could not find matching database state for timestamp: " + timeStamp;
    throw OsmReplicationServerHelperException(msg.c_str());
}

// _________________________________________________________________________________________________
std::vector<olu::osm::OsmDatabaseState>
olu::osm::OsmReplicationServerHelper::fetchDatabaseStatesForSequenceNumbers(const int fromSeqNum,
    const int toSeqNum) const {
    std::vector<OsmDatabaseState> states;
#pragma omp parallel for
    for (int seqNum = fromSeqNum; seqNum <= toSeqNum; seqNum++) {
        OsmDatabaseState state;
        try {
            state = fetchDatabaseStateForSeqNumber(seqNum);
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            const std::string msg = "Exception while trying to fetch state file for sequence "
                                    "number: " + std::to_string(seqNum);
            throw OsmReplicationServerHelperException(msg.c_str());
        }

#pragma omp critical
        {
            states.push_back(state);
        }
    }

    // Since we fetched the states in parallel, we have to sort them by sequence number to make sure
    // they are in the correct order.
    std::ranges::sort(states, [](const OsmDatabaseState& a, const OsmDatabaseState& b) {
        return a.sequenceNumber > b.sequenceNumber;
    });

    return states;
}

// _________________________________________________________________________________________________
olu::osm::OsmDatabaseState
olu::osm::OsmReplicationServerHelper::extractStateFromStateFile(const std::string& stateFile) {
    OsmDatabaseState state;
    // The state file always contains the sequence number like "sequenceNumber=4290"
    const std::regex regexSeqNumber("sequenceNumber=(\\d+)");
    if (std::smatch matchSeqNumber;
        regex_search(stateFile, matchSeqNumber, regexSeqNumber)) {
        state.sequenceNumber = std::stoi(matchSeqNumber[1]);
    } else {
        const std::string msg = "Could not extract sequence number from state file: " +
                                stateFile;
        throw OsmReplicationServerHelperException(msg.c_str());
    }

    // The state file always contains the timestamp like "timestamp=2025-01-04T21\:21\:15Z"
    const std::regex regexTimestamp(
            R"(timestamp=([0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}\\:[0-9]{2}\\:[0-9]{2}Z))");
    if (std::smatch matchTimestamp;
        regex_search(stateFile, matchTimestamp, regexTimestamp)) {
        state.timeStamp = matchTimestamp[1];
    } else {
        throw OsmReplicationServerHelperException(
                "Timestamp of latest database state could not be fetched");
    }

    return state;
}

// _________________________________________________________________________________________________
int olu::osm::OsmReplicationServerHelper::makeEducatedGuessForSequenceNumber(
    const std::string &timeStamp, const int &latestSequenceNumber) const {
    // We can only make an educated guess for sequence numbers if the OSM planet replication server
    // that provides minute, hour, and day diffs is used
    if (!_config.replicationServerUri.starts_with("https://planet.osm.org/replication/")) {
        return -1;
    }

    int sequencesSinceLatest = 0;
    if (_config.replicationServerUri.ends_with("day/")) {
        sequencesSinceLatest = util::daysBetweenNowAndTimestamp(timeStamp);
    } else if (_config.replicationServerUri.ends_with("hour/")) {
        sequencesSinceLatest = util::hoursBetweenNowAndTimestamp(timeStamp);
    } else if (_config.replicationServerUri.ends_with("minute/")) {
        sequencesSinceLatest = util::minutesBetweenNowAndTimestamp(timeStamp);
    } else {
        return -1; // Not a valid replication server URL for making an educated guess
    }

    return latestSequenceNumber - sequencesSinceLatest;
}