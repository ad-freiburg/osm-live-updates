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

#include <algorithm>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <util/Time.h>

#include "omp.h"
#include "osmium/visitor.hpp"
#include "osmium/object_pointer_collection.hpp"
#include "osmium/io/file.hpp"
#include "osmium/io/xml_input.hpp"
#include "osmium/io/xml_output.hpp"
#include "osmium/io/writer.hpp"
#include "osmium/io/gzip_compression.hpp"
#include "osmium/io/output_iterator.hpp"
#include "osmium/io/reader.hpp"
#include "osmium/memory/buffer.hpp"
#include "osmium/osm/object_comparisons.hpp"

#include "osm/OsmChangeHandler.h"
#include "osm/OsmDataFetcherQLever.h"
#include "osm/OsmDataFetcherSparql.h"
#include "osm/Osm2ttl.h"
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

        auto och{OsmChangeHandler(_config, *_odf, _stats)};
        och.run();

        insertMetadataTriples(och);
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

        if (_stats.getStartDatabaseState().sequenceNumber == latestState.sequenceNumber) {
            util::Logger::log(util::LogEvent::INFO, "Database is already up to date. DONE.");
            return;
        }

        if (_stats.getStartDatabaseState().sequenceNumber > latestState.sequenceNumber) {
            throw OsmUpdaterException("Start sequence number is greater than the latest sequence "
                                      "number on the replication server.");
        }

        _stats.startTimeFetchingChangeFiles();
        fetchChangeFiles();
        _stats.endTimeFetchingChangeFiles();

        _stats.startTimeMergingChangeFiles();
        mergeChangeFiles(cnst::PATH_TO_CHANGE_FILE_DIR);
        clearChangesDir();
        _stats.endTimeMergingChangeFiles();

        auto och{OsmChangeHandler(_config, *_odf, _stats)};
        och.run();

        insertMetadataTriples(och);
    }


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
    if (const auto seqNumFromEndpoint = _odf->fetchUpdatesCompleteUntil(); seqNumFromEndpoint > 0) {
        util::Logger::log(util::LogEvent::INFO,
                          "Start from SPARQL endpoint specified sequence number: " +
                          std::to_string(seqNumFromEndpoint));
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

// We need to create a new ordering because we have to take the deleted status into account for
// the objects
struct object_order_type_id_reverse_version_delete {

    bool operator()(const osmium::OSMObject& lhs, const osmium::OSMObject& rhs) const noexcept {
        return const_tie(lhs.type(), lhs.id() > 0, lhs.positive_id(), rhs.version(), rhs.deleted(),
                    lhs.timestamp().valid() && rhs.timestamp().valid() ? rhs.timestamp() : osmium::Timestamp()) <
               const_tie(rhs.type(), rhs.id() > 0, rhs.positive_id(), lhs.version(), lhs.deleted(),
                    lhs.timestamp().valid() && rhs.timestamp().valid() ? lhs.timestamp() : osmium::Timestamp());
    }

    /// @pre lhs and rhs must not be nullptr
    bool operator()(const osmium::OSMObject* lhs, const osmium::OSMObject* rhs) const noexcept {
        assert(lhs && rhs);
        return operator()(*lhs, *rhs);
    }

};

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

    util::Logger::log(util::LogEvent::INFO,
                      "Merging and sorting change files...");
    if (inputs.empty()) {
        throw OsmUpdaterException("No input files found");
    }

    osmium::io::Writer writer{cnst::PATH_TO_CHANGE_FILE, osmium::io::overwrite::allow};
    const auto out = make_output_iterator(writer);

    osm2rdf::util::ProgressBar readProgress(inputs.size(),
                                           inputs.size() > 1);
    size_t counter = 0;
    readProgress.update(counter);

    std::vector<osmium::memory::Buffer> changes;
    osmium::ObjectPointerCollection objects;
    for (const osmium::io::File& change_file : inputs) {
        osmium::io::Reader reader{change_file, osmium::osm_entity_bits::object};
        while (osmium::memory::Buffer buffer = reader.read()) {
            apply(buffer, objects);
            // We need to keep the buffer in storage
            changes.push_back(std::move(buffer));
        }
        reader.close();
        readProgress.update(++counter);
    }
    readProgress.done();

    objects.sort(object_order_type_id_reverse_version_delete());

    std::unique_copy(objects.cbegin(), objects.cend(), out, osmium::object_equal_type_id());
    writer.close();
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

