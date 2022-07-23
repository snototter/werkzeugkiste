#ifndef __WERKZEUGKISTE_GEOMETRY_VECTOR_H__
#define __WERKZEUGKISTE_GEOMETRY_VECTOR_H__

#include <stdexcept>
#include <string>
#include <ostream>
#include <initializer_list>
#include <cmath>
#include <vector>
#include <type_traits>

#include <werkzeugkiste/geometry/utils.h>

namespace werkzeugkiste {
namespace geometry {

//------------------------------------------------- Vectors/Coordinates

/// Template class to represent a vector/coordinate.
/// Provides named access for the first 4 dimensions as
/// `x`, `y`, `z` and `w`.
///
/// Any dimension can be accessed via random access,
/// i.e. `operator[]`, or via its `val` array member.
///
/// 2D vectors additionally provide access via
/// `width`/`height`, so using them to hold
/// dimensions/size feels lexically correct.
template<typename _Tp, int dim>
class Vec {
 public:
  using value_type = _Tp;
  static constexpr int ndim = dim;

  //------------------------------------------------- Initialization
  Vec();
  Vec(_Tp x, _Tp y);
  Vec(_Tp x, _Tp y, _Tp z);
  Vec(_Tp x, _Tp y, _Tp z, _Tp w);
  Vec(std::initializer_list<_Tp> values);

  ~Vec() {}

  Vec(const Vec<_Tp, dim>& other);
  Vec(Vec<_Tp, dim> &&other) noexcept;
  Vec<_Tp, dim> &operator=(const Vec<_Tp, dim> &other);
  Vec<_Tp, dim> &operator=(Vec<_Tp, dim> &&other) noexcept;


  /// Allow explicitly casting each vector to its
  /// double-precision counterpart.
  explicit operator Vec<double, dim>() const;


  /// Returns the homogeneous representation of this vector, *i.e.* the vector
  /// has an additional dimension which is set to 1.
  Vec<_Tp, dim+1> Homogeneous() const;


  //------------------------------------------------- Value access
  const _Tp &operator[](int i) const;
  _Tp &operator[](int i);

  const _Tp &x() const;
  const _Tp &y() const;
  const _Tp &width() const;
  const _Tp &height() const;
  const _Tp &z() const;
  const _Tp &w() const;

//  /**
//   * Vectors with 2 dimensions can also define a size.
//   * For clarity, I want to be able to access 'size' elements
//   * as width() and height().
//   *
//   * ---- This worked as expected, but I opted against ----
//   * ---- it, because it made the python bindings much ----
//   * ---- more complex (and I prefer a simple python   ----
//   * ---- wrapper + some extra methods on 3+ dim Vecs  ----
//   * ---- over a "clean" template interface)           ----
//   *
//   * Note that to perform SFINAE, the dummy template
//   * argument (T = _Tp) must be specified. Otherwise, this would
//   * not compile, because at compilation time, _Tp is already
//   * known (from the class definition). Useful SO thread on SFINAE:
//   * https://stackoverflow.com/a/13401982/400948
//   */
//  template<typename T = _Tp>
//  const typename std::enable_if<(dim == 2), T>::type& width() const {
//    return x();
//  }
//
//  template<typename T = _Tp>
//  const typename std::enable_if<(dim == 2), T>::type& height() const {
//    return y();
//  }

  _Tp &x();
  _Tp &y();
  _Tp &width();
  _Tp &height();
  _Tp &z();
  _Tp &w();


  void SetX(_Tp x);
  void SetY(_Tp y);
  void SetWidth(_Tp width);
  void SetHeight(_Tp height);
  void SetZ(_Tp z);
  void SetW(_Tp w);


  _Tp val[dim];  ///< Holds the values of this vector.


  //------------------------------------------------- Arithmetics
  Vec<_Tp, dim> &operator+=(const Vec<_Tp, dim>& rhs);
  Vec<_Tp, dim> &operator+=(double value);
  Vec<_Tp, dim> &operator-=(const Vec<_Tp, dim>& rhs);
  Vec<_Tp, dim> &operator-=(double value);
  Vec<_Tp, dim> &operator*=(double scale);
  Vec<_Tp, dim> &operator/=(double scale);


