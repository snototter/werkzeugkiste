#ifndef __WERKZEUGKISTE_GEOMETRY_UTILS_H__
#define __WERKZEUGKISTE_GEOMETRY_UTILS_H__

#include <limits>
#include <type_traits>

#include <cmath>

namespace werkzeugkiste {
/// Math utils for 2D/3D geometry.
namespace geometry {

//-----------------------------------------------------------------------------
// Angle conversions.

/// Convert angle from degrees to radians.
inline double deg2rad(double deg) {
  return deg * M_PI / 180.0;
}


/// Convert angle from radians to degrees.
inline double rad2deg(double rad) {
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


/// Uses the machine epsilon to check whether the given number is
/// approximately zero, i.e. computes `|x| < 2*eps` for floating
/// point numbers. Integral types will be compared to zero using standard
/// equality check.
template<typename T>
inline bool eps_zero(T x) {
  if (std::is_floating_point<T>::value)   {
    return std::fabs(x) < (2 * std::numeric_limits<T>::epsilon());
  } else {
    return x == static_cast<T>(0);
  }
}


/// Uses the machine epsilon to check for equality based on the desired
/// precision in ULPs (units in the last place).
/// Caveat: NEVER use this to check a number against 0!
/// For example, 0.0 is NOT eps_equal to 1.1e-16 (mostly - depends on your
/// machine's epsilon...)
/// Check the `GeometryUtilsTest.FloatingPointEquality` test case for some
/// caveats when comparing floating point numbers.
template<typename T>
inline bool eps_equal(T x, T y, int ulp=2) {
  if (std::is_floating_point<T>::value) {
    // Adapted (using fabs) from the STL reference:
    // https://en.cppreference.com/w/cpp/types/numeric_limits/epsilon
    // The machine epsilon has to be scaled to the magnitude of the values used
    // and multiplied by the desired precision in ULPs (units in the last place)
    return std::fabs(x-y) <= std::numeric_limits<T>::epsilon() * std::fabs(x+y) * ulp
    // unless the result is subnormal
           || std::fabs(x-y) < std::numeric_limits<T>::min();
  } else {
    return x == y;
  }
}


//-----------------------------------------------------------------------------
// Templated sign/signum function

/// Signum helper for unsigned types (to avoid compiler warnings).
template <typename T> inline constexpr
int _sgn(T x, std::false_type /*is_signed*/) {
  return T(0) < x;
}


/// Signum helper for signed types (to avoid compiler warnings when using
/// unsigned types).
template <typename T> inline constexpr
int _sgn(T x, std::true_type /*is_signed*/) {
   return (T(0) < x) - (x < T(0));
}


/// Signum function which returns +1 (if x is positive), 0 (if x equals 0),
/// or -1 (if x is negative).
/// This type-safe implementation has been taken from
/// https://stackoverflow.com/a/4609795 by `user79758` (CC BY-SA 4.0).
template <typename T> inline constexpr
int sgn(T x) {
  return _sgn(x, std::is_signed<T>());
}

} // namespace geometry
} // namespace werkzeugkiste

#endif // __WERKZEUGKISTE_GEOMETRY_UTILS_H__
