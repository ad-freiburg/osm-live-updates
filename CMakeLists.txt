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

cmake_minimum_required(VERSION 3.16.3)
set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)

# --------------------------------------------------------------------------------------------------
# Project
# --------------------------------------------------------------------------------------------------
project(
        osm_live_updates
        VERSION 1.0.0
        DESCRIPTION "Live updates for the complete open street map data in an rdf database"
        LANGUAGES CXX
)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# export compile commands to tools like clang
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ----------------------------------------------------------------------------
# Global settings
# ----------------------------------------------------------------------------
add_compile_options(-Wall -Wextra -Wno-missing-field-initializers)
add_compile_options(-DGTEST_HAS_TR1_TUPLE=0 -DGTEST_USE_OWN_TR1_TUPLE=0)
# Basic optimization
add_compile_options(-march=native)
# Enable fast-math
add_compile_options(-ffast-math)

# ----------------------------------------------------------------------------
# External programs
# ----------------------------------------------------------------------------
find_program(
        CLANG_TIDY_EXE
        NAMES "clang-tidy"
        DOC "Path to clang-tidy executable"
)

find_program(
        CPPCHECK_EXE
        NAMES "cppcheck"
        DOC "Path to cppcheck executable"
)

# ----------------------------------------------------------------------------
# Configure dependencies
# ----------------------------------------------------------------------------
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED serialization iostreams regex)
mark_as_advanced(CLEAR BOOST_ROOT)

if(Boost_FOUND)
    include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
else()
    set(BOOST_ROOT "NOT FOUND: please choose" CACHE PATH "")
    message(FATAL_ERROR "PLEASE, specify the directory where the Boost library is installed in BOOST_ROOT")
endif()

find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
    include_directories(SYSTEM ${ZLIB_INCLUDE_DIRS})
else()
    set(ZLIB_ROOT "NOT FOUND: please choose" CACHE PATH "")
    message(FATAL_ERROR "PLEASE, specify the directory where the zlib library is installed in ZLIB_ROOT")
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Disable installation of google stuff
set(INSTALL_GMOCK OFF)
set(INSTALL_GTEST OFF)
set(BENCHMARK_ENABLE_INSTALL OFF)

find_package(Git QUIET)
if (GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    # Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if (GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if (NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif ()
    endif ()
endif ()

if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vendor/google/googletest/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif ()

include(FetchContent)
include(CMakePrintHelpers)
find_package(POPL REQUIRED)
include_directories(SYSTEM ${POPL_INCLUDE_DIR})

FetchContent_Declare(osm2rdf
        GIT_REPOSITORY  https://github.com/ad-freiburg/osm2rdf.git
        GIT_TAG 07a9f65216f6df2fce5d064156e6d15903e8ccfd
)
FetchContent_MakeAvailable(osm2rdf)
include_directories(SYSTEM ${osm2rdf_SOURCE_DIR}/include)
include_directories(SYSTEM ${osm2rdf_SOURCE_DIR}/vendor/spatialjoin/src)

find_package(Protozero REQUIRED)
include_directories(SYSTEM ${PROTOZERO_INCLUDE_DIR})
find_package(Osmium REQUIRED COMPONENTS pbf xml)
include_directories(SYSTEM ${OSMIUM_INCLUDE_DIRS})

find_package(CURL REQUIRED)
if(NOT CURL_FOUND)
    message(FATAL_ERROR "Curl Not found")
endif()
include_directories(${CURL_INCLUDE_DIR})

# Only do these if this is the main project, and not if it is included through add_subdirectory
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

    set(default_build_type "Release")
    if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
        message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
        set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
        # Set the possible values of build type for cmake-gui
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
    endif ()

    # Optionally set things like CMAKE_CXX_STANDARD, CMAKE_POSITION_INDEPENDENT_CODE here

    # Let's ensure -std=c++xx instead of -std=g++xx
    set(CMAKE_CXX_EXTENSIONS OFF)

    # Let's nicely support folders in IDEs
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    # Testing only available if this is the main app
    # Note this needs to be done in the main CMakeLists
    # since it calls enable_testing, which must be in the
    # main CMakeLists.
    option(PACKAGE_TESTS "Build the tests" ON)
    if (PACKAGE_TESTS)
        enable_testing()
        include(GoogleTest)
        add_subdirectory(tests)
        include_directories(SYSTEM ${gtest_SOURCE_DIR}/include)
    endif ()
    option(PACKAGE_BENCHMARKS "Build the benchmarks" ON)
    if (PACKAGE_BENCHMARKS)
        enable_testing()
        add_subdirectory(benchmarks)
        include_directories(SYSTEM ${benchmark_SOURCE_DIR}/include)
    endif ()
endif ()

if (CLANG_TIDY_EXE)
    message(STATUS "Found CLANG_TIDY_EXE at ${CLANG_TIDY_EXE}, enabling checks")
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-checks=-*,readability-*")
else()
    message(STATUS "CLANG_TIDY_EXE not found")
endif ()
if (CPPCHECK_EXE)
    message(STATUS "Found CPPCHECK_EXE at ${CPPCHECK_EXE}, enabling checks")
    set(CMAKE_CXX_CPPCHECK "${CPPCHECK_EXE}")
else()
    message(STATUS "CPPCHECK_EXE not found")
endif ()

# The compiled library code is here
add_subdirectory(src)

# The executable code is here
add_subdirectory(apps)

# Install target settings
install(
        FILES build/apps/olu DESTINATION bin
        PERMISSIONS OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE COMPONENT binaries
)