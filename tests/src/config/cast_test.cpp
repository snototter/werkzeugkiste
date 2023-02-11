#include <werkzeugkiste/config/casts.h>
#include <werkzeugkiste/strings/strings.h>

#include <cmath>
#include <exception>
#include <limits>
#include <sstream>

#include "../test_utils.h"

namespace wkc = werkzeugkiste::config;
namespace wks = werkzeugkiste::strings;

// NOLINTBEGIN
TEST(CastTest, Static) {
  static_assert(wkc::are_integral_v<int, int16_t>);
  static_assert(wkc::are_integral_v<unsigned int, int16_t>);
  static_assert(wkc::are_integral_v<int, bool>);

  static_assert(!wkc::are_integral_v<float, bool>);
  static_assert(!wkc::are_integral_v<int, float>);
  static_assert(!wkc::are_integral_v<int, double>);
  static_assert(!wkc::are_integral_v<std::string, short>);

  static_assert(wkc::are_floating_point_v<float, float>);
  static_assert(wkc::are_floating_point_v<float, double>);
  static_assert(wkc::are_floating_point_v<double, long double>);

  static_assert(!wkc::are_floating_point_v<float, int>);
  static_assert(!wkc::are_floating_point_v<int, float>);
  static_assert(!wkc::are_floating_point_v<std::string, float>);

  static_assert(wkc::IsPromotable<int, long>());
  static_assert(wkc::IsPromotable<char, int>());
  static_assert(wkc::IsPromotable<float, double>());
  static_assert(!wkc::IsPromotable<int, char>());
  static_assert(!wkc::IsPromotable<uint, int>());
}

TEST(CastTest, Boolean) {
  // From bool to integral (signed/unsigned):
  EXPECT_EQ(1, wkc::CheckedCast<int>(true));
  EXPECT_EQ(1, wkc::CheckedCast<int8_t>(true));
  EXPECT_EQ(1, wkc::CheckedCast<uint8_t>(true));
  EXPECT_EQ(1, wkc::CheckedCast<int16_t>(true));

  EXPECT_EQ(0, wkc::CheckedCast<int>(false));
  EXPECT_EQ(0, wkc::CheckedCast<int8_t>(false));
  EXPECT_EQ(0, wkc::CheckedCast<uint8_t>(false));
  EXPECT_EQ(0, wkc::CheckedCast<int16_t>(false));

  // From bool to bool:
  EXPECT_EQ(false, wkc::CheckedCast<bool>(false));
  EXPECT_EQ(true, wkc::CheckedCast<bool>(true));

  // From integral (signed/unsigned) to bool:
  EXPECT_EQ(true, wkc::CheckedCast<bool>(1));
  EXPECT_EQ(true, wkc::CheckedCast<bool>(2));
  EXPECT_EQ(true, wkc::CheckedCast<bool>(-1));
  EXPECT_EQ(true, wkc::CheckedCast<bool>(-42));
}

TEST(CastTest, Integral) {
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
  EXPECT_EQ(static_cast<int8_t>(0), wkc::CheckedCast<int8_t>(0));

  const auto int8_min = std::numeric_limits<int8_t>::min();
  EXPECT_EQ(int8_min, wkc::CheckedCast<int8_t>(static_cast<int32_t>(int8_min)));
  EXPECT_EQ(int8_min + 1,
            wkc::CheckedCast<int8_t>(static_cast<int32_t>(int8_min) + 1));
  EXPECT_THROW(wkc::CheckedCast<int8_t>(static_cast<int32_t>(int8_min) - 1),
               std::domain_error);

  const auto int8_max = std::numeric_limits<int8_t>::max();
  EXPECT_EQ(int8_max, wkc::CheckedCast<int8_t>(static_cast<int32_t>(int8_max)));
  EXPECT_EQ(int8_max - 1,
            wkc::CheckedCast<int8_t>(static_cast<int32_t>(int8_max) - 1));
  EXPECT_THROW(wkc::CheckedCast<int8_t>(static_cast<int32_t>(int8_max) + 1),
               std::domain_error);

  // (2) From signed to signed, widening/promotion:
  EXPECT_EQ(0L, wkc::CheckedCast<int64_t>(0));

  const auto int32_min = std::numeric_limits<int32_t>::min();
  EXPECT_EQ(static_cast<int64_t>(int32_min),
            wkc::CheckedCast<int64_t>(int32_min));

  const auto int32_max = std::numeric_limits<int32_t>::max();
  EXPECT_EQ(static_cast<int64_t>(int32_max),
            wkc::CheckedCast<int64_t>(int32_max));

  // (3) From signed to unsigned, narrowing:
  EXPECT_EQ(0, wkc::CheckedCast<uint8_t>(0L));

  EXPECT_EQ(100, wkc::CheckedCast<uint8_t>(100L));
  EXPECT_EQ(255, wkc::CheckedCast<uint8_t>(255L));

  EXPECT_THROW(wkc::CheckedCast<uint8_t>(-1), std::domain_error);
  EXPECT_THROW(wkc::CheckedCast<uint8_t>(256L), std::domain_error);

  // (4) From signed to unsigned, widening/promotion:
  EXPECT_THROW(wkc::CheckedCast<uint16_t>(static_cast<int8_t>(-1)),
               std::domain_error);
  EXPECT_EQ(0, wkc::CheckedCast<uint16_t>(static_cast<int8_t>(0)));
  EXPECT_EQ(127, wkc::CheckedCast<uint16_t>(static_cast<int8_t>(127)));

  // (5) From unsigned to signed, narrowing:
  // TODO

  // (6) From unsigned to signed, widening/promotion:
  // TODO

  // (7) From unsigned to unsigned, narrowing cast:
  EXPECT_EQ(0, wkc::CheckedCast<uint8_t>(0UL));
  EXPECT_EQ(100, wkc::CheckedCast<uint8_t>(100UL));
  EXPECT_EQ(255, wkc::CheckedCast<uint8_t>(255UL));
  EXPECT_THROW(wkc::CheckedCast<uint8_t>(256UL), std::domain_error);

  // (8) From unsigned to unsigned, widening cast:
  EXPECT_EQ(0L, wkc::CheckedCast<uint64_t>(0));
  EXPECT_EQ(100L, wkc::CheckedCast<uint64_t>(100));
  const auto uint32_max = std::numeric_limits<uint32_t>::max();
  EXPECT_EQ(static_cast<uint64_t>(uint32_max),
            wkc::CheckedCast<uint64_t>(uint32_max));
}

