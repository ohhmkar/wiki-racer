cmake_minimum_required(VERSION 3.20)

project(wiki-racer
    VERSION 0.1.1
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Core library
add_library(wiki-core
    src/graph.cpp
    src/search.cpp
)

target_include_directories(wiki-core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

# Main executable
add_executable(wiki-racer
    src/main.cpp
)

target_link_libraries(wiki-racer
    PRIVATE
        wiki-core
)

# Tests
enable_testing()

find_package(GTest REQUIRED)

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
