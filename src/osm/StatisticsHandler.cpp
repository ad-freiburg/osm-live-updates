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

#include "osm/StatisticsHandler.h"

#include <iostream>

#include "config/Constants.h"
#include "sparql/SparqlWrapper.h"
#include "util/Logger.h"
#include "util/Time.h"

namespace cnst = olu::config::constants;

// _________________________________________________________________________________________________
void olu::osm::StatisticsHandler::printOsmStatistics() const {
    util::Logger::log(util::LogEvent::INFO, "OSM Statistics:");
    if (_config.changeFileDir.empty()) {
        util::Logger::stream() << util::Logger::PREFIX_SPACER << "Started update process at database state: "
                  << olu::osm::to_string(_startDatabaseState)
                  << std::endl;

        util::Logger::stream() << util::Logger::PREFIX_SPACER << "Ended update process at database state: "
                  << olu::osm::to_string(_latestDatabaseState)
                  << std::endl;

        util::Logger::stream() << util::Logger::PREFIX_SPACER << "Handled "
                  << std::to_string(getNumOfChangeFiles())
                  << " change files in total."
                  << std::endl;
    } else {
        util::Logger::stream() << util::Logger::PREFIX_SPACER << "Handled change files at: "
                  << _config.changeFileDir
                  << std::endl;
    }

    if (numOfNodes() == 0) {
        util::Logger::stream() << util::Logger::PREFIX_SPACER << "0 nodes in change files." << std::endl;
    } else {
        util::Logger::stream() << util::Logger::PREFIX_SPACER << "Nodes created: " << _numOfCreatedNodes
                  << ", modified: " << _numOfModifiedNodes
                  << ", deleted: " << _numOfDeletedNodes
                  << std::endl;
    }

    if (numOfWays() == 0) {
        util::Logger::stream() << util::Logger::PREFIX_SPACER << "0 ways in change files." << std::endl;
    } else {
        util::Logger::stream() << util::Logger::PREFIX_SPACER << "Ways created: " << _numOfCreatedWays
                  << ", modified: " << _numOfModifiedWays
                  << ", deleted: " << _numOfDeletedWays
                  << std::endl;
    }

    if (numOfRelations() == 0) {
        util::Logger::stream() << util::Logger::PREFIX_SPACER << "0 relations in change files." << std::endl;
    } else {
        util::Logger::stream() << util::Logger::PREFIX_SPACER << "Relations created: " << _numOfCreatedRelations
                  << ", modified: " << _numOfModifiedRelations
                  << ", deleted: " << _numOfDeletedRelations
                  << std::endl;
    }
}

// _________________________________________________________________________________________________
void olu::osm::StatisticsHandler::printUpdateStatistics() const {
    util::Logger::log(util::LogEvent::INFO, "Update Statistics:");
    util::Logger::stream() << std::fixed << std::setprecision(config::Config::DEFAULT_PERCENTAGE_PRECISION);

    if (_config.showDetailedStatistics) {
        if (_numOfNodesWithLocationChange == 0) {
            util::Logger::stream() << util::Logger::PREFIX_SPACER << "No nodes with location change." << std::endl;
        } else {
            util::Logger::stream() << util::Logger::PREFIX_SPACER << _numOfNodesWithLocationChange
                      << " modified nodes changed their location ("
                      << calculatePercentage(_numOfModifiedNodes, _numOfNodesWithLocationChange)
                      << "%)"
                      << std::endl;
        }
    }

    if (_numOfRelationsToUpdateGeometry == 0 && _numOfWaysToUpdateGeometry == 0) {
        util::Logger::stream() << util::Logger::PREFIX_SPACER << "No geometries to update" << std::endl;
    } else {
        util::Logger::stream() << util::Logger::PREFIX_SPACER << "Updated geometries for " << _numOfWaysToUpdateGeometry
                  << " ways and " << _numOfRelationsToUpdateGeometry
                  << " relations"
                  << std::endl;
    }

    if (_config.showDetailedStatistics) {
        if (getNumOfDummyNodes() == 0 && getNumOfDummyWays() == 0 && getNumOfDummyRelations() == 0) {
            util::Logger::stream() << util::Logger::PREFIX_SPACER << "No references to nodes, ways or relations needed." << std::endl;
        } else {
            util::Logger::stream() << util::Logger::PREFIX_SPACER << "Created objects from SPARQL endpoint for "
                      << getNumOfDummyNodes() << " nodes, "
                      << getNumOfDummyWays() << " ways, "
                      << getNumOfDummyRelations() << " relations"
                      << std::endl;
        }
    }
}

// _________________________________________________________________________________________________
void olu::osm::StatisticsHandler::printOsm2RdfStatistics() const {
    util::Logger::log(util::LogEvent::INFO, "Osm2Rdf Statistics:");

    util::Logger::stream() << util::Logger::PREFIX_SPACER << "Osm2Rdf converted the OSM objects into "
              << _numOfConvertedTriples << " triples"
              << std::endl;

    util::Logger::stream() << util::Logger::PREFIX_SPACER << _numOfTriplesToInsert
              << " of them are relevant for the update."
              << std::endl;
}

