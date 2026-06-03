cmake_minimum_required(VERSION 3.24)

project(Nightfall LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(nightfall_core STATIC
  core/nightfall.cpp
)

target_include_directories(nightfall_core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/core
)

target_compile_options(nightfall_core PRIVATE
  -Wall
  -Wextra
  -Wpedantic
)

enable_testing()

add_executable(nightfall_core_tests
  core/nightfall.test.cpp
)

target_link_libraries(nightfall_core_tests PRIVATE nightfall_core)

target_include_directories(nightfall_core_tests
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/doctest
)

add_test(NAME nightfall_core_tests COMMAND nightfall_core_tests)

add_executable(nightfall_core_sanitizer_smoke
  core/nightfall.sanitizer.cpp
)

target_link_libraries(nightfall_core_sanitizer_smoke PRIVATE nightfall_core)
