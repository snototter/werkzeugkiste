#ifndef __WERKZEUGKISTE_GEOMETRY_VECTOR_H__
#define __WERKZEUGKISTE_GEOMETRY_VECTOR_H__

#include <stdexcept>
#include <string>
#include <ostream>
#include <initializer_list>
#include <cmath>


namespace werkzeugkiste {
namespace geometry {

//------------------------------------------------- Vectors/Coordinates
//TODO(snototter) check detailed testing of vector class (with dev task list)!
/** @brief Template class to represent a vector/coordinate. */
template<typename _Tp, int dim>
class Vec {
 public:
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


  /**
   * @brief Allow casting each vector to its double-precision counterpart.
   * Needed because we work with Cairo, which heavily uses doubles.
   */
  explicit operator Vec<double, dim>() const;


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


  /** @brief Returns a vector where each dimension is negated. */
  Vec<_Tp, dim> operator-() const;


  /** @brief Returns the maximum dimension value. */
  _Tp MaxValue() const;


  /** @brief Returns the minimum dimension value. */
  _Tp MinValue() const;


  /** @brief Returns the index/dimension holding the maximum value. */
  int MaxIndex() const;


  /** @brief Returns the index/dimension holding the minimum value. */
  int MinIndex() const;


  /** @brief Computes the dot product. */
  _Tp Dot(const Vec<_Tp, dim>& other) const;


  /** @brief Returns the vector's length. */
  Vec<_Tp, dim> Cross(const Vec<_Tp, dim>& other) const;


  /** @brief Returns the vector's length. */
  double Length() const;


  /** @brief Returns the squared vector's length. */
  double LengthSquared() const;


  /** @brief Computes the distance between this and the other. */
  double Distance(const Vec<_Tp, dim>& other) const;


  /** @brief Returns the direction vector from 'this' to 'to'. */
  Vec<_Tp, dim> DirectionVector(const Vec<_Tp, dim>& to) const;


  /** @brief Returns the unit vector. */
  Vec<double, dim> UnitVector() const;


  /** @brief Returns a human-readable string representation. */
  std::string ToString() const;


  /** @brief Overloaded stream operator. */
  friend std::ostream &operator<<(std::ostream &os, const Vec<_Tp, dim> &vec) {
    os << vec.ToString();
    return os;
  }


  /** @brief Returns the class type name, e.g. "Vec2d". */
  static std::string TypeName();

  //TODO doc, test, bind
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
// If you implement another operator, don't forget
// to add the corresponding explicit vector instantiation
// in primitives.cpp

/** Vector addition. */
template<typename _Tp, int dim>
Vec<_Tp, dim> operator+(Vec<_Tp, dim> lhs, const Vec<_Tp, dim>& rhs);

/** Vector subtraction. */
template<typename _Tp, int dim>
Vec<_Tp, dim> operator-(Vec<_Tp, dim> lhs, const Vec<_Tp, dim>& rhs);


template<typename _Tp, int dim>
Vec<_Tp, dim> operator-(Vec<_Tp, dim> lhs, double rhs);

/** Multiply (rhs) by scalar. */
template<typename _Tp, int dim>
Vec<_Tp, dim> operator*(Vec<_Tp, dim> lhs, double rhs);

/** Multiply (lhs) by scalar. */
template<typename _Tp, int dim>
Vec<_Tp, dim> operator*(double lhs, Vec<_Tp, dim> rhs);

/** Divide (scale) by scalar. */
template<typename _Tp, int dim>
Vec<_Tp, dim> operator/(Vec<_Tp, dim> lhs, double rhs);


//-------------------------------------------------  Available specializations:
typedef Vec<double, 2> Vec2d;
typedef Vec<double, 3> Vec3d;
typedef Vec<double, 4> Vec4d;

typedef Vec<int, 2> Vec2i;
typedef Vec<int, 3> Vec3i;


//---------------------------------------------------- Math/Geometry Helpers
//TODO MOVE TO GEOMETRY
/** @brief Project point onto line. */
Vec2d ProjectPointOntoLine(const Vec2d &pt, const Vec2d &line_from, const Vec2d &line_to);


/** Computes the determinant of the two 2d vectors. */
double Determinant(const Vec2d &a, const Vec2d &b);


/** Computes the angle (in radians) of a 2d direction vector w.r.t. the positive X axis. */
double AngleRadFromDirectionVec(const Vec2d &vec);


/** Computes the angle (in degrees) of a 2d direction vector w.r.t. the positive X axis. */
double AngleDegFromDirectionVec(const Vec2d &vec);


/** Computes the direction vector given its angle (in radians) w.r.t. the positive X axis. */
Vec2d DirectionVecFromAngleRad(double rad);


/** Computes the direction vector given its angle (in radians) w.r.t. the positive X axis. */
Vec2d DirectionVecFromAngleDeg(double deg);


} // namespace geometry
} // namespace werkzeugkiste

#endif // __WERKZEUGKISTE_GEOMETRY_VECTOR_H__
