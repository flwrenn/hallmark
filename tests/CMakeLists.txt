cmake_minimum_required(VERSION 3.20)
project(hallmark_tests C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# -------------------------------------------------------------------
# Unity test framework (embedded C standard)
# -------------------------------------------------------------------
# After the initial clone, skip network access on reconfigure.
# For fully offline or vendored builds, configure with:
#   -DFETCHCONTENT_SOURCE_DIR_UNITY=/path/to/unity
include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED_UNITY ON)
FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG        cbcd08fa7de711053a3deec6339ee89cad5d2697 # v2.6.1
)
FetchContent_MakeAvailable(unity)

# -------------------------------------------------------------------
# Shared algorithm library -- pure C, no RTOS dependencies
# -------------------------------------------------------------------
set(FIRMWARE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/src)

# Algorithm sources -- list new modules here explicitly
set(ALGO_SOURCES)

if(ALGO_SOURCES)
    add_library(hallmark_algorithms STATIC ${ALGO_SOURCES})
    target_include_directories(hallmark_algorithms PUBLIC ${FIRMWARE_SRC_DIR})
else()
    # No algorithm sources yet -- create an interface-only target
    add_library(hallmark_algorithms INTERFACE)
    target_include_directories(hallmark_algorithms INTERFACE ${FIRMWARE_SRC_DIR})
endif()

# -------------------------------------------------------------------
# Test suites
# -------------------------------------------------------------------
enable_testing()

add_subdirectory(smoke)
