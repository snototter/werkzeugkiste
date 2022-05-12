# For the werkzeugkiste examples, we need the same target definitions for three
# different "how-to-use werkzeugkiste" setups:
# 1) from werkzeugkiste's `PRJ/CMakeLists.txt`
# 2) from `examples/installed-locally/CMakeLists.txt`
# 3) from `examples/cmake-fetch/CMakeLists.txt`
#
# Thus, we simply define these targets once within the following function and
# invoke this from the corresponding list file:

function(register_werkzeugkiste_examples EXAMPLE_SRC_DIR)
    ## werkzeugkiste::timing
    add_executable(example-timing
        ${EXAMPLE_SRC_DIR}/timing_example.cpp)
    target_link_libraries(example-timing
        PRIVATE werkzeugkiste::timing)

    #TODO add future (sub-)library demos here!

    ## TODO werkzeugkiste::strings
    ## TODO werkzeugkiste::geometry
endfunction()

