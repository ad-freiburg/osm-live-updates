//
// Created by Nicolas von Trott on 24.11.24.
//

#include "osm/Relation.h"
#include "util/XmlReader.h"

#include <sstream>

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
        tags.emplace_back(key, util::XmlReader::xmlEncode(value));
    }

    std::string Relation::getXml() const {
        std::ostringstream oss;

        oss << "<relation id=\"";
        oss << std::to_string(this->id);
        oss << "\"";

        if (!this->timestamp.empty()) {
            oss << " timestamp=\"";
            oss << this->timestamp;
            oss << "Z\"";
        }

        oss << ">";

        for (const auto &[id, osmTag, role] : this->members) {
            oss << "<member type=\"";
            oss << osmTag;
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
