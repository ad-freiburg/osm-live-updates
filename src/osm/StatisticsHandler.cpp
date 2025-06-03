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
#include <util/Time.h>

#include "config/Constants.h"

static inline constexpr std::string_view prefix = "                          ";

// _________________________________________________________________________________________________
void olu::osm::StatisticsHandler::printCurrentStep(const std::string &stepDescription) const {
    std::cout << util::currentTimeFormatted()
              << stepDescription
              << std::endl;
}

// _________________________________________________________________________________________________
void olu::osm::StatisticsHandler::printOsmStatistics() const {
    printCurrentStep("OSM Statistics:");

    if (numOfNodes() == 0) {
        std::cout << prefix << "0 nodes in change file." << std::endl;
    } else {
        std::cout << prefix << "Nodes created: " << _numOfCreatedNodes
                  << ", modified: " << _numOfModifiedNodes
                  << ", deleted: " << _numOfDeletedNodes
                  << std::endl;
    }

    if (numOfWays() == 0) {
        std::cout << prefix << "0 ways in change file." << std::endl;
    } else {
        std::cout << prefix << "Ways created: " << _numOfCreatedWays
                  << ", modified: " << _numOfModifiedWays
                  << ", deleted: " << _numOfDeletedWays
                  << std::endl;
    }

    if (numOfRelations() == 0) {
        std::cout << prefix << "0 relations in change file." << std::endl;
    } else {
        std::cout << prefix << "Relations created: " << _numOfCreatedRelations
                  << ", modified: " << _numOfModifiedRelations
                  << ", deleted: " << _numOfDeletedRelations
                  << std::endl;
    }
}

// _________________________________________________________________________________________________
void olu::osm::StatisticsHandler::printUpdateStatistics() const {
    printCurrentStep("Update Statistics:");
    std::cout << std::fixed << std::setprecision(config::Config::DEFAULT_PERCENTAGE_PRECISION);

    if (_config.showDetailedStatistics) {
        if (_numOfNodesWithLocationChange == 0) {
            std::cout << prefix << "No nodes with location change." << std::endl;
        } else {
            std::cout << prefix << _numOfNodesWithLocationChange
                      << " modified nodes changed their location ("
                      << calculatePercentage(_numOfModifiedNodes, _numOfNodesWithLocationChange)
                      << "%)"
                      << std::endl;
        }

        if (_numOfWaysWithMemberChange == 0) {
            std::cout << prefix << "No ways with member change." << std::endl;
        } else {
            std::cout << prefix << _numOfWaysWithMemberChange
                      << " modified ways have changed a member ("
                      << calculatePercentage(_numOfModifiedWays, _numOfWaysWithMemberChange)
                      << "%)"
                      << std::endl;
        }

        if (_numOfRelationsWithMemberChange == 0) {
            std::cout << prefix << "No Relations with member change." << std::endl;
        } else {
            std::cout << prefix << _numOfRelationsWithMemberChange
                      << " of modified relations have changed a member ("
                      << calculatePercentage(_numOfModifiedRelations, _numOfRelationsWithMemberChange)
                      << "%)"
                      << std::endl;
        }
    }

    if (_numOfRelationsToUpdateGeometry == 0 && _numOfWaysToUpdateGeometry == 0) {
        std::cout << prefix << "No geometries to update" << std::endl;
    } else {
        std::cout << prefix << "Updated geometries for " << _numOfWaysToUpdateGeometry
                  << " ways and " << _numOfRelationsToUpdateGeometry
                  << " relations"
                  << std::endl;
    }

    if (_config.showDetailedStatistics) {
        if (_numOfDummyNodes == 0 && _numOfDummyWays == 0 && _numOfDummyRelations == 0) {
            std::cout << prefix << "No references to nodes, ways or relations needed." << std::endl;
        } else {
            std::cout << prefix << "Created dummys for "
                      << _numOfDummyNodes << " nodes, "
                      << _numOfDummyWays << " ways, "
                      << _numOfDummyRelations << " relations"
                      << std::endl;
        }
    }
}

