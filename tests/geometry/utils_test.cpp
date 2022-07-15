#include <exception>
#include <initializer_list>
#include <cmath>
#include <vector>
#include <list>

#include <gtest/gtest.h>

#include <werkzeugkiste/geometry/utils.h>

namespace wkg = werkzeugkiste::geometry;

TEST(GeometryUtilsTest, AngleConversion) {
  EXPECT_DOUBLE_EQ(wkg::deg2rad(0), 0.0);
  EXPECT_DOUBLE_EQ(wkg::deg2rad(45), M_PI_4);
  EXPECT_DOUBLE_EQ(wkg::deg2rad(90), M_PI_2);
  EXPECT_DOUBLE_EQ(wkg::deg2rad(180), M_PI);
  EXPECT_DOUBLE_EQ(wkg::deg2rad(360), 2 * M_PI);
  EXPECT_DOUBLE_EQ(wkg::deg2rad(720), 4 * M_PI);

  EXPECT_DOUBLE_EQ(wkg::rad2deg(wkg::deg2rad(0)), 0.0);
  EXPECT_DOUBLE_EQ(wkg::rad2deg(wkg::deg2rad(45)), 45);
  EXPECT_DOUBLE_EQ(wkg::rad2deg(wkg::deg2rad(90)), 90);
  EXPECT_DOUBLE_EQ(wkg::rad2deg(wkg::deg2rad(135)), 135);
  EXPECT_DOUBLE_EQ(wkg::rad2deg(wkg::deg2rad(180)), 180);
  EXPECT_DOUBLE_EQ(wkg::rad2deg(wkg::deg2rad(270)), 270);
  EXPECT_DOUBLE_EQ(wkg::rad2deg(wkg::deg2rad(360)), 360);
  EXPECT_DOUBLE_EQ(wkg::rad2deg(wkg::deg2rad(480)), 480);
}


TEST(GeometryUtilsTest, FloatingPointEquality) {
  EXPECT_TRUE(wkg::eps_zero(0.0));
  EXPECT_TRUE(wkg::eps_zero(1e-64));

  // Never test for eqs_equal with 0!
  EXPECT_TRUE(wkg::eps_zero(1e-32));
  EXPECT_FALSE(wkg::eps_equal(0.0, 1e-32));


  EXPECT_TRUE(wkg::eps_equal(3, 3));
  EXPECT_TRUE(wkg::eps_equal(5.0f, 5.0f));
  EXPECT_TRUE(wkg::eps_equal(3.000000001f, 3.000000002f));
  EXPECT_FALSE(wkg::eps_equal(5.0f, 5.0001f));
}
