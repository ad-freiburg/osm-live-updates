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

#include "osm/Node.h"

#include <iostream>
#include <sstream>
#include <string>

#include "spatialjoin/WKTParse.h"

/// The maximum number of decimals for the location in a node
static inline constexpr int MAX_NODE_LOC_PRECISION = 7;

// _________________________________________________________________________________________________
olu::osm::Node::Node(const id_t id, const osmium::Location& location) {
    this->id = id;
    this->loc = location;
}

// _________________________________________________________________________________________________
olu::osm::Node::Node(const id_t id, const wktPoint_t& locationAsWkt) {
    this->id = id;

    try {
        const auto point = util::geo::pointFromWKT<double>(locationAsWkt);
        this->loc = osmium::Location( point.getX(), point.getY() );
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        const std::string message = "Location can not be inferred from WKT point: "
                                        + locationAsWkt;
        throw NodeException(message.c_str());
    }
}

// _________________________________________________________________________________________________
std::string olu::osm::Node::getXml() const {
    std::ostringstream oss;
    oss.precision(MAX_NODE_LOC_PRECISION);
    oss << std::fixed << "<node id=\"" << this->getId() << "\" lat=\"" << this->loc.lat()
        << "\" lon=\"" << this->loc.lon() << "\"/>";

    return oss.str();
}
