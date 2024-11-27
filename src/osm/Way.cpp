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

#include "osm/Way.h"

namespace olu::osm {

    void Way::addMember(id_t nodeId) {
        members.emplace_back(nodeId);
    }

    std::string Way::getXml() const {
        std::string xml = "<way id=\"" + std::to_string(this->getId()) + "\">";
        for (const auto nodeId: this->members) {
            xml += "<nd ref=\"" + std::to_string(nodeId) + "\"/>";
        }
        xml += "</way>";
        return xml;
    }

}