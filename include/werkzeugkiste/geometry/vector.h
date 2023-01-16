#ifndef WERKZEUGKISTE_GEOMETRY_VECTOR_H
#define WERKZEUGKISTE_GEOMETRY_VECTOR_H

#include <stdexcept>
#include <string>
#include <ostream>
#include <iomanip>
#include <initializer_list>
#include <cmath>
#include <limits>
#include <vector>
#include <cstdint>
#include <type_traits>

#include <werkzeugkiste/geometry/utils.h>
#include <werkzeugkiste/werkzeugkiste_export.h>

namespace werkzeugkiste::geometry {

//------------------------------------------------- Vectors/Coordinates

//TODO update doc
// Operator overloading megathread on SO:
// https://stackoverflow.com/questions/4421706/what-are-the-basic-rules-and-idioms-for-operator-overloading

/// Template class to represent a vector/coordinate.
///
/// Any dimension can be accessed via random access,
/// i.e. `operator[]`, or via its publicly exposed `val` array member.
///
/// 2D vectors additionally provide access via `Width`/`Height`. Thus, using
/// them to hold 2D dimensions feels lexically correct.
template<typename T, std::size_t Dim>
WERKZEUGKISTE_EXPORT
class Vec {
 public:
  static_assert(
    std::is_signed<T>::value,
    "Only signed arithmetic types are supported!");


  static_assert(
    Dim > 0,
    "Dimension of Vec type must be > 0!");


  // NOLINTNEXTLINE(readability-identifier-naming)
  using value_type = T;


  // NOLINTNEXTLINE(readability-identifier-naming)
  using index_type = std::size_t;


  // NOLINTNEXTLINE(readability-identifier-naming)
  static constexpr std::size_t ndim = Dim;


  //------------------------------------------------- Initialization

  /// Default constructor initializes all fields with 0.
  Vec() noexcept {
    for (index_type i = 0; i < Dim; ++i) {
      val[i] = static_cast<T>(0);
    }
  }


  /// Convenience (x, y) constructor for 2D vector.
  template<typename Tp = T>
  Vec(typename std::enable_if<(Dim == 2), Tp>::type x,
      Tp y) noexcept {
    val[0] = x;
    val[1] = y;
  }


  /// Convenience (x, y, z) constructor for 3D vector.
  template<typename Tp = T>
  Vec(typename std::enable_if<(Dim == 3), Tp>::type x,
      Tp y, Tp z) noexcept {
    val[0] = x;
    val[1] = y;
    val[2] = z;
  }


  /// Convenience (x, y, z, w) constructor for 4D vector.
  template<typename Tp = T>
  Vec(typename std::enable_if<(Dim == 4), Tp>::type x,
      Tp y, Tp z, Tp w) noexcept {
    val[0] = x;
    val[1] = y;
    val[2] = z;
    val[3] = w;
  }


  /// Constructor accepting an `initializer_list` with
  /// either 0 or `Dim` entries.
  Vec(std::initializer_list<T> values) {
    if ((values.size() != 0) &&
        (values.size() != static_cast<std::size_t>(Dim))) {
      std::ostringstream s;
      s << "You cannot initialize " << TypeName()
        << " with " << values.size() << " values. The initializer list must"
           " either be empty, or contain " << Dim << " values!";
      throw std::invalid_argument(s.str());
    }

    if (values.size() == 0) {
      for (index_type i = 0; i < Dim; ++i) {
        val[i] = static_cast<T>(0);
      }
    } else {
      for (index_type i = 0; i < values.size(); ++i) {
        val[i] = values.begin()[i];
      }
    }
  }


  /// Default destruction is fine - no special cleanup procedure required.
  ~Vec() = default;


  /// Copy constructor.
  Vec(const Vec<T, Dim>& other) noexcept {
    for (index_type i = 0; i < Dim; ++i) {
      val[i] = other.val[i];
    }
  }


  /// Move constructor.
  Vec(Vec<T, Dim> &&other) noexcept {
    for (index_type i = 0; i < Dim; ++i) {
      val[i] = other.val[i];
    }
  }


