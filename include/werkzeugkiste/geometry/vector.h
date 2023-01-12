#ifndef WERKZEUGKISTE_GEOMETRY_VECTOR_H
#define WERKZEUGKISTE_GEOMETRY_VECTOR_H

#include <stdexcept>
#include <string>
#include <ostream>
#include <initializer_list>
#include <cmath>
#include <vector>
#include <type_traits>

#include <werkzeugkiste/geometry/utils.h>

namespace werkzeugkiste::geometry {

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
template<typename T, int Dim>
class Vec {
 public:
  using value_type = T;  // NOLINT(readability-identifier-naming)
  static constexpr int ndim = Dim;

  //------------------------------------------------- Initialization
  Vec() noexcept {
    for (int i = 0; i < Dim; ++i) {
      val[i] = static_cast<T>(0);
    }
  }


  Vec(T x, T y);
  Vec(T x, T y, T z);
  Vec(T x, T y, T z, T w);

  Vec(std::initializer_list<T> values);

  ~Vec() = default;
  Vec(const Vec<T, Dim>& other);
  Vec(Vec<T, Dim> &&other) noexcept;
  Vec<T, Dim> &operator=(const Vec<T, Dim> &other);
  Vec<T, Dim> &operator=(Vec<T, Dim> &&other) noexcept;


  /// Allow implicitly casting each vector to its
  /// double-precision counterpart.
  operator Vec<double, Dim>() const;
  //TODO make explicit


  /// Returns the homogeneous representation of this vector, *i.e.* the vector
  /// has an additional dimension which is set to 1.
  inline constexpr
  Vec<T, Dim+1> Homogeneous() const {
    Vec<T, Dim+1> vh;
    for (int i = 0; i < Dim; ++i) {
      vh[i] = val[i];
    }
    vh[Dim] = static_cast<T>(1);
    return vh;
  }


  //------------------------------------------------- Value access
  T &operator[](int i);

  const T &operator[](int i) const;


  inline const T &X() const {
    return (*this)[0];
  }


  inline const T &Y() const {
    return (*this)[1];
  }


  inline const T &Width() const {
    if (Dim != 2) {
        throw std::logic_error(
            "Only 2D vectors support member access via Width().");
    }
    return X();
  }


  inline const T &Height() const {
    if (Dim != 2) {
        throw std::logic_error(
            "Only 2D vectors support member access via Height().");
    }
    return Y();
  }


  inline const T &Z() const {
    return (*this)[2];
  }


  inline const T &W() const {
    return (*this)[3];
  }

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
//   * argument (_Tp = T) must be specified. Otherwise, this would
//   * not compile, because at compilation time, T is already
//   * known (from the class definition). Useful SO thread on SFINAE:
//   * https://stackoverflow.com/a/13401982/400948
//   */
//  template<typename _Tp = T>
//  const typename std::enable_if<(dim == 2), _Tp>::type& width() const {
//    return x();
//  }
//
//  template<typename _Tp = T>
//  const typename std::enable_if<(dim == 2), _Tp>::type& height() const {
//    return y();
//  }

  inline T &X() {
    return (*this)[0];
  }


  inline T &Y() {
    return (*this)[1];
  }


  inline T &Width() {
    if (Dim != 2) {
      throw std::logic_error(
          "Only 2D vectors support member access via Width().");
    }
    return X();
  }


  inline T &Height() {
    if (Dim != 2) {
      throw std::logic_error(
          "Only 2D vectors support member access via Height().");
    }
    return Y();
  }


  inline T &Z() {
    return (*this)[2];
  }


  inline T &W() {
    return (*this)[3];
  }


  inline void SetX(T x) {
    (*this)[0] = x;
  }


  inline void SetY(T y) {
    (*this)[1] = y;
  }


  inline void SetWidth(T width) {
    if (Dim != 2) {
      throw std::logic_error(
            "Only 2D vectors support "
            "setting the x dimension via SetWidth().");
    }
    SetX(width);
  }


  inline void SetHeight(T height) {
    if (Dim != 2) {
      throw std::logic_error(
            "Only 2D vectors support "
            "setting the Y dimension via SetHeight().");
    }
    SetY(height);
  }


  inline void SetZ(T z) {
    (*this)[2] = z;
  }


  inline void SetW(T w) {
    (*this)[3] = w;
  }


