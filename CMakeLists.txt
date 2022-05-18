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
# If included via CMake's FetchContent, we want to pass the version
# explicitly to the parent scope:
set(werkzeugkiste_VERSION ${werkzeugkiste_VERSION} PARENT_SCOPE)

# Set up the project
project(werkzeugkiste VERSION ${werkzeugkiste_VERSION} LANGUAGES CXX)

#TODO(snototter) Before release, disable examples and tests by default
option(werkzeugkiste_WITH_EXAMPLES "Build examples." ON)
option(werkzeugkiste_WITH_TESTS "Build test suite." ON)
option(INSTALL_werkzeugkiste "Configure installation target." ON)

#TODO(snototter) Use logging, similar to viren2d
#set(werkzeugkiste_LOG_LEVEL "info" CACHE STRING "Select log level")
#set_property(CACHE werkzeugkiste_LOG_LEVEL PROPERTY STRINGS disable trace debug info warn error critical)


# Use the common Unix default installation paths (the GNUInstallDirs package
# should also provide sane paths on Windows, but I didn't check this yet):
include(GNUInstallDirs)
set(INSTALL_DIR_INCLUDE ${CMAKE_INSTALL_INCLUDEDIR})
set(INSTALL_DIR_LIB ${CMAKE_INSTALL_LIBDIR})
# A default search location for CMake config files is /usr/lib/cmake/<package-name>
set(INSTALL_DIR_CMAKE ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})


#TODO(snototter) I could instead set CXX_STANDARD, etc per target
# Set compiler options for all targets
if(NOT CMAKE_CXX_STANDARD)
    #TODO(snototter) Check C++ standard (14 for std::exchange, 17 for string_view/filesystem, anything else is 11)
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    # To ensure portability, turn off compiler-specific extensions
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

#FIXME adapt to modern cmake
# check https://stackoverflow.com/questions/33829152/for-cmake-can-you-modify-the-release-debug-compiler-flags-with-add-compiler-fl
#FIXME check spdlog's CMakeLists on what to set extra for MSVC :/

if (MSVC)
    # warning level 4
    add_compile_options(/W4)
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

# Header files
set(wkzggeometry_PUBLIC_HEADER_FILES
    include/werkzeugkiste/geometry/utils.h
    include/werkzeugkiste/geometry/vector.h)

# Source files
set(wkzggeometry_SOURCE_FILES
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


if(INSTALL_werkzeugkiste)
    #TODO(snototter) New libraries: Install header files per library target
    install(FILES ${werkzeugkiste_VERSION_HEADER} DESTINATION ${INSTALL_DIR_INCLUDE}/werkzeugkiste)
    install(FILES ${wkzgtiming_PUBLIC_HEADER_FILES} DESTINATION ${INSTALL_DIR_INCLUDE}/werkzeugkiste/timing)
    install(FILES ${wkzgstrings_PUBLIC_HEADER_FILES} DESTINATION ${INSTALL_DIR_INCLUDE}/werkzeugkiste/strings)
    install(FILES ${wkzggeometry_PUBLIC_HEADER_FILES} DESTINATION ${INSTALL_DIR_INCLUDE}/werkzeugkiste/geometry)

    #TODO(snototter) New libraries: Add targets for export
    install(TARGETS
        werkzeugkiste-timing
        werkzeugkiste-strings
        werkzeugkiste-geometry
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

    # Where to export the targets:
    install(EXPORT werkzeugkisteTargets
        FILE werkzeugkisteTargets.cmake
        NAMESPACE werkzeugkiste::
        DESTINATION ${INSTALL_DIR_CMAKE})
endif()
        

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

# Install config files
if(INSTALL_werkzeugkiste)
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/werkzeugkisteConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/werkzeugkisteConfigVersion.cmake"
        DESTINATION ${INSTALL_DIR_CMAKE})

    # Generate the export targets for the build tree
    export(EXPORT werkzeugkisteTargets
        FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/werkzeugkisteTargets.cmake"
        NAMESPACE werkzeugkiste::)
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

if(werkzeugkiste_WITH_EXAMPLES)
    #TODO(snototter) New libraries: Add example application
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/werkzeugkiste_examples.cmake)
    register_werkzeugkiste_examples("${CMAKE_CURRENT_SOURCE_DIR}/examples/src")
endif()


#########################################################################################
# Set up tests

if(werkzeugkiste_WITH_TESTS)
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

    #TODO(snototter) New libraries: Add test suite
    add_executable(tests-werkzeugkiste
#        src/helpers/enum.h
        tests/geometry/vector_test.cpp
        tests/strings/strings_test.cpp
        tests/timing/test_utils.h
        tests/timing/test_utils.cpp
        tests/timing/stopwatch_test.cpp
        tests/timing/tictoc_test.cpp)

    #TODO(snototter) If we need to add private headers just for testing:
#    target_include_directories(werkzeugkiste-tests
#        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

    #TODO(snototter) New libraries: Add link dependency
    target_link_libraries(tests-werkzeugkiste
        PRIVATE gtest_main 
        werkzeugkiste::geometry
        werkzeugkiste::strings
        werkzeugkiste::timing)

    #TODO(snototter) If we need to provide preprocessor definitions:
#    target_compile_definitions(${TARGET_CPP_TEST} PRIVATE
#        EXAMPLE_IMAGE_WIDTH=400
#        EXAMPLE_IMAGE_HEIGHT=400)

    include(GoogleTest)
    gtest_discover_tests(tests-werkzeugkiste)
endif()

