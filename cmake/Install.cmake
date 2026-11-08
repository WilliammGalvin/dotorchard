include(CMakePackageConfigHelpers)

set(ORCHARD_DOT_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/orchard-dot)

install(TARGETS orchard_dot
  EXPORT orchard-dot-targets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT orchard-dot-targets
  NAMESPACE orchard::
  FILE orchard-dot-targets.cmake
  DESTINATION ${ORCHARD_DOT_CMAKE_DIR})

configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/orchard-dot-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/orchard-dot-config.cmake
  INSTALL_DESTINATION ${ORCHARD_DOT_CMAKE_DIR})

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/orchard-dot-config-version.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMinorVersion)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/orchard-dot-config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/orchard-dot-config-version.cmake
  DESTINATION ${ORCHARD_DOT_CMAKE_DIR})
