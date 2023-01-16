#ifndef WERKZEUGKISTE_GEOMETRY_UTILS_H
#define WERKZEUGKISTE_GEOMETRY_UTILS_H

#include <limits>
#include <type_traits>
#include <stdexcept>
#include <sstream>
#include <cmath>

/// Math utils for 2D/3D geometry.
namespace werkzeugkiste::geometry {

//-----------------------------------------------------------------------------
// Angle conversions.

/// Convert angle from degrees to radians.
inline constexpr
double Deg2Rad(double deg) {
  // NOLINTNEXTLINE(*-magic-numbers)
  return deg * M_PI / 180.0;
}


/// Convert angle from radians to degrees.
inline constexpr
double Rad2Deg(double rad) {
  // NOLINTNEXTLINE(*-magic-numbers)
  return rad * 180.0 / M_PI;
}


//-----------------------------------------------------------------------------
// Number comparisons (to properly deal with floating point numbers).
//
// To learn more about the caveats of floating point math in programming, check
// out Bruce Dawson's very informative blog post:
// https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
// Other useful resources:
// https://bitbashing.io/comparing-floats.html
// https://peps.python.org/pep-0485


/// Uses the machine epsilon to check whether the given number is
/// approximately zero, i.e. computes `|x| <= eps` for floating
/// point numbers. Integral types will be compared to zero using the
/// default equality check.
template<typename T> inline constexpr
bool IsEpsZero(T x) {
  static_assert(
    std::is_arithmetic<T>::value,
    "Non-arithmetic input type provided for IsEpsZero().");
  if constexpr (std::is_floating_point<T>::value) {
    return std::fabs(x) <= std::numeric_limits<T>::epsilon();
  } else {
    return x == static_cast<T>(0);
  }
}


/// Computes the floating point precision at the given value via
/// the next/previous representable number. This can be used as
/// a flexible epsilon in comparisons.
template <typename T> inline constexpr
typename std::enable_if<
    std::is_floating_point<T>::value, T>::type ExpectedPrecision(T x) {
  T next = std::nextafter(x, std::numeric_limits<T>::infinity());
  T prev = std::nextafter(x, -std::numeric_limits<T>::infinity());
  return std::max(next - x, x - prev);
}

/// Epsilon equality check for floating point numbers.
/// Uses a relative tolerance of 1e-6 for floats and 1e-10 for double or
/// long double floating point numbers. This tolerance is scaled by
/// the magnitude |x+y|.
template <typename T> inline constexpr
bool _eps_equal(T x, T y) {
  if (std::isinf(x) || std::isinf(y)) {
    return false;
  }

  const auto diff = std::fabs(x - y);
  if (diff < std::numeric_limits<T>::min()) {
    // Difference is subnormal
    return true;
  }

  const auto sum = std::fabs(x + y);
  if constexpr (std::is_same<float, T>::value) {
    constexpr float eps {0.000001F};
    return diff <= (eps * sum);
  } else {
    constexpr T eps {1e-10};
    return diff <= (eps * sum);
  }
}


/// Returns true if the two numbers are approximately the same, i.e. if
/// they are "close enough". This check should NOT be used to compare a
/// non-zero number against 0!
///
/// Integral numbers are compared via the default equality comparison, i.e.
/// they are either exactly equal or not.
///
/// Floating point numbers are compared using a relative tolerance, scaled
/// by their magnitude. The relative tolerance (epsilon) is 1e-6 for floats
/// and 1e-10 for double/long double floating point numbers.
/// This check will then return the result of
///   `|x-y| <= epsilon * |x+y|`
template<typename T> inline constexpr
bool IsEpsEqual(T x, T y) {
  static_assert(
    std::is_arithmetic<T>::value,
    "Non-arithmetic input type provided for IsEpsEqual().");
  if constexpr (std::is_floating_point<T>::value) {
    return _eps_equal(x, y);
  } else {
    return x == y;
  }
}



//-----------------------------------------------------------------------------
// Templated sign/signum function

/// Signum helper for unsigned types (to avoid compiler warnings).
template <typename T> inline constexpr
int _sgn(T x, std::false_type /*is_signed*/) {  // NOLINT
  return static_cast<T>(0) < x;
}


/// Signum helper for signed types (to avoid compiler warnings when using
/// unsigned types).
template <typename T> inline constexpr
int _sgn(T x, std::true_type /*is_signed*/) {  // NOLINT
   return (static_cast<T>(0) < x) - (x < static_cast<T>(0));
}


/// Signum function which returns +1 (if x is positive), 0 (if x equals 0),
/// or -1 (if x is negative).
/// This type-safe implementation is based on
/// https://stackoverflow.com/a/4609795 by `user79758` (CC BY-SA 4.0).
template <typename T> inline constexpr
int Sign(T x) {
  static_assert(
    std::is_arithmetic<T>::value,
    "Non-arithmetic input type provided for Sign().");
  return _sgn(x, std::is_signed<T>());
}

} // namespace werkzeugkiste::geometry

#endif // WERKZEUGKISTE_GEOMETRY_UTILS_H
