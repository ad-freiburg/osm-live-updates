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
        util::Logger::stream() << util::Logger::PREFIX_SPACER
            << "QLever response time: " << _qleverResponseTimeMs << " ms" << std::endl;

        if (_config.sparqlOutput == config::SparqlOutput::ENDPOINT) {
            util::Logger::stream() << util::Logger::PREFIX_SPACER
                << "QLever update time: " << getQleverUpdateTimeMs() << " ms [insert operations: "
                      << _qleverInsertTimeMs << " ms, delete operations: "
                      << _qleverDeleteTimeMs << " ms]" << std::endl;

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
void olu::osm::StatisticsHandler::countQleverUpdateTime(const long &timeInMs,
                                                        const sparql::UpdateOperation & updateOp) {
    if (updateOp == sparql::UpdateOperation::INSERT) {
        _qleverInsertTimeMs += timeInMs;
    } else if (updateOp == sparql::UpdateOperation::DELETE) {
        _qleverDeleteTimeMs += timeInMs;
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

struct QLeverUpdateTriplesTimingStatistics {
    long total;
    long updateMetadata;
    long performUpdate() const { return this->total - this->updateMetadata; }
};

struct QleverProcessUpdateStatistics {
    long total;
    long preparation;
    long materializeResult;
    long clearCache;
    QLeverUpdateTriplesTimingStatistics deleteTriples;
    QLeverUpdateTriplesTimingStatistics insertTriples;
};

struct QleverExecutionTimingStatistics {
    long total;
    long acquiringDeltaTriplesWriteLock;
    long diskWriteback;
    long snapshotCreation;
    QleverProcessUpdateStatistics processUpdateImpl;
};

struct QleverTimingStatistics {
    long total;
    long snapshot;
    long planning;
    long waitingForUpdateThread;
    QleverExecutionTimingStatistics execution;
};

struct QleverDeltaTriplesStatistics {
    long deleted;
    long inserted;
};

// _________________________________________________________________________________________________
void olu::osm::StatisticsHandler::logQLeverUpdateInfo(const simdjson::padded_string &qleverResponse,
                                                      const sparql::UpdateOperation & updateOp) {
    QleverTimingStatistics qleverTiming{};
    QleverDeltaTriplesStatistics qleverDeltaTriples{};
    for (auto doc = _parser.iterate(qleverResponse);
         auto field: doc.get_object()) {
        handleParsingObjectError(field, "qlever-update-response");

        const std::string_view key = field.escaped_key();
        if (key == "delta-triples") {
            for (auto deltaField: field.value().get_object()) {
                handleParsingObjectError(deltaField, "delta-triples");

                if (deltaField.key() == "operation") {
                    for (auto diffField: deltaField.value().get_object()) {
                        handleParsingObjectError(diffField, "operation");

                        if (diffField.key() == "deleted") {
                            qleverDeltaTriples.deleted = diffField.value().get_int64().value();
                        }

                        if (diffField.key() == "inserted") {
                            qleverDeltaTriples.inserted = diffField.value().get_int64().value();
                        }
                    }
                }
            }
        }

        if ( key == "time") {
            for (auto timeField: field.value().get_object()) {
                handleParsingObjectError(timeField, "time");

                if (timeField.key() == "total") {
                    auto timeFieldValue = timeField.value().value().get_int64();
                    handleParsingIntError(timeFieldValue, "time:total");

                    qleverTiming.total = timeFieldValue.value();
                }

                if (timeField.key() == "snapshot") {
                    auto timeFieldValue = timeField.value().value().get_int64();
                    handleParsingIntError(timeFieldValue, "time:snapshot");

                    qleverTiming.snapshot = timeFieldValue.value();
                }

                if (timeField.key() == "planning") {
                    auto timeFieldValue = timeField.value().value().get_int64();
                    handleParsingIntError(timeFieldValue, "time:planning");

                    qleverTiming.planning = timeFieldValue.value();
                }

                if (timeField.key() == "waitingForUpdateThread") {
                    auto timeFieldValue = timeField.value().value().get_int64();
                    handleParsingIntError(timeFieldValue, "time:waitingForUpdateThread");

                    qleverTiming.waitingForUpdateThread = timeFieldValue.value();
                }

                if (timeField.key() == "execution") {
                    for (auto executionField: timeField.value().get_object()) {
                        handleParsingObjectError(executionField, "execution");

                        if (executionField.key() == "total") {
                            auto intField = executionField.value().value().get_int64();
                            handleParsingIntError(intField, "execution:total");
                            qleverTiming.execution.total = intField.value();
                        }

                        if (executionField.key() == "acquiringDeltaTriplesWriteLock") {
                            auto intField = executionField.value().value().get_int64();
                            handleParsingIntError(intField, "acquiringDeltaTriplesWriteLock");
                            qleverTiming.execution.acquiringDeltaTriplesWriteLock = intField.value();
                        }

                        if (executionField.key() == "diskWriteback") {
                            auto intField = executionField.value().value().get_int64();
                            handleParsingIntError(intField, "diskWriteback");
                            qleverTiming.execution.diskWriteback = intField.value();
                        }

                        if (executionField.key() == "snapshotCreation") {
                            auto intField = executionField.value().value().get_int64();
                            handleParsingIntError(intField, "snapshotCreation");
                            qleverTiming.execution.snapshotCreation = intField.value();
                        }

                        if (executionField.key() == "processUpdateImpl") {
                            for (auto processUpdateField: executionField.value().get_object()) {
                                handleParsingObjectError(processUpdateField, "processUpdateImpl");

                                if (processUpdateField.key() == "total") {
                                    auto intField = processUpdateField.value().value().get_int64();
                                    handleParsingIntError(intField, "processUpdateImpl:total");
                                    qleverTiming.execution.processUpdateImpl.total = intField.value();
                                }

                                if (processUpdateField.key() == "preparation") {
                                    auto intField = processUpdateField.value().value().get_int64();
                                    handleParsingIntError(intField, "preparation");
                                    qleverTiming.execution.processUpdateImpl.preparation = intField.value();
                                }

                                if (processUpdateField.key() == "materializeResult") {
                                    auto intField = processUpdateField.value().value().get_int64();
                                    handleParsingIntError(intField, "materializeResult");
                                    qleverTiming.execution.processUpdateImpl.materializeResult = intField.value();
                                }

                                if (processUpdateField.key() == "clearCache") {
                                    auto intField = processUpdateField.value().value().get_int64();
                                    handleParsingIntError(intField, "clearCache");
                                    qleverTiming.execution.processUpdateImpl.clearCache = intField.value();
                                }

                                if (processUpdateField.key() == "deleteTriples") {
                                    if (processUpdateField.value().type() != simdjson::ondemand::json_type::object) {
                                        auto intField = processUpdateField.value().value().get_int64();
                                        handleParsingIntError(intField, "deleteTriples");
                                        qleverTiming.execution.processUpdateImpl.insertTriples.total = intField.value();
                                    } else {
                                        for (auto deleteTriplesField: processUpdateField.value().get_object()) {
                                            handleParsingObjectError(deleteTriplesField, "deleteTriplesField");
                                            if (deleteTriplesField.key() == "total") {
                                                auto intField = deleteTriplesField.value().value().get_int64();
                                                handleParsingIntError(intField, "deleteTriples:total");
                                                qleverTiming.execution.processUpdateImpl.deleteTriples.total = intField.value();
                                            }

                                            if (deleteTriplesField.key() == "updateMetadata") {
                                                auto intField = deleteTriplesField.value().value().get_int64();
                                                handleParsingIntError(intField, "updateMetadata");
                                                qleverTiming.execution.processUpdateImpl.deleteTriples.updateMetadata = intField.value();
                                            }
                                        }
                                    }
                                }

                                if (processUpdateField.key() == "insertTriples") {
                                    if (processUpdateField.value().type() != simdjson::ondemand::json_type::object) {
                                        auto intField = processUpdateField.value().value().get_int64();
                                        handleParsingIntError(intField, "insertTriples");
                                        qleverTiming.execution.processUpdateImpl.insertTriples.total = intField.value();
                                    } else {
                                        for (auto insertTriplesField: processUpdateField.value().get_object()) {
                                            handleParsingObjectError(insertTriplesField, "insertTriplesField");
                                            if (insertTriplesField.key() == "total") {
                                                auto intField = insertTriplesField.value().value().get_int64();
                                                handleParsingIntError(intField, "deleteTriples:total");
                                                qleverTiming.execution.processUpdateImpl.insertTriples.total = intField.value();
                                            }

                                            if (insertTriplesField.key() == "updateMetadata") {
                                                auto intField = insertTriplesField.value().value().get_int64();
                                                handleParsingIntError(intField, "updateMetadata");
                                                qleverTiming.execution.processUpdateImpl.insertTriples.updateMetadata = intField.value();
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Keep track of some statistics over all update operations
    _qleverInsertedTriplesCount += qleverDeltaTriples.inserted;
    _qleverDeletedTriplesCount += qleverDeltaTriples.deleted;

    // For overall statistics we only care about the total time spent
    if (updateOp == sparql::UpdateOperation::INSERT) {
        _qleverInsertTimeMs += qleverTiming.total;
    } else if (updateOp == sparql::UpdateOperation::DELETE) {
        _qleverDeleteTimeMs += qleverTiming.total;
    }

    // Detailed statistics for this specific update operation
    if (_config.showDetailedStatistics) {
        if (updateOp == sparql::UpdateOperation::INSERT) {
            util::Logger::log(util::LogEvent::INFO, "Processed INSERT Operation:");
        } else if (updateOp == sparql::UpdateOperation::DELETE) {
            util::Logger::log(util::LogEvent::INFO, "Processed DELETE Operation:");
        }
        util::Logger::stream() << util::Logger::PREFIX_SPACER << "Inserted "
                << qleverDeltaTriples.inserted << " triples and deleted: "
                << qleverDeltaTriples.deleted  << " triples" << std::endl;

        util::Logger::stream() << util::Logger::PREFIX_SPACER << "Total time: "
                << qleverTiming.total
                << " ms [Snapshot: " << qleverTiming.execution.snapshotCreation
                << " ms, Preparation: " << qleverTiming.execution.processUpdateImpl.preparation;

        if (updateOp == sparql::UpdateOperation::INSERT) {
            util::Logger::stream() << ", Update Metadata: " << qleverTiming.execution.processUpdateImpl.insertTriples.updateMetadata
                    << " ms, Perform Update: " << qleverTiming.execution.processUpdateImpl.insertTriples.performUpdate()
                    << " ms";
        } else {
            util::Logger::stream() << ", Update Metadata: " << qleverTiming.execution.processUpdateImpl.deleteTriples.updateMetadata
                    << " ms, Perform Update: " << qleverTiming.execution.processUpdateImpl.deleteTriples.performUpdate()
                    << " ms";
        }

        util::Logger::stream() << "]" << std::endl;
    }
}

// _________________________________________________________________________________________________
void olu::osm::StatisticsHandler::handleParsingObjectError(
    const simdjson::simdjson_result<simdjson::ondemand::field> &parsingResult,
    const std::string & objectName) {
    if (parsingResult.error()) {
        util::Logger::log(util::LogEvent::ERROR,
                simdjson::error_message(parsingResult.error()));
        const auto exceptionMsg = "Error while parsing " + objectName + " object.";
        throw StatisticsHandlerException(exceptionMsg.c_str());
    }
}

// _________________________________________________________________________________________________
void olu::osm::StatisticsHandler::handleParsingIntError(
    const simdjson::simdjson_result<int64_t> &parsingResult,
    const std::string & fieldName) {
    if (parsingResult.error()) {
        util::Logger::log(util::LogEvent::ERROR,
                simdjson::error_message(parsingResult.error()));
        const auto exceptionMsg = "Error while parsing " + fieldName + " field.";
        throw StatisticsHandlerException(exceptionMsg.c_str());
    }
}
