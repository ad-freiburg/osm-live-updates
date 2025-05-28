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
        std::cout << "0 nodes in change file." << std::endl;
    } else {
        std::cout << "nodes created: " << _numOfCreatedNodes
                  << ", modified: " << _numOfModifiedNodes
                  << ", deleted: " << _numOfDeletedNodes
                  << std::endl;
    }

    if (numOfWays() == 0) {
        std::cout << "0 ways in change file." << std::endl;
    } else {
        std::cout << "ways created: " << _numOfCreatedWays
                  << ", modified: " << _numOfModifiedWays
                  << ", deleted: " << _numOfDeletedWays
                  << std::endl;
    }

    if (numOfRelations() == 0) {
        std::cout << "0 relations in change file." << std::endl;
    } else {
        std::cout << "relations created: " << _numOfCreatedRelations
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
            std::cout << "No nodes with location change." << std::endl;
        } else {
            std::cout << _numOfNodesWithLocationChange
                      << " modified nodes changed their location ("
                      << calculatePercentage(_numOfModifiedNodes, _numOfNodesWithLocationChange)
                      << "%)"
                      << std::endl;
        }

        if (_numOfWaysWithMemberChange == 0) {
            std::cout << "No ways with member change." << std::endl;
        } else {
            std::cout << _numOfWaysWithMemberChange
                      << " modified ways have changed a member ("
                      << calculatePercentage(_numOfModifiedWays, _numOfWaysWithMemberChange)
                      << "%)"
                      << std::endl;
        }

        if (_numOfRelationsWithMemberChange == 0) {
            std::cout << "No Relations with member change." << std::endl;
        } else {
            std::cout << _numOfRelationsWithMemberChange
                      << "% of modified relations have changed a member ("
                      << calculatePercentage(_numOfModifiedRelations, _numOfRelationsWithMemberChange)
                      << "%)"
                      << std::endl;
        }
    }

    if (_numOfRelationsToUpdateGeometry == 0 && _numOfWaysToUpdateGeometry == 0) {
        std::cout << "No geometries to update" << std::endl;
    } else {
        std::cout << "Updated geometries for " << _numOfWaysToUpdateGeometry
                  << " ways and " << _numOfRelationsToUpdateGeometry
                  << " relations"
                  << std::endl;
    }

    if (_config.showDetailedStatistics) {
        if (_numOfDummyNodes == 0 && _numOfDummyWays == 0 && _numOfDummyRelations == 0) {
            std::cout << "No references to nodes, ways or relations needed." << std::endl;
        } else {
            std::cout << "Created dummys for "
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

    std::cout << "Osm2Rdf converted the OSM objects into "
              << _numOfConvertedTriples << " triples"
              << std::endl;

    std::cout << _numOfTriplesToInsert
              << " of them where inserted into the database."
              << std::endl;
}

// _________________________________________________________________________________________________
void olu::osm::StatisticsHandler::printSparqlStatistics() const {
    printCurrentStep("SPARQL Statistics:");

    std::cout << _queriesCount << " SPARQL queries and "
          << _updateQueriesCount << " update queries were send to the endpoint."
          << std::endl;

    if (_config.isQLever) {
        std::cout << "QLever response time: " << _qleverResponseTimeMs << " ms, "
                  << "QLever update time: " << _qleverUpdateTimeMs << " ms" << std::endl;
    }
}

// _________________________________________________________________________________________________
void olu::osm::StatisticsHandler::printTimingStatistics() const {
    printCurrentStep("Timing Statistics:");

    std::cout << std::fixed << std::setprecision(config::Config::DEFAULT_PERCENTAGE_PRECISION);
    std::cout << "The complete update process took "
            << getTimeInMSTotal()
            << " ms." << std::endl;

    if (!_config.showDetailedStatistics) {
        return;
    }

    long partTime;
    if (_config.changeFileDir.empty()) {
        partTime = getTimeInMSDeterminingSequenceNumber();
        std::cout << "Determining sequence number took "
                << time
                << " ms. ("
                << calculatePercentageOfTotalTime(partTime) << "% of total time)"
                << std::endl;

        partTime = getTimeInMSFetchingChangeFiles();
        std::cout << "Fetching change files took "
                << partTime
                << " ms. ("
                << calculatePercentageOfTotalTime(partTime) << "% of total time)"
                << std::endl;
    }

    partTime = getTimeInMSMergingChangeFiles();
    std::cout << "Merging change files took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSProcessingChangeFiles();
    std::cout << "Processing the change files took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSCheckingNodeLocations();
    std::cout << "Checking nodes for location change took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSCheckingWayMembers();
    std::cout << "Checking ways for member change took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSCheckingRelationMembers();
    std::cout << "Checking relations for member change took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSFetchingObjectsToUpdateGeo();
    std::cout << "Fetching objects to update geometry for took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSFetchingReferences();
    std::cout << "Fetching references took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSCreatingDummys();
    std::cout << "Creating dummy objects took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSOsm2RdfConversion();
    std::cout << "Osm2rdf conversion took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSDeletingTriples();
    std::cout << "Deleting triples took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSInsertingTriples();
    std::cout << "Inserting triples took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;

    partTime = getTimeInMSFilteringTriples();
    std::cout << "Filtering the triples took "
            << partTime
            << " ms. ("
            << calculatePercentageOfTotalTime(partTime) << "% of total time)"
            << std::endl;
}
