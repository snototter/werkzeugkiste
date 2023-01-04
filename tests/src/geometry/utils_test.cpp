#include <exception>
#include <initializer_list>
#include <cmath>
#include <vector>
#include <list>

#include "../test_utils.h"

#include <werkzeugkiste/geometry/utils.h>

namespace wkg = werkzeugkiste::geometry;


// NOLINTBEGIN

TEST(GeometryUtilsTest, AngleConversion) {
  EXPECT_DOUBLE_EQ(wkg::Deg2Rad(0), 0.0);
  EXPECT_DOUBLE_EQ(wkg::Deg2Rad(45), M_PI_4);
  EXPECT_DOUBLE_EQ(wkg::Deg2Rad(90), M_PI_2);
  EXPECT_DOUBLE_EQ(wkg::Deg2Rad(180), M_PI);
  EXPECT_DOUBLE_EQ(wkg::Deg2Rad(360), 2 * M_PI);
  EXPECT_DOUBLE_EQ(wkg::Deg2Rad(720), 4 * M_PI);

  EXPECT_DOUBLE_EQ(wkg::Rad2Deg(wkg::Deg2Rad(0)), 0.0);
  EXPECT_DOUBLE_EQ(wkg::Rad2Deg(wkg::Deg2Rad(45)), 45);
  EXPECT_DOUBLE_EQ(wkg::Rad2Deg(wkg::Deg2Rad(90)), 90);
  EXPECT_DOUBLE_EQ(wkg::Rad2Deg(wkg::Deg2Rad(135)), 135);
  EXPECT_DOUBLE_EQ(wkg::Rad2Deg(wkg::Deg2Rad(180)), 180);
  EXPECT_DOUBLE_EQ(wkg::Rad2Deg(wkg::Deg2Rad(270)), 270);
  EXPECT_DOUBLE_EQ(wkg::Rad2Deg(wkg::Deg2Rad(360)), 360);
  EXPECT_DOUBLE_EQ(wkg::Rad2Deg(wkg::Deg2Rad(480)), 480);
}


TEST(GeometryUtilsTest, FloatingPointEquality) {
  EXPECT_TRUE(wkg::IsEpsZero(0.0));
  EXPECT_TRUE(wkg::IsEpsZero(1e-64));

  // Never test for eqs_equal with 0!
  EXPECT_TRUE(wkg::IsEpsZero(1e-32));
  EXPECT_FALSE(wkg::IsEpsEqual(0.0, 1e-32));


  EXPECT_TRUE(wkg::IsEpsEqual(3, 3));
  EXPECT_TRUE(wkg::IsEpsEqual(5.0f, 5.0f));
  EXPECT_TRUE(wkg::IsEpsEqual(3.000000001f, 3.000000002f));
  EXPECT_FALSE(wkg::IsEpsEqual(5.0f, 5.0001f));
}


TEST(GeometryUtilsTest, Signum) {
  EXPECT_EQ(wkg::Sign(0), 0);
  EXPECT_EQ(wkg::Sign(-0), 0);

  EXPECT_EQ(wkg::Sign(static_cast<unsigned short>(0)), 0);
  EXPECT_EQ(wkg::Sign(static_cast<unsigned int>(0)), 0);
  EXPECT_EQ(wkg::Sign(0.0f), 0);
  EXPECT_EQ(wkg::Sign(0.0), 0);

  EXPECT_EQ(wkg::Sign(1), 1);
  EXPECT_EQ(wkg::Sign(-1), -1);
  EXPECT_EQ(wkg::Sign(static_cast<unsigned short>(1)), 1);
  EXPECT_EQ(wkg::Sign(static_cast<unsigned int>(1)), 1);

  EXPECT_EQ(wkg::Sign(23), 1);
  EXPECT_EQ(wkg::Sign(-13), -1);

  EXPECT_EQ(wkg::Sign(47.3), 1);
  EXPECT_EQ(wkg::Sign(-0.1), -1);

  EXPECT_EQ(wkg::Sign(0.001), 1);
  EXPECT_EQ(wkg::Sign(-0.001), -1);
  EXPECT_EQ(wkg::Sign(1e-6), 1);
  EXPECT_EQ(wkg::Sign(-(1e-6)), -1);
}

// NOLINTEND
