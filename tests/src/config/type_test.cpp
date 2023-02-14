#include <werkzeugkiste/config/casts.h>
#include <werkzeugkiste/strings/strings.h>

#include <cmath>
#include <exception>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>

#include "../test_utils.h"

namespace wkc = werkzeugkiste::config;
namespace wks = werkzeugkiste::strings;
using namespace std::string_view_literals;

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
  auto parsed = wkc::date(date.ToString());
  EXPECT_EQ(date, parsed);

  // Most common format: Y-m-d
  EXPECT_EQ(wkc::date(2023, 02, 28), wkc::date("2023-02-28"sv));
  // A trailing delimiter will be ignored
  EXPECT_EQ(wkc::date(2023, 02, 28), wkc::date("2023-02-28-"sv));

  EXPECT_THROW(wkc::date("2023-1"sv), wkc::ParseError);
  EXPECT_THROW(wkc::date("2023-1-"sv), wkc::ParseError);
  EXPECT_THROW(wkc::date("2023-1-2--"sv), wkc::ParseError);
  EXPECT_THROW(wkc::date("-2023-1-2-"sv), wkc::ParseError);
  EXPECT_THROW(wkc::date("invalid"sv), wkc::ParseError);
  EXPECT_THROW(wkc::date("invalid-"sv), wkc::ParseError);
  EXPECT_THROW(wkc::date("Y-m-d"sv), wkc::ParseError);

  // There are only basic value range checks, but you can still
  // enter invalid dates:
  EXPECT_EQ(wkc::date(1, 2, 3), wkc::date("1-2-3"sv));
  EXPECT_EQ(wkc::date(2023, 02, 31), wkc::date("2023-02-31"sv));

  EXPECT_THROW(wkc::date("2023-02-0"sv), wkc::ParseError);
  EXPECT_THROW(wkc::date("2023-02-32"sv), wkc::ParseError);
  EXPECT_THROW(wkc::date("2023-13-3"sv), wkc::ParseError);
  EXPECT_THROW(wkc::date("2023-0-3"sv), wkc::ParseError);
  EXPECT_THROW(wkc::date("10000-1-3"sv), wkc::ParseError);

  // We also commonly use: d.m.Y
  EXPECT_EQ(wkc::date(2020, 3, 1), wkc::date("01.03.2020"sv));
  // A trailing delimiter will be ignored
  EXPECT_EQ(wkc::date(2020, 3, 1), wkc::date("01.03.2020."sv));

  EXPECT_THROW(wkc::date("1.2."), wkc::ParseError);
  EXPECT_THROW(wkc::date("1.2.2023.."), wkc::ParseError);
  EXPECT_THROW(wkc::date(".1.2.2023."), wkc::ParseError);
  EXPECT_THROW(wkc::date("invalid"), wkc::ParseError);
  EXPECT_THROW(wkc::date("invalid."), wkc::ParseError);
  EXPECT_THROW(wkc::date("d.m.Y"), wkc::ParseError);

  EXPECT_EQ(wkc::date(2023, 02, 31), wkc::date("31.02.2023"));
  EXPECT_EQ(wkc::date(2023, 12, 3), wkc::date("3.12.2023"));
  EXPECT_EQ(wkc::date(1, 2, 3), wkc::date("3.2.1"));

  EXPECT_THROW(wkc::date("30.1.10000"), wkc::ParseError);
  EXPECT_THROW(wkc::date("30.0.1234"), wkc::ParseError);
  EXPECT_THROW(wkc::date("30.13.1234"), wkc::ParseError);
  EXPECT_THROW(wkc::date("0.2.1234"), wkc::ParseError);
  EXPECT_THROW(wkc::date("32.2.1234"), wkc::ParseError);
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
  auto parsed = wkc::time{time.ToString()};
  EXPECT_EQ(time, parsed);

  EXPECT_EQ(wkc::time(10, 11), wkc::time("10:11"sv));
  EXPECT_EQ(wkc::time(10, 11, 12), wkc::time("10:11:12"sv));

  // White space is allowed
  EXPECT_EQ(wkc::time(10, 11, 12), wkc::time(" 10:11:12"sv));
  EXPECT_EQ(wkc::time(10, 11, 12), wkc::time(" 10:11:12 "sv));
  EXPECT_EQ(wkc::time(10, 11, 12), wkc::time(" 10: 11:12"sv));

  // Sub-second component must explicitly contain either 3 (ms), 6 (us)
  // or 9 (ns) digits.
  EXPECT_THROW(wkc::time("10:11:12.1"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time("10:11:12.12"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time("10:11:12.1234"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time("10:11:12.12345"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time("10:11:12.1234567"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time("10:11:12.12345678"sv), wkc::ParseError);

  EXPECT_EQ(wkc::time(10, 11, 12, 1000000), wkc::time("10:11:12.001"sv));
  EXPECT_EQ(wkc::time(10, 11, 12, 1002000), wkc::time("10:11:12.001002"sv));
  EXPECT_EQ(wkc::time(10, 11, 12, 1002003), wkc::time("10:11:12.001002003"sv));

  // Parsing checks the value ranges:
  EXPECT_THROW(wkc::time("-1:00"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time("24:00"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time("00:-1"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time("00:60"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time("00:01:-1"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time("00:01:60"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time("00:01:02.-12"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time("00:01:02.1234567890"sv), wkc::ParseError);

  // Further invalid inputs:
  EXPECT_THROW(wkc::time("10:11:12:123"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time("10:11::12"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time(":10:11:12"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time("10:11.12"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time("10:11:12."sv), wkc::ParseError);
  EXPECT_THROW(wkc::time("now"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time("invalid:input"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time("tomorrow"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time("today"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time("yesterday"sv), wkc::ParseError);
}

TEST(TypeTest, TimeOffset) {
  // Check that the `time_offset` type is implemented correctly
  wkc::time_offset offset{};
  EXPECT_EQ("Z", offset.ToString());

  offset = wkc::time_offset{90};
  EXPECT_EQ(90, offset.minutes);
  offset = wkc::time_offset{1, 30};
  EXPECT_EQ(90, offset.minutes);
  EXPECT_EQ("+01:30", offset.ToString());

  offset = wkc::time_offset{-61};
  EXPECT_EQ(-61, offset.minutes);

  offset = wkc::time_offset{-1, -18};
  EXPECT_EQ(-78, offset.minutes);
  EXPECT_EQ("-01:18", offset.ToString());

  offset = wkc::time_offset{-1, 18};
  EXPECT_EQ(-42, offset.minutes);
  EXPECT_EQ("-00:42", offset.ToString());

  EXPECT_EQ(25, wkc::time_offset(1, -35).minutes);
  EXPECT_EQ(-25, wkc::time_offset(-1, 35).minutes);
  EXPECT_EQ(-95, wkc::time_offset(-1, -35).minutes);

  EXPECT_EQ(-24, wkc::time_offset(0, -24).minutes);
  EXPECT_THROW(wkc::time_offset(-24, 0), wkc::TypeError);
  EXPECT_THROW(wkc::time_offset(24, 0), wkc::TypeError);

  // Operators
  EXPECT_TRUE(wkc::time_offset("-01:30"sv) < wkc::time_offset("-01:20"sv));
  EXPECT_TRUE(wkc::time_offset("-01:30"sv) <= wkc::time_offset("-01:20"sv));
  EXPECT_TRUE(wkc::time_offset("-01:30"sv) != wkc::time_offset("-01:20"sv));

  EXPECT_TRUE(wkc::time_offset("-01:20"sv) > wkc::time_offset("-01:30"sv));
  EXPECT_TRUE(wkc::time_offset("-01:20"sv) >= wkc::time_offset("-01:30"sv));

  EXPECT_TRUE(wkc::time_offset("-00:10"sv) <= wkc::time_offset("-00:10"sv));
  EXPECT_TRUE(wkc::time_offset("-00:10"sv) == wkc::time_offset("-00:10"sv));
  EXPECT_TRUE(wkc::time_offset("-00:10"sv) >= wkc::time_offset("-00:10"sv));

  EXPECT_TRUE(wkc::time_offset("-00:10"sv) < wkc::time_offset("00:10"sv));

  EXPECT_TRUE(wkc::time_offset("00:10"sv) < wkc::time_offset("10:50"sv));
  EXPECT_TRUE(wkc::time_offset("00:10"sv) <= wkc::time_offset("10:50"sv));
  EXPECT_TRUE(wkc::time_offset("02:00"sv) > wkc::time_offset("00:50"sv));
  EXPECT_TRUE(wkc::time_offset("02:00"sv) >= wkc::time_offset("00:50"sv));

  // Parsing
  EXPECT_EQ("-00:42", offset.ToString());
  EXPECT_EQ(offset, wkc::time_offset(offset.ToString()));
  EXPECT_EQ(0, wkc::time_offset(""sv).minutes);
  EXPECT_EQ(0, wkc::time_offset("Z"sv).minutes);
  EXPECT_EQ(0, wkc::time_offset("z"sv).minutes);
  EXPECT_EQ(0, wkc::time_offset("+00:00"sv).minutes);
  EXPECT_EQ(0, wkc::time_offset("-00:00"sv).minutes);
  EXPECT_EQ(62, wkc::time_offset("+01:02"sv).minutes);
  EXPECT_EQ(-63, wkc::time_offset("-01:03"sv).minutes);
  EXPECT_THROW(wkc::time_offset("+01:02Z"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time_offset("+01:02z"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time_offset("+01:-02"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time_offset("-01:-02"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time_offset("-24:02"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time_offset("+23:60"sv), wkc::ParseError);
}

// NOLINTEND