  /// Assignment operator.
  Vec<T, Dim> &operator=(const Vec<T, Dim> &other) noexcept {
    if (this != &other) {
      for (index_type i = 0; i < Dim; ++i) {
        val[i] = other.val[i];
      }
    }
    return *this;
  }


  /// Move assignment operator.
  Vec<T, Dim> &operator=(Vec<T, Dim> &&other) noexcept {
    if (this != &other) {
      for (index_type i = 0; i < Dim; ++i) {
        val[i] = other.val[i];
      }
    }
    return *this;
  }


  template<typename TargetType>
  explicit operator Vec<TargetType, Dim>() const {
    Vec<TargetType, Dim> conv;
    for (index_type i = 0; i < Dim; ++i) {
      conv.val[i] = static_cast<TargetType>(val[i]);
    }
    return conv;
  }


  /// Convenience conversion to double precision.
  Vec<double, Dim> ToDouble() const {
    return static_cast<Vec<double, Dim>>(*this);
  }


  Vec<int32_t, Dim> ToInteger() const {
    return static_cast<Vec<int32_t, Dim>>(*this);
  }


  //-------------------------------------------------
  // Convenience construction

  /// Returns a vector with all coordinates set to `value`.
  static Vec<T, Dim> All(T value) {
    Vec<T, Dim> vec;
    for (index_type i = 0; i < Dim; ++i) {
      vec.val[i] = value;
    }
    return vec;
  }


  /// Returns the homogeneous representation of this vector, *i.e.* the vector
  /// has an additional dimension which is set to 1.
  inline constexpr
  Vec<T, Dim+1> Homogeneous() const {
    Vec<T, Dim+1> vh;
    for (index_type i = 0; i < Dim; ++i) {
      vh[i] = val[i];
    }
    vh[Dim] = static_cast<T>(1);
    return vh;
  }


  //-------------------------------------------------
  // Array-like access

  T &operator[](std::size_t idx) {
    if (idx >= static_cast<int>(Dim)) {
      std::stringstream s;
      s << "Index-out-of-bounds: cannot access element at ["
        << idx << "] for " << TypeName() << ".";
      throw std::out_of_range(s.str());
    }

    return val[idx];
  }

  /// Array-like mutable access, which supports negative indexing.
  /// For example, vec[-1] == vec[Dim - 1].
  T &operator[](int idx) {
    if (idx < 0) {
      idx += static_cast<int>(Dim);
    }

    if (idx < 0) {
      std::stringstream s;
      s << "Index-out-of-bounds: cannot access element at ["
        << idx << "] for " << TypeName() << ".";
      throw std::out_of_range(s.str());
    }

    return (*this)[static_cast<std::size_t>(idx)];
  }


  const T &operator[](std::size_t idx) const {
    if (idx >= static_cast<int>(Dim)) {
      std::stringstream s;
      s << "Index-out-of-bounds: cannot access element at ["
        << idx << "] for " << TypeName() << ".";
      throw std::out_of_range(s.str());
    }

    return val[idx];
  }


  /// Array-like non-mutable access, which supports negative indexing.
  /// For example, vec[-1] == vec[Dim - 1].
  const T &operator[](int idx) const {
    if (idx < 0) {
      idx += static_cast<int>(Dim);
    }

    if (idx < 0) {
      std::stringstream s;
      s << "Index-out-of-bounds: cannot access element at ["
        << idx << "] for " << TypeName() << ".";
      throw std::out_of_range(s.str());
    }

    return (*this)[static_cast<std::size_t>(idx)];
  }



  //-------------------------------------------------
  // Value access via named methods: X,Y,Z,W

  inline const T &X() const noexcept {
    return val[0];
  }


  template<typename Tp = T> inline
  const typename std::enable_if<(Dim > 1), Tp>::type &Y() const noexcept {
    return val[1];
  }


  template<typename Tp = T> inline
  const typename std::enable_if<
      (Dim == 2), Tp>::type &Width() const noexcept {
    return X();
  }


