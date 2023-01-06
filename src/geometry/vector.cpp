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

namespace werkzeugkiste::geometry {

template<typename T, int Dim>
Vec<T, Dim>::Vec(T x, T y) {
  if (Dim != 2) {
    std::stringstream s;
    s << "You cannot initialize " << TypeName() << " with 2 values.";
    throw std::invalid_argument(s.str());
  }
  val[0] = x;
  val[1] = y;
}


template<typename T, int Dim>
Vec<T, Dim>::Vec(T x, T y, T z) {
  if (Dim != 3) {
    std::stringstream s;
    s << "You cannot initialize " << TypeName() << " with 3 values.";
    throw std::invalid_argument(s.str());
  }
  val[0] = x;
  val[1] = y;
  val[2] = z;
}


template<typename T, int Dim>
Vec<T, Dim>::Vec(T x, T y, T z, T w) {
  if (Dim != 4) {
    std::stringstream s;
    s << "You cannot initialize " << TypeName() << " with 4 values.";
    throw std::invalid_argument(s.str());
  }
  val[0] = x;
  val[1] = y;
  val[2] = z;
  val[3] = w;
}


template<typename T, int Dim>
Vec<T, Dim>::Vec(std::initializer_list<T> values) {
  if ((values.size() != 0) &&
      (values.size() != static_cast<size_t>(Dim))) {
    std::ostringstream s;
    s << "You cannot initialize " << TypeName()
      << " with " << values.size() << " values. The initializer list must"
         " either be empty, or contain " << Dim << " values!";
    throw std::invalid_argument(s.str());
  }

  if (values.size() == 0) {
    for (int i = 0; i < Dim; ++i) {
      val[i] = static_cast<T>(0);
    }
  } else {
    for (size_t i = 0; i < values.size(); ++i) {
      val[i] = values.begin()[i];
    }
  }
}


template<typename T, int Dim>
Vec<T, Dim>::Vec(const Vec<T, Dim>& other) {
  for (int i = 0; i < Dim; ++i) {
    val[i] = other.val[i];
  }
}


template<typename T, int Dim>
Vec<T, Dim>::Vec(Vec<T, Dim> &&other) noexcept {
  for (int i = 0; i < Dim; ++i) {
    val[i] = other.val[i];
  }
}


template<typename T, int Dim>
Vec<T, Dim>::operator Vec<double, Dim>() const {
  Vec<double, Dim> conv;
  for (int i = 0; i < Dim; ++i) {
    conv.val[i] = static_cast<double>(val[i]);
  }
  return conv;
}


template<typename T, int Dim>
const T& Vec<T, Dim>::operator[](int i) const {
  if (i < 0) {
    i += Dim;
  }
  if ((i < 0) ||(i >= Dim)) {
    std::stringstream s;
    s << "Index-out-of-bounds: cannot access element at ["
      << i << "] for " << TypeName() << ".";
    throw std::out_of_range(s.str());
  }
  return val[i];
}


template<typename T, int Dim>
T& Vec<T, Dim>::operator[](int i) {
  if (i < 0) {
    i += Dim;
  }
  if ((i < 0) ||(i >= Dim)) {
    std::stringstream s;
    s << "Index-out-of-bounds: cannot access element at ["
      << i << "] for " << TypeName() << ".";
    throw std::out_of_range(s.str());
  }
  return val[i];
}


template<typename T, int Dim>
Vec<T, Dim> &Vec<T, Dim>::operator=(const Vec<T, Dim> &other) {
  if (this != &other) {
    for (int i = 0; i < Dim; ++i) {
      val[i] = other.val[i];
    }
  }
  return *this;
}


template<typename T, int Dim>
Vec<T, Dim> &Vec<T, Dim>::operator=(Vec<T, Dim> &&other) noexcept {
  if (this != &other) {
    for (int i = 0; i < Dim; ++i) {
      val[i] = other.val[i];
    }
  }
  return *this;
}




template<typename T, int Dim>
Vec<T, Dim> &Vec<T, Dim>::operator+=(const Vec<T, Dim>& rhs) {
  for (int i = 0; i < Dim; ++i) {
    val[i] += rhs[i];
  }
  return *this;
}


template<typename T, int Dim>
Vec<T, Dim> &Vec<T, Dim>::operator+=(double value) {
  for (int i = 0; i < Dim; ++i) {
    val[i] += value;
  }
  return *this;
}


template<typename T, int Dim>
Vec<T, Dim> &Vec<T, Dim>::operator-=(const Vec<T, Dim>& rhs) {
  for (int i = 0; i < Dim; ++i) {
    val[i] -= rhs[i];
  }
  return *this;
}


template<typename T, int Dim>
Vec<T, Dim> &Vec<T, Dim>::operator-=(double value) {
  for (int i = 0; i < Dim; ++i) {
    val[i] -= value;
  }
  return *this;
}


template<typename T, int Dim>
Vec<T, Dim> &Vec<T, Dim>::operator*=(double scale) {
  for (int i = 0; i < Dim; ++i) {
    val[i] *= scale;
  }
  return *this;
}


template<typename T, int Dim>
Vec<T, Dim> &Vec<T, Dim>::operator/=(double scale) {
  for (int i = 0; i < Dim; ++i) {
    val[i] /= scale;
  }
  return *this;
}


template<typename T, int Dim>
Vec<T, Dim> Vec<T, Dim>::operator-() const {
  Vec<T, Dim> cp(*this);
  for (int i = 0; i < Dim; ++i) {
    cp[i] *= -1;
  }
  return cp;
}


template<typename T, int Dim>
T Vec<T, Dim>::MaxValue() const {
  T max = val[0];
  for (int i = 1; i < Dim; ++i) {
    if (val[i] > max) {
      max = val[i];
    }
  }
  return max;
}


template<typename T, int Dim>
T Vec<T, Dim>::MinValue() const {
  T min = val[0];
  for (int i = 1; i < Dim; ++i) {
    if (val[i] < min) {
      min = val[i];
    }
  }
  return min;
}


template<typename T, int Dim>
int Vec<T, Dim>::MaxIndex() const {
  int max_idx = 0;
  for (int i = 1; i < Dim; ++i) {
    if (val[i] > val[max_idx]) {
      max_idx = i;
    }
  }
  return max_idx;
}


template<typename T, int Dim>
int Vec<T, Dim>::MinIndex() const {
  int min_idx = 0;
  for (int i = 1; i < Dim; ++i) {
    if (val[i] < val[min_idx]) {
      min_idx = i;
    }
  }
  return min_idx;
}


template<typename T, int Dim>
T Vec<T, Dim>::Dot(const Vec<T, Dim>& other) const {
  T s = static_cast<T>(0);  // NOLINT
  for (int i = 0; i < Dim; ++i) {
    s += val[i] * other.val[i];
  }
  return s;
}


template<typename T, int Dim>
Vec<T, Dim> Vec<T, Dim>::Cross(const Vec<T, Dim>& other) const {
  if (Dim != 3) {
      throw std::logic_error(
          "Cross product is only defined for 3d vectors! For 2d vectors, use"
          " Cross2d instead!");
  }

  return Vec<T, Dim>{
      val[1] * other.val[2] - val[2] * other.val[1],
      val[2] * other.val[0] - val[0] * other.val[2],
      val[0] * other.val[1] - val[1] * other.val[0]
  };
}


template<typename T, int Dim>
double Vec<T, Dim>::Length() const {
  return std::sqrt(LengthSquared());
}


template<typename T, int Dim>
double Vec<T, Dim>::LengthSquared() const {
  return static_cast<double>(Dot(*this));
}


template<typename T, int Dim>
double Vec<T, Dim>::Distance(const Vec<T, Dim>& other) const {
  auto diff = *this - other;
  return diff.Length();
}


template<typename T, int Dim>
double Vec<T, Dim>::DistanceManhattan(const Vec<T, Dim>& other) const {
  auto diff = *this - other;
  double abs_sum = 0.0;
  for (int i = 0; i < Dim; ++i) {
    abs_sum += std::abs(diff[i]);
  }
  return abs_sum;
}


template<typename T, int Dim>
Vec<T, Dim> Vec<T, Dim>::DirectionVector(const Vec<T, Dim>& to) const {
  return to - *this;
}


template<typename T, int Dim>
Vec<double, Dim> Vec<T, Dim>::UnitVector() const {
  const double len = Length();

  if (len > 0.0) {
    return static_cast<Vec<double, Dim>>(*this) / len;
  } else {  // NOLINT
    return Vec<double, Dim>();  // Return 0-vector
  }
}


// Typename to char lookup:
template<typename T> char VecType();
template<> char VecType<unsigned char>() { return 'b'; }
template<> char VecType<short>()         { return 's'; }
template<> char VecType<int>()           { return 'i'; }
template<> char VecType<double>()        { return 'd'; }


template<typename T, int Dim>
std::string Vec<T, Dim>::TypeName() {
  std::stringstream s;
  s << "Vec" << Dim << VecType<T>();
  return s.str();
}


template<typename T, int Dim>
Vec<T, Dim> Vec<T, Dim>::All(T value) {
  Vec<T, Dim> vec;
  for (int i = 0; i < Dim; ++i) {
    vec.val[i] = value;
  }
  return vec;
}


template<typename T, int Dim>
std::string Vec<T, Dim>::ToString(bool include_type) const {
  std::stringstream s;
  if (include_type) {
    s << Vec<T, Dim>::TypeName();
  }
  s << '(' << std::fixed << std::setprecision(2);

  for (int i = 0; i < Dim; ++i) {
    s << val[i];
    if (i < Dim -1) {
      s << ", ";
    }
  }

  s << ')';
  return s.str();
}

//---------------------------------------------------- Vector operators
template<typename T, int Dim>
bool operator==(const Vec<T, Dim>& lhs, const Vec<T, Dim>& rhs) {
  for (int i = 0; i < Dim; ++i) {
    if (!IsEpsEqual(lhs.val[i], rhs.val[i])) {
      return false;
    }
  }
  return true;
}


template<typename T, int Dim>
bool operator!=(const Vec<T, Dim>& lhs, const Vec<T, Dim>& rhs) {
  return !(lhs == rhs);
}


template<typename T, int Dim>
Vec<T, Dim> operator+(Vec<T, Dim> lhs, const Vec<T, Dim>& rhs) {
  lhs += rhs;
  return lhs;
}


template<typename T, int Dim>
Vec<T, Dim> operator-(Vec<T, Dim> lhs, const Vec<T, Dim>& rhs) {
  lhs -= rhs;
  return lhs;
}


template<typename T, int Dim>
Vec<T, Dim> operator+(Vec<T, Dim> lhs, double rhs) {
  lhs += rhs;
  return lhs;
}


template<typename T, int Dim>
Vec<T, Dim> operator-(Vec<T, Dim> lhs, double rhs) {
  lhs -= rhs;
  return lhs;
}


template<typename T, int Dim>
Vec<T, Dim> operator*(Vec<T, Dim> lhs, double rhs) {
  lhs *= rhs;
  return lhs;
}


template<typename T, int Dim>
Vec<T, Dim> operator*(double lhs, Vec<T, Dim> rhs) {
  rhs *= lhs;
  return rhs;
}


template<typename T, int Dim>
Vec<T, Dim> operator/(Vec<T, Dim> lhs, double rhs) {
  lhs /= rhs;
  return lhs;
}


template<typename T, int Dim>
double LengthPolygon(const std::vector<Vec<T, Dim>> &points) {
  double length = 0.0;
  for (std::size_t idx = 1; idx < points.size(); ++idx) {
    length += points[idx-1].Distance(points[idx]);
  }
  return length;
}


//----------------------------------------------------
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

} // namespace werkzeugkiste::geometry
