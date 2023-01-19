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


inline constexpr
float Deg2Rad(float deg) {
  // NOLINTNEXTLINE(*-magic-numbers)
  return deg * 3.1415926535F / 180.0F;
}


inline constexpr double Deg2Rad(int deg) {
  return Deg2Rad(static_cast<double>(deg));
}


/// Convert angle from radians to degrees.
inline constexpr
double Rad2Deg(double rad) {
  // NOLINTNEXTLINE(*-magic-numbers)
  return rad * 180.0 / M_PI;
}


inline constexpr
float Rad2Deg(float rad) {
  // NOLINTNEXTLINE(*-magic-numbers)
  return rad * 180.0F / 3.1415926535F;
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


///// Computes the floating point precision at the given value via
///// the next/previous representable number. This can be used as
///// a flexible epsilon in comparisons.
//template <typename T> inline constexpr
//typename std::enable_if<
//    std::is_floating_point<T>::value, T>::type ExpectedPrecision(T x) {
//  T next = std::nextafter(x, std::numeric_limits<T>::infinity());
//  T prev = std::nextafter(x, -std::numeric_limits<T>::infinity());
//  return std::max(next - x, x - prev);
//}


///// Epsilon equality check for floating point numbers.
///// Uses a relative tolerance of 1e-6 for floats and 1e-10 for double or
///// long double floating point numbers. This tolerance is scaled by
///// the magnitude |x+y|.
//template <typename T> inline constexpr
//bool _util_eps_equal(T x, T y, std::true_type /* is_floating_point */) {  // NOLINT
//  if (std::isinf(x) || std::isinf(y)) {
//    return false;
//  }

//  const auto diff = std::fabs(x - y);
//  if (diff < std::numeric_limits<T>::min()) {
//    // Difference is subnormal
//    return true;
//  }

//  const auto scale = std::max(
//        std::max(std::fabs(x), std::fabs(y)), std::fabs(x + y));
//  if constexpr (std::is_same<float, T>::value) {
//    constexpr float eps {0.00001F};
//    return diff <= (eps * scale);
//  } else {
//    constexpr T eps {1e-9};
//    return diff <= (eps * scale);
////    abs(a-b) <= max( rel_tol * max(abs(a), abs(b)), abs_tol ) FIXME https://peps.python.org/pep-0485/
//  }
//}


///// Overloaded template for integral number types.
//template <typename T> inline constexpr
//bool _util_eps_equal(T x, T y, std::false_type /* is_floating_point */) {  // NOLINT
//  return x == y;
//}


///// Returns true if the two numbers are approximately the same, i.e. if
///// they are "close enough". This check should NOT be used to compare a
///// non-zero number against 0!
/////
///// Integral numbers are compared via the default equality comparison, i.e.
///// they are either exactly equal or not.
/////
///// Floating point numbers are compared using a relative tolerance, scaled
///// by their magnitude. The relative tolerance (epsilon) is 1e-5 for floats
///// and 1e-9 for double/long double floating point numbers.
///// This check will then return the result of
/////   `|x-y| <= epsilon * |x+y|`
//template<typename T> inline constexpr
//bool IsEpsEqual(T x, T y) {
//  static_assert(
//    std::is_arithmetic<T>::value,
//    "Non-arithmetic input type provided for IsEpsEqual().");
//  return _util_eps_equal(x, y, std::is_floating_point<T>());
//}


/// Epsilon equality check for floating point numbers, similar
/// to Python's math.isclose(), see PEP 485, https://peps.python.org/pep-0485/
template <typename TVal, typename TTol = double> inline constexpr
bool IsClose(TVal x, TVal y, TTol relative_tolerance = 1e-9, TTol absolute_tolerance = 0.0) {  // NOLINT
  static_assert(
    std::is_floating_point<TVal>(),
    "Approximately equal check requires floating point types!");

  if (std::isinf(x) || std::isinf(y)) {
    return false;
  }

  const auto diff = std::fabs(x - y);
  if (diff < std::numeric_limits<TVal>::min()) {
    // Difference is subnormal
    return true;
  }

  return (diff <= std::max(
            relative_tolerance * std::max(std::fabs(x), std::fabs(y)),
            absolute_tolerance));
}


template <typename T> inline constexpr
bool IsEpsEqual(T x, T y) {
  if constexpr(std::is_integral<T>::value) {
    return x == y;
  } else {
    if constexpr(std::is_same<float, T>::value) {
      return IsClose<float, float>(x, y, 0.00001F, 0.0F);
    } else {
      return IsClose(x, y, 1e-9, 0.0);
    }
  }
}



//-----------------------------------------------------------------------------
// Templated sign/signum function

/// Signum helper for unsigned types (to avoid compiler warnings).
template <typename T> inline constexpr
int _util_sign(T x, std::false_type /*is_signed*/) {  // NOLINT
  return static_cast<T>(0) < x;
}


/// Signum helper for signed types (to avoid compiler warnings when using
/// unsigned types).
template <typename T> inline constexpr
int _util_sign(T x, std::true_type /*is_signed*/) {  // NOLINT
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
  return _util_sign(x, std::is_signed<T>());
}

} // namespace werkzeugkiste::geometry

#endif // WERKZEUGKISTE_GEOMETRY_UTILS_H
