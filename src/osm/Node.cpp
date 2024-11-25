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

#include <boost/regex.hpp>
#include <iostream>
#include <sstream>
#include <string>

namespace olu::osm {

    Node::Node(u_id id, const WKTPoint& locationAsWkt) {
        this->id = id;

        const boost::regex pattern(R"(POINT\((-?\d+)\.(\d+)\s(-?\d+)\.(\d+)\))");
        boost::smatch match;

        std::string lat; std::string lon;
        if (boost::regex_search(locationAsWkt, match, pattern)) {
            lon = match[1] + match[2];
            lat = match[3] + match[4];
        } else {
            std::string message = "Location can not be inferred from WKT point: " + locationAsWkt;
            throw NodeException(message.c_str());
        }

        int32_t i_lat; int32_t i_lon;
        try {
            i_lat = stoi(lat);
            i_lon = stoi(lon);
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            std::string message = "Can not cast lon: " + lon + " or lat: " + lat + " to int";
            throw NodeException(message.c_str());
        }

        this->loc = osmium::Location( i_lon, i_lat );
    }

    std::string Node::getXml() const {
        std::ostringstream oss;
        oss.precision(7);
        oss << std::fixed << "<node id=\"" << this->getId() << "\" lat=\"" << this->loc.lat()
            << "\" lon=\"" << this->loc.lon() << "\"/>";

        return oss.str();
    }
}