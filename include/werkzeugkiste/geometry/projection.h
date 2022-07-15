#ifndef __WERKZEUGKISTE_GEOMETRY_PROJECTION_H__
#define __WERKZEUGKISTE_GEOMETRY_PROJECTION_H__

#include <cmath>
#include <tuple>
#include <utility>
#include <type_traits>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <werkzeugkiste/geometry/utils.h>
#include <werkzeugkiste/geometry/vector.h>


namespace werkzeugkiste {
namespace geometry {

template <typename _Tp, int Rows, int Columns>
using Matrix = Eigen::Matrix<_Tp, Rows, Columns, (Columns > 1) ? Eigen::RowMajor : 0>;

template <typename _Tp, int Rows>
using MatrixDynWidth = Eigen::Matrix<_Tp, Rows, Eigen::Dynamic, Eigen::RowMajor>;

//// row-major layout is not supported for vectors in eigen (and being a vector, the layout doesn't matter anyhow)
//template <typename _Tp, int Dimension>
//using VectorBase = Eigen::Matrix<_Tp, Dimension, 1>;

//using Mat2x2d = Matrix<double, 2, 2>;
using Mat3x3d = Matrix<double, 3, 3>;
using Mat3x4d = Matrix<double, 3, 4>;


/// Converts a werkzeugkiste vector to an Eigen vector (single-column matrix).
template <typename _V> inline
Matrix<typename _V::value_type, _V::ndim, 1>
VecToEigen(const _V &vec) {
  Matrix<typename _V::value_type, _V::ndim, 1> mat;
  for (int i = 0; i < _V::ndim; ++i) {
    mat[i] = vec[i];
  }
  return mat;
}


///// Converts a werkzeugkiste vector to an Eigen vector (i.e. single-column
///// matrix) and adds a homogeneous coordinate (set to 1).
//template <typename _Tp, int dim> inline
//VectorBase<_Tp, dim + 1> VecToEigenColHomogeneous(const Vec<_Tp, dim> &vec) {
//  VectorBase<_Tp, dim + 1> m;
//  for (int i = 0; i < dim; ++i) {
//    m[i] = vec[i];
//  }
//  m[dim] = static_cast<_Tp>(1);
//  return m;
//}


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
    "TODO warning text: Fixed size matrix...");

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


template <typename _Tp, int Rows, int Columns> inline
auto EigenMatToVecTuple(
    const Matrix<_Tp, Rows, Columns> &vec_mat) {
  std::array<Vec<_Tp, Rows>, Columns> arr;
  for (int c = 0; c < Columns; ++c) {
    arr[c] = EigenColToVec<_Tp, Rows, Columns>(vec_mat, c);//vec_mat.col(c));
  }
  return ArrayToTuple(arr, std::make_index_sequence<Columns>{});
}


/// Computes `mat * [vec0, ...]` and returns the result as a matrix.
template <typename _V, typename... _Vs, int Rows> inline
Matrix<typename _V::value_type, Rows, 1 + sizeof...(_Vs)>
TransformToMat(
      const Matrix<typename _V::value_type, Rows, _V::ndim> &mat,
      const _V &vec0, const _Vs &... others) {
  return mat * VecsToEigenMat<_V::ndim>(vec0, others...);
}


/// Computes `mat * [vec0, vec1, ...]` and returns a tuple of vectors.
///
/// Example:
///   >>> wkg::Vec2d v1{17, 42}, v2{9, -3}, v3{0, 0.01};
///   >>> wkg::Matrix<double, 4, 2> M;
///   >>> M << 1, 2, 3, 4,
///   >>>      5, 6, 7, 8;
///   >>> wkg::Vec4d a, b, c;
///   >>> std::tie(out1, out2, out3) = TransformToVecs(
///   >>>   T_matrix, in1, in2, in3);
template <typename _V, typename... _Vs, int Rows> inline
std::tuple<
  Vec<typename _V::value_type, Rows>,
  Vec<typename _Vs::value_type, Rows>...>
TransformToVecs(
      const Matrix<typename _V::value_type, Rows, _V::ndim> &mat,
      const _V &vec0, const _Vs &... others) {

  constexpr int num_vecs = 1 + sizeof...(others);
  const auto transformed = TransformToMat(mat, vec0, others...);
  return EigenMatToVecTuple<typename _V::value_type, Rows, num_vecs>(transformed);
}






/// Computes `mat * [vec0, vec1, ...]` and divides the result by the
/// homogeneous coordinate (last row).
///
/// The vector dimensionality must match the number of columns of
/// the projection matrix. Alternatively, use `ProjectInhomogeneousToMat`
template <typename _V, typename... _Vs, int Rows> inline
Matrix<typename _V::value_type, Rows - 1, 1 + sizeof...(_Vs)>
ProjectHomogeneousToMat(
      const Matrix<typename _V::value_type, Rows, _V::ndim> &mat,
      const _V &vec0, const _Vs &... others) {

  constexpr int num_vecs = 1 + sizeof...(others);
  const Matrix<typename _V::value_type, Rows, num_vecs> transformed =
      mat * VecsToEigenMat<_V::ndim>(vec0, others...);

  return transformed.colwise().hnormalized();
}


/// Computes `mat * [vec0, vec1, ...]`, divides the result by the
/// homogeneous coordinate (last row) and returns a tuple of vectors.
///
/// The vector dimensionality must match the number of columns of
/// the projection matrix. Alternatively, use `ProjectInhomogeneousToMat`
///
/// Example:
///   >>> wkg::Vec3d v1{17, 42, 1}, v2{9, -3, 1};
///   >>> wkg::Matrix<double, 3, 3> H;
///   >>> H << 1, 2, 3,
///   >>>      4, 5, 6,
///   >>>      7, 8, 9;
///   >>> wkg::Vec2d out1, out2;
///   >>> std::tie(out1, out2) = TransformToVecs(H, v1, v2);
template <typename _V, typename... _Vs, int Rows> inline
std::tuple<
  Vec<typename _V::value_type, Rows - 1>,
  Vec<typename _Vs::value_type, Rows - 1>...>
ProjectHomogeneousToVecs(
      const Matrix<typename _V::value_type, Rows, _V::ndim> &mat,
      const _V &vec0, const _Vs &... others) {

  constexpr int num_vecs = 1 + sizeof...(others);
  const auto projected = ProjectHomogeneousToMat(mat, vec0, others...);
  return EigenMatToVecTuple<typename _V::value_type, Rows - 1, num_vecs>(projected);
}


/// Adds the homogeneous coordinate to each vector and then
/// computes `mat * [vec0, vec1, ...]`. Finally, the result is divided by the
/// homogeneous coordinate.
template <typename _V, typename... _Vs, int Rows> inline
Matrix<typename _V::value_type, Rows - 1, 1 + sizeof...(_Vs)>
ProjectInhomogeneousToMat(
      const Matrix<typename _V::value_type, Rows, _V::ndim + 1> &mat,
      const _V &vec0, const _Vs &... others) {

  constexpr int num_vecs = 1 + sizeof...(others);
  const Matrix<typename _V::value_type, Rows, num_vecs> transformed =
      mat * VecsToEigenMat<_V::ndim + 1>(vec0, others...);

  return transformed.colwise().hnormalized();
}


/// Adds the homogeneous coordinate to each vector and then
/// computes `mat * [vec0, vec1, ...]`. Finally, the result is divided by the
/// homogeneous coordinate.
///
/// Example:
///   >>> wkg::Vec2d v1{17, 42}, v2{9, -3};
///   >>> wkg::Matrix<double, 3, 3> H;
///   >>> H << 1, 2, 3,
///   >>>      4, 5, 6,
///   >>>      7, 8, 9;
///   >>> wkg::Vec2d out1, out2;
///   >>> std::tie(out1, out2) = TransformToVecs(H, v1, v2);
template <typename _V, typename... _Vs, int Rows> inline
std::tuple<
  Vec<typename _V::value_type, Rows - 1>,
  Vec<typename _Vs::value_type, Rows - 1>...>
ProjectInhomogeneousToVecs(
      const Matrix<typename _V::value_type, Rows, _V::ndim + 1> &mat,
      const _V &vec0, const _Vs &... others) {

  constexpr int num_vecs = 1 + sizeof...(others);
  const auto projected = ProjectInhomogeneousToMat(mat, vec0, others...);
  return EigenMatToVecTuple<typename _V::value_type, Rows - 1, num_vecs>(projected);
}


// TODO project with scale --> project, then scale.
// RotationMatrixToEulerAngles --> projection.h
// RotationX , Y, Z
// RotationMatrix

/// Returns the 3x3 rotation matrix, rotating around the x-axis.
template <typename _Tp> inline
Matrix<_Tp, 3, 3> RotationX(double angle, bool angle_in_deg) {
  const double rad = angle_in_deg ? deg2rad(angle) : angle;
  const double ct = std::cos(rad);
  const double st = std::sin(rad);

  Matrix<_Tp, 3, 3> R;
  R << 1.0, 0.0, 0.0,
       0.0,  ct, -st,
       0.0,  st,  ct;
  return R;
}


/// Returns the 3x3 rotation matrix, rotating around the y-axis.
template <typename _Tp> inline
Matrix<_Tp, 3, 3> RotationY(double angle, bool angle_in_deg) {
  const double rad = angle_in_deg ? deg2rad(angle) : angle;
  const double ct = std::cos(rad);
  const double st = std::sin(rad);

  Matrix<_Tp, 3, 3> R;
  R << ct, 0.0,  st,
      0.0, 1.0, 0.0,
      -st, 0.0,  ct;
  return R;
}



/// Returns the 3x3 rotation matrix, rotating around the z-axis.
template <typename _Tp> inline
Matrix<_Tp, 3, 3> RotationZ(double angle, bool angle_in_deg) {
  const double rad = angle_in_deg ? deg2rad(angle) : angle;
  const double ct = std::cos(rad);
  const double st = std::sin(rad);

  Matrix<_Tp, 3, 3> R;
  R << ct, -st, 0.0,
       st,  ct, 0.0,
      0.0, 0.0, 1.0;
  return R;
}



/// Returns the 3x3 rotation matrix in ZYX order.
template <typename _Tp> inline
Matrix<_Tp, 3, 3> RotationMatrix(
    double angle_x, double angle_y, double angle_z, bool angles_in_deg) {
  auto Rx = RotationX<_Tp>(angle_x, angles_in_deg);
  auto Ry = RotationY<_Tp>(angle_y, angles_in_deg);
  auto Rz = RotationZ<_Tp>(angle_z, angles_in_deg);
  return Rx * (Ry * Rz);
}



} // namespace geometry
} // namespace werkzeugkiste

#endif // __WERKZEUGKISTE_GEOMETRY_PROJECTION_H__
