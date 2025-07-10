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

cmake_minimum_required(VERSION 3.27)
project(osm_live_updates
        DESCRIPTION "Live updates for the complete open street map data in an rdf database"
        LANGUAGES CXX
)

# Set path of cmake modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# C++ versions
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Export compile commands to tools like clang
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wall -Wextra -march=native -ffast-math")
# Enable the specification of additional compiler flags manually from the commandline
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ADDITIONAL_COMPILER_FLAGS}")

# Enable the specification of additional linker flags manually from the commandline
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${ADDITIONAL_LINKER_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${ADDITIONAL_LINKER_FLAGS}")

if (USE_PARALLEL)
    include(FindOpenMP)
    if (OPENMP_FOUND)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
    endif ()
endif ()

######################################
# BOOST
######################################
find_package(Boost 1.81 COMPONENTS serialization iostreams regex REQUIRED CONFIG)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

######################################
# Threading
######################################
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

######################################
# POPL
######################################
find_package(POPL REQUIRED)
include_directories(SYSTEM ${POPL_INCLUDE_DIR})

######################################
# Protozero
######################################
find_package(Protozero REQUIRED)
include_directories(SYSTEM ${PROTOZERO_INCLUDE_DIR})

######################################
# Osmium
######################################
find_package(Osmium REQUIRED COMPONENTS pbf xml)
include_directories(SYSTEM ${OSMIUM_INCLUDE_DIRS})

######################################
# Curl
######################################
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})

## Build targets for address sanitizer
set(CMAKE_C_FLAGS_ASAN
        "-fsanitize=address -fsanitize=undefined -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -fno-omit-frame-pointer -g -O1"
        CACHE STRING "Flags used by the C compiler during AddressSanitizer builds."
        FORCE)
set(CMAKE_CXX_FLAGS_ASAN
        "-fsanitize=address -fsanitize=undefined -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -fno-omit-frame-pointer -g -O1"
        CACHE STRING "Flags used by the C++ compiler during AddressSanitizer builds."
        FORCE)

# Add colored output for Ninja
if ("${CMAKE_GENERATOR}" STREQUAL "Ninja")
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        add_compile_options(-fdiagnostics-color=always)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        add_compile_options(-fcolor-diagnostics)
    endif ()
endif ()

######################################
# FetchContent
######################################
include(FetchContent)

######################################
# GTEST AND GMOCK
######################################
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG 33af80a883ddc33d9c0fac0a5b4578301efb18de # main branch on 2024/05/16
)

######################################
# osm2rdf
######################################
FetchContent_Declare(osm2rdf
        GIT_REPOSITORY  https://github.com/nicolano/osm2rdf.git
        GIT_TAG 5e043a0801e7ecbd917c0d40522d8c7fd3703856
)

######################################
# simdjson
######################################
FetchContent_Declare(
        simdjson
        GIT_REPOSITORY https://github.com/simdjson/simdjson.git
        GIT_TAG  tags/v3.12.3
        GIT_SHALLOW TRUE
)

######################################
# Apply FetchContent
######################################
FetchContent_MakeAvailable(googletest osm2rdf simdjson)

target_compile_options(osm2rdf PRIVATE -Wno-extra -Wno-all -Wno-error)

include_directories(SYSTEM ${osm2rdf_SOURCE_DIR}/include)
include_directories(SYSTEM ${osm2rdf_SOURCE_DIR}/vendor/spatialjoin/src)
include_directories(SYSTEM ${simdjson_SOURCE_DIR}/include)

message(STATUS ---)
message(STATUS "CXX_FLAGS are : " ${CMAKE_CXX_FLAGS})
message(STATUS "CXX_FLAGS_RELEASE are : " ${CMAKE_CXX_FLAGS_RELEASE})
message(STATUS "CXX_FLAGS_DEBUG are : " ${CMAKE_CXX_FLAGS_DEBUG})
message(STATUS "IMPORTANT: Make sure you have selected the desired CMAKE_BUILD_TYPE")
message(STATUS "CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
message(STATUS ---)

# The compiled library code is here
add_subdirectory(src)

# The executable code is here
add_subdirectory(apps)