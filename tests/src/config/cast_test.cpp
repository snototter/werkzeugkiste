#include <werkzeugkiste/config/casts.h>

#include <cmath>
#include <limits>

#include "../test_utils.h"

namespace wkc = werkzeugkiste::config;

// NOLINTBEGIN
TEST(CastTest, Static) {
  static_assert(wkc::are_integral_v<int, int16_t>);
  bool flag = wkc::are_integral_v<int, int16_t>;
  EXPECT_TRUE(flag);

  static_assert(wkc::are_integral_v<unsigned int, int16_t>);
  flag = wkc::are_integral_v<unsigned int, int16_t>;
  EXPECT_TRUE(flag);

  static_assert(wkc::are_integral_v<int, bool>);
  flag = wkc::are_integral_v<int, bool>;
  EXPECT_TRUE(flag);

  static_assert(!wkc::are_integral_v<float, bool>);
  flag = wkc::are_integral_v<float, bool>;
  EXPECT_FALSE(flag);

  static_assert(!wkc::are_integral_v<int, float>);
  flag = wkc::are_integral_v<int, float>;
  EXPECT_FALSE(flag);

  static_assert(!wkc::are_integral_v<int, double>);
  flag = wkc::are_integral_v<int, double>;
  EXPECT_FALSE(flag);

  static_assert(!wkc::are_integral_v<std::string, short>);
  flag = wkc::are_integral_v<std::string, short>;
  EXPECT_FALSE(flag);

  static_assert(wkc::are_floating_point_v<float, float>);
  flag = wkc::are_floating_point_v<float, float>;
  EXPECT_TRUE(flag);

  static_assert(wkc::are_floating_point_v<float, double>);
  flag = wkc::are_floating_point_v<float, double>;
  EXPECT_TRUE(flag);

  static_assert(wkc::are_floating_point_v<double, long double>);
  flag = wkc::are_floating_point_v<double, long double>;
  EXPECT_TRUE(flag);

  static_assert(!wkc::are_floating_point_v<float, int>);
  flag = wkc::are_floating_point_v<float, int>;
  EXPECT_FALSE(flag);

  static_assert(!wkc::are_floating_point_v<int, float>);
  flag = wkc::are_floating_point_v<int, float>;
  EXPECT_FALSE(flag);

  static_assert(!wkc::are_floating_point_v<std::string, float>);
  flag = wkc::are_floating_point_v<std::string, float>;
  EXPECT_FALSE(flag);

  static_assert(wkc::is_promotable<int, long>());
  flag = wkc::is_promotable<int, long>();
  EXPECT_TRUE(flag);

  static_assert(wkc::is_promotable<char, int>());
  flag = wkc::is_promotable<char, int>();
  EXPECT_TRUE(flag);

  static_assert(wkc::is_promotable<float, double>());
  flag = wkc::is_promotable<float, double>();
  EXPECT_TRUE(flag);

  static_assert(!wkc::is_promotable<int, char>());
  flag = wkc::is_promotable<int, char>();
  EXPECT_FALSE(flag);

  static_assert(!wkc::is_promotable<uint, int>());
  flag = wkc::is_promotable<uint, int>();
  EXPECT_FALSE(flag);

  static_assert(wkc::is_promotable<int_fast8_t, int_fast16_t>());
  flag = wkc::is_promotable<int_fast8_t, int_fast16_t>();
  EXPECT_TRUE(flag);

  static_assert(wkc::is_promotable<int_fast8_t, int_fast8_t>());
  flag = wkc::is_promotable<int_fast8_t, int_fast8_t>();
  EXPECT_TRUE(flag);

  static_assert(wkc::is_promotable<int_fast16_t, int_fast16_t>());
  flag = wkc::is_promotable<int_fast16_t, int_fast16_t>();
  EXPECT_TRUE(flag);

  EXPECT_FLOAT_EQ(1.0F, wkc::detail::pow2<float>(0));
  EXPECT_FLOAT_EQ(8.0F, wkc::detail::pow2<float>(3));
  EXPECT_DOUBLE_EQ(32.0, wkc::detail::pow2<double>(5));
  EXPECT_THROW(wkc::detail::pow2<float>(-1), std::logic_error);

  auto range = wkc::detail::float_to_int_range<int8_t, float>();
  EXPECT_FLOAT_EQ(-wkc::detail::pow2<float>(7), range.first);

  // To increase the coverage, we also need to perform these checks at runtime
  bool val = wkc::is_promotable<int, int>();
  EXPECT_TRUE(val);
  val = wkc::is_promotable<float, double>();
  EXPECT_TRUE(val);
  val = wkc::is_promotable<long double, float>();
  EXPECT_FALSE(val);

  val = wkc::is_promotable<int8_t, int16_t>();
  EXPECT_TRUE(val);
  val = wkc::is_promotable<uint8_t, uint16_t>();
  EXPECT_TRUE(val);

  val = wkc::is_promotable<int32_t, int16_t>();
  EXPECT_FALSE(val);
  val = wkc::is_promotable<uint32_t, uint16_t>();
  EXPECT_FALSE(val);

  val = wkc::is_promotable<bool, uint8_t>();
  EXPECT_TRUE(val);
  val = wkc::is_promotable<uint8_t, bool>();
  EXPECT_FALSE(val);
}

