#ifndef WERKZEUGKISTE_GEOMETRY_VECTOR_H
#define WERKZEUGKISTE_GEOMETRY_VECTOR_H

#include <werkzeugkiste/geometry/geometry_export.h>
#include <werkzeugkiste/geometry/utils.h>

#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <iomanip>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace werkzeugkiste::geometry {

//-----------------------------------------------------------------------------
// Vectors/Coordinates

// TODO update doc
// TODO add noexcept where it can be guaranteed
//  Operator overloading megathread on SO:
//  https://stackoverflow.com/questions/4421706/what-are-the-basic-rules-and-idioms-for-operator-overloading

/// Template class to represent a vector/coordinate.
///
/// Any dimension can be accessed via random access,
/// i.e. `operator[]`, or via its publicly exposed `val` array member.
///
/// 2D vectors additionally provide access via `Width`/`Height`. Thus, using
/// them to hold 2D dimensions feels lexically correct.
template <typename T, std::size_t Dim>
class Vec {
 public:
  static_assert(std::is_signed<T>::value,
      "Only signed arithmetic types are supported!");

  static_assert(Dim > 0, "Dimension of Vec type must be > 0!");

  // NOLINTNEXTLINE(readability-identifier-naming)
  using value_type = T;

  // NOLINTNEXTLINE(readability-identifier-naming)
  using index_type = std::size_t;

  // NOLINTNEXTLINE(readability-identifier-naming)
  static constexpr std::size_t ndim = Dim;

  //---------------------------------------------------------------------------
  // Initialization

  /// Default constructor initializes all fields with 0.
  Vec() noexcept {
    for (index_type i = 0; i < Dim; ++i) {
      val[i] = static_cast<T>(0);
    }
  }

  /// Convenience (x, y) constructor for 2D vector.
  template <typename Tp = T>
  Vec(typename std::enable_if<(Dim == 2), Tp>::type x, Tp y) noexcept {
    val[0] = x;
    val[1] = y;
  }

  /// Convenience (x, y, z) constructor for 3D vector.
  template <typename Tp = T>
  Vec(typename std::enable_if<(Dim == 3), Tp>::type x, Tp y, Tp z) noexcept {
    val[0] = x;
    val[1] = y;
    val[2] = z;
  }

