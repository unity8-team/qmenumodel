project(qmenumodel)

cmake_minimum_required(VERSION 2.8.9)

find_package(Qt5Core REQUIRED)
include(FindPkgConfig)
pkg_check_modules(GLIB REQUIRED glib-2.0>=2.32)
pkg_check_modules(GIO REQUIRED gio-2.0>=2.32)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
add_definitions(-DQT_NO_KEYWORDS)

find_program(DBUS_RUNNER dbus-test-runner)

# Cooverage tools
OPTION(BUILD_WITH_COVERAGE "Build with coverage analysis support" OFF)
if(BUILD_WITH_COVERAGE)
    message(STATUS "Using coverage flags")
    find_program(COVERAGE_COMMAND gcov)
    if(NOT COVERAGE_COMMAND)
        message(FATAL_ERROR "gcov command not found")
    endif()
    SET(CMAKE_C_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
    SET(CMAKE_CXX_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage -lgcov")
    include(${CMAKE_SOURCE_DIR}/cmake/lcov.cmake)
endif()

add_subdirectory(libqmenumodel)

# Tests Tools
if(NOT DBUS_RUNNER)
    message(STATUS "dbus-test-runner not found tests disabled.")
else()
    enable_testing()
    add_subdirectory(tests)
endif()

# Doc
OPTION(GENERATE_DOC "Enable qdoc generation" OFF)
if(GENERATE_DOC)
    message(STATUS "QDoc enabled.")
    find_program(QDOC_BIN qdoc)
    if(NOT QDOC_BIN)
        message(FATAL_ERROR "qdoc command not found")
    else()
        add_subdirectory(doc)
    endif()
endif()