TEST(CastTest, CheckedBoolean) {
  // From bool to integral (signed/unsigned):
  EXPECT_EQ(1, wkc::checked_numcast<int>(true));
  EXPECT_EQ(1, wkc::checked_numcast<int8_t>(true));
  EXPECT_EQ(1, wkc::checked_numcast<uint8_t>(true));
  EXPECT_EQ(1, wkc::checked_numcast<int16_t>(true));

  EXPECT_EQ(0, wkc::checked_numcast<int>(false));
  EXPECT_EQ(0, wkc::checked_numcast<int8_t>(false));
  EXPECT_EQ(0, wkc::checked_numcast<uint8_t>(false));
  EXPECT_EQ(0, wkc::checked_numcast<int16_t>(false));

  // From bool to float:
  EXPECT_FLOAT_EQ(1.0F, wkc::checked_numcast<float>(true));
  EXPECT_DOUBLE_EQ(1.0, wkc::checked_numcast<double>(true));

  EXPECT_FLOAT_EQ(0.0F, wkc::checked_numcast<float>(false));
  EXPECT_DOUBLE_EQ(0.0, wkc::checked_numcast<double>(false));

  // From bool to bool:
  EXPECT_EQ(false, wkc::checked_numcast<bool>(false));
  EXPECT_EQ(true, wkc::checked_numcast<bool>(true));

  // From integral (signed/unsigned) to bool:
  EXPECT_EQ(false, wkc::checked_numcast<bool>(0));
  EXPECT_EQ(true, wkc::checked_numcast<bool>(1));
  EXPECT_EQ(true, wkc::checked_numcast<bool>(2));
  EXPECT_EQ(true, wkc::checked_numcast<bool>(-1));
  EXPECT_EQ(true, wkc::checked_numcast<bool>(-42));

  EXPECT_EQ(false, wkc::checked_numcast<bool>(static_cast<uint8_t>(0)));
  EXPECT_EQ(true, wkc::checked_numcast<bool>(static_cast<uint16_t>(1)));
  EXPECT_EQ(true, wkc::checked_numcast<bool>(static_cast<uint32_t>(2)));

  // From float to bool:
  EXPECT_EQ(false, wkc::checked_numcast<bool>(0.0F));
  EXPECT_EQ(false, wkc::checked_numcast<bool>(0.0));
  EXPECT_EQ(false, wkc::checked_numcast<bool>(0.0L));

  EXPECT_EQ(true, wkc::checked_numcast<bool>(0.001F));
  EXPECT_EQ(true, wkc::checked_numcast<bool>(0.00001));
  EXPECT_EQ(true, wkc::checked_numcast<bool>(0.0000001L));

  EXPECT_EQ(true, wkc::checked_numcast<bool>(-42.0F));
  EXPECT_EQ(true, wkc::checked_numcast<bool>(-42.0));
  EXPECT_EQ(true, wkc::checked_numcast<bool>(-42.0L));
}