  /// Returns a vector where each dimension is negated.
  Vec<_Tp, dim> operator-() const;


  /// Returns the maximum dimension value.
  _Tp MaxValue() const;


  /// Returns the minimum dimension value.
  _Tp MinValue() const;


  /// Returns the index/dimension holding the maximum value.
  int MaxIndex() const;


  /// Returns the index/dimension holding the minimum value.
  int MinIndex() const;


  /// Computes the dot product.
  _Tp Dot(const Vec<_Tp, dim>& other) const;


  /// Returns the vector cross product. Only supported for 3-dimensional
  /// vectors.
  Vec<_Tp, dim> Cross(const Vec<_Tp, dim>& other) const;


  /// Returns the vector's length.
  double Length() const;


  /// Returns the squared vector's length.
  double LengthSquared() const;


  /// Computes the distance between this and the other.
  double Distance(const Vec<_Tp, dim>& other) const;


  /// Returns the direction vector from `this` to `to`.
  Vec<_Tp, dim> DirectionVector(const Vec<_Tp, dim>& to) const;


  /// Returns the unit vector.
  Vec<double, dim> UnitVector() const;


  /// Returns a human-readable string representation.
  /// If `include_type` is false, it will only return
  /// the coordinates within parentheses, e.g. "(13, 77)".
  std::string ToString(bool include_type = true) const;


  /// Overloaded stream operator.
  friend std::ostream &operator<<(std::ostream &os, const Vec<_Tp, dim> &vec) {
    os << vec.ToString();
    return os;
  }


  /// Returns the class type name, e.g. "Vec2d".
  static std::string TypeName();

