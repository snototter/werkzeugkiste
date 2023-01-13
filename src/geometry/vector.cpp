#include <sstream>
#include <iomanip>
#include <type_traits>
#include <stdexcept>
#include <cstdlib>
#include <cassert>

#include <werkzeugkiste/geometry/vector.h>
#include <werkzeugkiste/geometry/utils.h>

namespace werkzeugkiste::geometry {

//template<typename T, std::size_t Dim>
//std::string Vec<T, Dim>::ToString(
//    bool include_type,
//    int fixed_precision) const {
//  std::ostringstream s;
//  if (include_type) {
//    s << Vec<T, Dim>::TypeName();
//  }
//  s << '(' << std::fixed << std::setprecision(fixed_precision);

//  for (std::size_t i = 0; i < Dim; ++i) {
//    s << val[i];
//    if (i < (Dim - 1)) {
//      s << ", ";
//    }
//  }

//  s << ')';
//  return s.str();
//}


//---------------------------------------------------- Vector operators

//template<typename T, std::size_t Dim>
//Vec<double, Dim> operator/(Vec<T, Dim> lhs, double rhs) {
//  auto result = static_cast<Vec<double, Dim>>(lhs);
//  result /= rhs;
//  return result;
//}


//----------------------------------------------------
// Explicit instantiations:
template class Vec<double, 2>;
template class Vec<double, 3>;
template class Vec<double, 4>;

template class Vec<int32_t, 2>;
template class Vec<int32_t, 3>;

// Comparison Vec2d
template bool operator==(const Vec2d& lhs, const Vec2d& rhs);
template bool operator!=(const Vec2d& lhs, const Vec2d& rhs);
// Arithmetic Vec2d
template Vec2d operator+(Vec2d lhs, const Vec2d& rhs);
template Vec2d operator+(Vec2d lhs, double rhs);
template Vec2d operator-(Vec2d lhs, const Vec2d& rhs);
template Vec2d operator-(Vec2d lhs, double rhs);
template Vec2d operator*(Vec2d lhs, double scale);
//template Vec2d operator*(Vec2d lhs, int scale);
template Vec2d operator*(double scale, Vec2d rhs);
//template Vec2d operator*(int scale, Vec2d rhs);
template Vec2d operator/(Vec2d lhs, double rhs);


// Comparison Vec3d
template bool operator==(const Vec3d& lhs, const Vec3d& rhs);
template bool operator!=(const Vec3d& lhs, const Vec3d& rhs);
// Arithmetic Vec3d
template Vec3d operator+(Vec3d lhs, const Vec3d& rhs);
template Vec3d operator+(Vec3d lhs, double rhs);
template Vec3d operator-(Vec3d lhs, const Vec3d& rhs);
template Vec3d operator-(Vec3d lhs, double rhs);
template Vec3d operator*(Vec3d lhs, double scale);
template Vec3d operator*(double scale, Vec3d rhs);
template Vec3d operator/(Vec3d lhs, double rhs);


// Comparison Vec4d
template bool operator==(const Vec4d& lhs, const Vec4d& rhs);
template bool operator!=(const Vec4d& lhs, const Vec4d& rhs);
// Arithmetic Vec4d
template Vec4d operator+(Vec4d lhs, const Vec4d& rhs);
template Vec4d operator+(Vec4d lhs, double rhs);
template Vec4d operator-(Vec4d lhs, const Vec4d& rhs);
template Vec4d operator-(Vec4d lhs, double rhs);
template Vec4d operator*(Vec4d lhs, double scale);
template Vec4d operator*(double scale, Vec4d rhs);
template Vec4d operator/(Vec4d lhs, double rhs);


// Comparison Vec2i
template bool operator==(const Vec2i& lhs, const Vec2i& rhs);
template bool operator!=(const Vec2i& lhs, const Vec2i& rhs);
// Arithmetic Vec2i
template Vec2i operator+(Vec2i lhs, const Vec2i& rhs);
template Vec2i operator+(Vec2i lhs, int32_t rhs);
template Vec2i operator-(Vec2i lhs, const Vec2i& rhs);
template Vec2i operator-(Vec2i lhs, int32_t rhs);
template Vec2i operator*(Vec2i lhs, int32_t scale);
template Vec2i operator*(int32_t scale, Vec2i rhs);
// Division is not supported for integral types

// Comparison Vec3i
template bool operator==(const Vec3i& lhs, const Vec3i& rhs);
template bool operator!=(const Vec3i& lhs, const Vec3i& rhs);
// Arithmetic Vec3i
template Vec3i operator+(Vec3i lhs, const Vec3i& rhs);
template Vec3i operator+(Vec3i lhs, int32_t rhs);
template Vec3i operator-(Vec3i lhs, const Vec3i& rhs);
template Vec3i operator-(Vec3i lhs, int32_t rhs);
template Vec3i operator*(Vec3i lhs, int32_t scale);
template Vec3i operator*(int32_t scale, Vec3i rhs);
// Division is not supported for integral types

} // namespace werkzeugkiste::geometry