  template<typename Tp = T> inline
  const typename std::enable_if<
      (Dim == 2), Tp>::type &Height() const noexcept {
    return Y();
  }


  template<typename Tp = T> inline
  const typename std::enable_if<(Dim > 2), Tp>::type &Z() const noexcept {
    return val[2];
  }


  template<typename Tp = T> inline
  const typename std::enable_if<(Dim > 3), Tp>::type &W() const noexcept {
    return val[3];
  }


  inline T &X() noexcept {
    return val[0];
  }


  template<typename Tp = T> inline
  typename std::enable_if<(Dim > 1), Tp>::type &Y() noexcept {
    return val[1];
  }


  template<typename Tp = T> inline
  typename std::enable_if<
      (Dim == 2), Tp>::type &Width() noexcept {
    return X();
  }


  template<typename Tp = T> inline
  typename std::enable_if<
      (Dim == 2), Tp>::type &Height() noexcept {
    return Y();
  }


  template<typename Tp = T> inline
  typename std::enable_if<(Dim > 2), Tp>::type &Z() noexcept {
    return val[2];
  }


  template<typename Tp = T> inline
  typename std::enable_if<(Dim > 3), Tp>::type &W() noexcept {
    return val[3];
  }


  inline void SetX(T x) noexcept {
    val[0] = x;
  }


  template<typename Tp = T> inline
  void SetY(typename std::enable_if<(Dim > 1), Tp>::type y) noexcept {
    val[1] = y;
  }


  template<typename Tp = T> inline
  void SetWidth(
      typename std::enable_if<(Dim == 2), Tp>::type width) noexcept {
    SetX(width);
  }


  template<typename Tp = T> inline
  void SetHeight(
      typename std::enable_if<(Dim == 2), Tp>::type height) noexcept {
    SetY(height);
  }


  template<typename Tp = T> inline
  void SetZ(typename std::enable_if<(Dim > 2), Tp>::type z) noexcept {
    val[2] = z;
  }


  template<typename Tp = T> inline
  void SetW(typename std::enable_if<(Dim > 3), Tp>::type w) noexcept {
    val[3] = w;
  }


  /// Holds the values of this vector.
  // NOLINTNEXTLINE(*-avoid-c-arrays)
  T val[Dim] = {0};



  //-------------------------------------------------
  // Comparison

  /// Returns true if all dimensions of both vectors are (approximately)
  /// equal. Floating point numbers are considered approximately equal if
  /// they differ by less than 4 units in the last place (ULP, units of
  /// least precision).
  bool EpsEquals(const Vec<T, Dim> &other) const {
    for (index_type i = 0; i < Dim; ++i) {
      if (!IsEpsEqual(val[i], other.val[i])) {
        return false;
      }
    }
    return true;
  }


  /// Equality checks can be used for floating point specializations.
  friend bool operator==(const Vec<T, Dim>& lhs, const Vec<T, Dim>& rhs) {
    return lhs.EpsEquals(rhs);
  }


  /// Inequality checks can be used for floating point specializations.
  friend bool operator!=(const Vec<T, Dim>& lhs, const Vec<T, Dim>& rhs) {
    return !(lhs == rhs);
  }

  //-------------------------------------------------
  // Arithmetics

  /// Performs element-wise addition and returns the modified instance.
  Vec<T, Dim> &AddVector(const Vec<T, Dim>& rhs) {
    for (index_type i = 0; i < Dim; ++i) {
      val[i] += rhs[i];
    }
    return *this;
  }


  /// Overloaded `vec1 + vec2`, see `AddVector`.
  Vec<T, Dim> &operator+=(const Vec<T, Dim>& rhs) {
    return AddVector(rhs);
  }


  /// Overloaded `vec1 + vec2`. By-value passing of lhs helps optimizing
  /// chained a+b+c equations.
  friend Vec<T, Dim> operator+(Vec<T, Dim> lhs, const Vec<T, Dim> &rhs) {
    lhs += rhs;
    return lhs;
  }


