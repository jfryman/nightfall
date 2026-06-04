cmake_minimum_required(VERSION 3.24)

project(Nightfall LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(nightfall_core STATIC
  core/QuickDrawPort.cpp
  core/QuickDrawLine.cpp
  core/QuickDrawRect.cpp
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
  core/QuickDrawPort.test.cpp
  core/QuickDrawLine.test.cpp
  core/QuickDrawRect.test.cpp
  core/nightfall.test.cpp
)

target_link_libraries(nightfall_core_tests PRIVATE nightfall_core)

if(NOT DEFINED NIGHTFALL_DOCTEST_INCLUDE_DIR)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/doctest/doctest.h")
    set(NIGHTFALL_DOCTEST_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/doctest")
  elseif(EXISTS "/opt/homebrew/include/doctest/doctest.h")
    set(NIGHTFALL_DOCTEST_INCLUDE_DIR "/opt/homebrew/include/doctest")
  elseif(EXISTS "/usr/local/include/doctest/doctest.h")
    set(NIGHTFALL_DOCTEST_INCLUDE_DIR "/usr/local/include/doctest")
  else()
    message(FATAL_ERROR "doctest.h not found; install doctest or vendor third_party/doctest/doctest.h")
  endif()
endif()

target_include_directories(nightfall_core_tests
  PRIVATE
    ${NIGHTFALL_DOCTEST_INCLUDE_DIR}
)

add_test(NAME nightfall_core_tests COMMAND nightfall_core_tests)

add_executable(nightfall_core_sanitizer_smoke
  core/nightfall.sanitizer.cpp
)

target_link_libraries(nightfall_core_sanitizer_smoke PRIVATE nightfall_core)
