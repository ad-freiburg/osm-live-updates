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

#include "osm/Osm2Rdf.h"

#include <osm2ttl/config/Config.h>
#include <osm2ttl/util/Output.h>
#include <osm2ttl/ttl/Writer.h>
#include <osm2ttl/osm/OsmiumHandler.h>
#include <osm2ttl/ttl/Format.h>
#include <iostream>

namespace olu::osm {
    // _____________________________________________________________________________________________
    Osm2Rdf::Osm2Rdf() {
        _config = osm2ttl::config::Config();
        _config.input = _config.getTempPath("osm2rdf", "input");
        _config.output = _config.getTempPath("osm2rdf", "output");
        _config.cache = _config.getTempPath("osm2rdf", "scratch");
    }

    // _____________________________________________________________________________________________
    std::string Osm2Rdf::convert(std::string &osmData) const {
        // Write data to input file
        writeToInputFile(osmData);

        // Generate output file
        auto output = osm2ttl::util::Output{_config, _config.output};
        if (!output.open()) {
            std::cerr << "Error opening outputfile: " << _config.output << std::endl;
            exit(1);
        }

        osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER> writer{_config, &output};
        writer.writeHeader();

        osm2ttl::osm::OsmiumHandler osmiumHandler{_config, &writer};
        osmiumHandler.handle();

        // All work done, close output
        output.close();

        return readTripletsFromOutputFile();
    }

    // _____________________________________________________________________________________________
    void Osm2Rdf::writeToInputFile(std::string &data) const {
        std::ofstream input;
        input.open(_config.input, std::ofstream::out | std::ofstream::trunc);
        if (!input) {
            std::cerr << "Error opening input file: " << _config.input << std::endl;
            exit(1);
        }

        input << data;
        input.close();
    }

    // _____________________________________________________________________________________________
    std::string Osm2Rdf::readTripletsFromOutputFile() const {
        std::ifstream ifs(_config.output);
        std::string data((std::istreambuf_iterator<char>(ifs)),
                         (std::istreambuf_iterator<char>()));
        return data;
    }

} // namespace olu::osm