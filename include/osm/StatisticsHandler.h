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

#ifndef STATISTICSHANDLER_H
#define STATISTICSHANDLER_H

#include <string>
#include <cstddef>

#include "simdjson.h"

#include <config/Config.h>
#include <util/Types.h>

#include "OsmDatabaseState.h"

namespace olu::osm {
    class StatisticsHandler {
    public:
        explicit StatisticsHandler(const config::Config &config): _config(config) {};

        void printOsmStatistics() const;
        void printUpdateStatistics() const;
        void printOsm2RdfStatistics() const;
        void printSparqlStatistics() const;
        void printTimingStatistics() const;

        void startTime() { _startTime = std::chrono::system_clock::now(); }
        void endTime() { _endTime = std::chrono::system_clock::now(); }
        long getTimeInMSTotal() const {
            return std::chrono::duration_cast<std::chrono::milliseconds>(_endTime - _startTime).count();
        }

        void startTimeDeterminingSequenceNumber() { _startTimeDeterminingSequenceNumber = std::chrono::system_clock::now(); }
        void endTimeDeterminingSequenceNumber() { _endTimeDeterminingSequenceNumber = std::chrono::system_clock::now(); }
        long getTimeInMSDeterminingSequenceNumber() const {
            return std::chrono::duration_cast<std::chrono::milliseconds>(_endTimeDeterminingSequenceNumber - _startTimeDeterminingSequenceNumber).count();
        }

        void startTimeMergingChangeFiles() { _startTimeMergingChangeFiles = std::chrono::system_clock::now(); }
        void endTimeMergingChangeFiles() { _endTimeMergingChangeFiles = std::chrono::system_clock::now(); }
        long getTimeInMSMergingChangeFiles() const {
            return std::chrono::duration_cast<std::chrono::milliseconds>(_endTimeMergingChangeFiles - _startTimeMergingChangeFiles).count();
        }

        void startTimeFetchingChangeFiles() { _startTimeFetchingChangeFiles = std::chrono::system_clock::now(); }
        void endTimeFetchingChangeFiles() { _endTimeFetchingChangeFiles = std::chrono::system_clock::now(); }
        long getTimeInMSFetchingChangeFiles() const {
            return std::chrono::duration_cast<std::chrono::milliseconds>(_endTimeFetchingChangeFiles - _startTimeFetchingChangeFiles).count();
        }

        void startTimeProcessingChangeFiles() { _startTimeProcessingChangeFiles = std::chrono::system_clock::now(); }
        void endTimeProcessingChangeFiles() { _endTimeProcessingChangeFiles = std::chrono::system_clock::now(); }
        long getTimeInMSProcessingChangeFiles() const {
            return std::chrono::duration_cast<std::chrono::milliseconds>(_endTimeProcessingChangeFiles - _startTimeProcessingChangeFiles).count();
        }

        void startTimeCheckingNodeLocations() { _startTimeCheckingNodeLocations = std::chrono::system_clock::now(); }
        void endTimeCheckingNodeLocations() { _endTimeCheckingNodeLocations = std::chrono::system_clock::now(); }
        long getTimeInMSCheckingNodeLocations() const {
            return std::chrono::duration_cast<std::chrono::milliseconds>(_endTimeCheckingNodeLocations - _startTimeCheckingNodeLocations).count();
        }

        void startTimeFetchingObjectsToUpdateGeo() { _startTimeFetchingObjectsToUpdateGeo = std::chrono::system_clock::now(); }
        void endTimeFetchingObjectsToUpdateGeo() { _endTimeFetchingObjectsToUpdateGeo = std::chrono::system_clock::now(); }
        long getTimeInMSFetchingObjectsToUpdateGeo() const {
            return std::chrono::duration_cast<std::chrono::milliseconds>(_endTimeFetchingObjectsToUpdateGeo - _startTimeFetchingObjectsToUpdateGeo).count();
        }

