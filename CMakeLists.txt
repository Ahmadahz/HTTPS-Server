cmake_minimum_required(VERSION 3.10.0)
project(lark)

# Make sure developers do not run cmake in the main project directory, to keep
# build artifacts from becoming clutter
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed.
    Please make a new directory (called a build directory) and run CMake from there.
    You may need to remove CMakeCache.txt." )
endif()

# Turn on debug builds if we are building in a devel shell
if (CMAKE_BUILD_TYPE STREQUAL "" AND "$ENV{DEVEL_SHELL}" STREQUAL "1")
    message(STATUS "Setting debug build type by default in devel shell")
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Output binaries to a sub directory "bin"
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Enable cmake testing
include(CTest)
enable_testing()

# Enable GoogleTest
include(GoogleTest)
add_subdirectory(/usr/src/googletest googletest)

# Enable Boost
# Use static libraries so binaries can be deployed without a full boost install
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.50 REQUIRED COMPONENTS system thread log_setup log)
message(STATUS "Boost version: ${Boost_VERSION}")

#Enable OpenSSL for boost:ssl extension
set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)
message(STATUS "OpenSSL version: ${Openssl_VERSION}")

include_directories(include)

# RESOLVED: Update name and srcs
add_library(session_lib src/session.cc)
add_library(server_lib src/server.cc)
add_library(config_parser src/config_parser.cc)
add_library(request_handler src/echo_handler.cc src/file_handler.cc src/404_handler.cc src/status_handler.cc src/proxy_handler.cc src/sleep_handler.cc src/health_handler.cc src/logger.cc)
add_library(dispatcher src/dispatcher.cc src/echo_handler.cc src/file_handler.cc src/config_parser.cc src/404_handler.cc src/status_handler.cc src/proxy_handler.cc src/sleep_handler.cc src/health_handler.cc)
add_library(logger src/logger.cc)

# RESOLVED: Update executable name, srcs, and 
add_executable(server src/server_main.cc)
target_link_libraries(server server_lib session_lib request_handler dispatcher logger Boost::system Boost::thread Boost::log_setup Boost::log  OpenSSL::SSL OpenSSL::Crypto)
add_executable(config_parser_main src/config_parser_main.cc)
target_link_libraries(server config_parser)
target_link_libraries(config_parser_main config_parser Boost::log_setup Boost::log)

# RESOLVED: Update test executable name, srcs, and deps
add_executable(file_handler_test tests/file_handler_test.cc)
target_link_libraries(file_handler_test request_handler session_lib config_parser Boost::system Boost::thread Boost::log_setup Boost::log gtest_main)
add_executable(dispatcher_test tests/dispatcher_test.cc)
target_link_libraries(dispatcher_test dispatcher session_lib Boost::system Boost::thread Boost::log_setup Boost::log OpenSSL::SSL OpenSSL::Crypto gtest_main)
add_executable(config_parser_test tests/config_parser_test.cc)
target_link_libraries(config_parser_test config_parser Boost::log_setup Boost::log OpenSSL::SSL OpenSSL::Crypto gtest_main)
add_executable(server_test tests/server_test.cc)
target_link_libraries(server_test server_lib session_lib request_handler dispatcher logger Boost::system Boost::thread Boost::log_setup Boost::log OpenSSL::SSL OpenSSL::Crypto gtest_main gmock_main)
add_executable(session_test tests/session_test.cc)
target_link_libraries(session_test session_lib request_handler dispatcher logger Boost::system Boost::thread Boost::log_setup Boost::log OpenSSL::SSL OpenSSL::Crypto gtest_main)

# RESOLVED: Update with test binary
gtest_discover_tests(config_parser_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(server_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(session_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(file_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(dispatcher_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
add_test(NAME integration_test COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration.sh)
add_test(NAME status_test COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/tests/status_test.sh)
add_test(NAME multithreading_test COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/tests/multithread_test.sh)

# RESOLVED: Update with target/test targets
include(cmake/CodeCoverageReportConfig.cmake)
generate_coverage_report(TARGETS config_parser server_lib session_lib request_handler TESTS server_test session_test file_handler_test config_parser_test dispatcher_test)
