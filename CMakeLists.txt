project(qmenumodel)

cmake_minimum_required(VERSION 2.8.0)

find_package(Qt4 REQUIRED)

include(FindPkgConfig)
pkg_check_modules(GLIB REQUIRED glib-2.0>=2.32)
add_definitions(-DQT_NO_KEYWORDS)

find_program(DBUS_RUNNER dbus-test-runner)

# Cooverage tools
OPTION(BUILD_WITH_COVERAGE "Build with coverage analysis support" OFF)
if(BUILD_WITH_COVERAGE)
    message(STATUS "Using coverage flags")
    SET(COVERAGE_COMMAND "/usr/bin/gcov")
    SET(CMAKE_C_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
    SET(CMAKE_CXX_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage -lgcov")
    include(${CMAKE_SOURCE_DIR}/cmake/lcov.cmake)
endif()

add_subdirectory(src)
add_subdirectory(examples)


# Tests Tools
#if(NOT DBUS_RUNNER)
#    message(STATUS "dbus-test-runner not found tests disabled.")
#else()
#    enable_testing()
#    add_subdirectory(tests)
#endif()

