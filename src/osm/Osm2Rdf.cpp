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

#include "osm/Osm2Rdf.h"

#include <osm2ttl/config/Config.h>
#include <osm2ttl/util/Output.h>
#include <osm2ttl/ttl/Writer.h>
#include <osm2ttl/osm/OsmiumHandler.h>
#include <osm2ttl/ttl/Format.h>

#include "iostream"

namespace olu::osm {
// _________________________________________________________________________________________________
Osm2Rdf::Osm2Rdf() {
    auto config = osm2ttl::config::Config();
    osm2ttl::util::Output output{config, config.output};
    if (!output.open()) {
        std::cerr << "Error opening outputfile: " << config.output << std::endl;
        exit(1);
    }

    osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER> writer{config, &output};
    writer.writeHeader();

    osm2ttl::osm::OsmiumHandler osmiumHandler{config, &writer};
    osmiumHandler.handle();

    // All work done, close output
    output.close();
}

} // namespace olu::osm