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

#include <string>

namespace olu::util {

    /**
     * Helper class for dealing with xml Files. The `ptree` data class from the
     * `boost::property_tree` library is used to store xml elements.
     */
    class XmlHelper {
    public:
        /**
         * @return True, if the given string has a xml encoded character in it
         */
        static bool isXmlEncoded(const std::string &input);

        /**
         * Encodes string for xml format.
         */
        static std::string xmlEncode(const std::string &input);

        /**
         * Decodes string for xml format.
         */
        static std::string xmlDecode(const std::string &input);
    };

} // namespace  olu::util

#endif //OSM_LIVE_UPDATES_XMLREADER_H
