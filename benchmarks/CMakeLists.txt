# Copyright 2024, University of Freiburg
# Authors: Nicolas von Trott <nicolasvontrott@gmail.com>.

# This file is part of osm-live-updates.
#
# osm-live-updates is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# osm-live-updates is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with osm-live-updates.  If not, see <https://www.gnu.org/licenses/>.

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Suppressing benchmark's tests" FORCE)

set_target_properties(benchmark PROPERTIES FOLDER vendor)
set_target_properties(benchmark_main PROPERTIES FOLDER vendor)

macro(package_add_benchmark BENCHMARKNAME)
    # create an executable in which the benchmarks will be stored
    add_executable(${BENCHMARKNAME} ${ARGN})
    target_link_libraries(${BENCHMARKNAME} PRIVATE benchmark::benchmark_main)
    target_link_libraries(${BENCHMARKNAME} PRIVATE olu_library)
    set_target_properties(${BENCHMARKNAME} PROPERTIES FOLDER benchmarks)
    # register benchmark for global build target
    add_dependencies(build_benchmarks ${BENCHMARKNAME})
    add_dependencies(run_benchmarks run_${BENCHMARKNAME})
    add_custom_target(run_${BENCHMARKNAME} COMMAND ${BENCHMARKNAME} DEPENDS ${BENCHMARKNAME} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/benchmarks)
endmacro()

add_custom_target(build_benchmarks)
add_custom_target(run_benchmarks)
package_add_benchmark(SparqlWrapperBenchmark sparql/SparqlWrapper.cpp)
package_add_benchmark(QueryWriterBenchmark sparql/QueryWriter.cpp)
package_add_benchmark(Osm2TtlBenchmark osm/Osm2ttl.cpp)
package_add_benchmark(OsmChangeHandlerBenchmark osm/OsmChangeHandler.cpp)
package_add_benchmark(OsmDataFetcherBenchmark osm/OsmDataFetcher.cpp)
package_add_benchmark(OsmObjectHelperBenchmark osm/OsmObjectHelper.cpp)
package_add_benchmark(TtlHelperBenchmark util/TtlHelper.cpp)
package_add_benchmark(XmlHelperBenchmark util/XmlHelper.cpp)
package_add_benchmark(UrlHelperBenchmark util/URLHelper.cpp)
