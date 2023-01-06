# Werkzeugkiste - Yet Another C++ Utility Library
This library is a collection of frequently used C++ snippets copy-pasted across way too many of my projects.
Since there's tons of so-called utilities, utils, tools & toolboxes out there, I prefer to call this one **`werkzeugkiste`** (the German word for toolbox).


## Functionality

This toolbox will only receive sporadic updates, basically whenever I have to reuse (and modernize) some of my older code.
Currently, it provides the following functionality. Note that the namespace name is equal to the corresponding CMake target:
* `werkzeugkiste::container`: Sorting utilities and a custom (`stl`-like) circular buffer.
* `werkzeugkiste::files`: Basic file I/O and filesystem utilities.  
  _Caveat:_ These utilities are only tested on GNU/Linux and will be replaced by wrappers to C++17's `std::filesystem`.
* `werkzeugkiste::geometry`: Math utilities, focused on basic 2D & 3D geometry.
  * Type-safe number comparisons (`IsEpsEqual`, `IsEpsZero`) and basic math utils (such as `Sign`).
  * 2D geometry: Circles & lines.
  * 3D geometry: Planes & lines.
* `werkzeugkiste::strings`: Common string manipulation.
* `werkzeugkiste::timing`: Provides a stop watch & additional helpers on top of `std::chrono` (hiding some of its template boilerplate).


## Requirements
* A C++ compiler supporting at least C++17
* [CMake][1] >= 3.14 and a [compatible build tool][2], like [Make][3], [Ninja][4], etc.
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




# DEPRECATED - NOT YET MERGED 

TODO move to contributing/hacking/building/etc


## Development Notes
* For each library, there should be a demo binary within `examples/`  
  Note, however, that demos are the lowest priority features on my list for this toolbox
* Ensure it is working with:
  * `cmake -Dwerkzeugkiste_WITH_EXAMPLES=ON`
  * `examples/cmake-fetch`
  * `examples/installed-locally`

## TODOs

* [ ] Change to newer gcc and add `-Wextra-semi` to dev presets
* [x] Check external inclusion via cmake FetchContent
* [ ] Utils:
  * [x] Enumeration utils (iterator + flags)
  * [x] strings
  * [x] circular buffer
  * [ ] file stuff (will be obsolete once C++17 support is more widespread)
  * [x] sort utils
  * [x] vec/basicmath
  * [x] geo2d
  * [ ] geo3d (WIP)
  * [ ] pinhole camera projection (WIP: basic transformations done; want to extend it s.t. we can project an arbitrary number of points - i.e. matrices on the heap vs stack)
  * [ ] pinhole camera class which takes care of storing intrinsics/extrinsics, projects points, etc.
  * [ ] Trajectory smoothing
    * [x] Moving average, similar to MATLAB's `smooth`
    * [ ] Sketch filtering (xkcd-ify), e.g. via [1D interpolation](https://github.com/slayton/matlab-xkcdify),
      or [as in matplotlib](https://github.com/JohannesBuchner/matplotlib-xkcdify), although the latter would be
      a major effort due to spline fitting (*e.g.* via [ALGLIB](http://www.alglib.net/interpolation/spline3.php#header7)
      or [NCAR/EOL bspline](https://github.com/NCAR/bspline)).
* [ ] Future: after upgrading to C++17, add support for `string_view` manipulation and change `::files` to use `std::filesystem` instead



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