  /// Adds the scalar to each dimension and returns the modified instance.
  Vec<T, Dim> &AddScalar(T value) {
    for (index_type i = 0; i < Dim; ++i) {
      val[i] += value;
    }
    return *this;
  }


  /// Overloaded `vec += scalar`, see `AddScalar`.
  Vec<T, Dim> &operator+=(T value) {
    return AddScalar(value);
  }


  /// Overloaded `vec + scalar`.
  friend Vec<T, Dim> operator+(Vec<T, Dim> lhs, T rhs) {
    lhs += rhs;
    return lhs;
  }


  /// Overloaded `scalar + vec`.
  friend Vec<T, Dim> operator+(T lhs, Vec<T, Dim> rhs) {
    rhs += lhs;
    return rhs;
  }


  /// Performs element-wise subtraction and returns the modified instance.
  Vec<T, Dim> &SubtractVector(const Vec<T, Dim>& rhs) {
    for (index_type i = 0; i < Dim; ++i) {
      val[i] -= rhs[i];
    }
    return *this;
  }


  /// Overloaded `vec1 -= vec2`, see `SubtractVector`.
  Vec<T, Dim> &operator-=(const Vec<T, Dim>& rhs) {
    return SubtractVector(rhs);
  }


  /// Overloaded `vec1 -= vec2`.
  friend Vec<T, Dim> operator-(Vec<T, Dim> lhs, const Vec<T, Dim> &rhs) {
    lhs -= rhs;
    return lhs;
  }


  /// Subtracts `value` from each dimension and returns the modified instance.
  Vec<T, Dim> &SubtractScalar(T value) {
    for (index_type i = 0; i < Dim; ++i) {
      val[i] -= value;
    }
    return *this;
  }


  /// Overloaded `vec -= scalar`, see `SubtractScalar`.
  Vec<T, Dim> &operator-=(T value) {
    return SubtractScalar(value);
  }


  /// Overloaded `vec - scalar`.
  friend Vec<T, Dim> operator-(Vec<T, Dim> lhs, T rhs) {
    lhs -= rhs;
    return lhs;
  }


  /// Multiplies each dimension by the given scalar and returns the modified
  /// instance.
  Vec<T, Dim> &Multiply(T scale) {
    for (index_type i = 0; i < Dim; ++i) {
      val[i] *= scale;
    }
    return *this;
  }


  /// Overloaded `vec *= scalar`, see `Multiply`.
  Vec<T, Dim> &operator*=(T scale) {
    return Multiply(scale);
  }


  /// Overloaded `vec * scalar`.
  friend Vec<T, Dim> operator*(Vec<T, Dim> lhs, T rhs) {
    lhs *= rhs;
    return lhs;
  }


  /// Overloaded `scalar * vec`.
  friend Vec<T, Dim> operator*(T lhs, Vec<T, Dim> rhs) {
    rhs *= lhs;
    return rhs;
  }

//TODO doc

  /// Divides each element by the given scalar and returns the modified
  /// instance. Note that division is only supported for floating point
  /// types. Use ToDouble() to obtain a casted copy of this vector before
  /// division.
  template<typename Tp = T>
  Vec<typename std::enable_if<
          std::is_floating_point<Tp>::value,
          Tp>::type, Dim> &Divide(Tp scale) {
    for (index_type i = 0; i < Dim; ++i) {
      val[i] /= scale;
    }
    return *this;
  }


  /// Overloaded for convenience, see `Divide`. Only supported for
  /// floating point-based vector specializations.
  template<typename Tp = T>
  Vec<typename std::enable_if<
          std::is_floating_point<Tp>::value,
          Tp>::type, Dim> &operator/=(double scale) {
    return Divide(scale);
  }


  /// Overloaded for convenience. Divides each dimension by the given scalar.
  template<typename Tp = T>
  friend Vec<typename std::enable_if<
            std::numeric_limits<Tp>::is_iec559, Tp>::type,
            Dim> operator/(Vec<T, Dim> lhs, T rhs) {
    lhs /= rhs;
    return lhs;
  }


