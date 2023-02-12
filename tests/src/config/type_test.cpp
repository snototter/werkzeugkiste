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
TEST(TypeTest, DateType) {
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

TEST(TypeTest, DateParsing) {
  auto date = wkc::date(2000, 11, 04);
  auto parsed = wkc::date::FromString(date.ToString());
  EXPECT_EQ(date, parsed);

  // Most common format: Y-m-d
  EXPECT_EQ(wkc::date(2023, 02, 28), wkc::date::FromString("2023-02-28"));
  // A trailing delimiter will be ignored
  EXPECT_EQ(wkc::date(2023, 02, 28), wkc::date::FromString("2023-02-28-"));

  EXPECT_THROW(wkc::date::FromString("2023-1"), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString("2023-1-"), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString("2023-1-2--"), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString("-2023-1-2-"), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString("invalid"), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString("invalid-"), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString("Y-m-d"), wkc::ParseError);

  // There are only basic value range checks, but you can still
  // enter invalid dates:
  EXPECT_EQ(wkc::date(1, 2, 3), wkc::date::FromString("1-2-3"));
  EXPECT_EQ(wkc::date(2023, 02, 31), wkc::date::FromString("2023-02-31"));

  EXPECT_THROW(wkc::date::FromString("2023-02-0"), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString("2023-02-32"), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString("2023-13-3"), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString("2023-0-3"), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString("10000-1-3"), wkc::ParseError);

  // We also commonly use: d.m.Y
  EXPECT_EQ(wkc::date(2020, 3, 1), wkc::date::FromString("01.03.2020"));
  // A trailing delimiter will be ignored
  EXPECT_EQ(wkc::date(2020, 3, 1), wkc::date::FromString("01.03.2020."));

  EXPECT_THROW(wkc::date::FromString("1.2."), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString("1.2.2023.."), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString(".1.2.2023."), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString("invalid"), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString("invalid."), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString("d.m.Y"), wkc::ParseError);

  EXPECT_EQ(wkc::date(2023, 02, 31), wkc::date::FromString("31.02.2023"));
  EXPECT_EQ(wkc::date(2023, 12, 3), wkc::date::FromString("3.12.2023"));
  EXPECT_EQ(wkc::date(1, 2, 3), wkc::date::FromString("3.2.1"));

  EXPECT_THROW(wkc::date::FromString("30.1.10000"), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString("30.0.1234"), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString("30.13.1234"), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString("0.2.1234"), wkc::ParseError);
  EXPECT_THROW(wkc::date::FromString("32.2.1234"), wkc::ParseError);
}

TEST(TypeTest, TimeType) {
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

TEST(TypeTest, TimeParsing) {
  auto time = wkc::time{8, 10, 32, 123456789};
  auto parsed = wkc::time::FromString(time.ToString());
  EXPECT_EQ(time, parsed);

  EXPECT_EQ(wkc::time(10, 11), wkc::time::FromString("10:11"));
  EXPECT_EQ(wkc::time(10, 11, 12), wkc::time::FromString("10:11:12"));

  // White space is allowed
  EXPECT_EQ(wkc::time(10, 11, 12), wkc::time::FromString(" 10:11:12"));
  EXPECT_EQ(wkc::time(10, 11, 12), wkc::time::FromString(" 10:11:12 "));
  EXPECT_EQ(wkc::time(10, 11, 12), wkc::time::FromString(" 10: 11:12"));

  // Sub-second component must explicitly contain either 3 (ms), 6 (us)
  // or 9 (ns) digits.
  EXPECT_THROW(wkc::time::FromString("10:11:12.1"), wkc::ParseError);
  EXPECT_THROW(wkc::time::FromString("10:11:12.12"), wkc::ParseError);
  EXPECT_THROW(wkc::time::FromString("10:11:12.1234"), wkc::ParseError);
  EXPECT_THROW(wkc::time::FromString("10:11:12.12345"), wkc::ParseError);
  EXPECT_THROW(wkc::time::FromString("10:11:12.1234567"), wkc::ParseError);
  EXPECT_THROW(wkc::time::FromString("10:11:12.12345678"), wkc::ParseError);

  EXPECT_EQ(wkc::time(10, 11, 12, 1000000),
            wkc::time::FromString("10:11:12.001"));
  EXPECT_EQ(wkc::time(10, 11, 12, 1002000),
            wkc::time::FromString("10:11:12.001002"));
  EXPECT_EQ(wkc::time(10, 11, 12, 1002003),
            wkc::time::FromString("10:11:12.001002003"));

  // Parsing checks the value ranges:
  EXPECT_THROW(wkc::time::FromString("-1:00"), wkc::ParseError);
  EXPECT_THROW(wkc::time::FromString("24:00"), wkc::ParseError);
  EXPECT_THROW(wkc::time::FromString("00:-1"), wkc::ParseError);
  EXPECT_THROW(wkc::time::FromString("00:60"), wkc::ParseError);
  EXPECT_THROW(wkc::time::FromString("00:01:-1"), wkc::ParseError);
  EXPECT_THROW(wkc::time::FromString("00:01:60"), wkc::ParseError);
  EXPECT_THROW(wkc::time::FromString("00:01:02.-12"), wkc::ParseError);
  EXPECT_THROW(wkc::time::FromString("00:01:02.1234567890"), wkc::ParseError);

  // Further invalid inputs:
  EXPECT_THROW(wkc::time::FromString("10:11:12:123"), wkc::ParseError);
  EXPECT_THROW(wkc::time::FromString("10:11::12"), wkc::ParseError);
  EXPECT_THROW(wkc::time::FromString(":10:11:12"), wkc::ParseError);
  EXPECT_THROW(wkc::time::FromString("10:11.12"), wkc::ParseError);
  EXPECT_THROW(wkc::time::FromString("10:11:12."), wkc::ParseError);
  EXPECT_THROW(wkc::time::FromString("now"), wkc::ParseError);
  EXPECT_THROW(wkc::time::FromString("invalid:input"), wkc::ParseError);
}

// NOLINTEND
