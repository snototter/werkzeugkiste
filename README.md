# Werkzeugkiste - A small C++ Utility Library
Trying to refactor all the frequently copy-pasted code snippets into a single toolbox/utility library (and spice it up with some more modern C++).
And since there's tons of utilities, utils, tools & toolboxes out there (and I happen to know some german), this is going to be my **werkzeugkiste**.

## Requirements
TODO
Build
* CMake
* make or ninja-build, ...
* g++, msvc, ...
* std=c++14

Test
* googletest


## Functionality
* Basic geometry
* String utilities
* Stopwatch

## Usage
Two major usage scenarios:
TODO actually 3 (needs verification!) separate cmake project, include werkzeugkiste via add_dir...

### Fetch from github via CMake
TODO

see examples/cmake-fetch

### Install Werkzeug Locally
TODO
see examples/installed-locally

## Development Notes
* For each library, add a demo binary to `examples/`
* Ensure it is working with:
  * `cmake -Dwerkzeugkiste_WITH_EXAMPLES=ON`
  * `examples/cmake-fetch`
  * `examples/installed-locally`

## TODOs

* [ ] Install globally or locally via `cmake --install .`
* [ ] Check external inclusion via cmake FetchContent
* [ ] Utils:
  * [ ] Enumeration utils (iterator + flags)
  * [ ] strings
  * [ ] circular buffer
  * [ ] file stuff (will be obsolete once C++17 support is more widespread)
  * [ ] sort utils
  * [ ] vec/basicmath
  * [ ] Simplified logging? (static library set up to ensure default log is set up :thinking-face:)
  
* [ ] check if static lib setup works when linked into consuming application/library
* [ ] Delete time-utils repository
* [ ] After initial release --> consider upgrading to C++17 and supporting string_views; or maybe backporting a string_view?

