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

#include "osm/OsmUpdater.h"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <util/Time.h>

#include "omp.h"

#include "osm/OsmChangeHandler.h"
#include "osm/OsmDataFetcherQLever.h"
#include "osm/OsmDataFetcherSparql.h"
#include "osm/Osm2ttl.h"
#include "osm/OsmFileHelper.h"
#include "config/Constants.h"
#include "ttl/Triple.h"
#include "util/Logger.h"

namespace cnst = olu::config::constants;

std::unique_ptr<olu::osm::OsmDataFetcher>
createOsmDataFetcher(const olu::config::Config& config, olu::osm::StatisticsHandler &stats) {
    if (config.isQLever) {
        return std::make_unique<olu::osm::OsmDataFetcherQLever>(config, stats);
    }

    return std::make_unique<olu::osm::OsmDataFetcherSparql>(config, stats);
}

// _________________________________________________________________________________________________
olu::osm::OsmUpdater::OsmUpdater(const config::Config &config) : _config(config),
                                                                 _stats(config),
                                                                 _repServer(config, _stats),
                                                                 _odf(createOsmDataFetcher(
                                                                     config, _stats)),
                                                                 _queryWriter(_config) {
    _stats.startTime();

#if defined(_OPENMP)
    omp_set_num_threads(config.numThreads);
#endif

    try {
        std::filesystem::create_directory(cnst::PATH_TO_TEMP_DIR);
        std::filesystem::create_directory(cnst::PATH_TO_CHANGE_FILE_DIR);
        std::filesystem::create_directory(cnst::PATH_TO_DUMMY_DIR);
    } catch (const std::exception &e) {
        util::Logger::log(util::LogEvent::ERROR, e.what());
        throw OsmUpdaterException("Failed to create temporary directories");
    }

    if (_config.sparqlOutput != config::ENDPOINT) {
        try {
            std::ofstream outputFile;
            outputFile.open(_config.sparqlOutputFile,
                            std::ofstream::out | std::ios_base::trunc);
            outputFile.close();
        } catch (const std::exception &e) {
            util::Logger::log(util::LogEvent::ERROR, e.what());
            throw OsmUpdaterException("Failed to clear sparql output file");
        }
    }
}

// _________________________________________________________________________________________________
void olu::osm::OsmUpdater::run() {
    // Check if the osm2rdf version that was used to create the dump on the SPARQL endpoint is the
    // same that is used in this program.
    checkOsm2RdfVersions();

    // Handle either local directory with change files or external one depending on the user
    // input
    if (!_config.changeFileDir.empty()) {
        util::Logger::log(util::LogEvent::INFO,
                          "Start handling change files at:  " + _config.changeFileDir);

        _stats.startTimeMergingChangeFiles();
        mergeChangeFiles(_config.changeFileDir);
        _stats.endTimeMergingChangeFiles();
    } else {
        // Fetch the latest database state from the replication server
        const OsmDatabaseState latestState = _repServer.fetchLatestDatabaseState();
        _stats.setLatestDatabaseState(latestState);
        util::Logger::log(util::LogEvent::INFO,
                          "Latest database state on replication server is: "
                           + olu::osm::to_string(latestState));

        _stats.startTimeDeterminingSequenceNumber();
        decideStartSequenceNumber();
        _stats.endTimeDeterminingSequenceNumber();

        if (_stats.getStartDatabaseState().sequenceNumber > latestState.sequenceNumber) {
            util::Logger::log(util::LogEvent::INFO, "Database is already up to date. DONE.");
            return;
        }

        _stats.startTimeFetchingChangeFiles();
        fetchChangeFiles();
        _stats.endTimeFetchingChangeFiles();

        _stats.startTimeMergingChangeFiles();
        mergeChangeFiles(cnst::PATH_TO_CHANGE_FILE_DIR);
        clearChangesDir();
        _stats.endTimeMergingChangeFiles();
    }

    if (!_config.bbox.empty() || !_config.pathToPolygonFile.empty()) {
        _stats.startTimeApplyingBoundaries();
        applyBoundaries();
        _stats.endTimeApplyingBoundaries();
    }

    auto och{OsmChangeHandler(_config, *_odf, _stats)};
    och.run();

    insertMetadataTriples(och);

    deleteTmpDir();
    _stats.endTime();

    _stats.printOsmStatistics();
    _stats.printUpdateStatistics();
    if (_config.showDetailedStatistics) {
        _stats.printOsm2RdfStatistics();
        _stats.printSparqlStatistics();
    }
    _stats.printTimingStatistics();

    util::Logger::log(util::LogEvent::INFO, "DONE");
}

