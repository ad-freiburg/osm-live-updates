//
// Created by Nicolas von Trott on 28.08.24.
//

#include "util/WktHelper.h"
#include "config/Constants.h"
#include <boost/regex.hpp>
#include <string>

namespace cnst = olu::config::constants;

std::string olu::osm::WktHelper::createNodeFromPoint(const long long &nodeId,
                                                     const std::string& pointAsWkt) {
    const boost::regex pattern(R"(POINT\(([-+]?[0-9]*\.?[0-9]+)\s+([-+]?[0-9]*\.?[0-9]+)\))");
    boost::smatch match;

    std::string lat;
    std::string lon;
    if (boost::regex_search(pointAsWkt, match, pattern)) {
        lon = match[1];
        lat = match[2];
    } else {
        std::string message = "No WKT Point found in " + pointAsWkt;
        throw WktHelperException(message.c_str());
    }

    return "<node id=\"" + std::to_string(nodeId) + "\" lat=\"" + lat + "\" lon=\"" + lon + "\"/>";
}

std::string
olu::osm::WktHelper::createWayFromReferences(long long wayId, const std::set<long long> &nodeRefs) {
    std::string dummyWay = "<way id=\"" + std::to_string(wayId) + "\">";
    for (auto nodeId: nodeRefs) {
        dummyWay += "<nd ref=\"" + std::to_string(nodeId) + "\"/>";
    }
    dummyWay += "</way>";
    return dummyWay;
}

std::string
olu::osm::WktHelper::createRelationFromReferences(long long relationId,
                                                  const std::vector<std::string> &members) {
    std::vector<std::string> nodeReferences;
    std::vector<std::string> wayReferences;
    std::vector<std::string> relationReferences;

    for (const auto& member: members) {
        if (member.starts_with(cnst::OSM_NODE_URI)) {
            std::string id = member.substr(cnst::OSM_NODE_URI.length());
            std::string reference = R"(<member type="node" ref=")" + id + "\"/>";
            nodeReferences.emplace_back(reference);
        }

        if (member.starts_with(cnst::OSM_WAY_URI)) {
            std::string id = member.substr(cnst::OSM_WAY_URI.length());
            std::string reference = R"(<member type="way" ref=")" + id + "\"/>";
            wayReferences.emplace_back(reference);
        }

        if (member.starts_with(cnst::OSM_REL_URI)) {
            std::string id = member.substr(cnst::OSM_REL_URI.length());
            std::string reference = R"(<member type="relation" ref=")" + id + "\"/>";
            relationReferences.emplace_back(reference);
        }
    }

    std::string dummyRelation = "<relation id=\"" + std::to_string(relationId) + "\" >";
    for (const auto& nodeRef: nodeReferences) {
        dummyRelation += nodeRef;
    }

    for (const auto& wayRef: wayReferences) {
        dummyRelation += wayRef;
    }

    for (const auto& relRef: relationReferences) {
        dummyRelation += relRef;
    }

    dummyRelation += "</relation>";

    return dummyRelation;
}