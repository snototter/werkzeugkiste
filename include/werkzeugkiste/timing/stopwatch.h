#ifndef WERKZEUGKISTE_TIMING_STOPWATCH_H
#define WERKZEUGKISTE_TIMING_STOPWATCH_H

#include <chrono>  // NOLINT
#include <string>
#include <stdexcept>
#include <sstream>


/// Stop watch & additional helpers on top of `std::chrono` (to hide
/// some of its template boilerplate).
namespace werkzeugkiste::timing {

// NOLINTBEGIN(*-else-after-return)

/// Returns the abbreviation for the given
/// duration type, e.g. std::chrono::hours --> "hrs".
template <typename Duration>
std::string DurationAbbreviation() {
  if (std::is_same<Duration, std::chrono::nanoseconds>::value) {
    return "ns";
  } else if (std::is_same<Duration, std::chrono::microseconds>::value) {
    return "us";
  } else if (std::is_same<Duration, std::chrono::milliseconds>::value) {
    return "ms";
  } else if (std::is_same<Duration, std::chrono::seconds>::value) {
    return "sec";
  } else if (std::is_same<Duration, std::chrono::minutes>::value) {
    return "min";
  } else if (std::is_same<Duration, std::chrono::hours>::value) {
    return "hrs";
  }
#if __cplusplus >= 202002L // C++20
  // Trivia: MSVC didn't properly define __cplusplus until
  // early 2018. Since it is 2022, we don't need to add support
  // for outdated MSVC versions.
  // https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-__cplusplus
  else if (std::is_same<Duration, std::chrono::days>::value) {
    return "days";
  } else if (std::is_same<Duration, std::chrono::weeks>::value) {
    return "wks";
  } else if (std::is_same<Duration, std::chrono::months>::value) {
    return "mth";
  } else if (std::is_same<Duration, std::chrono::years>::value) {
    return "yrs";
  }
#endif  // C++20
  std::ostringstream s;
  s << "Duration \"" << typeid(Duration).name()
    << "\" has not been mapped to its abbreviation yet.";
  throw std::runtime_error(s.str());
}


/// Returns the fully-qualified name of the
/// given clock type, e.g. "std::chrono::system_clock".
template <typename Clock>
std::string ClockTypeName() {
  if (std::is_same<Clock, std::chrono::steady_clock>::value) {
    return "std::chrono::steady_clock";
  } else if (std::is_same<Clock, std::chrono::system_clock>::value) {
    return "std::chrono::system_clock";
  } else if (std::is_same<Clock, std::chrono::high_resolution_clock>::value) {
    // The highres clock should just be an alias to system or steady (C++11),
    // thus we should never enter (but it doesn't hurt to include it either).
    return "std::chrono::high_resolution_clock";
  }
#if __cplusplus >= 202002L // C++20
  else if (std::is_same<Clock, std::chrono::utc_clock>::value) {
    return "std::chrono::utc_clock";
  } else if (std::is_same<Clock, std::chrono::tai_clock>::value) {
    return "std::chrono::tai_clock";
  } else if (std::is_same<Clock, std::chrono::gps_clock>::value) {
    return "std::chrono::gps_clock";
  } else if (std::is_same<Clock, std::chrono::file_clock>::value) {
    return "std::chrono::file_clock";
  } else if (std::is_same<Clock, std::chrono::local_t>::value) {
    return "std::chrono::local_t";
  }
#endif  // C++20

  std::ostringstream s;
  s << "Clock type \"" << typeid(Clock).name()
    << "\" has not been mapped to its name yet.";
  throw std::runtime_error(s.str());
}


/// Returns the fully-qualified name of the given
/// duration type, e.g. "std::chrono::nanoseconds".
template <typename Duration>
std::string PrecisionTypeName() {
  if (std::is_same<Duration, std::chrono::nanoseconds>::value) {
    return "std::chrono::nanoseconds";
  } else if (std::is_same<Duration, std::chrono::microseconds>::value) {
    return "std::chrono::microseconds";
  } else if (std::is_same<Duration, std::chrono::milliseconds>::value) {
    return "std::chrono::milliseconds";
  } else if (std::is_same<Duration, std::chrono::seconds>::value) {
    return "std::chrono::seconds";
  } else if (std::is_same<Duration, std::chrono::minutes>::value) {
    return "std::chrono::minutes";
  } else if (std::is_same<Duration, std::chrono::hours>::value) {
    return "std::chrono::hours";
  }
#if __cplusplus >= 202002L // C++20
  else if (std::is_same<Duration, std::chrono::days>::value) {
    return "std::chrono::days";
  } else if (std::is_same<Duration, std::chrono::weeks>::value) {
    return "std::chrono::weeks";
  } else if (std::is_same<Duration, std::chrono::months>::value) {
    return "std::chrono::months";
  } else if (std::is_same<Duration, std::chrono::years>::value) {
    return "std::chrono::years";
  }
#endif  // C++20

  std::ostringstream s;
  s << "Duration type \"" << typeid(Duration).name()
    << "\" has not been mapped yet.";
  throw std::runtime_error(s.str());
}


// NOLINTEND(*-else-after-return)


/// Returns the number of ticks for the given
/// std::chrono::duration with a different precision.
template<typename DurationFrom, typename DurationTo> inline constexpr
double CastToTicks(const DurationFrom &duration) {
  // Floating-point duration requires no explicit duration_cast
  // https://en.cppreference.com/w/cpp/chrono/duration/duration_cast
  const std::chrono::duration<double, typename DurationTo::period> target(duration);
  return target.count();
}


/// Returns the number of seconds for the given std::chrono::duration.
template<typename Duration> inline constexpr
double ToSeconds(const Duration &duration) {
  return CastToTicks<Duration, std::chrono::seconds>(duration);
}


/// Returns the number of milliseconds for the given std::chrono::duration.
template<typename Duration> inline constexpr
double ToMilliseconds(const Duration &duration) {
  return CastToTicks<Duration, std::chrono::milliseconds>(duration);
}


/// Returns the number of microseconds for the given std::chrono::duration.
template<typename Duration> inline constexpr
double ToMicroseconds(const Duration &duration) {
  return CastToTicks<Duration, std::chrono::microseconds>(duration);
}


/// Returns the number of nanoseconds for the given std::chrono::duration.
template<typename Duration> inline constexpr
double ToNanoseconds(const Duration &duration) {
  return CastToTicks<Duration, std::chrono::nanoseconds>(duration);
}


/// Returns a human readable string approximating the given time.
///
/// For example, SecondsToString(3700 * 24 + 50) = '1 day 40 minutes'
std::string SecondsToString(unsigned int seconds);



/// A stop watch with configurable clock.
///
/// A stop watch measures the time since you last called @see Start(),
/// or since its construction.
/// Use its ``ElapsedXXX()`` methods to get the elapsed time in the
/// corresponding time interval (i.e. precision), e.g. ElapsedSeconds().
///
/// Duration measurements should use a monotonic clock. Thus, we
/// explicitly default to ``steady_clock`` (since ``high_resolution_clock`` is
/// not guaranteed to be steady).
template <typename Clock = std::chrono::steady_clock>
class stop_watch {  // NOLINT
 public:
  /// Clock type used by this stop watch.
  using clock_type = Clock;  // NOLINT


