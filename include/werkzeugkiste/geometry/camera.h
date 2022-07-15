#ifndef __WERKZEUGKISTE_GEOMETRY_CAMERA_H__
#define __WERKZEUGKISTE_GEOMETRY_CAMERA_H__


#include <Eigen/Core>

#include <werkzeugkiste/geometry/vector.h>
#include <werkzeugkiste/geometry/projection.h>


namespace werkzeugkiste {
namespace geometry {

/// Returns the projection matrix `P = K * [R | t] = K * Rt`.
template <typename _Tp> inline
Matrix<_Tp, 3, 4> ProjectionMatrixFromKRt(
    const Matrix<_Tp, 3, 3> &K, const Matrix<_Tp, 3, 4> &Rt) {
  return K * Rt;
}


/// Returns the projection matrix `P = K * [R | t]`.
template <typename _Tp> inline
Matrix<_Tp, 3, 4> ProjectionMatrixFromKRt(
    const Matrix<_Tp, 3, 3> &K, const Matrix<_Tp, 3, 3> &R, const Matrix<_Tp, 3, 1> &t) {
  Matrix<_Tp, 3, 4> Rt;
  Rt << R, t;
  return ProjectionMatrixFromKRt(K, Rt);
}


/// Returns the projection matrix `P = K * [R | t]`.
template <typename _Tp> inline
Matrix<_Tp, 3, 4> ProjectionMatrixFromKRt(
    const Matrix<_Tp, 3, 3> &K, const Matrix<_Tp, 3, 3> &R, const Vec<_Tp, 3> &t) {
  return ProjectionMatrixFromKRt(K, R, VecToEigen(t));
}


/// Returns the optical center C = -R' * t.
template <typename _Tp> inline
Vec<_Tp, 3> CameraCenterFromRt(const Matrix<_Tp, 3, 3> &R, const Matrix<_Tp, 3, 1> &t) {
  return EigenColToVec<_Tp, 3, 1>(-R.transpose() * t, 0);
}


/// Returns the optical center C = -R' * t.
template <typename _Tp> inline
Vec<_Tp, 3> CameraCenterFromRt(const Matrix<_Tp, 3, 3> &R, const Vec<_Tp, 3> &t) {
  return CameraCenterFromRt(R, VecToEigen(t));
}


//////// TODOs
// CameraCenterFromRt(Rt)
// ImagePlaneInWorldCoordinateSystem
// ClipLineSegmentByPlane / Line/Plane, ...
// bool IsInFrontOfImagePlane(vec3 pt, Rt)
// ProjectsOntoImage(Vec3d pt, P, image_size, Vec2d *projected)


} // namespace geometry
} // namespace werkzeugkiste

#endif // __WERKZEUGKISTE_GEOMETRY_PROJECTION_H__
