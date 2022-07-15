#ifndef __WERKZEUGKISTE_GEOMETRY_CAMERA_H__
#define __WERKZEUGKISTE_GEOMETRY_CAMERA_H__


#include <Eigen/Core>

#include <werkzeugkiste/geometry/vector.h>
#include <werkzeugkiste/geometry/projection.h>


namespace werkzeugkiste {
namespace geometry {

//-----------------------------------------------------------------------------
// Camera projection matrix

/// Returns the pinhole projection matrix `P = K * [R | t] = K * Rt`.
template <typename _Tp> inline
Matrix<_Tp, 3, 4> ProjectionMatrixFromKRt(
    const Matrix<_Tp, 3, 3> &K, const Matrix<_Tp, 3, 4> &Rt) {
  return K * Rt;
}


/// Returns the pinhole projection matrix `P = K * [R | t]`.
template <typename _Tp> inline
Matrix<_Tp, 3, 4> ProjectionMatrixFromKRt(
    const Matrix<_Tp, 3, 3> &K,
    const Matrix<_Tp, 3, 3> &R,
    const Matrix<_Tp, 3, 1> &t) {
  Matrix<_Tp, 3, 4> Rt;
  Rt << R, t;
  return ProjectionMatrixFromKRt(K, Rt);
}


/// Returns the projection matrix `P = K * [R | t]`.
template <typename _Tp> inline
Matrix<_Tp, 3, 4> ProjectionMatrixFromKRt(
    const Matrix<_Tp, 3, 3> &K,
    const Matrix<_Tp, 3, 3> &R,
    const Vec<_Tp, 3> &t) {
  return ProjectionMatrixFromKRt(K, R, VecToEigen(t));
}


//-----------------------------------------------------------------------------
// Optical center

/// Returns the optical center C = -R' * t.
template <typename _Tp> inline
Vec<_Tp, 3> CameraCenterFromRt(
      const Matrix<_Tp, 3, 3> &R, const Matrix<_Tp, 3, 1> &t) {
  return EigenColToVec<_Tp, 3, 1>(-R.transpose() * t, 0);
}


/// Returns the optical center C = -R' * t.
template <typename _Tp> inline
Vec<_Tp, 3> CameraCenterFromRt(
      const Matrix<_Tp, 3, 3> &R, const Vec<_Tp, 3> &t) {
  return CameraCenterFromRt(R, VecToEigen(t));
}


/// Returns the optical center C = -R' * t.
template <typename _Tp> inline
Vec<_Tp, 3> CameraCenterFromRt(const Matrix<_Tp, 3, 4> &Rt) {
  return EigenColToVec<_Tp, 3, 1>(
        -Rt.block(0, 0, 3, 3).transpose() * Rt.col(3), 0);
}



//-----------------------------------------------------------------------------
// Ground Plane

  //FIXME test
/// Returns the ground plane-to-image plane homography from the camera's
/// projection matrix.
/// `H_gp2cam = [p_0, p_1, p_3]`, where `p_i` is the i-th column of P
template <typename _Tp> inline
Matrix<_Tp, 3, 3> GroundplaneToImageHomography(const Matrix<_Tp, 3, 4> &P) {
  Matrix<_Tp, 3, 3> H;
  H << P.col(0), P.col(1), P.col(3);
  return H;
}


  //FIXME test
/// Returns the image plane-to-ground plane homography from the camera's
/// projection matrix.
template <typename _Tp> inline
Matrix<_Tp, 3, 3> ImageToGroundplaneHomography(const Matrix<_Tp, 3, 4> &P) {
  Matrix<_Tp, 3, 3> H_gp2img = GroundplaneToImageHomography(P);
  return H_gp2img.inverse();
}


//-----------------------------------------------------------------------------
// Image Plane

///// Returns the Hessian normal form of the image plane given the camera's
///// extrinsic parameters.
//template <typename _Tp> inline
//Vec<_Tp, 4> ImagePlaneInWorldCoordinateSystem(
//    const Matrix<_Tp, 3, 3> &R, const Matrix<_Tp, 3, 3> &t) {
//  // Camera looks along the positive z-axis.
//  // Get the distance between the image plane and the world origin (in camera coordinates).
//  // World origin in camera coordinates is t = -RC = [R|t] (0,0,0,1).
//  // Image plane in camera coordinates is the xy plane, at z=1. Thus, its Hessian form is
//  // [0,0,1,-1] (distance=-1 because the camera center (origin) is *behind* the image plane!
//  const Vec<_Tp, 3> unit_vec_z {0, 0, 1};
//  const double distance = DistancePointPlane(vcp::convert::ToVec3d(t), MakePlane(unit_vec_z, -1.0));
//  // Rotate plane normal to express it in the world reference frame:
//  const cv::Mat Rinv = R.t();
//  Vec3d plane_normal = Apply3x3(Rinv, unit_vec_z); // In world reference
//  return MakePlane(plane_normal, distance);
//}


///** @brief Returns a the projected line of horizon for the given camera intrinsics/extrinsics. If a valid image size
//  * is given, the line will be clipped to the visible region (in this case, check result.empty() as the horizon may lie
//  * outside of the image.
//  */
//Line2d GetProjectionOfHorizon(const cv::Mat &K, const cv::Mat &R, const cv::Mat &t, const cv::Size &image_size=cv::Size());


//////// TODOs
// ImagePlaneInWorldCoordinateSystem
// ClipLineSegmentByPlane / Line/Plane, ...
// bool IsInFrontOfImagePlane(vec3 pt, Rt)
// ProjectsOntoImage(Vec3d pt, P, image_size, Vec2d *projected)


} // namespace geometry
} // namespace werkzeugkiste

#endif // __WERKZEUGKISTE_GEOMETRY_PROJECTION_H__
