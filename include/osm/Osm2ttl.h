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

#ifndef OSM_LIVE_UPDATES_OSM2TTL_H
#define OSM_LIVE_UPDATES_OSM2TTL_H

#include <osm2rdf/config/Config.h>
#include <osm2rdf/util/Output.h>

namespace olu::osm {

    class Osm2ttl {
    public:
        // Converts osm data to ttl triplets
        std::vector<std::string> convert(std::vector<std::string> &osmData);
    private:
        template <typename T>
        void run(const osm2rdf::config::Config& config) const;
        static void writeToInputFile(std::vector<std::string> &osmElements);
        static std::string removePrefixes(const std::string& data);
        [[nodiscard]] static std::vector<std::string> readTripletsFromOutputFile(const osm2rdf::config::Config& config) ;
    };

} // namespace olu::osm

#endif //OSM_LIVE_UPDATES_OSM2TTL_H
