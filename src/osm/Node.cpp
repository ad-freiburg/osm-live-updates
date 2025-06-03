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
#include <config/Config.h>

#include "spatialjoin/WKTParse.h"

// _________________________________________________________________________________________________
olu::osm::Node::Node(const id_t id, const osmium::Location& location) {
    this->id = id;
    this->loc = location;
}

// _________________________________________________________________________________________________
olu::osm::Node::Node(const id_t id, const wktPoint_t& locationAsWkt) {
    this->id = id;

    try {
        // Location can be given as a WKT point, e.g., "POINT(13.5690032 42.7957187)"
        // or with a prefix "\"POINT(1.622847 42.525981)\"^^<http://www.opengis.net/ont/geosparql#wktLiteral>"
        if (locationAsWkt.starts_with("P")) {
            const auto point = util::geo::pointFromWKT<double>(locationAsWkt);
            this->loc = osmium::Location( point.getX(), point.getY() );
        } else {
            const auto point = util::geo::pointFromWKT<double>(parseWktPoint(locationAsWkt));
            this->loc = osmium::Location( point.getX(), point.getY() );
        }
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
    oss.precision(config::Config::DEFAULT_WKT_PRECISION);
    oss << std::fixed << "<node id=\"" << this->getId() << "\" lat=\"" << this->loc.lat()
        << "\" lon=\"" << this->loc.lon() << "\"/>";

    return oss.str();
}

// _________________________________________________________________________________________________
std::string olu::osm::Node::parseWktPoint(const std::string &wktPoint) {
    auto string = wktPoint.substr(1, wktPoint.find('^') - 2);
    return string;
}
