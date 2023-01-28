cmake_minimum_required(VERSION 3.14)

project(werkzeugkisteExamples CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)

if(PROJECT_IS_TOP_LEVEL)
  find_package(werkzeugkiste REQUIRED)
endif()

add_custom_target(run-examples)

function(add_example NAME LIBS)
  add_executable("${NAME}" "src/${NAME}_example.cpp")
  target_link_libraries("${NAME}" PRIVATE "${LIBS}")
  target_compile_features("${NAME}" PRIVATE cxx_std_17)
  add_custom_target("run_${NAME}" COMMAND "${NAME}" VERBATIM)
  add_dependencies("run_${NAME}" "${NAME}")
  add_dependencies(run-examples "run_${NAME}")
endfunction()

add_example(config werkzeugkiste::werkzeugkiste)
add_example(container werkzeugkiste::container)
add_example(files werkzeugkiste::files)
add_example(geometry werkzeugkiste::geometry)
add_example(strings werkzeugkiste::strings)
add_example(timing werkzeugkiste::timing)

add_folders(Examples)