TEST(CastTest, SafeBoolean) {
  // From bool to integral (signed/unsigned):
  EXPECT_EQ(1, wkc::safe_numcast<int>(true).value());
  EXPECT_EQ(1, wkc::safe_numcast<int8_t>(true).value());
  EXPECT_EQ(1, wkc::safe_numcast<uint8_t>(true).value());
  EXPECT_EQ(1, wkc::safe_numcast<int16_t>(true).value());

  EXPECT_EQ(0, wkc::safe_numcast<int>(false).value());
  EXPECT_EQ(0, wkc::safe_numcast<int8_t>(false).value());
  EXPECT_EQ(0, wkc::safe_numcast<uint8_t>(false).value());
  EXPECT_EQ(0, wkc::safe_numcast<int16_t>(false).value());

  // From bool to float:
  EXPECT_FLOAT_EQ(1.0F, wkc::safe_numcast<float>(true).value());
  EXPECT_DOUBLE_EQ(1.0, wkc::safe_numcast<double>(true).value());

  EXPECT_FLOAT_EQ(0.0F, wkc::safe_numcast<float>(false).value());
  EXPECT_DOUBLE_EQ(0.0, wkc::safe_numcast<double>(false).value());

  // From bool to bool:
  EXPECT_EQ(false, wkc::safe_numcast<bool>(false).value());
  EXPECT_EQ(true, wkc::safe_numcast<bool>(true).value());

  // From integral (signed/unsigned) to bool:
  EXPECT_EQ(false, wkc::safe_numcast<bool>(0).value());
  EXPECT_EQ(true, wkc::safe_numcast<bool>(1).value());
  EXPECT_EQ(true, wkc::safe_numcast<bool>(2).value());
  EXPECT_EQ(true, wkc::safe_numcast<bool>(-1).value());
  EXPECT_EQ(true, wkc::safe_numcast<bool>(-42).value());

  EXPECT_EQ(false, wkc::safe_numcast<bool>(static_cast<uint8_t>(0)).value());
  EXPECT_EQ(true, wkc::safe_numcast<bool>(static_cast<uint16_t>(1)).value());
  EXPECT_EQ(true, wkc::safe_numcast<bool>(static_cast<uint32_t>(2)).value());

  // From float to bool:
  EXPECT_EQ(false, wkc::safe_numcast<bool>(0.0F).value());
  EXPECT_EQ(false, wkc::safe_numcast<bool>(0.0).value());
  EXPECT_EQ(false, wkc::safe_numcast<bool>(0.0L).value());

  EXPECT_EQ(true, wkc::safe_numcast<bool>(0.001F).value());
  EXPECT_EQ(true, wkc::safe_numcast<bool>(0.00001).value());
  EXPECT_EQ(true, wkc::safe_numcast<bool>(0.0000001L).value());

  EXPECT_EQ(true, wkc::safe_numcast<bool>(-42.0F).value());
  EXPECT_EQ(true, wkc::safe_numcast<bool>(-42.0).value());
  EXPECT_EQ(true, wkc::safe_numcast<bool>(-42.0L).value());
}

