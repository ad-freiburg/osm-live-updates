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
#include "util/CacheFile.h"
#include "sparql/SparqlWrapper.h"

#include <string>
#include <boost/property_tree/ptree.hpp>

namespace olu {

class OsmDataFetcher {
public:
    explicit OsmDataFetcher(OsmDiffGranularity diffGranularity, olu::sparql::SparqlWrapper& _sparqlWrapper);

    // Fetches the sequence number of the latest diff from the osm server and returns it
    std::string fetchLatestSequenceNumber();

    // Fetches the gzipped .osc change file from the server, writes it to a file and returns the
    // path to the file
    std::string fetchDiffWithSequenceNumber(std::string& sequenceNumber);

    // Fetches all nodes that are referenced in the given way element
    static std::vector<std::string> fetchNodeReferencesForWay(const boost::property_tree::ptree& way);

    // Asynchronously Fetches all nodes that are referenced in the given way element
    static std::vector<std::string> fetchNodes(const std::vector<std::string> &nodeIds);

    std::vector<std::string> fetchNodesFromSparql(const std::vector<std::string> &nodeIds);


    static std::string fetchNode(std::string &nodeId, bool extractNodeElement = false);
private:
    OsmDiffGranularity _diffGranularity;
    olu::sparql::SparqlWrapper _sparqlWrapper;
protected:
    util::CacheFile _cacheFile = util::CacheFile("/tmp/dataFetcherCache");
};

} // namespace olu

#endif //OSM_LIVE_UPDATES_OSMDATAFETCHER_H
