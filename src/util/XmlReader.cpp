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

#include <string>

#include "boost/regex.hpp"

#include "util/XmlReader.h"
#include "config/Constants.h"

namespace cnst = olu::config::constants;

// _________________________________________________________________________________________________
bool olu::util::XmlHelper::isXmlEncoded(const std::string &input) {
    const boost::regex xmlEntityRegex("&(amp|lt|gt|quot|apos|#xA|#xD|#x9);");
    return regex_search(input, xmlEntityRegex);
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