TEST(CastTest, CheckedIntegral) {
  // To check: (S)igned, (U)nsigned
  // (1) S -> S, narrowing
  // (2) S -> S, widening/promoting
  // (3) S -> U, narrowing
  // (4) S -> U, widening/promoting
  // (5) U -> S, narrowing
  // (6) U -> S, widening/promoting
  // (7) U -> U, narrowing
  // (8) U -> U, widening/promoting

  // (0) Sanity check, no cast required
  EXPECT_EQ(false, wkc::checked_numcast<bool>(false));
  EXPECT_EQ(-17, wkc::checked_numcast<int>(-17));
  EXPECT_EQ(3.5F, wkc::checked_numcast<float>(3.5F));

  // (1) From signed to signed, narrowing:
  EXPECT_EQ(static_cast<int8_t>(0), wkc::checked_numcast<int8_t>(0));

  const auto int8_min = std::numeric_limits<int8_t>::min();
  EXPECT_EQ(
      int8_min, wkc::checked_numcast<int8_t>(static_cast<int32_t>(int8_min)));
  EXPECT_EQ(int8_min + 1,
      wkc::checked_numcast<int8_t>(static_cast<int32_t>(int8_min) + 1));
  EXPECT_THROW(wkc::checked_numcast<int8_t>(static_cast<int32_t>(int8_min) - 1),
      std::domain_error);

  const auto int8_max = std::numeric_limits<int8_t>::max();
  EXPECT_EQ(
      int8_max, wkc::checked_numcast<int8_t>(static_cast<int32_t>(int8_max)));
  EXPECT_EQ(int8_max - 1,
      wkc::checked_numcast<int8_t>(static_cast<int32_t>(int8_max) - 1));
  EXPECT_THROW(wkc::checked_numcast<int8_t>(static_cast<int32_t>(int8_max) + 1),
      std::domain_error);

  // (2) From signed to signed, widening/promotion:
  EXPECT_EQ(0L, wkc::checked_numcast<int64_t>(0));

  const auto int32_min = std::numeric_limits<int32_t>::min();
  EXPECT_EQ(static_cast<int64_t>(int32_min),
      wkc::checked_numcast<int64_t>(int32_min));

  const auto int32_max = std::numeric_limits<int32_t>::max();
  EXPECT_EQ(static_cast<int64_t>(int32_max),
      wkc::checked_numcast<int64_t>(int32_max));

  // (3) From signed to unsigned, narrowing:
  EXPECT_EQ(0, wkc::checked_numcast<uint8_t>(0L));

  EXPECT_EQ(100, wkc::checked_numcast<uint8_t>(100L));
  EXPECT_EQ(255, wkc::checked_numcast<uint8_t>(255L));

  EXPECT_THROW(wkc::checked_numcast<uint8_t>(-1), std::domain_error);
  EXPECT_THROW(wkc::checked_numcast<uint8_t>(256L), std::domain_error);

  // (4) From signed to unsigned, widening/promotion:
  EXPECT_THROW(wkc::checked_numcast<uint16_t>(static_cast<int8_t>(-1)),
      std::domain_error);
  EXPECT_EQ(0, wkc::checked_numcast<uint16_t>(static_cast<int8_t>(0)));
  EXPECT_EQ(127, wkc::checked_numcast<uint16_t>(static_cast<int8_t>(127)));

  // (5) From unsigned to signed, narrowing:
  EXPECT_EQ(127, wkc::checked_numcast<int8_t>(static_cast<uint8_t>(127)));
  EXPECT_EQ(127, wkc::checked_numcast<int8_t>(static_cast<uint16_t>(127)));
  EXPECT_EQ(0, wkc::checked_numcast<int8_t>(static_cast<uint16_t>(0)));
  EXPECT_THROW(wkc::checked_numcast<int8_t>(static_cast<uint8_t>(255)),
      std::domain_error);
  EXPECT_THROW(wkc::checked_numcast<int8_t>(static_cast<uint32_t>(1000)),
      std::domain_error);
  EXPECT_THROW(wkc::checked_numcast<int16_t>(static_cast<uint32_t>(100000)),
      std::domain_error);

  // (6) From unsigned to signed, widening/promotion:
  EXPECT_EQ(127, wkc::checked_numcast<int16_t>(static_cast<uint16_t>(127)));
  EXPECT_EQ(1000, wkc::checked_numcast<int32_t>(static_cast<uint16_t>(1000)));
  EXPECT_EQ(0, wkc::checked_numcast<int32_t>(static_cast<uint16_t>(0)));
  EXPECT_EQ(12345L, wkc::checked_numcast<long>(static_cast<uint16_t>(12345)));

  // (7) From unsigned to unsigned, narrowing cast:
  EXPECT_EQ(0, wkc::checked_numcast<uint8_t>(0UL));
  EXPECT_EQ(100, wkc::checked_numcast<uint8_t>(100UL));
  EXPECT_EQ(255, wkc::checked_numcast<uint8_t>(255UL));
  EXPECT_THROW(wkc::checked_numcast<uint8_t>(256UL), std::domain_error);

  // (8) From unsigned to unsigned, widening cast:
  EXPECT_EQ(0L, wkc::checked_numcast<uint64_t>(0));
  EXPECT_EQ(100L, wkc::checked_numcast<uint64_t>(100));
  const auto uint32_max = std::numeric_limits<uint32_t>::max();
  EXPECT_EQ(static_cast<uint64_t>(uint32_max),
      wkc::checked_numcast<uint64_t>(uint32_max));
}

