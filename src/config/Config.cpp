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
#include <regex>
#include <string>

#include "popl.hpp"

#include "config/Constants.h"
#include "config/ExitCode.h"
#include "util/HttpRequest.h"
#include "util/URLHelper.h"
#include "util/Logger.h"

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

    const auto replicationServerUriOp = parser.add<popl::Value<std::string>,
        popl::Attribute::optional>(
        constants::REPLICATION_SERVER_URI_OPTION_SHORT,
        constants::REPLICATION_SERVER_URI_OPTION_LONG,
        constants::REPLICATION_SERVER_URI_OPTION_HELP);

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

    const auto sparqlResponseOutputOp = parser.add<popl::Value<std::string>,
        popl::Attribute::optional>(
        constants::SPARQL_RESPONSE_OUTPUT_OPTION_SHORT,
        constants::SPARQL_RESPONSE_OUTPUT_OPTION_LONG,
        constants::SPARQL_RESPONSE_OUTPUT_OPTION_HELP);

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

    const auto maxSequenceNumberOp = parser.add<popl::Value<int>,
        popl::Attribute::optional>(
        constants::MAX_SEQUENCE_NUMBER_OPTION_SHORT,
        constants::MAX_SEQUENCE_NUMBER_OPTION_LONG,
        constants::MAX_SEQUENCE_NUMBER_OPTION_HELP);

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

    const auto bboxOp = parser.add<popl::Value<std::string>,
        popl::Attribute::optional>(
        constants::BBOX_OPTION_SHORT,
        constants::BBOX_OPTION_LONG,
        constants::BBOX_OPTION_HELP);

    const auto polyFileOp = parser.add<popl::Value<std::string>,
    popl::Attribute::optional>(
        constants::POLY_FILE_OPTION_SHORT,
        constants::POLY_FILE_OPTION_LONG,
        constants::POLY_FILE_OPTION_HELP);

    const auto extractStrategyOp = parser.add<popl::Value<std::string>,
    popl::Attribute::optional>(
        constants::EXTRACT_STRATEGY_OPTION_SHORT,
        constants::EXTRACT_STRATEGY_OPTION_LONG,
        constants::EXTRACT_STRATEGY_OPTION_HELP);

    const auto tmpFileDirOp = parser.add<popl::Value<std::string>,
    popl::Attribute::optional>(
        constants::TMP_FILE_DIR_OPTION_SHORT,
        constants::TMP_FILE_DIR_OPTION_LONG,
        constants::TMP_FILE_DIR_OPTION_HELP);

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
            std::stringstream errorDescription;
            errorDescription << "Unknown argument(s) specified:\n";
            for (const auto& option : parser.unknown_options()) {
                errorDescription << option << "\n";
            }
            errorDescription << "\n" << parser.help() << "\n";

            util::Logger::log(util::LogEvent::ERROR, errorDescription.str());
            exit(UNKNOWN_ARGUMENT);
        }

        // Handle sparql endpoint uri
        if (parser.non_option_args().size() != 1) {
            std::stringstream errorDescription;
            errorDescription << "No SPARQL endpoint URI specified!\n" << parser.help() << "\n";
            util::Logger::log(util::LogEvent::ERROR, errorDescription.str());
            exit(ENDPOINT_URI_MISSING);
        }
        sparqlEndpointUri = parser.non_option_args()[0];
        if (!util::URLHelper::isValidUri(sparqlEndpointUri)) {
            std::stringstream errorDescription;
            errorDescription << "SPARQL endpoint URI is not valid: " << sparqlEndpointUri << "\n"
                             << parser.help() << "\n";
            util::Logger::log(util::LogEvent::ERROR, errorDescription.str());

            exit(ENDPOINT_URI_INVALID);
        }

        if (pathToOsmChangeFileInputDirOp->is_set() == replicationServerUriOp->is_set()) {
            std::stringstream errorDescription;
            errorDescription << "You have to EITHER provide the path to an directory with the "
                                "change files you want to process (--input) or the URI to an server"
                                " where the osm change files are located (--file-server)"
                             << std::endl;
            util::Logger::log(util::LogEvent::ERROR, errorDescription.str());
            exit(INCORRECT_ARGUMENTS);
        }

        if (pathToOsmChangeFileInputDirOp->is_set()) {
            changeFileDir = pathToOsmChangeFileInputDirOp->value();
            if (!std::filesystem::exists(changeFileDir)) {
                std::stringstream errorDescription;
                errorDescription << "Input does not exist: " << changeFileDir << std::endl;
                util::Logger::log(util::LogEvent::ERROR, errorDescription.str());
                exit(INPUT_NOT_EXISTS);
            }
            if (!std::filesystem::is_directory(changeFileDir)) {
                std::stringstream errorDescription;
                errorDescription << "Input is not a directory: " << changeFileDir << std::endl;
                util::Logger::log(util::LogEvent::ERROR, errorDescription.str());
                exit(INPUT_IS_NOT_DIRECTORY);
            }
        }

        if (tmpFileDirOp->is_set()) {
            tmpDir = tmpFileDirOp->value();
            if (!std::filesystem::exists(tmpDir)) {
                std::stringstream errorDescription;
                errorDescription << "Directory for temporary files does not exist: " << tmpDir << std::endl;
                util::Logger::log(util::LogEvent::ERROR, errorDescription.str());
                exit(TMP_DIR_NOT_EXISTS);
            }
            if (!std::filesystem::is_directory(tmpDir)) {
                std::stringstream errorDescription;
                errorDescription << "Directory for temporary files is not a directory: " << tmpDir << std::endl;
                util::Logger::log(util::LogEvent::ERROR, errorDescription.str());
                exit(TMP_DIR_IS_NOT_DIRECTORY);
            }
        }

        if (replicationServerUriOp->is_set()) {
            replicationServerUri = replicationServerUriOp->value();
            if (!util::URLHelper::isValidUri(replicationServerUri)) {
                std::stringstream errorDescription;
                errorDescription << "URI for OsmChange file server is not valid: "
                                 << replicationServerUri << std::endl;
                util::Logger::log(util::LogEvent::ERROR, errorDescription.str());
                exit(ENDPOINT_URI_INVALID);
            }
        }

        if (sparqlGraphUriOp->is_set()) {
            graphUri = sparqlGraphUriOp->value();
            if (!util::URLHelper::isValidUri(graphUri)) {
                std::stringstream errorDescription;
                errorDescription << "URI for SPARQL graph is not valid: " << graphUri << std::endl;
                util::Logger::log(util::LogEvent::ERROR, errorDescription.str());
                exit(GRAPH_URI_INVALID);
            }
        }

        if (sparqlAccessTokenOp->is_set()) {
            accessToken = sparqlAccessTokenOp->value();
        }

        if (sparqlUpdateUri->is_set()) {
            sparqlEndpointUriForUpdates = sparqlUpdateUri->value();
            if (!util::URLHelper::isValidUri(sparqlEndpointUriForUpdates)) {
                std::stringstream errorDescription;
                errorDescription << "URI for SPARQL updates is not valid: "
                                 << sparqlEndpointUriForUpdates << std::endl;
                util::Logger::log(util::LogEvent::ERROR, errorDescription.str());
                exit(ENDPOINT_UPDATE_URI_INVALID);
            }
        } else {
            sparqlEndpointUriForUpdates = sparqlEndpointUri;
        }

        if (bboxOp->is_set() && polyFileOp->is_set()) {
            std::stringstream errorDescription;
            errorDescription << "You can EITHER provide a bounding box (--bbox) or a polygon "
                                "file (--polygon-file), but not both at the same time."
                             << std::endl;
            util::Logger::log(util::LogEvent::ERROR, errorDescription.str());
            exit(INCORRECT_ARGUMENTS);
        }

        if ((bboxOp->is_set() || polyFileOp->is_set()) &&
            std::system("osmium >/dev/null 2>&1") != 0) {
            std::stringstream errorDescription;
            errorDescription << "Missing dependency: 'osmium-tool' is required for bounding box "
                                "(--bbox) or polygon file (--polygon-file) support.\n"
                                "Please install 'osmium-tool' and ensure it is available as "
                                "'osmium' in your PATH.";
            util::Logger::log(util::LogEvent::ERROR, errorDescription.str());
            exit(FAILURE);
        }

        if (bboxOp->is_set()) {
            bbox = bboxOp->value();
            // Check if the bounding box is in the correct format "LEFT,BOTTOM,RIGHT,TOP"
            std::regex bboxRegex(R"(^-?\d+(\.\d+)?,-?\d+(\.\d+)?,-?\d+(\.\d+)?,-?\d+(\.\d+)?$)");
            if (!std::regex_match(bbox, bboxRegex)) {
                std::stringstream errorDescription;
                errorDescription << "Bounding box is not valid: " << bbox << std::endl;
                util::Logger::log(util::LogEvent::ERROR, errorDescription.str());
                exit(BBOX_INVALID);
            }
        }

        if (polyFileOp->is_set()) {
            pathToPolygonFile = polyFileOp->value();
            if (!std::filesystem::exists(pathToPolygonFile)) {
                std::stringstream errorDescription;
                errorDescription << "Polygon file does not exist at path: " << pathToPolygonFile
                                 << std::endl;
                util::Logger::log(util::LogEvent::ERROR, errorDescription.str());
                exit(POLYGON_FILE_NOT_EXISTS);
            }
            if (!std::filesystem::is_regular_file(pathToPolygonFile)) {
                std::stringstream errorDescription;
                errorDescription << "Polygon file at : " << pathToPolygonFile
                                 << "is not a regular file." << std::endl;
                util::Logger::log(util::LogEvent::ERROR, errorDescription.str());
                exit(POLYGON_FILE_NOT_EXISTS);
            }
        }

        if (extractStrategyOp->is_set()) {
            extractStrategy = extractStrategyOp->value();
            if (!polyFileOp->is_set() && !bboxOp->is_set()) {
                std::stringstream errorDescription;
                errorDescription << "Specified extract strategy without specifying a bounding box "
                                    "or polygon file: " << extractStrategy
                                 << std::endl;
                util::Logger::log(util::LogEvent::ERROR, errorDescription.str());
                exit(INCORRECT_ARGUMENTS);
            }

            if (extractStrategy != "smart" &&
                extractStrategy != "complete_ways" &&
                extractStrategy != "simple") {
                std::stringstream errorDescription;
                errorDescription << "Invalid extract strategy specified: " << extractStrategy
                                 << ". Valid strategies are 'smart', 'complete_ways', and 'simple'."
                                    " See osmium manual for more information."
                                 << std::endl;
                util::Logger::log(util::LogEvent::ERROR, errorDescription.str());
                exit(INCORRECT_ARGUMENTS);
            }
        }

        if (timestampOp->is_set()) {
            timestamp = timestampOp->value();
        }

        if (sequenceNumberOp->is_set()) {
            sequenceNumber = sequenceNumberOp->value();
        }

        if (maxSequenceNumberOp->is_set()) {
            maxSequenceNumber = maxSequenceNumberOp->value();

            if (maxSequenceNumber < 0) {
                std::stringstream errorDescription;
                errorDescription << "Maximum sequence number must be positiv: "
                                 << maxSequenceNumber << std::endl;
                util::Logger::log(util::LogEvent::ERROR, errorDescription.str());
                exit(INCORRECT_ARGUMENTS);
            }

            if (maxSequenceNumber < sequenceNumber) {
                std::stringstream errorDescription;
                errorDescription << "Maximum sequence number must be larger than the start sequence number: "
                                 << maxSequenceNumber << std::endl;
                util::Logger::log(util::LogEvent::ERROR, errorDescription.str());
                exit(INCORRECT_ARGUMENTS);
            }
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

        if (sparqlResponseOutputOp->is_set()) {
            sparqlResponseFile = sparqlResponseOutputOp->value();
        }
    } catch (const popl::invalid_option& e) {
        std::stringstream errorDescription;
        errorDescription << "Invalid Option Exception: " << e.what() << "\n";
        errorDescription << "error:  ";
        if (e.error() == popl::invalid_option::Error::missing_argument) {
            errorDescription << "missing_argument\n";
        } else if (e.error() == popl::invalid_option::Error::invalid_argument) {
            errorDescription << "invalid_argument\n";
        } else if (e.error() == popl::invalid_option::Error::too_many_arguments) {
            errorDescription << "too_many_arguments\n";
        } else if (e.error() == popl::invalid_option::Error::missing_option) {
            errorDescription << "missing_option\n";
        }

        if (e.error() == popl::invalid_option::Error::missing_option) {
            std::string option_name(
                    e.option()->name(popl::OptionName::short_name, true));
            if (option_name.empty()) {
                option_name = e.option()->name(popl::OptionName::long_name, true);
            }
            errorDescription << "option: " << option_name << "\n";
        } else {
            errorDescription << "option: " << e.option()->name(e.what_name()) << "\n";
            errorDescription << "value:  " << e.value() << "\n";
        }

        util::Logger::log(util::LogEvent::ERROR, errorDescription.str());
        exit(FAILURE);
    }
}

