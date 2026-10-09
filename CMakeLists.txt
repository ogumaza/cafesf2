# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.20)
# cafesf2 --version reports this version. The file properties of the Windows program and the Info.plist of the macOS
# app show it too. A release changes it here and adds its notes in docs/release-notes/.
project(cafesf2 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR AND NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()
if(MSVC)
  add_compile_options(/W4 /utf-8)
  # Link the runtime statically so the drag-and-drop executable runs without runtime DLLs.
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
  # Disable FMA contraction. Clang enables it by default on CPUs such as Apple Silicon. Without it, results are the
  # same on every platform and centre pan is exactly zero (pan.h).
  add_compile_options(-Wall -Wextra -ffp-contract=off)
  # Use SSE2 on 32-bit x86 to avoid x87's extra precision. Results then match 64-bit x86.
  if(CMAKE_SIZEOF_VOID_P EQUAL 4 AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(i.86|x86|x86_64|AMD64)$")
    add_compile_options(-msse2 -mfpmath=sse)
  endif()
endif()

add_library(cafesf2_core STATIC
  src/fsar.cpp
  src/wave.cpp
  src/instruments.cpp
  src/envelope.cpp
  src/pitch.cpp
  src/filter.cpp
  src/performance.cpp
  src/midi.cpp
  src/sf2.cpp
  src/sfz.cpp
  src/convert.cpp)
target_include_directories(cafesf2_core PUBLIC src)

add_executable(cafesf2 src/main.cpp)
target_link_libraries(cafesf2 PRIVATE cafesf2_core)
target_compile_definitions(cafesf2 PRIVATE CAFESF2_VERSION="${PROJECT_VERSION}")
if(WIN32)
  target_link_libraries(cafesf2 PRIVATE shell32) # CommandLineToArgvW
  # A version resource. Explorer shows its version and description in the file's properties.
  configure_file(platform/windows/version.rc.in version.rc @ONLY)
  target_sources(cafesf2 PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/version.rc")
endif()

# Finder cannot drop files onto a command-line executable. Package it in a macOS droplet app, with the version in its
# Info.plist.
if(APPLE)
  find_program(OSACOMPILE osacompile)
  find_program(CODESIGN codesign)
  find_program(PLUTIL plutil)
  if(OSACOMPILE AND CODESIGN AND PLUTIL)
    set(app "${CMAKE_CURRENT_BINARY_DIR}/Cafesf2.app")
    set(app_script "${PROJECT_SOURCE_DIR}/platform/macos/Cafesf2.applescript")
    add_custom_command(
      OUTPUT "${app}/Contents/Resources/cafesf2"
      COMMAND "${CMAKE_COMMAND}" -E rm -rf "${app}"
      COMMAND "${OSACOMPILE}" -o "${app}" "${app_script}"
      COMMAND "${PLUTIL}" -replace CFBundleShortVersionString -string "${PROJECT_VERSION}" "${app}/Contents/Info.plist"
      COMMAND "${PLUTIL}" -replace CFBundleVersion -string "${PROJECT_VERSION}" "${app}/Contents/Info.plist"
      COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:cafesf2>" "${app}/Contents/Resources/cafesf2"
      COMMAND "${CODESIGN}" --force --deep --sign - "${app}"
      DEPENDS cafesf2 "${app_script}"
      COMMENT "Building Cafesf2.app"
      VERBATIM)
    add_custom_target(cafesf2_app ALL DEPENDS "${app}/Contents/Resources/cafesf2")
  endif()
endif()

include(CTest)
if(BUILD_TESTING)
  add_executable(cafesf2_test tests/cafesf2_test.cpp)
  target_link_libraries(cafesf2_test PRIVATE cafesf2_core)
  add_test(NAME cafesf2_units COMMAND cafesf2_test)

  # The program reports the version it was built as.
  string(REPLACE "." "\\." version_pattern "${PROJECT_VERSION}")
  add_test(NAME cafesf2_version COMMAND cafesf2 --version)
  set_tests_properties(cafesf2_version PROPERTIES PASS_REGULAR_EXPRESSION "^cafesf2 ${version_pattern}[\r\n]*$")

  # It rejects a variable that the game doesn't have.
  add_test(NAME cafesf2_var_range COMMAND cafesf2 --var 32=1 missing.bfsar)
  set_tests_properties(cafesf2_var_range PROPERTIES PASS_REGULAR_EXPRESSION "--var 32=1 is out of range")
  # Optionally convert and validate a complete archive:
  # -DCAFESF2_TEST_ARCHIVE=path/to/archive.bfsar
  set(CAFESF2_TEST_ARCHIVE "" CACHE FILEPATH "Sound archive (.bfsar) for the conversion test")
  if(CAFESF2_TEST_ARCHIVE AND EXISTS "${CAFESF2_TEST_ARCHIVE}")
    add_test(NAME cafesf2_archive COMMAND cafesf2_test "${CAFESF2_TEST_ARCHIVE}")
  endif()

  # Load local_tests.cmake from each immediate subdirectory to register tests that use locally supplied files.
  file(GLOB local_tests CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/*/local_tests.cmake")
  foreach(file IN LISTS local_tests)
    include("${file}")
  endforeach()
endif()