        void startTimeFetchingReferences() { _startTimeFetchingReferences = std::chrono::system_clock::now(); }
        void endTimeFetchingReferences() { _endTimeFetchingReferences = std::chrono::system_clock::now(); }
        long getTimeInMSFetchingReferences() const {
            return std::chrono::duration_cast<std::chrono::milliseconds>(_endTimeFetchingReferences - _startTimeFetchingReferences).count();
        }

        void startTimeCreatingDummyNodes() { _startTimeCreatingDummyNodes = std::chrono::system_clock::now(); }
        void endTimeCreatingDummyNodes() { _endTimeCreatingDummyNodes = std::chrono::system_clock::now(); }
        long getTimeInMSCreatingDummyNodes() const {
            return std::chrono::duration_cast<std::chrono::milliseconds>(_endTimeCreatingDummyNodes - _startTimeCreatingDummyNodes).count();
        }

        void startTimeCreatingDummyWays() { _startTimeCreatingDummyWays = std::chrono::system_clock::now(); }
        void endTimeCreatingDummyWays() { _endTimeCreatingDummyWays = std::chrono::system_clock::now(); }
        long getTimeInMSCreatingDummyWays() const {
            return std::chrono::duration_cast<std::chrono::milliseconds>(_endTimeCreatingDummyWays - _startTimeCreatingDummyWays).count();
        }

        void startTimeCreatingDummyRelations() { _startTimeCreatingDummyRelations = std::chrono::system_clock::now(); }
        void endTimeCreatingDummyRelations() { _endTimeCreatingDummyRelations = std::chrono::system_clock::now(); }
        long getTimeInMSCreatingDummyRelations() const {
            return std::chrono::duration_cast<std::chrono::milliseconds>(_endTimeCreatingDummyRelations - _startTimeCreatingDummyRelations).count();
        }

        void startTimeMergingAndSortingDummyFiles() { _startTimeMergingAndSortingDummyFiles = std::chrono::system_clock::now(); }
        void endTimeMergingAndSortingDummyFiles() { _endTimeMergingAndSortingDummyFiles = std::chrono::system_clock::now(); }
        long getTimeInMSMergingAndSortingDummyFiles() const {
            return std::chrono::duration_cast<std::chrono::milliseconds>(_endTimeMergingAndSortingDummyFiles - _startTimeMergingAndSortingDummyFiles).count();
        }

        void startTimeOsm2RdfConversion() { _startTimeOsm2RdfConversion = std::chrono::system_clock::now(); }
        void endTimeOsm2RdfConversion() { _endTimeOsm2RdfConversion = std::chrono::system_clock::now(); }
        long getTimeInMSOsm2RdfConversion() const {
            return std::chrono::duration_cast<std::chrono::milliseconds>(_endTimeOsm2RdfConversion - _startTimeOsm2RdfConversion).count();
        }

        void startTimeDeletingTriples() { _startTimeDeletingTriples = std::chrono::system_clock::now(); }
        void endTimeDeletingTriples() { _endTimeDeletingTriples = std::chrono::system_clock::now(); }
        long getTimeInMSDeletingTriples() const {
            return std::chrono::duration_cast<std::chrono::milliseconds>(_endTimeDeletingTriples - _startTimeDeletingTriples).count();
        }

        void startTimeFilteringTriples() { _startTimeFilteringTriples = std::chrono::system_clock::now(); }
        void endTimeFilteringTriples() { _endTimeFilteringTriples = std::chrono::system_clock::now(); }
        long getTimeInMSFilteringTriples() const {
            return std::chrono::duration_cast<std::chrono::milliseconds>(_endTimeFilteringTriples - _startTimeFilteringTriples).count();
        }

        void startTimeInsertingTriples() { _startTimeInsertingTriples = std::chrono::system_clock::now(); }
        void endTimeInsertingTriples() { _endTimeInsertingTriples = std::chrono::system_clock::now(); }
        long getTimeInMSInsertingTriples() const {
            return std::chrono::duration_cast<std::chrono::milliseconds>(_endTimeInsertingTriples - _startTimeInsertingTriples).count();
        }

