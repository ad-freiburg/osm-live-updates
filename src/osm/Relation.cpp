//
// Created by Nicolas von Trott on 24.11.24.
//

#include "osm/Relation.h"

namespace olu::osm {
    void Relation::setType(std::string const &type) {
        this->type = type;
    }

    void Relation::setTimestamp(std::string const &timestamp) {
        this->timestamp = timestamp;
    }

    void Relation::addMember(const RelationMember& member) {
        this->members.push_back(member);
    }

    void Relation::addTag(const std::string& key, const std::string& value) {
        tags.emplace_back(key, value);
    }

    std::string Relation::getXml() const {
        const std::string timestamp = this->timestamp.empty() ? "" : " timestamp=\"" + this->timestamp + "Z\"";
        std::string xml = "<relation id=\"" + std::to_string(this->id) + "\" " + timestamp + ">";

        for (const auto &[id, osmTag, role] : this->members) {
            xml += "<member type=\"" + osmTag + "\" ref=\"" + std::to_string(id) + "\" role=\"" + role + "\"/>";
        }

        xml += R"(<tag k="type" v=")" + this->type + "\"/>";

        for (const auto& [key, value] : this->tags) {
            xml += "<tag k=\"";
            xml += key;
            xml += "\" v=\"";
            xml += value;
            xml += "\"/>";
        }

        xml += "</relation>";

        return xml;
    }

}
