#ifndef WERKZEUGKISTE_GEOMETRY_CAMERA_H
#define WERKZEUGKISTE_GEOMETRY_CAMERA_H

#include <werkzeugkiste/geometry/primitives.h>
#include <werkzeugkiste/geometry/projection.h>
#include <werkzeugkiste/geometry/vector.h>

#include <Eigen/Core>
#include <tuple>
#include <type_traits>

// NOLINTBEGIN(readability-identifier-naming)

namespace werkzeugkiste::geometry {

//-----------------------------------------------------------------------------
// Camera projection matrix

/// Returns the pinhole projection matrix `P = K * [R | t] = K * Rt`.
template <typename Tp>
inline Matrix<Tp, 3, 4> ProjectionMatrixFromKRt(const Matrix<Tp, 3, 3>& K,
    const Matrix<Tp, 3, 4>& Rt) {
  static_assert(
      std::is_floating_point_v<Tp>, "Template type must be float or double!");
  return K * Rt;
}

/// Returns the pinhole projection matrix `P = K * [R | t]`.
template <typename Tp>
inline Matrix<Tp, 3, 4> ProjectionMatrixFromKRt(const Matrix<Tp, 3, 3>& K,
    const Matrix<Tp, 3, 3>& R,
    const Matrix<Tp, 3, 1>& t) {
  static_assert(
      std::is_floating_point_v<Tp>, "Template type must be float or double!");
  Matrix<Tp, 3, 4> Rt;
  Rt << R, t;
  return ProjectionMatrixFromKRt(K, Rt);
}

/// Returns the projection matrix `P = K * [R | t]`.
template <typename Tp>
inline Matrix<Tp, 3, 4> ProjectionMatrixFromKRt(const Matrix<Tp, 3, 3>& K,
    const Matrix<Tp, 3, 3>& R,
    const Vec<Tp, 3>& t) {
  static_assert(
      std::is_floating_point_v<Tp>, "Template type must be float or double!");
  return ProjectionMatrixFromKRt(K, R, VecToEigenMat<3>(t));
}

//-----------------------------------------------------------------------------
// Optical center

/// Returns the optical center C = -R' * t.
template <typename Tp>
inline Vec<Tp, 3> CameraCenterFromRt(const Matrix<Tp, 3, 3>& R,
    const Matrix<Tp, 3, 1>& t) {
  static_assert(
      std::is_floating_point_v<Tp>, "Template type must be float or double!");
  return EigenColToVec<Tp, 3, 1>(-R.transpose() * t, 0);
}

/// Returns the optical center C = -R' * t.
template <typename Tp>
inline Vec<Tp, 3> CameraCenterFromRt(const Matrix<Tp, 3, 3>& R,
    const Vec<Tp, 3>& t) {
  static_assert(
      std::is_floating_point_v<Tp>, "Template type must be float or double!");
  return CameraCenterFromRt(R, VecToEigenMat<3>(t));
}

/// Returns the optical center C = -R' * t.
template <typename Tp>
inline Vec<Tp, 3> CameraCenterFromRt(const Matrix<Tp, 3, 4>& Rt) {
  static_assert(
      std::is_floating_point_v<Tp>, "Template type must be float or double!");
  return EigenColToVec<Tp, 3, 1>(
      -Rt.block(0, 0, 3, 3).transpose() * Rt.col(3), 0);
}

//-----------------------------------------------------------------------------
// Ground Plane

// FIXME test
/// Returns the ground plane-to-image plane homography from the camera's
/// projection matrix.
/// `H_gp2cam = [p_0, p_1, p_3]`, where `p_i` is the i-th column of P
template <typename Tp>
inline Matrix<Tp, 3, 3> GroundplaneToImageHomography(
    const Matrix<Tp, 3, 4>& P) {
  static_assert(
      std::is_floating_point_v<Tp>, "Template type must be float or double!");
  Matrix<Tp, 3, 3> H;
  H << P.col(0), P.col(1), P.col(3);
  return H;
}

// FIXME test
/// Returns the image plane-to-ground plane homography from the camera's
/// projection matrix.
template <typename Tp>
inline Matrix<Tp, 3, 3> ImageToGroundplaneHomography(
    const Matrix<Tp, 3, 4>& P) {
  static_assert(
      std::is_floating_point_v<Tp>, "Template type must be float or double!");
  Matrix<Tp, 3, 3> H_gp2img = GroundplaneToImageHomography(P);
  return H_gp2img.inverse();
}

//-----------------------------------------------------------------------------
// Image Plane

template <typename Tp>
inline Plane_<Tp> ImagePlaneInCameraCoordinateSystem() {
  // Pinhole camera looks along the positive z-axis and the image plane is
  // at z = 1 in the camera reference frame. Thus, it's Hessian form is
  // n = (0, 0, 1), d = -1:
  return Plane_<Tp>{{0, 0, 1}, -1};
}

/// Returns the image plane in the world reference frame, given the camera's
/// extrinsic parameters.
template <typename Tp>
inline Plane_<Tp> ImagePlaneInWorldCoordinateSystem(const Matrix<Tp, 3, 3>& R,
    const Vec<Tp, 3>& t) {
  static_assert(
      std::is_floating_point_v<Tp>, "Template type must be float or double!");

  // Rotate the image plane normal to express it in the world reference frame:
  const Plane_<Tp> img_plane_cam = ImagePlaneInCameraCoordinateSystem<Tp>();
  Vec<Tp, 3> normal_world;
  const Matrix<Tp, 3, 3> Rinv = R.transpose();
  std::tie(normal_world) = TransformToVecs(Rinv, img_plane_cam.Normal());

  // The world origin in camera coordinates is t = -R*C = [R|t] * (0, 0, 0, 1).
  const Tp offset = img_plane_cam.DistancePointToPlane(t);

  return Plane_<Tp>(normal_world, offset);
}

/// Returns true if the world point lies in front of the image plane.
template <typename Tp>
inline bool IsInFrontOfImagePlane(const Vec<Tp, 3>& pt_world,
    const Matrix<Tp, 3, 4>& Rt) {
  static_assert(
      std::is_floating_point_v<Tp>, "Template type must be float or double!");
  Vec<Tp, 3> pt_cam;
  std::tie(pt_cam) = TransformToVecs(Rt, pt_world);

  const Plane_<Tp> img_plane_cam = ImagePlaneInCameraCoordinateSystem<Tp>();
  return img_plane_cam.IsPointInFrontOfPlane(pt_cam);
}

/// Returns true if the world point lies in front of the image plane.
template <typename Tp>
inline bool IsInFrontOfImagePlane(const Vec<Tp, 3>& pt_world,
    const Matrix<Tp, 3, 3>& R,
    const Vec<Tp, 3>& t) {
  static_assert(
      std::is_floating_point_v<Tp>, "Template type must be float or double!");
  Matrix<Tp, 3, 4> Rt;
  Rt << R, t;
  return IsInFrontOfImagePlane(pt_world, Rt);
}

//-----------------------------------------------------------------------------
// Horizon

/// Returns a the projected line of horizon for the given pinhole camera
/// calibration. If a valid image size is given, the line will be clipped
/// to the visible region. Check `result.IsValid()`, as the horizon may lie
/// outside of the image.
template <typename Tp>
inline Line2d GetProjectionOfHorizon(const Matrix<Tp, 3, 3>& K,
    const Matrix<Tp, 3, 3>& R,
    const Vec<Tp, 3>& t,
    const Vec2i& image_size = {0, 0}) {
  static_assert(
      std::is_floating_point_v<Tp>, "Template type must be float or double!");
  // Get a vector pointing along the camera's optical axis, which is orthogonal
  // to the ground plane normal:
  const Vec<Tp, 3> img_plane_normal =
      ImagePlaneInWorldCoordinateSystem(R, t).Normal();
  const Vec2d horizon_dir =
      Vec<Tp, 2>(img_plane_normal[0], img_plane_normal[1]).UnitVector();
  if (IsEpsZero(horizon_dir[0]) && IsEpsZero(horizon_dir[1])) {
    // Camera points along the world's z-axis. Horizon is not visible.
    return Line2d{};
  }

  // Get two points in front of the camera, which project onto the horizon
  // line, i.e. all points at the same height as the camera.
  const Vec<Tp, 3> camera_center3d = CameraCenterFromRt(R, t);
  const Vec2d camera_center2d(camera_center3d[0], camera_center3d[1]);

  const Vec2d perpendicular_dir(horizon_dir[1], -horizon_dir[0]);
  const Vec2d pt1 = camera_center2d + 1000.0 * horizon_dir;
  const Vec2d pt2 = pt1 + 500.0 * perpendicular_dir;

  const Matrix<Tp, 3, 4> P = ProjectionMatrixFromKRt(K, R, t);
  Vec<Tp, 2> prj1, prj2;
  std::tie(prj1, prj2) = ProjectToVecs(P,
      Vec<Tp, 3>(pt1[0], pt1[1], camera_center3d[2]),
      Vec<Tp, 3>(pt2[0], pt2[1], camera_center3d[2]));

  Line2d horizon{prj1, prj2};
  if ((image_size.Width() > 0) && (image_size.Height() > 0)) {
    return horizon
        .ClipLineByRectangle({0, 0}, static_cast<Line2d::vec_type>(image_size))
        .LeftToRight();
  }

  return horizon.LeftToRight();
}

//-----------------------------------------------------------------------------
// Field-of-View
template <typename Tp>
inline bool IsPointInsideImage(const Vec<Tp, 2>& pt, const Vec2i& img_size) {
  return IsPointInsideRectangle<Tp>(pt,
      Vec<Tp, 2>{0.0, 0.0},
      Vec<Tp, 2>{static_cast<Tp>(img_size.Width()),
          static_cast<Tp>(img_size.Height())});
}

/// Returns true if the given world point would be visible if projected into
/// the camera image. If `projected` is a valid pointer, it will be set to
/// the projected image coordinates.
template <typename Tp>
inline bool ProjectsPointOntoImage(const Vec<Tp, 3>& pt_world,
    const Matrix<Tp, 3, 4>& P,
    const Vec2i& img_size,
    Vec<Tp, 2>* projected) {
  static_assert(
      std::is_floating_point_v<Tp>, "Template type must be float or double!");
  Vec<Tp, 2> pt_img;
  std::tie(pt_img) = ProjectToVecs(P, pt_world);
  if (projected) {
    *projected = pt_img;
  }
  return IsPointInsideImage(pt_img, img_size);
}

//////// TODOs
// ClipLineSegmentByPlane / Line/Plane, ...
// ProjectsOntoImage(Vec3d pt, P, image_size, Vec2d *projected)

}  // namespace werkzeugkiste::geometry

// NOLINTEND(readability-identifier-naming)

#endif  // WERKZEUGKISTE_GEOMETRY_PROJECTION_H
