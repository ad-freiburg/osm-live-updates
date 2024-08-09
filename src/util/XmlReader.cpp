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

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <string>

namespace pt = boost::property_tree;

// _________________________________________________________________________________________________
void olu::util::XmlReader::populatePTreeFromString(const std::string &xml, pt::ptree &tree) {
    std::stringstream ss;
    ss << xml;
    pt::xml_parser::read_xml(ss, tree, pt::xml_parser::trim_whitespace);
}

// _________________________________________________________________________________________________
std::string olu::util::XmlReader::writeXmlElementToString(const pt::ptree &element) {
    std::ostringstream oss;
    write_xml_element(oss,
                      {},
                      element,
                      -1,
                      pt::xml_parser::xml_writer_settings<pt::ptree::key_type>{});
    return oss.str();
}

// _________________________________________________________________________________________________
std::string olu::util::XmlReader::readNodeElement(const std::string& xml) {
    pt::ptree tree;
    populatePTreeFromString(xml, tree);

    auto nodeElement = tree.get_child("osm");
    auto nodeElementAsString = writeXmlElementToString(nodeElement);

    tree.clear();
    nodeElement.clear();

    return nodeElementAsString;
}

//    std::cout << nodeElement.get_value<std::string>() << std::endl;
//    tree.get_value<std::string>()
//    BOOST_FOREACH(pt::ptree::value_type &v, nodeElement) {
//        std::cout << v.second.get_value<std::string>() << std::endl;
//    }