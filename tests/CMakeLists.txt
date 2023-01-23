#FIXME check https://www.kitware.com/dynamic-google-test-discovery-in-cmake-3-10 and use gtest
cmake_minimum_required(VERSION 3.14)

project(werkzeugkisteTests LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)
include(../cmake/windows-set-path.cmake)

# ---- Dependencies ----

if(PROJECT_IS_TOP_LEVEL)
  find_package(werkzeugkiste REQUIRED)

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
endif()

# ---- Tests ----

add_executable(werkzeugkiste-test
  src/test_utils.h
  src/test_utils.cpp
  src/geometry/utils_test.cpp    
  src/geometry/projection_test.cpp
  src/geometry/primitives_test.cpp
  src/geometry/vector_test.cpp
  src/container/sort_utils_test.cpp
  src/container/math_utils_test.cpp
  src/container/circular_buffer_test.cpp
  src/files/fileio_test.cpp
  src/files/filesys_test.cpp
  src/strings/strings_test.cpp
  src/timing/stopwatch_test.cpp
  src/timing/tictoc_test.cpp
)
target_link_libraries(werkzeugkiste-test
  PRIVATE
  gtest_main
  gtest
  pthread
  werkzeugkiste::container
  werkzeugkiste::geometry
  werkzeugkiste::strings
  werkzeugkiste::files
  werkzeugkiste::timing)

target_compile_features(werkzeugkiste-test PRIVATE cxx_std_17)

add_test(NAME werkzeugkiste-test COMMAND werkzeugkiste-test)
windows_set_path(werkzeugkiste-test werkzeugkiste::container werkzeugkiste::geometry werkzeugkiste::strings werkzeugkiste::files werkzeugkiste::timing)

include(GoogleTest)
gtest_discover_tests(werkzeugkiste-test)

# ---- End-of-file commands ----

add_folders(Tests)



#add_executable(tests-werkzeugkiste
#        tests/geometry/utils_test.cpp    
#        tests/geometry/projection_test.cpp
#        tests/geometry/primitives_test.cpp
#        tests/geometry/vector_test.cpp
#        tests/container/sort_utils_test.cpp
#        tests/container/math_utils_test.cpp
#        tests/container/circular_buffer_test.cpp
#        tests/files/fileio_test.cpp
#        tests/files/filesys_test.cpp
#        tests/strings/strings_test.cpp
#        tests/timing/test_utils.h
#        tests/timing/test_utils.cpp
#        tests/timing/stopwatch_test.cpp
#        tests/timing/tictoc_test.cpp)

    #TODO(new-library-extensions) If we need to add private
    # headers just for testing):
#    target_include_directories(werkzeugkiste-tests
#        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")