  /// Convenience (x, y, z, w) constructor for 4D vector.
  template <typename Tp = T>
  Vec(typename std::enable_if<(Dim == 4), Tp>::type x,
      Tp y,
      Tp z,
      Tp w) noexcept {
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
      s << "You cannot initialize " << TypeName() << " with " << values.size()
        << " values. The initializer list must"
           " either be empty, or contain "
        << Dim << " values!";
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
  Vec(Vec<T, Dim>&& other) noexcept {
    for (index_type i = 0; i < Dim; ++i) {
      val[i] = other.val[i];
    }
  }

  /// Assignment operator.
  Vec<T, Dim>& operator=(const Vec<T, Dim>& other) noexcept {
    if (this != &other) {
      for (index_type i = 0; i < Dim; ++i) {
        val[i] = other.val[i];
      }
    }
    return *this;
  }

  /// Move assignment operator.
  Vec<T, Dim>& operator=(Vec<T, Dim>&& other) noexcept {
    if (this != &other) {
      for (index_type i = 0; i < Dim; ++i) {
        val[i] = other.val[i];
      }
    }
    return *this;
  }

  //---------------------------------------------------------------------------
  // Casts

  template <typename TargetType>
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

  //---------------------------------------------------------------------------
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
  inline constexpr Vec<T, Dim + 1> Homogeneous() const {
    Vec<T, Dim + 1> vh;
    for (index_type i = 0; i < Dim; ++i) {
      vh[i] = val[i];
    }
    vh[Dim] = static_cast<T>(1);
    return vh;
  }

  //---------------------------------------------------------------------------
  // Array-like access

  T& operator[](std::size_t idx) {
    if (idx >= static_cast<int>(Dim)) {
      std::stringstream s;
      s << "Index-out-of-bounds: cannot access element at [" << idx << "] for "
        << TypeName() << ".";
      throw std::out_of_range(s.str());
    }

    return val[idx];
  }

  /// Array-like mutable access, which supports negative indexing.
  /// For example, vec[-1] == vec[Dim - 1].
  T& operator[](int idx) {
    if (idx < 0) {
      idx += static_cast<int>(Dim);
    }

    if (idx < 0) {
      std::stringstream s;
      s << "Index-out-of-bounds: cannot access element at [" << idx << "] for "
        << TypeName() << ".";
      throw std::out_of_range(s.str());
    }

    return (*this)[static_cast<std::size_t>(idx)];
  }

  const T& operator[](std::size_t idx) const {
    if (idx >= static_cast<int>(Dim)) {
      std::stringstream s;
      s << "Index-out-of-bounds: cannot access element at [" << idx << "] for "
        << TypeName() << ".";
      throw std::out_of_range(s.str());
    }

    return val[idx];
  }

  /// Array-like non-mutable access, which supports negative indexing.
  /// For example, vec[-1] == vec[Dim - 1].
  const T& operator[](int idx) const {
    if (idx < 0) {
      idx += static_cast<int>(Dim);
    }

    if (idx < 0) {
      std::stringstream s;
      s << "Index-out-of-bounds: cannot access element at [" << idx << "] for "
        << TypeName() << ".";
      throw std::out_of_range(s.str());
    }

    return (*this)[static_cast<std::size_t>(idx)];
  }

  //---------------------------------------------------------------------------
  // Value access via named methods: X, Y, Z, ...

  inline const T& X() const noexcept { return val[0]; }

  template <typename Tp = T>
  inline const typename std::enable_if<(Dim > 1), Tp>::type& Y()
      const noexcept {
    return val[1];
  }

  template <typename Tp = T>
  inline const typename std::enable_if<(Dim == 2), Tp>::type& Width()
      const noexcept {
    return X();
  }

  template <typename Tp = T>
  inline const typename std::enable_if<(Dim == 2), Tp>::type& Height()
      const noexcept {
    return Y();
  }

  template <typename Tp = T>
  inline const typename std::enable_if<(Dim > 2), Tp>::type& Z()
      const noexcept {
    return val[2];
  }

  template <typename Tp = T>
  inline const typename std::enable_if<(Dim > 3), Tp>::type& W()
      const noexcept {
    return val[3];
  }

  inline T& X() noexcept { return val[0]; }

  template <typename Tp = T>
  inline typename std::enable_if<(Dim > 1), Tp>::type& Y() noexcept {
    return val[1];
  }

  template <typename Tp = T>
  inline typename std::enable_if<(Dim == 2), Tp>::type& Width() noexcept {
    return X();
  }

  template <typename Tp = T>
  inline typename std::enable_if<(Dim == 2), Tp>::type& Height() noexcept {
    return Y();
  }

  template <typename Tp = T>
  inline typename std::enable_if<(Dim > 2), Tp>::type& Z() noexcept {
    return val[2];
  }

  template <typename Tp = T>
  inline typename std::enable_if<(Dim > 3), Tp>::type& W() noexcept {
    return val[3];
  }

  inline void SetX(T x) noexcept { val[0] = x; }

  template <typename Tp = T>
  inline void SetY(typename std::enable_if<(Dim > 1), Tp>::type y) noexcept {
    val[1] = y;
  }

  template <typename Tp = T>
  inline void SetWidth(
      typename std::enable_if<(Dim == 2), Tp>::type width) noexcept {
    SetX(width);
  }

  template <typename Tp = T>
  inline void SetHeight(
      typename std::enable_if<(Dim == 2), Tp>::type height) noexcept {
    SetY(height);
  }

  template <typename Tp = T>
  inline void SetZ(typename std::enable_if<(Dim > 2), Tp>::type z) noexcept {
    val[2] = z;
  }

  template <typename Tp = T>
  inline void SetW(typename std::enable_if<(Dim > 3), Tp>::type w) noexcept {
    val[3] = w;
  }

  /// Holds the values of this vector.
  // NOLINTNEXTLINE(*-avoid-c-arrays)
  T val[Dim] = {0};

  //---------------------------------------------------------------------------
  // Comparison

  /// "Almost equal" check for vectors with floating point value type.
  /// Returns true if all dimensions of both vectors are approximately
  /// equal, depending on both relative and absolute tolerance thresholds,
  /// see `werkzeugkiste::geometry::IsClose`.
  template <typename Tp = T>
  bool IsClose(
      const Vec<
          typename std::enable_if<std::is_floating_point<Tp>::value, Tp>::type,
          Dim>& other,
      Tp relative_tolerance,
      Tp absolute_tolerance) const {
    for (index_type i = 0; i < Dim; ++i) {
      if (!werkzeugkiste::geometry::IsClose(
              val[i], other.val[i], relative_tolerance, absolute_tolerance)) {
        return false;
      }
    }
    return true;
  }

  /// Vectors with integral value type can be compared for equality.
  template <typename Tp = T>
  bool IsEqual(
      const Vec<typename std::enable_if<std::is_integral<Tp>::value, Tp>::type,
          Dim>& other) const {
    for (index_type i = 0; i < Dim; ++i) {
      if (val[i] != other.val[i]) {
        return false;
      }
    }
    return true;
  }

  /// Equality checks can be used for all vector specializations.
  /// Floating point value types use the `IsClose` check with:
  /// * relative tolerance of 1e-6 (float) and 1e-9 (double), and
  /// * absolute tolerance of 1e-9 (float) and 1e-12 (double).
  ///
  /// Float can store 6-9 significant digits, whereas double can
  /// typically store 15-18 significant digits. Given the usage
  /// scenarios of this template class, the above tolerances
  /// are sufficiently tight.
  friend bool operator==(const Vec<T, Dim>& lhs, const Vec<T, Dim>& rhs) {
    if constexpr (std::is_integral<T>::value) {
      return lhs.IsEqual(rhs);
    } else {
      if constexpr (std::is_same<float, T>::value) {
        // NOLINTNEXTLINE(*-magic-numbers)
        return lhs.IsClose(rhs, 1e-6F, 1e-9F);
      } else {
        // NOLINTNEXTLINE(*-magic-numbers)
        return lhs.IsClose(rhs, 1e-9, 1e-12);
      }
    }
  }

  /// Inequality checks can be used for floating point specializations.
  friend bool operator!=(const Vec<T, Dim>& lhs, const Vec<T, Dim>& rhs) {
    return !(lhs == rhs);
  }

  //---------------------------------------------------------------------------
  // Arithmetic

  /// Performs element-wise addition and returns the modified instance.
  Vec<T, Dim>& AddVector(const Vec<T, Dim>& rhs) {
    for (index_type i = 0; i < Dim; ++i) {
      val[i] += rhs[i];
    }
    return *this;
  }

  /// Overloaded `vec1 + vec2`, see `AddVector`.
  Vec<T, Dim>& operator+=(const Vec<T, Dim>& rhs) { return AddVector(rhs); }

  /// Overloaded `vec1 + vec2`. By-value passing of lhs helps optimizing
  /// chained a+b+c equations.
  friend Vec<T, Dim> operator+(Vec<T, Dim> lhs, const Vec<T, Dim>& rhs) {
    lhs += rhs;
    return lhs;
  }

  /// Adds the scalar to each dimension and returns the modified instance.
  Vec<T, Dim>& AddScalar(T value) {
    for (index_type i = 0; i < Dim; ++i) {
      val[i] += value;
    }
    return *this;
  }

  /// Overloaded `vec += scalar`, see `AddScalar`.
  Vec<T, Dim>& operator+=(T value) { return AddScalar(value); }

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
  Vec<T, Dim>& SubtractVector(const Vec<T, Dim>& rhs) {
    for (index_type i = 0; i < Dim; ++i) {
      val[i] -= rhs[i];
    }
    return *this;
  }

  /// Overloaded `vec1 -= vec2`, see `SubtractVector`.
  Vec<T, Dim>& operator-=(const Vec<T, Dim>& rhs) {
    return SubtractVector(rhs);
  }

  /// Overloaded `vec1 -= vec2`.
  friend Vec<T, Dim> operator-(Vec<T, Dim> lhs, const Vec<T, Dim>& rhs) {
    lhs -= rhs;
    return lhs;
  }

  /// Subtracts `value` from each dimension and returns the modified instance.
  Vec<T, Dim>& SubtractScalar(T value) {
    for (index_type i = 0; i < Dim; ++i) {
      val[i] -= value;
    }
    return *this;
  }

  /// Overloaded `vec -= scalar`, see `SubtractScalar`.
  Vec<T, Dim>& operator-=(T value) { return SubtractScalar(value); }

  /// Overloaded `vec - scalar`.
  friend Vec<T, Dim> operator-(Vec<T, Dim> lhs, T rhs) {
    lhs -= rhs;
    return lhs;
  }

  /// Overloaded `scalar - vec`, which is `All(scalar) - vec`.
  friend Vec<T, Dim> operator-(T lhs, const Vec<T, Dim>& rhs) {
    auto all = All(lhs);
    all -= rhs;
    return all;
  }

  /// Multiplies each dimension by the given scalar and returns the modified
  /// instance.
  Vec<T, Dim>& MultiplyScalar(T scale) {
    for (index_type i = 0; i < Dim; ++i) {
      val[i] *= scale;
    }
    return *this;
  }

  /// Overloaded `vec *= scalar`, see `MultiplyScalar`.
  Vec<T, Dim>& operator*=(T scale) { return MultiplyScalar(scale); }

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

  /// Performs element-wise multiplication and returns the
  /// modified instance.
  Vec<T, Dim>& MultiplyVector(const Vec<T, Dim>& other) {
    for (index_type i = 0; i < Dim; ++i) {
      val[i] *= other[i];
    }
    return *this;
  }

  /// Overloaded `vec1 *= vec2`, see `MultiplyVector`.
  Vec<T, Dim>& operator*=(const Vec<T, Dim>& other) {
    return MultiplyVector(other);
  }

  /// Overloaded `vec1 * vec2`.
  friend Vec<T, Dim> operator*(Vec<T, Dim> lhs, const Vec<T, Dim>& rhs) {
    lhs *= rhs;
    return lhs;
  }

  /// Divides each element by the given scalar and returns the modified
  /// instance. Note that division is only supported for floating point
  /// types. Use ToDouble() to obtain a casted copy of this vector before
  /// division.
  template <typename Tp = T>
  Vec<typename std::enable_if<std::is_floating_point<Tp>::value, Tp>::type,
      Dim>&
  DivideScalar(Tp divisor) {
    for (index_type i = 0; i < Dim; ++i) {
      val[i] /= divisor;
    }
    return *this;
  }

  /// Overloaded for convenience, see `DivideScalar`. Only supported
  /// for floating point-based vector specializations. Use ToDouble() to
  /// convert integral vectors before division.
  template <typename Tp = T>
  Vec<typename std::enable_if<std::is_floating_point<Tp>::value, Tp>::type,
      Dim>&
  operator/=(double divisor) {
    return DivideScalar(divisor);
  }

  /// Overloaded for convenience. Divides each dimension by the given scalar.
  /// Only supported for floating point-based vector specializations.
  template <typename Tp = T>
  friend Vec<
      typename std::enable_if<std::is_floating_point<Tp>::value, Tp>::type,
      Dim>
  operator/(Vec<T, Dim> lhs, double divisor) {
    lhs /= divisor;
    return lhs;
  }

  /// Performs element-wise division and returns the modified
  /// instance. Only supported for floating point-based vector
  /// specializations. Use ToDouble() to convert integral vectors
  /// before division.
  template <typename Tp = T>
  Vec<typename std::enable_if<std::is_floating_point<Tp>::value, Tp>::type,
      Dim>&
  DivideVector(const Vec<Tp, Dim>& divisor) {
    for (index_type i = 0; i < Dim; ++i) {
      val[i] /= divisor[i];
    }
    return *this;
  }

  /// Overloaded element-wise division, see `DivideVector`. Only supported for
  /// floating point-based vector specializations.
  template <typename Tp = T>
  Vec<typename std::enable_if<std::is_floating_point<Tp>::value, Tp>::type,
      Dim>&
  operator/=(const Vec<Tp, Dim>& divisor) {
    return DivideVector(divisor);
  }

  /// Overloaded element-wise division, see `DivideVector`. Only supported for
  /// floating point-based vector specializations.
  template <typename Tp = T>
  friend Vec<
      typename std::enable_if<std::is_floating_point<Tp>::value, Tp>::type,
      Dim>
  operator/(Vec<T, Dim> lhs, const Vec<Tp, Dim>& divisor) {
    lhs /= divisor;
    return lhs;
  }

  /// Returns a vector where each dimension is the result of the element-wise
  /// division `scalar / divisor[dimension]. Only supported for
  /// floating point-based vector specializations.
  template <typename Tp = T>
  friend Vec<
      typename std::enable_if<std::is_floating_point<Tp>::value, Tp>::type,
      Dim>
  operator/(Tp scalar, Vec<T, Dim> vec) {
    for (index_type idx = 0; idx < Dim; ++idx) {
      vec.val[idx] = scalar / vec.val[idx];
    }
    return vec;
  }

  /// Inverts (i.e. negates) each dimension.
  Vec<T, Dim>& Negate() {
    for (index_type i = 0; i < Dim; ++i) {
      val[i] *= -1;
    }
    return *this;
  }

  /// Returns a copy of this vector where each dimension is negated.
  /// Use `Negate` to explicitly negate this instance.
  Vec<T, Dim> operator-() const {
    Vec<T, Dim> cp{*this};
    cp.Negate();
    return cp;
  }

  //---------------------------------------------------------------------------
  // Vector-specific arithmetic & utils (Scalar product, cross, length, ...)

  /// Computes the dot product.
  T Dot(const Vec<T, Dim>& other) const {
    T s{0};
    for (index_type i = 0; i < Dim; ++i) {
      s += val[i] * other.val[i];
    }
    return s;
  }

  /// Returns the determinant of the two 2d vectors, i.e.
  /// the "2d cross product".
  template <typename Tp = T>
  inline Tp Determinant(
      const Vec<typename std::enable_if<(Dim == 2), Tp>::type, Dim>& other)
      const {
    return (X() * other.Y()) - (other.X() * Y());
  }

  /// Returns the 3D vector cross product.
  template <typename Tp = T>
  Vec<typename std::enable_if<(Dim == 3), Tp>::type, Dim> Cross(
      const Vec<T, Dim>& other) const {
    return Vec<T, Dim>{Y() * other.Z() - Z() * other.Y(),
        Z() * other.X() - X() * other.Z(),
        X() * other.Y() - Y() * other.X()};
  }

  /// Returns the squared vector's length.
  double LengthSquared() const { return static_cast<double>(Dot(*this)); }

  /// Returns the vector's length.
  double Length() const { return std::sqrt(LengthSquared()); }

  /// Returns the direction vector from `this` to `to`.
  Vec<T, Dim> DirectionVector(const Vec<T, Dim>& to) const {
    return to - *this;
  }

  /// Returns a vector holding the absolute values of this vector.
  Vec<T, Dim> Absolute() const {
    Vec<T, Dim> abs{};
    for (index_type idx = 0; idx < Dim; ++idx) {
      if constexpr (std::is_floating_point_v<T>) {
        abs.val[idx] = std::fabs(val[idx]);
      } else {
        abs.val[idx] = std::abs(val[idx]);
      }
    }
    return abs;
  }

  /// Computes the L2 distance between this and the other.
  double DistanceEuclidean(const Vec<T, Dim>& other) const {
    auto diff = DirectionVector(other);
    return diff.Length();
  }

  /// Computes the L1 distance between this and the other.
  double DistanceManhattan(const Vec<T, Dim>& other) const {
    auto diff = DirectionVector(other);
    double abs_sum{0};
    for (index_type i = 0; i < Dim; ++i) {
      abs_sum += static_cast<double>(std::abs(diff[i]));
    }
    return abs_sum;
  }

  /// Returns the sum over all dimensions.
  T Sum() const {
    T sum{0};
    for (index_type i = 0; i < Dim; ++i) {
      sum += val[i];
    }
    return sum;
  }

  /// Returns the unit vector.
  Vec<double, Dim> UnitVector() const {
    const double len = Length();

    if (IsEpsZero(len)) {
      return Vec<double, Dim>{};
    }
    return static_cast<Vec<double, Dim>>(*this) / len;
  }

  //---------------------------------------------------------------------------
  // Queries

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
  T MaxValue() const { return val[MaxIndex()]; }

  /// Returns the minimum dimension value.
  T MinValue() const { return val[MinIndex()]; }

  //-------------------------------------------------
  // Rotations

  /// Returns the 90° clock-wise rotated vector. This method assumes
  /// that the coordinate system is right-handed and is only
  /// supported for 2D vectors.
  template <typename Tp = T>
  Vec<typename std::enable_if<(Dim == 2), Tp>::type, Dim>
  PerpendicularClockwise() const {
    return Vec<T, Dim>{Y(), -X()};
  }

  /// Returns the 90° counter-clock-wise rotated vector. This method assumes
  /// that the coordinate system is right-handed and is only
  /// supported for 2D vectors.
  template <typename Tp = T>
  Vec<typename std::enable_if<(Dim == 2), Tp>::type, Dim>
  PerpendicularCounterClockwise() const {
    return Vec<T, Dim>{-Y(), X()};
  }

  // TODO doc & test
  /// Returns a copy of this vector rotated by the given radians.
  /// This method assumes that the coordinate system is right-handed and
  /// is only supported for 2D vector specialization with floating point
  /// value type.
  template <typename Tp = double>
  Vec<typename std::enable_if<(Dim == 2), Tp>::type, Dim> RotateRadians(
      double angle_rad) const {
    const double ct = std::cos(angle_rad);
    const double st = std::sin(angle_rad);
    const double x = static_cast<double>(val[0]);
    const double y = static_cast<double>(val[1]);
    return Vec<Tp, 2>{(ct * x) - (st * y), (st * x) + (ct * y)};
  }

  // TODO doc & test
  /// Returns a double precision vector which is the result of
  /// rotating this vector by the given angle in degrees.
  /// This method assumes that the coordinate system is right-handed
  /// and is only supported for 2D vectors.
  template <typename Tp = double>
  Vec<typename std::enable_if<(Dim == 2), Tp>::type, Dim> RotateDegrees(
      double angle_deg) const {
    return RotateRadians(Deg2Rad(angle_deg));
  }

  // TODO doc & test
  template <typename Tp = double>
  Vec<typename std::enable_if<(Dim == 2), Tp>::type, Dim> RotateRadians(
      const Vec<Tp, 2>& rotation_center,
      double angle_rad) const {
    Vec<Tp, 2> vec = *this - rotation_center;
    return vec.RotateRadians(angle_rad) + rotation_center;
  }

  // TODO doc & test
  template <typename Tp = double>
  Vec<typename std::enable_if<(Dim == 2), Tp>::type, Dim> RotateDegrees(
      const Vec<Tp, 2>& rotation_center,
      double angle_deg) const {
    return RotateRadians(rotation_center, Deg2Rad(angle_deg));
  }

  //---------------------------------------------------------------------------
  // String representation

  /// Returns a human-readable string representation.
  /// If `include_type` is true, the class name will be included, e.g.
  /// "Vec2i(1, 2)". Otherwise, it will only return
  /// the coordinates within parentheses, e.g. "(13, 77)".
  ///
  /// For floating point types, the output precision will be set to the
  /// maximum precision if fixed_precision <= 0. If a positive fixed_precision
  /// is provided, the output format will be adjusted.
  std::string ToString(bool include_type = true,
      int fixed_precision = 0) const {
    std::ostringstream s;
    if (include_type) {
      s << Vec<T, Dim>::TypeName();
    }
    s << '(';

    if constexpr (std::is_floating_point_v<T>) {
      if (fixed_precision > 0) {
        s << std::fixed << std::setprecision(fixed_precision);
      } else {
        s << std::setprecision(std::numeric_limits<T>::max_digits10);
      }
    }

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
  friend std::ostream& operator<<(std::ostream& os, const Vec<T, Dim>& vec) {
    os << vec.ToString();
    return os;
  }

  template <typename Tp = T,
      typename std::enable_if<std::is_same<Tp, int16_t>::value, int>::type = 0>
  inline static char TypeAbbreviation() {
    return 's';
  }

  template <typename Tp = T,
      typename std::enable_if<std::is_same<Tp, int32_t>::value, int>::type = 0>
  inline static char TypeAbbreviation() {
    return 'i';
  }

  template <typename Tp = T,
      typename std::enable_if<std::is_same<Tp, double>::value, int>::type = 0>
  inline static char TypeAbbreviation() {
    return 'd';
  }

  template <typename Tp = T,
      typename std::enable_if<std::is_same<Tp, float>::value, int>::type = 0>
  inline static char TypeAbbreviation() {
    return 'f';
  }

  /// Returns the class type name, e.g. "Vec2d".
  static std::string TypeName() {
    std::ostringstream s;
    s << "Vec" << Dim << TypeAbbreviation<T>();
    return s.str();
  }
};

// template <typename Type, typename... Types>
// inline auto TupleToVec(const std::tuple<Type, Types...> &tpl) {
//   using Vector = Vec<Type, sizeof...(Types)+1>;
//   return std::make_from_tuple<Vector>(tpl);
// }
////template <template <typename Type, typename... Types> class Container>
////inline auto TuplesToPolygon(Container tuples) {
// template <typename Type, typename... Types, typename Container>
// inline auto TuplesToPolygon(const Container &tuples) {
//   using V = Vec<Type, sizeof...(Types)+1>;
//   std::vector<V> poly;
//   for (const auto &tpl : tuples) {
//     poly.emplace_back(TupleToVec<Type, Types...>(tpl));
//   }
//   return poly;
// }

//-----------------------------------------------------------------------------
// Aliases & available specializations

/// Single-precision, 2-dimensional vector.
extern template class WERKZEUGKISTE_GEOMETRY_EXPORT Vec<float, 2>;
using Vec2f = Vec<float, 2>;

/// Single-precision, 3-dimensional vector.
extern template class WERKZEUGKISTE_GEOMETRY_EXPORT Vec<float, 3>;
using Vec3f = Vec<float, 3>;

/// Single-precision, 4-dimensional vector.
extern template class WERKZEUGKISTE_GEOMETRY_EXPORT Vec<float, 4>;
using Vec4f = Vec<float, 4>;

/// Double-precision, 2-dimensional vector.
extern template class WERKZEUGKISTE_GEOMETRY_EXPORT Vec<double, 2>;
using Vec2d = Vec<double, 2>;

/// Double-precision, 3-dimensional vector.
extern template class WERKZEUGKISTE_GEOMETRY_EXPORT Vec<double, 3>;
using Vec3d = Vec<double, 3>;

/// Double-precision, 4-dimensional vector.
extern template class WERKZEUGKISTE_GEOMETRY_EXPORT Vec<double, 4>;
using Vec4d = Vec<double, 4>;

/// Integral, 2-dimensional vector.
extern template class WERKZEUGKISTE_GEOMETRY_EXPORT Vec<int32_t, 2>;
using Vec2i = Vec<int32_t, 2>;

/// Integral, 3-dimensional vector.
extern template class WERKZEUGKISTE_GEOMETRY_EXPORT Vec<int32_t, 3>;
using Vec3i = Vec<int32_t, 3>;

/// Integral, 4-dimensional vector.
extern template class WERKZEUGKISTE_GEOMETRY_EXPORT Vec<int32_t, 4>;
using Vec4i = Vec<int32_t, 4>;

//-------------------------------------------------
// Additional utility functions for vectors/points.

/// Returns the length of the given polygon.
template <typename T, std::size_t Dim>
inline double LengthPolygon(const std::vector<Vec<T, Dim>>& points) {
  double length{0};
  for (std::size_t idx = 1; idx < points.size(); ++idx) {
    length += points[idx - 1].DistanceEuclidean(points[idx]);
  }
  return length;
}

//---------------------------------------------------- Math/Geometry Helpers

// TODO reconsider which helpers should be moved into the vector class

// TODO move inside vector - ScalarProjectionOnto()
/// Scalar projection is the length of the vector projection, which is the
/// vector component of a in the direction of b.
/// See also: https://en.wikipedia.org/wiki/Vector_projection
template <typename T, std::size_t Dim>
inline T ScalarProjection(const Vec<T, Dim>& a, const Vec<T, Dim>& b) {
  static_assert(
      std::is_floating_point<T>::value, "Vector type must be float or double!");
  return a.Dot(b.UnitVector());
}

// TODO move inside vector - VectorProjectionOnto(other)
/// Returns :math:`\operatorname{proj}_{\mathbf{b}} \mathbf{a}`, *i.e.* the
/// projection of a onto b
/// See also: https://en.wikipedia.org/wiki/Vector_projection
template <typename T, std::size_t Dim>
inline Vec<T, Dim> VectorProjection(const Vec<T, Dim>& a,
    const Vec<T, Dim>& b) {
  static_assert(
      std::is_floating_point<T>::value, "Vector type must be float or double!");
  // Alternative formulation would be ScalarProjection(a, b) * b.UnitVector(),
  // but we can save some computation via:
  return (a.Dot(b) / b.LengthSquared()) * b;
}

// TODO move inside vector, enable if dim == 2
/// Computes the angle (in radians) of a 2d direction vector w.r.t. the
/// positive X axis.
template <typename T>
inline double AngleRadFromDirectionVec(const Vec<T, 2>& vec) {
  // Dot product is proportional to the cosine, whereas
  // the determinant is proportional to the sine.
  // See: https://math.stackexchange.com/a/879474
  const Vec<double, 2> ref{1, 0};
  const Vec<double, 2> unit = vec.UnitVector();
  return std::atan2(ref.Determinant(unit), ref.Dot(unit));
}

/// Computes the angle (in degrees) of a 2d direction vector w.r.t. the
/// positive X axis.
template <typename T>
inline double AngleDegFromDirectionVec(const Vec<T, 2>& vec) {
  return Rad2Deg(AngleRadFromDirectionVec(vec));
}

/// Returns the unit direction vector given its angle (in
/// radians) w.r.t. the positive X axis.
inline Vec2d DirectionVecFromAngleRad(double rad) {
  return Vec2d{std::cos(rad),
      std::sin(
          rad)};  // TODO test case to ensure numerical stability at edge cases
}

/// Returns the unit direction vector given its angle (in
/// degrees) w.r.t. the positive X axis.
inline Vec2d DirectionVecFromAngleDeg(double deg) {
  return DirectionVecFromAngleRad(Deg2Rad(deg));
}

// TODO move into vec class
///// Rotates the vector by the given radians about the given rotation
///// center, assuming a right-handed(!) coordinate system.
// inline Vec2d RotateVector(
//     const Vec2d &vec, const Vec2d &rotation_center, double theta) {
//   return RotateVector(vec - rotation_center, theta) + rotation_center;
// }

/// Computes the minimum/maximum along each dimension.
///
/// Useful to get axis-aligned bounding boxes, a starting
/// point for hull computations, etc.
template <typename T,
    std::size_t Dim,
    template <typename...> class Container = std::vector>
void MinMaxCoordinates(const Container<Vec<T, Dim>>& values,
    Vec<T, Dim>& min,
    Vec<T, Dim>& max) {
  using VecType = Vec<T, Dim>;
  if (values.empty()) {
    return;
  }

  min = *values.begin();
  max = *values.begin();
  for (const VecType& v : values) {
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

}  // namespace werkzeugkiste::geometry

#endif  // WERKZEUGKISTE_GEOMETRY_VECTOR_H
