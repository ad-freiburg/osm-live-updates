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
#include "osm/OsmDataFetcher.h"
#include "osm/OsmUpdater.h"
#include "config/ExitCode.h"
#include "util/Logger.h"

#include <fstream>

#include "util/Exceptions.h"

int main(int argc, char** argv) {
    auto config((olu::config::Config()));
    config.fromArgs(argc, argv);
    config.printInfo();

    try {
        auto osmUpdater = olu::osm::OsmUpdater(config);
        osmUpdater.run();
    } catch (const olu::util::DatabaseUpToDateException& e) {
        olu::util::Logger::log(olu::util::LogEvent::INFO,
                               "Database is already up to date. DONE.");
    } catch (const std::exception& e) {
        olu::util::Logger::log(olu::util::LogEvent::ERROR,
                               "Failed update process with reason: " + std::string(e.what()));
        std::exit(olu::config::ExitCode::EXCEPTION);
    }

    std::exit(olu::config::ExitCode::SUCCESS);
}
