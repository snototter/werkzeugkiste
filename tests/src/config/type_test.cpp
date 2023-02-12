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
TEST(ConfigTest, DateType) {
  // Check that the `date` type is implemented correctly
  auto date = wkc::date(2000, 11, 04);
  auto tpl_date = date.ToTuple();
  EXPECT_EQ(date.year, std::get<0>(tpl_date));
  EXPECT_EQ(date.month, std::get<1>(tpl_date));
  EXPECT_EQ(date.day, std::get<2>(tpl_date));

  EXPECT_LT(wkc::date(2000, 10, 20), wkc::date(2020, 1, 21));
  EXPECT_LT(wkc::date(2000, 10, 20), wkc::date(2000, 11, 21));
  EXPECT_LT(wkc::date(2000, 10, 20), wkc::date(2000, 10, 21));

  EXPECT_LE(wkc::date(2000, 10, 20), wkc::date(2000, 10, 21));

  EXPECT_NE(wkc::date(2000, 10, 20), wkc::date(2000, 10, 21));
  EXPECT_EQ(wkc::date(2000, 10, 20), wkc::date(2000, 10, 20));

  EXPECT_FALSE(wkc::date(2000, 10, 20) < wkc::date(2000, 10, 20));
  EXPECT_FALSE(wkc::date(2000, 10, 20) > wkc::date(2000, 10, 20));
  EXPECT_FALSE(wkc::date(2000, 10, 20) != wkc::date(2000, 10, 20));

  EXPECT_LE(wkc::date(2000, 10, 20), wkc::date(2000, 10, 20));
  EXPECT_GE(wkc::date(2000, 10, 20), wkc::date(2000, 10, 20));

  EXPECT_GT(wkc::date(2000, 10, 21), wkc::date(2000, 10, 20));
  EXPECT_GT(wkc::date(2000, 11, 04), wkc::date(2000, 10, 20));
  EXPECT_GT(wkc::date(2001, 1, 01), wkc::date(2000, 10, 20));
  EXPECT_GE(wkc::date(2001, 1, 01), wkc::date(2000, 10, 20));

  EXPECT_EQ("2000-11-04", wkc::date(2000, 11, 04).ToString());

  std::ostringstream str;
  str << wkc::date{2000, 11, 04};
  EXPECT_EQ("2000-11-04", str.str());
}

TEST(ConfigTest, DateParsing) {
  auto date = wkc::date(2000, 11, 04);
  auto parsed = wkc::date::FromString(date.ToString());
  EXPECT_EQ(date, parsed);

  // Most common format: Y-m-d
  EXPECT_EQ(wkc::date(2023, 02, 28), wkc::date::FromString("2023-02-28"));
  EXPECT_THROW(wkc::date::FromString("2023-1"), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString("2023-1-"), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString("2023-1-2-"),
               wkc::ParseError);  // TODO check tokenizer
  EXPECT_THROW(wkc::date::FromString("-2023-1-2-"), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString("invalid"), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString("invalid-"), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString("Y-m-d"), wkc::ParseError);
  // There are no checks for valid dates:
  EXPECT_EQ(wkc::date(2023, 02, 33), wkc::date::FromString("2023-02-33"));
  EXPECT_EQ(wkc::date(2023, 99, 3), wkc::date::FromString("2023-99-3"));
  EXPECT_EQ(wkc::date(1, 2, 3), wkc::date::FromString("1-2-3"));
  // But the value types imply constraints:
  EXPECT_THROW(wkc::date::FromString("1234567-1-30"), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString("1234-256-30"), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString("1234-2-266"), wkc::ParseError);

  // We also commonly use: d.m.Y
  EXPECT_EQ(wkc::date(2020, 3, 1), wkc::date::FromString("01.03.2020"));
  EXPECT_THROW(wkc::date::FromString("1.2."), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString("1.2.2023."), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString(".1.2.2023."), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString("invalid"), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString("invalid."), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString("d.m.Y"), wkc::ParseError);

  EXPECT_EQ(wkc::date(2023, 02, 33), wkc::date::FromString("33.02.2023"));
  EXPECT_EQ(wkc::date(2023, 99, 3), wkc::date::FromString("3.99.2023"));
  EXPECT_EQ(wkc::date(1, 2, 3), wkc::date::FromString("3.2.1"));

  EXPECT_THROW(wkc::date::FromString("30.1.1234567"), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString("30.256.1234"), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString("260.2.1234"), wkc::ParseError);
}

TEST(ConfigTest, TimeType) {
  // Check that the `time` type is implemented correctly
  auto time = wkc::time{23, 49, 30, 987654321};
  auto tpl_time = time.ToTuple();
  EXPECT_EQ(time.hour, std::get<0>(tpl_time));
  EXPECT_EQ(time.minute, std::get<1>(tpl_time));
  EXPECT_EQ(time.second, std::get<2>(tpl_time));
  EXPECT_EQ(time.nanosecond, std::get<3>(tpl_time));

  EXPECT_EQ("23:49:30.987654321", time.ToString());
  std::ostringstream str;
  str << time;
  EXPECT_EQ("23:49:30.987654321", str.str());

  EXPECT_LE(wkc::time(8, 10, 22), wkc::time(8, 10, 22, 1));
  EXPECT_LT(wkc::time(8, 10, 22), wkc::time(8, 10, 22, 1));
  EXPECT_LT(wkc::time(8, 10, 22, 1), wkc::time(8, 10, 22, 2));
  EXPECT_LT(wkc::time(8, 10, 22), wkc::time(8, 10, 23));
  EXPECT_LT(wkc::time(8, 10, 22, 1), wkc::time(8, 11, 22));
  EXPECT_LT(wkc::time(8, 10, 22, 1), wkc::time(10, 10, 22));

  EXPECT_NE(wkc::time(10, 11, 12, 999888777), wkc::time(10, 11, 12, 999888776));
  EXPECT_EQ(wkc::time(10, 11, 12, 999888777), wkc::time(10, 11, 12, 999888777));

  EXPECT_LE(wkc::time(10, 11, 12, 999888777), wkc::time(10, 11, 12, 999888777));
  EXPECT_GE(wkc::time(10, 11, 12, 999888777), wkc::time(10, 11, 12, 999888777));

  EXPECT_FALSE(wkc::time(10, 11, 12, 999888777) <
               wkc::time(10, 11, 12, 999888777));
  EXPECT_FALSE(wkc::time(10, 11, 12, 999888777) >
               wkc::time(10, 11, 12, 999888777));
  EXPECT_FALSE(wkc::time(10, 11, 12, 999888777) !=
               wkc::time(10, 11, 12, 999888777));

  EXPECT_GT(wkc::time(12, 10, 2, 1), wkc::time(12, 10, 2));
}

// NOLINTEND
