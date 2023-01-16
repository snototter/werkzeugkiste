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


TEST(GeometryUtilsTest, FloatingPointZero) {
  // Check template specialization for integral types
  EXPECT_TRUE(wkg::IsEpsZero(0));
  EXPECT_FALSE(wkg::IsEpsZero(1));
  EXPECT_FALSE(wkg::IsEpsZero(-1));

  // Double-precision checks
  EXPECT_TRUE(wkg::IsEpsZero(0.0));
  EXPECT_TRUE(wkg::IsEpsZero(-0.0));

  EXPECT_TRUE(wkg::IsEpsZero(1e-64));
  EXPECT_TRUE(wkg::IsEpsZero(-(1e-64)));

  EXPECT_FALSE(wkg::IsEpsZero(0.1));
  EXPECT_FALSE(wkg::IsEpsZero(-0.1));

  EXPECT_FALSE(wkg::IsEpsZero(0.01));
  EXPECT_FALSE(wkg::IsEpsZero(-0.01));

  EXPECT_FALSE(wkg::IsEpsZero(0.001));
  EXPECT_FALSE(wkg::IsEpsZero(-0.001));

  EXPECT_FALSE(wkg::IsEpsZero(0.0001));
  EXPECT_FALSE(wkg::IsEpsZero(-0.0001));

  EXPECT_FALSE(wkg::IsEpsZero(0.00001));
  EXPECT_FALSE(wkg::IsEpsZero(-0.00001));

  EXPECT_FALSE(wkg::IsEpsZero(1e-6));
  EXPECT_FALSE(wkg::IsEpsZero(-(1e-6)));

  EXPECT_FALSE(wkg::IsEpsZero(1e-7));
  EXPECT_FALSE(wkg::IsEpsZero(-(1e-7)));

  EXPECT_FALSE(wkg::IsEpsZero(1e-8));
  EXPECT_FALSE(wkg::IsEpsZero(-(1e-8)));

  EXPECT_FALSE(wkg::IsEpsZero(1e-9));
  EXPECT_FALSE(wkg::IsEpsZero(-(1e-9)));

  EXPECT_FALSE(wkg::IsEpsZero(1e-10));
  EXPECT_FALSE(wkg::IsEpsZero(-(1e-10)));

  EXPECT_FALSE(wkg::IsEpsZero(1e-11));
  EXPECT_FALSE(wkg::IsEpsZero(-(1e-11)));

  EXPECT_FALSE(wkg::IsEpsZero(2.0 * std::numeric_limits<double>::epsilon()));
  EXPECT_FALSE(wkg::IsEpsZero(std::nextafter(std::numeric_limits<double>::epsilon(), 1.0)));
  EXPECT_TRUE(wkg::IsEpsZero(std::numeric_limits<double>::epsilon()));
  EXPECT_TRUE(wkg::IsEpsZero(std::nextafter(std::numeric_limits<double>::epsilon(), -1.0)));
  EXPECT_TRUE(wkg::IsEpsZero(std::numeric_limits<double>::epsilon() / 2.0));

  EXPECT_FALSE(wkg::IsEpsZero(-2.0 * std::numeric_limits<double>::epsilon()));
  EXPECT_TRUE(wkg::IsEpsZero(-std::numeric_limits<double>::epsilon()));
  EXPECT_TRUE(wkg::IsEpsZero(-std::numeric_limits<double>::epsilon() / 2.0));

  // Single-precision checks
  // (Only need to check values close to the machine epsilon as this is used
  //  as the absolute tolerance in this check)
  EXPECT_FALSE(wkg::IsEpsZero(2.0F * std::numeric_limits<float>::epsilon()));
  EXPECT_FALSE(wkg::IsEpsZero(std::nextafter(std::numeric_limits<float>::epsilon(), 1.0F)));
  EXPECT_TRUE(wkg::IsEpsZero(std::numeric_limits<float>::epsilon()));
  EXPECT_TRUE(wkg::IsEpsZero(std::nextafter(std::numeric_limits<float>::epsilon(), -1.0F)));
  EXPECT_TRUE(wkg::IsEpsZero(std::numeric_limits<float>::epsilon() / 2.0F));

  EXPECT_FALSE(wkg::IsEpsZero(-2.0 * std::numeric_limits<float>::epsilon()));
  EXPECT_TRUE(wkg::IsEpsZero(-std::numeric_limits<float>::epsilon()));
  EXPECT_TRUE(wkg::IsEpsZero(-std::numeric_limits<float>::epsilon() / 2.0F));

  // Special numbers
  const double nan_d = std::numeric_limits<double>::quiet_NaN();
  const double inf_d = std::numeric_limits<double>::infinity();

  EXPECT_FALSE(wkg::IsEpsZero(nan_d));
  EXPECT_FALSE(wkg::IsEpsZero(inf_d));
  EXPECT_FALSE(wkg::IsEpsZero(-inf_d));

  const float nan_f = std::numeric_limits<float>::quiet_NaN();
  const float inf_f = std::numeric_limits<float>::infinity();

  EXPECT_FALSE(wkg::IsEpsZero(nan_f));
  EXPECT_FALSE(wkg::IsEpsZero(inf_f));
  EXPECT_FALSE(wkg::IsEpsZero(-inf_f));
}


TEST(GeometryUtilsTest, FloatingPointEquality) {
  // Check template specialization for integral types
  EXPECT_TRUE(wkg::IsEpsEqual(1, 1));
  EXPECT_TRUE(wkg::IsEpsEqual(-1, -1));
  EXPECT_FALSE(wkg::IsEpsEqual(1, 0));
  EXPECT_FALSE(wkg::IsEpsEqual(-1, 0));

  // Test eps equality with the next representable number (do *not*
  // compare against 0, as the next representable would be really small,
  // e.g. 1e-324!)
  for (double value : {0.1, 0.01, 0.01, 1.0, 10.0, 12.0, 1e3, 1.17e16, 1.23e45, 4.5e98}) {
    auto next = std::nextafter(value, value + 1);
    EXPECT_TRUE(wkg::IsEpsEqual(value, next))
        << value << " should equal " << next
        << " (which is the next representable number).";
    EXPECT_FALSE(wkg::IsEpsEqual(-value, next))
        << (-value) << " should NOT equal " << next << '.';

    auto scaled = value + (value * 1e-12);
    EXPECT_TRUE(wkg::IsEpsEqual(value, scaled))
        << value << " should equal " << next
        << " (because of 1e-10 precision threshold).";
    scaled = value + (value * 1e-9);
    EXPECT_FALSE(wkg::IsEpsEqual(value, scaled))
        << value << " should NOT equal " << next
        << " (because of 1e-10 precision threshold).";
  }

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
