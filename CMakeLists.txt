# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.20)
# cafesf2 --version reports this version. The file properties of the Windows program and the Info.plist of the macOS
# app show it too. A release changes it here and adds its notes in docs/release-notes/.
project(cafesf2 VERSION 1.3.2 LANGUAGES CXX)

# The submodules: seqlib writes the MIDI, SoundFont and SFZ files. ntdlib contains the Nintendo code that cafesf2 shares
# with the other converters and the players: the archive reader, the performer, the conversion and the command-line
# frame (ntdlib::sequence_export). xsflib contains the common code that ntdlib builds on, and the CMake helpers.
# xsflib's XsflibProject.cmake sets the C++ standard, a release build by default, the static C runtime and the compiler
# options that keep the output the same on every platform. ntdlib builds on seqlib and xsflib. xsflib_project adds the
# libraries in that order, after it turns on NTDLIB_SEQUENCE_EXPORT, ntdlib's conversion of sequences. It stops if a
# submodule is missing. The check below covers xsflib itself. xsflib_project also turns off clang-cl's
# -Wmissing-designated-field-initializers for every target, the libraries too. A designated initializer names only the
# fields that differ from their defaults.
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/externals/xsflib/CMakeLists.txt")
  message(FATAL_ERROR "externals/xsflib is missing. Run: git submodule update --init")
endif()
include(externals/xsflib/cmake/XsflibProject.cmake)
xsflib_project(WARNINGS_ON_EVERY_TARGET ENABLE NTDLIB_SEQUENCE_EXPORT LIBRARIES seqlib xsflib ntdlib)

# The libraries that cafesf2 and its tests link. ntdlib's sequence_export/consoles.h contains the Wii U's settings.
# ntdlib::cafe_sequence_starts contains the starts of the game sequences. The program also links ntdlib's conversion of
# JAudio games (sequence_export_jaudio).
add_library(cafesf2_core INTERFACE)
target_link_libraries(cafesf2_core INTERFACE seqlib::seqlib ntdlib::sar ntdlib::sequence_export
                      ntdlib::cafe_sequence_starts)

add_executable(cafesf2 src/main.cpp src/profiles.cpp)
target_link_libraries(cafesf2 PRIVATE cafesf2_core ntdlib::sequence_export_jaudio)
xsflib_add_version(cafesf2 VERSION ${PROJECT_VERSION} DEFINE CAFESF2_VERSION
                   DESCRIPTION "Nintendo Wii U sound archive converter" PRODUCT cafesf2
                   COPYRIGHT "Copyright (c) 2026 the cafesf2 authors. MIT licence.")

# Windows: UTF-8 as the code page. An archive or a folder whose name the system's code page can't hold then opens.
xsflib_add_utf8_manifest(cafesf2)

# Finder cannot drop files onto a command-line executable. Package it in a macOS droplet app, with the version in its
# Info.plist.
xsflib_add_droplet_tool(cafesf2 APP_NAME Cafesf2 VERSION ${PROJECT_VERSION}
                        DESCRIPTION "Drop Wii U sound archives (.bfsar) or JAudio start-up archives (.baa, .aaf) here to convert them to MIDI, SoundFont and SFZ. Output goes in a folder beside each archive."
                        PROMPT "Choose Wii U sound archives (.bfsar) or JAudio start-up archives (.baa, .aaf) to convert:" TRUNCATED GROUPS)

include(CTest)
if(BUILD_TESTING)
  # The program reports its version, and it rejects --var 32=1. -DCAFESF2_TEST_ARCHIVE=path/to/archive.bfsar also
  # converts and checks every sequence of an archive with tests/cafesf2_test.cpp. GENERATED_FILES and GENERATED_ARCHIVE
  # add the conversion of a Wii U game that plays its music with JAudio, from the game's start-up archive with its other
  # sound files beside it. `cafesf2_test --jaudio` writes them. xsflib's XsflibApps.cmake sets the tests' pass and fail
  # expressions.
  xsflib_add_converter_tests(cafesf2 EXTENSION .bfsar VERSION ${PROJECT_VERSION}
                             LIBRARIES cafesf2_core ntdlib::testing GENERATED_FILES jaudio GENERATED_ARCHIVE test.baa)

  # The starts of the sequences (src/profiles.h): how a start and an arrangement set the variables and the tracks of
  # the performance.
  add_executable(cafesf2_profiles_test tests/profiles_test.cpp src/profiles.cpp)
  target_include_directories(cafesf2_profiles_test PRIVATE src)
  target_link_libraries(cafesf2_profiles_test PRIVATE cafesf2_core ntdlib::testing)
  add_test(NAME cafesf2_profiles COMMAND cafesf2_profiles_test)

  # Load local_tests.cmake from each immediate subdirectory to register tests that use locally supplied files.
  xsflib_include_local_tests()
endif()
