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

cmake_minimum_required(VERSION 3.25)

# --------------------------------------------------------------------------------------------------
# Project
# --------------------------------------------------------------------------------------------------
project(
        osm_live_updates
        VERSION 0.1
        DESCRIPTION "Live updates for the complete open street map data in an rdf database"
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ----------------------------------------------------------------------------
# Global settings
# ----------------------------------------------------------------------------
# ----------------------------------------------------------------------------
# External programs
# ----------------------------------------------------------------------------
# ----------------------------------------------------------------------------
# Configure dependencies
# ----------------------------------------------------------------------------
set(BOOST_ROOT "/opt/homebrew/Cellar/boost/")
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED)
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost Not found")
endif()
include_directories (${Boost_INCLUDE_DIR})

find_package(CURL REQUIRED)
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
    endif ()
    option(PACKAGE_BENCHMARKS "Build the benchmarks" ON)
    if (PACKAGE_BENCHMARKS)
        enable_testing()
        add_subdirectory(benchmarks)
    endif ()
endif ()


# The compiled library code is here
add_subdirectory(src)

# The executable code is here
add_subdirectory(apps)