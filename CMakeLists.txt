# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.20)
# cafesf2 --version reports this version. The file properties of the Windows program and the Info.plist of the macOS
# app show it too. A release changes it here and adds its notes in docs/release-notes/.
project(cafesf2 VERSION 1.1.2 LANGUAGES CXX)

# The submodules: seqlib writes the MIDI, SoundFont and SFZ files. ntdlib holds the Nintendo code that cafesf2 shares
# with the other converters and the players: the archive reader, the performer, the conversion and the command-line
# frame (ntdlib::sequence_export). xsflib holds the common code that ntdlib builds on, and the CMake helpers. xsflib's
# XsflibProject.cmake sets the C++ standard, a release build by default, the static C runtime and the compiler options
# that keep the output the same on every platform. ntdlib builds on seqlib and xsflib. xsflib_add_libraries adds ntdlib
# last. XsflibProject.cmake defines that macro. The macro stops if a submodule is missing. The check below
# covers xsflib itself.
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/externals/xsflib/CMakeLists.txt")
  message(FATAL_ERROR "externals/xsflib is missing. Run: git submodule update --init")
endif()
include(externals/xsflib/cmake/XsflibProject.cmake)
xsflib_project(WARNINGS_ON_EVERY_TARGET)
set(NTDLIB_SEQUENCE_EXPORT ON)
xsflib_add_libraries(seqlib xsflib ntdlib)

# The libraries that cafesf2 and its tests link. ntdlib's sequence_export/consoles.h holds the Wii U's settings. The
# program also links ntdlib's conversion of JAudio games (sequence_export_jaudio).
add_library(cafesf2_core INTERFACE)
target_link_libraries(cafesf2_core INTERFACE seqlib::seqlib ntdlib::sar ntdlib::sequence_export)

add_executable(cafesf2 src/main.cpp)
target_link_libraries(cafesf2 PRIVATE cafesf2_core ntdlib::sequence_export_jaudio)
xsflib_add_version(cafesf2 VERSION ${PROJECT_VERSION} DEFINE CAFESF2_VERSION
                   DESCRIPTION "Nintendo Wii U sound archive converter" PRODUCT cafesf2
                   COPYRIGHT "Copyright (c) 2026 the cafesf2 authors. MIT licence.")

# Finder cannot drop files onto a command-line executable. Package it in a macOS droplet app, with the version in its
# Info.plist.
xsflib_add_droplet_tool(cafesf2 APP_NAME Cafesf2 VERSION ${PROJECT_VERSION}
                        DESCRIPTION "Drop Wii U sound archives (.bfsar) or JAudio start-up archives (.baa, .aaf) here to convert them to MIDI, SoundFont and SFZ. Output goes in a folder beside each archive."
                        PROMPT "Choose Wii U sound archives (.bfsar) or JAudio start-up archives (.baa, .aaf) to convert:" TRUNCATED GROUPS)

include(CTest)
if(BUILD_TESTING)
  # The program reports its version, and it rejects --var 32=1. -DCAFESF2_TEST_ARCHIVE=path/to/archive.bfsar also
  # converts and checks every sequence of an archive with tests/cafesf2_test.cpp.
  xsflib_add_converter_tests(cafesf2 EXTENSION .bfsar VERSION ${PROJECT_VERSION}
                             LIBRARIES cafesf2_core ntdlib::testing)

  # It converts a Wii U game that plays its music with JAudio, from the game's start-up archive with its other sound
  # files beside it. cafesf2_test writes them. ctest ignores the exit code of a test with a pass expression. The
  # conversion therefore also fails if cafesf2 reports a failure.
  set(cafesf2_jaudio ${CMAKE_CURRENT_BINARY_DIR}/jaudio)
  add_test(NAME cafesf2_jaudio_files COMMAND cafesf2_test --jaudio ${cafesf2_jaudio})
  add_test(NAME cafesf2_jaudio_archive COMMAND cafesf2 ${cafesf2_jaudio}/test.baa)
  set_tests_properties(cafesf2_jaudio_files PROPERTIES FIXTURES_SETUP cafesf2_jaudio)
  set_tests_properties(cafesf2_jaudio_archive PROPERTIES FIXTURES_REQUIRED cafesf2_jaudio
                       PASS_REGULAR_EXPRESSION "test\\.baa: 1 sequences -> .*test_cafesf2"
                       FAIL_REGULAR_EXPRESSION "can't read| [1-9][0-9]* failed")

  # Load local_tests.cmake from each immediate subdirectory to register tests that use locally supplied files.
  xsflib_include_local_tests()
endif()
