cmake_minimum_required(VERSION 3.15)
# If you're familiar with CMake, you a) know how to replace the following features
# I want/need with functionality available in previous versions and b) you should
# have a much more recent CMake installed anyhow.
#
# CMake features I want/need here
# v3.14 FetchContent + make available
# v3.15 cmake --install

# Load the library version
if(NOT werkzeugkiste_VERSION)
    file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/VERSION" werkzeugkiste_VERSION)
endif()

# If a parent project includes this CMakeLists (e.g. if you integrate werkzeugkiste
# via CMake's FetchContent), we want to pass the version explicitly to the parent scope.
# If we currently are the top level project, then skip this call to avoid
# an unnecessary CMake warning:
get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(HAS_PARENT)
    set(werkzeugkiste_VERSION ${werkzeugkiste_VERSION} PARENT_SCOPE)
endif()

# Set up the project
project(werkzeugkiste VERSION ${werkzeugkiste_VERSION} LANGUAGES CXX)

#TODO(documentation) Provide a library usage example which disables
# examples/tests (as this is not needed if we just want to use the library).
option(werkzeugkiste_BUILD_EXAMPLES "Build the examples." ON)
option(werkzeugkiste_BUILD_TESTS "Build the test suite." ON)
option(werkzeugkiste_INSTALL "Generate the install target." ON)


#TODO(implementation) Use logging, similar to viren2d
#set(werkzeugkiste_LOG_LEVEL "info" CACHE STRING "Select log level")
#set_property(CACHE werkzeugkiste_LOG_LEVEL PROPERTY STRINGS disable trace debug info warn error critical)


# Use the common Unix default installation paths (the GNUInstallDirs package
# should also provide sane paths on Windows, but I didn't check this yet):
include(GNUInstallDirs)
set(INSTALL_DIR_INCLUDE ${CMAKE_INSTALL_INCLUDEDIR})
set(INSTALL_DIR_LIB ${CMAKE_INSTALL_LIBDIR})
# A default search location for CMake config files is /usr/lib/cmake/<package-name>
set(INSTALL_DIR_CMAKE ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})


#TODO(library-usage-improvement) I could instead set CXX_STANDARD, etc *per target*
# Set compiler options for all targets
if(NOT CMAKE_CXX_STANDARD)
    # c++14 is required for std::exchange
    # c++17 for string_view/filesystem
    # anything else is c++11
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    # To ensure portability, turn off compiler-specific extensions
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()


#TODO(windows-support) check spdlog's CMakeLists (msvc vs cygwin vs msys)
if (MSVC)
    # warning level 4
    add_compile_options(/W4)
    # make sure __cplusplus is defined when using msvc and enable parallel build
    string(APPEND CMAKE_CXX_FLAGS " /Zc:__cplusplus /MP")
else()
    # be pedantic
    add_compile_options(-Wall -Wextra -pedantic)
    add_compile_options(
        "$<$<CONFIG:RELEASE>:-O3>"
        "$<$<CONFIG:DEBUG>:-g3>")
endif()


# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose Release or Debug" FORCE)
endif()


# Compile library with position-independent code (-fPIC)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


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

# Include directories for the different usage scenarios
target_include_directories(werkzeugkiste-geometry
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
           $<INSTALL_INTERFACE:${INSTALL_DIR_INCLUDE}>)

target_link_libraries(werkzeugkiste-geometry
    PUBLIC Eigen3::Eigen)


# Other properties
set_target_properties(werkzeugkiste-geometry PROPERTIES
# I need to specify the exported target name explicitly (without namespace prefix).
# Otherwise, the "installed-locally" example won't work (due to different
# target name in the installed werkzeugkisteTarget.cmake which is loaded by the
# generated werkzeugkisteConfig.cmake
    EXPORT_NAME geometry
    DEBUG_POSTFIX "d")
