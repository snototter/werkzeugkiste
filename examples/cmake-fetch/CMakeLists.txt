cmake_minimum_required(VERSION 3.14)
# We need at least CMake 3.14 for FetchContent_MakeAvailable()
# For building werkzeugkiste, we required 3.15, but this was just so we can
# use "cmake --install" in the documentation. If you know what I'm talking
# about, you know how to a) get it running with an older CMake and b) how
# to install a new CMake anyhow...
project(werkzeugkiste-examples-fetched)

#########################################################################################
# Let CMake fetch and set up werkzeugkiste automatically via its git support
# Afterwards, you simply need to link the desired werkzeugkiste::<target>
# into your executable/library (no need to fiddle around with INCLUDE_DIRS
# or the like)

include(FetchContent)
FetchContent_Declare(
  werkzeugkiste
  GIT_REPOSITORY https://github.com/snototter/werkzeugkiste.git
  GIT_TAG main
)
FetchContent_MakeAvailable(werkzeugkiste)


# Now, you can simply create your targets and link werkzeugkiste. For example, if
# you want to link against the werkzeugkiste/timing library:
#
#    add_executable(myfoo
#        path/to/myfoo.cpp)
#
#    target_link_libraries(myfoo
#        PRIVATE werkzeugkiste::timing)
#
# Since there are multiple usage scenarios (and I hate copy/pasting code I have to
# maintain in the future), I defined the demo applications only once:
include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/werkzeugkiste_examples.cmake)
register_werkzeugkiste_examples("${CMAKE_CURRENT_LIST_DIR}/../src")