// _________________________________________________________________________________________________
void olu::osm::StatisticsHandler::printSparqlStatistics() const {
    util::Logger::log(util::LogEvent::INFO, "SPARQL Statistics:");

    util::Logger::stream() << util::Logger::PREFIX_SPACER
          << _queriesCount << " queries, "
          << _deleteOpCount << " delete and "
          << _insertOpCount << " insert operations were ";
    if ( _config.sparqlOutputFile.empty()) {
        util::Logger::stream() << "send to the endpoint." << std::endl;
    } else {
        util::Logger::stream() << "written to the output file at path " << _config.sparqlOutputFile << std::endl;
    }

    if (_config.isQLever) {
        util::Logger::stream() << util::Logger::PREFIX_SPACER << "QLever response time: " << _qleverResponseTimeMs << " ms";

        if (_config.sparqlOutput == config::SparqlOutput::ENDPOINT) {
            util::Logger::stream() << ", QLever update time: " << getQleverUpdateTimeMs() << " ms" << std::endl;
            util::Logger::stream() << util::Logger::PREFIX_SPACER  << "(of which "
                      << _qleverInsertTimeMs << " ms were spent on insert operations and "
                      << _qleverDeleteTimeMs << " ms on delete operations)"
                      << std::endl;

            util::Logger::stream() << util::Logger::PREFIX_SPACER << "Inserted: " << _qleverInsertedTriplesCount << " and deleted "
                      << _qleverDeletedTriplesCount << " triples at QLever endpoint" << std::endl;

            if ((_qleverInsertedTriplesCount - 3) != _numOfTriplesToInsert) {
                util::Logger::log(util::LogEvent::WARNING, "The number of triples inserted"
                                  " at the end point is not equal to the"
                                  " number of triples that need to be"
                                  " inserted.");
            }
        } else {
            util::Logger::stream() << std::endl;
        }
    }
}

// _________________________________________________________________________________________________
void olu::osm::StatisticsHandler::printTimingStatistics() const {
    util::Logger::log(util::LogEvent::INFO, "Timing Statistics:");

    util::Logger::stream() << std::fixed << std::setprecision(config::Config::DEFAULT_PERCENTAGE_PRECISION);
    util::Logger::stream() << util::Logger::PREFIX_SPACER << "The complete update process took "
            << getTimeInMSTotal()
            << " ms." << std::endl;

    if (!_config.showDetailedStatistics) {
        return;
    }

    long partTime;
    if (_config.changeFileDir.empty()) {
        partTime = getTimeInMSDeterminingSequenceNumber();
        util::Logger::stream() << util::Logger::PREFIX_SPACER << "Determining sequence number took "
                << partTime
                << " ms. ("
                << calculatePercentageOfTotalTime(partTime) << "% of total time)"
                << std::endl;

        partTime = getTimeInMSFetchingChangeFiles();
        util::Logger::stream() << util::Logger::PREFIX_SPACER << "Fetching change files took "
                << partTime
                << " ms. ("
                << calculatePercentageOfTotalTime(partTime) << "% of total time)"
                << std::endl;
    }

    partTime = getTimeInMSMergingChangeFiles();
    util::Logger::stream() << util::Logger::PREFIX_SPACER << "Merging change files took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    if (!_config.bbox.empty() || !_config.pathToPolygonFile.empty()) {
        partTime = getTimeInMSApplyingBoundaries();
        util::Logger::stream() << util::Logger::PREFIX_SPACER << "Applying boundaries took "
                << partTime
                << " ms. ("
                << calculatePercentageOfTotalTime(partTime) << "% of total time)"
                << std::endl;
    }

    partTime = getTimeInMSProcessingChangeFiles();
    util::Logger::stream() << util::Logger::PREFIX_SPACER << "Processing the change files took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSCheckingNodeLocations();
    util::Logger::stream() << util::Logger::PREFIX_SPACER << "Checking nodes for location change took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSFetchingObjectsToUpdateGeo();
    util::Logger::stream() << util::Logger::PREFIX_SPACER << "Fetching objects to update geometry for took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSFetchingReferences();
    util::Logger::stream() << util::Logger::PREFIX_SPACER << "Fetching references took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSCreatingDummyNodes();
    util::Logger::stream() << util::Logger::PREFIX_SPACER << "Creating referenced node objects took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSCreatingDummyWays();
    util::Logger::stream() << util::Logger::PREFIX_SPACER << "Creating referenced way objects took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSCreatingDummyRelations();
    util::Logger::stream() << util::Logger::PREFIX_SPACER << "Creating referenced relation objects took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSMergingAndSortingDummyFiles();
    util::Logger::stream() << util::Logger::PREFIX_SPACER << "Merging and sorting dummy files took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSOsm2RdfConversion();
    util::Logger::stream() << util::Logger::PREFIX_SPACER << "Osm2rdf conversion took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSDeletingTriples();
    util::Logger::stream() << util::Logger::PREFIX_SPACER << "Deleting triples took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSFilteringTriples();
    util::Logger::stream() << util::Logger::PREFIX_SPACER << "Filtering the triples took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSInsertingTriples();
    util::Logger::stream() << util::Logger::PREFIX_SPACER << "Inserting triples took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSInsertingMetadataTriples();
    util::Logger::stream() << util::Logger::PREFIX_SPACER << "Inserting metadata triples took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSCleanUpTmpDir();
    util::Logger::stream() << util::Logger::PREFIX_SPACER << "Cleaning up temporary files took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;
}

