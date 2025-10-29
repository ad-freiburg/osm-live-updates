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

#include "util/XmlHelper.h"

#include <string>
#include <regex>

#include "config/Constants.h"

namespace cnst = olu::config::constants;

// _______________________________________________________________________________________________
std::string olu::util::XmlHelper::getNodeDummy(const id_t &nodeId, const lon_lat_t &lonLat) {
    std::string result;
    result.reserve(64);
    result.append("<node id=\"")
          .append(std::to_string(nodeId))
          .append("\" lat=\"")
          .append(lonLat.second)
          .append("\" lon=\"")
          .append(lonLat.first)
          .append("\"/>");
    return result;
}

// _______________________________________________________________________________________________
std::string olu::util::XmlHelper::getWayDummy(const id_t &wayId, const member_ids_t &memberIds,
                                              const bool &hasTag) {
    std::string result;
    result.reserve(30 + 24 * memberIds.size());
    result.append("<way id=\"")
          .append(std::to_string(wayId))
          .append("\">");

    for (const auto nodeId : memberIds) {
        result.append("<nd ref=\"")
              .append(std::to_string(nodeId))
              .append("\"/>");
    }

    if (hasTag) {
        result.append(R"(<tag k="K" v="V"/>)");
    }
    result.append("</way>");

    return result;
}

// _________________________________________________________________________________________________
std::string olu::util::XmlHelper::getRelationDummy(const id_t &relationId,
                                                   const std::string_view &relationType,
                                                   const osm::relation_members_t &members) {
    std::string result;
    result.reserve(64 + relationType.size() + members.size() * 64);
    result.append("<relation id=\"")
          .append(std::to_string(relationId))
          .append("\">");

    for (const auto &[id, type, role] : members) {
        result.append("<member type=\"");

        switch (type) {
            case osm::OsmObjectType::NODE:
                result.append(config::constants::XML_TAG_NODE);
                break;
            case osm::OsmObjectType::WAY:
                result.append(config::constants::XML_TAG_WAY);
                break;
            case osm::OsmObjectType::RELATION:
                result.append(config::constants::XML_TAG_REL);
                break;
        }

        result.append("\" ref=\"")
              .append(std::to_string(id))
              .append("\" role=\"")
              .append(xmlEncode(role))
              .append("\"/>");
    }

    result.append("<tag k=\"type\" v=\"")
          .append(xmlEncode(std::string(relationType)))
          .append("\"/>")
          .append("</relation>");

return result;
}

// _________________________________________________________________________________________________
bool olu::util::XmlHelper::isXmlEncoded(const std::string &input) {
    const std::regex xmlEntityRegex("&(amp|lt|gt|quot|apos|#xA|#xD|#x9);");
    return std::regex_search(input, xmlEntityRegex);
}

// _________________________________________________________________________________________________
std::string olu::util::XmlHelper::xmlEncode(const std::string &input) {
    std::stringstream ss;

    for (const auto c : input) {
        switch (c) {
            case '&':  ss << "&amp;";  break;
            case '\"': ss << "&quot;"; break;
            case '\'': ss << "&apos;"; break;
            case '<':  ss << "&lt;";   break;
            case '>':  ss << "&gt;";   break;
            case '\n': ss << "&#xA;";  break;
            case '\r': ss << "&#xD;";  break;
            case '\t': ss << "&#x9;";  break;
            default:   ss << c;    break;
        }
    }

    return ss.str();
}

// _________________________________________________________________________________________________
std::string olu::util::XmlHelper::xmlDecode(const std::string &input) {
    std::string output;
    std::string::size_type pos = 0, prev_pos = 0;

    while ((pos = input.find('&', prev_pos)) != std::string::npos) {
        output.append(input, prev_pos, pos - prev_pos);

        if (input.compare(pos, 5, "&amp;") == 0) {
            output.push_back('&');
            pos += 5;
        } else if (input.compare(pos, 4, "&lt;") == 0) {
            output.push_back('<');
            pos += 4;
        } else if (input.compare(pos, 4, "&gt;") == 0) {
            output.push_back('>');
            pos += 4;
        } else if (input.compare(pos, 6, "&quot;") == 0) {
            output.push_back('\\');
            output.push_back('\"');
            pos += 6;
        } else if (input.compare(pos, 6, "&apos;") == 0) {
            output.push_back('\\');
            output.push_back('\'');
            pos += 6;
        } else if (input.compare(pos, 5, "&#xA;") == 0) {
            output.push_back('\n');
            pos += 5;
        } else if (input.compare(pos, 5, "&#xD;") == 0) {
            output.push_back('\r');
            pos += 5;
        } else if (input.compare(pos, 5, "&#x9;") == 0) {
            output.push_back('\t');
            pos += 5;
        } else {
            output.push_back('&');
            pos++;
        }

        prev_pos = pos;
    }

    output.append(input, prev_pos, std::string::npos);
    return output;
}

// _________________________________________________________________________________________________
std::string olu::util::XmlHelper::parseKeyName(const std::string& uri) {
    // Remove angle brackets if present
    std::string clean = uri;
    if (!clean.empty() && clean.front() == '<' && clean.back() == '>') {
        clean = clean.substr(1, clean.size() - 2);
    }

    // Find the last colon (Key:source:population)
    if (const size_t pos = clean.rfind("Key:"); pos != std::string::npos) {
        return clean.substr(pos + 4);
    }

    const std::string msg = "Cannot parse key name from URI: " + uri;
    throw XmlHelperException(msg.c_str());
}