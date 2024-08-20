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

#include "osm/Osm2ttl.h"
#include "osm2ttl/util/Time.h"
#include "osm2ttl/Version.h"
#include "osm2ttl/config/ExitCode.h"
#include "config/Constants.h"
#include "util/Decompressor.h"

#include <osm2ttl/config/Config.h>
#include <osm2ttl/util/Output.h>
#include <osm2ttl/ttl/Writer.h>
#include <osm2ttl/osm/OsmiumHandler.h>
#include <osm2ttl/ttl/Format.h>
#include <iostream>

namespace olu::osm {

    // _____________________________________________________________________________________________
    std::string Osm2ttl::convert(std::string& osmData) {
        writeToInputFile(osmData);

        auto config = osm2ttl::config::Config();
        std::vector<std::string> arguments = { " ",
                                               olu::config::constants::PATH_TO_INPUT_FILE,
                                               "-o",
                                               olu::config::constants::PATH_TO_OUTPUT_FILE,
                                               "-t",
                                               olu::config::constants::PATH_TO_SCRATCH_DIRECTORY
//                                               "--" + osm2ttl::config::constants::OUTPUT_NO_COMPRESS_OPTION_LONG
                                               };
        std::vector<char*> argv;
        for (const auto& arg : arguments) {
            argv.push_back((char*)arg.data());
        }
        argv.push_back(nullptr);

        config.fromArgs(argv.size() - 1, argv.data());

//        std::cerr << osm2ttl::util::currentTimeFormatted()
//                  << "osm2ttl :: " << osm2ttl::version::GIT_INFO << " :: BEGIN"
//                  << std::endl;
//        std::cerr << config.getInfo(osm2ttl::util::formattedTimeSpacer) << std::endl;
//
//        std::cerr << osm2ttl::util::currentTimeFormatted() << "Free ram: "
//                  << osm2ttl::util::ram::available() /
//                     (osm2ttl::util::ram::GIGA * 1.0)
//                  << "G/"
//                  << osm2ttl::util::ram::physPages() /
//                     (osm2ttl::util::ram::GIGA * 1.0)
//                  << "G" << std::endl;

        try {
            if (config.outputFormat == "qlever") {
                run<osm2ttl::ttl::format::QLEVER>(config);
            } else if (config.outputFormat == "nt") {
                run<osm2ttl::ttl::format::NT>(config);
            } else if (config.outputFormat == "ttl") {
                run<osm2ttl::ttl::format::TTL>(config);
            } else {
                std::cerr << osm2ttl::util::currentTimeFormatted()
                          << "osm2ttl :: " << osm2ttl::version::GIT_INFO << " :: ERROR"
                          << std::endl;
                std::cerr << "Unknown output format: " << config.outputFormat
                          << std::endl;
                std::exit(osm2ttl::config::ExitCode::FAILURE);
            }
        } catch (const std::exception& e) {
            // All exceptions used by the Osmium library derive from std::exception.
            std::cerr << osm2ttl::util::currentTimeFormatted()
                      << "osm2ttl :: " << osm2ttl::version::GIT_INFO << " :: ERROR"
                      << std::endl;
            std::cerr << e.what() << std::endl;
            std::exit(osm2ttl::config::ExitCode::EXCEPTION);
        }
        std::cerr << osm2ttl::util::currentTimeFormatted()
                  << "osm2ttl :: " << osm2ttl::version::GIT_INFO << " :: FINISHED"
                  << std::endl;

        return readTripletsFromOutputFile(config);
    }

    // _____________________________________________________________________________________________
    void Osm2ttl::writeToInputFile(std::string &data) {
        std::ofstream input;
        input.open(olu::config::constants::PATH_TO_INPUT_FILE);
        if (!input) {
            std::cerr << "Error opening file: " << olu::config::constants::PATH_TO_INPUT_FILE << std::endl;
            exit(1);
        }

        input << data;
        input.close();
    }

    // _____________________________________________________________________________________________
    std::string Osm2ttl::readTripletsFromOutputFile(const osm2ttl::config::Config& config) {
//        std::ifstream ifs(config.output);
//        std::string data((std::istreambuf_iterator<char>(ifs)),
//                         (std::istreambuf_iterator<char>()));

        std::string dataDecompressed = olu::util::Decompressor::readBzip2(config.output);
        return dataDecompressed;
    }

    template <typename T>
    void Osm2ttl::run(const osm2ttl::config::Config &config) const {
        // Setup
        // Input file reference
        osm2ttl::util::Output output{config, config.output};
        if (!output.open()) {
            std::cerr << "Error opening outputfile: " << config.output << std::endl;
            exit(1);
        }
        osm2ttl::ttl::Writer<T> writer{config, &output};
        writer.writeHeader();

        osm2ttl::osm::OsmiumHandler osmiumHandler{config, &writer};
        osmiumHandler.handle();

        // All work done, close output
        output.close();
    }

} // namespace olu::osm