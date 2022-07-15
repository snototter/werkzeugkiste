#include <exception>
#include <initializer_list>
#include <cmath>
#include <vector>
#include <list>

#include <gtest/gtest.h>

#include <werkzeugkiste/geometry/projection.h>
#include <werkzeugkiste/geometry/camera.h>

namespace wkg = werkzeugkiste::geometry;

TEST(ProjectionTest, Transformations) {
  wkg::Vec2d v1{17, 42};
  wkg::Vec2d v2{-3, 0.5};
  wkg::Matrix<double, 4, 2> M;
  M << 1, 2, 3, 4,
       5, 6, 7, 8;

  // First transform to mat, then convert mat to tuple
  wkg::Matrix<double, 4, 2> mat_res = wkg::TransformToMat(M, v1, v2);
  wkg::Vec4d a, b;
  std::tie(a, b) = wkg::EigenMatToVecTuple<double, 4, 2>(mat_res);

  wkg::Vec4d exp1{
    (17 + 84), (3 * 17 + 4 * 42), (5 * 17 + 6 * 42), (7 * 17 + 8 * 42)};
  wkg::Vec4d exp2{
    (-3 + 1), (-3 * 3 + 2), (-5 * 3 + 3), (-7 * 3 + 4)};
  EXPECT_EQ(a, exp1);
  EXPECT_EQ(b, exp2);

  // Test the convenience util which directly outputs the tuple
  wkg::Vec4d c, d;
  std::tie(c, d) = wkg::TransformToVecs(M, v1, v2);
  EXPECT_EQ(a, exp1);
  EXPECT_EQ(b, exp2);

  // Test the transformation with only a single vector
  std::tie(a) = wkg::TransformToVecs(M, v2);
  EXPECT_EQ(a, exp2);
  std::tie(a) = wkg::TransformToVecs(M, v1);
  EXPECT_EQ(a, exp1);
}


TEST(ProjectionTest, Projections) {
  wkg::Vec2d v1{17, 42};
  wkg::Vec2d v2{-3, 0.5};
  wkg::Vec2d v3{1, -50};

  wkg::Matrix<double, 3, 3> P;
  P << 1, 2, 3,
       4, 5, 6,
       7, 8, 9;

  // Test util which adds the homogeneous coordinate on its own
  wkg::Vec2d p1, p2, p3;
  std::tie(p1, p2) = wkg::ProjectInhomogeneousToVecs(P, v1, v2);

  wkg::Vec2d exp1{0.22413793, 0.61206897};
  wkg::Vec2d exp2{-0.125, 0.4375};
  EXPECT_TRUE(std::abs(p1[0] - exp1[0]) < 1e-6);
  EXPECT_TRUE(std::abs(p1[1] - exp1[1]) < 1e-6);
  EXPECT_EQ(p2, exp2);

  // Test projection with only a single vector
  std::tie(p3) = wkg::ProjectInhomogeneousToVecs(P, v3);
  wkg::Vec2d exp3{0.25, 0.625};
  EXPECT_EQ(p3, exp3);


  // Test the same, but this time already provide homogeneous coordinates
  std::tie(p2, p1) = wkg::ProjectHomogeneousToVecs(P, v1.Homogeneous(), v2.Homogeneous());
  // Tuple assignment flipped on purpose
  EXPECT_TRUE(std::abs(p2[0] - exp1[0]) < 1e-6);
  EXPECT_TRUE(std::abs(p2[1] - exp1[1]) < 1e-6);
  EXPECT_EQ(p1, exp2);

  // Again with only a single vector
  std::tie(p1) = wkg::ProjectHomogeneousToVecs(P, v3.Homogeneous());
  EXPECT_EQ(p1, exp3);
}


TEST(ProjectionTest, PinholeCamera) {
//  FIXME test camera-related utils
//  wkg::Mat3x3d K, R;
//  wkg::Vec3d t{0.5, 0.3, 0.1};

//  K << 400, 0, 300,
//       0, 400, 300,
//      0, 0, 1;
//  R << 1, 0, 0,
//       0, 1, 0,
//       0, 0, 1;

//  wkg::Matrix<double, 3, 4> Rt;
//  Rt << R, wkg::VecToEigen(t);

//  wkg::Mat3x4d cam_prj = wkg::ProjectionMatrixFromKRt(K, R, t);

//  std::cout << "Projection matrix:\nK = " << K << ", R = " << R << ", t = " << t << " --> P = \n" << cam_prj << std::endl;

//  std::cout << "GP-2-image:\n" << wkg::GroundplaneToImageHomography(cam_prj)
//            << "\nImage-2-GP:\n" << wkg::ImageToGroundplaneHomography(cam_prj)
//            << "\n... must equal GP-2-img^(-1):\n" << wkg::GroundplaneToImageHomography(cam_prj).inverse() << std::endl;

//  std::cout << "Camera center (R, t): " << wkg::CameraCenterFromRt(R, t) << std::endl
//            << "Camera center (Rt):   " << wkg::CameraCenterFromRt(Rt) << std::endl;

//  std::cout << "Rotation matrix (float):\n" << wkg::RotationMatrix<float>(10, 20, 30, true) << std::endl
//            << "Rotation matrix (double):\n" << wkg::RotationMatrix<double>(10, 20, 30, true) << std::endl;
}

