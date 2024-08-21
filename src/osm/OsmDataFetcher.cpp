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

#include "osm/OsmDataFetcher.h"
#include "config/Constants.h"
#include "util/URLHelper.h"
#include "util/HttpRequest.h"
#include "util/XmlReader.h"

#include <string>
#include <vector>
#include <boost/regex.hpp>

namespace constants = olu::config::constants;

namespace olu {

// _________________________________________________________________________________________________
OsmDataFetcher::OsmDataFetcher(OsmDiffGranularity diffGranularity) {
    _diffGranularity = diffGranularity;
}

// _________________________________________________________________________________________________
std::string OsmDataFetcher::fetchLatestSequenceNumber() {
    // Build url for state file
    std::vector<std::string> pathSegments;
    pathSegments.emplace_back(constants::OSM_REPLICATION_BASE_URL);
    pathSegments.emplace_back(urlSegmentFor.at(_diffGranularity));
    pathSegments.emplace_back(constants::OSM_DIFF_STATE_FILE + constants::TXT_EXTENSION);
    std::string url = util::URLHelper::buildUrl(pathSegments);

    // Get state file from osm server
    auto request = util::HttpRequest(util::GET, url);
    std::string readBuffer = request.perform();

    // Extract sequence number from state file
    boost::regex regex("sequenceNumber=(\\d+)");
    boost::smatch match;
    if (boost::regex_search(readBuffer, match, regex)) {
        std::string number = match[1];
        return number;
    } else {
       // TODO: Throw error if nothing found.
    }
}

// _________________________________________________________________________________________________
std::string OsmDataFetcher::fetchDiffWithSequenceNumber(std::string &sequenceNumber) {
    // Build url for diff file
    std::string sequenceNumberFormatted = util::URLHelper::formatSequenceNumberForUrl(sequenceNumber);
    std::string diffFilename = sequenceNumberFormatted + constants::OSM_CHANGE_FILE_EXTENSION;
    std::vector<std::string> pathSegments;
    pathSegments.emplace_back(constants::OSM_REPLICATION_BASE_URL);
    pathSegments.emplace_back(urlSegmentFor.at(_diffGranularity));
    pathSegments.emplace_back(diffFilename);
    std::string url = util::URLHelper::buildUrl(pathSegments);

    // Get Diff file from server and write to cache file.
    std::string filePath = constants::DIFF_CACHE_FILE + sequenceNumber + constants::OSM_CHANGE_FILE_EXTENSION + constants::GZIP_EXTENSION;
    auto request = util::HttpRequest(util::GET, url);
    auto cacheFile = util::CacheFile(filePath);
    cacheFile.write(request.perform());
    cacheFile.close();
    return filePath;
}

// _________________________________________________________________________________________________
    std::string OsmDataFetcher::fetchNode(std::string &nodeId, bool extractNodeElement) {
    std::vector<std::string> pathSegments;
    pathSegments.emplace_back(constants::OSM_NODE_BASE_URL);
    pathSegments.emplace_back(nodeId);
    std::string url = util::URLHelper::buildUrl(pathSegments);

    auto request = util::HttpRequest(util::GET, url);
    std::string response = request.perform();

    if (extractNodeElement) {
        auto nodeElement = util::XmlReader::readNodeElement(response);
        return nodeElement;
    }

    return response;
}

// _________________________________________________________________________________________________
    std::vector<std::string> OsmDataFetcher::fetchNodeReferencesForWay(const boost::property_tree::ptree &way) {
    std::vector<std::string> referencedNodes;
    std::set<std::string> visitedNodes;

    for (const auto &child : way.get_child("")) {
        if (child.first != olu::config::constants::NODE_REFERENCE_TAG) {
            continue;
        }

        auto identifier = util::XmlReader::readAttribute(
                constants::REFERENCE_ATTRIBUTE,
                way);

        if (!visitedNodes.contains(identifier)) {
            visitedNodes.insert(identifier);

            auto nodeElement = fetchNode(identifier, true);
            referencedNodes.push_back(nodeElement);
        }
    }

    return referencedNodes;
}

} // namespace olu



