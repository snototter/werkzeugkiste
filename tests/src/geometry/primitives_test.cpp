#include <cmath>
#include <exception>
#include <initializer_list>
#include <list>
#include <vector>

// NOLINTBEGIN
//TODO select which warnings to silence

#include <werkzeugkiste/geometry/primitives.h>

#include "../test_utils.h"

namespace wkg = werkzeugkiste::geometry;


TEST(GeometricPrimitives, Circle)
{
  // Collinear points
  wkg::Circle c1({0, 0}, {0, 0}, {10, 20});
  EXPECT_FALSE(c1.IsValid());

  const double x = 3.0;
  const double y = 4.0;
  const double r = 5.0;
  wkg::Circle c2({x, y + r}, {x, y - r}, {x + r, y});
  EXPECT_TRUE(c2.IsValid());
  EXPECT_DOUBLE_EQ(c2.CenterX(), x);
  EXPECT_DOUBLE_EQ(c2.CenterY(), y);
  EXPECT_DOUBLE_EQ(c2.Radius(), r);

  // Circle-circle intersection:
  c1 = c2;
  EXPECT_EQ(c1.IntersectionCircleCircle(c2), -1);
  EXPECT_EQ(c2.IntersectionCircleCircle(c1), -1);

  // c2 contained in c1:
  c1 = wkg::Circle {{0, 0}, 20};
  EXPECT_TRUE(c1.IsValid());
  EXPECT_EQ(c1.IntersectionCircleCircle(c2), 0);
  EXPECT_EQ(c2.IntersectionCircleCircle(c1), 0);

  // Not touching:
  c1 = wkg::Circle {{-6, -10}, 2};
  EXPECT_EQ(c1.IntersectionCircleCircle(c2), 0);
  EXPECT_EQ(c2.IntersectionCircleCircle(c1), 0);

  // Touching:
  c1 = wkg::Circle {{0, 0}, 2};
  c2 = wkg::Circle {{3, 0}, 1};
  EXPECT_EQ(c1.IntersectionCircleCircle(c2), 1);
  EXPECT_EQ(c2.IntersectionCircleCircle(c1), 1);

  // Intersecting
  c2 = wkg::Circle {{0, 3}, 1.5};
  EXPECT_EQ(c1.IntersectionCircleCircle(c2), 2);
  EXPECT_EQ(c2.IntersectionCircleCircle(c1), 2);

  // Line intersection:
  wkg::Circle circle {{2.5, 0.5}, 1.0};
  wkg::Line2d l1 {{1.0, 1.5}, {2.0, 1.7}};
  EXPECT_EQ(circle.IntersectionCircleLine(l1), 0);
  EXPECT_EQ(l1.IntersectionLineCircle(circle), 0);

  // Tangent to the circle
  wkg::Line2d l2 {{1.0, 1.5}, {2.0, 1.5}};
  EXPECT_EQ(circle.IntersectionCircleLine(l2), 1);
  EXPECT_EQ(l2.IntersectionLineCircle(circle), 1);

  // Finally, two intersection points
  wkg::Line2d l3 {{1.0, 1.3}, {7.0, 0.5}};
  EXPECT_EQ(circle.IntersectionCircleLine(l3), 2);
  EXPECT_EQ(l3.IntersectionLineCircle(circle), 2);

  // TODO test line segment intersection
}

