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

}

#endif  // OLU_UTIL_TIME_H
