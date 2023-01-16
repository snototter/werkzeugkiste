#include <exception>
#include <initializer_list>
#include <cmath>
#include <limits>
#include <vector>
#include <list>

#include "../test_utils.h"

#include <werkzeugkiste/geometry/utils.h>

namespace wkg = werkzeugkiste::geometry;


// NOLINTBEGIN

TEST(GeometryUtilsTest, AngleConversion) {
  EXPECT_DOUBLE_EQ(0.0, wkg::Deg2Rad(0));
  EXPECT_DOUBLE_EQ(M_PI_4, wkg::Deg2Rad(45));
  EXPECT_DOUBLE_EQ(M_PI_2, wkg::Deg2Rad(90));
  EXPECT_DOUBLE_EQ(M_PI, wkg::Deg2Rad(180));
  EXPECT_DOUBLE_EQ(M_PI * 2, wkg::Deg2Rad(360));
  EXPECT_DOUBLE_EQ(M_PI * 4, wkg::Deg2Rad(720));

  EXPECT_DOUBLE_EQ(0.0, wkg::Rad2Deg(wkg::Deg2Rad(0)));
  EXPECT_DOUBLE_EQ(45, wkg::Rad2Deg(wkg::Deg2Rad(45)));
  EXPECT_DOUBLE_EQ(90, wkg::Rad2Deg(wkg::Deg2Rad(90)));
  EXPECT_DOUBLE_EQ(135, wkg::Rad2Deg(wkg::Deg2Rad(135)));
  EXPECT_DOUBLE_EQ(180, wkg::Rad2Deg(wkg::Deg2Rad(180)));
  EXPECT_DOUBLE_EQ(270, wkg::Rad2Deg(wkg::Deg2Rad(270)));
  EXPECT_DOUBLE_EQ(360, wkg::Rad2Deg(wkg::Deg2Rad(360)));
  EXPECT_DOUBLE_EQ(480, wkg::Rad2Deg(wkg::Deg2Rad(480)));
}


TEST(GeometryUtilsTest, FloatingPointEquality) {
  // Template specialization for integral types
  EXPECT_TRUE(wkg::IsEpsZero(0));
  EXPECT_FALSE(wkg::IsEpsZero(1));
  EXPECT_FALSE(wkg::IsEpsZero(-1));

  EXPECT_TRUE(wkg::IsEpsEqual(1, 1));
  EXPECT_FALSE(wkg::IsEpsEqual(1, 0));

  // Floating point checks
  EXPECT_TRUE(wkg::IsEpsZero(0.0));
  EXPECT_TRUE(wkg::IsEpsZero(std::numeric_limits<double>::epsilon()));
  EXPECT_TRUE(wkg::IsEpsZero(1e-64));
  EXPECT_TRUE(wkg::IsEpsZero(-(1e-64)));

  // Never test for eqs_equal with 0!
  EXPECT_FALSE(wkg::IsEpsZero(1e-7));
  EXPECT_FALSE(wkg::IsEpsEqual(0.0, 1e-7));
  EXPECT_TRUE(wkg::IsEpsZero(1e-50));
  EXPECT_FALSE(wkg::IsEpsEqual(0.0, 1e-50));
  EXPECT_FALSE(wkg::IsEpsEqual(0.0, -(1e-50)));
  EXPECT_FALSE(wkg::IsEpsEqual(0.0, 1e-7));

  // Special numbers
  const double nan = std::numeric_limits<double>::quiet_NaN();
  const double inf = std::numeric_limits<double>::infinity();

  EXPECT_FALSE(wkg::IsEpsZero(nan));
  EXPECT_FALSE(wkg::IsEpsZero(inf));
  EXPECT_FALSE(wkg::IsEpsEqual(nan, inf));
  EXPECT_FALSE(wkg::IsEpsEqual(inf, nan));
  EXPECT_FALSE(wkg::IsEpsEqual(inf, inf));
  EXPECT_FALSE(wkg::IsEpsEqual(inf, -inf));
  EXPECT_FALSE(wkg::IsEpsEqual(-inf, inf));
  EXPECT_FALSE(wkg::IsEpsEqual(-inf, inf));

  // Single-precision floats
  EXPECT_TRUE(wkg::IsEpsEqual(5.0f, 5.0f));
  EXPECT_TRUE(wkg::IsEpsEqual(3.000000001f, 3.000000002f));
  EXPECT_FALSE(wkg::IsEpsEqual(5.0f, 5.0001f));
  EXPECT_TRUE( wkg::IsEpsEqual(5.0f, 5.000001f));
}


TEST(GeometryUtilsTest, Signum) {
  EXPECT_EQ(wkg::Sign(0), 0);
  EXPECT_EQ(wkg::Sign(-0), 0);

  EXPECT_EQ(wkg::Sign(static_cast<unsigned short>(0)), 0);
  EXPECT_EQ(wkg::Sign(static_cast<unsigned int>(0)), 0);
  EXPECT_EQ(wkg::Sign(0.0f), 0);
  EXPECT_EQ(wkg::Sign(+0.0f), 0);
  EXPECT_EQ(wkg::Sign(-0.0f), 0);
  EXPECT_EQ(wkg::Sign(0.0), 0);
  EXPECT_EQ(wkg::Sign(+0.0), 0);
  EXPECT_EQ(wkg::Sign(-0.0), 0);

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
