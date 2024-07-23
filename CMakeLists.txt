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

# The compiled library code is here
add_subdirectory(src)

# The executable code is here
add_subdirectory(apps)