//
// Created by Nicolas von Trott on 11.09.24.
//

#include "osm/OsmUpdater.h"
#include "osm/OsmChangeHandler.h"
#include "config/Constants.h"
#include "osm2rdf/util/Time.h"

#include <osmium/visitor.hpp>
#include <osmium/object_pointer_collection.hpp>
#include <osmium/io/file.hpp>
#include <osmium/io/xml_output.hpp>
#include <osmium/io/writer.hpp>
#include <osmium/io/gzip_compression.hpp>
#include <osmium/io/output_iterator.hpp>
#include <osmium/io/reader.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/osm/object_comparisons.hpp>

#include <algorithm>
#include <iostream>
#include <filesystem>

namespace cnst = olu::config::constants;
namespace olu::osm {
    OsmUpdater::OsmUpdater(const config::Config &config) : _config(config), _odf(config),
                                                          _latestState({}) {
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

    void OsmUpdater::run() {
        // Handle either local directory with change files or external one depending on the user
        // input
        if (!(_config.changeFileDir.empty())) {
            std::cout
            << osm2rdf::util::currentTimeFormatted()
            << "Start handling change files at:  "
            << _config.changeFileDir
            << std::endl;

            mergeChangeFiles(_config.changeFileDir);

            auto och{OsmChangeHandler(_config)};
            och.run();
        } else {
            std::cout
            << osm2rdf::util::currentTimeFormatted()
            << "Determine sequence number to start with ..."
            << std::endl;

            _latestState = _odf.fetchLatestDatabaseState();
            const auto sequenceNumber = decideStartSequenceNumber();

            std::cout
            << osm2rdf::util::currentTimeFormatted()
            << "Start sequence number: "
            << sequenceNumber
            << " of "
            << _latestState.sequenceNumber
            << std::endl;

            if (sequenceNumber > _latestState.sequenceNumber) {
                std::cout
                << osm2rdf::util::currentTimeFormatted()
                << "Database is already up to date. DONE."
                << std::endl;

                return;
            }

            fetchChangeFiles(sequenceNumber);
            mergeChangeFiles(cnst::PATH_TO_CHANGE_FILE_DIR);
            clearChangesDir();

            std::cout
            << osm2rdf::util::currentTimeFormatted()
            << "Process changes from "
            << _latestState.sequenceNumber - sequenceNumber + 1
            << " change files..."
            << std::endl;

            auto och{OsmChangeHandler(_config)};
            och.run();

        }

        deleteTmpDir();

        std::cout
        << osm2rdf::util::currentTimeFormatted()
        << "DONE"
        << std::endl;
    }

    int OsmUpdater::decideStartSequenceNumber() {
        if (_config.sequenceNumber > 0) {
            return _config.sequenceNumber;
        }

        std::string timestamp;
        if (_config.timestamp.empty()) {
            timestamp = _odf.fetchLatestTimestampOfAnyNode();
        } else {
            timestamp = _config.timestamp;
        }

        auto [_, sequenceNumber] = _odf.fetchDatabaseStateForTimestamp(timestamp);
        return sequenceNumber;
    }

    // We need to create a new ordering because we have to take the deleted status into account for
    // the objects
    struct object_order_type_id_reverse_version_delete {

        bool operator()(const osmium::OSMObject& lhs, const osmium::OSMObject& rhs) const noexcept {
            return const_tie(lhs.type(), lhs.id() > 0, lhs.positive_id(), rhs.version(), rhs.deleted(),
                        ((lhs.timestamp().valid() && rhs.timestamp().valid()) ? rhs.timestamp() : osmium::Timestamp())) <
                   const_tie(rhs.type(), rhs.id() > 0, rhs.positive_id(), lhs.version(), lhs.deleted(),
                        ((lhs.timestamp().valid() && rhs.timestamp().valid()) ? lhs.timestamp() : osmium::Timestamp()));
        }

        /// @pre lhs and rhs must not be nullptr
        bool operator()(const osmium::OSMObject* lhs, const osmium::OSMObject* rhs) const noexcept {
            assert(lhs && rhs);
            return operator()(*lhs, *rhs);
        }

    };

    void OsmUpdater::mergeChangeFiles(const std::string &pathToChangeFileDir) {
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
        auto out = osmium::io::make_output_iterator(writer);

        std::vector<osmium::memory::Buffer> changes;
        osmium::ObjectPointerCollection objects;
        for (const osmium::io::File& change_file : inputs) {
            osmium::io::Reader reader{change_file, osmium::osm_entity_bits::object};
            while (osmium::memory::Buffer buffer = reader.read()) {
                osmium::apply(buffer, objects);
                changes.push_back(std::move(buffer));
            }
            reader.close();
        }

        objects.sort(object_order_type_id_reverse_version_delete());

        std::unique_copy(objects.cbegin(), objects.cend(), out, osmium::object_equal_type_id());
        writer.close();
    }

    void OsmUpdater::fetchChangeFiles(int sequenceNumber) {
        std::cout
        << osm2rdf::util::currentTimeFormatted()
        << "Fetch and merge change files..."
        << std::endl;

        osm2rdf::util::ProgressBar downloadProgress(
            _latestState.sequenceNumber + 1 - sequenceNumber, _config.showProgress);
        size_t counter = 0;
        downloadProgress.update(counter);

        while (sequenceNumber <= _latestState.sequenceNumber) {
            auto pathToOsmChangeFile = _odf.fetchChangeFile(sequenceNumber);
            downloadProgress.update(counter++);
            sequenceNumber++;
        }

        downloadProgress.done();
    }

    void OsmUpdater::clearChangesDir() {
        try {
            for (const auto& entry : std::filesystem::directory_iterator(
                cnst::PATH_TO_CHANGE_FILE_DIR)) {
                std::filesystem::remove_all(entry.path());
            }
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << e.what() << std::endl;
            throw OsmUpdaterException("Error while removing changes directory.");
        }
    }

    void OsmUpdater::deleteTmpDir() {
        try {
            for (const auto& entry : std::filesystem::directory_iterator(
                cnst::PATH_TO_TEMP_DIR)) {
                std::filesystem::remove_all(entry.path());
                }
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << e.what() << std::endl;
            throw OsmUpdaterException("Error while removing temporary files.");
        }
    }
}
