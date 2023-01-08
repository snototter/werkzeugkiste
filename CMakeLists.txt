cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

#########################################################################################
# ---- Load the library version from the file ----

if(NOT werkzeugkiste_VERSION)
    file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/VERSION" werkzeugkiste_VERSION)
endif()


#########################################################################################
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
# This toolbox consists of several "sub-utilities", each of which provides a separate
# library to be consumed. For the lazy, there is a single werkzeugkiste::werkzeugkiste
# target, which includes all "sub-utilities" in its link interface.

# ---- Auto-generate werkzeugkiste's version.h file ----
set(werkzeugkiste_VERSION_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/include/werkzeugkiste/version.h)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in
    ${werkzeugkiste_VERSION_HEADER}
)

#FIXME use the export header s.t. it can be used as a shared library!!!


#########################################################################################
# ---- Container Utils: werkzeugkiste::container ----

# Header files (this utility is header-only)
set(wkzgcontainer_PUBLIC_HEADER_FILES
    include/werkzeugkiste/container/circular_buffer.h
    include/werkzeugkiste/container/sort.h
    include/werkzeugkiste/container/math.h
    ${werkzeugkiste_VERSION_HEADER}
)
# Library
add_library(werkzeugkiste-container
    ${wkzgcontainer_PUBLIC_HEADER_FILES}
)
# Namespace'd alias
add_library(werkzeugkiste::container ALIAS werkzeugkiste-container)

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
    OUTPUT_NAME werkzeugkiste-container
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
# ---- Math/Geometry Utils: werkzeugkiste::geometry ----
# Adds a dependency to Eigen3::Eigen
find_package(Eigen3 REQUIRED NO_MODULE)
# Header files
set(wkzggeometry_PUBLIC_HEADER_FILES
    include/werkzeugkiste/geometry/geometry.h
    include/werkzeugkiste/geometry/utils.h
    include/werkzeugkiste/geometry/vector.h
    include/werkzeugkiste/geometry/projection.h
    include/werkzeugkiste/geometry/camera.h
    include/werkzeugkiste/geometry/primitives.h
    ${werkzeugkiste_VERSION_HEADER}
)
# Source files
set(wkzggeometry_SOURCE_FILES
    src/geometry/primitives.cpp
    src/geometry/vector.cpp
)
# Library
add_library(werkzeugkiste-geometry
    ${wkzggeometry_PUBLIC_HEADER_FILES}
    ${wkzggeometry_SOURCE_FILES}
)
# Namespace'd alias
add_library(werkzeugkiste::geometry ALIAS werkzeugkiste-geometry)

target_link_libraries(werkzeugkiste-geometry PUBLIC Eigen3::Eigen)