void olu::config::Config::printInfo() const {
    util::Logger::log(util::LogEvent::CONFIG,
                      constants::SPARQL_ENDPOINT_URI_INFO + " " + sparqlEndpointUri);

    if (isQLever) {
        util::Logger::log(util::LogEvent::CONFIG, constants::QLEVER_ENDPOINT_INFO);
    }

    if (!graphUri.empty()) {
        util::Logger::log(util::LogEvent::CONFIG,
                          constants::SPARQL_GRAPH_URI_INFO + " " + graphUri);
    }

    if (!changeFileDir.empty()) {
        util::Logger::log(util::LogEvent::CONFIG,
                          constants::PATH_TO_INPUT_INFO + " " + changeFileDir);
    } else {
        if (!replicationServerUri.empty()) {
            util::Logger::log(util::LogEvent::CONFIG,
                              constants::REPLICATION_SERVER_URI_INFO + " " +
                              replicationServerUri);
        }

        if (sequenceNumber > 0) {
            util::Logger::log(util::LogEvent::CONFIG,
                              constants::SEQUENCE_NUMBER_INFO + " " + std::to_string(
                                  sequenceNumber));
        } else if (!timestamp.empty()) {
            util::Logger::log(util::LogEvent::CONFIG,
                              constants::TIME_STAMP_INFO + " " + timestamp);
        }
    }

    if (!bbox.empty()) {
        util::Logger::log(util::LogEvent::CONFIG,
                  constants::BBOX_INFO + " " + bbox);
    }

    if (!pathToPolygonFile.empty()) {
        util::Logger::log(util::LogEvent::CONFIG,
                  constants::POLY_FILE_INFO + " " + pathToPolygonFile);
    }

    if (!extractStrategy.empty()) {
        util::Logger::log(util::LogEvent::CONFIG,
                  constants::EXTRACT_STRATEGY_INFO + " " + extractStrategy);
    }

    if (!sparqlResponseFile.empty()) {
        util::Logger::log(util::LogEvent::CONFIG,
                          constants::SPARQL_RESPONSE_OUTPUT_INFO + " " + sparqlResponseFile.generic_string());
    }

    if (batchSize != DEFAULT_BATCH_SIZE) {
        util::Logger::log(util::LogEvent::CONFIG,
                          constants::BATCH_SIZE_INFO + " " + std::to_string(batchSize));
    }

    util::Logger::log(util::LogEvent::CONFIG,
        constants::TMP_FILE_DIR_INFO + " " + tmpDir.string());
}


