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
#include "util/Exceptions.h"
#include "util/Logger.h"

namespace cnst = olu::config::constants;

std::unique_ptr<olu::osm::OsmDataFetcher>
createOsmDataFetcher(olu::config::Config &config, olu::osm::StatisticsHandler &stats) {
    if (config.isQLever) {
        return std::make_unique<olu::osm::OsmDataFetcherQLever>(config, stats);
    }

    return std::make_unique<olu::osm::OsmDataFetcherSparql>(config, stats);
}

// _________________________________________________________________________________________________
olu::osm::OsmUpdater::OsmUpdater(const config::Config &config) : _config(config),
                                                                 _stats(_config),
                                                                 _repServer(_config, _stats),
                                                                 _queryWriter(_config) {
    _stats.startTime();

#if defined(_OPENMP)
    omp_set_num_threads(config.numThreads);
#endif

    // Delete all files and folders in the temporary directory to ensure a clean start.
    // This is needed to potentially avoid conflicts with files from a previous failed update.
    deleteTmpDir();

    try {
        std::filesystem::create_directory(_config.tmpDir);
        std::filesystem::create_directory(cnst::getPathToOluTmpDir(_config.tmpDir));
        std::filesystem::create_directory(cnst::getPathToChangeFileDir(_config.tmpDir));
        std::filesystem::create_directory(cnst::getPathToDummyDir(_config.tmpDir));
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

    _odf = createOsmDataFetcher(_config, _stats);
}

// _________________________________________________________________________________________________
void olu::osm::OsmUpdater::run() {
    // Check if the osm2rdf version that was used to create the dump on the SPARQL endpoint is the
    // same that is used in this program.
    // checkOsm2RdfVersions();

    // Read the osm2rdf options from the SPARQL endpoint and adapt the config accordingly
    readOsm2RdfOptionsFromEndpoint();

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
            const std::string msg = "The sequence number from the SPARQL endpoint is larger that "
                                    "the one on the replication server.";
            throw util::DatabaseUpToDateException(msg.c_str());
        }

        if (_config.maxSequenceNumber > 0) {
            util::Logger::log(util::LogEvent::INFO, "End at user specified sequence number: "
                + std::to_string(_config.maxSequenceNumber));
            _stats.setLatestDatabaseState({"", _config.maxSequenceNumber});
        }

        _stats.startTimeFetchingChangeFiles();
        fetchChangeFiles();
        _stats.endTimeFetchingChangeFiles();

        _stats.startTimeMergingChangeFiles();
        mergeChangeFiles(cnst::getPathToChangeFileDir(_config.tmpDir));
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
    try {
        auto databaseStateFromEndpoint = _odf->fetchUpdatesCompleteUntil();
        auto replicationServerUri = _odf->fetchReplicationServer();

        if (!replicationServerUri.empty() && replicationServerUri == _config.replicationServerUri) {
            std::stringstream message;
            message.imbue(util::commaLocale);
            message << "SPARQL endpoint was last updated to database state: "
                    << osm::to_string(databaseStateFromEndpoint);
            util::Logger::log(util::LogEvent::INFO, message.str());
            // Start one sequence number after the last run
            auto startSequenceNumber = databaseStateFromEndpoint.sequenceNumber + 1;
            _stats.setStartDatabaseState({"", startSequenceNumber});
            return;
        }

        // If the replication server uri is different, we can't rely on the sequence number, so we
        // will use the timestamp instead
        std::stringstream repServerMessage;
        repServerMessage.imbue(util::commaLocale);
        repServerMessage << "SPARQL endpoint was last updated from replication server: "
                         << replicationServerUri;
        util::Logger::log(util::LogEvent::INFO, repServerMessage.str());

        std::stringstream message;
        message.imbue(util::commaLocale);
        message << "SPARQL endpoint was last updated up to timestamp: "
                << databaseStateFromEndpoint.timeStamp;
        util::Logger::log(util::LogEvent::INFO, message.str());
        _repServer.fetchDatabaseStateForTimestamp(databaseStateFromEndpoint.timeStamp);
        return;
    } catch (const util::DatabaseUpToDateException &e) {
        // We pass the exception that occurs when the database is already up to date
        throw util::DatabaseUpToDateException(e.what());
    } catch (const OsmDataFetcherException &e) {
        // This will fail if the SPARQL endpoint was never updated, so we move on to use the latest
        // node timestamp
    }

    // Check for the latest timestamp of any OSM object on the SPARQL endpoint
    util::Logger::log(util::LogEvent::INFO,
                      "Fetch latest timestamp on SPARQL endpoint...");
    const std::string timestamp = _odf->fetchLatestTimestamp();
    util::Logger::log(util::LogEvent::INFO,
                      "Latest timestamp on SPARQL endpoint is: " + timestamp);
    _repServer.fetchDatabaseStateForTimestamp(timestamp);
}