TEST(CastTest, SafeIntegral) {
  // To check: (S)igned, (U)nsigned
  // (1) S -> S, narrowing
  // (2) S -> S, widening/promoting
  // (3) S -> U, narrowing
  // (4) S -> U, widening/promoting
  // (5) U -> S, narrowing
  // (6) U -> S, widening/promoting
  // (7) U -> U, narrowing
  // (8) U -> U, widening/promoting

  // (1) From signed to signed, narrowing:
  EXPECT_EQ(static_cast<int8_t>(0), wkc::safe_numcast<int8_t>(0).value());

  const auto int8_min = std::numeric_limits<int8_t>::min();
  auto val_i32 = static_cast<int32_t>(int8_min);
  EXPECT_EQ(int8_min, wkc::safe_numcast<int8_t>(val_i32).value());
  val_i32 = static_cast<int32_t>(int8_min) + 1;
  EXPECT_EQ(int8_min + 1, wkc::safe_numcast<int8_t>(val_i32).value());
  val_i32 = static_cast<int32_t>(int8_min) - 1;
  EXPECT_FALSE(wkc::safe_numcast<int8_t>(val_i32).has_value());

  const auto int8_max = std::numeric_limits<int8_t>::max();
  val_i32 = static_cast<int32_t>(int8_max);
  EXPECT_EQ(int8_max, wkc::safe_numcast<int8_t>(val_i32).value());
  val_i32 = static_cast<int32_t>(int8_max) - 1;
  EXPECT_EQ(int8_max - 1, wkc::safe_numcast<int8_t>(val_i32).value());
  val_i32 = static_cast<int32_t>(int8_max) + 1;
  EXPECT_FALSE(wkc::safe_numcast<int8_t>(val_i32).has_value());

  // (2) From signed to signed, widening/promotion:
  EXPECT_EQ(0L, wkc::safe_numcast<int64_t>(0).value());

  const auto int32_min = std::numeric_limits<int32_t>::min();
  EXPECT_EQ(static_cast<int64_t>(int32_min),
      wkc::safe_numcast<int64_t>(int32_min).value());

  const auto int32_max = std::numeric_limits<int32_t>::max();
  EXPECT_EQ(static_cast<int64_t>(int32_max),
      wkc::safe_numcast<int64_t>(int32_max).value());

  // (3) From signed to unsigned, narrowing:
  EXPECT_EQ(0, wkc::safe_numcast<uint8_t>(0L).value());

  EXPECT_EQ(100, wkc::safe_numcast<uint8_t>(100L).value());
  EXPECT_EQ(255, wkc::safe_numcast<uint8_t>(255L).value());

  EXPECT_FALSE(wkc::safe_numcast<uint8_t>(-1).has_value());
  EXPECT_FALSE(wkc::safe_numcast<uint8_t>(256L).has_value());

  // (4) From signed to unsigned, widening/promotion:
  EXPECT_FALSE(
      wkc::safe_numcast<uint16_t>(static_cast<int8_t>(-1)).has_value());
  EXPECT_EQ(0, wkc::safe_numcast<uint16_t>(static_cast<int8_t>(0)).value());
  EXPECT_EQ(127, wkc::safe_numcast<uint16_t>(static_cast<int8_t>(127)).value());

  // (5) From unsigned to signed, narrowing:
  EXPECT_EQ(127, wkc::safe_numcast<int8_t>(static_cast<uint8_t>(127)).value());
  EXPECT_EQ(127, wkc::safe_numcast<int8_t>(static_cast<uint16_t>(127)).value());
  EXPECT_EQ(0, wkc::safe_numcast<int8_t>(static_cast<uint16_t>(0)).value());
  EXPECT_FALSE(
      wkc::safe_numcast<int8_t>(static_cast<uint8_t>(255)).has_value());
  EXPECT_FALSE(
      wkc::safe_numcast<int8_t>(static_cast<uint32_t>(1000)).has_value());
  EXPECT_FALSE(
      wkc::safe_numcast<int16_t>(static_cast<uint32_t>(100000)).has_value());

  // (6) From unsigned to signed, widening/promotion:
  EXPECT_EQ(
      127, wkc::safe_numcast<int16_t>(static_cast<uint16_t>(127)).value());
  EXPECT_EQ(
      1000, wkc::safe_numcast<int32_t>(static_cast<uint16_t>(1000)).value());
  EXPECT_EQ(0, wkc::safe_numcast<int32_t>(static_cast<uint16_t>(0)).value());
  EXPECT_EQ(
      12345L, wkc::safe_numcast<long>(static_cast<uint16_t>(12345)).value());

  // (7) From unsigned to unsigned, narrowing cast:
  EXPECT_EQ(0, wkc::safe_numcast<uint8_t>(0UL).value());
  EXPECT_EQ(100, wkc::safe_numcast<uint8_t>(100UL).value());
  EXPECT_EQ(255, wkc::safe_numcast<uint8_t>(255UL).value());
  EXPECT_FALSE(wkc::safe_numcast<uint8_t>(256UL).has_value());

  // (8) From unsigned to unsigned, widening cast:
  EXPECT_EQ(0L, wkc::safe_numcast<uint64_t>(0).value());
  EXPECT_EQ(100L, wkc::safe_numcast<uint64_t>(100).value());
  const auto uint32_max = std::numeric_limits<uint32_t>::max();
  EXPECT_EQ(static_cast<uint64_t>(uint32_max),
      wkc::safe_numcast<uint64_t>(uint32_max).value());
}

