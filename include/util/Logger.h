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
#include <iomanip>
#include <sstream>

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

    class LogStream;

    class Logger {
    public:
        // Prefix for log messages to align them in the console output
        static constexpr std::string_view PREFIX_SPACER = "                          > ";

        /**
         * Formats and writes a log message to the console and to a log file.
         * Example: "[2025-07-13 13:53:19.929] - INFO : Filtering converted triples..."
         *
         * @param eventType The type of log event
         * @param description The description of the log event
         */
        static void log(const LogEvent &eventType, const std::string_view &description);

        /**
         * Writes a log message to the console and to a log file a message without formatting
         * (i.e., no information about the log event and no timestamp).
         *
         * @param description The raw message to log
         */
        static void logWithOutFormatting(const std::string_view &description);

        /**
         * Stream for a log message that will be written to the console and log file without
         * formatting.
         */
        static LogStream stream();

    private:
        /**
         * Formats a log message with timestamp and event type.
         * Example: "[2025-07-13 13:53:19.929] - INFO : Filtering converted triples..."
         *
         * @param eventType The type of log event
         * @param description The description of the log event
         * @return A formatted log message string
         */
        static std::string formatLogMessage(const LogEvent &eventType,
                                            const std::string_view &description);
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

    // LogStream class that provides stream-like operations
    class LogStream {
        std::ostringstream _stream;

    public:
        explicit LogStream() {
            _stream.imbue(commaLocale);
            _stream << std::setprecision(3);
        }

        // Destructor will automatically log the message
        ~LogStream() {
            Logger::logWithOutFormatting(_stream.str());
        }

        // Template for stream insertion
        template<typename T>
        LogStream& operator<<(const T& value) {
            _stream << value;
            return *this;
        }

        // Special handling for manipulators like std::endl
        LogStream& operator<<(std::ostream& (*manip)(std::ostream&)) {
            manip(_stream);
            return *this;
        }
    };
}

#endif //LOGGER_H
