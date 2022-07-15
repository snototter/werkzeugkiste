#ifndef __WERKZEUGKISTE_GEOMETRY_PROJECTION_H__
#define __WERKZEUGKISTE_GEOMETRY_PROJECTION_H__

#include <tuple>
#include <utility>
#include <type_traits>

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/Eigen>

#include <werkzeugkiste/geometry/vector.h>


namespace werkzeugkiste {
namespace geometry {

template <typename _Tp, int Rows, int Columns>
using Matrix = Eigen::Matrix<_Tp, Rows, Columns, (Columns > 1) ? Eigen::RowMajor : 0>;

template <typename _Tp, int Rows>
using MatrixDynWidth = Eigen::Matrix<_Tp, Rows, Eigen::Dynamic, Eigen::RowMajor>;

// row-major layout is not supported for vectors in eigen (and being a vector, the layout doesn't matter anyhow)
template <typename _Tp, int Dimension>
using VectorBase = Eigen::Matrix<_Tp, Dimension, 1>;

using Mat2x2d = Matrix<double, 2, 2>;
using Mat3x3d = Matrix<double, 3, 3>;
using Mat3x4d = Matrix<double, 3, 4>;


/// Converts a werkzeugkiste vector to an Eigen vector (single-column matrix).
template <typename _V> inline
VectorBase<typename _V::value_type, _V::ndim>
VecToEigenCol(const _V &vec) {
  VectorBase<typename _V::value_type, _V::ndim> mat;
  for (int i = 0; i < _V::ndim; ++i) {
    mat[i] = vec[i];
  }
  return mat;
}


/// Converts a werkzeugkiste vector to an Eigen vector (i.e. single-column
/// matrix) and adds a homogeneous coordinate (set to 1).
template <typename _Tp, int dim> inline
VectorBase<_Tp, dim + 1> VecToEigenColHomogeneous(const Vec<_Tp, dim> &vec) {
  VectorBase<_Tp, dim + 1> m;
  for (int i = 0; i < dim; ++i) {
    m[i] = vec[i];
  }
  m[dim] = static_cast<_Tp>(1);
  return m;
}


/// Returns a vector for the given matrix column.
template <typename _Tp, int Columns>
_Tp EigenColToVec(
    const Matrix<typename _Tp::value_type, _Tp::ndim, Columns> &eig, int col) {
//    const VectorBase<typename _Tp::value_type, _Tp::ndim> &eig) {
  _Tp v;
  for (int i = 0; i < _Tp::ndim; ++i) {
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


/// Base case for making a tuple out of a variadic number of vectors.
template <typename _Tp> inline
std::tuple<_Tp> VecsToTuple(_Tp &&vec) {
  return std::tuple<_Tp>(std::move(vec));
}


/// Returns a tuple of vectors.
template <typename _V0, typename _V1, typename... _Vs> inline
std::tuple<_V0, _V1, _Vs...>
VecsToTuple(
    _V0 &&v0, _V1 &&v1, _Vs&&... others) {
  return std::tuple_cat(
        std::tuple<_V0>(std::move(v0)),
        VecsToTuple<_V1, _Vs...>(std::move(v1), std::move(others)...));
}


template <typename Array, std::size_t... Idx>
auto ArrayToTuple(const Array &arr, std::index_sequence<Idx...>) {
  return std::make_tuple(arr[Idx]...);
}


template <typename _Tp, int Rows, int Columns>
auto EigenMatToVecTuple(
    const Matrix<_Tp, Rows, Columns> &vec_mat) {
  std::array<Vec<_Tp, Rows>, Columns> arr;
  for (int c = 0; c < Columns; ++c) {
    arr[c] = EigenColToVec<Vec<_Tp, Rows>, Columns>(vec_mat, c);//vec_mat.col(c));
  }
  return ArrayToTuple(arr, std::make_index_sequence<Columns>{});
}


///// Applies the `Rows x Dim` transformation matrix on the given Dim-dimensional
///// vector, *i.e.* returns `mat * vec`, which is a Rows-dimensional vector.
//template <typename _V, int Rows> inline
//Vec<typename _V::value_type, Rows>
//Transform(
//    const Matrix<typename _V::value_type, Rows, _V::ndim> &mat,
//    const _V &vec) {
//  const VectorBase<
//      typename _V::value_type, _V::ndim> res = mat * VecToEigenCol(vec);
//  return EigenColToVec(res);
//}


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




//template <typename _V, typename... _Vs, int Rows> inline
////Matrix<typename _V::value_type, Rows, 2 + sizeof...(_Vs)>
//////Matrix<typename _V::value_type, Rows, Eigen::Dynamic>
//auto ProjectToMat(
//      const Matrix<typename _V::value_type, Rows, _V::ndim> &mat,
//      const _V &vec0, const _V &vec1, const _Vs &... others) {

//  auto transformed = TransformToMat(mat, vec0, vec1, others...);
//  return transformed.colwise().hnormalized();
//}


//template <typename _V, typename... _Vs, int Rows, int Columns> inline
////Matrix<typename _V::value_type, Rows, 2 + sizeof...(_Vs)>
//////Matrix<typename _V::value_type, Rows, Eigen::Dynamic>
//auto ProjectInhomogeneousToMat(
//    const Matrix<typename _V::value_type, Rows, Columns> &mat,
//    const _V &vec0, const _V &vec1, const _Vs &... others) {

//  static_assert(
//    Columns == _V::ndim + 1,
//    "Number of columns in projection matrix must be _V::ndim + 1!");

//  auto transformed = mat * VecsToEigenMat<Columns, _V, _Vs...>(vec0, vec1, others...);
////  auto transformed = VecsToEigenMat<Columns>(vec0, vec1, others...);
//  return transformed.colwise().hnormalized();
////  return transformed;
//}



template <typename _V, typename... _Vs, int Rows> inline
Matrix<typename _V::value_type, Rows, 1 + sizeof...(_Vs)>
////Matrix<typename _V::value_type, Rows, Eigen::Dynamic>
//auto
ProjectInhomogeneousToMat(
      const Matrix<typename _V::value_type, Rows, _V::ndim + 1> &mat,
      const _V &vec0, const _Vs &... others) {

  constexpr int num_vecs = 1 + sizeof...(others);

  const Matrix<typename _V::value_type, Rows, num_vecs> transformed =
      mat * VecsToEigenMat<_V::ndim + 1>(vec0, others...);

  return transformed.colwise().hnormalized();
}


template <typename _V, typename... _Vs, int Rows> inline
std::tuple<
  Vec<typename _V::value_type, Rows - 1>,
  Vec<typename _Vs::value_type, Rows - 1>...>
ProjectInhomogeneousToVecs(
      const Matrix<typename _V::value_type, Rows, _V::ndim + 1> &mat,
      const _V &vec0, const _Vs &... others) {

  constexpr int num_vecs = 1 + sizeof...(others);

//  const auto hnormed = ProjectInhomogeneousToMat(mat, vec0, others...);

////  const Matrix<typename _V::value_type, Rows, num_vecs> transformed =
////      TransformToMat(mat, vec0, vec1, others...);
  const Matrix<typename _V::value_type, Rows, num_vecs> transformed =
      mat * VecsToEigenMat<_V::ndim + 1>(vec0, others...);

  const auto hnormed = transformed.colwise().hnormalized();
  return EigenMatToVecTuple<typename _V::value_type, Rows - 1, num_vecs>(hnormed);
}

} // namespace geometry
} // namespace werkzeugkiste

#endif // __WERKZEUGKISTE_GEOMETRY_PROJECTION_H__
