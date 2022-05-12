cmake_minimum_required(VERSION 3.14)
project(werkzeugkiste-examples-installed)

#########################################################################################
# Workflow:
# 1) Tell CMake where to find the werkzeugkisteConfig.cmake file
# 2) Let CMake load all werkzeugkiste targets via find_package
# 3) Link the appropriate werkzeugkiste::<target> to your executable/library (no need
#    to fiddle around with INCLUDE_DIRS, xxx_LIB, or the like)


# In your dev environment, you would know which werkzeugkiste version you built
# and where you've installed it. For this demo, we simply assume you used the
# default settings (i.e. installed werkzeugkiste to PROJECT_ROOT/install/vX.Y.Z/).
# Thus, we query the version string directly from this repository:
file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/../../VERSION" werkzeugkiste_VERSION)

# Tell CMake where to find werkzeugkisteConfig.cmake:
set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/../../install/v${werkzeugkiste_VERSION}/cmake")

# Let CMake locate the library:
find_package(werkzeugkiste REQUIRED CONFIG)


# Now, you can simply create your targets and link werkzeugkiste. For example, if
# you want to link against the werkzeugkiste/timing library:
#
#    add_executable(myfoo
#        path/to/myfoo.cpp)
#
#    target_link_libraries(myfoo
#        PRIVATE werkzeugkiste::timing)

# Since there are multiple usage scenarios (and I hate copy/pasting code I have to
# maintain in the future), I defined the demo applications only once:
include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/werkzeugkiste_examples.cmake)
register_werkzeugkiste_examples("${CMAKE_CURRENT_LIST_DIR}/../src")

