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