TEST(GeometricPrimitives, Line2d)
{
  wkg::Line2d line1 {{0.0, 0.0}, {3.0, 0.0}};
  wkg::Line2d line2 {{1.0, -0.6}, {-17.0, -0.6}};
  wkg::Line2d line3 {{-100.0, -0.6}, {-170.0, -0.6}};

  EXPECT_FALSE(line1.IsCollinear(line2));
  EXPECT_FALSE(line2.IsCollinear(line1));
  EXPECT_TRUE(line2.IsCollinear(line3));
  EXPECT_TRUE(line3.IsCollinear(line2));

  wkg::Vec2d expected {3.0, -0.6};
  EXPECT_EQ(line2.ClosestPointOnLine(line1.To()), expected);
  EXPECT_EQ(line2.ClosestPointOnSegment(line1.To()), line2.From());

  expected = {-17.0, -0.6};
  EXPECT_EQ(line2.ClosestPointOnSegment({-99, 0}), expected);
  EXPECT_EQ(line2.ClosestPointOnSegment({-17, 0}), expected);

  expected = {-16, -0.6};
  EXPECT_EQ(line2.ClosestPointOnSegment({-16, 0}), expected);

  expected = {0.0, -0.6};
  EXPECT_EQ(line2.ClosestPointOnSegment({0, 0}), expected);
  EXPECT_EQ(line2.ClosestPointOnSegment({0, 3}), expected);

  expected = {1.0, -0.6};
  EXPECT_EQ(line2.ClosestPointOnSegment({1, 3}), expected);
  EXPECT_EQ(line2.ClosestPointOnSegment({2, 3}), expected);

  EXPECT_DOUBLE_EQ(line1.AngleDeg({1, 0}), 0);
  EXPECT_DOUBLE_EQ(line2.AngleDeg({17, 0}), 180);
  EXPECT_DOUBLE_EQ(line2.AngleDeg({3, 0}), 180);
  EXPECT_DOUBLE_EQ(line3.AngleDeg({1, 0}), 180);
  EXPECT_DOUBLE_EQ(line3.AngleDeg({0, 1}), 90);
  EXPECT_DOUBLE_EQ(line3.AngleDeg({-1, 0}), 0);

  // Sort from left-to-right (and vertical lines from top-to-bottom):
  wkg::Line2d sorted = line1.LeftToRight();
  EXPECT_EQ(sorted.From(), line1.From());
  EXPECT_EQ(sorted.To(), line1.To());

  sorted = line2.LeftToRight();
  EXPECT_EQ(sorted.From(), line2.To());
  EXPECT_EQ(sorted.To(), line2.From());

  wkg::Line2d line4({70.0, -0.6}, {70.0, -0.6});
  EXPECT_FALSE(line4.IsValid());
  line4.SetTo({70.0, 300.2});
  EXPECT_TRUE(line4.IsValid());
  sorted = line4.LeftToRight();
  EXPECT_EQ(sorted.From(), line4.From());
  EXPECT_EQ(sorted.To(), line4.To());

  line4.SetTo({70.0, -300.2});
  EXPECT_TRUE(line4.IsValid());
  sorted = line4.LeftToRight();
  EXPECT_EQ(sorted.From(), line4.To());
  EXPECT_EQ(sorted.To(), line4.From());

  EXPECT_DOUBLE_EQ(line4.AngleDeg({0, -1}), 0);
  EXPECT_DOUBLE_EQ(sorted.AngleDeg({0, -1}), 180);
  EXPECT_DOUBLE_EQ(line4.AngleDeg({1, -1}), 45);
  EXPECT_DOUBLE_EQ(sorted.AngleDeg({1, -1}), 135);

  // TODO extend test suite
}