        void setStartDatabaseState(const OsmDatabaseState &state) { _startDatabaseState = state; }
        void setLatestDatabaseState(const OsmDatabaseState &state) { _latestDatabaseState = state; }
        OsmDatabaseState getStartDatabaseState() const { return _startDatabaseState; }
        OsmDatabaseState getLatestDatabaseState() const { return _latestDatabaseState; }
        size_t getNumOfChangeFiles() const {
            return _latestDatabaseState.sequenceNumber - _startDatabaseState.sequenceNumber + 1;
        }

        size_t getNumOfDummyNodes() const { return _numOfReferencesToNodes; }
        size_t getNumOfDummyWays() const {
            return _numOfReferencesToWays + _numOfWaysToUpdateGeometry;
        }
        size_t getNumOfDummyRelations() const {
            return _numOfReferencesToRelations + _numOfRelationsToUpdateGeometry;
        }

        void setNumberOfNodesWithLocationChange(const size_t num) { _numOfNodesWithLocationChange = num; }
        void setNumberOfWaysToUpdateGeometry(const size_t num) { _numOfWaysToUpdateGeometry = num; }
        void setNumberOfRelationsToUpdateGeometry(const size_t num) { _numOfRelationsToUpdateGeometry = num; }
        void setNumberOfTriplesToInsert(const size_t num) { _numOfTriplesToInsert = num; }

        void countCreatedNode() { ++_numOfCreatedNodes; }
        void countModifiedNode() { ++_numOfModifiedNodes; }
        void countDeletedNode() { ++_numOfDeletedNodes; }
        void switchModifiedToCreatedNode() { ++_numOfCreatedNodes; --_numOfModifiedNodes; }
        void countNodeWithLocationChange() { ++_numOfNodesWithLocationChange; }

        void countCreatedWay() { ++_numOfCreatedWays; }
        void countModifiedWay() { ++_numOfModifiedWays; }
        void countDeletedWay() { ++_numOfDeletedWays; }
        void switchModifiedToCreatedWay() { ++_numOfCreatedWays; --_numOfModifiedWays; }

        void countCreatedRelation() { ++_numOfCreatedRelations; }
        void countModifiedRelation() { ++_numOfModifiedRelations; }
        void countDeletedRelation() { ++_numOfDeletedRelations; }
        void switchModifiedToCreatedRelation() { ++_numOfCreatedRelations; --_numOfModifiedRelations; }

        void countWayToUpdateGeometry() { ++_numOfWaysToUpdateGeometry; }
        void countRelationToUpdateGeometry() { ++_numOfRelationsToUpdateGeometry; }

        void setNodeReferenceCount(const size_t &count) { _numOfReferencesToNodes = count; }
        void setWayReferenceCount(const size_t &count) { _numOfReferencesToWays = count; }
        void setRelationReferenceCount(const size_t &count) { _numOfReferencesToRelations = count; }

        void countQuery() { ++_queriesCount; }
        void countDeleteOp() { ++_deleteOpCount; }
        void countInsertOp() { ++_insertOpCount; }
        void countTriple() { ++_numOfConvertedTriples; }

        void logQleverQueryInfo(simdjson::ondemand::object qleverResponse);
        void logQLeverUpdateInfo(const simdjson::padded_string &qleverResponse);
    private:
        config::Config _config;
        simdjson::ondemand::parser _parser;

        OsmDatabaseState _latestDatabaseState;
        OsmDatabaseState _startDatabaseState;

        size_t _numOfCreatedNodes = 0;
        size_t _numOfModifiedNodes = 0;
        size_t _numOfDeletedNodes = 0;
        size_t numOfNodes() const {return _numOfCreatedNodes + _numOfModifiedNodes + _numOfDeletedNodes;}
        size_t _numOfNodesWithLocationChange = 0;
        // Nodes that are referenced by elements in the change file and elements for which the
        // geometry needs to be updated, that are not already in the change file.
        size_t _numOfReferencesToNodes = 0;

