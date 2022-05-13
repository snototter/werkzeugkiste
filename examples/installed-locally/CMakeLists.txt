cmake_minimum_required(VERSION 3.14)
project(werkzeugkiste-examples-installed)

#########################################################################################
# Workflow:
# (1) Tell CMake where to find the werkzeugkisteConfig.cmake file if you installed to
#     a non-standard location
# (2) Let CMake load all werkzeugkiste targets via find_package
# (3) Link the appropriate werkzeugkiste::<target> to your executable/library (no need
#     to fiddle around with INCLUDE_DIRS, xxx_LIB, or the like)


# This demo assumes that you installed werkzeugkiste using the default CMAKE_INSTALL_PREFIX.
# Thus, we don't need to set the CMAKE_PREFIX_PATH manually and CMake will easily
# locate the library via
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

