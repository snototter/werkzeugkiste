#include <sstream>
#include <iomanip>
#include <type_traits>
#include <stdexcept>
#include <cstdlib>
#include <cassert>
#include <cstring> // memcpy
#include <algorithm> // std::swap

#include <werkzeugkiste/geometry/vector.h>
#include <werkzeugkiste/geometry/utils.h>

namespace werkzeugkiste {
namespace geometry {
template<typename _Tp, int dim>
Vec<_Tp, dim>::Vec() {
  for (int i = 0; i < dim; ++i)
    val[i] = static_cast<_Tp>(0);
}

template<typename _Tp, int dim>
Vec<_Tp, dim>::Vec(_Tp x, _Tp y) {
  if (dim != 2) {
    std::stringstream s;
    s << "You cannot initialize " << TypeName() << " with 2 values.";
    throw std::invalid_argument(s.str());
  }
  val[0] = x;
  val[1] = y;
}

template<typename _Tp, int dim>
Vec<_Tp, dim>::Vec(_Tp x, _Tp y, _Tp z) {
  if (dim != 3) {
    std::stringstream s;
    s << "You cannot initialize " << TypeName() << " with 3 values.";
    throw std::invalid_argument(s.str());
  }
  val[0] = x;
  val[1] = y;
  val[2] = z;
}


template<typename _Tp, int dim>
Vec<_Tp, dim>::Vec(std::initializer_list<_Tp> values) {
  if ((values.size() != 0) &&
      (values.size() != static_cast<size_t>(dim))) {
    std::stringstream s;
    s << "You cannot initialize " << TypeName()
      << " with " << values.size() << " values";
    throw std::invalid_argument(s.str());
  }

  if (values.size() == 0) {
    for (int i = 0; i < dim; ++i)
      val[i] = static_cast<_Tp>(0);
  } else {
    for (size_t i = 0; i < values.size(); ++i)
      val[i] = values.begin()[i];
  }
}


template<typename _Tp, int dim>
Vec<_Tp, dim>::Vec(_Tp x, _Tp y, _Tp z, _Tp w) {
  if (dim != 4) {
    std::stringstream s;
    s << "You cannot initialize " << TypeName() << " with 4 values.";
    throw std::invalid_argument(s.str());
  }
  val[0] = x;
  val[1] = y;
  val[2] = z;
  val[3] = w;
}


template<typename _Tp, int dim>
Vec<_Tp, dim>::Vec(const Vec<_Tp, dim>& other) {
  for (int i = 0; i < dim; ++i)
    val[i] = other.val[i];
}


template<typename _Tp, int dim>
Vec<_Tp, dim>::Vec(Vec<_Tp, dim> &&other) noexcept {
  for (int i = 0; i < dim; ++i)
    val[i] = other.val[i];
}


template<typename _Tp, int dim>
Vec<_Tp, dim+1> Vec<_Tp, dim>::Homogeneous() const {
  Vec<_Tp, dim+1> vh;
  for (int i = 0; i < dim; ++i) {
    vh[i] = val[i];
  }
  vh[dim] = static_cast<_Tp>(1);
  return vh;
}


template<typename _Tp, int dim>
Vec<_Tp, dim> &Vec<_Tp, dim>::operator=(const Vec<_Tp, dim> &other) {
  for (int i = 0; i < dim; ++i)
    val[i] = other.val[i];
  return *this;
}


template<typename _Tp, int dim>
Vec<_Tp, dim> &Vec<_Tp, dim>::operator=(Vec<_Tp, dim> &&other) noexcept {
  for (int i = 0; i < dim; ++i)
    val[i] = other.val[i];
  return *this;
}


template<typename _Tp, int dim>
Vec<_Tp, dim>::operator Vec<double, dim>() const {
  Vec<double, dim> conv;
  for (int i = 0; i < dim; ++i)
    conv.val[i] = static_cast<double>(val[i]);
  return conv;
}


template<typename _Tp, int dim>
const _Tp& Vec<_Tp, dim>::operator[](int i) const {
  if (i < 0) {
    i += dim;
  }
  if ((i < 0) ||(i >= dim)) {
    std::stringstream s;
    s << "Index-out-of-bounds: cannot access element at ["
      << i << "] for " << TypeName() << ".";
    throw std::out_of_range(s.str());
  }
  return val[i];
}


template<typename _Tp, int dim>
_Tp& Vec<_Tp, dim>::operator[](int i) {
  if (i < 0) {
    i += dim;
  }
  if ((i < 0) ||(i >= dim)) {
    std::stringstream s;
    s << "Index-out-of-bounds: cannot access element at ["
      << i << "] for " << TypeName() << ".";
    throw std::out_of_range(s.str());
  }
  return val[i];
}


template<typename _Tp, int dim>
const _Tp& Vec<_Tp, dim>::x() const {
  return (*this)[0];
}


template<typename _Tp, int dim>
const _Tp& Vec<_Tp, dim>::y() const {
  return (*this)[1];
}


template<typename _Tp, int dim>
const _Tp& Vec<_Tp, dim>::width() const {
  if (dim != 2)
    throw std::logic_error("Only 2D vectors support"
                           " member access via width().");
  return x();
}


template<typename _Tp, int dim>
const _Tp& Vec<_Tp, dim>::height() const {
  if (dim != 2)
    throw std::logic_error("Only 2D vectors support"
                           " member access via height().");
  return y();
}


template<typename _Tp, int dim>
const _Tp& Vec<_Tp, dim>::z() const {
  return (*this)[2];
}


template<typename _Tp, int dim>
const _Tp& Vec<_Tp, dim>::w() const {
  return (*this)[3];
}


template<typename _Tp, int dim>
_Tp& Vec<_Tp, dim>::x() {
  return (*this)[0];
}


template<typename _Tp, int dim>
_Tp& Vec<_Tp, dim>::y() {
  return (*this)[1];
}


template<typename _Tp, int dim>
_Tp& Vec<_Tp, dim>::width() {
  if (dim != 2)
    throw std::logic_error("Only 2D vectors support"
                           " member access via width().");
  return x();
}


template<typename _Tp, int dim>
_Tp& Vec<_Tp, dim>::height() {
  if (dim != 2)
    throw std::logic_error("Only 2D vectors support"
                           " member access via height().");
  return y();
}


template<typename _Tp, int dim>
_Tp& Vec<_Tp, dim>::z() {
  return (*this)[2];
}


template<typename _Tp, int dim>
_Tp& Vec<_Tp, dim>::w() {
  return (*this)[3];
}


template<typename _Tp, int dim>
void Vec<_Tp, dim>::SetX(_Tp x) {
  (*this)[0] = x;
}


template<typename _Tp, int dim>
void Vec<_Tp, dim>::SetY(_Tp y) {
  (*this)[1] = y;
}


template<typename _Tp, int dim>
void Vec<_Tp, dim>::SetWidth(_Tp width) {
  if (dim != 2)
    throw std::logic_error("Only 2D vectors support"
                           " setting the x dimension via SetWidth().");
  SetX(width);
}


template<typename _Tp, int dim>
void Vec<_Tp, dim>::SetHeight(_Tp height) {
  if (dim != 2)
    throw std::logic_error("Only 2D vectors support"
                           " setting the x dimension via SetHeight().");
  SetY(height);
}


template<typename _Tp, int dim>
void Vec<_Tp, dim>::SetZ(_Tp z) {
  (*this)[2] = z;
}


template<typename _Tp, int dim>
void Vec<_Tp, dim>::SetW(_Tp w) {
  (*this)[3] = w;
}


template<typename _Tp, int dim>
Vec<_Tp, dim> &Vec<_Tp, dim>::operator+=(const Vec<_Tp, dim>& rhs) {
  for (int i = 0; i < dim; ++i)
    val[i] += rhs[i];
  return *this;
}


template<typename _Tp, int dim>
Vec<_Tp, dim> &Vec<_Tp, dim>::operator+=(double value) {
  for (int i = 0; i < dim; ++i)
    val[i] += value;
  return *this;
}


template<typename _Tp, int dim>
Vec<_Tp, dim> &Vec<_Tp, dim>::operator-=(const Vec<_Tp, dim>& rhs) {
  for (int i = 0; i < dim; ++i)
    val[i] -= rhs[i];
  return *this;
}


template<typename _Tp, int dim>
Vec<_Tp, dim> &Vec<_Tp, dim>::operator-=(double value) {
  for (int i = 0; i < dim; ++i)
    val[i] -= value;
  return *this;
}


template<typename _Tp, int dim>
Vec<_Tp, dim> &Vec<_Tp, dim>::operator*=(double scale) {
  for (int i = 0; i < dim; ++i)
    val[i] *= scale;
  return *this;
}


template<typename _Tp, int dim>
Vec<_Tp, dim> &Vec<_Tp, dim>::operator/=(double scale) {
  for (int i = 0; i < dim; ++i)
    val[i] /= scale;
  return *this;
}


template<typename _Tp, int dim>
Vec<_Tp, dim> Vec<_Tp, dim>::operator-() const {
  Vec<_Tp, dim> cp(*this);
  for (int i = 0; i < dim; ++i) {
    cp[i] *= -1.0;
  }
  return cp;
}


template<typename _Tp, int dim>
_Tp Vec<_Tp, dim>::MaxValue() const {
  _Tp max = val[0];
  for (int i = 1; i < dim; ++i) {
    if (val[i] > max)
      max = val[i];
  }
  return max;
}


template<typename _Tp, int dim>
_Tp Vec<_Tp, dim>::MinValue() const {
  _Tp min = val[0];
  for (int i = 1; i < dim; ++i) {
    if (val[i] < min)
      min = val[i];
  }
  return min;
}


template<typename _Tp, int dim>
int Vec<_Tp, dim>::MaxIndex() const {
  int max_idx = 0;
  for (int i = 1; i < dim; ++i) {
    if (val[i] > val[max_idx])
      max_idx = i;
  }
  return max_idx;
}


template<typename _Tp, int dim>
int Vec<_Tp, dim>::MinIndex() const {
  int min_idx = 0;
  for (int i = 1; i < dim; ++i) {
    if (val[i] < val[min_idx])
      min_idx = i;
  }
  return min_idx;
}


template<typename _Tp, int dim>
_Tp Vec<_Tp, dim>::Dot(const Vec<_Tp, dim>& other) const {
  _Tp s = static_cast<_Tp>(0);
  for (int i = 0; i < dim; ++i)
    s += val[i] * other.val[i];
  return s;
}


template<typename _Tp, int dim>
Vec<_Tp, dim> Vec<_Tp, dim>::Cross(const Vec<_Tp, dim>& other) const {
  if (dim != 3)
    throw std::logic_error("Cross product is only defined for 3d vectors!");
  // There's actually an analog for 2d space, but I didn't need
  // it yet: https://mathworld.wolfram.com/CrossProduct.html

  return Vec<_Tp, dim>(val[1] * other.val[2] - val[2] * other.val[1],
                       val[2] * other.val[0] - val[0] * other.val[2],
                       val[0] * other.val[1] - val[1] * other.val[0]);
}


template<typename _Tp, int dim>
double Vec<_Tp, dim>::Length() const {
  return std::sqrt(LengthSquared());
}


template<typename _Tp, int dim>
double Vec<_Tp, dim>::LengthSquared() const {
  return static_cast<double>(Dot(*this));
}


template<typename _Tp, int dim>
double Vec<_Tp, dim>::Distance(const Vec<_Tp, dim>& other) const {
  auto diff = *this - other;
  return diff.Length();
}


template<typename _Tp, int dim>
Vec<_Tp, dim> Vec<_Tp, dim>::DirectionVector(const Vec<_Tp, dim>& to) const {
  return to - *this;
}


template<typename _Tp, int dim>
Vec<double, dim> Vec<_Tp, dim>::UnitVector() const {
  const double len = Length();

  if (len > 0.0) {
    return static_cast<Vec<double, dim>>(*this) / len;
  } else {
    return static_cast<Vec<double, dim>>(*this);
  }
}


// Typename to char lookup:
template<typename _Tp> char VecType();
template<> char VecType<unsigned char>() { return 'b'; }
template<> char VecType<short>()         { return 's'; }
template<> char VecType<int>()           { return 'i'; }
template<> char VecType<double>()        { return 'd'; }


template<typename _Tp, int dim>
std::string Vec<_Tp, dim>::TypeName() {
  std::stringstream s;
  s << "Vec" << dim << VecType<_Tp>();
  return s.str();
}


template<typename _Tp, int dim>
Vec<_Tp, dim> Vec<_Tp, dim>::All(_Tp value) {
  Vec<_Tp, dim> vec;
  for (int i = 0; i < dim; ++i) {
    vec.val[i] = value;
  }
  return vec;
}


template<typename _Tp, int dim>
std::string Vec<_Tp, dim>::ToString() const {
  std::stringstream s;
  s << Vec<_Tp, dim>::TypeName() << "("
    << std::fixed << std::setprecision(2);

  for (int i = 0; i < dim; ++i) {
    s << val[i];
    if (i < dim -1)
      s << ", ";
  }

  s << ")";
  return s.str();
}

//---------------------------------------------------- Vector operators
template<typename _Tp, int dim>
bool operator==(const Vec<_Tp, dim>& lhs, const Vec<_Tp, dim>& rhs) {
  for (int i = 0; i < dim; ++i) {
    if (!eps_equal(lhs.val[i], rhs.val[i])) {
      return false;
    }
  }
  return true;
}


template<typename _Tp, int dim>
bool operator!=(const Vec<_Tp, dim>& lhs, const Vec<_Tp, dim>& rhs) {
  return !(lhs == rhs);
}


template<typename _Tp, int dim>
Vec<_Tp, dim> operator+(Vec<_Tp, dim> lhs, const Vec<_Tp, dim>& rhs) {
  lhs += rhs;
  return lhs;
}


template<typename _Tp, int dim>
Vec<_Tp, dim> operator-(Vec<_Tp, dim> lhs, const Vec<_Tp, dim>& rhs) {
  lhs -= rhs;
  return lhs;
}


template<typename _Tp, int dim>
Vec<_Tp, dim> operator+(Vec<_Tp, dim> lhs, double rhs) {
  lhs += rhs;
  return lhs;
}


template<typename _Tp, int dim>
Vec<_Tp, dim> operator-(Vec<_Tp, dim> lhs, double rhs) {
  lhs -= rhs;
  return lhs;
}


template<typename _Tp, int dim>
Vec<_Tp, dim> operator*(Vec<_Tp, dim> lhs, double scale) {
  lhs *= scale;
  return lhs;
}


template<typename _Tp, int dim>
Vec<_Tp, dim> operator*(double scale, Vec<_Tp, dim> rhs) {
  rhs *= scale;
  return rhs;
}


template<typename _Tp, int dim>
Vec<_Tp, dim> operator/(Vec<_Tp, dim> lhs, double scale) {
  lhs /= scale;
  return lhs;
}


template<typename _Tp, int dim>
double LengthPolygon(const std::vector<Vec<_Tp, dim>> &points) {
  double length = 0.0;
  for (std::size_t idx = 1; idx < points.size(); ++idx) {
    length += points[idx-1].Distance(points[idx]);
  }
  return length;
}


//---------------------------------------------------- Explicit vector instantiation
// Explicit instantiation:
template class Vec<double, 2>;
template class Vec<double, 3>;
template class Vec<double, 4>;

template class Vec<int, 2>;
template class Vec<int, 3>;


// Comparison Vec2d
template bool operator==(const Vec2d& lhs, const Vec2d& rhs);
template bool operator!=(const Vec2d& lhs, const Vec2d& rhs);
// Arithmetic Vec2d
template Vec2d operator+(Vec2d lhs, const Vec2d& rhs);
template Vec2d operator-(Vec2d lhs, const Vec2d& rhs);
template Vec2d operator+(Vec2d lhs, double rhs);
template Vec2d operator-(Vec2d lhs, double rhs);
template Vec2d operator*(Vec2d lhs, double scale);
template Vec2d operator*(double scale, Vec2d rhs);
template Vec2d operator/(Vec2d lhs, double scale);
template double LengthPolygon(const std::vector<Vec2d> &points);


// Comparison Vec3d
template bool operator==(const Vec3d& lhs, const Vec3d& rhs);
template bool operator!=(const Vec3d& lhs, const Vec3d& rhs);
// Arithmetic Vec3d
template Vec3d operator+(Vec3d lhs, const Vec3d& rhs);
template Vec3d operator-(Vec3d lhs, const Vec3d& rhs);
template Vec3d operator+(Vec3d lhs, double rhs);
template Vec3d operator-(Vec3d lhs, double rhs);
template Vec3d operator*(Vec3d lhs, double scale);
template Vec3d operator*(double scale, Vec3d rhs);
template Vec3d operator/(Vec3d lhs, double scale);
template double LengthPolygon(const std::vector<Vec3d> &points);


// Comparison Vec4d
template bool operator==(const Vec4d& lhs, const Vec4d& rhs);
template bool operator!=(const Vec4d& lhs, const Vec4d& rhs);
// Arithmetic Vec4d
template Vec4d operator+(Vec4d lhs, const Vec4d& rhs);
template Vec4d operator-(Vec4d lhs, const Vec4d& rhs);
template Vec4d operator+(Vec4d lhs, double rhs);
template Vec4d operator-(Vec4d lhs, double rhs);
template Vec4d operator*(Vec4d lhs, double scale);
template Vec4d operator*(double scale, Vec4d rhs);
template Vec4d operator/(Vec4d lhs, double scale);
template double LengthPolygon(const std::vector<Vec4d> &points);


// Comparison Vec2i
template bool operator==(const Vec2i& lhs, const Vec2i& rhs);
template bool operator!=(const Vec2i& lhs, const Vec2i& rhs);
// Arithmetic Vec2i
template Vec2i operator+(Vec2i lhs, const Vec2i& rhs);
template Vec2i operator-(Vec2i lhs, const Vec2i& rhs);
template Vec2i operator+(Vec2i lhs, double rhs);
template Vec2i operator-(Vec2i lhs, double rhs);
template Vec2i operator*(Vec2i lhs, double scale);
template Vec2i operator*(double scale, Vec2i rhs);
template Vec2i operator/(Vec2i lhs, double scale);
template double LengthPolygon(const std::vector<Vec2i> &points);


// Comparison Vec3i
template bool operator==(const Vec3i& lhs, const Vec3i& rhs);
template bool operator!=(const Vec3i& lhs, const Vec3i& rhs);
// Arithmetic Vec3i
template Vec3i operator+(Vec3i lhs, const Vec3i& rhs);
template Vec3i operator-(Vec3i lhs, const Vec3i& rhs);
template Vec3i operator+(Vec3i lhs, double rhs);//FIXME change rhs type for integer vectors?
template Vec3i operator-(Vec3i lhs, double rhs);
template Vec3i operator*(Vec3i lhs, double scale);
template Vec3i operator*(double scale, Vec3i rhs);
template Vec3i operator/(Vec3i lhs, double scale);
template double LengthPolygon(const std::vector<Vec3i> &points);


//---------------------------------------------------- Math/Geometry Helpers
Vec2d ProjectPointOntoLine(const Vec2d &pt,
                           const Vec2d &line_from, const Vec2d &line_to) {


  // Vector from line start to point:
  const Vec2d v = line_from.DirectionVector(pt);
  // Project point onto line via dot product:
  const Vec2d unit_direction = line_from.DirectionVector(line_to).UnitVector();
  const double lambda = unit_direction.Dot(v);
  return line_from + lambda * unit_direction;
}


double Determinant(const Vec2d &a, const Vec2d &b) {
  return a.x() * b.y() - b.x() * a.y();
}


double AngleRadFromDirectionVec(const Vec2d &vec) {
  // Dot product is proportional to the cosine, whereas
  // the determinant is proportional to the sine.
  // See: https://math.stackexchange.com/a/879474
  Vec2d ref(1, 0);
  Vec2d unit = vec.UnitVector();
  return std::atan2(Determinant(ref, unit), ref.Dot(unit));
}


double AngleDegFromDirectionVec(const Vec2d &vec) {
  return rad2deg(AngleRadFromDirectionVec(vec));
}


Vec2d DirectionVecFromAngleRad(double rad) {
  return Vec2d(std::cos(rad), std::sin(rad)); // TODO verify it's unit length
}


Vec2d DirectionVecFromAngleDeg(double deg) {
  return DirectionVecFromAngleRad(deg2rad(deg));
}

} // namespace geometry
} // namespace werkzeugkiste