// _________________________________________________________________________________________________
void olu::osm::OsmUpdater::decideStartSequenceNumber() {
    // Check if the user specified a sequence number
    if (_config.sequenceNumber > 0) {
        util::Logger::log(util::LogEvent::INFO, "Start from user specified sequence number: " +
                                                std::to_string(_config.sequenceNumber));
        _stats.setStartDatabaseState({"", _config.sequenceNumber});
        return;
    }

    // Check if the user specified a timestamp
    if (!_config.timestamp.empty()) {
        const std::string timestamp = _config.timestamp;
        util::Logger::log(util::LogEvent::INFO,
                          "Start from user specified timestamp: " + timestamp);

        _repServer.fetchDatabaseStateForTimestamp(timestamp);
        return;
    }

    // Check if the SPARQL endpoint was already updated from olu once
    // and pick up the sequence number
    if (auto seqNumFromEndpoint = _odf->fetchUpdatesCompleteUntil(); seqNumFromEndpoint > 0) {
        util::Logger::log(util::LogEvent::INFO,
                          "SPARQL endpoint returned sequence number: " +
                          std::to_string(seqNumFromEndpoint));
        seqNumFromEndpoint += 1; // Start one sequence number after the last run
        _stats.setStartDatabaseState({"", seqNumFromEndpoint});
        return;
    }

    // Check SPARQL endpoint for the latest node timestamp
    util::Logger::log(util::LogEvent::INFO,
                      "Fetch latest node-timestamp on SPARQL endpoint...");
    const std::string timestamp = _odf->fetchLatestTimestampOfAnyNode();
    util::Logger::log(util::LogEvent::INFO,
                      "Latest node-timestamp on SPARQL endpoint is: " + timestamp);
    _repServer.fetchDatabaseStateForTimestamp(timestamp);
}

// _________________________________________________________________________________________________
void olu::osm::OsmUpdater::mergeChangeFiles(const std::string &pathToChangeFileDir) {
    // Get names for each change file and order them after their id
    std::vector<osmium::io::File> inputs;
    for (const auto& file : std::filesystem::directory_iterator(
        pathToChangeFileDir)) {
        if (file.is_regular_file()) {
            inputs.emplace_back(file.path());
        }
    }

    if (inputs.empty()) {
        throw OsmUpdaterException("No change files found for merging.");
    }

    util::Logger::log(util::LogEvent::INFO, "Merging and sorting change files...");
    OsmFileHelper::mergeAndSortFiles(inputs, cnst::PATH_TO_CHANGE_FILE,
                                     object_order_type_id_reverse_version_delete(),
                                     inputs.size() > 1);
}

// _________________________________________________________________________________________________
void olu::osm::OsmUpdater::fetchChangeFiles() {
    util::Logger::log(util::LogEvent::INFO, "Fetching " +
        std::to_string(_stats.getNumOfChangeFiles()) + " change files from replication server...");

    osm2rdf::util::ProgressBar downloadProgress(_stats.getNumOfChangeFiles(),
                                                _stats.getNumOfChangeFiles() > 1);
    size_t counter = 0;
    downloadProgress.update(counter);
#pragma omp parallel for
    for (int currentSeqNum = _stats.getStartDatabaseState().sequenceNumber;
         currentSeqNum <= _stats.getLatestDatabaseState().sequenceNumber; currentSeqNum++) {
        _repServer.fetchChangeFile(currentSeqNum);
#pragma omp critical
        {
            downloadProgress.update(counter++);
        }
    }
    downloadProgress.done();
}

