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

#include "osm/Relation.h"

#include <sstream>

#include "util/XmlReader.h"

namespace olu::osm {
    void Relation::setType(std::string const &type) {
        this->type = type;
    }

    void Relation::setTimestamp(std::string const &timestamp) {
        // Ensure the timestamp ends with 'Z' to indicate UTC which is needed for osmium
        if (timestamp.ends_with("Z")) {
            this->timestamp = timestamp;
        } else {
            this->timestamp = timestamp + "Z";
        }
    }

    void Relation::setVersion(version_t const &version) {
        this->version = version;
    }

    void Relation::setChangesetId(changeset_id_t const &changeset_id) {
        this->changeset_id = changeset_id;
    }

    void Relation::addMember(const RelationMember& member) {
        this->members.push_back(member);
    }

    void Relation::addTag(const std::string& key, const std::string& value) {
        tags.emplace_back(key, util::XmlHelper::xmlEncode(value));
    }

    std::string Relation::getXml() const {
        std::ostringstream oss;

        oss << "<relation id=\"";
        oss << std::to_string(this->id);
        oss << "\"";

        if (this->version > 0) {
            oss << " version=\"";
            oss << this->version;
            oss << "\"";
        }

        if (this->changeset_id > 0) {
            oss << " changeset=\"";
            oss << this->changeset_id;
            oss << "\"";
        }

        if (!this->timestamp.empty()) {
            oss << " timestamp=\"";
            oss << this->timestamp;
            oss << "\"";
        }

        oss << ">";

        for (const auto &[id, type, role] : this->members) {
            oss << "<member type=\"";

            switch (type) {
                case OsmObjectType::NODE:
                    oss << config::constants::XML_TAG_NODE;
                    break;
                case OsmObjectType::WAY:
                    oss << config::constants::XML_TAG_WAY;
                    break;
                case OsmObjectType::RELATION:
                    oss << config::constants::XML_TAG_REL;
                    break;
            }

            oss << "\" ref=\"";
            oss << std::to_string(id);
            oss << "\" role=\"";
            oss << role;
            oss << "\"/>";
        }

        oss << R"(<tag k="type" v=")";
        oss << this->type;
        oss << "\"/>";

        for (const auto& [key, value] : this->tags) {
            oss << "<tag k=\"";
            oss << key;
            oss << "\" v=\"";
            oss << value;
            oss << "\"/>";
        }

        oss << "</relation>";

        return oss.str();
    }

}
