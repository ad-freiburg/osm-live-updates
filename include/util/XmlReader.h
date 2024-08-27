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

#ifndef OSM_LIVE_UPDATES_XMLREADER_H
#define OSM_LIVE_UPDATES_XMLREADER_H

#include <boost/property_tree/ptree.hpp>
#include <vector>

namespace pt = boost::property_tree;
namespace olu::util {

    class XmlReader {
    public:
        static std::string readAttribute(const std::string& attributePath, const pt::ptree& tree);
        static std::vector<std::string> readTagOfChildren(const std::string& parentPath,
                                                          const boost::property_tree::ptree &tree,
                                                          const bool excludeXmlAttr = true);
        static std::string readNodeElement(const std::string& xml);
        static void populatePTreeFromString(const std::string& xml, pt::ptree& tree);
        static std::string readTree(const pt::ptree& element,
                                    const pt::ptree::key_type& key = {},
                                    const int& indent = -1);
    };

} // namespace  olu::util

#endif //OSM_LIVE_UPDATES_XMLREADER_H
