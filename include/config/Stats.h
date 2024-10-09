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

#ifndef OSM_LIVE_UPDATES_STATS_H
#define OSM_LIVE_UPDATES_STATS_H

#include <chrono>
#include <iostream>

namespace olu::osm {
    enum OsmElement {
        NODE, WAY, RELATION
    };

    enum ChangeOperation {
        MODIFY, INSERT, DELETE
    };

    struct ChangesetStat {
        std::string elementTag;
        ChangeOperation changeOperation;

        [[nodiscard]] OsmElement osmElement() const {
            if (elementTag == "node") {
                return NODE;
            } else if (elementTag == "way") {
                return WAY;
            } else if (elementTag == "relation") {
                return RELATION;
            }
        }
    };

    class Stats {
    public:
        void add(const ChangesetStat& changesetStat) {
            _changesets.emplace_back(changesetStat);
        };

        void printResults() {
            std::cout << "Statistics ------------------------------" << std::endl;
            std::cout << "Number of modifies: " << getNumberOf(MODIFY) << std::endl;
            std::cout << "Number of insertions: " << getNumberOf(INSERT) << std::endl;
            std::cout << "Number of deletions: " << getNumberOf(DELETE) << std::endl;

            std::cout << "Number of nodes: " << getNumberOf(NODE) << std::endl;
            std::cout << "Number of ways: " << getNumberOf(WAY) << std::endl;
            std::cout << "Number of relations: " << getNumberOf(RELATION) << std::endl;
        }

    private:
        std::vector<ChangesetStat> _changesets;

        int getNumberOf(ChangeOperation operation) {
            int counter = 0;
            for (auto & changeset : _changesets) {
                if (changeset.changeOperation == operation) {
                    counter++;
                }
            }
            return counter;
        }

        int getNumberOf(OsmElement osmElement) {
            int counter = 0;
            for (auto & changeset : _changesets) {
                if (changeset.osmElement() == osmElement) {
                    counter++;
                }
            }
            return counter;
        }
    };
}

#endif //OSM_LIVE_UPDATES_STATS_H
