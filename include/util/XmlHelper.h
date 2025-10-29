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

#ifndef OSM_LIVE_UPDATES_XMLHELPER_H
#define OSM_LIVE_UPDATES_XMLHELPER_H

#include <iostream>
#include <string>

#include "osm/RelationMember.h"

#include "Types.h"

namespace olu::util {
    /**
     * Exception that can appear inside the `XmlHelper` class.
     */
    class XmlHelperException final : public std::exception {
        std::string message;

    public:
        explicit XmlHelperException(const char* msg) : message(msg) { }

        [[nodiscard]] const char* what() const noexcept override {
            return message.c_str();
        }
    };

    /**
     * Helper class for dealing with XML Files.
     */
    class XmlHelper {
    public:
        /**
         * Creates a dummy XML node with the given id and location.
         * The XML element will look like:
         * <node id="123456789" lat="42.7957187" lon="13.5690032"/>
         *
         * @param nodeId The id of the node.
         * @param lonLat The longitude and latitude of the node.
         * @return A string containing the XML representation of the node.
         */
        static std::string getNodeDummy(const id_t &nodeId, const lon_lat_t &lonLat);

        /**
         * Creates a dummy XML way with the given id and member ids.
         * The XML element will look like:
         * <way id="123456789">
         *   <nd ref="123456789"/>
         *   <nd ref="987654321"/>
         * </way>
         *
         * @param wayId The id of the way.
         * @param memberIds The ids of the nodes that are part of the way.
         * @param hasTag
         * @return A string containing the XML representation of the way.
         */
        static std::string getWayDummy(const id_t &wayId, const member_ids_t &memberIds, const bool &hasTag);

        /**
         * Creates a dummy XML relation with the given id, type and members.
         * The XML element will look like:
         * <relation id="123456789">
         *   <member type="node" ref="123456789" role="outer"/>
         *   <member type="way" ref="987654321" role=""/>
         *   <tag k="type" v="relation_type"/>
         * </relation>
         *
         * @param relationId The id of the relation.
         * @param relationType The type of the relation.
         * @param members The members of the relation.
         * @return A string containing the XML representation of the relation.
         */
        static std::string getRelationDummy(const id_t &relationId,
                                            const std::string_view &relationType,
                                            const osm::relation_members_t &members);

        /**
         * @return True, if the given string has an XML encoded character in it
         */
        static bool isXmlEncoded(const std::string &input);

        /**
         * Encodes string for XML format.
         */
        static std::string xmlEncode(const std::string &input);

        /**
         * Decodes string for XML format.
         */
        static std::string xmlDecode(const std::string &input);

        /**
         * Parses a given string in the form of "<http://www.openstreetmap.org/wiki/Key:keyname>"
         * and returns the key name part.
         *
         * @param uri The input string to parse.
         * @return The key name part of the URI.
         */
        static std::string parseKeyName(const std::string& uri);

        /**
         * Parses a given string in rdf syntax (Example:
         * "\"2023-12-30T17:07:40\"^^<http://www.w3.org/2001/XMLSchema#dateTime>") or a string
         * in quotation marks (Example: "\"2023-12-30T17:07:40\"") and return the content inside
         * the quotation marks.
         *
         * @param input The input string to parse.
         * @return
         */
         template <typename T> static T parseRdfString(const std::string &input) {
             const auto end = input.find("^^");
             std::string output;
             if (end == std::string::npos) {
                 // Input has no rdf syntax, just return the content inside the quotation marks
                 if (input.front() != '"' || input.back() != '"') {
                     const std::string msg = "Cannot parse string: " + input;
                     throw XmlHelperException(msg.c_str());
                 }
                 output = input.substr(1, input.size() - 2);
             } else {
                 output = input.substr(1, end - 2);
             }

             if constexpr (std::is_same_v<T, std::string>) {
                 return output;
             } else if constexpr (std::is_same_v<T, int>) {
                 try {
                     return std::stoi(output);
                 } catch (const std::exception &e) {
                     std::cerr << e.what() << std::endl;
                     const std::string msg = "Cannot parse integer from rdf string: " + output;
                     throw XmlHelperException(msg.c_str());
                 }
             } else if constexpr (std::is_same_v<T, float>) {
                 try {
                     return std::stof(output);
                 } catch (const std::exception &e) {
                     std::cerr << e.what() << std::endl;
                     const std::string msg = "Cannot parse float from rdf string: " + output;
                     throw XmlHelperException(msg.c_str());
                 }
             } else if constexpr (std::is_same_v<T, double>) {
                 try {
                     return std::stod(output);
                 } catch (const std::exception &e) {
                     std::cerr << e.what() << std::endl;
                     const std::string msg = "Cannot parse double from rdf string: " + output;
                     throw XmlHelperException(msg.c_str());
                 }
             }

             throw XmlHelperException("Unsupported return type for rdf string.");
         }
    };

} // namespace  olu::util

#endif //OSM_LIVE_UPDATES_XMLHELPER_H