  /// Constructor starts the stop watch.
  stop_watch() {
    Start();
  }


  /// Copy-constructor copies the other's start time point.
  stop_watch(const stop_watch &other)
    : t_start_{other.t_start_}
  {}


  // TODO future extension: similar to a scoped guard, add an
  // "disarm" method. Otherwise, the destructor could print
  // the elapsed time.
  // Requires a 2nd tpl parameter (default to millisec) that
  // specifies the unit to print the elapsed time...
  // Then, the move ctor/assignments should be implemented
  // properly (and disarm the moved instance)
  ~stop_watch() = default;

  stop_watch(stop_watch &&other) noexcept = default;
  stop_watch &operator=(const stop_watch &other) = default;
  stop_watch &operator=(stop_watch &&other) noexcept = default;


  /// Starts or restarts the stop watch.
  void Start() {
    t_start_ = clock_type::now();
  }


  /// Returns the elapsed time as ticks of the
  /// given time interval (specifed as tick interval, which
  /// is a rational fraction - std::ratio - representing the
  /// time in seconds from one tick to the next, i.e. similar
  /// to the Period template parameter in ``std::chrono::duration``).
  template<typename Ratio>
  double ElapsedAs() const {
    const auto t_end = clock_type::now();
    // Floating-point duration requires no explicit duration_cast
    const std::chrono::duration<double, Ratio> duration = t_end - t_start_;
    return duration.count();
  }


  /// Returns the elapsed time in seconds.
  double ElapsedSeconds() const {
    return ElapsedAs<std::ratio<1>>();
  }


  /// Returns the elapsed time in milliseconds.
  double ElapsedMilliseconds() const {
    return ElapsedAs<std::milli>();
  }


  /// Returns the elapsed time in microseconds.
  double ElapsedMicroseconds() const {
    return ElapsedAs<std::micro>();
  }


  /// Returns the elapsed time in nanoseconds.
  double ElapsedNanoseconds() const {
    return ElapsedAs<std::nano>();
  }


  /// Returns the number of years before this stop watch
  /// will overflow.
  double YearsUntilOverflow() const {
    const auto duration_hrs =
        std::chrono::duration_cast<std::chrono::hours>(
            clock_type::time_point::max() - clock_type::now());
    // Convert to years, similar to C++20's std::chrono::years definition
    constexpr double hrs_per_year = 8765.82; // = 24 * 365.2425
    return static_cast<double>(duration_hrs.count()) / hrs_per_year;
  }


  /// Returns true if the used clock is steady (monotonic).
  bool IsSteady() const {
    return clock_type::is_steady;
  }


  /// Returns a readable clock typeid, for
  /// example "std::chrono::steady_clock".
  std::string ClockName() const {
    return ClockTypeName<Clock>();
  }


 private:
  /// Time point from which we measure the elapsed time.
  typename clock_type::time_point t_start_{};
};

using StopWatch = stop_watch<std::chrono::steady_clock>;

}  // namespace werkzeugkiste::timing

#endif  // WERKZEUGKISTE_TIMING_STOPWATCH_H