TEST(CastTest, CheckedFloatingPoint) {
  EXPECT_DOUBLE_EQ(5.0, wkc::checked_numcast<double>(5.0F));
  EXPECT_FLOAT_EQ(5.0F, wkc::checked_numcast<float>(5.0));
  EXPECT_DOUBLE_EQ(24.0, wkc::checked_numcast<double>(24.0L));
  EXPECT_FLOAT_EQ(24.0F, wkc::checked_numcast<float>(24.0L));

  // TODO Extend with additional edge cases! (Here & in SafeFloatingPoint)
  using dbl_limits = std::numeric_limits<double>;
  EXPECT_TRUE(std::isnan(wkc::checked_numcast<float>(dbl_limits::quiet_NaN())));
  EXPECT_TRUE(
      std::isnan(wkc::checked_numcast<double>(dbl_limits::quiet_NaN())));
  EXPECT_TRUE(std::isinf(wkc::checked_numcast<float>(dbl_limits::infinity())));
  EXPECT_GT(wkc::checked_numcast<float>(dbl_limits::infinity()), 0.0F);
  EXPECT_LT(wkc::checked_numcast<float>(-dbl_limits::infinity()), 0.0F);

  float flt_val = std::numeric_limits<float>::lowest();
  double dbl_val = static_cast<double>(flt_val);
  EXPECT_FLOAT_EQ(flt_val, wkc::checked_numcast<float>(dbl_val));

  flt_val = std::numeric_limits<float>::max();
  dbl_val = static_cast<double>(flt_val);
  EXPECT_FLOAT_EQ(flt_val, wkc::checked_numcast<float>(dbl_val));

  EXPECT_THROW(
      wkc::checked_numcast<float>(dbl_limits::max()), std::domain_error);
  EXPECT_THROW(
      wkc::checked_numcast<float>(dbl_limits::lowest()), std::domain_error);
}

TEST(CastTest, SafeFloatingPoint) {
  EXPECT_DOUBLE_EQ(5.0, wkc::safe_numcast<double>(5.0F).value());
  EXPECT_FLOAT_EQ(5.0F, wkc::safe_numcast<float>(5.0).value());
  EXPECT_DOUBLE_EQ(24.0, wkc::safe_numcast<double>(24.0L).value());
  EXPECT_FLOAT_EQ(24.0F, wkc::safe_numcast<float>(24.0L).value());

  using dbl_limits = std::numeric_limits<double>;
  EXPECT_TRUE(
      std::isnan(wkc::safe_numcast<float>(dbl_limits::quiet_NaN()).value()));
  EXPECT_TRUE(
      std::isnan(wkc::safe_numcast<double>(dbl_limits::quiet_NaN()).value()));
  EXPECT_TRUE(
      std::isinf(wkc::safe_numcast<float>(dbl_limits::infinity()).value()));
  EXPECT_GT(wkc::safe_numcast<float>(dbl_limits::infinity()).value(), 0.0F);
  EXPECT_LT(wkc::safe_numcast<float>(-dbl_limits::infinity()).value(), 0.0F);

  float flt_val = std::numeric_limits<float>::lowest();
  double dbl_val = static_cast<double>(flt_val);
  EXPECT_FLOAT_EQ(flt_val, wkc::safe_numcast<float>(dbl_val).value());

  flt_val = std::numeric_limits<float>::max();
  dbl_val = static_cast<double>(flt_val);
  EXPECT_FLOAT_EQ(flt_val, wkc::safe_numcast<float>(dbl_val).value());

  EXPECT_FALSE(wkc::safe_numcast<float>(dbl_limits::max()).has_value());
  EXPECT_FALSE(wkc::safe_numcast<float>(dbl_limits::lowest()).has_value());
}

