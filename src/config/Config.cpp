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

#include "config/Config.h"

#include <filesystem>
#include <string>

#include "popl.hpp"
#include "osm2rdf/util/Time.h"

#include "config/Constants.h"
#include "config/ExitCode.h"
#include "util/HttpRequest.h"
#include "util/URLHelper.h"

// _________________________________________________________________________________________________
void olu::config::Config::fromArgs(const int argc, char **argv) {
    popl::OptionParser parser("Allowed options");

    const auto helpOp = parser.add<popl::Switch>(
        constants::HELP_OPTION_SHORT,
        constants::HELP_OPTION_LONG,
        constants::HELP_OPTION_HELP);

    const auto sparqlGraphUriOp = parser.add<popl::Value<std::string>,
        popl::Attribute::optional>(
        constants::SPARQL_GRAPH_URI_OPTION_SHORT,
        constants::SPARQL_GRAPH_URI_OPTION_LONG,
        constants::SPARQL_GRAPH_URI_OPTION_HELP);

    const auto sparqlAccessTokenOp = parser.add<popl::Value<std::string>,
        popl::Attribute::optional>(
        constants::SPARQL_ACCESS_TOKEN_OPTION_SHORT,
        constants::SPARQL_ACCESS_TOKEN_OPTION_LONG,
        constants::SPARQL_ACCESS_TOKEN_OPTION_HELP);

    const auto sparqlUpdateUri = parser.add<popl::Value<std::string>,
        popl::Attribute::optional>(
        constants::SPARQL_UPDATE_PATH_OPTION_SHORT,
        constants::SPARQL_UPDATE_PATH_OPTION_LONG,
        constants::SPARQL_UPDATE_PATH_OPTION_HELP);

    const auto pathToOsmChangeFileInputDirOp = parser.add<popl::Value<std::string>,
        popl::Attribute::optional>(
        constants::PATH_TO_INPUT_OPTION_SHORT,
        constants::PATH_TO_INPUT_OPTION_LONG,
        constants::PATH_TO_INPUT_OPTION_HELP);

    const auto osmChangeFileServerUriOp = parser.add<popl::Value<std::string>,
        popl::Attribute::optional>(
        constants::OSM_CHANGE_FILE_SERVER_URI_OPTION_SHORT,
        constants::OSM_CHANGE_FILE_SERVER_URI_OPTION_LONG,
        constants::OSM_CHANGE_FILE_SERVER_URI_OPTION_HELP);

    const auto sparqlOutputOp = parser.add<popl::Value<std::string>,
        popl::Attribute::optional>(
        constants::SPARQL_OUTPUT_OPTION_SHORT,
        constants::SPARQL_OUTPUT_OPTION_LONG,
        constants::SPARQL_OUTPUT_OPTION_HELP);

    const auto sparqlOutputFormatOp = parser.add<popl::Switch,
        popl::Attribute::optional>(
        constants::SPARQL_OUTPUT_FORMAT_OPTION_SHORT,
        constants::SPARQL_OUTPUT_FORMAT_OPTION_LONG,
        constants::SPARQL_OUTPUT_FORMAT_OPTION_HELP);

    const auto timestampOp = parser.add<popl::Value<std::string>,
        popl::Attribute::optional>(
        constants::TIME_STAMP_OPTION_SHORT,
        constants::TIME_STAMP_OPTION_LONG,
        constants::TIME_STAMP_OPTION_HELP);

    const auto sequenceNumberOp = parser.add<popl::Value<int>,
        popl::Attribute::optional>(
        constants::SEQUENCE_NUMBER_OPTION_SHORT,
        constants::SEQUENCE_NUMBER_OPTION_LONG,
        constants::SEQUENCE_NUMBER_OPTION_HELP);

    const auto noBlankNodesOp = parser.add<popl::Switch,
        popl::Attribute::advanced>(
        constants::BLANK_NODES_OPTION_SHORT,
        constants::BLANK_NODES_OPTION_LONG,
        constants::BLANK_NODES_OPTION_HELP);

    const auto wktPrecisionOp = parser.add<popl::Value<u_int16_t>,
        popl::Attribute::advanced>(
        constants::WKT_PRECISION_OPTION_SHORT,
        constants::WKT_PRECISION_OPTION_LONG,
        constants::WKT_PRECISION_OPTION_HELP);

    const auto batchSizeOp = parser.add<popl::Value<u_int32_t>,
        popl::Attribute::advanced>(
        constants::BATCH_SIZE_OPTION_SHORT,
        constants::BATCH_SIZE_OPTION_LONG,
        constants::BATCH_SIZE_OPTION_HELP);

    const auto isQleverEndpointOp = parser.add<popl::Switch,
        popl::Attribute::advanced>(
        constants::QLEVER_ENDPOINT_OPTION_SHORT,
        constants::QLEVER_ENDPOINT_OPTION_LONG,
        constants::QLEVER_ENDPOINT_OPTION_HELP);

    const auto showStatisticsOp = parser.add<popl::Switch,
        popl::Attribute::advanced>(
        constants::STATISTICS_OPTION_SHORT,
        constants::STATISTICS_OPTION_LONG,
        constants::STATISTICS_OPTION_HELP);

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
            exit(SUCCESS);
        }

        if (!parser.unknown_options().empty()) {
            std::cerr << "Unknown argument(s) specified:\n";
            for (const auto& option : parser.unknown_options()) {
                std::cerr << option << "\n";
            }
            std::cerr << "\n" << parser.help() << "\n";
            exit(UNKNOWN_ARGUMENT);
        }

        // Handle sparql endpoint uri
        if (parser.non_option_args().size() != 1) {
            std::cerr << "No SPARQL endpoint URI specified!\n" << parser.help() << "\n";
            exit(ENDPOINT_URI_MISSING);
        }
        sparqlEndpointUri = parser.non_option_args()[0];
        if (!util::URLHelper::isValidUri(sparqlEndpointUri)) {
            std::cerr << "SPARQL endpoint URI is not valid: " << sparqlEndpointUri << "\n"
                      << parser.help() << "\n";
            exit(ENDPOINT_URI_INVALID);
        }

        if (pathToOsmChangeFileInputDirOp->is_set() == osmChangeFileServerUriOp->is_set()) {
            std::cerr << "You have to EITHER provide the path to an directory with the change files "
                         "you want to process (--input) or the URI to an server where the osm change "
                         "files are located (--file-server)" << std::endl;
            exit(INCORRECT_ARGUMENTS);
        }

        if (pathToOsmChangeFileInputDirOp->is_set()) {
            changeFileDir = pathToOsmChangeFileInputDirOp->value();
            if (!std::filesystem::exists(changeFileDir)) {
                std::cerr << "Input does not exist: " << changeFileDir << "\n"
                          << parser.help() << "\n";
                exit(INPUT_NOT_EXISTS);
            }
            if (!std::filesystem::is_directory(changeFileDir)) {
                std::cerr << "Input is not a directory: " << changeFileDir << "\n"
                          << parser.help() << "\n";
                exit(INPUT_IS_NOT_DIRECTORY);
            }
        }

        if (osmChangeFileServerUriOp->is_set()) {
            changeFileDirUri = osmChangeFileServerUriOp->value();
            if (!util::URLHelper::isValidUri(changeFileDirUri)) {
                std::cerr << "URI for OsmChange file server is not valid: " << changeFileDirUri << "\n"
                          << parser.help() << "\n";
                exit(ENDPOINT_URI_INVALID);
            }
        }

        if (sparqlGraphUriOp->is_set()) {
            graphUri = sparqlGraphUriOp->value();
            if (!util::URLHelper::isValidUri(graphUri)) {
                std::cerr << "URI for SPARQL graph is not valid: " << graphUri << "\n"
                          << parser.help() << "\n";
                exit(GRAPH_URI_INVALID);
            }
        }

        if (sparqlAccessTokenOp->is_set()) {
            accessToken = sparqlAccessTokenOp->value();
        }

        noBlankNodes = noBlankNodesOp->is_set();

        if (sparqlUpdateUri->is_set()) {
            sparqlEndpointUriForUpdates = sparqlUpdateUri->value();
            if (!util::URLHelper::isValidUri(sparqlEndpointUriForUpdates)) {
                std::cerr << "URI for SPARQL updates is not valid: "
                    << sparqlEndpointUriForUpdates << "\n"
                    << parser.help() << "\n";
                exit(ENDPOINT_UPDATE_URI_INVALID);
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

        if (wktPrecisionOp->is_set()) {
            wktPrecision = wktPrecisionOp->value();
        }

        if (batchSizeOp->is_set()) {
            batchSize = batchSizeOp->value();
        }

        if (isQleverEndpointOp->is_set()) {
            isQLever = true;
        }

        if (showStatisticsOp->is_set()) {
            showDetailedStatistics = true;
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

        exit(FAILURE);
    }
}

std::string olu::config::Config::getInfo(const std::string_view prefix) const {
    std::ostringstream oss;

    oss
    << osm2rdf::util::currentTimeFormatted()
    << prefix
    << constants::HEADER
    << std::endl;

    oss
    << osm2rdf::util::currentTimeFormatted()
    << prefix
    << constants::SPARQL_ENDPOINT_URI_INFO
    << " "
    << sparqlEndpointUri
    << std::endl;

    if (isQLever) {
        oss
        << osm2rdf::util::currentTimeFormatted()
        << prefix
        << constants::QLEVER_ENDPOINT_INFO
        << std::endl;
    }

    if (!graphUri.empty()) {
        oss
        << osm2rdf::util::currentTimeFormatted()
        << prefix
        << constants::SPARQL_GRAPH_URI_INFO
        << " "
        << graphUri
        << std::endl;
    }

    if (!changeFileDir.empty()) {
        oss
        << osm2rdf::util::currentTimeFormatted()
        << prefix
        << constants::PATH_TO_INPUT_INFO
        << " "
        << changeFileDir
        << std::endl;
    } else {
        if (!changeFileDirUri.empty()) {
            oss
            << osm2rdf::util::currentTimeFormatted()
            << prefix
            << constants::OSM_CHANGE_FILE_DIRECTORY_URI_INFO
            << " "
            << changeFileDirUri
            << std::endl;
        }

        if (sequenceNumber > 0) {
            oss
            << osm2rdf::util::currentTimeFormatted()
            << prefix
            << constants::SEQUENCE_NUMBER_INFO
            << " "
            << sequenceNumber
            << std::endl;
        } else if (!timestamp.empty()) {
            oss
            << osm2rdf::util::currentTimeFormatted()
            << prefix
            << constants::TIME_STAMP_INFO
            << " "
            << timestamp
            << std::endl;
        }
    }

    if (noBlankNodes) {
        oss
        << osm2rdf::util::currentTimeFormatted()
        << prefix
        << constants::BLANK_NODES_INFO
        << std::endl;
    }

    if (wktPrecision != DEFAULT_WKT_PRECISION) {
        oss
        << osm2rdf::util::currentTimeFormatted()
        << prefix
        << constants::WKT_PRECISION_INFO
        << " "
        << std::to_string(wktPrecision)
        << std::endl;
    }

    if (batchSize != DEFAULT_BATCH_SIZE) {
        oss
        << osm2rdf::util::currentTimeFormatted()
        << prefix
        << constants::BATCH_SIZE_INFO
        << " "
        << std::to_string(batchSize)
        << std::endl;
    }

    return oss.str();
}


