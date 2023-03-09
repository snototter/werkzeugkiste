#include <werkzeugkiste/config/casts.h>

#include <cmath>
#include <limits>

#include "../test_utils.h"

namespace wkc = werkzeugkiste::config;

// NOLINTBEGIN

// TODO FIXME
// [ ] Check coverage:
//   https://stackoverflow.com/questions/9666800/getting-useful-gcov-results-for-header-only-libraries
// [ ] Use CheckSafeCast to simplify remaining test cases.

template <typename T, typename S>
void CheckSafeCast(std::string_view lbl,
    S value,
    bool should_be_representable) {
  EXPECT_NO_THROW(wkc::safe_numcast<T>(value))
      << "Cannot cast " << wkc::TypeName<S>() << ' ' << value << " to "
      << wkc::TypeName<T>() << ". Test: " << lbl;

  const auto opt_tgt = wkc::safe_numcast<T>(value);
  if (opt_tgt.has_value()) {
    EXPECT_NO_THROW(wkc::safe_numcast<S>(opt_tgt.value()))
        << "Cannot cast " << wkc::TypeName<T>() << ' ' << opt_tgt.value()
        << " back to " << wkc::TypeName<S>()
        << " as it would throw! Test: " << lbl;
    const auto opt_src = wkc::safe_numcast<S>(opt_tgt.value());
    EXPECT_TRUE(opt_src.has_value())
        << "Cannot cast " << wkc::TypeName<T>() << ' ' << opt_tgt.value()
        << " back to " << wkc::TypeName<S>() << ". Test: " << lbl;
    if (opt_src.has_value()) {
      if constexpr (std::is_same_v<S, float>) {
        EXPECT_FLOAT_EQ(value, opt_src.value());
      } else if constexpr (std::is_floating_point_v<S>) {
        EXPECT_DOUBLE_EQ(value, opt_src.value());
      } else {
        EXPECT_EQ(value, opt_src.value());
      }
    }
  }

  EXPECT_EQ(should_be_representable, opt_tgt.has_value())
      << "safe_numcast didn't work as expected for " << wkc::TypeName<S>()
      << ' ' << value << " to " << wkc::TypeName<T>() << ". Test: " << lbl;
}