// _________________________________________________________________________________________________
void olu::osm::StatisticsHandler::countQleverResponseTime(const std::string_view &timeInMs) {
    const auto timeString = timeInMs.substr(0, timeInMs.size() - 2); // Remove trailing "ms"
    _qleverResponseTimeMs += std::stoi(std::string(timeString));
}

// _________________________________________________________________________________________________
void olu::osm::StatisticsHandler::countQleverUpdateTime(const std::string_view &timeInMs,
                                                        const sparql::UpdateOperation & updateOp) {
    const auto timeString = timeInMs.substr(0, timeInMs.size() - 2); // Remove trailing "ms"
    if (updateOp == sparql::UpdateOperation::INSERT) {
        _qleverInsertTimeMs += std::stoi(std::string(timeString));
    } else if (updateOp == sparql::UpdateOperation::DELETE) {
        _qleverDeleteTimeMs += std::stoi(std::string(timeString));
    }
}

// _________________________________________________________________________________________________
void olu::osm::StatisticsHandler::logQleverQueryInfo(simdjson::ondemand::object qleverResponse) {
    for (auto field: qleverResponse) {
        if (field.key() == cnst::KEY_QLEVER_COMPUTE_RESULT) {
            countQleverResponseTime(field.value().value().get_string());
        }
    }
}

// _________________________________________________________________________________________________
void olu::osm::StatisticsHandler::logQLeverUpdateInfo(const simdjson::padded_string &qleverResponse,
                                                      const sparql::UpdateOperation & updateOp) {
    for (auto doc = _parser.iterate(qleverResponse);
         auto field: doc.get_object()) {
        if (field.error()) {
            util::Logger::log(util::LogEvent::ERROR,
                "simdjson threw exception with error code: " + field.error());
            throw StatisticsHandlerException("Error while parsing QLever update response.");
        }

        const std::string_view key = field.escaped_key();
        if (key == cnst::KEY_QLEVER_DELTA_TRIPLES) {
            for (auto deltaField: field.value().get_object()) {
                if (deltaField.error()) {
                    util::Logger::log(util::LogEvent::ERROR,
                        "simdjson threw exception with error code: " + deltaField.error());
                    throw StatisticsHandlerException("Error while parsing QLever delta-field "
                                                     "response.");
                }

                if (deltaField.key() == cnst::KEY_QLEVER_DIFFERENCE) {
                    for (auto diffField: deltaField.value().get_object()) {
                        if (diffField.error()) {
                            std::cerr << diffField.error() << std::endl;
                            util::Logger::log(util::LogEvent::ERROR,
                                "simdjson threw exception with error code: " + diffField.error());
                            throw StatisticsHandlerException("Error while parsing QLever "
                                                             "differences-field response.");
                        }

                        if (diffField.key() == cnst::KEY_QLEVER_DELETED) {
                            auto diffFieldValue = diffField.value().get_int64();
                            if (diffFieldValue.error()) {
                                util::Logger::log(util::LogEvent::ERROR,
                                    "simdjson threw exception with error code: " + diffFieldValue.error());
                                throw StatisticsHandlerException("Error while parsing QLever "
                                                                 "deleted-field response.");
                            }

                            _qleverDeletedTriplesCount += diffFieldValue.value();
                        }

                        if (diffField.key() == cnst::KEY_QLEVER_INSERTED) {
                            auto diffFieldValue = diffField.value().get_int64();
                            if (diffFieldValue.error()) {
                                util::Logger::log(util::LogEvent::ERROR,
                                    "simdjson threw exception with error code: " + diffFieldValue.error());
                                throw StatisticsHandlerException("Error while parsing QLever "
                                                                 "inserted-field response.");
                            }

                            _qleverInsertedTriplesCount += diffFieldValue.value();
                        }
                    }
                }
            }
        }

        if ( key == cnst::KEY_QLEVER_TIME) {
            for (auto timeField: field.value().get_object()) {
                if (timeField.error()) {
                    util::Logger::log(util::LogEvent::ERROR,
                        "simdjson threw exception with error code: " + timeField.error());
                    throw StatisticsHandlerException("Error while parsing QLever time-field "
                                                     "response.");
                }

                if (timeField.key() == cnst::KEY_QLEVER_TOTAL) {
                    auto timeFieldValue = timeField.value().get_string();
                    if (timeFieldValue.error()) {
                        util::Logger::log(util::LogEvent::ERROR,
                            "simdjson threw exception with error code: " + timeFieldValue.error());
                        throw StatisticsHandlerException("Error while parsing QLever total-time "
                                                         "field response.");
                    }

                    countQleverUpdateTime(timeFieldValue.value(), updateOp);
                }
            }
        }
    }
}
