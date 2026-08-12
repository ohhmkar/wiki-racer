cmake_minimum_required(VERSION 3.20)

project(wiki-racer
    VERSION 0.1.1
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

# Core library

add_library(wiki-core
    src/graph.cpp
    src/search.cpp
    src/wiki_client.cpp
)

target_include_directories(wiki-core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(wiki-core
    PUBLIC
        CURL::libcurl
        nlohmann_json::nlohmann_json
)

# Main executable

add_executable(wiki-racer
    src/main.cpp
)

target_link_libraries(wiki-racer
    PRIVATE
        wiki-core
)

# Benchmarks

add_executable(wiki-racer-bench
    bench/bench.cpp
)

target_link_libraries(wiki-racer-bench
    PRIVATE
        wiki-core
)

# Tests

enable_testing()

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
)
FetchContent_MakeAvailable(googletest)

add_executable(wiki-racer-tests
    tests/search_test.cpp
)

target_link_libraries(wiki-racer-tests
    PRIVATE
        wiki-core
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(wiki-racer-tests)
