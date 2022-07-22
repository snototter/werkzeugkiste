#ifndef __WERKZEUGKISTE_GEOMETRY_CAMERA_H__
#define __WERKZEUGKISTE_GEOMETRY_CAMERA_H__

#include <type_traits>

#include <Eigen/Core>

#include <werkzeugkiste/geometry/vector.h>
#include <werkzeugkiste/geometry/projection.h>
#include <werkzeugkiste/geometry/primitives.h>


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

template <typename _Tp> inline
Plane_<_Tp> ImagePlaneInCameraCoordinateSystem() {
  // Pinhole camera looks along the positive z-axis and the image plane is
  // at z = 1 in the camera reference frame. Thus, it's Hessian form is
  // n = (0, 0, 1), d = -1:
  return Plane_<_Tp>{{0, 0, 1}, -1};
}


/// Returns the image plane in the world reference frame, given the camera's
/// extrinsic parameters.
template <typename _Tp> inline
Plane_<_Tp> ImagePlaneInWorldCoordinateSystem(
    const Matrix<_Tp, 3, 3> &R, const Vec<_Tp, 3> &t) {
  static_assert(
      std::is_floating_point<_Tp>::value,
      "Template type must be float or double!");

  // Rotate the image plane normal to express it in the world reference frame:
  const Plane_<_Tp> img_plane_cam = ImagePlaneInCameraCoordinateSystem<_Tp>();
  Vec<_Tp, 3> normal_world;
  const Matrix<_Tp, 3, 3> Rinv = R.transpose();
  std::tie(normal_world) = TransformToVecs(Rinv, img_plane_cam.Normal());

  // The world origin in camera coordinates is t = -R*C = [R|t] * (0, 0, 0, 1).
  const _Tp offset = img_plane_cam.DistancePointToPlane(t);

  return Plane_<_Tp>(normal_world, offset);
}


/// Returns true if the world point lies in front of the image plane.
template <typename _Tp> inline
bool IsInFrontOfImagePlane(
    const Vec<_Tp, 3> &pt_world, const Matrix<_Tp, 3, 4> &Rt) {
  static_assert(
      std::is_floating_point<_Tp>::value,
      "Template type must be float or double!");
  Vec<_Tp, 3> pt_cam;
  std::tie(pt_cam) = TransformToVecs(Rt, pt_world);

  const Plane_<_Tp> img_plane_cam = ImagePlaneInCameraCoordinateSystem<_Tp>();
  return img_plane_cam.IsPointInFrontOfPlane(pt_cam);
}


/// Returns true if the world point lies in front of the image plane.
template <typename _Tp> inline
bool IsInFrontOfImagePlane(
    const Vec<_Tp, 3> &pt_world, const Matrix<_Tp, 3, 3> &R,
    const Vec<_Tp, 3> &t) {
  static_assert(
      std::is_floating_point<_Tp>::value,
      "Template type must be float or double!");
  Matrix<_Tp, 3, 4> Rt;
  Rt << R, t;
  return IsInFrontOfImagePlane(pt_world, Rt);
}


//-----------------------------------------------------------------------------
// Horizon
///** @brief Returns a the projected line of horizon for the given camera intrinsics/extrinsics. If a valid image size
//  * is given, the line will be clipped to the visible region (in this case, check result.empty() as the horizon may lie
//  * outside of the image.
//  */
//Line2d GetProjectionOfHorizon(const cv::Mat &K, const cv::Mat &R, const cv::Mat &t, const cv::Size &image_size=cv::Size());


//-----------------------------------------------------------------------------
// Field-of-View
template <typename _Tp> inline
bool IsPointInsideImage(const Vec<_Tp, 2> &pt, const Vec2i &img_size) {
  return IsPointInsideRectangle<_Tp>(
        pt, Vec<_Tp, 2>{0.0, 0.0}, Vec<_Tp, 2>{
          static_cast<_Tp>(img_size.x()), static_cast<_Tp>(img_size.y())});
}



/// Returns true if the given world point would be visible if projected into
/// the camera image. If `projected` is a valid pointer, it will be set to
/// the projected image coordinates.
template <typename _Tp> inline
bool ProjectsPointOntoImage(
    const Vec<_Tp, 3> &pt_world, const Matrix<_Tp, 3, 4> &P, const Vec2i &img_size,
    Vec<_Tp, 2> *projected) {
  Vec<_Tp, 2> pt_img;
  std::tie(pt_img) = ProjectToVecs(P, pt_world);
  if (projected) {
    *projected = pt_img;
  }
  return IsPointInsideImage(pt_img, img_size);
}



//////// TODOs
// ClipLineSegmentByPlane / Line/Plane, ...
// ProjectsOntoImage(Vec3d pt, P, image_size, Vec2d *projected)


} // namespace geometry
} // namespace werkzeugkiste

#endif // __WERKZEUGKISTE_GEOMETRY_PROJECTION_H__
