#ifndef __WERKZEUGKISTE_TIMING_STOPWATCH_H__
#define __WERKZEUGKISTE_TIMING_STOPWATCH_H__

#include <chrono>  // NOLINT [build/c++11]
#include <string>
#include <stdexcept>
#include <sstream>


namespace werkzeugkiste {
/**
 * Simple timing utils to quickly measure time
 * with a stop watch.
 */
namespace timing {

 /**
  * @brief Returns the abbreviation for the given
  * duration type, e.g. std::chrono::hours --> "hrs".
  */
template <typename P>
std::string DurationAbbreviation() {
  if (std::is_same<P, std::chrono::nanoseconds>::value)
    return "ns";
  else if (std::is_same<P, std::chrono::microseconds>::value)
    return "us";
  else if (std::is_same<P, std::chrono::milliseconds>::value)
    return "ms";
  else if (std::is_same<P, std::chrono::seconds>::value)
    return "sec";
  else if (std::is_same<P, std::chrono::minutes>::value)
    return "min";
  else if (std::is_same<P, std::chrono::hours>::value)
    return "hrs";
#if __cplusplus >= 202002L
  // Trivia: MSVC didn't properly define __cplusplus until
  // early 2018. Since it is 2022, we don't need to add support
  // for outdated MSVC versions.
  // https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-__cplusplus
  else if (std::is_same<P, std::chrono::days>::value)
    return "days";
  else if (std::is_same<P, std::chrono::weeks>::value)
    return "wks";
  else if (std::is_same<P, std::chrono::months>::value)
    return "mth";
  else if (std::is_same<P, std::chrono::years>::value)
    return "yrs";
#endif  // C++20

  std::stringstream s;
  s << "Precision type \"" << typeid(P).name()
    << "\" has not been mapped yet.";
  throw std::runtime_error(s.str());
}


/**
 * @brief Returns the fully-qualified name of the
 * given clock type, e.g. "std::chrono::system_clock"
 */
template <typename C>
std::string ClockTypeName() {
  if (std::is_same<C, std::chrono::steady_clock>::value)
    return "std::chrono::steady_clock";
  else if (std::is_same<C, std::chrono::system_clock>::value)
    return "std::chrono::system_clock";
  else if (std::is_same<C, std::chrono::high_resolution_clock>::value)  // Should just be an alias to system or steady (C++11), thus we should
    return "std::chrono::high_resolution_clock";                        // never enter (doesn't hurt to include it either)
#if __cplusplus >= 202002L
  else if (std::is_same<C, std::chrono::utc_clock>::value)
    return "std::chrono::utc_clock";
  else if (std::is_same<C, std::chrono::tai_clock>::value)
    return "std::chrono::tai_clock";
  else if (std::is_same<C, std::chrono::gps_clock>::value)
    return "std::chrono::gps_clock";
  else if (std::is_same<C, std::chrono::file_clock>::value)
    return "std::chrono::file_clock";
  else if (std::is_same<C, std::chrono::local_t>::value)
    return "std::chrono::local_t";
#endif  // C++20

  std::stringstream s;
  s << "Precision type \"" << typeid(C).name()
    << "\" has not been mapped yet.";
  throw std::runtime_error(s.str());
}


/**
 * @brief Returns the fully-qualified name of the given
 * duration type, e.g. "std::chrono::nanoseconds"
 */
template <typename P>
std::string PrecisionTypeName() {
  if (std::is_same<P, std::chrono::nanoseconds>::value)
    return "std::chrono::nanoseconds";
  else if (std::is_same<P, std::chrono::microseconds>::value)
    return "std::chrono::microseconds";
  else if (std::is_same<P, std::chrono::milliseconds>::value)
    return "std::chrono::milliseconds";
  else if (std::is_same<P, std::chrono::seconds>::value)
    return "std::chrono::seconds";
  else if (std::is_same<P, std::chrono::minutes>::value)
    return "std::chrono::minutes";
  else if (std::is_same<P, std::chrono::hours>::value)
    return "std::chrono::hours";
#if __cplusplus >= 202002L
  else if (std::is_same<P, std::chrono::days>::value)
    return "std::chrono::days";
  else if (std::is_same<P, std::chrono::weeks>::value)
    return "std::chrono::weeks";
  else if (std::is_same<P, std::chrono::months>::value)
    return "std::chrono::months";
  else if (std::is_same<P, std::chrono::years>::value)
    return "std::chrono::years";
#endif  // C++20

  std::stringstream s;
  s << "Precision type \"" << typeid(P).name()
    << "\" has not been mapped yet.";
  throw std::runtime_error(s.str());
}


/**
 * Returns the number of ticks for the given
 * std::chrono::duration with a different precision.
 */
template<typename _DurationFrom, typename _DurationTo>
inline double CastToTicks(const _DurationFrom &duration) {
  // Floating-point duration requires no explicit duration_cast
  // https://en.cppreference.com/w/cpp/chrono/duration/duration_cast
  const std::chrono::duration<double, typename _DurationTo::period> target(duration);
  return target.count();
}


/** Returns the number of seconds for the given std::chrono::duration. */
template<typename P>
inline double ToSeconds(const P &duration) {
  return CastToTicks<P, std::chrono::seconds>(duration);
}


/** Returns the number of milliseconds for the given std::chrono::duration. */
template<typename P>
inline double ToMilliseconds(const P &duration) {
  return CastToTicks<P, std::chrono::milliseconds>(duration);
}


/** Returns the number of microseconds for the given std::chrono::duration. */
template<typename P>
inline double ToMicroseconds(const P &duration) {
  return CastToTicks<P, std::chrono::microseconds>(duration);
}


/** Returns the number of nanoseconds for the given std::chrono::duration. */
template<typename P>
inline double ToNanoseconds(const P &duration) {
  return CastToTicks<P, std::chrono::nanoseconds>(duration);
}


/**
 * @brief A stop watch with configurable clock.
 *
 * A stop watch measures the time since you last called @see Start(),
 * or since its construction.
 * Use its ElapsedXXX() methods to get the elapsed time in the
 * corresponding time interval (i.e. precision), e.g. ElapsedSeconds().
 *
 * Duration measurements should use a monotonic clock. Thus, we
 * explicitly default to steady_clock (high_resolution_clock is
 * not guaranteed to be steady).
 */
template <typename C = std::chrono::steady_clock>
class StopWatch {
 public:
  using clock_type = C;  /**< @brief Clock type used by this stop watch. */


