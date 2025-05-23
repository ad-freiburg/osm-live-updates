//
// Created by Nicolas von Trott on 15.05.25.
//

#include "config/Constants.h"
#include "osm/OsmReplicationServerHelper.h"
#include "util/URLHelper.h"
#include "util/HttpRequest.h"

#include <vector>
#include <boost/regex.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include <fstream>

namespace cnst = olu::config::constants;
namespace olu::osm {

    // _____________________________________________________________________________________________
    OsmDatabaseState
    OsmReplicationServerHelper::fetchDatabaseStateFromUrl(const std::string &stateFilePath) const {
        const std::string url = util::URLHelper::buildUrl({
            _config.changeFileDirUri,
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

    // _____________________________________________________________________________________________
    OsmDatabaseState
    OsmReplicationServerHelper::fetchDatabaseStateForSeqNumber(int sequenceNumber) const {
        const auto stateFileName =
                util::URLHelper::formatSequenceNumberForUrl(sequenceNumber) + "." +
                cnst::PATH_TO_STATE_FILE;
        return fetchDatabaseStateFromUrl(stateFileName);
    }

    // _____________________________________________________________________________________________
    OsmDatabaseState OsmReplicationServerHelper::fetchLatestDatabaseState() const {
        return fetchDatabaseStateFromUrl(cnst::PATH_TO_STATE_FILE);
    }

    // _____________________________________________________________________________________________
    std::string OsmReplicationServerHelper::fetchChangeFile(int &sequenceNumber) {
        std::string diffFilename = util::URLHelper::formatSequenceNumberForUrl(sequenceNumber)
                                   + cnst::OSM_CHANGE_FILE_EXTENSION
                                   + cnst::GZIP_EXTENSION;
        std::string url = util::URLHelper::buildUrl({
            _config.changeFileDirUri,
            diffFilename});

        // Get change file from server and write to cache file.
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

        return fileName;
    }

    // _____________________________________________________________________________________________
    OsmDatabaseState
    OsmReplicationServerHelper::fetchDatabaseStateForTimestamp(const std::string& timeStamp) const {
        // We start with the latest state file on the replication server
        OsmDatabaseState state = fetchLatestDatabaseState();
        while (true) {
            // We have found state file at which we have to start, if the timestamp from the state
            // file is further in the past than the latest timestamp from the sparql endpoint. (We
            // can lexigraphically compare timestamps because they are ISO-formatted
            // YYYY-MM-DDTHH:MM:SS)
            if (state.timeStamp <= timeStamp) {
                return state;
            }

            auto sequenceNumber = state.sequenceNumber;
            sequenceNumber--;

            try {
                // Try the next older database state next
                state = fetchDatabaseStateForSeqNumber(sequenceNumber);
            } catch (std::exception &e) {
                std::cerr << e.what() << std::endl;
                const std::string msg = "Exception while trying to determine database state "
                                        "for timestamp: " + timeStamp;
                throw OsmReplicationServerHelperException(msg.c_str());
            }
        }
    }

    // _____________________________________________________________________________________________
    OsmDatabaseState
    OsmReplicationServerHelper::extractStateFromStateFile(const std::string& stateFile) {
        OsmDatabaseState state;
        // The state file always contains the sequence number like "sequenceNumber=4290"
        const boost::regex regexSeqNumber("sequenceNumber=(\\d+)");
        if (boost::smatch matchSeqNumber;
            regex_search(stateFile, matchSeqNumber, regexSeqNumber)) {
            state.sequenceNumber = std::stoi(matchSeqNumber[1]);
        } else {
            const std::string msg = "Could not extract sequence number from state file: " +
                                    stateFile;
            throw OsmReplicationServerHelperException(msg.c_str());
        }

        // The state file always contains the timestamp like "timestamp=2025-01-04T21\:21\:15Z"
        const boost::regex regexTimestamp(
                R"(timestamp=([0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}\\:[0-9]{2}\\:[0-9]{2}Z))");
        if (boost::smatch matchTimestamp;
            regex_search(stateFile, matchTimestamp, regexTimestamp)) {
            state.timeStamp = matchTimestamp[1];
        } else {
            throw OsmReplicationServerHelperException(
                    "Timestamp of latest database state could not be fetched");
        }

        return state;
    }

}