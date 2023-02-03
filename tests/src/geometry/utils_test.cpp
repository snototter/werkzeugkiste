#include <werkzeugkiste/geometry/utils.h>

#include <cmath>
#include <exception>
#include <initializer_list>
#include <limits>
#include <list>
#include <vector>

#include "../test_utils.h"

namespace wkg = werkzeugkiste::geometry;

// NOLINTBEGIN

TEST(GeometryUtilsTest, AngleConversion) {
  // Integer angles are cast to double-precision floats:
  EXPECT_DOUBLE_EQ(0.0, wkg::Deg2Rad(0));
  EXPECT_DOUBLE_EQ(0.0, wkg::Deg2Rad(0.0));
  EXPECT_DOUBLE_EQ(wkg::constants::pi_d / 4, wkg::Deg2Rad(45));
  EXPECT_DOUBLE_EQ(wkg::constants::pi_d / 4, wkg::Deg2Rad(45.0));
  EXPECT_DOUBLE_EQ(wkg::constants::pi_d / 2, wkg::Deg2Rad(90));
  EXPECT_DOUBLE_EQ(wkg::constants::pi_d / 2, wkg::Deg2Rad(90.0));
  EXPECT_DOUBLE_EQ(wkg::constants::pi_d, wkg::Deg2Rad(180));
  EXPECT_DOUBLE_EQ(wkg::constants::pi_d, wkg::Deg2Rad(180.0));
  EXPECT_DOUBLE_EQ(wkg::constants::pi_d * 2, wkg::Deg2Rad(360));
  EXPECT_DOUBLE_EQ(wkg::constants::pi_d * 2, wkg::Deg2Rad(360.0));
  EXPECT_DOUBLE_EQ(wkg::constants::pi_d * 4, wkg::Deg2Rad(720));
  EXPECT_DOUBLE_EQ(wkg::constants::pi_d * 4, wkg::Deg2Rad(720.0));

  // Back-and-forth conversion:
  EXPECT_DOUBLE_EQ(0.0, wkg::Rad2Deg(wkg::Deg2Rad(0)));
  EXPECT_DOUBLE_EQ(45, wkg::Rad2Deg(wkg::Deg2Rad(45)));
  EXPECT_DOUBLE_EQ(90, wkg::Rad2Deg(wkg::Deg2Rad(90)));
  EXPECT_DOUBLE_EQ(135, wkg::Rad2Deg(wkg::Deg2Rad(135)));
  EXPECT_DOUBLE_EQ(180, wkg::Rad2Deg(wkg::Deg2Rad(180)));
  EXPECT_DOUBLE_EQ(270, wkg::Rad2Deg(wkg::Deg2Rad(270)));
  EXPECT_DOUBLE_EQ(360, wkg::Rad2Deg(wkg::Deg2Rad(360)));
  EXPECT_DOUBLE_EQ(480, wkg::Rad2Deg(wkg::Deg2Rad(480)));

  // Single-precision floats:
  EXPECT_FLOAT_EQ(0.0F, wkg::Deg2Rad(0.0F));
  EXPECT_FLOAT_EQ(wkg::constants::pi_f / 4, wkg::Deg2Rad(45.0F));
  EXPECT_FLOAT_EQ(wkg::constants::pi_f / 2, wkg::Deg2Rad(90.0F));
  EXPECT_FLOAT_EQ(wkg::constants::pi_f, wkg::Deg2Rad(180.0F));
  EXPECT_FLOAT_EQ(wkg::constants::pi_f * 2, wkg::Deg2Rad(360.0F));
  EXPECT_FLOAT_EQ(wkg::constants::pi_f * 4, wkg::Deg2Rad(720.0F));
  EXPECT_FLOAT_EQ(0.0F, wkg::Rad2Deg(wkg::Deg2Rad(0.0F)));
  EXPECT_FLOAT_EQ(45.0F, wkg::Rad2Deg(wkg::Deg2Rad(45.0F)));
  EXPECT_FLOAT_EQ(90.0F, wkg::Rad2Deg(wkg::Deg2Rad(90.0F)));
  EXPECT_FLOAT_EQ(135.0F, wkg::Rad2Deg(wkg::Deg2Rad(135.0F)));
  EXPECT_FLOAT_EQ(180.0F, wkg::Rad2Deg(wkg::Deg2Rad(180.0F)));
  EXPECT_FLOAT_EQ(270.0F, wkg::Rad2Deg(wkg::Deg2Rad(270.0F)));
  EXPECT_FLOAT_EQ(360.0F, wkg::Rad2Deg(wkg::Deg2Rad(360.0F)));
  EXPECT_FLOAT_EQ(480.0F, wkg::Rad2Deg(wkg::Deg2Rad(480.0F)));
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

  EXPECT_TRUE(wkg::IsEpsZero(1e-50));
  EXPECT_TRUE(wkg::IsEpsZero(-(1e-50)));

  EXPECT_FALSE(wkg::IsEpsZero(2.0 * std::numeric_limits<double>::epsilon()));
  EXPECT_FALSE(wkg::IsEpsZero(
      std::nextafter(std::numeric_limits<double>::epsilon(), 1.0)));
  EXPECT_TRUE(wkg::IsEpsZero(std::numeric_limits<double>::epsilon()));
  EXPECT_TRUE(wkg::IsEpsZero(
      std::nextafter(std::numeric_limits<double>::epsilon(), -1.0)));
  EXPECT_TRUE(wkg::IsEpsZero(std::numeric_limits<double>::epsilon() / 2.0));

  EXPECT_FALSE(wkg::IsEpsZero(-2.0 * std::numeric_limits<double>::epsilon()));
  EXPECT_TRUE(wkg::IsEpsZero(-std::numeric_limits<double>::epsilon()));
  EXPECT_TRUE(wkg::IsEpsZero(-std::numeric_limits<double>::epsilon() / 2.0));

  // Single-precision checks
  // (Only need to check values close to the machine epsilon as this is used
  //  as the absolute tolerance in this check)
  EXPECT_FALSE(wkg::IsEpsZero(2.0F * std::numeric_limits<float>::epsilon()));
  EXPECT_FALSE(wkg::IsEpsZero(
      std::nextafter(std::numeric_limits<float>::epsilon(), 1.0F)));
  EXPECT_TRUE(wkg::IsEpsZero(std::numeric_limits<float>::epsilon()));
  EXPECT_TRUE(wkg::IsEpsZero(
      std::nextafter(std::numeric_limits<float>::epsilon(), -1.0F)));
  EXPECT_TRUE(wkg::IsEpsZero(std::numeric_limits<float>::epsilon() / 2.0F));

  EXPECT_FALSE(wkg::IsEpsZero(-2.0F * std::numeric_limits<float>::epsilon()));
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

TEST(GeometryUtilsTest, FloatingPointEquality1) {
  // Check template specialization for integral types
  EXPECT_TRUE(wkg::IsEpsEqual(1, 1));
  EXPECT_TRUE(wkg::IsEpsEqual(-1, -1));
  EXPECT_FALSE(wkg::IsEpsEqual(1, 0));
  EXPECT_FALSE(wkg::IsEpsEqual(-1, 0));

  // IsEpsEqual uses a practical relative
  // tolerance of 1e-9 (in my opinion).
  // This is a single test case - for more, refer
  // to the `FloatingPointEquality2` test.
  EXPECT_FALSE(wkg::IsEpsEqual(5.0, 5.1));
  EXPECT_FALSE(wkg::IsEpsEqual(5.0, 5.01));
  EXPECT_FALSE(wkg::IsEpsEqual(5.0, 5.001));
  EXPECT_FALSE(wkg::IsEpsEqual(5.0, 5.0001));
  EXPECT_FALSE(wkg::IsEpsEqual(5.0, 5.000001));
  EXPECT_FALSE(wkg::IsEpsEqual(5.0, 5.0000001));
  EXPECT_FALSE(wkg::IsEpsEqual(5.0, 5.00000001));

  EXPECT_TRUE(wkg::IsEpsEqual(5.0, 5.0000000001));
  EXPECT_TRUE(wkg::IsEpsEqual(5.0, 5.00000000001));
  EXPECT_TRUE(wkg::IsEpsEqual(5.0, 5.000000000001));

  EXPECT_FALSE(wkg::IsEpsEqual(5e12, 5e-12));
  EXPECT_TRUE(wkg::IsEpsEqual(5e12, 5e12 + 0.001));
  EXPECT_FALSE(wkg::IsEpsEqual(5e111, 5e-111));

  // Never test for eqs_equal with 0!
  EXPECT_FALSE(wkg::IsEpsZero(1e-7));
  EXPECT_FALSE(wkg::IsEpsZero(1e-12));
  EXPECT_FALSE(wkg::IsEpsZero(1e-14));
  EXPECT_TRUE(wkg::IsEpsZero(std::numeric_limits<double>::epsilon()));
  EXPECT_FALSE(wkg::IsEpsEqual(0.0, 1e-7));
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

  EXPECT_TRUE(wkg::IsEpsEqual(3.0000001F, 3.0000002F));
  EXPECT_TRUE(wkg::IsEpsEqual(3.0000001F, 3.000002F));
  EXPECT_FALSE(wkg::IsEpsEqual(3.0000001F, 3.0002F));

  EXPECT_TRUE(wkg::IsEpsEqual(5.0F, 5.0F));
  EXPECT_FALSE(wkg::IsEpsEqual(5.0F, 5.1F));
  EXPECT_FALSE(wkg::IsEpsEqual(5.0F, 5.01F));
  EXPECT_FALSE(wkg::IsEpsEqual(5.0F, 5.001F));
  EXPECT_FALSE(wkg::IsEpsEqual(5.0F, 5.0001F));
  EXPECT_FALSE(wkg::IsEpsEqual(5.0F, 5.00001F));

  EXPECT_TRUE(wkg::IsEpsEqual(5.0F, 5.000001F));
  EXPECT_TRUE(wkg::IsEpsEqual(5.0F, 5.0000001F));
  EXPECT_TRUE(wkg::IsEpsEqual(5.0F, 5.00000001F));
}

TEST(GeometryUtilsTest, FloatingPointEquality2) {
  // Test eps equality with the next representable number (do *not*
  // compare against 0, as the next representable would be really small,
  // e.g. 1e-324!)
  for (double value :
       {0.1, 0.01, 0.01, 1.0, 10.0, 12.0, 1e3, 1.17e16, 1.23e45, 4.5e98}) {
    auto next = std::nextafter(value, value + 1);
    EXPECT_TRUE(wkg::IsEpsEqual(value, next))
        << "    " << value << " should equal " << next
        << " (which is the next representable number).";

    EXPECT_FALSE(wkg::IsEpsEqual(-value, next))
        << "    " << (-value) << " should NOT equal " << next << '.';

    // TODO filter out flaky tests (i.e. value >= 10), then the difference
    // becomes larger than the machine epsilon (1e-16, 1e-15, etc).
    //    // If above checks succeed, then the difference between
    //    // these must be 0 (but not the value itself).
    //    EXPECT_FALSE(wkg::IsEpsZero(value))
    //        << "    Value " << value << " should NOT equal 0.";
    //    EXPECT_TRUE(wkg::IsEpsZero(value - next))
    //        << "    Difference (" << value << " - " << next
    //        << ") should equal 0.";
    //    EXPECT_TRUE(wkg::IsEpsZero(next - value))
    //        << "    Difference (" << next << " - " << value
    //        << ") should equal 0.";

    // Scale the current value close to the precision
    // threshold.
    auto scaled = value + (value * 1e-10);
    EXPECT_TRUE(wkg::IsEpsEqual(value, scaled))
        << "    Value " << value << " should equal " << scaled
        << " (because of 1e-9 precision threshold), difference: "
        << (value - scaled);

    scaled = value + (value * 1e-8);
    EXPECT_FALSE(wkg::IsEpsEqual(value, scaled))
        << "    Value " << value << " should NOT equal " << scaled
        << " (because of 1e-9 precision threshold), difference: "
        << (value - scaled);
  }

  // Similar to the double-precision loop above, we also
  // conduct additional single-precision checks:
  for (float value :
       {0.1F, 0.01F, 0.01F, 1.0F, 10.0F, 12.0F, 1000.0F, 1234.56F, 0.001234F}) {
    auto next = std::nextafter(value, value + 1);
    EXPECT_TRUE(wkg::IsEpsEqual(value, next))
        << "    Value " << value << " should equal " << next
        << " (which is the next representable number).";

    EXPECT_FALSE(wkg::IsEpsEqual(-value, next))
        << "    Value " << (-value) << " should NOT equal " << next << '.';

    // Scale the current value close to the precision
    // threshold.
    // Skip values > 1e3 as these would cause false alerts due to
    // the limited float precision.
    auto scaled = value + (value * 0.0000009F);
    EXPECT_TRUE(wkg::IsEpsEqual(value, scaled))
        << "    Value " << value << " should equal " << scaled
        << " (because of 1e-6 precision threshold), difference: "
        << (value - scaled);

    scaled = value + (value * 0.00002F);
    EXPECT_FALSE(wkg::IsEpsEqual(value, scaled))
        << "    Value " << value << " should NOT equal " << scaled
        << " (because of 1e-6 precision threshold), difference: "
        << (value - scaled);
  }
}

TEST(GeometryUtilsTest, Constants) {
  // Pi, double precision
  constexpr double pi_dbl{3.14159265358979323};
  EXPECT_TRUE(wkg::IsEpsEqual(wkg::constants::pi_d, pi_dbl));
  EXPECT_TRUE(wkg::IsEpsEqual(wkg::constants::pi_d + 1e-10, pi_dbl));
  EXPECT_FALSE(wkg::IsEpsEqual(wkg::constants::pi_d + 1e-8, pi_dbl));
  EXPECT_FALSE(wkg::IsEpsEqual(wkg::constants::pi_d + 1e-7, pi_dbl));

  EXPECT_TRUE(wkg::IsEpsEqual(1.0 / pi_dbl, wkg::constants::inv_pi_d));
  EXPECT_TRUE(
      wkg::IsEpsEqual(1.0 / wkg::constants::pi_d, wkg::constants::inv_pi_d));

  // Pi, single precision
  constexpr float pi_flt{3.14159265358979323F};
  EXPECT_TRUE(wkg::IsEpsEqual(wkg::constants::pi_f, pi_flt));
  EXPECT_FALSE(wkg::IsEpsEqual(wkg::constants::pi_f + 1e-5F, pi_flt));
  EXPECT_FALSE(wkg::IsEpsEqual(wkg::constants::pi_f + 1e-4F, pi_flt));

  EXPECT_TRUE(wkg::IsEpsEqual(1.0F / pi_flt, wkg::constants::inv_pi_f));
  EXPECT_TRUE(
      wkg::IsEpsEqual(1.0F / wkg::constants::pi_f, wkg::constants::inv_pi_f));

  // Square root of 2
  EXPECT_TRUE(wkg::IsEpsEqual(wkg::constants::sqrt2_d, 1.41421356237309504));
  EXPECT_TRUE(
      wkg::IsEpsEqual(wkg::constants::sqrt2_d * wkg::constants::sqrt2_d, 2.0));

  EXPECT_TRUE(wkg::IsEpsEqual(wkg::constants::sqrt2_f, 1.41421356237309504F));
  EXPECT_TRUE(
      wkg::IsEpsEqual(wkg::constants::sqrt2_f * wkg::constants::sqrt2_f, 2.0F));
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

TEST(GeometryUtilsTest, Rounding) {
  // Base 5
  EXPECT_DOUBLE_EQ(10.0, wkg::RoundBase(9.0, 5.0));
  EXPECT_DOUBLE_EQ(15.0, wkg::RoundBase(13.0, 5.0));
  EXPECT_DOUBLE_EQ(10.0, wkg::RoundBase(12.4, 5.0));

  EXPECT_EQ(10, wkg::RoundBase(9, 5));
  EXPECT_EQ(15, wkg::RoundBase(13, 5));
  EXPECT_EQ(10, wkg::RoundBase(12, 5));

  // Base 2
  EXPECT_EQ(2, wkg::RoundBase(1, 2));
  EXPECT_DOUBLE_EQ(2.0, wkg::RoundBase(1.0, 2.0));
  EXPECT_DOUBLE_EQ(0.0, wkg::RoundBase(0.2, 2.0));

  // Base 10
  EXPECT_EQ(0, wkg::RoundBase(1, 10));
  EXPECT_EQ(20, wkg::RoundBase(15, 10));
  EXPECT_EQ(12350, wkg::RoundBase(12345, 10));
  EXPECT_DOUBLE_EQ(10.0, wkg::RoundBase(14.9, 10.0));

  // Base < 1
  EXPECT_DOUBLE_EQ(1.00, wkg::RoundBase(0.96, 0.1));
  EXPECT_DOUBLE_EQ(0.95, wkg::RoundBase(0.96, 0.05));
  EXPECT_DOUBLE_EQ(1.00, wkg::RoundBase(0.999, 0.1));
  EXPECT_DOUBLE_EQ(0.90, wkg::RoundBase(0.921, 0.05));
  EXPECT_DOUBLE_EQ(0.67, wkg::RoundBase(0.671, 0.01));
  EXPECT_DOUBLE_EQ(0.70, wkg::RoundBase(0.671, 0.1));
}

// NOLINTEND