// _________________________________________________________________________________________________
void olu::osm::StatisticsHandler::printOsm2RdfStatistics() const {
    printCurrentStep("Osm2Rdf Statistics:");

    std::cout << prefix << "Osm2Rdf converted the OSM objects into "
              << _numOfConvertedTriples << " triples"
              << std::endl;

    std::cout << prefix << _numOfTriplesToInsert
              << " of them are relevant for the update."
              << std::endl;
}

// _________________________________________________________________________________________________
void olu::osm::StatisticsHandler::printSparqlStatistics() const {
    printCurrentStep("SPARQL Statistics:");

    std::cout << prefix << _queriesCount << " SPARQL queries and "
          << _updateQueriesCount << " update queries were send to the endpoint."
          << std::endl;

    if (_config.isQLever) {
        std::cout << prefix << "QLever response time: " << _qleverResponseTimeMs << " ms, "
                  << "QLever update time: " << _qleverUpdateTimeMs << " ms" << std::endl;
    }
}

// _________________________________________________________________________________________________
void olu::osm::StatisticsHandler::printTimingStatistics() const {
    printCurrentStep("Timing Statistics:");

    std::cout << std::fixed << std::setprecision(config::Config::DEFAULT_PERCENTAGE_PRECISION);
    std::cout << prefix << "The complete update process took "
            << getTimeInMSTotal()
            << " ms." << std::endl;

    if (!_config.showDetailedStatistics) {
        return;
    }

    long partTime;
    if (_config.changeFileDir.empty()) {
        partTime = getTimeInMSDeterminingSequenceNumber();
        std::cout << prefix << "Determining sequence number took "
                << partTime
                << " ms. ("
                << calculatePercentageOfTotalTime(partTime) << "% of total time)"
                << std::endl;

        partTime = getTimeInMSFetchingChangeFiles();
        std::cout << prefix << "Fetching change files took "
                << partTime
                << " ms. ("
                << calculatePercentageOfTotalTime(partTime) << "% of total time)"
                << std::endl;
    }

    partTime = getTimeInMSMergingChangeFiles();
    std::cout << prefix << "Merging change files took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSProcessingChangeFiles();
    std::cout << prefix << "Processing the change files took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSCheckingNodeLocations();
    std::cout << prefix << "Checking nodes for location change took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSCheckingWayMembers();
    std::cout << prefix << "Checking ways for member change took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSCheckingRelationMembers();
    std::cout << prefix << "Checking relations for member change took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSFetchingObjectsToUpdateGeo();
    std::cout << prefix << "Fetching objects to update geometry for took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSFetchingReferences();
    std::cout << prefix << "Fetching references took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSCreatingDummys();
    std::cout << prefix << "Creating dummy objects took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSOsm2RdfConversion();
    std::cout << prefix << "Osm2rdf conversion took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSDeletingTriples();
    std::cout << prefix << "Deleting triples took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSInsertingTriples();
    std::cout << prefix << "Inserting triples took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSFilteringTriples();
    std::cout << prefix << "Filtering the triples took "
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
void olu::osm::StatisticsHandler::countQleverUpdateTime(const std::string_view &timeInMs) {
    const auto timeString = timeInMs.substr(0, timeInMs.size() - 2); // Remove trailing "ms"
    _qleverUpdateTimeMs += std::stoi(std::string(timeString));
}

// _________________________________________________________________________________________________
void olu::osm::StatisticsHandler::logQleverTimingInfoQuery(simdjson::ondemand::object timeResult) {
    for (auto field: timeResult) {
        if (field.key() == config::constants::KEY_QLEVER_COMPUTE_RESULT) {
            countQleverResponseTime(field.value().value().get_string());
        }
    }
}

// _________________________________________________________________________________________________
void olu::osm::StatisticsHandler::logQleverTimingInfoUpdate(simdjson::ondemand::object timeResult) {
    for (auto field: timeResult) {
        if (field.key() == config::constants::KEY_QLEVER_TOTAL) {
            countQleverUpdateTime(field.value().value().get_string());
        }
    }
}
