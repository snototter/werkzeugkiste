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
// Actually, he posted several helpful and interesting posts on that topic:
// https://randomascii.wordpress.com/category/floating-point/


///// Helper to check if the floating point number x is approximately zero.
//template <typename T>
//inline bool _eps_zero(T x, std::true_type /* is_floating_point */) { // NOLINT
//  return std::fabs(x) < (2 * std::numeric_limits<T>::epsilon());
//}


///// Helper to check if the integral number x is zero.
//template <typename T>
//inline bool _eps_zero(T x, std::false_type /* is_floating_point */) { // NOLINT
//  return x == static_cast<T>(0);
//}


/// Uses the machine epsilon to check whether the given number is
/// approximately zero, i.e. computes `|x| < 2*eps` for floating
/// point numbers. Integral types will be compared to zero using standard
/// equality check.
template<typename T> inline constexpr
bool IsEpsZero(T x) {
  static_assert(
    std::is_arithmetic<T>::value,
    "Non-arithmetic input type provided for IsEpsZero().");
//  return _eps_zero(x, std::is_floating_point<T>());
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
//template <typename T> inline constexpr
//bool _eps_equal(  // NOLINT
//    T x, T y, unsigned int ulp, std::true_type /* is_floating_point */) {
//      T x, T y, T eps, std::true_type /* is_floating_point */) {
//  // Adapted (using fabs) from the STL reference:
//  // https://en.cppreference.com/w/cpp/types/numeric_limits/epsilon
//  // Returns (A) || (B), where:
//  // (A) The machine epsilon has to be scaled to the magnitude of the values used
//  //     and multiplied by the desired precision in ULPs (units in the last place),
//  // (B) unless the result is subnormal.
//  return (
//        (std::fabs(x-y) <= std::numeric_limits<T>::epsilon() * std::fabs(x+y) * static_cast<T>(ulp))
//      || (std::fabs(x-y) < std::numeric_limits<T>::min()));
//FIXME doc - 6 digits single-precision, 10digits double/long precision
template <typename T> inline constexpr
bool _eps_equal(T x, T y) {//, //T relative_tolerance, T absolute_tolerance) {
                //unsigned int ulp) {
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
    constexpr float eps = 0.000001F;
    return diff <= (eps * sum);
  } else {
    constexpr T eps = 1e-10;
    return diff <= (eps * sum);
  }
}


///// Epsilon equality check for non-floating point numbers.
//template <typename T> inline constexpr
//bool _eps_equal(  // NOLINT
//    T x, T y, unsigned int /* ulp */, std::false_type /* is_floating_point*/) {
////  T x, T y, T /* eps */, std::false_type /* is_floating_point */) {
//  return x == y;
//}


/// Uses the machine epsilon to check for equality based on the desired
/// precision in ULPs (units in the last place).
///
/// Caveat: *NEVER* use this to check a number against 0!
/// For example, 0.0 is NOT eps_equal to 1.1e-16 (depends on your
/// machine's epsilon...)
/// Check the `GeometryUtilsTest.FloatingPointEquality` test case for some
/// caveats when comparing floating point numbers.
template<typename T> inline constexpr
bool IsEpsEqual(T x, T y) {
  static_assert(
    std::is_arithmetic<T>::value,
    "Non-arithmetic input type provided for IsEpsEqual().");
  if constexpr (std::is_floating_point<T>::value) {
    return _eps_equal(x, y);
//          x, y,
//          std::numeric_limits<T>::epsilon() * 1000,
//          std::numeric_limits<T>::epsilon());
  } else {
    return x == y;
  }
//  return _eps_equal(x, y, ulp, std::is_floating_point<T>());
//  return _eps_equal(x, y, , std::is_floating_point<T>());
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
