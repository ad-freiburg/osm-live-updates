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

#ifndef OSMFILEHELPER_H
#define OSMFILEHELPER_H

#include <string>

#include <osmium/io/file.hpp>
#include <osmium/osm/object.hpp>
#include <osmium/object_pointer_collection.hpp>
#include <osmium/visitor.hpp>
#include <osmium/io/output_iterator.hpp>
#include <osmium/io/reader.hpp>
#include <osmium/io/writer.hpp>
#include "osmium/io/xml_input.hpp"
#include "osmium/io/xml_output.hpp"
#include <osmium/osm/object_comparisons.hpp>

#include "osm2rdf/util/ProgressBar.h"

namespace olu::osm {
    /**
     * Ordering function for osmium::OSMObject that takes the deleted of the osm object into
     * account.
     */
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

    class OsmFileHelper {
    public:
        /**
         * Merges multiple osmium::io::File objects into a single output file
         * while sorting the objects by the given comparator.
         *
         * @tparam TCompare Comparator type that defines the comparison function for osm objects.
         * @param inputFiles Files to merge and sort.
         * @param outputFile Path to the output file where the merged and sorted objects will be
         * written.
         * @param compareFunction Compartor implementation that defines the sorting order of the
         * osm objects.
         * @param withProgressbar If true, a progress bar will be displayed during the process.
         */
        template <typename TCompare>
        static void mergeAndSortFiles(std::vector<osmium::io::File> &inputFiles,
                                      const std::string &outputFile,
                                      TCompare && compareFunction,
                                      const bool &withProgressbar) {
            osmium::io::Writer writer{outputFile, osmium::io::overwrite::allow};
            const auto out = make_output_iterator(writer);

            osm2rdf::util::ProgressBar readProgress(inputFiles.size(), withProgressbar);
            size_t counter = 0;
            readProgress.update(counter);

            std::vector<osmium::memory::Buffer> changes;
            osmium::ObjectPointerCollection objects;
            for (const osmium::io::File& change_file : inputFiles) {
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

            objects.sort(compareFunction);

            std::unique_copy(objects.cbegin(), objects.cend(), out, osmium::object_equal_type_id());
            writer.close();
        }
    };
}

#endif //OSMFILEHELPER_H