  /// Holds the values of this vector.
  T val[static_cast<std::size_t>(Dim)] = {0};


  //------------------------------------------------- Arithmetics
  //FIXME disable division on integral types, or implement explicit clipping?
  Vec<T, Dim> &operator+=(const Vec<T, Dim>& rhs);
  Vec<T, Dim> &operator+=(double value);
  Vec<T, Dim> &operator-=(const Vec<T, Dim>& rhs);
  Vec<T, Dim> &operator-=(double value);
  Vec<T, Dim> &operator*=(double scale);
  Vec<T, Dim> &operator/=(double scale);


  /// Returns a vector where each dimension is negated.
  Vec<T, Dim> operator-() const;


  /// Returns the maximum dimension value.
  T MaxValue() const;


  /// Returns the minimum dimension value.
  T MinValue() const;


  /// Returns the index/dimension holding the maximum value.
  int MaxIndex() const;


  /// Returns the index/dimension holding the minimum value.
  int MinIndex() const;


  /// Computes the dot product.
  T Dot(const Vec<T, Dim>& other) const;


  /// Returns the vector cross product. Only supported for 3-dimensional
  /// vectors.
  Vec<T, Dim> Cross(const Vec<T, Dim>& other) const;


  /// Returns the vector's length.
  double Length() const;


  /// Returns the squared vector's length.
  double LengthSquared() const;


  /// Computes the L2 distance between this and the other.
  double Distance(const Vec<T, Dim>& other) const;


  /// Computes the L1 distance between this and the other.
  double DistanceManhattan(const Vec<T, Dim>& other) const;


  /// Returns the direction vector from `this` to `to`.
  Vec<T, Dim> DirectionVector(const Vec<T, Dim>& to) const;


  /// Returns the unit vector.
  Vec<double, Dim> UnitVector() const;

//  //TODO fix return types (unit/length/normalized, etc --> floating point type)
//  //TODO Clock-wise if right-handed coordinate system
  template<typename _Tp = T>
  const Vec<typename std::enable_if<(Dim == 2), _Tp>::type, Dim>
  PerpendicularClockwise() const {
    return Vec<T, Dim>{Y(), -X()};
//    return Vec<T, Dim>{val[1], -val[0]};
  }


  template<typename _Tp = T>
  const Vec<typename std::enable_if<(Dim == 2), _Tp>::type, Dim>
  PerpendicularCounterClockwise() const {
    return Vec<T, Dim>{-Y(), X()};
  }


  /// Returns a human-readable string representation.
  /// If `include_type` is false, it will only return
  /// the coordinates within parentheses, e.g. "(13, 77)".
  std::string ToString(bool include_type = true) const;


  /// Overloaded stream operator.
  friend std::ostream &operator<<(std::ostream &os, const Vec<T, Dim> &vec) {
    os << vec.ToString();
    return os;
  }


  /// Returns the class type name, e.g. "Vec2d".
  static std::string TypeName();

