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

#ifndef OSM_LIVE_UPDATES_OSMDATAFETCHER_H
#define OSM_LIVE_UPDATES_OSMDATAFETCHER_H

#include "osm/OsmDiffGranularities.h"
#include "osm/OsmDatabaseState.h"
#include "util/CacheFile.h"
#include "sparql/SparqlWrapper.h"

#include <string>
#include <boost/property_tree/ptree.hpp>

namespace olu::osm {

    class OsmDataFetcher {
    public:
        explicit OsmDataFetcher(olu::config::Config& config);

        // Fetches the database state (sequence number and timestamp) for the given sequence number
        // from the server
        [[nodiscard]] OsmDatabaseState fetchDatabaseState(int sequenceNumber) const;

        // Fetches the database state (sequence number and timestamp) of the latest diff from the
        // server
        [[nodiscard]] OsmDatabaseState fetchLatestDatabaseState() const;

        // Fetches the gzipped .osc change file from the server, writes it to a file and returns the
        // path to the file
        std::string fetchDiffWithSequenceNumber(int &sequenceNumber) ;

        // Fetches all nodes that are referenced in the given way element
        std::vector<std::string> fetchNodeReferencesForWay(const boost::property_tree::ptree& way);

        std::vector<std::string> fetchNodesFromSparql(const std::vector<std::string> &nodeIds);


        static std::string fetchNode(std::string &nodeId, bool extractNodeElement = false);

        // Runs a SPARQL query to get the latest timestamp of any node in the database
        std::string fetchLatestTimestampOfAnyNode();
        // Fetches the 'nearest' sequence number for the given timestamp from the server.
        int fetchNearestSequenceNumberForTimestamp(const std::string& timeStamp) const;

    private:
        olu::config::Config _config;
        olu::sparql::SparqlWrapper _sparqlWrapper;

        static OsmDatabaseState extractStateFromStateFile(const std::string& stateFile);
    protected:
        util::CacheFile _cacheFile = util::CacheFile("/tmp/dataFetcherCache");
    };

    class OsmDataFetcherException : public std::exception {
    private:
        std::string message;

    public:
        explicit OsmDataFetcherException(const char* msg) : message(msg) { }

        [[nodiscard]] const char* what() const noexcept override {
            return message.c_str();
        }
    };

} // namespace olu

#endif //OSM_LIVE_UPDATES_OSMDATAFETCHER_H
