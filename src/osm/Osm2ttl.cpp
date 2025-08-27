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

#include <omp.h>

#include <iostream>

#include "osm2rdf/util/Time.h"
#include "osm2rdf/config/ExitCode.h"
#include "osm2rdf/Version.h"
#include "osm2rdf/config/Config.h"
#include "osm2rdf/util/Output.h"
#include "osm2rdf/ttl/Writer.h"
#include "osm2rdf/ttl/Format.h"
#include "osm2rdf/osm/OsmiumHandler.h"

#include "config/Constants.h"

namespace cnst = olu::config::constants;

// _________________________________________________________________________________________________
void olu::osm::Osm2ttl::convert() {
    // Create a directory for scratch, if not already existent
    if (!std::filesystem::exists(cnst::PATH_TO_SCRATCH_DIRECTORY)) {
        std::filesystem::create_directories(cnst::PATH_TO_SCRATCH_DIRECTORY);
    }

    auto config = osm2rdf::config::Config();
    auto arguments = getArgsFromEndpoint();

    std::vector<char*> argv;
    for (const auto& arg : arguments) {
        argv.push_back((char*)arg.data());
    }
    argv.push_back(nullptr);

    config.fromArgs(argv.size() - 1, argv.data());

#if defined(_OPENMP)
    omp_set_num_threads(config.numThreads);
#endif

    try {
        // Redirect std::cout to avoid output from osm2rdf
        std::string outputFile;

        // Keep osm2rdf output in debug mode, otherwise write to temp directory and delete
        // after done
        if (_config.sparqlOutput == config::DEBUG_FILE) {
            outputFile = cnst::PATH_TO_OSM2RDF_INFO_OUTPUT_FILE_DEBUG;
        } else {
            outputFile = cnst::PATH_TO_OSM2RDF_INFO_OUTPUT_FILE;
        }
        const std::ofstream out(outputFile);
        std::streambuf *coutbuf = std::cerr.rdbuf();
        std::cerr.rdbuf(out.rdbuf());

        _stats->startTimeOsm2RdfConversion();
        run<osm2rdf::ttl::format::QLEVER>(config);
        _stats->endTimeOsm2RdfConversion();

        std::cerr.rdbuf(coutbuf); //reset to standard output again
    } catch (const std::exception& e) {
        // All exceptions used by the Osmium library derive from std::exception.
        std::cerr << osm2rdf::util::currentTimeFormatted()
                  << "osm2rdf :: " << osm2rdf::version::GIT_INFO << " :: ERROR"
                  << std::endl;
        std::cerr << e.what() << std::endl;
        std::exit(osm2rdf::config::ExitCode::EXCEPTION);
    }
}

// _________________________________________________________________________________________________
template <typename T>
void olu::osm::Osm2ttl::run(const osm2rdf::config::Config &config) {
    osm2rdf::util::Output output{config, config.output};
    if (!output.open()) {
        std::cerr << "Error opening outputfile: " << config.output << std::endl;
        exit(1);
    }
    osm2rdf::ttl::Writer<T> writer{config, &output};
    writer.writeHeader();

    osm2rdf::osm::FactHandler<T> factHandler(config, &writer);
    osm2rdf::osm::GeometryHandler<T> geomHandler(config, &writer);

    {
        osm2rdf::osm::OsmiumHandler osmiumHandler{config, &factHandler,
                                                  &geomHandler};
        osmiumHandler.handle();
    }

    // All work done, close output
    output.close();
}

// _________________________________________________________________________________________________
bool olu::osm::Osm2ttl::hasTripleForOption(const std::string& option, const std::string& condition) const {
    if (!_config.osm2rdfOptions.contains(option) || _config.osm2rdfOptions.at(option) == condition) {
        return true;
    }

    return false;
}

// _________________________________________________________________________________________________
std::vector<std::string> olu::osm::Osm2ttl::getArgsFromEndpoint() {
    std::vector<std::string> arguments = {" ",
       cnst::PATH_TO_INPUT_FILE,
       "-o",
       cnst::PATH_TO_OUTPUT_FILE,
       "-t",
       cnst::PATH_TO_SCRATCH_DIRECTORY,
       "--" + osm2rdf::config::constants::OUTPUT_COMPRESS_OPTION_LONG,
       "none"
    };

    _config.osm2rdfOptions = _odf->fetchOsm2RdfOptions();
    if (_config.osm2rdfOptions.empty()) {
        util::Logger::log(util::LogEvent::WARNING, "No osm2rdf options found on SPARQL "
                                                   "endpoint, using default options.");
        return arguments;
    }

    for (const auto& [optionName, optionValue] : _config.osm2rdfOptions) {
        if (optionValue.starts_with("false")) {
            continue;
        }

        if (optionValue.starts_with("true")) {
            arguments.emplace_back( "--" + optionName);
            continue;
        }

        arguments.emplace_back("--" + optionName);
        arguments.emplace_back(optionValue);
    }

    return arguments;
}

