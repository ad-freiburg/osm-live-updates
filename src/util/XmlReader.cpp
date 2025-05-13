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

#include "util/XmlReader.h"
#include "config/Constants.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/regex.hpp>
#include <string>
#include <iostream>

namespace pt = boost::property_tree;
namespace cnst = olu::config::constants;

// _________________________________________________________________________________________________
void olu::util::XmlReader::populatePTreeFromString(const std::string &xml, pt::ptree &tree) {
    std::stringstream ss;
    ss << xml;

    try {
        read_xml(ss, tree, pt::xml_parser::trim_whitespace);
    } catch(std::exception &e) {
        std::cout << e.what() << std::endl;
        std::string msg = "Exception while trying to read the xml: " + xml;
        throw XmlReaderException(msg.c_str());
    }
}

// _________________________________________________________________________________________________
void olu::util::XmlReader::populatePTreeFromFile(const std::string &pathToFile,
                                                 boost::property_tree::ptree &tree) {
    std::ifstream ifs (pathToFile);
    const std::string fileContent( (std::istreambuf_iterator<char>(ifs) ),
                                (std::istreambuf_iterator<char>()) );

    populatePTreeFromString(fileContent, tree);
}

// _________________________________________________________________________________________________
std::string olu::util::XmlReader::readTree(const pt::ptree &tree,
                                           const pt::ptree::key_type& key,
                                           const int& indent) {
    std::ostringstream oss;

    try {
        write_xml_element(oss,
                          key,
                          tree,
                          indent,
                          pt::xml_parser::xml_writer_settings<pt::ptree::key_type>{});
    } catch(std::exception &e) {
        std::cout << e.what() << std::endl;
        std::string msg = "Exception while trying to write tree as string";
        throw XmlReaderException(msg.c_str());
    }

    return oss.str();
}

// _________________________________________________________________________________________________
std::vector<std::string> olu::util::XmlReader::readTagOfChildren(
        const std::string &parentPath,
        const boost::property_tree::ptree &tree,
        const bool excludeXmlAttr) {

    std::vector<std::string> childrenNames;
    for (const auto &child : tree.get_child(parentPath)) {
        if (excludeXmlAttr && child.first == olu::config::constants::XML_TAG_ATTR) {
            continue;
        }

        childrenNames.push_back(child.first);
    }

    return childrenNames;
}

// _________________________________________________________________________________________________
void olu::util::XmlReader::sanitizeXmlTags(pt::ptree &tree) {
    for (auto &tag : tree.get_child("")) {
        if (tag.first == cnst::XML_TAG_TAG) {
            auto value = tag.second.get<std::string>(cnst::XML_PATH_ATTR_VALUE);
            if (isXmlEncoded(value)) {
                value = xmlEncode(value);
            }
            tag.second.put<std::string>(cnst::XML_PATH_ATTR_VALUE, value);
        }
    }
}

bool olu::util::XmlReader::isXmlEncoded(const std::string &input) {
    const boost::regex xmlEntityRegex("&(amp|lt|gt|quot|apos|#xA|#xD|#x9);");
    return regex_search(input, xmlEntityRegex);
}


// _________________________________________________________________________________________________
std::string olu::util::XmlReader::xmlEncode(const std::string &input) {
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
std::string olu::util::XmlReader::xmlDecode(const std::string &input) {
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