# Don't use PUBLIC_HEADER, because we have to install the header files manually
# to ensure that the folder hierarchy is created (CMake would simply copy all
# PUBLIC_HEADERs into the same destination directory).
#    PUBLIC_HEADER "${wkzggeometry_PUBLIC_HEADER_FILES}"
# Setting the output name is no longer needed (I renamed the targets properly)
#    OUTPUT_NAME werkzeugkiste-timing


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

# Include directories for the different usage scenarios
target_include_directories(werkzeugkiste-container
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
           $<INSTALL_INTERFACE:${INSTALL_DIR_INCLUDE}>)

# Other properties
set_target_properties(werkzeugkiste-container PROPERTIES
    EXPORT_NAME container
# To export a header only target, the linker needs to know the programming language
    LINKER_LANGUAGE CXX
    DEBUG_POSTFIX "d")


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

# Include directories for the different usage scenarios
target_include_directories(werkzeugkiste-strings
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
           $<INSTALL_INTERFACE:${INSTALL_DIR_INCLUDE}>)

# Other properties
set_target_properties(werkzeugkiste-strings PROPERTIES
    EXPORT_NAME strings
    DEBUG_POSTFIX "d")


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

# Include directories for the different usage scenarios
target_include_directories(werkzeugkiste-files
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
           $<INSTALL_INTERFACE:${INSTALL_DIR_INCLUDE}>)

target_link_libraries(werkzeugkiste-files
        PRIVATE werkzeugkiste::strings)

# Other properties
set_target_properties(werkzeugkiste-files PROPERTIES
    EXPORT_NAME files
    DEBUG_POSTFIX "d")


#########################################################################################
# Timing Utils: werkzeugkiste::timing

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

# Include directories for the different usage scenarios
target_include_directories(werkzeugkiste-timing
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
           $<INSTALL_INTERFACE:${INSTALL_DIR_INCLUDE}>)

# Other properties
set_target_properties(werkzeugkiste-timing PROPERTIES
    EXPORT_NAME timing
    DEBUG_POSTFIX "d")


#########################################################################################
# Installation & Exports

# Auto-generate werkzeugkiste's version.h file
set(werkzeugkiste_VERSION_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/include/werkzeugkiste/version.h)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in
    ${werkzeugkiste_VERSION_HEADER})


# Create the werkzeugkisteTargets export (this only registers
# the targets, CMake will not install it yet)
#TODO(new-library-extensions) Add export targets here.
install(TARGETS
    werkzeugkiste-geometry
    werkzeugkiste-container
    werkzeugkiste-files
    werkzeugkiste-strings
    werkzeugkiste-timing
    EXPORT werkzeugkisteTargets
    LIBRARY DESTINATION ${INSTALL_DIR_LIB}
    ARCHIVE DESTINATION ${INSTALL_DIR_LIB})
# We don't have any installable executables, so we don't need to set
#    RUNTIME DESTINATION ${INSTALL_DIR_BIN}
# We can't use the PUBLIC_HEADER destination because all registered headers
# will be copied (flattened) directly into this destination
#    PUBLIC_HEADER DESTINATION ${INSTALL_DIR_INCLUDE}/werkzeugkiste)
# Since we already defined the PUBLIC include directories for our target,
# the INCLUDES DESTINATION should also not be specified. Doing this would not
# result in an error, but would simply cause a duplicated path entry in the
# generated .cmake file
#    INCLUDES DESTINATION ${INSTALL_INCLUDE_DIR}

# Generate the export targets for the build tree
# https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html#exporting-targets-from-the-build-tree
export(EXPORT werkzeugkisteTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/werkzeugkisteTargets.cmake"
    NAMESPACE werkzeugkiste::)

