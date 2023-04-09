#ifndef WERKZEUGKISTE_GEOMETRY_PROJECTION_H
#define WERKZEUGKISTE_GEOMETRY_PROJECTION_H

#include <werkzeugkiste/geometry/utils.h>
#include <werkzeugkiste/geometry/vector.h>

#include <Eigen/Core>
#include <Eigen/Eigen>
#include <Eigen/Geometry>
#include <cmath>
#include <tuple>
#include <type_traits>
#include <utility>

namespace werkzeugkiste::geometry {

//-----------------------------------------------------------------------------
// Matrix definitions

template <typename Tp, int Rows, int Columns>
using Matrix =
    Eigen::Matrix<Tp, Rows, Columns, (Columns > 1) ? Eigen::RowMajor : 0>;

template <typename Tp, int Rows>
using MatrixDynWidth = Eigen::Matrix<Tp, Rows, Eigen::Dynamic, Eigen::RowMajor>;

// using Mat2x2d = Matrix<double, 2, 2>;
using Mat3x3d = Matrix<double, 3, 3>;
using Mat3x4d = Matrix<double, 3, 4>;

//-----------------------------------------------------------------------------
// Conversion between werkzeugkiste and Eigen

/// @brief Converts a werkzeugkiste vector to an Eigen vector (single-column
/// matrix).
template <int Rows, typename V>
inline Matrix<typename V::value_type, Rows, 1> VecToEigenMat(const V& vec) {
  constexpr int v_dim_int = static_cast<int>(V::ndim);
  static_assert((Rows == v_dim_int) || (Rows == (v_dim_int + 1)),
      "Invalid number of rows for the matrix - must be either the vector "
      "dimension or 1 more (for automatically added homogeneous coordinate)!");

  Matrix<typename V::value_type, Rows, 1> mat;
  for (int i = 0; i < v_dim_int; ++i) {
    mat[i] = vec[i];
  }

  if (Rows == (v_dim_int + 1)) {
    // Add homogeneous coordinate:
    mat[v_dim_int] = static_cast<typename V::value_type>(1);
  }
  return mat;
}

/// @brief Returns a vector for the given matrix column.
template <typename Tp, int Rows, int Columns>
Vec<Tp, static_cast<std::size_t>(Rows)> EigenColToVec(
    const Matrix<Tp, Rows, Columns>& eig,
    int col) {
  Vec<Tp, static_cast<std::size_t>(Rows)> v;
  for (int i = 0; i < Rows; ++i) {
    v[i] = eig(i, col);
  }
  return v;
}

/// @brief Returns a matrix where each column holds one vector.
///
/// Note that the matrix size is fixed at compile time, thus it will be
/// created on the stack. This function should only be used for a maximum
/// of up to 32 vectors. Refer to the Eigen3 docs for details:
/// https://eigen.tuxfamily.org/dox/group__TutorialMatrixClass.html
template <int Dim, typename V, typename... Vs>
inline Matrix<typename V::value_type, Dim, static_cast<int>(1 + sizeof...(Vs))>
VecsToEigenMat(const V& vec0, const Vs&... others) {
  constexpr int v_dim_int = static_cast<int>(V::ndim);
  static_assert((Dim == v_dim_int) || (Dim == (v_dim_int + 1)),
      "Invalid number of rows for the matrix - must be either the vector "
      "dimension or 1 more (for homogeneous)!");

  constexpr int num_vecs = 1 + sizeof...(others);
  constexpr int max_columns = 32;
  static_assert(num_vecs <= max_columns,
      "Fixed size matrices should not be used for operations "
      "with more than (roughly) 32 vectors.");

  const std::array<V, static_cast<std::size_t>(num_vecs)> vecs{
      vec0, static_cast<const V&>(others)...};
  // TODO clean up
  //  const V vecs[num_vecs] {
  //    vec0, static_cast<const V &>(others)...};

  Matrix<typename V::value_type, Dim, num_vecs> m;
  for (int row = 0; row < v_dim_int; ++row) {
    for (int col = 0; col < num_vecs; ++col) {
      m(row, col) = vecs[static_cast<std::size_t>(col)][row];
    }
  }

  if (Dim == (v_dim_int + 1)) {
    // Add homogeneous coordinate:
    for (int col = 0; col < num_vecs; ++col) {
      m(v_dim_int, col) = static_cast<typename V::value_type>(1);
    }
  }
  return m;
}

///// Base case for making a tuple out of a variadic number of vectors.
// template <typename _Tp> inline
// std::tuple<_Tp> VecsToTuple(_Tp &&vec) {
//   return std::tuple<_Tp>(std::move(vec));
// }

///// Returns a tuple of vectors.
// template <typename _V0, typename _V1, typename... _Vs> inline
// std::tuple<_V0, _V1, _Vs...>
// VecsToTuple(
//     _V0 &&v0, _V1 &&v1, _Vs&&... others) {
//   return std::tuple_cat(
//         std::tuple<_V0>(std::move(v0)),
//         VecsToTuple<_V1, _Vs...>(std::move(v1), std::move(others)...));
// }

template <typename Array, std::size_t... Idx>
inline auto ArrayToTuple(const Array& arr,
    std::index_sequence<Idx...> /* indices */) {
  return std::make_tuple(arr[Idx]...);
}

/// @brief Returns a tuple of vectors (one vector per matrix column).
template <typename Tp, int Rows, int Columns>
inline auto EigenMatToVecTuple(const Matrix<Tp, Rows, Columns>& vec_mat) {
  static_assert((Rows > 0) && (Columns > 0),
      "Template parameters Rows and Columns must be > 0!");
  using Vector = Vec<Tp, static_cast<std::size_t>(Rows)>;
  constexpr std::size_t array_size = static_cast<std::size_t>(Columns);
  std::array<Vector, array_size> arr;
  for (std::size_t c = 0; c < array_size; ++c) {
    arr[c] = EigenColToVec<Tp, Rows, Columns>(vec_mat, static_cast<int>(c));
  }
  return ArrayToTuple(arr, std::make_index_sequence<array_size>{});
}

//-----------------------------------------------------------------------------
// Transformation/projection utitilites

/// @brief Computes `mat * [vec0, vec1, ...]` and returns the result as a tuple
/// of vectors.
///
/// The vector dimensionality `D` must be either equal to or 1 less than the
/// number of projection matrix columns `C`. If `D == C - 1`, a homogeneous
/// coordinate will be implicitly added to each vector, *i.e.*
/// `[vec0.X(), vec0.Y(), ..., 1]`.
///
/// Example:
/// @code
/// wkg::Vec2d v1{17, 42}, v2{9, -3}, v3{0, 0.01};
/// wkg::Matrix<double, 4, 2> M;
/// M << 1, 2, 3, 4, 5, 6, 7, 8;
/// wkg::Vec4d a, b, c;
/// std::tie(out1, out2, out3) = TransformToVecs(T_matrix, in1, in2, in3);
/// // Or with padded homogeneous coordinate:
/// wkg::Matrix<double, 4, 3> M;
/// M << 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12;
/// wkg::Vec4d a, b, c;
/// std::tie(out1, out2, out3) = TransformToVecs(T_matrix, in1, in2, in3);
/// @endcode
template <typename V, typename... Vs, int Rows, int Columns>
inline std::tuple<Vec<typename V::value_type, static_cast<std::size_t>(Rows)>,
    Vec<typename Vs::value_type, static_cast<std::size_t>(Rows)>...>
TransformToVecs(const Matrix<typename V::value_type, Rows, Columns>& mat,
    const V& vec0,
    const Vs&... others) {
  static_assert((Columns == V::ndim) || (Columns == (V::ndim + 1)),
      "Invalid dimensions: vector dimensionality must be equal or 1 less than"
      "the number of matrix columns!");

  constexpr int num_vecs = 1 + sizeof...(others);
  const Matrix<typename V::value_type, Columns, num_vecs> vec_mat =
      VecsToEigenMat<Columns>(vec0, others...);
  const auto transformed = mat * vec_mat;
  return EigenMatToVecTuple<typename V::value_type, Rows, num_vecs>(
      transformed);
}

/// @brief Convenience utility to avoid using std::tie() for a single vector.
/// See `TransformToVecs`.
template <typename V, typename... Vs, int Rows, int Columns>
inline Vec<typename V::value_type, static_cast<typename V::index_type>(Rows)>
TransformToVec(const Matrix<typename V::value_type, Rows, Columns>& mat,
    const V& vec) {
  constexpr int v_dim_int = static_cast<int>(V::ndim);
  static_assert((Columns == v_dim_int) || (Columns == (v_dim_int + 1)),
      "Invalid dimensions: vector dimensionality must be equal or 1 less than"
      "the number of matrix columns!");
  const Matrix<typename V::value_type, Columns, 1> vec_mat =
      VecToEigenMat<Columns>(vec);
  const auto transformed = mat * vec_mat;
  return EigenColToVec<typename V::value_type, Rows, 1>(transformed, 0);
}

/// @brief Returns the (normalized) projection result.
///
/// Computes `mat * [vec0, vec1, ...]`, divides the result by the
/// homogeneous coordinate (__i.e.__ last row) and strips the homogeneous
/// coordinate.
///
/// The vector dimensionality must be either equal to or 1 less than the number
/// of projection matrix columns. If not equal, a homogeneous coordinate will
/// be implicitly added to each vector, *i.e.* [vec0.x(), vec0.y(), ..., 1].
///
/// Example:
/// @code
/// wkg::Vec3d v1{17, 42, 1}, v2{9, -3, 1};
/// wkg::Matrix<double, 3, 3> H;
/// H << 1, 2, 3,
///      4, 5, 6,
///      7, 8, 9;
/// wkg::Vec2d out1, out2;
/// std::tie(out1, out2) = ProjectToVecs(H, v1, v2);
/// @endcode
/// @return A tuple of vectors.
template <typename V, typename... Vs, int Rows, int Columns>
inline std::tuple<
    Vec<typename V::value_type, static_cast<typename V::index_type>(Rows - 1)>,
    Vec<typename Vs::value_type,
        static_cast<typename Vs::index_type>(Rows - 1)>...>
ProjectToVecs(const Matrix<typename V::value_type, Rows, Columns>& mat,
    const V& vec0,
    const Vs&... others) {
  static_assert((Rows > 0) && (Columns > 0),
      "Template parameters Rows and Columns must be > 0!");

  constexpr int v_dim_int = static_cast<int>(V::ndim);
  static_assert((Columns == v_dim_int) || (Columns == (v_dim_int + 1)),
      "Invalid dimensions: vector dimensionality must be equal or 1 less than"
      "the number of matrix columns!");

  constexpr int num_vecs = 1 + sizeof...(others);

  const Matrix<typename V::value_type, Columns, num_vecs> vec_mat =
      VecsToEigenMat<Columns>(vec0, others...);
  const auto transformed = mat * vec_mat;
  const auto projected = transformed.colwise().hnormalized();
  return EigenMatToVecTuple<typename V::value_type, Rows - 1, num_vecs>(
      projected);
}

/// @brief Convenience utility to avoid using std::tie() for a single vector.
/// See `ProjectToVecs`.
template <typename V, typename... Vs, int Rows, int Columns>
inline Vec<typename V::value_type, static_cast<std::size_t>(Rows - 1)>
ProjectToVec(const Matrix<typename V::value_type, Rows, Columns>& mat,
    const V& vec) {
  static_assert((Rows > 0) && (Columns > 0),
      "Template parameters Rows and Columns must be > 0!");

  constexpr int v_dim_int = static_cast<int>(V::ndim);
  static_assert((Columns == v_dim_int) || (Columns == (v_dim_int + 1)),
      "Invalid dimensions: vector dimensionality must be equal or 1 less than"
      "the number of matrix columns!");

  const Matrix<typename V::value_type, Columns, 1> vec_mat =
      VecToEigenMat<Columns>(vec);
  const auto transformed = mat * vec_mat;
  const auto projected = transformed.colwise().hnormalized();
  return EigenColToVec<typename V::value_type, Rows - 1, 1>(projected, 0);
}

//-----------------------------------------------------------------------------
// Rotation utilities

/// @brief Returns the 3x3 rotation matrix, rotating around the x-axis.
template <typename Tp>
inline constexpr Matrix<Tp, 3, 3> RotationX(Tp angle, bool angle_in_deg) {
  static_assert(std::is_floating_point<Tp>::value,
      "Template type must be floating point!");

  const Tp rad = angle_in_deg ? Deg2Rad(angle) : angle;
  const Tp ct = std::cos(rad);
  const Tp st = std::sin(rad);

  Matrix<Tp, 3, 3> rot;
  rot << 1.0, 0.0, 0.0, 0.0, ct, -st, 0.0, st, ct;
  return rot;
}

/// @brief Returns the 3x3 rotation matrix, rotating around the y-axis.
template <typename Tp>
inline constexpr Matrix<Tp, 3, 3> RotationY(Tp angle, bool angle_in_deg) {
  static_assert(std::is_floating_point<Tp>::value,
      "Template type must be floating point!");

  const Tp rad = angle_in_deg ? Deg2Rad(angle) : angle;
  const Tp ct = std::cos(rad);
  const Tp st = std::sin(rad);

  Matrix<Tp, 3, 3> rot;
  rot << ct, 0.0, st, 0.0, 1.0, 0.0, -st, 0.0, ct;
  return rot;
}

/// @brief Returns the 3x3 rotation matrix, rotating around the z-axis.
template <typename Tp>
inline constexpr Matrix<Tp, 3, 3> RotationZ(Tp angle, bool angle_in_deg) {
  static_assert(std::is_floating_point<Tp>::value,
      "Template type must be floating point!");

  const Tp rad = angle_in_deg ? Deg2Rad(angle) : angle;
  const Tp ct = std::cos(rad);
  const Tp st = std::sin(rad);

  Matrix<Tp, 3, 3> rot;
  rot << ct, -st, 0.0, st, ct, 0.0, 0.0, 0.0, 1.0;
  return rot;
}

/// @brief Returns the 3x3 rotation matrix in ZYX order.
template <typename Tp>
inline constexpr Matrix<Tp, 3, 3> RotationMatrix(Tp angle_x,
    Tp angle_y,
    Tp angle_z,
    bool angles_in_deg) {
  static_assert(std::is_floating_point<Tp>::value,
      "Template type must be floating point!");

  auto rx = RotationX<Tp>(angle_x, angles_in_deg);
  auto ry = RotationY<Tp>(angle_y, angles_in_deg);
  auto rz = RotationZ<Tp>(angle_z, angles_in_deg);
  return rx * (ry * rz);
}

// TODO(vcp) RotationMatrixToEulerAngles

}  // namespace werkzeugkiste::geometry

#endif  // WERKZEUGKISTE_GEOMETRY_PROJECTION_H
