// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>
//          Patrick Brosi <brosi@cs.uni-freiburg.de>.

// This file is part of olu.
//
// olu is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// olu is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with olu.  If not, see <https://www.gnu.org/licenses/>.

#include "config/Config.h"
#include "config/Constants.h"
#include "config/ExitCode.h"

#include <filesystem>
#include <string>
#include <popl.hpp>

// ____________________________________________________________________________
std::filesystem::path olu::config::Config::getTempPath(
    const std::string& path, const std::string& suffix) const {
  std::filesystem::path resultPath{cache};
  resultPath /= path + "-" + suffix;
  return std::filesystem::absolute(resultPath);
}

// ____________________________________________________________________________
void olu::config::Config::fromArgs(int argc, char **argv) {
    popl::OptionParser parser("Allowed options");

    auto helpOp = parser.add<popl::Switch>(
            olu::config::constants::HELP_OPTION_SHORT,
            olu::config::constants::HELP_OPTION_LONG,
            olu::config::constants::HELP_OPTION_HELP);

    auto sparqlEndpointUriOp = parser.add<popl::Value<std::string>, popl::Attribute::required>(
            olu::config::constants::SPARQL_ENDPOINT_URI_OPTION_SHORT,
            olu::config::constants::SPARQL_ENDPOINT_URI_OPTION_LONG,
            olu::config::constants::SPARQL_ENDPOINT_URI_OPTION_HELP);

    auto pathToOsmChangeFileOp = parser.add<popl::Value<std::string>, popl::Attribute::optional>(
            olu::config::constants::PATH_TO_OSM_CHANGE_FILE_OPTION_SHORT,
            olu::config::constants::PATH_TO_OSM_CHANGE_FILE_OPTION_LONG,
            olu::config::constants::PATH_TO_OSM_CHANGE_FILE_OPTION_HELP);

    auto osmChangeFileDirectoryUriOp = parser.add<popl::Value<std::string>, popl::Attribute::optional>(
            olu::config::constants::OSM_CHANGE_FILE_DIRECTORY_URI_OPTION_SHORT,
            olu::config::constants::OSM_CHANGE_FILE_DIRECTORY_URI_OPTION_LONG,
            olu::config::constants::OSM_CHANGE_FILE_DIRECTORY_URI_OPTION_HELP);

    auto timestampOp = parser.add<popl::Value<std::string>, popl::Attribute::optional>(
            olu::config::constants::TIME_STAMP_OPTION_SHORT,
            olu::config::constants::TIME_STAMP_OPTION_LONG,
            olu::config::constants::TIME_STAMP_OPTION_HELP);

    auto sequenceNumberOp = parser.add<popl::Value<int>, popl::Attribute::optional>(
            olu::config::constants::SEQUENCE_NUMBER_OPTION_SHORT,
            olu::config::constants::SEQUENCE_NUMBER_OPTION_LONG,
            olu::config::constants::SEQUENCE_NUMBER_OPTION_HELP);

    try {
        parser.parse(argc, argv);

        if (helpOp->count() > 0) {
            if (helpOp->count() == 1) {
                std::cerr << parser << "\n";
            } else if (helpOp->count() == 2) {
                std::cerr << parser.help(popl::Attribute::advanced) << "\n";
            } else {
                std::cerr << parser.help(popl::Attribute::expert) << "\n";
            }
            exit(olu::config::ExitCode::SUCCESS);
        }

        if (!parser.unknown_options().empty()) {
            std::cerr << "Unknown argument(s) specified:\n";
            for (const auto& option : parser.unknown_options()) {
                std::cerr << option << "\n";
            }
            std::cerr << "\n" << parser.help() << "\n";
            exit(olu::config::ExitCode::UNKNOWN_ARGUMENT);
        }

        if (!(pathToOsmChangeFileOp->is_set()) && !(osmChangeFileDirectoryUriOp->is_set())) {
            std::cerr << "You have to provide the path to an osm change file or the URI to an directory where the osm change files are located" << std::endl;
            exit(olu::config::ExitCode::ARGUMENT_MISSING);
        }

        sparqlEndpointUri = sparqlEndpointUriOp->value();

        if (pathToOsmChangeFileOp->is_set()) {
            pathToOsmChangeFile = pathToOsmChangeFileOp->value();
        }

        if (osmChangeFileDirectoryUriOp->is_set()) {
            osmChangeFileDirectoryUri = osmChangeFileDirectoryUriOp->value();
        }

        if (timestampOp->is_set()) {
            timestamp = timestampOp->value();
        }

        if (sequenceNumberOp->is_set()) {
            sequenceNumber = sequenceNumberOp->value();
        }
    } catch (const popl::invalid_option& e) {
        std::cerr << "Invalid Option Exception: " << e.what() << "\n";
        std::cerr << "error:  ";
        if (e.error() == popl::invalid_option::Error::missing_argument) {
            std::cerr << "missing_argument\n";
        } else if (e.error() == popl::invalid_option::Error::invalid_argument) {
            std::cerr << "invalid_argument\n";
        } else if (e.error() == popl::invalid_option::Error::too_many_arguments) {
            std::cerr << "too_many_arguments\n";
        } else if (e.error() == popl::invalid_option::Error::missing_option) {
            std::cerr << "missing_option\n";
        }

        if (e.error() == popl::invalid_option::Error::missing_option) {
            std::string option_name(
                    e.option()->name(popl::OptionName::short_name, true));
            if (option_name.empty()) {
                option_name = e.option()->name(popl::OptionName::long_name, true);
            }
            std::cerr << "option: " << option_name << "\n";
        } else {
            std::cerr << "option: " << e.option()->name(e.what_name()) << "\n";
            std::cerr << "value:  " << e.value() << "\n";
        }

        exit(olu::config::ExitCode::FAILURE);
    }
}

std::string olu::config::Config::getInfo(std::string_view prefix) const {
    std::ostringstream oss;

    oss << prefix << olu::config::constants::HEADER << std::endl;
    oss << prefix << olu::config::constants::SPARQL_ENDPOINT_URI_INFO << " " << sparqlEndpointUri << std::endl;

    if (!pathToOsmChangeFile.empty()) {
        oss << prefix << olu::config::constants::PATH_TO_OSM_CHANGE_FILE_INFO << " " << pathToOsmChangeFile << std::endl;
    } else {
        if (!osmChangeFileDirectoryUri.empty()) {
            oss << prefix << olu::config::constants::OSM_CHANGE_FILE_DIRECTORY_URI_INFO << " " << osmChangeFileDirectoryUri << std::endl;
        }

        if (sequenceNumber > 0) {
            oss << prefix << olu::config::constants::SEQUENCE_NUMBER_INFO << " " << sequenceNumber << std::endl;
        } else if (!timestamp.empty()) {
            oss << prefix << olu::config::constants::TIME_STAMP_INFO << " " << timestamp << std::endl;
        } else {
            oss << prefix << "As you have not entered a sequence number or timestamp, the programme will determine the starting point for you." << timestamp << std::endl;
        }
    }

    return oss.str();
}


