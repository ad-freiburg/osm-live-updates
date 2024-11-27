//
// Created by Nicolas von Trott on 24.11.24.
//

#include "osm/Relation.h"

namespace olu::osm {

    void Relation::setType(std::string const &type) {
        this->type = type;
    }

    void Relation::addNodeAsMember(id_t const& id, Role const& role) {
        this->nodes.insert(RelationMember(id, role));
    }
    void Relation::addWayAsMember(id_t const& id, Role const& role) {
        this->ways.insert(RelationMember(id, role));
    }

    void Relation::addRelationAsMember(id_t const& id, Role const& role) {
        this->relations.insert(RelationMember(id, role));
    }

    std::string Relation::getXml() const {
        std::string xml = "<relation id=\"" + std::to_string(this->id) + "\"><member type=\"";

        for (const auto &[id, role] : this->nodes) {
            xml += R"(node" ref=")" + std::to_string(id) + "\" role=\"" + role;
        }

        for (const auto &[id, role] : this->ways) {
            xml += R"(way" ref=")" + std::to_string(id) + "\" role=\"" + role;
        }

        for (const auto &[id, role] : this->relations) {
            xml += R"(relation" ref=")" + std::to_string(id) + "\" role=\"" + role;
        }

        xml += R"("/><tag k="type" v=")" + this->type + "\"/></relation>";

        return xml;
    }

}
