cmake_minimum_required(VERSION 3.20)

project(wiki-racer
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(wiki-racer
    src/main.cpp
    src/graph.cpp
    src/search.cpp
)

target_include_directories(wiki-racer
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

enable_testing()

find_package(GTest REQUIRED)

add_executable(wiki-racer-tests
    tests/search_test.cpp
    src/graph.cpp
    src/search.cpp
)

target_include_directories(wiki-racer-tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(wiki-racer-tests
    PRIVATE
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(wiki-racer-tests)