set_target_properties(
    werkzeugkiste-geometry PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    EXPORT_NAME geometry
    OUTPUT_NAME werkzeugkiste-geometry
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

target_compile_features(werkzeugkiste-geometry PUBLIC cxx_std_17)


#########################################################################################
# String Utils: werkzeugkiste::strings
# Header files
set(wkzgstrings_PUBLIC_HEADER_FILES
    include/werkzeugkiste/strings/strings.h
    ${werkzeugkiste_VERSION_HEADER}
)
# Source files
set(wkzgstrings_SOURCE_FILES
    src/strings/strings.cpp
)
# Library
add_library(werkzeugkiste-strings
    ${wkzgstrings_PUBLIC_HEADER_FILES}
    ${wkzgstrings_SOURCE_FILES}
)
# Namespace'd alias
add_library(werkzeugkiste::strings ALIAS werkzeugkiste-strings)

set_target_properties(
    werkzeugkiste-strings PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    EXPORT_NAME strings
    OUTPUT_NAME werkzeugkiste-strings
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
# ---- File Utils: werkzeugkiste::files ----
# ---- Note: Library must be declared after ::strings ----
#FIXME replace code by c++17 filesystem
# Header files
set(wkzgfiles_PUBLIC_HEADER_FILES
    include/werkzeugkiste/files/fileio.h
    include/werkzeugkiste/files/filesys.h
    ${werkzeugkiste_VERSION_HEADER}
)
# Source files
set(wkzgfiles_SOURCE_FILES
    src/files/fileio.cpp
    src/files/filesys.cpp
)
# Library
add_library(werkzeugkiste-files
    ${wkzgfiles_PUBLIC_HEADER_FILES}
    ${wkzgfiles_SOURCE_FILES}
)
# Namespace'd alias
add_library(werkzeugkiste::files ALIAS werkzeugkiste-files)

set_target_properties(
    werkzeugkiste-files PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    EXPORT_NAME files
    OUTPUT_NAME werkzeugkiste-files
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
# ---- Clock Utils: werkzeugkiste::timing ----
# Header files
set(wkzgtiming_PUBLIC_HEADER_FILES
    include/werkzeugkiste/timing/stopwatch.h
    include/werkzeugkiste/timing/tictoc.h
    ${werkzeugkiste_VERSION_HEADER}
)
# Source files
set(wkzgtiming_SOURCE_FILES
    src/timing/stopwatch.cpp
    src/timing/tictoc.cpp
)
# Library
add_library(werkzeugkiste-timing
    ${wkzgtiming_PUBLIC_HEADER_FILES}
    ${wkzgtiming_SOURCE_FILES}
)
# Namespace'd alias
add_library(werkzeugkiste::timing ALIAS werkzeugkiste-timing)

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
    OUTPUT_NAME werkzeugkiste-timing
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


#########################################################################################
# ---- The "all-in-one" target ----
# FIXME check if this works as intended!
# [ ] TODO automate these build checks!
# [x] worked for static build & fetch content example
# [ ] to test: shared build, fetch content
# [ ] to test: static build, local install
# [ ] to test: shared build, local install

# TODO does the all-in-one target need the other headers explicitly? they should be implicitly
# deployed by the corresponding sub-utilitiy target (?)
add_library(werkzeugkiste-werkzeugkiste
    ${wkzgcontainer_PUBLIC_HEADER_FILES}
    ${wkzggeometry_PUBLIC_HEADER_FILES}
    ${wkzgstrings_PUBLIC_HEADER_FILES}
    ${wkzgfiles_PUBLIC_HEADER_FILES}
    ${wkzgtiming_PUBLIC_HEADER_FILES}
    ${werkzeugkiste_VERSION_HEADER}
)
# ---- Namespace'd alias ----
add_library(werkzeugkiste::werkzeugkiste ALIAS werkzeugkiste-werkzeugkiste)

# ---- Generate the shared export header to be used by all targets ----
include(GenerateExportHeader)
generate_export_header(
    werkzeugkiste-werkzeugkiste
    BASE_NAME werkzeugkiste
    EXPORT_FILE_NAME export/werkzeugkiste/werkzeugkiste_export.h
    CUSTOM_CONTENT_FROM_VARIABLE pragma_suppress_c4251
)

if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(
        werkzeugkiste-werkzeugkiste PUBLIC WERKZEUGKISTE_STATIC_DEFINE
    )
endif()

set_target_properties(
    werkzeugkiste-werkzeugkiste PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    EXPORT_NAME werkzeugkiste
    OUTPUT_NAME werkzeugkiste-werkzeugkiste
    LINKER_LANGUAGE CXX
)

target_include_directories(
    werkzeugkiste-werkzeugkiste ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_include_directories(
    werkzeugkiste-werkzeugkiste SYSTEM
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
)

target_compile_features(werkzeugkiste-werkzeugkiste PUBLIC cxx_std_17)

target_link_libraries(
    werkzeugkiste-werkzeugkiste
    PUBLIC
    werkzeugkiste::container
    werkzeugkiste::files
    werkzeugkiste::geometry
    werkzeugkiste::strings
    werkzeugkiste::timing
)



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
#    EXPORT_FILE_NAME export/werkzeugkiste/werkzeugkiste_export.h
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


#########################################################################################
# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
    include(cmake/install-rules.cmake)
endif()


#########################################################################################
# ---- Add custom uninstall target ----

if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

#########################################################################################
# ---- Include optional examples ----

if(PROJECT_IS_TOP_LEVEL)
    option(BUILD_EXAMPLES "Build examples tree." "${werkzeugkiste_DEVELOPER_MODE}")
    if(BUILD_EXAMPLES)
        add_subdirectory(examples)
    endif()
else()
    # Let the consumer know which version of this library they are using
    set(werkzeugkiste_VERSION ${werkzeugkiste_VERSION} PARENT_SCOPE)
endif()


#########################################################################################
# ---- Developer mode ----

if(NOT werkzeugkiste_DEVELOPER_MODE)
    return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
    message(
        AUTHOR_WARNING
        "Developer mode is only intended for developers of werkzeugkiste"
    )
endif()

include(cmake/dev-mode.cmake)
