# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.20)
# cafesf2 --version reports this version. The file properties of the Windows program and the Info.plist of the macOS
# app show it too. A release changes it here and adds its notes in docs/release-notes/.
project(cafesf2 VERSION 1.0.2 LANGUAGES CXX)

# seqlib writes the MIDI and SoundFont files. ntdlib reads Nintendo's sound formats and converts the sequences and
# instruments. xsflib holds the code that any PSF-family program could use, and the CMake functions that make the
# version resource and the app. All three are submodules. xsflib's XsflibProject.cmake sets the C++ standard, a release
# build by default, the static C runtime and the compiler options that keep the output the same on every platform.
# seqlib comes first: ntdlib's sequence_export uses it.
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/externals/xsflib/CMakeLists.txt")
  message(FATAL_ERROR "externals/xsflib is missing. Run: git submodule update --init")
endif()
include(externals/xsflib/cmake/XsflibProject.cmake)
xsflib_project(WARNINGS_ON_EVERY_TARGET)
set(NTDLIB_SEQUENCE_EXPORT ON)
xsflib_add_libraries(seqlib xsflib ntdlib)

# The libraries that cafesf2, its test and its tools link.
add_library(cafesf2_core INTERFACE)
target_link_libraries(cafesf2_core INTERFACE seqlib::seqlib ntdlib::sar ntdlib::sequence_export)

add_executable(cafesf2 src/main.cpp)
target_link_libraries(cafesf2 PRIVATE cafesf2_core)
xsflib_add_version(cafesf2 VERSION ${PROJECT_VERSION} DEFINE CAFESF2_VERSION
                   DESCRIPTION "Nintendo Wii U sound archive converter" PRODUCT cafesf2
                   COPYRIGHT "Copyright (c) 2026 the cafesf2 authors. MIT licence.")

# Finder cannot drop files onto a command-line executable. Package it in a macOS droplet app, with the version in its
# Info.plist.
xsflib_add_droplet_tool(cafesf2 APP_NAME Cafesf2 VERSION ${PROJECT_VERSION}
                         DESCRIPTION "Drop Wii U sound archives (.bfsar) here to convert them to MIDI, SoundFont and SFZ."
                         PROMPT "Choose Wii U sound archives (.bfsar) to convert:" TRUNCATED GROUPS)

include(CTest)
if(BUILD_TESTING)
  # The program reports its version, and it rejects --var 32=1. -DCAFESF2_TEST_ARCHIVE=path/to/archive.bfsar also
  # converts and checks every sequence of an archive with tests/cafesf2_test.cpp.
  xsflib_add_converter_tests(cafesf2 EXTENSION .bfsar VERSION ${PROJECT_VERSION}
                             LIBRARIES cafesf2_core ntdlib::testing)

  # Load local_tests.cmake from each immediate subdirectory to register tests that use locally supplied files.
  xsflib_include_local_tests()
endif()
