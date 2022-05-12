cmake_minimum_required(VERSION 3.15)
# 3.14 FetchContent + make available + ...
# 3.15 cmake --install

# Ensure we know the current version
if(NOT werkzeugkiste_VERSION)
  file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" werkzeugkiste_VERSION)
endif()

# Set up the project
project(werkzeugkiste VERSION ${werkzeugkiste_VERSION} LANGUAGES CXX)

#TODO disable examples and tests before release
option(werkzeugkiste_WITH_EXAMPLES "Build examples." ON)
option(werkzeugkiste_WITH_TESTS "Build test suite." ON)
option(INSTALL_werkzeugkiste "Configure installation target." ON)

#set(werkzeugkiste_LOG_LEVEL "info" CACHE STRING "Select log level")
#set_property(CACHE werkzeugkiste_LOG_LEVEL PROPERTY STRINGS disable trace debug info warn error critical)


# Offer the user the choice of overriding the installation (sub-)directories
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    # will not take effect without FORCE
    set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/install/v${werkzeugkiste_VERSION}"
        CACHE PATH "Installation top-level directory" FORCE)
endif()
set(INSTALL_DIR_LIB lib CACHE PATH "Installation directory for library (relative to CMAKE_INSTALL_PREFIX or absolute)")
set(INSTALL_DIR_INCLUDE include CACHE PATH "Installation directory for header files (relative to CMAKE_INSTALL_PREFIX or absolute)")
set(INSTALL_DIR_CMAKE cmake CACHE PATH "Installation directory for CMake files (relative to CMAKE_INSTALL_PREFIX or absolute)")

# Ensure all paths are absolute (needed later on)
foreach(p LIB INCLUDE CMAKE)
    set(var INSTALL_DIR_${p})
    if(NOT IS_ABSOLUTE "${${var}}")
        set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
    endif()
endforeach()


# Set compiler options
#FIXME use target_compile_options for these
set(CMAKE_CXX_STANDARD 14)     # We use std::exchange
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # only std c++14, not gnu++14

#FIXME adapt to modern cmake
# check https://stackoverflow.com/questions/33829152/for-cmake-can-you-modify-the-release-debug-compiler-flags-with-add-compiler-fl

if (MSVC)
    # warning level 4
    add_compile_options(/W4)
else()
    # be pedantic
    add_compile_options(-Wall -Wextra -pedantic)
    add_compile_options(
        "$<$<CONFIG:RELEASE>:-O3>"
        "$<$<CONFIG:DEBUG>:-g3>"
    )
endif()

# FIXME can we change this to only this project?
# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()


#FIXME can we change this to only this project/all (sub)targets?
# Compile library with position-independent code (-fPIC)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)



#########################################################################################
# Timing Utils: werkzeugkiste::timing

# Header files
set(wkzgtiming_PUBLIC_HEADER_FILES
    include/werkzeugkiste/timing/stopwatch.h
    include/werkzeugkiste/timing/tictoc.h)

# Source files
set(wkzgtiming_SOURCE_FILES
    src/timing/tictoc.cpp)

add_library(werkzeugkiste-timing
    ${wkzgtiming_PUBLIC_HEADER_FILES}
    ${wkzgtiming_SOURCE_FILES})
# Add a namespace'd alias
add_library(werkzeugkiste::timing ALIAS werkzeugkiste-timing)


target_include_directories(werkzeugkiste-timing
    # Where external projects will look for the timeutils headers
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
           $<INSTALL_INTERFACE:${INSTALL_DIR_INCLUDE}>)


#set_target_properties(werkzeugkiste-timing PROPERTIES
#    PUBLIC_HEADER "${wkzgtiming_PUBLIC_HEADER_FILES}"
#    OUTPUT_NAME werkzeugkiste-timing
#    DEBUG_POSTFIX "d")



#########################################################################################
# Installation & Exports

# Auto-generate werkzeugkiste's version.h file
set(werkzeugkiste_VERSION_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/include/werkzeugkiste/version.h)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in
    ${werkzeugkiste_VERSION_HEADER})


if (INSTALL_werkzeugkiste)
#FIXME public_header destination will not work
    #TODO Install header files separately per target
    install(FILES ${werkzeugkiste_VERSION_HEADER} DESTINATION ${INSTALL_DIR_INCLUDE}/werkzeugkiste)
    install(FILES ${wkzgtiming_PUBLIC_HEADER_FILES} DESTINATION ${INSTALL_DIR_INCLUDE}/werkzeugkiste/timing)
    
    install(TARGETS werkzeugkiste-timing  #TODO add other exported targets here
        EXPORT werkzeugkisteTargets
        LIBRARY DESTINATION ${INSTALL_DIR_LIB}
        ARCHIVE DESTINATION ${INSTALL_DIR_LIB})
        # We don't have any installable executables, so we don't need to set the RUNTIME destination:
        ### RUNTIME DESTINATION ${INSTALL_DIR_BIN}
        # We can't use the PUBLIC_HEADER destination because all registered headers will be copied
        # directly into this destination, ignoring any folder hierarchy:
        ### PUBLIC_HEADER DESTINATION ${INSTALL_DIR_INCLUDE}/werkzeugkiste)
        # Since we already defined the PUBLIC include directories for our target, the INCLUDES DESTINATION
        # is not needed. It would not result in an error, but would simply cause a duplicated path entry
        # in the generated .cmake file
        # INCLUDES DESTINATION ${INSTALL_INCLUDE_DIR}


    install(EXPORT werkzeugkisteTargets
        FILE werkzeugkisteTargets.cmake
        NAMESPACE werkzeugkiste::
        DESTINATION ${INSTALL_DIR_CMAKE})
endif()
        

### Set up the CMake config file, so that werkzeugkiste::targets can easily be integrated in your projects
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
if (INSTALL_werkzeugkiste)
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
# Examples

if(werkzeugkiste_WITH_EXAMPLES)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/werkzeugkiste_examples.cmake)
    register_werkzeugkiste_examples("${CMAKE_CURRENT_SOURCE_DIR}/examples/src")
endif()


#########################################################################################
# Tests

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

    add_executable(tests-werkzeugkiste
#        src/helpers/enum.h
        tests/timing/test_utils.h
        tests/timing/test_utils.cpp
        tests/timing/stopwatch_test.cpp
        tests/timing/tictoc_test.cpp)

#    target_include_directories(werkzeugkiste-tests
#        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

    target_link_libraries(tests-werkzeugkiste
        PRIVATE gtest_main 
        werkzeugkiste::timing
        #TODO add future utils
    )

#    target_compile_definitions(${TARGET_CPP_TEST} PRIVATE
#        EXAMPLE_IMAGE_WIDTH=400
#        EXAMPLE_IMAGE_HEIGHT=400)

    include(GoogleTest)
    gtest_discover_tests(tests-werkzeugkiste)
endif()