// template <typename T, typename S>
// void CheckCheckedCast(S value, bool should_be_representable) {
//   if (should_be_representable) {
//     // TODO
//   } else {
//     EXPECT_THROW(wkc::checked_numcast<T>(value), std::domain_error)
//       << "checked_numcast didn't throw for " << wkc::TypeName<S>()
//       << ' ' << value << " to " << wkc::TypeName<T>();
//   }
// }

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
  CheckSafeCast<int8_t>("b2int", true, true);
  CheckSafeCast<int8_t>("b2int", true, true);
  CheckSafeCast<int16_t>("b2int", true, true);
  CheckSafeCast<uint16_t>("b2int", true, true);
  CheckSafeCast<int32_t>("b2int", true, true);

  CheckSafeCast<int8_t>("b2int", false, true);
  CheckSafeCast<int8_t>("b2int", false, true);
  CheckSafeCast<int16_t>("b2int", false, true);
  CheckSafeCast<uint16_t>("b2int", false, true);
  CheckSafeCast<int32_t>("b2int", false, true);

  // From bool to float:
  CheckSafeCast<float>("b2flt", true, true);
  CheckSafeCast<double>("b2flt", true, true);
  CheckSafeCast<float>("b2flt", false, true);
  CheckSafeCast<double>("b2flt", false, true);

  // From bool to bool:
  CheckSafeCast<bool>("b2b", true, true);
  CheckSafeCast<bool>("b2b", false, true);

  // From integral (signed/unsigned) to bool:
  CheckSafeCast<bool>("int2b", 0, true);
  CheckSafeCast<bool>("int2b", 1, true);
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
  CheckSafeCast<int8_t>("0", int8_t{0}, true);
  CheckSafeCast<int8_t>("0", int16_t{0}, true);
  CheckSafeCast<int8_t>("0", int32_t{0}, true);
  CheckSafeCast<int8_t>("0", int64_t{0}, true);

  const auto int8_min = std::numeric_limits<int8_t>::min();
  auto val_i32 = static_cast<int32_t>(int8_min);
  CheckSafeCast<int8_t>("int8::min", val_i32, true);
  ++val_i32;
  CheckSafeCast<int8_t>("int8::min+1", val_i32, true);
  val_i32 -= 2;
  CheckSafeCast<int8_t>("int8::min-1", val_i32, false);

  const auto int8_max = std::numeric_limits<int8_t>::max();
  val_i32 = static_cast<int32_t>(int8_max);
  CheckSafeCast<int8_t>("int8::max", val_i32, true);
  --val_i32;
  CheckSafeCast<int8_t>("int8::max-1", val_i32, true);
  val_i32 += 2;
  CheckSafeCast<int8_t>("int8::max+1", val_i32, false);

  // (2) From signed to signed, widening/promotion:
  CheckSafeCast<int64_t>("0", int8_t{0}, true);
  CheckSafeCast<int64_t>("0", int16_t{0}, true);
  CheckSafeCast<int64_t>("0", int32_t{0}, true);
  CheckSafeCast<int64_t>("0", int64_t{0}, true);

  const auto int32_min = std::numeric_limits<int32_t>::min();
  CheckSafeCast<int64_t>("int32::min", int32_min, true);

  const auto int32_max = std::numeric_limits<int32_t>::max();
  CheckSafeCast<int64_t>("int32::max", int32_max, true);

  // (3) From signed to unsigned, narrowing:
  CheckSafeCast<uint8_t>("narrow 0L", 0L, true);
  CheckSafeCast<uint8_t>("narrow 100L", 100L, true);
  CheckSafeCast<uint8_t>("narrow 255L", 255L, true);

  CheckSafeCast<uint8_t>("narrow -1", -1, false);
  CheckSafeCast<uint8_t>("narrow 256", 256, false);
  CheckSafeCast<uint8_t>("narrow 256L", 256L, false);

  // (4) From signed to unsigned, widening/promotion:
  CheckSafeCast<uint16_t>("widen -1", int8_t{-1}, false);
  CheckSafeCast<uint16_t>("widen 0", int8_t{0}, true);
  CheckSafeCast<uint16_t>("widen 127", int8_t{127}, true);

  // (5) From unsigned to signed, narrowing:
  CheckSafeCast<int8_t>("narrow 127", uint8_t{127}, true);
  CheckSafeCast<int8_t>("narrow 127", uint16_t{127}, true);
  CheckSafeCast<int8_t>("narrow 0", uint16_t{0}, true);
  CheckSafeCast<int8_t>("narrow 255", uint8_t{255}, false);
  CheckSafeCast<int8_t>("narrow 1000", uint32_t{1000}, false);
  CheckSafeCast<int16_t>("narrow 255", uint16_t{255}, true);
  CheckSafeCast<int16_t>("narrow 255", uint32_t{255}, true);
  CheckSafeCast<int16_t>("narrow 1000", uint16_t{1000}, true);
  CheckSafeCast<int16_t>("narrow 100000", uint32_t{100000}, false);

  // (6) From unsigned to signed, widening/promotion:
  CheckSafeCast<int16_t>("widen 127", uint16_t{127}, true);
  CheckSafeCast<int32_t>("widen 0", uint16_t{0}, true);
  CheckSafeCast<int32_t>("widen 1000", uint16_t{1000}, true);
  CheckSafeCast<int64_t>("widen 12345", uint16_t{12345}, true);

  // (7) From unsigned to unsigned, narrowing cast:
  CheckSafeCast<uint8_t>("narrow 0", 0UL, true);
  CheckSafeCast<uint8_t>("narrow 100", 100UL, true);
  CheckSafeCast<uint8_t>("narrow 255", 255UL, true);
  CheckSafeCast<uint8_t>("narrow 256", 256UL, false);

  // (8) From unsigned to unsigned, widening cast:
  CheckSafeCast<uint64_t>("widen 0", 0U, true);
  CheckSafeCast<uint64_t>("widen 100", 100U, true);
  const auto uint32_max = std::numeric_limits<uint32_t>::max();
  CheckSafeCast<uint64_t>("widen uint32::max", uint32_max, true);
  CheckSafeCast<uint32_t>("widen uint32::max", uint32_max, true);
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
  CheckSafeCast<float>("lng::max", lng_limits::max(), false);
  CheckSafeCast<double>("lng::max", lng_limits::max(), false);

  CheckSafeCast<float>("lng::max-1", lng_limits::max() - 1, false);
  CheckSafeCast<double>("lng::max-1", lng_limits::max() - 1, false);

  CheckSafeCast<float>("lng::min+1", lng_limits::min() + 1, false);
  CheckSafeCast<double>("lng::min+1", lng_limits::min() + 1, false);

  // Powers of two can be exactly represented:
  CheckSafeCast<float>("lng::min", lng_limits::min(), true);
  CheckSafeCast<double>("lng::min", lng_limits::min(), true);

  CheckSafeCast<float>("1UL<<63", 1UL << 63, true);
  CheckSafeCast<double>("1UL<<63", 1UL << 63, true);

  uint32_t u32 = 1U << 31;
  CheckSafeCast<float>("1U<<31", u32, true);
  CheckSafeCast<double>("1U<<31", u32, true);

  int32_t i32 = 1 << 31;
  CheckSafeCast<float>("1<<31", i32, true);
  CheckSafeCast<double>("1<<31", i32, true);

  int64_t val = 1L << 40;
  CheckSafeCast<float>("1L<<40", val, true);
  CheckSafeCast<double>("1L<<40", val, true);

  val = 1L << 50;
  CheckSafeCast<float>("1L<<50", val, true);
  CheckSafeCast<double>("1L<<50", val, true);

  val = 1L << 60;
  CheckSafeCast<float>("1L<<60", val, true);
  CheckSafeCast<double>("1L<<60", val, true);

  val = 1L << 62;
  CheckSafeCast<float>("1L<<62", val, true);
  CheckSafeCast<double>("1L<<62", val, true);

  val = 1L << 63;
  CheckSafeCast<float>("1L<<63", val, true);
  CheckSafeCast<double>("1L<<63", val, true);

  --val;
  CheckSafeCast<float>("(1L<<63)-1", val, false);
  CheckSafeCast<double>("(1L<<63)-1", val, false);
}

// NOLINTEND
