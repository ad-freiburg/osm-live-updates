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
#include "config/Constants.h"
#include "osm2rdf/util/Time.h"

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
                                                                 _repServer(config),
                                                                 _stats(config),
                                                                 _odf(createOsmDataFetcher(config, _stats)),
                                                                 _latestState({}) {
    _stats.startTime();

#if defined(_OPENMP)
    omp_set_num_threads(config.numThreads);
#endif

    try {
        std::filesystem::create_directory(cnst::PATH_TO_TEMP_DIR);
        std::filesystem::create_directory(cnst::PATH_TO_CHANGE_FILE_DIR);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        throw OsmUpdaterException("Failed to create temporary directories");
    }

    if (_config.sparqlOutput != config::ENDPOINT) {
        try {
            std::ofstream outputFile;
            outputFile.open (_config.sparqlOutputFile,
                std::ofstream::out | std::ios_base::trunc);
            outputFile.close();
        } catch (const std::exception &e) {
            std::cerr << e.what() << std::endl;
            throw OsmUpdaterException("Failed to clear sparql output file");
        }
    }
}

// _________________________________________________________________________________________________
void olu::osm::OsmUpdater::run() {
    // Handle either local directory with change files or external one depending on the user
    // input
    if (!_config.changeFileDir.empty()) {
        _stats.printCurrentStep("Start handling change files at:  " + _config.changeFileDir);

        _stats.startTimeMergingChangeFiles();
        mergeChangeFiles(_config.changeFileDir);
        _stats.endTimeFetchingChangeFiles();

        auto och{OsmChangeHandler(_config, *_odf, _stats)};
        och.run();
    } else {
        _stats.startTimeDeterminingSequenceNumber();
        _stats.printCurrentStep("Determine sequence number to start with ...");

        _latestState = _repServer.fetchLatestDatabaseState();
        const auto sequenceNumber = decideStartSequenceNumber();

        _stats.printCurrentStep("Start sequence number: " + std::to_string(sequenceNumber)
                                + " of " + std::to_string(_latestState.sequenceNumber));

        _stats.endTimeDeterminingSequenceNumber();

        if (sequenceNumber > _latestState.sequenceNumber) {
            _stats.printCurrentStep("Database is already up to date. DONE.");
            return;
        }

        _stats.startTimeFetchingChangeFiles();
        fetchChangeFiles(sequenceNumber);
        _stats.endTimeFetchingChangeFiles();

        _stats.startTimeMergingChangeFiles();
        mergeChangeFiles(cnst::PATH_TO_CHANGE_FILE_DIR);
        clearChangesDir();
        _stats.endTimeMergingChangeFiles();

        _stats.printCurrentStep("Process changes from "
                                + std::to_string(_latestState.sequenceNumber - sequenceNumber + 1)
                                + " change files...");

        auto och{OsmChangeHandler(_config, *_odf, _stats)};
        och.run();
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

    _stats.printCurrentStep("DONE");
}

// _________________________________________________________________________________________________
int olu::osm::OsmUpdater::decideStartSequenceNumber() const {
    if (_config.sequenceNumber > 0) {
        return _config.sequenceNumber;
    }

    std::string timestamp;
    if (_config.timestamp.empty()) {
        timestamp = _odf->fetchLatestTimestampOfAnyNode();
    } else {
        timestamp = _config.timestamp;
    }

    auto [_, sequenceNumber] = _repServer.fetchDatabaseStateForTimestamp(timestamp);
    return sequenceNumber;
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

    if (inputs.empty()) {
        throw OsmUpdaterException("No input files found");
    }

    osmium::io::Writer writer{cnst::PATH_TO_CHANGE_FILE, osmium::io::overwrite::allow};
    const auto out = make_output_iterator(writer);

    std::vector<osmium::memory::Buffer> changes;
    osmium::ObjectPointerCollection objects;
    for (const osmium::io::File& change_file : inputs) {
        osmium::io::Reader reader{change_file, osmium::osm_entity_bits::object};
        while (osmium::memory::Buffer buffer = reader.read()) {
            apply(buffer, objects);
            changes.push_back(std::move(buffer));
        }
        reader.close();
    }

    objects.sort(object_order_type_id_reverse_version_delete());

    std::unique_copy(objects.cbegin(), objects.cend(), out, osmium::object_equal_type_id());
    writer.close();
}

// _________________________________________________________________________________________________
void olu::osm::OsmUpdater::fetchChangeFiles(int sequenceNumber) {
    _stats.printCurrentStep("Fetch and merge change files...");

    osm2rdf::util::ProgressBar downloadProgress(
        _latestState.sequenceNumber + 1 - sequenceNumber, _config.showProgress);
    size_t counter = 0;
    downloadProgress.update(counter);

#pragma omp parallel for
    for (int i = sequenceNumber; i <= _latestState.sequenceNumber; i++) {
        auto pathToOsmChangeFile = _repServer.fetchChangeFile(i);
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
        std::cerr << e.what() << std::endl;
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
        std::cerr << e.what() << std::endl;
        throw OsmUpdaterException("Error while removing temporary files.");
    }
}
