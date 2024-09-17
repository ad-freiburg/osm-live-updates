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

#ifndef OSM_LIVE_UPDATES_OSMCHANGEHANDLER_H
#define OSM_LIVE_UPDATES_OSMCHANGEHANDLER_H

#include "sparql/SparqlWrapper.h"
#include "config/Config.h"
#include "osm/Osm2ttl.h"
#include "osm/OsmDataFetcher.h"

#include <boost/property_tree/ptree.hpp>

namespace pt = boost::property_tree;
namespace olu::osm {

    class OsmChangeHandler {
    public:
        explicit OsmChangeHandler(config::Config& config);

        // Processes a file in osmChange format ('https://wiki.openstreetmap.org/wiki/OsmChange')
        void handleChange(const std::string &pathToOsmChangeFile,
                          const bool &deleteChangeFile);

    private:
        config::Config _config;
        sparql::SparqlWrapper _sparql;
        Osm2ttl _osm2ttl;
        OsmDataFetcher _odf;

        void handleInsert(const std::string& elementTag, const pt::ptree& element);
        void handleDelete(const std::string& elementTag, const pt::ptree& element);
        void handleModify(const std::string& elementTag, const pt::ptree& element);

        // Populates a vector with all osm elements needed for conversion to ttl format, which
        // includes all nodes that are referenced in an 'way' element
        // The osm element is also embedded in the following xml element
        // <osm version="0.6">...</osm>
        std::vector<std::string> getOsmElementsForInsert(const std::string& elementTag,
                                                         const pt::ptree& element);

        void deleteChangeFile(const std::string& pathToOsmChangeFile);
    };
} // namespace olu::osm

#endif //OSM_LIVE_UPDATES_OSMCHANGEHANDLER_H
