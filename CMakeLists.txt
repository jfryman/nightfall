cmake_minimum_required(VERSION 3.24)

project(Nightfall LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(musashi STATIC
  third_party/musashi/m68kcpu.c
  third_party/musashi/m68kops.c
  third_party/musashi/softfloat/softfloat.c
)

target_include_directories(musashi
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/musashi
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/core
)

target_compile_definitions(musashi
  PRIVATE
    MUSASHI_CNF="NightfallMusashiConfig.h"
)

target_compile_options(musashi PRIVATE -w)

add_library(nightfall_core STATIC
  core/M68KRuntime.cpp
  core/ResourceFork.cpp
  core/ToolboxManagers.cpp
  core/QuickDrawPort.cpp
  core/QuickDrawBitTransfer.cpp
  core/QuickDrawLine.cpp
  core/QuickDrawPicture.cpp
  core/QuickDrawRegion.cpp
  core/QuickDrawRect.cpp
  core/nightfall.cpp
)

target_include_directories(nightfall_core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/core
)

target_link_libraries(nightfall_core PUBLIC musashi)

target_compile_options(nightfall_core PRIVATE
  -Wall
  -Wextra
  -Wpedantic
)

enable_testing()

add_executable(nightfall_core_tests
  core/M68KRuntime.test.cpp
  core/ResourceFork.test.cpp
  core/ToolboxManagers.test.cpp
  core/QuickDrawPort.test.cpp
  core/QuickDrawBitTransfer.test.cpp
  core/QuickDrawLine.test.cpp
  core/QuickDrawPicture.test.cpp
  core/QuickDrawRegion.test.cpp
  core/QuickDrawRect.test.cpp
  core/nightfall.test.cpp
)

target_link_libraries(nightfall_core_tests PRIVATE nightfall_core)

add_executable(tbtrace
  tools/tbtrace/main.cpp
)

target_link_libraries(tbtrace PRIVATE nightfall_core)

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

option(NIGHTFALL_ENABLE_FUZZERS "Build Nightfall libFuzzer targets" ON)

if(NIGHTFALL_ENABLE_FUZZERS AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_executable(fuzz-pict EXCLUDE_FROM_ALL
    core/QuickDrawPicture.fuzz.cpp
  )
  target_link_libraries(fuzz-pict PRIVATE nightfall_core)
  target_compile_options(fuzz-pict PRIVATE -fsanitize=fuzzer)
  target_link_options(fuzz-pict PRIVATE -fsanitize=fuzzer)
endif()
