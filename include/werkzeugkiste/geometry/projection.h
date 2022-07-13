#ifndef __WERKZEUGKISTE_GEOMETRY_PROJECTION_H__
#define __WERKZEUGKISTE_GEOMETRY_PROJECTION_H__

#include <Eigen/Core>

#include <werkzeugkiste/geometry/vector.h>


namespace werkzeugkiste {
namespace geometry {

template <typename _Tp, int Rows, int Columns>
using MatrixBase = Eigen::Matrix<_Tp, Rows, Columns, Eigen::RowMajor>;

template <typename _Tp, int Rows>
using MatrixBaseDynWidth = Eigen::Matrix<_Tp, Rows, Eigen::Dynamic, Eigen::RowMajor>;

// row-major layout is not supported for vectors in eigen (and being a vector, the layout doesn't matter anyhow)
template <typename _Tp, int Dimension>
using VectorBase = Eigen::Matrix<_Tp, Dimension, 1>;

//using Matrix3x3 = MatrixBase<double, 3, 3>;
//using Matrix3x4 = MatrixBase<double, 3, 4>;

//template <int Rows, int Columns> inline
//Vec<double, Dimension> Transform(const MatrixBase<double, )

template <typename _Tp, int dim> inline
VectorBase<_Tp, dim> VecToEigen(const Vec<_Tp, dim> &vec) {
  VectorBase<_Tp, dim> m;
  for (int i = 0; i < dim; ++i) {
    m[i] = vec[i];
  }
  return m;
}


template <typename _Tp, int dim> inline
Vec<_Tp, dim> EigenToVec(const VectorBase<_Tp, dim> &vec) {
  Vec<_Tp, dim> v;
  for (int i = 0; i < dim; ++i) {
    v[i] = vec[i];
  }
  return v;
}


template <typename _Tp, int dim> inline
VectorBase<_Tp, dim + 1> VecToEigenHomogeneous(const Vec<_Tp, dim> &vec) {
  VectorBase<_Tp, dim + 1> m;
  for (int i = 0; i < dim; ++i) {
    m[i] = vec[i];
  }
  m[dim] = static_cast<_Tp>(1);
  return m;
}


template <typename _Tp, int dim> inline
Vec<_Tp, dim> Transform(const MatrixBase<_Tp, dim, dim> &M, const Vec<_Tp, dim> &vec) {
  VectorBase<_Tp, dim> res = M * VecToEigen(vec);
  return EigenToVec(res);
}


// TODO check https://www.fluentcpp.com/2021/04/30/how-to-implement-stdconjunction-and-stddisjunction-in-c11/
//template <typename _Tp, typename... _Ts>
//using SameType = std::enable_if_t<std::conjunction_v<std::is_same<_Tp, _Ts>...>>;
// TODO check https://www.fluentcpp.com/2021/06/07/how-to-define-a-variadic-number-of-arguments-of-the-same-type-part-5/
//template <typename _Tp, typename... _Ts, typename = SameType<_Tp, _Ts...>> inline

template <typename _Tp, typename... _Ts> inline
MatrixBaseDynWidth<typename _Tp::value_type, _Tp::ndim>
VecsToEigen(const _Tp &vec0, const _Ts &...others) {
  const int num_vecs = 1 + sizeof...(others);
  const _Tp vecs[num_vecs] {
    vec0, static_cast<const _Tp &>(others)...};

  MatrixBase<typename _Tp::value_type, _Tp::ndim, num_vecs> m;
  for (int row = 0; row < _Tp::ndim; ++row) {
    for (int col = 0; col < num_vecs; ++col) {
      m(row, col) = vecs[col][row];
    }
  }
  return m;
}

} // namespace geometry
} // namespace werkzeugkiste

#endif // __WERKZEUGKISTE_GEOMETRY_PROJECTION_H__
