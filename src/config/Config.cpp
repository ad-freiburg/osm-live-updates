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
#include "osm2rdf/util/Time.h"

#include <filesystem>
#include <string>
#include <popl.hpp>
#include <util/HttpRequest.h>
#include <util/URLHelper.h>

// ____________________________________________________________________________
void olu::config::Config::fromArgs(int argc, char **argv) {
    popl::OptionParser parser("Allowed options");

    auto helpOp = parser.add<popl::Switch>(
            olu::config::constants::HELP_OPTION_SHORT,
            olu::config::constants::HELP_OPTION_LONG,
            olu::config::constants::HELP_OPTION_HELP);

    auto sparqlGraphUriOp = parser.add<popl::Value<std::string>, popl::Attribute::optional>(
            olu::config::constants::SPARQL_GRAPH_URI_OPTION_SHORT,
            olu::config::constants::SPARQL_GRAPH_URI_OPTION_LONG,
            olu::config::constants::SPARQL_GRAPH_URI_OPTION_HELP);

    auto sparqlAccessTokenOp = parser.add<popl::Value<std::string>, popl::Attribute::optional>(
            olu::config::constants::SPARQL_ACCESS_TOKEN_OPTION_SHORT,
            olu::config::constants::SPARQL_ACCESS_TOKEN_OPTION_LONG,
            olu::config::constants::SPARQL_ACCESS_TOKEN_OPTION_HELP);

    auto sparqlUpdateUri = parser.add<popl::Value<std::string>, popl::Attribute::optional>(
            olu::config::constants::SPARQL_UPDATE_PATH_OPTION_SHORT,
            olu::config::constants::SPARQL_UPDATE_PATH_OPTION_LONG,
            olu::config::constants::SPARQL_UPDATE_PATH_OPTION_HELP);

    auto pathToOsmChangeFileInputDirOp = parser.add<popl::Value<std::string>, popl::Attribute::optional>(
            olu::config::constants::PATH_TO_INPUT_OPTION_SHORT,
            olu::config::constants::PATH_TO_INPUT_OPTION_LONG,
            olu::config::constants::PATH_TO_INPUT_OPTION_HELP);

    auto osmChangeFileServerUriOp = parser.add<popl::Value<std::string>, popl::Attribute::optional>(
            olu::config::constants::OSM_CHANGE_FILE_SERVER_URI_OPTION_SHORT,
            olu::config::constants::OSM_CHANGE_FILE_SERVER_URI_OPTION_LONG,
            olu::config::constants::OSM_CHANGE_FILE_SERVER_URI_OPTION_HELP);

    auto sparqlOutputOp = parser.add<popl::Value<std::string>, popl::Attribute::optional>(
            olu::config::constants::SPARQL_OUTPUT_OPTION_SHORT,
            olu::config::constants::SPARQL_OUTPUT_OPTION_LONG,
            olu::config::constants::SPARQL_OUTPUT_OPTION_HELP);

    auto sparqlOutputFormatOp = parser.add<popl::Switch, popl::Attribute::optional>(
            olu::config::constants::SPARQL_OUTPUT_FORMAT_OPTION_SHORT,
            olu::config::constants::SPARQL_OUTPUT_FORMAT_OPTION_LONG,
            olu::config::constants::SPARQL_OUTPUT_FORMAT_OPTION_HELP);

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

        // Handle sparql endpoint uri
        if (parser.non_option_args().size() != 1) {
            std::cerr << "No SPARQL endpoint URI specified!\n" << parser.help() << "\n";
            exit(config::ExitCode::ENDPOINT_URI_MISSING);
        }
        sparqlEndpointUri = parser.non_option_args()[0];
        if (!olu::util::URLHelper::isValidUri(sparqlEndpointUri)) {
            std::cerr << "SPARQL endpoint URI is not valid: " << sparqlEndpointUri << "\n"
                      << parser.help() << "\n";
            exit(config::ExitCode::ENDPOINT_URI_INVALID);
        }

        if (pathToOsmChangeFileInputDirOp->is_set() == osmChangeFileServerUriOp->is_set()) {
            std::cerr << "You have to EITHER provide the path to an directory with the change files "
                         "you want to process (--input) or the URI to an server where the osm change "
                         "files are located (--file-server)" << std::endl;
            exit(olu::config::ExitCode::INCORRECT_ARGUMENTS);
        }

        if (pathToOsmChangeFileInputDirOp->is_set()) {
            changeFileDir = pathToOsmChangeFileInputDirOp->value();
            if (!std::filesystem::exists(changeFileDir)) {
                std::cerr << "Input does not exist: " << changeFileDir << "\n"
                          << parser.help() << "\n";
                exit(config::ExitCode::INPUT_NOT_EXISTS);
            }
            if (!std::filesystem::is_directory(changeFileDir)) {
                std::cerr << "Input is not a directory: " << changeFileDir << "\n"
                          << parser.help() << "\n";
                exit(config::ExitCode::INPUT_IS_NOT_DIRECTORY);
            }
        }

        if (osmChangeFileServerUriOp->is_set()) {
            changeFileDirUri = osmChangeFileServerUriOp->value();
            if (!olu::util::URLHelper::isValidUri(changeFileDirUri)) {
                std::cerr << "URI for OsmChange file server is not valid: " << changeFileDirUri << "\n"
                          << parser.help() << "\n";
                exit(config::ExitCode::ENDPOINT_URI_INVALID);
            }
        }

        if (sparqlGraphUriOp->is_set()) {
            graphUri = sparqlGraphUriOp->value();
            if (!olu::util::URLHelper::isValidUri(graphUri)) {
                std::cerr << "URI for SPARQL graph is not valid: " << graphUri << "\n"
                          << parser.help() << "\n";
                exit(config::ExitCode::GRAPH_URI_INVALID);
            }
        }

        if (sparqlAccessTokenOp->is_set()) {
            accessToken = sparqlAccessTokenOp->value();
        }

        if (sparqlUpdateUri->is_set()) {
            sparqlEndpointUriForUpdates = sparqlUpdateUri->value();
            if (!olu::util::URLHelper::isValidUri(sparqlEndpointUriForUpdates)) {
                std::cerr << "URI for SPARQL updates is not valid: "
                    << sparqlEndpointUriForUpdates << "\n"
                    << parser.help() << "\n";
                exit(config::ExitCode::ENDPOINT_UPDATE_URI_INVALID);
            }
        } else {
            sparqlEndpointUriForUpdates = sparqlEndpointUri;
        }

        if (timestampOp->is_set()) {
            timestamp = timestampOp->value();
        }

        if (sequenceNumberOp->is_set()) {
            sequenceNumber = sequenceNumberOp->value();
        }

        if (sparqlOutputOp->is_set()) {
            sparqlOutputFile = sparqlOutputOp->value();
            sparqlOutput = sparqlOutputFormatOp->is_set() ? DEBUG_FILE : FILE;
        } else {
            sparqlOutput = ENDPOINT;
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

    oss
    << prefix
    << osm2rdf::util::currentTimeFormatted()
    << olu::config::constants::HEADER
    << std::endl;

    oss
    << prefix
    << osm2rdf::util::currentTimeFormatted()
    << olu::config::constants::SPARQL_ENDPOINT_URI_INFO
    << " "
    << sparqlEndpointUri
    << std::endl;

    if (!graphUri.empty()) {
        oss
        << prefix
        << osm2rdf::util::currentTimeFormatted()
        << olu::config::constants::SPARQL_GRAPH_URI_INFO
        << " "
        << graphUri
        << std::endl;
    }

    if (!changeFileDir.empty()) {
        oss
        << prefix
        << osm2rdf::util::currentTimeFormatted()
        << olu::config::constants::PATH_TO_INPUT_INFO
        << " "
        << changeFileDir
        << std::endl;
    } else {
        if (!changeFileDirUri.empty()) {
            oss
            << prefix
            << osm2rdf::util::currentTimeFormatted()
            << olu::config::constants::OSM_CHANGE_FILE_DIRECTORY_URI_INFO
            << " "
            << changeFileDirUri
            << std::endl;
        }

        if (sequenceNumber > 0) {
            oss
            << prefix
            << osm2rdf::util::currentTimeFormatted()
            << olu::config::constants::SEQUENCE_NUMBER_INFO
            << " "
            << sequenceNumber
            << std::endl;
        } else if (!timestamp.empty()) {
            oss
            << prefix
            << osm2rdf::util::currentTimeFormatted()
            << olu::config::constants::TIME_STAMP_INFO
            << " "
            << timestamp
            << std::endl;
        }
    }

    if (sparqlOutput != ENDPOINT) {
        oss
        << prefix
        << osm2rdf::util::currentTimeFormatted()
        << olu::config::constants::SPARQL_OUTPUT_INFO
        << " "
        << sparqlOutputFile
        << std::endl;
    }

    return oss.str();
}


