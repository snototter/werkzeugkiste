# Werkzeugkiste - Yet Another C++ Utility Library
This is a collection of frequently used C++ snippets copy-pasted across way too many of my projects.
And since there's tons of so-called utilities, utils, tools & toolboxes out there, this is going to be my own **`werkzeugkiste`** (the German word for toolbox).

**Roadmap**
* [ ] Collect all utilities, refactor where needed & ensure everything is properly tested
  * [ ] Basic math
  * [ ] 2D geometry
  * [ ] 3D geometry
  * [ ] Basic file utilities
  * [x] Container / sorting utilities
  * [x] Stop watch
  * [x] Strings
* [ ] Raise requirements to C++17 and extend with `filesystem` and `string_view`


## Requirements
* A C++ compiler supporting at least C++14
* [CMake](https://cmake.org/) >= 3.15 and a [compatible build tool](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html), like [Make](https://www.gnu.org/software/make/), [Ninja](https://ninja-build.org/), etc.


## Functionality
* `container`: Sorting utilities and a custom circular buffer
* `geometry`: Math utilities (not surprisingly focusing on basic 2D & 3D geometry)
* `strings`: Helpers for string manipulation
* `timing`: A simple stop watch & helpers on top of `std::chrono`

## Usage
TODO 2 major usage scenarios: fetch_content or install + findpackage

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
* [x] Check external inclusion via cmake FetchContent
* [ ] Utils:
  * [x] Enumeration utils (iterator + flags)
  * [x] strings
  * [x] circular buffer
  * [ ] file stuff (will be obsolete once C++17 support is more widespread)
  * [x] sort utils
  * [x] vec/basicmath
  * [ ] geo2d (WIP)
  * [ ] geo3d
    * [ ] Pinhole camera class which takes care of storing intrinsics/extrinsics, projects points, etc.
* [x] check if static lib setup works when linked into consuming application/library
* [x] Delete time-utils repository
* [ ] After initial release --> consider upgrading to C++17 and supporting string_views?

