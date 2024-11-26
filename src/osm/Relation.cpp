//
// Created by Nicolas von Trott on 24.11.24.
//

#include "osm/Relation.h"

#include "osm2rdf/osm/RelationMember.h"

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
        std::string xml = "<relation id=\"" + std::to_string(this->id) + "\">";

        for (const auto&[id, role] : nodes) {
            xml += R"(<member type="node" ref=")" + std::to_string(id) + "\" role=\"" + role + "\"/>";
        }

        for (const auto&[id, role] : ways) {
            xml += R"(<member type="way" ref=")" + std::to_string(id) + "\" role=\"" + role + "\"/>";
        }

        for (const auto&[id, role] : relations) {
            xml += R"(<member type="relation" ref=")" + std::to_string(id) + "\" role=\"" + role + "\"/>";
        }

        xml += R"(<tag k="type" v=")" + this->type + "\"/></relation>";

        return xml;
    }

}
