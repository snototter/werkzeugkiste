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


#########################################################################################
# ---- Internal logging utils: werkzeugkiste::logging ----
# Library
add_library(werkzeugkiste-logging INTERFACE)
add_library(werkzeugkiste::logging ALIAS werkzeugkiste-logging)

if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(werkzeugkiste-logging INTERFACE WERKZEUGKISTE_STATIC_DEFINE)
endif()

set_target_properties(
    werkzeugkiste-logging PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    EXPORT_NAME logging
    OUTPUT_NAME werkzeugkiste-logging
    LINKER_LANGUAGE CXX
)

target_include_directories(
    werkzeugkiste-logging ${warning_guard}
    INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_include_directories(
    werkzeugkiste-logging SYSTEM
    INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
)

target_compile_features(werkzeugkiste-logging INTERFACE cxx_std_17)

find_package(spdlog QUIET)
if(spdlog_FOUND)
    message(STATUS "[werkzeugkiste] Logging enabled via spdlog.")
    if(TARGET spdlog::spdlog_header_only)
        set(werkzeugkiste_SPDLOG_TARGET spdlog::spdlog_header_only)
    endif()
    # Then, replace the target, if the compiled spdlog version is available
    if(TARGET spdlog::spdlog)
        set(werkzeugkiste_SPDLOG_TARGET spdlog::spdlog)
    endif()
    target_link_libraries(werkzeugkiste-logging INTERFACE ${werkzeugkiste_SPDLOG_TARGET})
    target_compile_definitions(werkzeugkiste-logging INTERFACE werkzeugkiste_WITH_SPDLOG)
else(spdlog_FOUND)
    message(STATUS "[werkzeugkiste] Logging disabled, spdlog not found.")
endif(spdlog_FOUND)


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
add_library(werkzeugkiste-container INTERFACE)
#add_library(werkzeugkiste-container
#    ${wkzgcontainer_PUBLIC_HEADER_FILES}
#)
# Namespace'd alias
add_library(werkzeugkiste::container ALIAS werkzeugkiste-container)

if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(werkzeugkiste-container INTERFACE WERKZEUGKISTE_STATIC_DEFINE)
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
    INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_include_directories(
    werkzeugkiste-container SYSTEM
    INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
)

target_compile_features(werkzeugkiste-container INTERFACE cxx_std_17)


#########################################################################################
# ---- Math/Geometry Utils: werkzeugkiste::geometry ----
# Header files
set(wkzggeometry_PUBLIC_HEADER_FILES
    include/werkzeugkiste/geometry/geometry.h
    include/werkzeugkiste/geometry/utils.h
    include/werkzeugkiste/geometry/vector.h
    include/werkzeugkiste/geometry/projection.h
    include/werkzeugkiste/geometry/camera.h
    include/werkzeugkiste/geometry/primitives.h
    include/werkzeugkiste/logging.h
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

find_package(Eigen3 REQUIRED NO_MODULE)
target_link_libraries(werkzeugkiste-geometry PUBLIC Eigen3::Eigen)
target_link_libraries(werkzeugkiste-geometry PRIVATE werkzeugkiste::logging)

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
# Header files
set(wkzgfiles_PUBLIC_HEADER_FILES
    include/werkzeugkiste/files/fileio.h
    include/werkzeugkiste/files/filesys.h
    include/werkzeugkiste/logging.h
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


#TODO 01/23 switched back to pre-17 solution
# with the CMake Community FindFilesystem module, library and examples
# could be build, but the test suite not. Simply adding -lstdc++fs to the 
# test suite linkage didn't solve this issue - need to investigate deeper
# and thus, I prefer to use the fallback
## There is no easily-portable way to link against std::filesystem
## Thankfully, the CMake Community Modules provide a workaround.
## Watch https://gitlab.kitware.com/cmake/cmake/-/issues/17834 for
## potential solution in the future.
##list(PREPEND CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})
#list(PREPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
##set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
## find_package(Filesystem QUIET)

target_link_libraries(
    werkzeugkiste-files
    PRIVATE
    werkzeugkiste::strings
    werkzeugkiste::logging
)

## if(Filesystem_FOUND)
##     message(STATUS "TODO stdc++fs supported, linking against unofficial std::filesystem target")
##     target_link_libraries(
##         werkzeugkiste-files
##         PRIVATE
##         std::filesystem
##     )
##     target_compile_definitions(
##         werkzeugkiste-files PRIVATE WZK_HAS_FILESYSTEM
##     )
## else()
##   message(STATUS "TODO stdc++fs support not found - might need to link to experimental/other extensions/... - using fallback solution (for linux only)")
## endif()


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
# ---- Configuration Utils: werkzeugkiste::config ----
# Requires toml++ for TOML support
include(FetchContent)
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG        v3.2.0
)
FetchContent_MakeAvailable(tomlplusplus)

# Header files
set(wkzgconfig_HEADER_FILES
    include/werkzeugkiste/config/configuration.h
    include/werkzeugkiste/logging.h
    ${tomlplusplus_SOURCE_DIR}/include/toml++/toml.h
    ${werkzeugkiste_VERSION_HEADER}
)
# Source files
set(wkzgconfig_SOURCE_FILES
    src/config/configuration.cpp
)
# Library
add_library(werkzeugkiste-config
    ${wkzgconfig_HEADER_FILES}
    ${wkzgconfig_SOURCE_FILES}
)
# Namespace'd alias
add_library(werkzeugkiste::config ALIAS werkzeugkiste-config)

#find_package(Eigen3 REQUIRED NO_MODULE)
#TODO make optional (load matrix) target_link_libraries(werkzeugkiste-config PUBLIC Eigen3::Eigen)
target_link_libraries(
    werkzeugkiste-config
    PRIVATE
    werkzeugkiste::logging
    werkzeugkiste::strings
    werkzeugkiste::files
)

set_target_properties(
    werkzeugkiste-config PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    EXPORT_NAME config
    OUTPUT_NAME werkzeugkiste-config
    LINKER_LANGUAGE CXX
)

target_include_directories(
    werkzeugkiste-config ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${tomlplusplus_SOURCE_DIR}/include>"
)

target_include_directories(
    werkzeugkiste-config SYSTEM
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
)

target_compile_features(werkzeugkiste-config PUBLIC cxx_std_17)

#########################################################################################
# ---- The "all-in-one" target ----
#
## TODO does the all-in-one target need the other headers explicitly? they should be implicitly
## deployed by the corresponding sub-utilitiy target (?)
add_library(werkzeugkiste-werkzeugkiste INTERFACE)
#add_library(werkzeugkiste-werkzeugkiste
#    ${wkzgcontainer_PUBLIC_HEADER_FILES}
#    ${wkzggeometry_PUBLIC_HEADER_FILES}
#    ${wkzgstrings_PUBLIC_HEADER_FILES}
#    ${wkzgfiles_PUBLIC_HEADER_FILES}
#    ${wkzgtiming_PUBLIC_HEADER_FILES}
#    ${werkzeugkiste_VERSION_HEADER}
#)
# ---- Namespace'd alias ----
add_library(werkzeugkiste::werkzeugkiste ALIAS werkzeugkiste-werkzeugkiste)

# ---- Generate the shared export header to be used by all targets ----
include(GenerateExportHeader)
generate_export_header(
    werkzeugkiste-geometry  # Requires a "full" library target (doesn't work with INTERFACE)
    BASE_NAME werkzeugkiste
    EXPORT_FILE_NAME export/werkzeugkiste/werkzeugkiste_export.h
    CUSTOM_CONTENT_FROM_VARIABLE pragma_suppress_c4251
)

if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(
        werkzeugkiste-werkzeugkiste INTERFACE WERKZEUGKISTE_STATIC_DEFINE
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
    INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_include_directories(
    werkzeugkiste-werkzeugkiste SYSTEM
    INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
)

target_compile_features(werkzeugkiste-werkzeugkiste INTERFACE cxx_std_17)

target_link_libraries(
    werkzeugkiste-werkzeugkiste
    INTERFACE
    werkzeugkiste::config
    werkzeugkiste::container
    werkzeugkiste::files
    werkzeugkiste::geometry
    werkzeugkiste::strings
    werkzeugkiste::timing
    werkzeugkiste::logging
)



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
