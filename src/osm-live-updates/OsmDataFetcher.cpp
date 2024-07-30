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

#include "osm-live-updates/OsmDataFetcher.h"
#include "config/Constants.h"
#include "util/URLHelper.h"
#include "util/HttpClient.h"

#include <string>
#include <vector>
#include <boost/regex.hpp>
#include <osmium/handler.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/visitor.hpp>

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
    std::string readBuffer = util::HttpClient::perform_get(url);

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
void OsmDataFetcher::fetchDiffWithSequenceNumber(std::string &sequenceNumber) {
    // Build url for diff file
    std::string sequenceNumberFormatted = util::URLHelper::formatSequenceNumberForUrl(sequenceNumber);
    std::vector<std::string> pathSegments;
    pathSegments.emplace_back(constants::OSM_REPLICATION_BASE_URL);
    pathSegments.emplace_back(urlSegmentFor.at(_diffGranularity));
    pathSegments.emplace_back(sequenceNumberFormatted + constants::OSM_CHANGE_FILE_EXTENSION);
    std::string url = util::URLHelper::buildUrl(pathSegments);

    // Get Diff file from OSM server
    std::string readBuffer = util::HttpClient::perform_get(url);

    // Decompress gzipped file
    auto otypes = osmium::osm_entity_bits::node | osmium::osm_entity_bits::way;
    osmium::io::Reader reader{readBuffer, otypes};
}

} // namespace olu



