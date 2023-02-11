#include <werkzeugkiste/config/casts.h>

#include <cmath>
#include <exception>
#include <limits>
#include <sstream>

#include "../test_utils.h"

namespace wkc = werkzeugkiste::config;

// NOLINTBEGIN
TEST(ConfigTest, Static) {
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

TEST(ConfigTest, Boolean) {
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

TEST(ConfigTest, Integral) {
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

// NOLINTEND