  /// Inverts (i.e. negates) each dimension.
  Vec<T, Dim> &Negate() {
    for (index_type i = 0; i < Dim; ++i) {
      val[i] *= -1;
    }
    return *this;
  }


  /// Returns a copy of this vector where each dimension is negated.
  /// Use `Negate` to explicitly negate this instance.
  Vec<T, Dim> operator-() const {
    Vec<T, Dim> cp(*this);
    cp.Negate();
    return cp;
  }


  /// Returns the index/dimension holding the maximum value.
  index_type MaxIndex() const {
    index_type max_idx = 0;
    for (index_type i = 1; i < Dim; ++i) {
      if (val[i] > val[max_idx]) {
        max_idx = i;
      }
    }
    return max_idx;
  }


  /// Returns the index/dimension holding the minimum value.
  index_type MinIndex() const {
    index_type min_idx = 0;
    for (index_type i = 1; i < Dim; ++i) {
      if (val[i] < val[min_idx]) {
        min_idx = i;
      }
    }
    return min_idx;
  }


  /// Returns the maximum dimension value.
  T MaxValue() const {
    return val[MaxIndex()];
  }


  /// Returns the minimum dimension value.
  T MinValue() const {
    return val[MinIndex()];
  }


  /// Computes the dot product.
  T Dot(const Vec<T, Dim>& other) const {
    T s{0};
    for (index_type i = 0; i < Dim; ++i) {
      s += val[i] * other.val[i];
    }
    return s;
  }


  /// Returns the 3D vector cross product.
  template<typename Tp = T>
  Vec<typename std::enable_if<(Dim == 3), Tp>::type, Dim>
  Cross(const Vec<T, Dim>& other) const {
    return Vec<T, Dim>{
        Y() * other.Z() - Z() * other.Y(),
        Z() * other.X() - X() * other.Z(),
        X() * other.Y() - Y() * other.X()
    };
  }


  /// Returns the squared vector's length.
  double LengthSquared() const {
    return static_cast<double>(Dot(*this));
  }


  /// Returns the vector's length.
  double Length() const {
    return std::sqrt(LengthSquared());
  }


  /// Returns the direction vector from `this` to `to`.
  Vec<T, Dim> DirectionVector(const Vec<T, Dim>& to) const {
    return to - *this;
  }

  /// Computes the L2 distance between this and the other.
  double DistanceEuclidean(const Vec<T, Dim>& other) const {
    auto diff = DirectionVector(other);
    return diff.Length();
  }


  /// Computes the L1 distance between this and the other.
  double DistanceManhattan(const Vec<T, Dim>& other) const {
    auto diff = DirectionVector(other);
    double abs_sum {0};
    for (index_type i = 0; i < Dim; ++i) {
      abs_sum += std::abs(diff[i]);
    }
    return abs_sum;
  }


  /// Returns the unit vector.
  Vec<double, Dim> UnitVector() const {
    const double len = Length();

    if (IsEpsZero(len)) {
      return Vec<double, Dim>{};
    }
    return static_cast<Vec<double, Dim>>(*this) / len;
  }

//TODO noexcept
//  //TODO Clock-wise if right-handed coordinate system
  template<typename Tp = T>
  Vec<typename std::enable_if<(Dim == 2), Tp>::type, Dim>
  PerpendicularClockwise() const {
    return Vec<T, Dim>{Y(), -X()};
  }


  template<typename Tp = T>
  Vec<typename std::enable_if<(Dim == 2), Tp>::type, Dim>
  PerpendicularCounterClockwise() const {
    return Vec<T, Dim>{-Y(), X()};
  }



  //-------------------------------------------------
  // String representation

  /// Returns a human-readable string representation.
  /// If `include_type` is false, it will only return
  /// the coordinates within parentheses, e.g. "(13, 77)".
  std::string ToString(
      bool include_type = true,
      int fixed_precision = 2) const {
    std::ostringstream s;
    if (include_type) {
      s << Vec<T, Dim>::TypeName();
    }
    s << '(' << std::fixed << std::setprecision(fixed_precision);

    for (std::size_t i = 0; i < Dim; ++i) {
      s << val[i];
      if (i < (Dim - 1)) {
        s << ", ";
      }
    }

    s << ')';
    return s.str();
  }


