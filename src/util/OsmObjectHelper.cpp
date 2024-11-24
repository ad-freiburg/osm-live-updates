//
// Created by Nicolas von Trott on 28.08.24.
//

#include "util/OsmObjectHelper.h"
#include "config/Constants.h"
#include "util/XmlReader.h"
#include <string>

namespace cnst = olu::config::constants;

std::string
olu::osm::OsmObjectHelper::createWayFromReferences(long long wayId, const std::vector<long long> &nodeRefs) {
    std::string dummyWay = "<way id=\"" + std::to_string(wayId) + "\">";
    for (auto nodeId: nodeRefs) {
        dummyWay += "<nd ref=\"" + std::to_string(nodeId) + "\"/>";
    }
    dummyWay += "<tag k=\"type\" v=\"tmp\"/></way>";
    return dummyWay;
}

std::string
olu::osm::OsmObjectHelper::createRelationFromReferences(long long relationId,
                                                        const std::pair<std::string, std::vector<std::pair<std::string, std::string>>> &members) {
    std::vector<std::string> nodeReferences;
    std::vector<std::string> wayReferences;
    std::vector<std::string> relationReferences;

    for (const auto& member: members.second) {
        auto uri = member.first;
        auto role = member.second;

        if (uri.starts_with(cnst::OSM_NODE_URI)) {
            std::string id = uri.substr(cnst::OSM_NODE_URI.length());
            std::string reference = R"(<member type="node" ref=")" + id + "\" role=\"" + role + "\"/>";
            nodeReferences.emplace_back(reference);
        }

        if (uri.starts_with(cnst::OSM_WAY_URI)) {
            std::string id = uri.substr(cnst::OSM_WAY_URI.length());
            std::string reference = R"(<member type="way" ref=")" + id + "\" role=\"" + role + "\"/>";
            wayReferences.emplace_back(reference);
        }

        if (uri.starts_with(cnst::OSM_REL_URI)) {
            std::string id = uri.substr(cnst::OSM_REL_URI.length());
            std::string reference = R"(<member type="relation" ref=")" + id + "\" role=\"" + role + "\"/>";
            relationReferences.emplace_back(reference);
        }
    }

    std::string dummyRelation = "<relation id=\"" + std::to_string(relationId) + "\">";
    for (const auto& nodeRef: nodeReferences) {
        dummyRelation += nodeRef;
    }

    for (const auto& wayRef: wayReferences) {
        dummyRelation += wayRef;
    }

    for (const auto& relRef: relationReferences) {
        dummyRelation += relRef;
    }
    dummyRelation += "<tag k=\"type\" v=\"" + members.first + "\"/></relation>";

    return dummyRelation;
}

bool olu::osm::OsmObjectHelper::isMultipolygon(const boost::property_tree::ptree &relation) {
    for (const auto &result: relation.get_child("")) {
        if (result.first == "tag") {
            if (olu::util::XmlReader::readAttribute("<xmlattr>.k",
                                                    result.second) == "type" &&
                olu::util::XmlReader::readAttribute("<xmlattr>.v",
                                                    result.second) == "multipolygon") {
             return true;
            }
        }
    }
    return false;
}