# Hacking

<!-- TODO update document


cmake --preset=ci-coverage -DCMAKE_VERBOSE_MAKEFILE=1


check exported symbols of shared libs:
readelf -s libwerkzeugkiste-timing.so
objdump -T --demangle libwerkzeugkiste-timing.so

inspect compilation units: https://stackoverflow.com/a/59614755/400948
nm -C
objdump -S file.o | c++filt


TODO fix werkzeugkiste.so - should link/refer to all sub-libs somehow (?)

ldd examples/timing

TODO rename examples and tests
executables should be:
example-timing
tests, e.g. test-timing (or are they derived from the filename - thus, underscore is a must)


TODO validate yaml file:
python -c 'import yaml, sys; print(yaml.safe_load(sys.stdin))' < .github/workflows/ci.yml

spellcheck:
cmake -P cmake/spell.cmake

format:
cmake -D FORMAT_COMMAND=clang-format-14 -P cmake/lint.cmake
cmake -D FORMAT_COMMAND=clang-format-14 -D FIX=YES -P cmake/lint.cmake

build docs:
cmake "-DPROJECT_SOURCE_DIR=$PWD" "-DPROJECT_BINARY_DIR=$PWD/build" -P cmake/docs-ci.cmake
-->

Here is some wisdom to help you build and test this project as a developer and
potential contributor.

If you plan to contribute, please read the [CONTRIBUTING](CONTRIBUTING.md)
guide.

## Developer mode

Build system targets that are only useful for developers of this project are
hidden if the `werkzeugkiste_DEVELOPER_MODE` option is disabled. Enabling this
option makes tests and other developer targets and options available. Not
enabling this option means that you are a consumer of this project and thus you
have no need for these targets and options.

Developer mode is always set to on in CI workflows.

### Presets

This project makes use of [presets][1] to simplify the process of configuring
the project. As a developer, you are recommended to always have the [latest
CMake version][2] installed to make use of the latest Quality-of-Life
additions.

You have a few options to pass `werkzeugkiste_DEVELOPER_MODE` to the configure
command, but this project prefers to use presets.

As a developer, you should create a `CMakeUserPresets.json` file at the root of
the project:

```json
{
  "version": 2,
  "cmakeMinimumRequired": {
    "major": 3,
    "minor": 14,
    "patch": 0
  },
  "configurePresets": [
    {
      "name": "dev",
      "binaryDir": "${sourceDir}/build/dev",
      "inherits": ["dev-mode", "ci-<os>"],
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug"
      }
    }
  ],
  "buildPresets": [
    {
      "name": "dev",
      "configurePreset": "dev",
      "configuration": "Debug"
    }
  ],
  "testPresets": [
    {
      "name": "dev",
      "configurePreset": "dev",
      "configuration": "Debug",
      "output": {
        "outputOnFailure": true
      }
    }
  ]
}
```

You should replace `<os>` in your newly created presets file with the name of
the operating system you have, which may be `win64` or `unix`. You can see what
these correspond to in the [`CMakePresets.json`](CMakePresets.json) file.

`CMakeUserPresets.json` is also the perfect place in which you can put all
sorts of things that you would otherwise want to pass to the configure command
in the terminal.

### Configure, build and test

If you followed the above instructions, then you can configure, build and test
the project respectively with the following commands from the project root on
any operating system with any build system:

```sh
cmake --preset=dev
cmake --build --preset=dev
ctest --preset=dev
```

If you are using a compatible editor (e.g. VSCode) or IDE (e.g. CLion, VS), you
will also be able to select the above created user presets for automatic
integration.

Please note that both the build and test commands accept a `-j` flag to specify
the number of jobs to use, which should ideally be specified to the number of
threads your CPU has. You may also want to add that to your preset using the
`jobs` property, see the [presets documentation][1] for more details.

### Developer mode targets

These are targets you may invoke using the build command from above, with an
additional `-t <target>` flag:

#### `coverage`

Available if `ENABLE_COVERAGE` is enabled. This target processes the output of
the previously run tests when built with coverage configuration. The commands
this target runs can be found in the `COVERAGE_TRACE_COMMAND` and
`COVERAGE_HTML_COMMAND` cache variables. The trace command produces an info
file by default, which can be submitted to services with CI integration. The
HTML command uses the trace command's output to generate a HTML document to
`<binary-dir>/coverage_html` by default.