# If installation is requested:
if(werkzeugkiste_INSTALL)
    #TODO(new-library-extensions) Install header files per target
    install(FILES ${werkzeugkiste_VERSION_HEADER}      DESTINATION ${INSTALL_DIR_INCLUDE}/werkzeugkiste)
    install(FILES ${wkzgcontainer_PUBLIC_HEADER_FILES} DESTINATION ${INSTALL_DIR_INCLUDE}/werkzeugkiste/container)
    install(FILES ${wkzgfiles_PUBLIC_HEADER_FILES}     DESTINATION ${INSTALL_DIR_INCLUDE}/werkzeugkiste/files)
    install(FILES ${wkzggeometry_PUBLIC_HEADER_FILES}  DESTINATION ${INSTALL_DIR_INCLUDE}/werkzeugkiste/geometry)
    install(FILES ${wkzgstrings_PUBLIC_HEADER_FILES}   DESTINATION ${INSTALL_DIR_INCLUDE}/werkzeugkiste/strings)
    install(FILES ${wkzgtiming_PUBLIC_HEADER_FILES}    DESTINATION ${INSTALL_DIR_INCLUDE}/werkzeugkiste/timing)

    # Where to export the targets:
    install(EXPORT werkzeugkisteTargets
        FILE werkzeugkisteTargets.cmake
        NAMESPACE werkzeugkiste::
        DESTINATION ${INSTALL_DIR_CMAKE})

    # Set up the CMake config file, so that we can link to werkzeugkiste::<target> in other projects
    include(CMakePackageConfigHelpers)
    # Generate the version file for the config file
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/werkzeugkisteConfigVersion.cmake"
        VERSION "${werkzeugkiste_VERSION}"
        COMPATIBILITY AnyNewerVersion)

    # Create config file
    configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/werkzeugkisteConfig.cmake"
        INSTALL_DESTINATION ${INSTALL_DIR_CMAKE})

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/werkzeugkisteConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/werkzeugkisteConfigVersion.cmake"
        DESTINATION ${INSTALL_DIR_CMAKE})
endif()


#########################################################################################
# Add uninstall target
if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()


#########################################################################################
# Set up examples

if(werkzeugkiste_BUILD_EXAMPLES)
    #TODO(new-library-extensions) Add example applications in the
    # included cmake script.
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/werkzeugkiste_examples.cmake)
    register_werkzeugkiste_examples("${CMAKE_CURRENT_SOURCE_DIR}/examples/src")
endif()


#########################################################################################
# Set up tests

if(werkzeugkiste_BUILD_TESTS)
    # Prevent gtest from adding itself to the install target
    set(INSTALL_GTEST OFF)
    set(BUILD_GMOCK OFF)

    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG main)
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    enable_testing()

    #TODO(new-library-extensions) Add test suite:
    add_executable(tests-werkzeugkiste
        tests/geometry/utils_test.cpp    
        tests/geometry/projection_test.cpp
        tests/geometry/primitives_test.cpp
        tests/geometry/vector_test.cpp
        tests/container/sort_utils_test.cpp
        tests/container/math_utils_test.cpp
        tests/container/circular_buffer_test.cpp
        tests/files/fileio_test.cpp
        tests/files/filesys_test.cpp
        tests/strings/strings_test.cpp
        tests/timing/test_utils.h
        tests/timing/test_utils.cpp
        tests/timing/stopwatch_test.cpp
        tests/timing/tictoc_test.cpp)

    #TODO(new-library-extensions) If we need to add private
    # headers just for testing):
#    target_include_directories(werkzeugkiste-tests
#        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

    #TODO(new-library-extensions) Add link dependencies:
    target_link_libraries(tests-werkzeugkiste
        PRIVATE gtest_main 
        werkzeugkiste::geometry
        werkzeugkiste::container
        werkzeugkiste::files
        werkzeugkiste::strings
        werkzeugkiste::timing)

    #TODO(new-library-extensions) If we need to provide
    # preprocessor definitions:
#    target_compile_definitions(${TARGET_CPP_TEST} PRIVATE
#        EXAMPLE_IMAGE_WIDTH=400
#        EXAMPLE_IMAGE_HEIGHT=400)

    include(GoogleTest)
    gtest_discover_tests(tests-werkzeugkiste)
endif()

