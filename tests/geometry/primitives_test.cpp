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
