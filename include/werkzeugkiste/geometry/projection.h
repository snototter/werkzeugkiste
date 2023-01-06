#ifndef WERKZEUGKISTE_GEOMETRY_PROJECTION_H
#define WERKZEUGKISTE_GEOMETRY_PROJECTION_H

#include <cmath>
#include <tuple>
#include <utility>
#include <type_traits>

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/Eigen>

#include <werkzeugkiste/geometry/utils.h>
#include <werkzeugkiste/geometry/vector.h>


namespace werkzeugkiste::geometry {

//-----------------------------------------------------------------------------
// Matrix definitions

template <typename _Tp, int Rows, int Columns>
using Matrix = Eigen::Matrix<_Tp, Rows, Columns, (Columns > 1) ? Eigen::RowMajor : 0>;

template <typename _Tp, int Rows>
using MatrixDynWidth = Eigen::Matrix<_Tp, Rows, Eigen::Dynamic, Eigen::RowMajor>;


//using Mat2x2d = Matrix<double, 2, 2>;
using Mat3x3d = Matrix<double, 3, 3>;
using Mat3x4d = Matrix<double, 3, 4>;


//-----------------------------------------------------------------------------
// Conversion between werkzeugkiste and Eigen

/// Converts a werkzeugkiste vector to an Eigen vector (single-column matrix).
template <int Rows, typename _V> inline
Matrix<typename _V::value_type, Rows, 1>
VecToEigenMat(const _V &vec) {
  static_assert(
    (Rows == _V::ndim) || (Rows == (_V::ndim + 1)),
    "Invalid number of rows for the matrix - must be either the vector "
    "dimension or 1 more (for automatically added homogeneous coordinate)!");

  Matrix<typename _V::value_type, Rows, 1> mat;
  for (int i = 0; i < _V::ndim; ++i) {
    mat[i] = vec[i];
  }

  if (Rows == _V::ndim + 1) {
    // Add homogeneous coordinate:
    mat[_V::ndim] = static_cast<typename _V::value_type>(1);
  }
  return mat;
}


/// Returns a vector for the given matrix column.
template <typename _Tp, int Rows, int Columns>
Vec<_Tp, Rows> EigenColToVec(
    const Matrix<_Tp, Rows, Columns> &eig, int col) {
//    const VectorBase<typename _Tp::value_type, _Tp::ndim> &eig) {
  Vec<_Tp, Rows> v;
  for (int i = 0; i < Rows; ++i) {
    v[i] = eig(i, col);
  }
  return v;
}


/// Returns a matrix where each column holds one vector.
/// Caution:
///   Matrix size is fixed at compile time, thus it will be created
///   on the stack. Should only be used for a maximum of up to 32 vectors.
///   Refer to the Eigen3 docs for details:
///   https://eigen.tuxfamily.org/dox/group__TutorialMatrixClass.html
template <int Dim, typename _V, typename... _Vs> inline
Matrix<typename _V::value_type, Dim, 1 + sizeof...(_Vs)>
VecsToEigenMat(
    const _V &vec0, const _Vs &... others) {
  static_assert(
    (Dim == _V::ndim) || (Dim == (_V::ndim + 1)),
    "Invalid number of rows for the matrix - must be either the vector "
    "dimension or 1 more (for homogeneous)!");

  const int num_vecs = 1 + sizeof...(others);
  static_assert(
    num_vecs <= 32,
    "Fixed size matrices should not be used for operations "
    "with more than (roughly) 32 vectors.");

  const _V vecs[num_vecs] {
    vec0, static_cast<const _V &>(others)...};

  Matrix<typename _V::value_type, Dim, num_vecs> m;
  for (int row = 0; row < _V::ndim; ++row) {
    for (int col = 0; col < num_vecs; ++col) {
      m(row, col) = vecs[col][row];
    }
  }

  if (Dim == _V::ndim + 1) {
    // Add homogeneous coordinate:
    for (int col = 0; col < num_vecs; ++col) {
      m(_V::ndim, col) = static_cast<typename _V::value_type>(1);
    }
  }
  return m;
}


///// Base case for making a tuple out of a variadic number of vectors.
//template <typename _Tp> inline
//std::tuple<_Tp> VecsToTuple(_Tp &&vec) {
//  return std::tuple<_Tp>(std::move(vec));
//}


///// Returns a tuple of vectors.
//template <typename _V0, typename _V1, typename... _Vs> inline
//std::tuple<_V0, _V1, _Vs...>
//VecsToTuple(
//    _V0 &&v0, _V1 &&v1, _Vs&&... others) {
//  return std::tuple_cat(
//        std::tuple<_V0>(std::move(v0)),
//        VecsToTuple<_V1, _Vs...>(std::move(v1), std::move(others)...));
//}


template <typename Array, std::size_t... Idx> inline
auto ArrayToTuple(const Array &arr, std::index_sequence<Idx...>) {
  return std::make_tuple(arr[Idx]...);
}


/// Returns a tuple of vectors (one vector per matrix column).
template <typename _Tp, int Rows, int Columns> inline
auto EigenMatToVecTuple(
    const Matrix<_Tp, Rows, Columns> &vec_mat) {
  std::array<Vec<_Tp, Rows>, Columns> arr;
  for (int c = 0; c < Columns; ++c) {
    arr[c] = EigenColToVec<_Tp, Rows, Columns>(vec_mat, c);//vec_mat.col(c));
  }
  return ArrayToTuple(arr, std::make_index_sequence<Columns>{});
}


//-----------------------------------------------------------------------------
// Transformation/projection utitilites

/// Computes `mat * [vec0, vec1, ...]` and returns the result as a tuple
/// of vectors.
///
/// The vector dimensionality must be either equal to or 1 less than the number
/// of projection matrix columns. If not equal, a homogeneous coordinate will
/// be implicitly added to each vector, *i.e.* [vec0.x(), vec0.y(), ..., 1].
///
/// Example:
///   >>> wkg::Vec2d v1{17, 42}, v2{9, -3}, v3{0, 0.01};
///   >>> wkg::Matrix<double, 4, 2> M;
///   >>> M << 1, 2, 3, 4, 5, 6, 7, 8;
///   >>> wkg::Vec4d a, b, c;
///   >>> std::tie(out1, out2, out3) = TransformToVecs(
///   >>>   T_matrix, in1, in2, in3);
///   >>> // Or with padded homogeneous coordinate:
///   >>> wkg::Matrix<double, 4, 3> M;
///   >>> M << 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12;
///   >>> wkg::Vec4d a, b, c;
///   >>> std::tie(out1, out2, out3) = TransformToVecs(
///   >>>   T_matrix, in1, in2, in3);
template <typename _V, typename... _Vs, int Rows, int Columns> inline
std::tuple<
  Vec<typename _V::value_type, Rows>,
  Vec<typename _Vs::value_type, Rows>...>
TransformToVecs(
      const Matrix<typename _V::value_type, Rows, Columns> &mat,
      const _V &vec0, const _Vs &... others) {
  static_assert(
      (Columns == _V::ndim) || (Columns == (_V::ndim + 1)),
      "Invalid dimensions: vector dimensionality must be equal or 1 less than"
      "the number of matrix columns!");

  constexpr int num_vecs = 1 + sizeof...(others);
  const Matrix<
      typename _V::value_type, Columns, num_vecs> vec_mat = VecsToEigenMat<Columns>(vec0, others...);
  const auto transformed = mat * vec_mat;
  return EigenMatToVecTuple<typename _V::value_type, Rows, num_vecs>(transformed);
}


/// Convenience utility to avoid using std::tie() for a single vector.
/// See `TransformToVecs`.
template <typename _V, typename... _Vs, int Rows, int Columns> inline
Vec<typename _V::value_type, Rows>
TransformToVec(
      const Matrix<typename _V::value_type, Rows, Columns> &mat,
      const _V &vec) {
  static_assert(
      (Columns == _V::ndim) || (Columns == (_V::ndim + 1)),
      "Invalid dimensions: vector dimensionality must be equal or 1 less than"
      "the number of matrix columns!");
  const Matrix<
      typename _V::value_type, Columns, 1> vec_mat = VecToEigenMat<Columns>(vec);
  const auto transformed = mat * vec_mat;
  return EigenColToVec<typename _V::value_type, Rows, 1>(transformed, 0);
}


/// Computes `mat * [vec0, vec1, ...]`, divides the result by the
/// homogeneous coordinate (*i.e.* last row) and returns a tuple of vectors.
///
/// The vector dimensionality must be either equal to or 1 less than the number
/// of projection matrix columns. If not equal, a homogeneous coordinate will
/// be implicitly added to each vector, *i.e.* [vec0.x(), vec0.y(), ..., 1].
///
/// Example:
///   >>> wkg::Vec3d v1{17, 42, 1}, v2{9, -3, 1};
///   >>> wkg::Matrix<double, 3, 3> H;
///   >>> H << 1, 2, 3,
///   >>>      4, 5, 6,
///   >>>      7, 8, 9;
///   >>> wkg::Vec2d out1, out2;
///   >>> std::tie(out1, out2) = ProjectToVecs(H, v1, v2);
template <typename _V, typename... _Vs, int Rows, int Columns> inline
std::tuple<
  Vec<typename _V::value_type, Rows - 1>,
  Vec<typename _Vs::value_type, Rows - 1>...>
ProjectToVecs(
      const Matrix<typename _V::value_type, Rows, Columns> &mat,
      const _V &vec0, const _Vs &... others) {
  static_assert(
      (Columns == _V::ndim) || (Columns == (_V::ndim + 1)),
      "Invalid dimensions: vector dimensionality must be equal or 1 less than"
      "the number of matrix columns!");

  constexpr int num_vecs = 1 + sizeof...(others);

  const Matrix<
      typename _V::value_type, Columns, num_vecs> vec_mat = VecsToEigenMat<Columns>(vec0, others...);
  const auto transformed = mat * vec_mat;
  const auto projected = transformed.colwise().hnormalized();
  return EigenMatToVecTuple<typename _V::value_type, Rows - 1, num_vecs>(projected);
}


/// Convenience utility to avoid using std::tie() for a single vector.
/// See `ProjectToVecs`.
template <typename _V, typename... _Vs, int Rows, int Columns> inline
Vec<typename _V::value_type, Rows - 1>
ProjectToVec(
      const Matrix<typename _V::value_type, Rows, Columns> &mat,
      const _V &vec) {
  static_assert(
      (Columns == _V::ndim) || (Columns == (_V::ndim + 1)),
      "Invalid dimensions: vector dimensionality must be equal or 1 less than"
      "the number of matrix columns!");
  const Matrix<
      typename _V::value_type, Columns, 1> vec_mat = VecToEigenMat<Columns>(vec);
  const auto transformed = mat * vec_mat;
  const auto projected = transformed.colwise().hnormalized();
  return EigenColToVec<typename _V::value_type, Rows - 1, 1>(projected, 0);
}


//-----------------------------------------------------------------------------
// Rotation utilities

/// Returns the 3x3 rotation matrix, rotating around the x-axis.
template <typename _Tp> inline constexpr
Matrix<_Tp, 3, 3> RotationX(double angle, bool angle_in_deg) {
  const double rad = angle_in_deg ? Deg2Rad(angle) : angle;
  const double ct = std::cos(rad);
  const double st = std::sin(rad);

  Matrix<_Tp, 3, 3> R;
  R << 1.0, 0.0, 0.0,
       0.0,  ct, -st,
       0.0,  st,  ct;
  return R;
}


/// Returns the 3x3 rotation matrix, rotating around the y-axis.
template <typename _Tp> inline constexpr
Matrix<_Tp, 3, 3> RotationY(double angle, bool angle_in_deg) {
  const double rad = angle_in_deg ? Deg2Rad(angle) : angle;
  const double ct = std::cos(rad);
  const double st = std::sin(rad);

  Matrix<_Tp, 3, 3> R;
  R << ct, 0.0,  st,
      0.0, 1.0, 0.0,
      -st, 0.0,  ct;
  return R;
}


/// Returns the 3x3 rotation matrix, rotating around the z-axis.
template <typename _Tp> inline constexpr
Matrix<_Tp, 3, 3> RotationZ(double angle, bool angle_in_deg) {
  const double rad = angle_in_deg ? Deg2Rad(angle) : angle;
  const double ct = std::cos(rad);
  const double st = std::sin(rad);

  Matrix<_Tp, 3, 3> R;
  R << ct, -st, 0.0,
       st,  ct, 0.0,
      0.0, 0.0, 1.0;
  return R;
}


/// Returns the 3x3 rotation matrix in ZYX order.
template <typename _Tp> inline constexpr
Matrix<_Tp, 3, 3> RotationMatrix(
    double angle_x, double angle_y, double angle_z, bool angles_in_deg) {
  auto Rx = RotationX<_Tp>(angle_x, angles_in_deg);
  auto Ry = RotationY<_Tp>(angle_y, angles_in_deg);
  auto Rz = RotationZ<_Tp>(angle_z, angles_in_deg);
  return Rx * (Ry * Rz);
}

// TODO(vcp) RotationMatrixToEulerAngles

} // namespace werkzeugkiste::geometry

#endif // WERKZEUGKISTE_GEOMETRY_PROJECTION_H
