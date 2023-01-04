cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

# ---- Load the library version ----

if(NOT werkzeugkiste_VERSION)
    file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/VERSION" werkzeugkiste_VERSION)
endif()

# ---- Set up the project ----

project(
    werkzeugkiste
    VERSION ${werkzeugkiste_VERSION}
    DESCRIPTION "Collection of frequently used C++ utility snippets."
    HOMEPAGE_URL "https://github.com/snototter/werkzeugkiste"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

#########################################################################################
# ---- Declare libraries ----
include(GenerateExportHeader)

#########################################################################################
# Container Utils: werkzeugkiste::container
# Header files
set(wkzgcontainer_PUBLIC_HEADER_FILES
    include/werkzeugkiste/container/circular_buffer.h
    include/werkzeugkiste/container/sort.h
    include/werkzeugkiste/container/math.h)
# Library
add_library(werkzeugkiste-container
    ${wkzgcontainer_PUBLIC_HEADER_FILES})
# Add a namespace'd alias
add_library(werkzeugkiste::container ALIAS werkzeugkiste-container)
# Static build
if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(werkzeugkiste-container PUBLIC WERKZEUGKISTE_STATIC_DEFINE)
endif()

set_target_properties(
    werkzeugkiste-container PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    EXPORT_NAME container
    OUTPUT_NAME container
    LINKER_LANGUAGE CXX
)

target_include_directories(
    werkzeugkiste-container ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_include_directories(
    werkzeugkiste-container SYSTEM
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
)

target_compile_features(werkzeugkiste-container PUBLIC cxx_std_17)


#########################################################################################
# Math/Geometry Utils: werkzeugkiste::geometry
find_package(Eigen3)
# Header files
set(wkzggeometry_PUBLIC_HEADER_FILES
    include/werkzeugkiste/geometry/geometry.h
    include/werkzeugkiste/geometry/utils.h
    include/werkzeugkiste/geometry/vector.h
    include/werkzeugkiste/geometry/projection.h
    include/werkzeugkiste/geometry/camera.h
    include/werkzeugkiste/geometry/primitives.h)
# Source files
set(wkzggeometry_SOURCE_FILES
    src/geometry/primitives.cpp
    src/geometry/vector.cpp)
# Library
add_library(werkzeugkiste-geometry
    ${wkzggeometry_PUBLIC_HEADER_FILES}
    ${wkzggeometry_SOURCE_FILES})
# Add a namespace'd alias
add_library(werkzeugkiste::geometry ALIAS werkzeugkiste-geometry)

set_target_properties(
    werkzeugkiste-geometry PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    EXPORT_NAME geometry
    OUTPUT_NAME geometry
    LINKER_LANGUAGE CXX
)

target_include_directories(
    werkzeugkiste-geometry ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_include_directories(
    werkzeugkiste-geometry SYSTEM
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
)

target_link_libraries(werkzeugkiste-geometry PUBLIC Eigen3::Eigen)

target_compile_features(werkzeugkiste-geometry PUBLIC cxx_std_17)


#########################################################################################
# String Utils: werkzeugkiste::strings
# Header files
set(wkzgstrings_PUBLIC_HEADER_FILES
    include/werkzeugkiste/strings/strings.h)
# Source files
set(wkzgstrings_SOURCE_FILES
    src/strings/strings.cpp)
# Library
add_library(werkzeugkiste-strings
    ${wkzgstrings_PUBLIC_HEADER_FILES}
    ${wkzgstrings_SOURCE_FILES})
# Add a namespace'd alias
add_library(werkzeugkiste::strings ALIAS werkzeugkiste-strings)

set_target_properties(
    werkzeugkiste-strings PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    EXPORT_NAME strings
    OUTPUT_NAME strings
    LINKER_LANGUAGE CXX
)

target_include_directories(
    werkzeugkiste-strings 
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)

target_include_directories(
    werkzeugkiste-strings ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_include_directories(
    werkzeugkiste-strings SYSTEM
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
)

target_compile_features(werkzeugkiste-strings PUBLIC cxx_std_17)


#########################################################################################
# File Utils: werkzeugkiste::files
# Must be declared after ::strings
# Header files
set(wkzgfiles_PUBLIC_HEADER_FILES
    include/werkzeugkiste/files/fileio.h
    include/werkzeugkiste/files/filesys.h)
# Source files
set(wkzgfiles_SOURCE_FILES
    src/files/fileio.cpp
    src/files/filesys.cpp)
# Library
add_library(werkzeugkiste-files
    ${wkzgfiles_PUBLIC_HEADER_FILES}
    ${wkzgfiles_SOURCE_FILES})
# Add a namespace'd alias
add_library(werkzeugkiste::files ALIAS werkzeugkiste-files)

set_target_properties(
    werkzeugkiste-files PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    EXPORT_NAME files
    OUTPUT_NAME files
    LINKER_LANGUAGE CXX
)

target_include_directories(
    werkzeugkiste-files ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_include_directories(
    werkzeugkiste-files SYSTEM
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
)

target_link_libraries(
    werkzeugkiste-files
    PRIVATE werkzeugkiste::strings
)

target_compile_features(werkzeugkiste-files PUBLIC cxx_std_17)


#########################################################################################
# Container Utils: werkzeugkiste::timing
# Header files
set(wkzgtiming_PUBLIC_HEADER_FILES
    include/werkzeugkiste/timing/stopwatch.h
    include/werkzeugkiste/timing/tictoc.h)
# Source files
set(wkzgtiming_SOURCE_FILES
    src/timing/stopwatch.cpp
    src/timing/tictoc.cpp)
# Library
add_library(werkzeugkiste-timing
    ${wkzgtiming_PUBLIC_HEADER_FILES}
    ${wkzgtiming_SOURCE_FILES})
# Add a namespace'd alias
add_library(werkzeugkiste::timing ALIAS werkzeugkiste-timing)
# Export header
generate_export_header(
    werkzeugkiste-timing
    BASE_NAME werkzeugkiste
    EXPORT_FILE_NAME export/werkzeugkiste/timing_export.hpp
    CUSTOM_CONTENT_FROM_VARIABLE pragma_suppress_c4251
)
# Static build
if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(werkzeugkiste-timing PUBLIC WERKZEUGKISTE_STATIC_DEFINE)
endif()

set_target_properties(
    werkzeugkiste-timing PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    EXPORT_NAME timing
    OUTPUT_NAME timing
    LINKER_LANGUAGE CXX
)

target_include_directories(
    werkzeugkiste-timing ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_include_directories(
    werkzeugkiste-timing SYSTEM
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
)

target_compile_features(werkzeugkiste-timing PUBLIC cxx_std_17)

#
#add_library(
#    werkzeugkiste_werkzeugkiste
#    source/werkzeugkiste.cpp
#)
#add_library(werkzeugkiste::werkzeugkiste ALIAS werkzeugkiste_werkzeugkiste)
#
#generate_export_header(
#    werkzeugkiste_werkzeugkiste
#    BASE_NAME werkzeugkiste
#    EXPORT_FILE_NAME export/werkzeugkiste/werkzeugkiste_export.hpp
#    CUSTOM_CONTENT_FROM_VARIABLE pragma_suppress_c4251
#)
#
#if(NOT BUILD_SHARED_LIBS)
#  target_compile_definitions(werkzeugkiste_werkzeugkiste PUBLIC WERKZEUGKISTE_STATIC_DEFINE)
#endif()
#
#set_target_properties(
#    werkzeugkiste_werkzeugkiste PROPERTIES
#    CXX_VISIBILITY_PRESET hidden
#    VISIBILITY_INLINES_HIDDEN YES
#    VERSION "${PROJECT_VERSION}"
#    SOVERSION "${PROJECT_VERSION_MAJOR}"
#    EXPORT_NAME werkzeugkiste
#    OUTPUT_NAME werkzeugkiste
#)
#
#target_include_directories(
#    werkzeugkiste_werkzeugkiste ${warning_guard}
#    PUBLIC
#    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
#)
#
#target_include_directories(
#    werkzeugkiste_werkzeugkiste SYSTEM
#    PUBLIC
#    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
#)
#
#target_compile_features(werkzeugkiste_werkzeugkiste PUBLIC cxx_std_17)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()


# ---- Add custom uninstall target ----

if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()


# ---- Examples ----

if(PROJECT_IS_TOP_LEVEL)
  option(BUILD_EXAMPLES "Build examples tree." "${werkzeugkiste_DEVELOPER_MODE}")
  if(BUILD_EXAMPLES)
    add_subdirectory(examples)
  endif()
endif()

# ---- Developer mode ----

if(NOT werkzeugkiste_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of werkzeugkiste"
  )
endif()

include(cmake/dev-mode.cmake)
