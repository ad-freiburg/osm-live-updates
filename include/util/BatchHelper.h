// Copyright 2025, University of Freiburg
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

#ifndef BATCHHELPER_H
#define BATCHHELPER_H

#include <set>
#include <functional>

#include "osm2rdf/util/ProgressBar.h"

#include "util/Types.h"

namespace olu::util {

    class BatchHelper {
    public :
        static void doInBatches(const std::set<id_t>& set,
                         const size_t elementsPerBatch,
                         const std::function<void(std::set<id_t>)> &func) {
            std::vector vector(set.begin(), set.end());
            std::vector<std::set<id_t> > vectorBatches;
            for (auto it = vector.cbegin(), e = vector.cend(); it != vector.cend(); it = e) {
                e = it + std::min<std::size_t>(vector.end() - it, elementsPerBatch);
                vectorBatches.emplace_back(it, e);
            }

            for (const auto &vectorBatch: vectorBatches) {
                func(vectorBatch);
            }
        }

        static void doInBatchesWithProgressBar(const std::set<id_t>& set,
                         const size_t elementsPerBatch,
                         const std::function<void(std::set<id_t>, size_t)> &func) {
            std::vector vector(set.begin(), set.end());
            std::vector<std::set<id_t> > vectorBatches;

            osm2rdf::util::ProgressBar progress(set.size(),
                                                vectorBatches.size() > 1);
            size_t counter = 0;
            progress.update(counter);

            for (auto it = vector.cbegin(), e = vector.cend(); it != vector.cend(); it = e) {
                e = it + std::min<std::size_t>(vector.end() - it, elementsPerBatch);
                vectorBatches.emplace_back(it, e);
            }

            size_t batchNumber = 0;
            for (const auto &vectorBatch: vectorBatches) {
                func(vectorBatch, batchNumber++);
                counter += elementsPerBatch;
                progress.update(counter);
            }

            progress.done();
        }
    };

} // namespace olu::util

#endif //BATCHHELPER_H