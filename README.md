# Werkzeugkiste - Yet Another C++ Utility Library

This library is a collection of frequently used C++ snippets copy-pasted across
way too many of my projects. Since there's tons of so-called utilities, utils,
tools & toolboxes out there, I prefer to call this one **`werkzeugkiste`**,
__i.e.__ the German word for toolbox.

[![Release](https://img.shields.io/github/v/release/snototter/werkzeugkiste)](https://github.com/snototter/werkzeugkiste/releases)

[![C++17](https://img.shields.io/badge/std-c%2B%2B17-blue.svg?style=flat&logo=c%2B%2B)](https://en.cppreference.com/w/cpp/compiler_support)

[![CI Status](https://github.com/snototter/werkzeugkiste/actions/workflows/ci.yml/badge.svg)](https://github.com/snototter/werkzeugkiste/actions/workflows/ci.yml)

[![Coverage Status](https://coveralls.io/repos/github/snototter/werkzeugkiste/badge.svg?branch=main)](https://coveralls.io/github/snototter/werkzeugkiste?branch=main)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/snototter/werkzeugkiste/blob/master/LICENSE?raw=true)


## Functionality

This toolbox will only receive sporadic updates, basically whenever I have to
reuse (and modernize) some of my older code.
Currently, it provides the following functionality. Note that the namespace
name also equals the corresponding CMake target to be linked against:
* `werkzeugkiste::config`: Utilitity to interact with TOML/JSON/libconfig configurations.
* `werkzeugkiste::container`: Sorting/lookup utilities and a custom
  circular buffer.
* `werkzeugkiste::files`: Basic file I/O and filesystem utilities.
  _Caveat:_ These utilities are only tested on/available for GNU/Linux.
  For portable applications, C++17's `std::filesystem` should be used instead.
* `werkzeugkiste::geometry`: Math utilities, focused on basic 2D & 3D geometry.
* `werkzeugkiste::strings`: Common string manipulation.
* `werkzeugkiste::timing`: Provides a stop watch & additional helpers on top
  of `std::chrono` (to hide some of its template boilerplate).


## Requirements

* A C++ compiler supporting at least C++17
* [CMake][] >= 3.14 and a [compatible build tool][Generators], like [Make][],
  [Ninja][], etc.
* The linear algebra library [Eigen3][]


## Usage

The **recommended way** to include `werkzeugkiste` in a C++ project is via
CMake's `FetchContent`:

```cmake
# Fetch the library:
include(FetchContent)
FetchContent_Declare(
    werkzeugkiste
    GIT_REPOSITORY https://github.com/snototter/werkzeugkiste.git
    GIT_TAG main)
FetchContent_MakeAvailable(werkzeugkiste)

# Optionally print the available library version:
message(STATUS "Using werkzeugkiste v${werkzeugkiste_VERSION}")
```

Afterwards, add it to your consuming executable/library via:
```cmake
target_link_libraries(
    your_target PRIVATE
    werkzeugkiste::config
    werkzeugkiste::geometry
    werkzeugkiste::strings
    # Add other werkzeugkiste packages if needed
)
```

## Contributing

* Details for developers or users who need to **manually build and install**
  `werkzeugkiste` are provided in the separate [BUILDING][] document.
* Contribution guidelines are summarized in the [CONTRIBUTING][] document.


[CMake]: https://cmake.org/
[Generators]: https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html
[Make]: https://www.gnu.org/software/make/
[Ninja]: https://ninja-build.org/
[Eigen3]: https://eigen.tuxfamily.org/
[BUILDING]: https://github.com/snototter/werkzeugkiste/blob/main/BUILDING.md
[CONTRIBUTING]: https://github.com/snototter/werkzeugkiste/blob/main/CONTRIBUTING.md