// _________________________________________________________________________________________________
void olu::osm::OsmUpdater::mergeChangeFiles(const std::string &pathToChangeFileDir) const {
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
    OsmFileHelper::mergeAndSortFiles(inputs, cnst::getPathToChangeFile(_config.tmpDir),
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

    // See the osmium-tool manual for details about the extract command:
    // https://osmcode.org/osmium-tool/manual.html#creating-geographic-extracts
    std::string cmd = "osmium extract " + cnst::getPathToChangeFile(_config.tmpDir);
    if (!_config.bbox.empty()) {
        cmd += " --bbox " + _config.bbox;
    } else if (!_config.pathToPolygonFile.empty()) {
        cmd += " --polygon " + _config.pathToPolygonFile;
    } else {
        throw OsmUpdaterException("No bounding box or polygon file specified.");
    }

    cmd += " -o " + cnst::getPathToChangeFileExtract(_config.tmpDir) + " --overwrite -s "
        + _config.extractStrategy + "  --no-progress";

    // Overwrite the original change file with the extracted one
    cmd += " && mv " + cnst::getPathToChangeFileExtract(_config.tmpDir) + " " + cnst::getPathToChangeFile(_config.tmpDir);

    if (std::system(cmd.c_str()) != 0) {
        throw OsmUpdaterException("Failed to apply boundaries using osmium extract command.");
    }
}

// _________________________________________________________________________________________________
void olu::osm::OsmUpdater::clearChangesDir() const {
    try {
        for (const auto& entry : std::filesystem::directory_iterator(
            cnst::getPathToChangeFileDir(_config.tmpDir))) {
            remove_all(entry.path());
        }
    } catch (const std::filesystem::filesystem_error& e) {
        util::Logger::log(util::LogEvent::ERROR, e.what());
        throw OsmUpdaterException("Error while removing changes directory.");
    }
}

// _________________________________________________________________________________________________
void olu::osm::OsmUpdater::deleteTmpDir() const {
    try {
        if (!std::filesystem::exists(cnst::getPathToOluTmpDir(_config.tmpDir))) {
            return;
        }

        for (const auto& entry : std::filesystem::directory_iterator(cnst::getPathToOluTmpDir(_config.tmpDir))) {
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
        std::stringstream errorMessage;
        errorMessage << "Could not verify that the osm2rdf version that was on the SPARQL endpoint."
                     << std::endl << util::Logger::PREFIX_SPACER
                     << "Please make sure that the osm2rdf version that was used to create the dump"
                        " is the same as the one used in this program.";
        util::Logger::log(util::LogEvent::WARNING, errorMessage.str());
    }
}

// _________________________________________________________________________________________________
void olu::osm::OsmUpdater::insertMetadataTriples(OsmChangeHandler &och) {
    // Delete the old updatesCompleteUntil and replicationServer triple if it exists
    ttl::Triple updatesCompleteUntilTriple = {
        cnst::PREFIXED_OSM2RDF_META_INFO,
        cnst::PREFIXED_OSM2RDF_META_UPDATES_COMPLETE_UNTIL,
        cnst::QUERY_VAR_UPDATES_COMPLETE_UNTIL
    };
    ttl::Triple replicationServerTriple = {
        cnst::PREFIXED_OSM2RDF_META_INFO,
        cnst::PREFIXED_OSM2RDF_META_REPLICATION_SERVER,
        cnst::QUERY_VAR_REPLICATION_SERVER
    };

    const auto deleteQuery = _queryWriter.writeDeleteTripleQuery({updatesCompleteUntilTriple, replicationServerTriple});
    och.runUpdateQuery(sparql::UpdateOperation::DELETE, deleteQuery, cnst::PREFIXES_FOR_METADATA_TRIPLES);

    // Do not insert new metadata triples if a replication server URI is not provided,
    // as the database state is unknown in that case.
    if (_config.replicationServerUri.empty()) {
        return;
    }

    std::vector<std::string> metadataTriples;
    // Create a new triple for the updatesCompleteUntil
    const std::string updatesCompleteUntil = osm::to_string(_stats.getLatestDatabaseState());
    updatesCompleteUntilTriple.object = "\"" + updatesCompleteUntil + "\"";
    metadataTriples.emplace_back(to_string(updatesCompleteUntilTriple));

    // Create a triple for the replication server
    replicationServerTriple.object = "\"" + _config.replicationServerUri + "\"";
    metadataTriples.emplace_back(to_string(replicationServerTriple));

    // Create a triple for the date modified
    const std::string dateModified = util::currentIsoTime();
    const ttl::Triple dateModifiedTriple = {
        cnst::PREFIXED_OSM2RDF_META_INFO,
        cnst::PREFIXED_OSM2RDF_META_DATE_MODIFIED,
        "\"" + dateModified + "\"^^" + cnst::IRI_XSD_DATE_TIME
    };
    metadataTriples.emplace_back(to_string(dateModifiedTriple));

    // Insert the new metadata triples into the database
    const auto query = _queryWriter.writeInsertQuery(metadataTriples);

    och.runUpdateQuery(sparql::UpdateOperation::INSERT, query,
                       cnst::PREFIXES_FOR_METADATA_TRIPLES);
}

// _________________________________________________________________________________________________
void olu::osm::OsmUpdater::readOsm2RdfOptionsFromEndpoint() {
    _config.osm2rdfOptions = _odf->fetchOsm2RdfOptions();
    if (_config.osm2rdfOptions.empty()) {
        util::Logger::log(util::LogEvent::WARNING, "No osm2rdf options found on SPARQL "
                                                   "endpoint, using default values.");
        return;
    }

    // Check if a separate IRI prefix for untagged nodes is used and set the corresponding value in
    // the config if that is the case
    try {
        const auto separateUriForUntaggedNodes = _config.osm2rdfOptions.at(
            osm2rdf::config::constants::IRI_PREFIX_FOR_UNTAGGED_NODES_OPTION_LONG);
        if (!separateUriForUntaggedNodes.empty() && separateUriForUntaggedNodes !=
            cnst::NAMESPACE_IRI_OSM_NODE) {
            _config.separatePrefixForUntaggedNodes = separateUriForUntaggedNodes;
        }
    } catch (const std::exception &e) {
        util::Logger::log(util::LogEvent::WARNING, "Could not find value for option"
                                                   " '--iri-prefix-for-untagged-nodes', using"
                                                   " default prefix.");
    }
}


