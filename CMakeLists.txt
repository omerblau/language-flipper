cmake_minimum_required(VERSION 3.31)
project(first_cmake_project LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

# ── production executable (no tests here) ───────────────────────────
add_executable(first_cmake_project
        main.cpp
)

# ── unit-test target (doctest) ──────────────────────────────────────
add_executable(unit_tests
        tests/compile_check.cpp
        tests/math_tests.cpp
)

target_include_directories(unit_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/extern        # where doctest.h lives
)

enable_testing()
add_test(NAME all_unit_tests COMMAND unit_tests)
