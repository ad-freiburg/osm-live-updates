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

    /**
     * Helper class for dealing with xml Files. The `ptree` data class from the
     * `boost::property_tree` library is used to store xml elements.
     */
    class XmlReader {
    public:
        /**
         * Reads the attribute at the given path from the given tree.
         *
         * @example For the tree:
         * `\<way id="25569745" version="10" timestamp="2024-09-19T11:53:42Z"\>`
         * and attributePath: `xmlattr.id` the function would return "25569745"
         *
         * @param attributePath The path of the attribute
         * @param tree The property tree
         * @return The attribute at the given path.
         */
        static std::string readAttribute(const std::string& attributePath, const pt::ptree& tree);

        /**
         * Returns the tag of all children of the element at `parentPath`.
         *
         * @param parentPath The path of the element to get the children for
         * @param tree The property tree that contains the parent
         * @param excludeXmlAttr Determines if the xml attribute should be returned as a child
         * @return A vector with the tags of all children of the element at `parentPath`
         */
        static std::vector<std::string>
        readTagOfChildren(const std::string& parentPath,
                          const boost::property_tree::ptree &tree,
                          bool excludeXmlAttr = true);

        /**
         * Populates the given property tree with the contents of the xml file.
         *
         * @param pathToFile The path to the xml file
         * @param tree The property tree that should be populated
         */
        static void populatePTreeFromFile(const std::string& pathToFile, pt::ptree& tree);

        /**
         * Populates the given property tree with the contents of the given string.
         *
         * @param string the string that contains the xml element
         * @param tree The property tree that should be populated
         */
        static void populatePTreeFromString(const std::string& xml, pt::ptree& tree);

        /**
         * @param tree The tree that contains the elements that should be printed
         * @param key The keyType
         * @param indent how deep to traverse into the tree
         * @return The contents of the given tree as string
         */
        static std::string
        readTree(const pt::ptree& tree,
                 const pt::ptree::key_type& key = {},
                 const int& indent = 0);


        /**
         * Sanitizes the tags of a way or relation.
         */
        static void sanitizeXmlTags(pt::ptree &tree);

        /**
         * Sanitizes string for xml format.
         */
        static std::string xmlEncode(const std::string &input);

    };

    /**
 * Exception that can appear inside the `XmlReader` class.
 */
    class XmlReaderException : public std::exception {
    private:
        std::string message;

    public:
        explicit XmlReaderException(const char* msg) : message(msg) { }

        [[nodiscard]] const char* what() const noexcept override {
            return message.c_str();
        }
    };

} // namespace  olu::util

#endif //OSM_LIVE_UPDATES_XMLREADER_H
