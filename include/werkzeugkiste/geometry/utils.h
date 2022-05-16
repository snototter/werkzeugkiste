#ifndef __WERKZEUGKISTE_GEOMETRY_UTILS_H__
#define __WERKZEUGKISTE_GEOMETRY_UTILS_H__

#include <limits>
#include <type_traits>

#include <math.h>

namespace werkzeugkiste {
/**
 TODO doc
 */
namespace geometry {

inline double deg2rad(double deg) {
  return deg * M_PI / 180.0;
}


inline double rad2deg(double rad) {
  return rad * 180.0 / M_PI;
}


//TODO [ ] add documentation
//TODO [ ] add C++ test (tests/xxx_test.cpp)
// Informative blog post about the caveats:
// https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
template<typename T>
inline bool eps_zero(T x) {
  if (std::is_floating_point<T>::value)   {
    return std::fabs(x) < 2*std::numeric_limits<T>::epsilon();
  } else {
    return x == static_cast<T>(0);
  }
}


//DONE [x] add documentation
//TODO [ ] add C++ test (tests/xxx_test.cpp)
/**
 * @brief Uses the machine epsilon to check for equality based on the desired
 * precision in ULPs (units in the last place).
 * Watch out: never use to check against 0! For example, 0.0 is NOT eps_equal to 1.1e-16!
 */
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

} // namespace geometry
} // namespace werkzeugkiste

#endif // __WERKZEUGKISTE_GEOMETRY_UTILS_H__
