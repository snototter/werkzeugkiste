# For the werkzeugkiste examples, we need the same target definitions for three
# different "how-to-use werkzeugkiste" setups:
# 1) from werkzeugkiste's `PRJ/CMakeLists.txt`
# 2) from `examples/installed-locally/CMakeLists.txt`
# 3) from `examples/cmake-fetch/CMakeLists.txt`
#
# Thus, we simply define these targets once within the following function and
# invoke this from the corresponding list file:

function(register_werkzeugkiste_examples EXAMPLE_SRC_DIR)
    ## werkzeugkiste::config
    add_executable(example-config
        ${EXAMPLE_SRC_DIR}/config_example.cpp)
    target_link_libraries(example-config
        PRIVATE werkzeugkiste::werkzeugkiste)
#        PRIVATE werkzeugkiste::config)

    ## werkzeugkiste::container
    add_executable(example-container
        ${EXAMPLE_SRC_DIR}/container_example.cpp)
    target_link_libraries(example-container
        PRIVATE werkzeugkiste::container)

    ## werkzeugkiste::geometry
    add_executable(example-geometry
        ${EXAMPLE_SRC_DIR}/geometry_example.cpp)
    target_link_libraries(example-geometry
        PRIVATE werkzeugkiste::geometry)

    ## werkzeugkiste::files
    find_package(Filesystem REQUIRED)
    add_executable(example-files
        ${EXAMPLE_SRC_DIR}/files_example.cpp)
    target_link_libraries(example-files
        PRIVATE
        werkzeugkiste::files
        werkzeugkiste::strings)

    ## werkzeugkiste::strings
    add_executable(example-strings
        ${EXAMPLE_SRC_DIR}/strings_example.cpp)
    target_link_libraries(example-strings
        PRIVATE werkzeugkiste::strings)

    ## werkzeugkiste::timing
    add_executable(example-timing
        ${EXAMPLE_SRC_DIR}/timing_example.cpp)
    target_link_libraries(example-timing
        PRIVATE werkzeugkiste::timing)

    #TODO add future (sub-)library demos here!

    ##TODO add an example which uses the all-in-one target werkzeugkiste::werkzeugkiste
endfunction()