TEST(CastTest, CheckedFloatingToIntegral) {
  // Edge cases:
  // * infinity, NaN
  // * integral wider than float
  // * integral unsigned
  // * cast would require truncating the number
  using limits_dbl = std::numeric_limits<double>;
  EXPECT_THROW(
      wkc::checked_numcast<int>(limits_dbl::quiet_NaN()), std::domain_error);
  EXPECT_THROW(
      wkc::checked_numcast<int>(limits_dbl::infinity()), std::domain_error);

  EXPECT_THROW(wkc::checked_numcast<int8_t>(312.0), std::domain_error);
  EXPECT_EQ(312, wkc::checked_numcast<int16_t>(312.0));

  EXPECT_THROW(wkc::checked_numcast<int8_t>(0.5), std::domain_error);
  EXPECT_EQ(1, wkc::checked_numcast<int8_t>(1.0));
  EXPECT_EQ(-2, wkc::checked_numcast<int8_t>(-2.0));

  EXPECT_THROW(
      wkc::checked_numcast<int32_t>(limits_dbl::max()), std::domain_error);
  EXPECT_THROW(
      wkc::checked_numcast<int32_t>(limits_dbl::lowest()), std::domain_error);

  EXPECT_THROW(wkc::checked_numcast<uint32_t>(0.2), std::domain_error);
  EXPECT_THROW(wkc::checked_numcast<uint32_t>(1e-5), std::domain_error);
  EXPECT_THROW(wkc::checked_numcast<uint32_t>(-1.0), std::domain_error);

  int64_t value = 1L << 40;
  EXPECT_THROW(wkc::checked_numcast<int32_t>(static_cast<double>(value)),
      std::domain_error);
  EXPECT_EQ(value, wkc::checked_numcast<int64_t>(static_cast<double>(value)));

  value = 1L << (std::numeric_limits<uint32_t>::digits - 1);
  EXPECT_THROW(wkc::checked_numcast<int16_t>(static_cast<double>(value)),
      std::domain_error);
  EXPECT_THROW(wkc::checked_numcast<uint16_t>(static_cast<double>(value)),
      std::domain_error);
  EXPECT_THROW(wkc::checked_numcast<int32_t>(static_cast<double>(value)),
      std::domain_error);
  EXPECT_EQ(value, wkc::checked_numcast<uint32_t>(static_cast<double>(value)));
  EXPECT_EQ(value, wkc::checked_numcast<int64_t>(static_cast<double>(value)));
  EXPECT_EQ(value, wkc::checked_numcast<uint64_t>(static_cast<double>(value)));
}

TEST(CastTest, SafeFloatingToIntegral) {
  // Edge cases:
  // * infinity, NaN
  // * integral wider than float
  // * integral unsigned
  // * cast would require truncating the number
  using limits_dbl = std::numeric_limits<double>;
  EXPECT_FALSE(wkc::safe_numcast<int>(limits_dbl::quiet_NaN()).has_value());
  EXPECT_FALSE(wkc::safe_numcast<int>(limits_dbl::infinity()).has_value());

  EXPECT_FALSE(wkc::safe_numcast<int8_t>(312.0).has_value());
  EXPECT_EQ(312, wkc::safe_numcast<int16_t>(312.0).value());

  EXPECT_FALSE(wkc::safe_numcast<int8_t>(0.5).has_value());
  EXPECT_EQ(1, wkc::safe_numcast<int8_t>(1.0).value());
  EXPECT_EQ(-2, wkc::safe_numcast<int8_t>(-2.0).value());

  EXPECT_FALSE(wkc::safe_numcast<int32_t>(limits_dbl::max()).has_value());
  EXPECT_FALSE(wkc::safe_numcast<int32_t>(limits_dbl::lowest()).has_value());

  EXPECT_FALSE(wkc::safe_numcast<uint32_t>(0.2).has_value());
  EXPECT_FALSE(wkc::safe_numcast<uint32_t>(1e-5).has_value());
  EXPECT_FALSE(wkc::safe_numcast<uint32_t>(-1.0).has_value());

  int64_t value = 1L << 40;
  EXPECT_FALSE(
      wkc::safe_numcast<int32_t>(static_cast<double>(value)).has_value());
  EXPECT_EQ(
      value, wkc::safe_numcast<int64_t>(static_cast<double>(value)).value());

  value = 1L << (std::numeric_limits<uint32_t>::digits - 1);
  EXPECT_FALSE(
      wkc::safe_numcast<int16_t>(static_cast<double>(value)).has_value());
  EXPECT_FALSE(
      wkc::safe_numcast<uint16_t>(static_cast<double>(value)).has_value());
  EXPECT_FALSE(
      wkc::safe_numcast<int32_t>(static_cast<double>(value)).has_value());
  EXPECT_EQ(
      value, wkc::safe_numcast<uint32_t>(static_cast<double>(value)).value());
  EXPECT_EQ(
      value, wkc::safe_numcast<int64_t>(static_cast<double>(value)).value());
  EXPECT_EQ(
      value, wkc::safe_numcast<uint64_t>(static_cast<double>(value)).value());
}