TEST(CastTest, FloatingPoint) {
  EXPECT_DOUBLE_EQ(5.0, wkc::CheckedCast<double>(5.0F));
  EXPECT_DOUBLE_EQ(5.0F, wkc::CheckedCast<float>(5.0));
  EXPECT_DOUBLE_EQ(24.0, wkc::CheckedCast<double>(24.0L));
  EXPECT_DOUBLE_EQ(24.0F, wkc::CheckedCast<float>(24.0L));

  // TODO Extend with edge cases!
}

TEST(CastTest, FloatingToIntegral) {
  // Edge cases:
  // * infinity, NaN
  // * integral wider than float
  // * integral unsigned
  // * cast would require truncating the number
  using limits_dbl = std::numeric_limits<double>;
  EXPECT_THROW(wkc::CheckedCast<int>(limits_dbl::quiet_NaN()),
               std::domain_error);
  EXPECT_THROW(wkc::CheckedCast<int>(limits_dbl::infinity()),
               std::domain_error);

  EXPECT_THROW(wkc::CheckedCast<int8_t>(312.0), std::domain_error);
  EXPECT_EQ(312, wkc::CheckedCast<int16_t>(312.0));

  EXPECT_THROW(wkc::CheckedCast<int8_t>(0.5), std::domain_error);
  EXPECT_EQ(1, wkc::CheckedCast<int8_t>(1.0));
  EXPECT_EQ(-2, wkc::CheckedCast<int8_t>(-2.0));

  EXPECT_THROW(wkc::CheckedCast<int32_t>(limits_dbl::max()), std::domain_error);
  EXPECT_THROW(wkc::CheckedCast<int32_t>(limits_dbl::lowest()),
               std::domain_error);

  EXPECT_THROW(wkc::CheckedCast<uint32_t>(0.2), std::domain_error);
  EXPECT_THROW(wkc::CheckedCast<uint32_t>(1e-5), std::domain_error);
  EXPECT_THROW(wkc::CheckedCast<uint32_t>(-1.0), std::domain_error);

  int64_t value = 1L << 40;
  EXPECT_THROW(wkc::CheckedCast<int32_t>(static_cast<double>(value)),
               std::domain_error);
  EXPECT_EQ(value, wkc::CheckedCast<int64_t>(static_cast<double>(value)));

  value = 1L << (std::numeric_limits<uint32_t>::digits - 1);
  EXPECT_THROW(wkc::CheckedCast<int16_t>(static_cast<double>(value)),
               std::domain_error);
  EXPECT_THROW(wkc::CheckedCast<uint16_t>(static_cast<double>(value)),
               std::domain_error);
  EXPECT_THROW(wkc::CheckedCast<int32_t>(static_cast<double>(value)),
               std::domain_error);
  EXPECT_EQ(value, wkc::CheckedCast<uint32_t>(static_cast<double>(value)));
  EXPECT_EQ(value, wkc::CheckedCast<int64_t>(static_cast<double>(value)));
  EXPECT_EQ(value, wkc::CheckedCast<uint64_t>(static_cast<double>(value)));
}

TEST(CastTest, IntegralToFloating) {
  EXPECT_DOUBLE_EQ(5.0, wkc::CheckedCast<double>(5));
  EXPECT_DOUBLE_EQ(-27.0F, wkc::CheckedCast<float>(-27));
  EXPECT_DOUBLE_EQ(-27.0F, wkc::CheckedCast<float>(static_cast<int8_t>(-27)));

  using lng_limits = std::numeric_limits<int64_t>;
  EXPECT_THROW(wkc::CheckedCast<float>(lng_limits::max()), std::domain_error);
  EXPECT_THROW(wkc::CheckedCast<float>(lng_limits::max() - 1),
               std::domain_error);
  EXPECT_THROW(wkc::CheckedCast<float>(lng_limits::min() + 1),
               std::domain_error);
  // Powers of two can be exactly represented:
  EXPECT_EQ(lng_limits::min(), wkc::CheckedCast<int64_t>(
                                   wkc::CheckedCast<float>(lng_limits::min())));

  EXPECT_EQ(1 << 31,
            wkc::CheckedCast<int32_t>(wkc::CheckedCast<float>(1 << 31)));
  EXPECT_EQ(1L << 40,
            wkc::CheckedCast<int64_t>(wkc::CheckedCast<float>(1L << 40)));
  EXPECT_EQ(1L << 50,
            wkc::CheckedCast<int64_t>(wkc::CheckedCast<float>(1L << 50)));
  EXPECT_EQ(1L << 60,
            wkc::CheckedCast<int64_t>(wkc::CheckedCast<float>(1L << 60)));
  EXPECT_EQ(1L << 62,
            wkc::CheckedCast<int64_t>(wkc::CheckedCast<float>(1L << 62)));
  EXPECT_EQ(1L << 63,
            wkc::CheckedCast<int64_t>(wkc::CheckedCast<float>(1L << 63)));
}

// NOLINTEND
