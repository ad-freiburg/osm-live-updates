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

#ifndef OLU_UTIL_TIME_H
#define OLU_UTIL_TIME_H

#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

namespace olu::util {
    // Return current time formatted as string.
    // https://github.com/ad-freiburg/osm2rdf/blob/master/include/osm2rdf/util/Time.h
    inline std::string currentTimeFormatted() {
      std::ostringstream oss;
      char tl[20];
      auto n = std::chrono::system_clock::now();
      time_t tt = std::chrono::system_clock::to_time_t(n);
      int m = std::chrono::duration_cast<std::chrono::milliseconds>(
                  n - std::chrono::time_point_cast<std::chrono::seconds>(n))
                  .count();
      struct tm t = *localtime(&tt);
      strftime(tl, 20, "%Y-%m-%d %H:%M:%S", &t);
      oss << "[" << tl << "." << std::setfill('0') << std::setw(3) << m << "] ";
      return oss.str();
    }

    inline std::string currentIsoTime() {
        char timeString[25];
        struct tm t;
        const auto n = std::chrono::system_clock::now();
        const time_t time = std::chrono::system_clock::to_time_t(n);
        strftime(timeString, 25, "%Y-%m-%dT%X", gmtime_r(&time, &t));
        return std::string(timeString);
    }

    inline int secondsBetweenNowAndTimestamp(const std::string& isoTimestamp) {
        std::tm tm = {};
        std::istringstream ss(isoTimestamp);
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
        auto time_c = std::mktime(&tm);

        const auto now = std::chrono::system_clock::now();
        const auto now_c = std::chrono::system_clock::to_time_t(now);

        return std::difftime(now_c, time_c);
    }

    inline int minutesBetweenNowAndTimestamp(const std::string& isoTimestamp) {
        return secondsBetweenNowAndTimestamp(isoTimestamp) / 60;
    }

    inline int hoursBetweenNowAndTimestamp(const std::string& isoTimestamp) {
        return secondsBetweenNowAndTimestamp(isoTimestamp) / (60 * 60);
    }

    inline int daysBetweenNowAndTimestamp(const std::string& isoTimestamp) {
        return secondsBetweenNowAndTimestamp(isoTimestamp) / (60 * 60 * 24);
    }
}

#endif  // OLU_UTIL_TIME_H
