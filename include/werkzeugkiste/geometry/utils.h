#ifndef WERKZEUGKISTE_GEOMETRY_UTILS_H
#define WERKZEUGKISTE_GEOMETRY_UTILS_H

#include <limits>
#include <type_traits>
#include <stdexcept>
#include <sstream>
#include <cmath>

/// Math utils for 2D/3D geometry.
namespace werkzeugkiste::geometry {

/// Mathematical constants.
namespace constants {

// Pi
template <typename Tp>
inline constexpr Tp pi_tpl = static_cast<std::enable_if_t<
    std::is_floating_point_v<Tp>, Tp>>(3.141592653589793238462643383279502884L);

inline constexpr double pi_d = pi_tpl<double>;
inline constexpr float pi_f = pi_tpl<float>;

// 1 / Pi
template <typename Tp>
inline constexpr Tp inv_pi_tpl = static_cast<std::enable_if_t<
    std::is_floating_point_v<Tp>, Tp>>(0.318309886183790671537767526745028724L);

inline constexpr double inv_pi_d = inv_pi_tpl<double>;
inline constexpr float inv_pi_f = inv_pi_tpl<float>;


// sqrt(2)
template <typename Tp>
inline constexpr Tp sqrt2_tpl = static_cast<std::enable_if_t<
    std::is_floating_point_v<Tp>, Tp>>(1.414213562373095048801688724209698079L);

inline constexpr double sqrt2_d = sqrt2_tpl<double>;
inline constexpr float sqrt2_f = sqrt2_tpl<float>;

}  // namespace constants

//-----------------------------------------------------------------------------
// Angle conversions.

/// Convert angle from degrees to radians.
inline constexpr
double Deg2Rad(double deg) {
  // NOLINTNEXTLINE(*-magic-numbers)
  return (constants::pi_d / 180.0) * deg;
}


inline constexpr
float Deg2Rad(float deg) {
  // NOLINTNEXTLINE(*-magic-numbers)
  return (constants::pi_f / 180.0F) * deg;
}


inline constexpr double Deg2Rad(int deg) {
  return Deg2Rad(static_cast<double>(deg));
}


/// Convert angle from radians to degrees.
inline constexpr
double Rad2Deg(double rad) {
  // NOLINTNEXTLINE(*-magic-numbers)
  return rad * 180.0 * constants::inv_pi_d;
}


inline constexpr
float Rad2Deg(float rad) {
  // NOLINTNEXTLINE(*-magic-numbers)
  return rad * 180.0F * constants::inv_pi_f;
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
template <typename T> inline constexpr
bool IsEpsZero(T x) {
  static_assert(
    std::is_arithmetic_v<T>,
    "Non-arithmetic input type provided for IsEpsZero().");
  if constexpr (std::is_floating_point_v<T>) {
    return std::fabs(x) <= std::numeric_limits<T>::epsilon();
  } else {
    return x == static_cast<T>(0);
  }
}


/// Epsilon equality check for floating point numbers, similar
/// to Python's math.isclose(), see PEP 485, https://peps.python.org/pep-0485/
///
/// Module special case handling, this function returns:
///   (|x-y| <= rel_tol * |x|)
///   or (|x-y| <= rel_tol * |y|)
///   or (|x-y| <= abs_tol)
template <typename TVal, typename TTol = double> inline constexpr
bool IsClose(TVal x, TVal y, TTol relative_tolerance = 1e-9, TTol absolute_tolerance = 0.0) {  // NOLINT
  static_assert(
    std::is_floating_point_v<TVal>,
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


/// Checks if two floating point numbers are "approximately" equal,
/// according to a fixed relative tolerance (1e-9 for double precision,
/// 1e-6 for single precision types), see `IsClose`.
/// As there is no sane default value for the absolute tolerance,
/// only the relative tolerance is checked.
template <typename T> inline constexpr
bool IsEpsEqual(T x, T y) {
  if constexpr(std::is_integral_v<T>) {
    return x == y;
  } else {
    if constexpr(std::is_same_v<float, T>) {
      return IsClose<float, float>(x, y, 1e-6F, 0.0F);
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
    std::is_arithmetic_v<T>,
    "Non-arithmetic input type provided for Sign().");
  return _util_sign(x, std::is_signed<T>());
}

} // namespace werkzeugkiste::geometry

#endif // WERKZEUGKISTE_GEOMETRY_UTILS_H
