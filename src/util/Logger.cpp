// Copyright 2025, University of Freiburg
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

#include "util/Logger.h"

#include <fstream>
#include <iostream>
#include <utility>
#include <util/Time.h>

#include "config/Constants.h"

namespace cnst = olu::config::constants;

// _________________________________________________________________________________________________
void olu::util::Logger::log(const LogEvent &eventType, const std::string_view &description) {
    auto message = formatLogMessage(eventType, description);
    
    auto& outStream = (eventType == LogEvent::ERROR) ? std::cerr : std::cout;
    outStream << message;

    std::ofstream outputFile;
    outputFile.open(cnst::PATH_TO_LOG_FILE, std::ios::app);
    outputFile << message;
    outputFile.close();
}

// _________________________________________________________________________________________________
void olu::util::Logger::logWithOutFormatting(const std::string_view &description) {
    std::cout.imbue(commaLocale);
    std::cout << description;

    std::ofstream outputFile;
    outputFile.open(cnst::PATH_TO_LOG_FILE, std::ios::app);
    outputFile << description;
    outputFile.close();
}

// _________________________________________________________________________________________________
olu::util::LogStream olu::util::Logger::stream() {
    return LogStream();
}

// _________________________________________________________________________________________________
std::string olu::util::Logger::formatLogMessage(const LogEvent &eventType,
                                                const std::string_view &description) {
    std::ostringstream oss;
    oss.imbue(commaLocale);
    oss << std::setprecision(3);

    oss << currentTimeFormatted()
        << "- "
        << LOG_TYPE_DESC[std::to_underlying(eventType)]
        << ": "
        << description
        << std::endl;

    return oss.str();
}
