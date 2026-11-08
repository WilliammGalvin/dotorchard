cmake_minimum_required(VERSION 3.23)

project(orchard-dot
  VERSION 0.1.0
  DESCRIPTION "Parser, writer and schema validator for the .orchard format"
  LANGUAGES CXX)

option(ORCHARD_DOT_INSTALL "Generate install rules" ${PROJECT_IS_TOP_LEVEL})
option(ORCHARD_DOT_BUILD_EXAMPLES "Build orchard-dot examples" ${PROJECT_IS_TOP_LEVEL})

include(GNUInstallDirs)

add_library(orchard_dot)
add_library(orchard::dot ALIAS orchard_dot)

set_target_properties(orchard_dot PROPERTIES
  EXPORT_NAME dot
  CXX_EXTENSIONS OFF)

target_compile_features(orchard_dot PUBLIC cxx_std_23)

target_include_directories(orchard_dot PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

if(MSVC)
  target_compile_options(orchard_dot PRIVATE /W4 /permissive-)
else()
  target_compile_options(orchard_dot PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
endif()

add_subdirectory(src)

if(ORCHARD_DOT_BUILD_EXAMPLES)
  add_subdirectory(example)
endif()

if(ORCHARD_DOT_INSTALL)
  include(cmake/Install.cmake)
endif()
