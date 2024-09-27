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
#include <string>
#include <iostream>

namespace pt = boost::property_tree;

// _________________________________________________________________________________________________
void olu::util::XmlReader::populatePTreeFromString(const std::string &xml, pt::ptree &tree) {
    std::stringstream ss;
    ss << xml;

    try {
        pt::xml_parser::read_xml(ss, tree, pt::xml_parser::trim_whitespace);
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
    std::string fileContent( (std::istreambuf_iterator<char>(ifs) ),
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
std::string olu::util::XmlReader::extractNodeElement(const std::string& osmElement) {
    pt::ptree tree;
    populatePTreeFromString(osmElement, tree);

    boost::property_tree::ptree nodeElement;
    try {
        nodeElement = tree.get_child(olu::config::constants::OSM_TAG);
    } catch (std::exception &e) {
        std::cerr << "Path not found: " << e.what() << std::endl;
        std::cerr << "In tree: " << readTree(tree, {}, 0) << std::endl;
        std::cerr << "Tree from xml: " << osmElement << std::endl;
        std::string msg = "Exception while trying to extract node element from: " + osmElement;
        throw XmlReaderException(msg.c_str());
    }

    auto nodeElementAsString = readTree(nodeElement);

    tree.clear();
    nodeElement.clear();

    return nodeElementAsString;
}

    // _____________________________________________________________________________________________
    std::string
    olu::util::XmlReader::readAttribute(const std::string& attributePath,
                                        const boost::property_tree::ptree& tree) {
        std::string attributeValue;
        try {
            attributeValue = tree.get<std::string>(attributePath);
        } catch (boost::property_tree::ptree_bad_path &e) {
            std::cerr << "Path not found: " << e.what() << std::endl;
            std::string msg = "Exeption while trying to read attribute at path " + attributePath;
            throw XmlReaderException(msg.c_str());
        } catch (boost::property_tree::ptree_bad_data &e) {
            std::cerr << "Bad data: " << e.what() << std::endl;
            std::string msg = "Exeption while trying to read attribute at path " + attributePath;
            throw XmlReaderException(msg.c_str());
        } catch (boost::property_tree::ptree_error &e) {
            std::cerr << "Property tree error: " << e.what() << std::endl;
            std::string msg = "Exeption while trying to read attribute at path " + attributePath;
            throw XmlReaderException(msg.c_str());
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            std::string msg = "Exeption while trying to read attribute at path " + attributePath;
            throw XmlReaderException(msg.c_str());
        }

        return attributeValue;
    }

// _________________________________________________________________________________________________
std::vector<std::string> olu::util::XmlReader::readTagOfChildren(
        const std::string &parentPath,
        const boost::property_tree::ptree &tree,
        const bool excludeXmlAttr) {

    std::vector<std::string> childrenNames;
    for (const auto &child : tree.get_child(parentPath)) {
        if (excludeXmlAttr && child.first == olu::config::constants::XML_ATTRIBUTE_TAG) {
            continue;
        }

        childrenNames.push_back(child.first);
    }

    return childrenNames;
}