// _________________________________________________________________________________________________`
void olu::osm::OsmUpdater::applyBoundaries() const {
    util::Logger::log(util::LogEvent::INFO, "Applying boundaries to change files...");

    std::string cmd = "osmium extract " + cnst::PATH_TO_CHANGE_FILE;
    if (!_config.bbox.empty()) {
        cmd += " --bbox " + _config.bbox;
    } else if (!_config.pathToPolygonFile.empty()) {
        cmd += " --polygon " + _config.pathToPolygonFile;
    } else {
        throw OsmUpdaterException("No bounding box or polygon file specified.");
    }

    // Use the 'smart' strategy to include entire ways and multipolygons that intersect the boundary
    // (as well as the referenced nodes that are not in the boundary).
    // See the osmium-tool manual for details:
    // https://osmcode.org/osmium-tool/manual.html#creating-geographic-extracts
    cmd += " -o " + cnst::PATH_TO_CHANGE_FILE_EXTRACT + " --overwrite -s smart --no-progress";

    // Overwrite the original change file with the extracted one
    cmd += " && mv " + cnst::PATH_TO_CHANGE_FILE_EXTRACT + " " + cnst::PATH_TO_CHANGE_FILE;

    if (std::system(cmd.c_str()) != 0) {
        throw OsmUpdaterException("Failed to apply boundaries using osmium extract command.");
    }
}

// _________________________________________________________________________________________________
void olu::osm::OsmUpdater::clearChangesDir() {
    try {
        for (const auto& entry : std::filesystem::directory_iterator(
            cnst::PATH_TO_CHANGE_FILE_DIR)) {
            remove_all(entry.path());
        }
    } catch (const std::filesystem::filesystem_error& e) {
        util::Logger::log(util::LogEvent::ERROR, e.what());
        throw OsmUpdaterException("Error while removing changes directory.");
    }
}

// _________________________________________________________________________________________________
void olu::osm::OsmUpdater::deleteTmpDir() {
    try {
        for (const auto& entry : std::filesystem::directory_iterator(cnst::PATH_TO_TEMP_DIR)) {
            remove_all(entry.path());
        }
    } catch (const std::filesystem::filesystem_error& e) {
        util::Logger::log(util::LogEvent::ERROR, e.what());
        throw OsmUpdaterException("Error while removing temporary files.");
    }
}

// _________________________________________________________________________________________________
void olu::osm::OsmUpdater::checkOsm2RdfVersions() const {
    try {
        if (const std::string osm2rdfVersionEndpoint = _odf->fetchOsm2RdfVersion();
            osm2rdfVersionEndpoint != Osm2ttl::getGitInfo()) {
            util::Logger::log(util::LogEvent::WARNING, "The osm2rdf version on the SPARQL"
                                                       " endpoint (" + osm2rdfVersionEndpoint +
                                                       ") is different from the one used in this "
                                                       "program (" + Osm2ttl::getGitInfo() + ")");
        }
    } catch (const OsmDataFetcherException &e) {
        util::Logger::log(util::LogEvent::WARNING,
                          "Could not verify that the osm2rdf version that was on the SPARQL"
                          " endpoint. Please make sure that the osm2rdf version that was used to"
                          " create the dump is the same as the one used in this program.");
    }
}

// _________________________________________________________________________________________________
void olu::osm::OsmUpdater::insertMetadataTriples(OsmChangeHandler &och) {
    // Delete the old updatesCompleteUntil triple if it exists
    const std::string updatesCompleteUntil = std::to_string(_stats.getLatestDatabaseState().sequenceNumber);
    ttl::Triple updatesCompleteUntilTriple = {
        cnst::PREFIXED_OSM2RDF_META_INFO,
        cnst::PREFIXED_OSM2RDF_META_UPDATES_COMPLETE_UNTIL,
        cnst::QUERY_VAR_VAL
    };

    const auto deleteQuery = _queryWriter.writeDeleteTripleQuery(updatesCompleteUntilTriple);
    och.runUpdateQuery(sparql::UpdateOperation::DELETE, deleteQuery, cnst::PREFIXES_FOR_METADATA_TRIPLES);

    // Create a new triple for the updatesCompleteUntil
    updatesCompleteUntilTriple.object = "\"" + updatesCompleteUntil + "\"^^" + cnst::IRI_XSD_INT;

    // Create a triple for the date modified
    const std::string dateModified = util::currentIsoTime();
    const ttl::Triple dateModifiedTriple = {
        cnst::PREFIXED_OSM2RDF_META_INFO,
        cnst::PREFIXED_OSM2RDF_META_DATE_MODIFIED,
        "\"" + dateModified + "\"^^" + cnst::IRI_XSD_DATE_TIME
    };

    // Insert the new metadata triples into the database
    const auto query = _queryWriter.writeInsertQuery({
        to_string(dateModifiedTriple),
        to_string(updatesCompleteUntilTriple)
    });

    och.runUpdateQuery(sparql::UpdateOperation::INSERT, query,
                       cnst::PREFIXES_FOR_METADATA_TRIPLES);
}