  /// Overloaded stream operator.
  friend std::ostream &operator<<(std::ostream &os, const Vec<T, Dim> &vec) {
    os << vec.ToString();
    return os;
  }


  template<typename Tp = T,
           typename std::enable_if<
             std::is_same<Tp, int16_t>::value, int>::type = 0>
  inline static char TypeAbbreviation() { return 's'; }

  template<typename Tp = T,
          typename std::enable_if<
             std::is_same<Tp, int32_t>::value, int>::type = 0>
  inline static char TypeAbbreviation() { return 'i'; }

  template<typename Tp = T,
          typename std::enable_if<
             std::is_same<Tp, double>::value, int>::type = 0>
  inline static char TypeAbbreviation() { return 'd'; }

  /// Returns the class type name, e.g. "Vec2d".
  static std::string TypeName() {
    std::ostringstream s;
    s << "Vec" << Dim << TypeAbbreviation<T>();
    return s.str();
  }

};

//-------------------------------------------------
// Aliases
using Vec2d = Vec<double, 2>;
using Vec3d = Vec<double, 3>;
using Vec4d = Vec<double, 4>;

using Vec2i = Vec<int32_t, 2>;
using Vec3i = Vec<int32_t, 3>;
using Vec4i = Vec<int32_t, 4>;


/// Returns the length of the given polygon.
template<typename T, std::size_t Dim>
double LengthPolygon(const std::vector<Vec<T, Dim>> &points) {
  double length {0};
  for (std::size_t idx = 1; idx < points.size(); ++idx) {
    length += points[idx-1].DistanceEuclidean(points[idx]);
  }
  return length;
}


//---------------------------------------------------- Math/Geometry Helpers


//TODO reconsider which helpers should be moved into the vector class

//TODO move inside Vector, enable if dim==2
/// Computes the determinant of the two 2d vectors.
template <typename T> inline
T Determinant(const Vec<T, 2> &a, const Vec<T, 2> &b) {
  return a.X() * b.Y() - b.X() * a.Y();
}


//TODO move inside vector
/// Scalar projection is the length of the vector projection, which is the
/// vector component of a in the direction of b.
/// See also: https://en.wikipedia.org/wiki/Vector_projection
template <typename T, std::size_t Dim> inline
T ScalarProjection(const Vec<T, Dim> &a, const Vec<T, Dim> &b) {
  static_assert(
      std::is_floating_point<T>::value,
      "Vector type must be float or double!");
  return a.Dot(b.UnitVector());
}


/// Returns :math:`\operatorname{proj}_{\mathbf{b}} \mathbf{a}`, *i.e.* the
/// projection of a onto b
/// See also: https://en.wikipedia.org/wiki/Vector_projection
template <typename T, std::size_t Dim> inline
Vec<T, Dim> VectorProjection(const Vec<T, Dim> &a, const Vec<T, Dim> &b) {
  static_assert(
      std::is_floating_point<T>::value,
      "Vector type must be float or double!");
  // Alternative formulation would be ScalarProjection(a, b) * b.UnitVector(),
  // but we can save some computation via:
  return (a.Dot(b) / b.LengthSquared()) * b;
}


//TODO move inside vector, enable if dim == 2
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


//TODO change to RotateVecDeg, RotateVecRad
//TODO take any vec, cast to double precision
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
template <typename T, std::size_t Dim,
          template<typename...> class Container = std::vector>
void MinMaxCoordinates(
    const Container<Vec<T, Dim>> &values,
    Vec<T, Dim> &min, Vec<T, Dim> &max) {
  using VecType = Vec<T, Dim>;
  if (values.empty()) {
    return;
  }

  min = *values.begin();
  max = *values.begin();
  for (const VecType &v : values) {
    for (typename VecType::index_type i = 0; i < Dim; ++i) {
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
