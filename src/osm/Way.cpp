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

#include <sstream>

namespace olu::osm {
    void Way::setTimestamp(std::string const &timestamp) {
        this->timestamp = timestamp;
    }

    void Way::addMember(id_t nodeId) {
        members.emplace_back(nodeId);
    }

    void Way::addTag(const std::string& key, const std::string& value) {
        tags.emplace_back(key, value);
    }

    std::string Way::getXml() const {
        std::ostringstream oss;

        oss << "<way id=\"";
        oss << std::to_string(this->getId());
        oss << "\"";

        if (!this->timestamp.empty()) {
            oss << " timestamp=\"";
            oss << this->timestamp;
            oss << "Z\"";
        }

        oss << ">";

        for (const auto nodeId: this->members) {
            oss << "<nd ref=\"";
            oss << std::to_string(nodeId);
            oss << "\"/>";
        }

        for (const auto& [key, value] : this->tags) {
            oss << "<tag k=\"";
            oss << key;
            oss << "\" v=\"";
            oss << value;
            oss << "\"/>";
        }

        oss << "</way>";
        return oss.str();
    }

}