  /// Returns a vector with all coordinates set to `value`.
  static Vec<_Tp, dim> All(_Tp value);
};


//-------------------------------------------------  Comparison operators
// If you implement another operator, don't forget
// to add the corresponding explicit vector instantiation
// in primitives.cpp

template<typename _Tp, int dim>
bool operator==(const Vec<_Tp, dim>& lhs, const Vec<_Tp, dim>& rhs);

template<typename _Tp, int dim>
bool operator!=(const Vec<_Tp, dim>& lhs, const Vec<_Tp, dim>& rhs);


//-------------------------------------------------  Arithmetic operators
// TODO(new-features) If you implement another operator,
// don't forget to add the corresponding explicit vector
// instantiation in vector.cpp

/// Vector addition.
template<typename _Tp, int dim>
Vec<_Tp, dim> operator+(Vec<_Tp, dim> lhs, const Vec<_Tp, dim>& rhs);


/// Vector subtraction.
template<typename _Tp, int dim>
Vec<_Tp, dim> operator-(Vec<_Tp, dim> lhs, const Vec<_Tp, dim>& rhs);


/// Add scalar to each dimension.
template<typename _Tp, int dim>
Vec<_Tp, dim> operator+(Vec<_Tp, dim> lhs, double rhs);


/// Subtract scalar from each dimension.
template<typename _Tp, int dim>
Vec<_Tp, dim> operator-(Vec<_Tp, dim> lhs, double rhs);


/// Multiply (rhs) by scalar.
template<typename _Tp, int dim>
Vec<_Tp, dim> operator*(Vec<_Tp, dim> lhs, double rhs);

/// Multiply (lhs) by scalar.
template<typename _Tp, int dim>
Vec<_Tp, dim> operator*(double lhs, Vec<_Tp, dim> rhs);


/// Divide (scale) by scalar.
template<typename _Tp, int dim>
Vec<_Tp, dim> operator/(Vec<_Tp, dim> lhs, double rhs);


/// Returns the length of the given polygon.
template<typename _Tp, int dim>
double LengthPolygon(const std::vector<Vec<_Tp, dim>> &points);


//-------------------------------------------------  Available specializations:
typedef Vec<double, 2> Vec2d;
typedef Vec<double, 3> Vec3d;
typedef Vec<double, 4> Vec4d;

typedef Vec<int, 2> Vec2i;
typedef Vec<int, 3> Vec3i;


//---------------------------------------------------- Math/Geometry Helpers


/// Computes the determinant of the two 2d vectors.
template <typename _Tp> inline
_Tp Determinant(const Vec<_Tp, 2> &a, const Vec<_Tp, 2> &b) {
  return a.x() * b.y() - b.x() * a.y();
}


/// Scalar projection is the length of the vector projection, which is the
/// vector component of a in the direction of b.
/// See also: https://en.wikipedia.org/wiki/Vector_projection
template <typename _Tp, int dim> inline
_Tp ScalarProjection(const Vec<_Tp, dim> &a, const Vec<_Tp, dim> &b) {
  static_assert(
      std::is_floating_point<_Tp>::value,
      "Vector type must be float or double!");
  return a.Dot(b.UnitVector());
}


/// Returns :math:`\operatorname{proj}_{\mathbf{b}} \mathbf{a}`, *i.e.* the
/// projection of a onto b
/// See also: https://en.wikipedia.org/wiki/Vector_projection
template <typename _Tp, int dim> inline
Vec<_Tp, dim> VectorProjection(const Vec<_Tp, dim> &a, const Vec<_Tp, dim> &b) {
  static_assert(
      std::is_floating_point<_Tp>::value,
      "Vector type must be float or double!");
  // Alternative formulation would be ScalarProjection(a, b) * b.UnitVector(),
  // but we can save some computation via:
  return (a.Dot(b) / b.LengthSquared()) * b;
}


/// Computes the angle (in radians) of a 2d direction vector w.r.t. the
/// positive X axis.
template <typename _Tp> inline
double AngleRadFromDirectionVec(const Vec<_Tp, 2> &vec) {
  // Dot product is proportional to the cosine, whereas
  // the determinant is proportional to the sine.
  // See: https://math.stackexchange.com/a/879474
  const Vec<double, 2> ref(1, 0);
  const Vec<double, 2> unit = vec.UnitVector();
  return std::atan2(Determinant(ref, unit), ref.Dot(unit));
}


/// Computes the angle (in degrees) of a 2d direction vector w.r.t. the
/// positive X axis.
template <typename _Tp> inline
double AngleDegFromDirectionVec(const Vec<_Tp, 2> &vec) {
  return rad2deg(AngleRadFromDirectionVec(vec));
}


/// Returns the unit direction vector given its angle (in
/// radians) w.r.t. the positive X axis.
inline Vec2d DirectionVecFromAngleRad(double rad) {
  return Vec2d(std::cos(rad), std::sin(rad)); // TODO test case to ensure numerical stability at edge cases
}


/// Returns the unit direction vector given its angle (in
/// degrees) w.r.t. the positive X axis.
inline Vec2d DirectionVecFromAngleDeg(double deg) {
  return DirectionVecFromAngleRad(deg2rad(deg));
}



/// Rotates the vector by the given radians, assuming a right-handed(!)
/// coordinate system.
inline Vec2d RotateVector(const Vec2d &vec, double theta) {
  // 2D rotation matrix R = [[ct, -st], [st, ct]]
  const double ct = std::cos(theta);
  const double st = std::sin(theta);
  return Vec2d{
    (ct * vec.val[0]) - (st * vec.val[1]),
    (st * vec.val[0]) + (ct * vec.val[1])
  };
}


/// Rotates the vector by the given radians about the given rotation
/// center, assuming a right-handed(!) coordinate system.
inline Vec2d RotateVector(
    const Vec2d &vec, const Vec2d &rotation_center, double theta) {
  return RotateVector(vec - rotation_center, theta) + rotation_center;
}



/// Computes the minimum/maximum along each dimension.
///
/// Useful to get axis-aligned bounding boxes, a starting
/// point for hull computations, etc.
template <typename _Tp, int dim,
          template<typename...> class Container = std::vector>
void MinMaxCoordinates(
    const Container<Vec<_Tp, dim>> &values,
    Vec<_Tp, dim> &min, Vec<_Tp, dim> &max) {
  if (values.empty()) {
    return;
  }

  min = *values.begin();
  max = *values.begin();
  for (const Vec<_Tp, dim> &v : values) {
    for (int i = 0; i < dim; ++i) {
      if (v[i] < min[i]) {
        min[i] = v[i];
      }
      if (max[i] < v[i]) {
        max[i] = v[i];
      }
    }
  }
}
} // namespace geometry
} // namespace werkzeugkiste

#endif // __WERKZEUGKISTE_GEOMETRY_VECTOR_H__
