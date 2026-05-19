
cmake_minimum_required(VERSION 3.15)

project(
  NanoExif
  VERSION 0.1.0
  LANGUAGES CXX)

string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWERCASE)
string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPERCASE)

# ---------------------------------------------------------------------------- #
#                              set project options                             #
# ---------------------------------------------------------------------------- #

option(${PROJECT_NAME_UPPERCASE}_WARNINGS_AS_ERRORS "Treat compiler warnings as errors." OFF)
option(${PROJECT_NAME_UPPERCASE}_EXAMPLES_ENABLED "Whether build examples." OFF)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()
message(STATUS "Started CMake for NanoExif v${PROJECT_VERSION}...")

# ---------------------------------------------------------------------------- #
#                             setup compile options                            #
# ---------------------------------------------------------------------------- #

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  add_compile_options(/utf-8)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-finput-charset=UTF-8)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/CompilerWarnings.cmake)

message(STATUS "Applied compiler warnings. Using standard C++${CMAKE_CXX_STANDARD}.")

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

# ---------------------------------------------------------------------------- #
#                                 setup sources                                #
# ---------------------------------------------------------------------------- #

add_library(NanoExif INTERFACE)

target_include_directories(
    NanoExif
    INTERFACE
    $<INSTALL_INTERFACE:src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

set_target_properties(
  NanoExif
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE}"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE}"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}"
)

add_library(NanoExif::NanoExif ALIAS NanoExif)
message(STATUS "Library is now aliased as NanoExif::NanoExif.")

if(${PROJECT_NAME_UPPERCASE}_EXAMPLES_ENABLED)
  message(STATUS "Building examples...")
  add_subdirectory(examples)
endif()

# ---------------------------------------------------------------------------- #
#                                install targets                               #
# ---------------------------------------------------------------------------- #

include(GNUInstallDirs)
install(
  TARGETS
  NanoExif
  EXPORT
  NanoExifTargets
  LIBRARY DESTINATION
  ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION
  ${CMAKE_INSTALL_BINDIR}
  ARCHIVE DESTINATION
  ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION
  include
  PUBLIC_HEADER DESTINATION
  include
)

install(
  EXPORT
  NanoExifTargets
  FILE
  NanoExifTargets.cmake
  NAMESPACE
  NanoExif::
  DESTINATION
  ${CMAKE_INSTALL_LIBDIR}/cmake/NanoExif
)

configure_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/version.h.in
  include/${PROJECT_NAME_LOWERCASE}/version.h
  @ONLY
)

install(
  FILES
  ${CMAKE_CURRENT_BINARY_DIR}/include/${PROJECT_NAME_LOWERCASE}/version.h
  DESTINATION
  include/${PROJECT_NAME_LOWERCASE}
)

install(
  DIRECTORY
  src/${PROJECT_NAME_LOWERCASE}
  DESTINATION
  include
)

message(STATUS "Install targets successfully build.")

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/NanoExifConfigVersion.cmake
  VERSION
  ${PROJECT_VERSION}
  COMPATIBILITY
  SameMajorVersion
)

configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/NanoExifConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/NanoExifConfig.cmake
  INSTALL_DESTINATION
  ${CMAKE_INSTALL_LIBDIR}/cmake/NanoExif
)

install(
  FILES
  ${CMAKE_CURRENT_BINARY_DIR}/NanoExifConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/NanoExifConfigVersion.cmake
  DESTINATION
  ${CMAKE_INSTALL_LIBDIR}/cmake/NanoExif
)