  /** @brief C'tor starts the stop watch. */
  StopWatch() {
    Start();
  }


  /** @brief Copies start time point. */
  StopWatch(const StopWatch &other)
    : t_start_(other.t_start_)
  {}


  /** @brief Starts or restarts the stop watch. */
  void Start() {
    t_start_ = clock_type::now();
  }


  /**
   * @brief Returns the elapsed time as ticks of the
   * given time interval (specifed as tick interval, which
   * is a rational fraction - std::ratio - representing the
   * time in seconds from one tick to the next, i.e. similar
   * to the Period template parameter in std::chrono::duration)
   */
  template<typename _ratio>
  double ElapsedAs() const {
    const auto t_end = clock_type::now();
    // Floating-point duration requires no explicit duration_cast
    const std::chrono::duration<double, _ratio> duration = t_end - t_start_;
    return duration.count();
  }


  /**
   * @brief Returns the elapsed time in seconds.
   */
  double ElapsedSeconds() const {
    return ElapsedAs<std::ratio<1>>();
  }


  /**
   * @brief Returns the elapsed time in milliseconds.
   */
  double ElapsedMilliseconds() const {
    return ElapsedAs<std::milli>();
  }


  /**
   * @brief Returns the elapsed time in microseconds.
   */
  double ElapsedMicroseconds() const {
    return ElapsedAs<std::micro>();
  }


  /**
   * @brief Returns the elapsed time in nanoseconds.
   */
  double ElapsedNanoseconds() const {
    return ElapsedAs<std::nano>();
  }


  /**
   * @brief Returns the number of years before this stop watch
   * will overflow.
   */
  double YearsUntilOverflow() const {
    const auto duration_hrs =
        std::chrono::duration_cast<std::chrono::hours>(clock_type::time_point::max() - clock_type::now());
    // Convert to years, similar to C++20's std::chrono::years definition
    return duration_hrs.count() / 8765.82;  // = 24 * 365.2425
  }


  /** @brief Returns true if the used clock is steady (monotonic). */
  bool IsSteady() const {
    return clock_type::is_steady;
  }


  /**
   * @brief Returns a readable clock typeid, for
   * example "std::chrono::steady_clock".
   */
  std::string ClockName() const {
    return ClockTypeName<C>();
  }


 private:
  /** Time point from which we measure the elapsed time. */
  typename clock_type::time_point t_start_;
};

typedef StopWatch<std::chrono::steady_clock> stop_watch;

}  // namespace timing
}  // namespace werkzeugkiste

#endif  // __WERKZEUGKISTE_TIMING_STOPWATCH_H__
