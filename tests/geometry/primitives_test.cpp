#include <exception>
#include <initializer_list>
#include <cmath>
#include <vector>
#include <list>

#include <gtest/gtest.h>

#include <werkzeugkiste/geometry/primitives.h>

namespace wkg = werkzeugkiste::geometry;

TEST(GeometricPrimitives, Line2) {
  wkg::Line2d line1({0.0, 0.0}, {3.0, 0.0});
  wkg::Line2d line2({1.0, -0.6}, {-17.0, -0.6});
  wkg::Line2d line3({-100.0, -0.6}, {-170.0, -0.6});

  EXPECT_FALSE(line1.IsCollinear(line2));
  EXPECT_FALSE(line2.IsCollinear(line1));
  EXPECT_TRUE(line2.IsCollinear(line3));
  EXPECT_TRUE(line3.IsCollinear(line2));

  EXPECT_EQ(line2.ClosestPointOnLine(line1.To()), wkg::Vec2d(3.0, -0.6));
  EXPECT_EQ(line2.ClosestPointOnSegment(line1.To()), line2.From());

  //TODO extend test suite
}


TEST(GeometricPrimitives, Plane) {
  wkg::Plane plane_inv({-7, 3, 0}, {3, 3, 10}, {5, 3, 12});
  EXPECT_FALSE(plane_inv.IsValid());

  wkg::Plane plane({-1, -2, 2}, {-1, 2, 2}, {1, 0, 1});
  EXPECT_TRUE(plane.IsValid());

  wkg::Vec3d pt1{0, 15, 2};
  // ~3.14 away from the plane's z-intercept
  wkg::Vec3d pt2{1.40425069,  0.0,  4.30850138};
  // Point on the plane
  wkg::Vec3d pt3{3, 0, 0};

  EXPECT_DOUBLE_EQ(plane.DistancePointToPlane(pt1), plane.Normal().x());
  EXPECT_EQ(static_cast<int>(100 * plane.DistancePointToPlane(pt2)), -314);
  EXPECT_DOUBLE_EQ(plane.DistancePointToPlane(pt3), 0.0);

  EXPECT_FALSE(plane.IsPointInFrontOfPlane(pt1));
  EXPECT_FALSE(plane.IsPointOnPlane(pt1));
  pt1 += plane.Normal();
  EXPECT_TRUE(plane.IsPointInFrontOfPlane(pt1));
  EXPECT_FALSE(plane.IsPointOnPlane(pt1));

  EXPECT_FALSE(plane.IsPointInFrontOfPlane(pt2));
  EXPECT_FALSE(plane.IsPointOnPlane(pt2));
  pt2 += 3.15 * plane.Normal();
  EXPECT_TRUE(plane.IsPointInFrontOfPlane(pt2));
  EXPECT_FALSE(plane.IsPointOnPlane(pt2));

  EXPECT_TRUE(plane.IsPointInFrontOfPlane(pt3));
  EXPECT_TRUE(plane.IsPointOnPlane(pt3));

  wkg::Vec3d pt = -plane.Offset() * plane.Normal();
  EXPECT_TRUE(wkg::eps_zero(plane.DistancePointToPlane(pt)));
  EXPECT_TRUE(plane.IsPointInFrontOfPlane(pt));
  EXPECT_TRUE(plane.IsPointOnPlane(pt));

  pt += plane.Normal();
  EXPECT_DOUBLE_EQ(plane.DistancePointToPlane(pt), 1.0);
  EXPECT_TRUE(plane.IsPointInFrontOfPlane(pt));
  EXPECT_FALSE(plane.IsPointOnPlane(pt));

  pt -= 23 * plane.Normal();
  EXPECT_DOUBLE_EQ(plane.DistancePointToPlane(pt), -22.0);
  EXPECT_FALSE(plane.IsPointInFrontOfPlane(pt));

//  wkg::Plane xy_plane({-1, 0, 0}, {0, 0, 0}, {1, 1, 0});
//  EXPECT_TRUE(xy_plane.IsValid());




  //TODO extend test suite
}
