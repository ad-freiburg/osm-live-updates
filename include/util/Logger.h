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

#ifndef LOGGER_H
#define LOGGER_H

#include <array>
#include <iostream>

namespace olu::util {
    static inline constexpr std::array<std::string_view, 5> LOG_TYPE_DESC =
        {"CONFIG", "DEBUG ", "INFO ", "WARNING", "ERROR"};

    enum class LogEvent {
        CONFIG = 0,
        DEBUG = 1,
        INFO = 2,
        WARNING = 3,
        ERROR = 4
    };

    class Logger {
    public:
        static void log(const LogEvent &eventType, const std::string_view &description,
                        const bool &writeToStdStream = true);
    };

    // _________________________________________________________________________________________________
    // Source: https://github.com/ad-freiburg/qlever/blob/master/src/util/Log.h
    // Helper class to get thousandth separators in a locale
    class CommaNumPunct : public std::numpunct<char> {
    protected:
        virtual char do_thousands_sep() const { return ','; }

        virtual std::string do_grouping() const { return "\03"; }
    };

    const static std::locale commaLocale(std::locale(), new CommaNumPunct());
}

#endif //LOGGER_H
