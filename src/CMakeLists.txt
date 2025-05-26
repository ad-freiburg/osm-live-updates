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

# Optionally glob, but only for CMake 3.12 or later:
file(GLOB_RECURSE HEADER_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/../include/osm-live-updates/*.h")
file(GLOB_RECURSE CPP_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

# Make an automatic library - will be static or dynamic based on user setting
add_library(olu_library ${CPP_LIST} ${HEADER_LIST})

## Add gtest header for FRIEND_TEST
target_include_directories(olu_library PUBLIC "${CMAKE_CURRENT_LIST_DIR}/../vendor/google/googletest/googletest/include")

# We need this directory, and users of our library will need it too
target_include_directories(olu_library PUBLIC "${CMAKE_CURRENT_LIST_DIR}/../include")

## This depends on (header only) boost (Boost::boost)
target_link_libraries(olu_library PRIVATE
        simdjson
        Boost::boost
        Boost::serialization
        Boost::iostreams
        Boost::regex
        Threads::Threads
        ${ZLIB_LIBRARIES}
        ${CURL_LIBRARIES}
        osm2rdf_library
)

# Link OpenMP if found
if (OpenMP_CXX_FOUND)
    target_link_libraries(osm2rdf_library PUBLIC OpenMP::OpenMP_CXX)
endif ()

# All users of this library will need at least C++20
target_compile_features(olu_library PUBLIC cxx_std_20)

# IDEs should put the headers in a nice place
source_group(
        TREE "${PROJECT_SOURCE_DIR}/include"
        PREFIX "Header Files"
        FILES ${HEADER_LIST})