// TODO FIXME FIXME FIXME
// [ ] Finish tests
// [ ] Check coverage:
//   https://stackoverflow.com/questions/9666800/getting-useful-gcov-results-for-header-only-libraries
// [ ] Document header

TEST(CastTest, CheckedIntegralToFloating) {
  EXPECT_DOUBLE_EQ(5.0, wkc::checked_numcast<double>(5));
  EXPECT_FLOAT_EQ(-27.0F, wkc::checked_numcast<float>(-27));
  EXPECT_FLOAT_EQ(
      -27.0F, wkc::checked_numcast<float>(static_cast<int8_t>(-27)));

  using lng_limits = std::numeric_limits<int64_t>;
  EXPECT_THROW(
      wkc::checked_numcast<float>(lng_limits::max()), std::domain_error);
  EXPECT_THROW(
      wkc::checked_numcast<float>(lng_limits::max() - 1), std::domain_error);
  EXPECT_THROW(
      wkc::checked_numcast<float>(lng_limits::min() + 1), std::domain_error);
  // Powers of two can be exactly represented:
  EXPECT_EQ(lng_limits::min(),
      wkc::checked_numcast<int64_t>(
          wkc::checked_numcast<float>(lng_limits::min())));

  EXPECT_EQ(1 << 31,
      wkc::checked_numcast<int32_t>(wkc::checked_numcast<float>(1 << 31)));
  int64_t val = 1L << 40;
  EXPECT_EQ(
      val, wkc::checked_numcast<int64_t>(wkc::checked_numcast<float>(val)));
  val = 1L << 50;
  EXPECT_EQ(
      val, wkc::checked_numcast<int64_t>(wkc::checked_numcast<float>(val)));
  val = 1L << 60;
  EXPECT_EQ(
      val, wkc::checked_numcast<int64_t>(wkc::checked_numcast<float>(val)));
  val = 1L << 62;
  EXPECT_EQ(
      val, wkc::checked_numcast<int64_t>(wkc::checked_numcast<float>(val)));
  val = 1L << 63;
  EXPECT_EQ(
      val, wkc::checked_numcast<int64_t>(wkc::checked_numcast<float>(val)));
}

TEST(CastTest, SafeIntegralToFloating) {
  EXPECT_DOUBLE_EQ(5.0, wkc::safe_numcast<double>(5).value());
  EXPECT_FLOAT_EQ(-27.0F, wkc::safe_numcast<float>(-27).value());
  EXPECT_FLOAT_EQ(
      -27.0F, wkc::safe_numcast<float>(static_cast<int8_t>(-27)).value());

  using lng_limits = std::numeric_limits<int64_t>;
  EXPECT_FALSE(wkc::safe_numcast<float>(lng_limits::max()).has_value());
  EXPECT_FALSE(wkc::safe_numcast<float>(lng_limits::max() - 1).has_value());
  EXPECT_FALSE(wkc::safe_numcast<float>(lng_limits::min() + 1).has_value());
  // Powers of two can be exactly represented:
  EXPECT_EQ(lng_limits::min(),
      wkc::safe_numcast<int64_t>(
          wkc::safe_numcast<float>(lng_limits::min()).value())
          .value());

  EXPECT_EQ(1 << 31,
      wkc::safe_numcast<int32_t>(wkc::safe_numcast<float>(1 << 31).value())
          .value());
  int64_t val = 1L << 40;
  EXPECT_EQ(val,
      wkc::safe_numcast<int64_t>(wkc::safe_numcast<float>(val).value())
          .value());
  val = 1L << 50;
  EXPECT_EQ(val,
      wkc::safe_numcast<int64_t>(wkc::safe_numcast<float>(val).value())
          .value());
  val = 1L << 60;
  EXPECT_EQ(val,
      wkc::safe_numcast<int64_t>(wkc::safe_numcast<float>(val).value())
          .value());
  val = 1L << 62;
  EXPECT_EQ(val,
      wkc::safe_numcast<int64_t>(wkc::safe_numcast<float>(val).value())
          .value());
  val = 1L << 63;
  EXPECT_EQ(val,
      wkc::safe_numcast<int64_t>(wkc::safe_numcast<float>(val).value())
          .value());
}

// NOLINTEND