#### `docs`

Available if `BUILD_MCSS_DOCS` is enabled. Builds to documentation using
Doxygen and m.css. The output will go to `<binary-dir>/docs` by default
(customizable using `DOXYGEN_OUTPUT_DIRECTORY`).

#### `format-check` and `format-fix`

These targets run the clang-format tool on the codebase to check errors and to
fix them respectively. Customization available using the `FORMAT_PATTERNS` and
`FORMAT_COMMAND` cache variables.

#### `run-examples`

Runs all the examples created by the `add_example` command.

#### `spell-check` and `spell-fix`

These targets run the codespell tool on the codebase to check errors and to fix
them respectively. Customization available using the `SPELL_COMMAND` cache
variable.

[1]: https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html
[2]: https://cmake.org/download/

<!--

# Werkzeugkiste - Yet Another C++ Utility Library
[![Releases](https://img.shields.io/github/v/release/snototter/werkzeugkiste)](https://github.com/snototter/werkzeugkiste/releases)
[![C++17](https://img.shields.io/badge/std-c%2B%2B17-blue.svg?style=flat&logo=c%2B%2B)](https://en.cppreference.com/w/cpp/compiler_support)
[![CI Status](https://github.com/snototter/werkzeugkiste/actions/workflows/ci.yml/badge.svg)](https://github.com/snototter/werkzeugkiste/actions/workflows/ci.yml)
[![Coverage Status](https://coveralls.io/repos/github/snototter/werkzeugkiste/badge.svg?branch=main)](https://coveralls.io/github/snototter/werkzeugkiste?branch=main)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/snototter/werkzeugkiste/blob/master/LICENSE?raw=true)

This library is a collection of frequently used C++ snippets copy-pasted across
way too many of my projects. Since there's tons of so-called utilities, utils,
tools & toolboxes out there, I prefer to call this one **`werkzeugkiste`** (the
German word for toolbox).


## Functionality

This toolbox will only receive sporadic updates, basically whenever I have to
reuse (and modernize) some of my older code.
Currently, it provides the following functionality. Note that the namespace
name also equals the corresponding CMake target to be linked against:
* `werkzeugkiste::config`: Utilitity to interact with TOML/JSON/libconfig configurations.
* `werkzeugkiste::container`: Sorting/lookup utilities and a custom
  circular buffer.
* `werkzeugkiste::files`: Basic file I/O and filesystem utilities.
  _Caveat:_ These utilities are only tested on GNU/Linux and will be replaced
  by wrappers to C++17's `std::filesystem`.
* `werkzeugkiste::geometry`: Math utilities, focused on basic 2D & 3D geometry.
* `werkzeugkiste::strings`: Common string manipulation.
* `werkzeugkiste::timing`: Provides a stop watch & additional helpers on top
  of `std::chrono` (to hide some of its template boilerplate).


## Requirements

* A C++ compiler supporting at least C++17
* [CMake][1] >= 3.14 and a [compatible build tool][2], like [Make][3],
  [Ninja][4], etc.
* The linear algebra library [Eigen3](https://eigen.tuxfamily.org/)


## Usage

If you need to **manually build and install** `werkzeugkiste`, refer to the
separate [BUILDING](BUILDING.md) document.

<details>
<summary>The <b>recommended way</b> to include <tt>werkzeugkiste</tt> in your C++
project is via CMake's <tt>FetchContent</tt>:</summary>


Since v3.14, CMake provides [_FetchContent_MakeAvailable_][9], which allows us
to easily set up `werkzeugkiste` in your CMake project as:
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
    werkzeugkiste::werkzeugkiste
)
```

*Note:* The `werkzeugkiste::werkzeugkiste` target is an _all-in-one_ target, _i.e._ it includes all utilities.
Typically, you will only want to link against a specific utility target, _e.g._ `werkzeugkiste::geometry`.
The available target names are listed in the [functionality overview][#functionality].
</details>

# Contributing

Contribution guidelines are summarized in the [CONTRIBUTING](CONTRIBUTING.md) document.

<details>
<summary>Developers of <tt>werkzeugkiste</tt> need additionally tools:</summary>

* <b>Note:</b> This library is primarily developed on Unix. Since it was set up using
  [cmake-init][10], **Windows users** should check the [cmake-init README][11]
  for required changes or suggested toolchain alternatives.

* A recent [clang-tidy][5] version >= 14.

  CI will always run clang-tidy, so it is optional to install and use it
  locally, but it is highly recommended.

* Additional static analysis is run by [cppcheck][6].

  CI will always run cppcheck, so it is optional to install and use it
  locally, but it is highly recommended.

* Testing requires [googletest][7].

* Test coverage is generated via GCC's `gcov` and summarized using [LCOV][8].

  The project has a `coverage` target in developer mode if the
  `ENABLE_COVERAGE` variable is enabled. The reason why a separate target is used
  instead of CTest's built-in `coverage` step is because it lacks necessary
  customization.
  This target should be run after the tests and will generate a report at
  `<binary-dir>/coverage.info` and an HTML report at the
  `<binary-dir>/coverage_html` directory.
</details>


<details>
<summary>Task list overview / TODOs:</summary>

* [ ] Shared library
  * [x] Export symbols
  * [x] Check if shared library works "locally" (e.g. tests and examples,
    which are separate targets that link against the werkzeugkiste library)
  * [ ] Check if the export header is correctly distributed in a consuming project on linux
* [ ] Properly set up Github Actions
  * [x] Test only on linux, but more versions
  * [x] Build library and examples on windows & macos (without testing)
  * [x] Sanitize on linux
  * [x] Lint on linux
  * [ ] Clean up ci workflow, maybe split into separate (dependent?) workflows
* [ ] Adjust pre-commit hooks, e.g. https://pypi.org/project/cmake-pre-commit-hooks/
  * Use errors instead of autofix?
* [ ] Split CI into different workflows, [run them consecutively](https://stackoverflow.com/questions/58457140/dependencies-between-workflows-on-github-actions)  would allow quick overview with separate lint/build/test badges
* [ ] Deploy docs
  * [ ] Check gh-pages action
  * [ ] Clean up README (messes up the doxygen front page)
* [ ] Change to newer gcc and add `-Wextra-semi` to dev presets
* [ ] Change the default clang-format rules
* [ ] Coverage
  * [x] Check coverage target
  * [ ] Document coverage command
  * [x] Set up codecov.io or coveralls on push/release
* [ ] Utils:
  * [ ] Configuration utils - WIP
  * [x] Enumeration utils (iterator + flags)
  * [x] strings
  * [ ] circular buffer
    * [ ] Refactor properly
    * [ ] Test with complex objects (memory management)
  * [ ] Refactor vcp file utils to leverage C++17
  * [x] sort utils
  * [x] vec/basicmath
  * [x] geo2d
  * [ ] geo3d (WIP)
  * [ ] tracking-by-detection
    * [ ] Refactor Kalman filter to use Eigen
    * [ ] Implement common Bounding Box, Detection & Target class
  * [ ] pinhole camera projection
    * [x] Basic transformations
    * [ ] Extend it s.t. we can project an arbitrary number of points, i.e.
      matrices allocated on the heap vs stack
  * [ ] pinhole camera class which takes care of storing intrinsics/extrinsics, projects points, etc.
  * [ ] Trajectory smoothing
    * [x] Moving average, similar to MATLAB's `smooth`
    * [ ] Sketch filtering (xkcd-ify), e.g. via [1D interpolation](https://github.com/slayton/matlab-xkcdify),
      or [as in matplotlib](https://github.com/JohannesBuchner/matplotlib-xkcdify), although the latter would be
      a major effort due to spline fitting (*e.g.* via [ALGLIB](http://www.alglib.net/interpolation/spline3.php#header7)
      or [NCAR/EOL bspline](https://github.com/NCAR/bspline)).
</details>


[1]: https://cmake.org/
[2]: https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html
[3]: https://www.gnu.org/software/make/
[4]: https://ninja-build.org/
[5]: https://clang.llvm.org/extra/clang-tidy/
[6]: https://cppcheck.sourceforge.io/
[7]: http://google.github.io/googletest/
[8]: http://ltp.sourceforge.net/coverage/lcov.php
[9]: https://cmake.org/cmake/help/latest/module/FetchContent.html
[10]: https://pypi.org/project/cmake-init/
[11]: https://github.com/friendlyanon/cmake-init#clang-tidy

-->