  /// Returns a vector with all coordinates set to `value`.
  static Vec<T, Dim> All(T value);
};


//-------------------------------------------------  Comparison operators
// If you implement another operator, don't forget
// to add the corresponding explicit vector instantiation
// in primitives.cpp

template<typename T, int Dim>
bool operator==(const Vec<T, Dim>& lhs, const Vec<T, Dim>& rhs);

template<typename T, int Dim>
bool operator!=(const Vec<T, Dim>& lhs, const Vec<T, Dim>& rhs);


//-------------------------------------------------  Arithmetic operators
// TODO(new-features) If you implement another operator,
// don't forget to add the corresponding explicit vector
// instantiation in vector.cpp

/// Vector addition.
template<typename T, int Dim>
Vec<T, Dim> operator+(Vec<T, Dim> lhs, const Vec<T, Dim>& rhs);


/// Vector subtraction.
template<typename T, int Dim>
Vec<T, Dim> operator-(Vec<T, Dim> lhs, const Vec<T, Dim>& rhs);


/// Add scalar to each dimension.
template<typename T, int Dim>
Vec<T, Dim> operator+(Vec<T, Dim> lhs, double rhs);


/// Subtract scalar from each dimension.
template<typename T, int Dim>
Vec<T, Dim> operator-(Vec<T, Dim> lhs, double rhs);


/// Multiply (rhs) by scalar.
template<typename T, int Dim>
Vec<T, Dim> operator*(Vec<T, Dim> lhs, double rhs);

/// Multiply (lhs) by scalar.
template<typename T, int Dim>
Vec<T, Dim> operator*(double lhs, Vec<T, Dim> rhs);


/// Divide (scale) by scalar.
template<typename T, int Dim>
Vec<T, Dim> operator/(Vec<T, Dim> lhs, double rhs);


/// Returns the length of the given polygon.
template<typename T, int Dim>
double LengthPolygon(const std::vector<Vec<T, Dim>> &points);


//-------------------------------------------------  Available specializations:
using Vec2d = Vec<double, 2>;
using Vec3d = Vec<double, 3>;
using Vec4d = Vec<double, 4>;

using Vec2i = Vec<int, 2>;
using Vec3i = Vec<int, 3>;


//---------------------------------------------------- Math/Geometry Helpers


/// Computes the determinant of the two 2d vectors.
template <typename T> inline
T Determinant(const Vec<T, 2> &a, const Vec<T, 2> &b) {
  return a.X() * b.Y() - b.X() * a.Y();
}


/// Scalar projection is the length of the vector projection, which is the
/// vector component of a in the direction of b.
/// See also: https://en.wikipedia.org/wiki/Vector_projection
template <typename T, int Dim> inline
T ScalarProjection(const Vec<T, Dim> &a, const Vec<T, Dim> &b) {
  static_assert(
      std::is_floating_point<T>::value,
      "Vector type must be float or double!");
  return a.Dot(b.UnitVector());
}


/// Returns :math:`\operatorname{proj}_{\mathbf{b}} \mathbf{a}`, *i.e.* the
/// projection of a onto b
/// See also: https://en.wikipedia.org/wiki/Vector_projection
template <typename T, int Dim> inline
Vec<T, Dim> VectorProjection(const Vec<T, Dim> &a, const Vec<T, Dim> &b) {
  static_assert(
      std::is_floating_point<T>::value,
      "Vector type must be float or double!");
  // Alternative formulation would be ScalarProjection(a, b) * b.UnitVector(),
  // but we can save some computation via:
  return (a.Dot(b) / b.LengthSquared()) * b;
}


/// Computes the angle (in radians) of a 2d direction vector w.r.t. the
/// positive X axis.
template <typename T> inline
double AngleRadFromDirectionVec(const Vec<T, 2> &vec) {
  // Dot product is proportional to the cosine, whereas
  // the determinant is proportional to the sine.
  // See: https://math.stackexchange.com/a/879474
  const Vec<double, 2> ref{1, 0};
  const Vec<double, 2> unit = vec.UnitVector();
  return std::atan2(Determinant(ref, unit), ref.Dot(unit));
}


/// Computes the angle (in degrees) of a 2d direction vector w.r.t. the
/// positive X axis.
template <typename T> inline
double AngleDegFromDirectionVec(const Vec<T, 2> &vec) {
  return Rad2Deg(AngleRadFromDirectionVec(vec));
}


/// Returns the unit direction vector given its angle (in
/// radians) w.r.t. the positive X axis.
inline Vec2d DirectionVecFromAngleRad(double rad) {
  return Vec2d{std::cos(rad), std::sin(rad)}; // TODO test case to ensure numerical stability at edge cases
}


/// Returns the unit direction vector given its angle (in
/// degrees) w.r.t. the positive X axis.
inline Vec2d DirectionVecFromAngleDeg(double deg) {
  return DirectionVecFromAngleRad(Deg2Rad(deg));
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
template <typename T, int Dim,
          template<typename...> class Container = std::vector>
void MinMaxCoordinates(
    const Container<Vec<T, Dim>> &values,
    Vec<T, Dim> &min, Vec<T, Dim> &max) {
  if (values.empty()) {
    return;
  }

  min = *values.begin();
  max = *values.begin();
  for (const Vec<T, Dim> &v : values) {
    for (int i = 0; i < Dim; ++i) {
      if (v[i] < min[i]) {
        min[i] = v[i];
      }
      if (max[i] < v[i]) {
        max[i] = v[i];
      }
    }
  }
}

} // namespace werkzeugkiste::geometry

#endif // WERKZEUGKISTE_GEOMETRY_VECTOR_H
