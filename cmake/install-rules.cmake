if(PROJECT_IS_TOP_LEVEL)
  set(
      CMAKE_INSTALL_INCLUDEDIR "include/werkzeugkiste-${PROJECT_VERSION}"
      CACHE PATH ""
  )
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package werkzeugkiste)

install(
    DIRECTORY
    include/
    "${PROJECT_BINARY_DIR}/export/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT werkzeugkiste_Development
)

install(
    TARGETS
      werkzeugkiste-container
      werkzeugkiste-geometry
      werkzeugkiste-strings
      werkzeugkiste-files
      werkzeugkiste-timing
    EXPORT werkzeugkisteTargets
    RUNTIME #
    COMPONENT werkzeugkiste_Runtime
    LIBRARY #
    COMPONENT werkzeugkiste_Runtime
    NAMELINK_COMPONENT werkzeugkiste_Development
    ARCHIVE #
    COMPONENT werkzeugkiste_Development
    INCLUDES #
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

# Allow package maintainers to freely override the path for the configs
set(
    werkzeugkiste_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${package}"
    CACHE PATH "CMake package config location relative to the install prefix"
)
mark_as_advanced(werkzeugkiste_INSTALL_CMAKEDIR)

install(
    FILES cmake/install-config.cmake
    DESTINATION "${werkzeugkiste_INSTALL_CMAKEDIR}"
    RENAME "${package}Config.cmake"
    COMPONENT werkzeugkiste_Development
)

install(
    FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION "${werkzeugkiste_INSTALL_CMAKEDIR}"
    COMPONENT werkzeugkiste_Development
)

install(
    EXPORT werkzeugkisteTargets
    NAMESPACE werkzeugkiste::
    DESTINATION "${werkzeugkiste_INSTALL_CMAKEDIR}"
    COMPONENT werkzeugkiste_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
