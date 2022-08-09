# Werkzeugkiste - Yet Another C++ Utility Library
This is a collection of frequently used C++ snippets copy-pasted across way too many of my projects.
And since there's tons of so-called utilities, utils, tools & toolboxes out there, I prefer to call this one **`werkzeugkiste`** (the German word for toolbox).


## Requirements
* A C++ compiler supporting at least C++14
* [CMake](https://cmake.org/) >= 3.15 and a [compatible build tool](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html), like [Make](https://www.gnu.org/software/make/), [Ninja](https://ninja-build.org/), etc.


## Functionality

This toolbox will only receive sporadic updates: I will add features whenever I have to reuse some of my older code.
So far, it supports:
* `container`: Sorting utilities and a custom (`stl`-like) circular buffer
* `files`: Basic file I/O and filesystem utilities  
  _Caveat:_ These utilities are only tested on GNU/Linux and you will be better of using C++17's `std::filesystem`.
* `geometry`: Math utilities, unsurprisingly focused on basic 2D & 3D geometry
  * Type-safe number comparisons (`eps_equal`, `eps_zero`) and basic math utils (such as `sgn`).
  * 2D geometry: Circles & lines.
  * 3D geometry: Planes & lines.
* `timing`: A stop watch & additional helpers on top of `std::chrono` (hiding some of its template boilerplate)
* `strings`: Common `std::string` manipulation  
  Currently, I'm focused at C++14 and thus, there is no support for `std::string_view`.


## Usage
The recommended way to include `werkzeugkiste` in your C++ project is via CMake's FetchContent:
```cmake
# The consuming target only needs the toolbox (no examples, no testing)
set(werkzeugkiste_BUILD_EXAMPLES OFF)
set(werkzeugkiste_BUILD_TESTS OFF)
# You might need werkzeugkiste's `install` targets, e.g. if you use some
# of its classes in your (library) targets `public` interface.
# set(werkzeugkiste_INSTALL ON)

# Fetch the library:
include(FetchContent)
FetchContent_Declare(
    werkzeugkiste
    GIT_REPOSITORY https://github.com/snototter/werkzeugkiste.git
    GIT_TAG main)
FetchContent_MakeAvailable(werkzeugkiste)
message(STATUS "Using werkzeugkiste v${werkzeugkiste_VERSION}")
```


## Development Notes
* For each library, there should be a demo binary within `examples/`  
  Note, however, that demos are the lowest priority features on my list for this toolbox
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