        size_t _numOfCreatedWays = 0;
        size_t _numOfModifiedWays = 0;
        size_t _numOfDeletedWays = 0;
        size_t numOfWays() const { return _numOfCreatedWays + _numOfModifiedWays + _numOfDeletedWays; }
        size_t _numOfWaysToUpdateGeometry = 0;
        // Ways that are referenced by elements in the change file and elements for which the
        // geometry needs to be updated, that are not already in the change file.
        size_t _numOfReferencesToWays = 0;

        size_t _numOfCreatedRelations = 0;
        size_t _numOfModifiedRelations = 0;
        size_t _numOfDeletedRelations = 0;
        size_t numOfRelations() const { return _numOfCreatedRelations + _numOfModifiedRelations + _numOfDeletedRelations; }
        size_t _numOfRelationsToUpdateGeometry = 0;
        // Relations that are referenced by elements in the change file and elements for which the
        // geometry needs to be updated, that are not already in the change file.
        size_t _numOfReferencesToRelations = 0;

        size_t _numOfConvertedTriples = 0;
        size_t _numOfTriplesToInsert = 0;

        size_t _queriesCount = 0;
        size_t _deleteOpCount = 0;
        size_t _insertOpCount = 0;
        size_t _updateOpCount = _deleteOpCount + _insertOpCount;

        size_t _qleverResponseTimeMs = 0;
        size_t _qleverUpdateTimeMs = 0;
        size_t _qleverInsertedTriplesCount = 0;
        size_t _qleverDeletedTriplesCount = 0;

        time_point_t _startTime;
        time_point_t _endTime;

        time_point_t _startTimeDeterminingSequenceNumber;
        time_point_t _endTimeDeterminingSequenceNumber;

        time_point_t _startTimeMergingChangeFiles;
        time_point_t _endTimeMergingChangeFiles;

        time_point_t _startTimeFetchingChangeFiles;
        time_point_t _endTimeFetchingChangeFiles;

        time_point_t _startTimeProcessingChangeFiles;
        time_point_t _endTimeProcessingChangeFiles;

        time_point_t _startTimeCheckingNodeLocations;
        time_point_t _endTimeCheckingNodeLocations;

        time_point_t _startTimeFetchingObjectsToUpdateGeo;
        time_point_t _endTimeFetchingObjectsToUpdateGeo;

        time_point_t _startTimeFetchingReferences;
        time_point_t _endTimeFetchingReferences;

        time_point_t _startTimeCreatingDummyNodes;
        time_point_t _endTimeCreatingDummyNodes;
        time_point_t _startTimeCreatingDummyWays;
        time_point_t _endTimeCreatingDummyWays;
        time_point_t _startTimeCreatingDummyRelations;
        time_point_t _endTimeCreatingDummyRelations;

        time_point_t _startTimeMergingAndSortingDummyFiles;
        time_point_t _endTimeMergingAndSortingDummyFiles;

        time_point_t _startTimeOsm2RdfConversion;
        time_point_t _endTimeOsm2RdfConversion;

        time_point_t _startTimeDeletingTriples;
        time_point_t _endTimeDeletingTriples;

        time_point_t _startTimeFilteringTriples;
        time_point_t _endTimeFilteringTriples;

        time_point_t _startTimeInsertingTriples;
        time_point_t _endTimeInsertingTriples;

        static double calculatePercentage(const size_t total, const size_t part) {
            if (total == 0) {
                return 0.0;
            }
            return static_cast<double>(part) / static_cast<double>(total) * 100.0;
        }

        double calculatePercentageOfTotalTime(const size_t part) const {
            return static_cast<double>(part) /
                   static_cast<double>(getTimeInMSTotal()) * 100.0;
        }

        void countQleverResponseTime(const std::string_view &timeInMs);
        void countQleverUpdateTime(const std::string_view &timeInMs);
    };

    /**
     * Exception that can appear inside the `StatisticsHandler` class.
     */
    class StatisticsHandlerException final : public std::exception {
        std::string message;

    public:
        explicit StatisticsHandlerException(const char *msg) : message(msg) { }

        [[nodiscard]] const char *what() const noexcept override {
            return message.c_str();
        }
    };
}

#endif //STATISTICSHANDLER_H