TEST(GeometricPrimitives, Line3d)
{
  wkg::Line3d line1({0.0, 0.0, 0.0}, {3.0, 0.0, 0.0});
  EXPECT_TRUE(line1.IsValid());

  wkg::Vec3d expected;
  EXPECT_EQ(line1.ClosestPointOnLine({0, 0, 1}), expected);
  EXPECT_EQ(line1.ClosestPointOnSegment({-1, 1, 0}), expected);

  EXPECT_EQ(line1.ClosestPointOnSegment({0, 1, 1}), expected);

  expected = {1, 0, 0};
  EXPECT_EQ(line1.ClosestPointOnSegment({1, 1, 1}), expected);

  expected = {2, 0, 0};
  EXPECT_EQ(line1.ClosestPointOnSegment({2, 1, 1}), expected);
  EXPECT_EQ(line1.ClosestPointOnLine({2, 1, 4}), expected);
  EXPECT_EQ(line1.ClosestPointOnSegment({2, 1, 4}), expected);

  expected = {3, 0, 0};
  EXPECT_EQ(line1.ClosestPointOnSegment({3, 1, 1}), expected);
  EXPECT_EQ(line1.ClosestPointOnSegment({4, 1, 1}), expected);
  EXPECT_EQ(line1.ClosestPointOnSegment({4, 1, 2}), expected);

  expected = {4, 0, 0};
  EXPECT_EQ(line1.ClosestPointOnLine({4, 1, 2}), expected);

  EXPECT_DOUBLE_EQ(line1.AngleDeg({1, 0, 0}), 0);
  EXPECT_DOUBLE_EQ(line1.AngleDeg({-1, 0, 0}), 180);
  EXPECT_DOUBLE_EQ(line1.AngleDeg({0, 1, 0}), 90);
  EXPECT_DOUBLE_EQ(line1.AngleDeg({0, -1, 0}), 90);

  EXPECT_DOUBLE_EQ(line1.AngleDeg({0, 0, 1}), 90);
  EXPECT_DOUBLE_EQ(line1.AngleDeg({0, 0, -1}), 90);

  EXPECT_DOUBLE_EQ(line1.AngleDeg({1, 1, 0}), 45);
  EXPECT_DOUBLE_EQ(line1.AngleDeg({1, -1, 0}), 45);
  EXPECT_DOUBLE_EQ(line1.AngleDeg({-1, 1, 0}), 135);
  EXPECT_DOUBLE_EQ(line1.AngleDeg({-1, -1, 0}), 135);

  EXPECT_DOUBLE_EQ(line1.AngleDeg({1, 0, 1}), 45);
  EXPECT_DOUBLE_EQ(line1.AngleDeg({1, 0, -1}), 45);
  EXPECT_DOUBLE_EQ(line1.AngleDeg({-1, 0, 1}), 135);
  EXPECT_DOUBLE_EQ(line1.AngleDeg({-1, 0, -1}), 135);

  // Line/Plane
  wkg::Plane plane_xy({0, 0, 1}, 0);
  EXPECT_TRUE(plane_xy.IsValid());
  EXPECT_DOUBLE_EQ(line1.AngleDeg(plane_xy), 0);

  wkg::Line3d line2({0, 0, 0}, {1, 0, 1});
  EXPECT_DOUBLE_EQ(line2.AngleDeg(plane_xy), 45);

  line2 = wkg::Line3d({17, 17, 17}, {17, 17, 18});
  EXPECT_DOUBLE_EQ(line2.AngleDeg(plane_xy), 90);

  line2 = wkg::Line3d({17, 17, 17}, {17, 17, 0});
  EXPECT_DOUBLE_EQ(line2.AngleDeg(plane_xy), -90);

  wkg::Plane plane_45({1, 1, 1}, {1, 0, 0}, {0, 0, 0});
  EXPECT_TRUE(plane_45.IsValid());
  EXPECT_DOUBLE_EQ(plane_xy.AngleDeg(plane_45), 135);

  EXPECT_DOUBLE_EQ(line1.AngleDeg(plane_45), 0);
  EXPECT_DOUBLE_EQ(line1.AngleDeg(plane_45), plane_45.AngleDeg(line1));

  line2 = wkg::Line3d({0, 0, 0}, {0, 1, 0});
  EXPECT_DOUBLE_EQ(line2.AngleDeg(plane_45), 45);
  EXPECT_DOUBLE_EQ(line2.AngleDeg(plane_45), plane_45.AngleDeg(line2));

  line2 = wkg::Line3d({0, 0, 0}, {1, 1, 0});
  EXPECT_DOUBLE_EQ(line2.AngleDeg(plane_45), 30);
  EXPECT_DOUBLE_EQ(line2.AngleDeg(plane_45), plane_45.AngleDeg(line2));

  line2 = wkg::Line3d({0, 0, 0}, {1, 0, 1});
  EXPECT_DOUBLE_EQ(line2.AngleDeg(plane_45), -30);
  EXPECT_DOUBLE_EQ(line2.AngleDeg(plane_45), plane_45.AngleDeg(line2));

  // TODO extend test suite
}

TEST(GeometricPrimitives, Plane)
{
  wkg::Plane plane_inv({-7, 3, 0}, {3, 3, 10}, {5, 3, 12});
  EXPECT_FALSE(plane_inv.IsValid());

  wkg::Plane plane({-1, -2, 2}, {-1, 2, 2}, {1, 0, 1});
  EXPECT_TRUE(plane.IsValid());

  wkg::Vec3d pt1 {0, 15, 2};
  // ~3.14 away from the plane's z-intercept
  wkg::Vec3d pt2 {1.40425069, 0.0, 4.30850138};
  // Point on the plane
  wkg::Vec3d pt3 {3, 0, 0};

  EXPECT_DOUBLE_EQ(plane.DistancePointToPlane(pt1), plane.Normal().X());
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
  EXPECT_TRUE(wkg::IsEpsZero(plane.DistancePointToPlane(pt)))
      << "Point should be on plane, but distance is "
      << plane.DistancePointToPlane(pt);
  EXPECT_TRUE(plane.IsPointInFrontOfPlane(pt));
  EXPECT_TRUE(plane.IsPointOnPlane(pt))
      << "Point should be on plane, but distance is "
      << plane.DistancePointToPlane(pt);

  pt += plane.Normal();
  EXPECT_DOUBLE_EQ(plane.DistancePointToPlane(pt), 1.0);
  EXPECT_TRUE(plane.IsPointInFrontOfPlane(pt));
  EXPECT_FALSE(plane.IsPointOnPlane(pt));

  pt -= 23.0 * plane.Normal();
  EXPECT_DOUBLE_EQ(plane.DistancePointToPlane(pt), -22.0);
  EXPECT_FALSE(plane.IsPointInFrontOfPlane(pt));

  //  wkg::Plane xy_plane({-1, 0, 0}, {0, 0, 0}, {1, 1, 0});
  //  EXPECT_TRUE(xy_plane.IsValid());

  // TODO extend test suite
}

// NOLINTEND
