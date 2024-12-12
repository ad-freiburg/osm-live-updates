//
// Created by Nicolas von Trott on 11.09.24.
//

#include "osm/OsmUpdater.h"
#include "osm/OsmChangeHandler.h"
#include "config/Constants.h"
#include "osm2rdf/util/Time.h"

#include <iostream>
#include <filesystem>
#include <osmium/io/header.hpp>

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

            if (sequenceNumber == _latestState.sequenceNumber) {
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
            << "Process changes in "
            << _latestState.sequenceNumber - sequenceNumber
            << " change files"
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

    void OsmUpdater::mergeChangeFiles(const std::string &pathToChangeFileDir) {
        const std::string command = "osmium merge-changes -o "+ cnst::PATH_TO_CHANGE_FILE + " "
            + pathToChangeFileDir + "*.gz " + " --overwrite --simplify > /dev/null";

        const int res = system(command.c_str());

        if (res == -1) {
            throw std::runtime_error("Error while merging osm change files.");
        }

        if (res != 0) {
            throw std::runtime_error(
                "Error while merging osm change files: " + std::to_string(res));
        }
    }

    void OsmUpdater::fetchChangeFiles(int sequenceNumber) {
        std::cout << "Fetch and merge change files ..." << std::endl;

        osm2rdf::util::ProgressBar downloadProgress(
            _latestState.sequenceNumber - sequenceNumber, _config.showProgress);
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
