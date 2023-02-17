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
  // Check basic handling of the `date` type
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

  // Decrement
  EXPECT_EQ(wkc::date(2000, 12, 1), --wkc::date(2000, 12, 2));
  EXPECT_EQ(wkc::date(2000, 11, 30), --wkc::date(2000, 12, 1));
  EXPECT_EQ(wkc::date(2000, 11, 1), --wkc::date(2000, 11, 2));
  EXPECT_EQ(wkc::date(2000, 10, 31), --wkc::date(2000, 11, 1));

  EXPECT_EQ(wkc::date(2000, 2, 29), --wkc::date(2000, 3, 1));
  EXPECT_EQ(wkc::date(1999, 2, 28), --wkc::date(1999, 3, 1));

  EXPECT_EQ(wkc::date(1999, 2, 1), --wkc::date(1999, 2, 2));
  EXPECT_EQ(wkc::date(1999, 1, 31), --wkc::date(1999, 2, 1));

  // Increment
  EXPECT_EQ(wkc::date(2000, 12, 1), ++wkc::date(2000, 11, 30));
  EXPECT_EQ(wkc::date(2000, 12, 2), ++wkc::date(2000, 12, 1));
  EXPECT_EQ(wkc::date(2000, 12, 31), ++wkc::date(2000, 12, 30));
  EXPECT_EQ(wkc::date(2001, 1, 1), ++wkc::date(2000, 12, 31));

  EXPECT_EQ(wkc::date(2004, 2, 29), ++wkc::date(2004, 2, 28));
  EXPECT_EQ(wkc::date(2004, 3, 1), ++wkc::date(2004, 2, 29));
  EXPECT_EQ(wkc::date(2005, 3, 1), ++wkc::date(2005, 2, 28));
}

TEST(TypeTest, DateParsing) {
  // Check date parsing in detail
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

  // Dates will be checked
  EXPECT_EQ(wkc::date(1, 2, 3), wkc::date("1-2-3"sv));

  EXPECT_THROW(wkc::date(2023, 02, 31), wkc::ValueError);
  EXPECT_NO_THROW(wkc::date(2023, 02, 28));
  EXPECT_THROW(wkc::date(2023, 02, 29), wkc::ValueError);
  EXPECT_NO_THROW(wkc::date(2024, 02, 29));

  EXPECT_THROW(wkc::date("2023-02-31"sv), wkc::ParseError);

  EXPECT_THROW(wkc::date("2023-02-29"sv), wkc::ParseError);
  EXPECT_NO_THROW(wkc::date("2024-02-29"sv));

  EXPECT_THROW(wkc::date("2023-02-0"sv), wkc::ParseError);
  EXPECT_THROW(wkc::date("2023-02-32"sv), wkc::ParseError);
  EXPECT_THROW(wkc::date("2023-13-3"sv), wkc::ParseError);
  EXPECT_THROW(wkc::date("2023-0-3"sv), wkc::ParseError);
  EXPECT_THROW(wkc::date("10000-1-3"sv), wkc::ParseError);

  // We also commonly use: d.m.Y
  EXPECT_EQ(wkc::date(2020, 3, 1), wkc::date("01.03.2020"sv));
  // A trailing delimiter will be ignored
  EXPECT_EQ(wkc::date(2020, 3, 1), wkc::date("01.03.2020."sv));

  EXPECT_THROW(wkc::date("1.2."sv), wkc::ParseError);
  EXPECT_THROW(wkc::date("1.2.2023.."sv), wkc::ParseError);
  EXPECT_THROW(wkc::date(".1.2.2023."sv), wkc::ParseError);
  EXPECT_THROW(wkc::date("invalid"sv), wkc::ParseError);
  EXPECT_THROW(wkc::date("invalid."sv), wkc::ParseError);
  EXPECT_THROW(wkc::date("d.m.Y"sv), wkc::ParseError);

  EXPECT_THROW(wkc::date(2023, 02, 31), wkc::ValueError);
  EXPECT_THROW(wkc::date("31.02.2023"), wkc::ParseError);

  EXPECT_EQ(wkc::date(2023, 02, 28), wkc::date("28.02.2023"sv));
  EXPECT_EQ(wkc::date(2023, 12, 3), wkc::date("3.12.2023"sv));
  EXPECT_EQ(wkc::date(1, 2, 3), wkc::date("3.2.1"sv));

  EXPECT_THROW(wkc::date("30.1.10000"sv), wkc::ParseError);
  EXPECT_THROW(wkc::date("30.0.1234"sv), wkc::ParseError);
  EXPECT_THROW(wkc::date("30.13.1234"sv), wkc::ParseError);
  EXPECT_THROW(wkc::date("0.2.1234"sv), wkc::ParseError);
  EXPECT_THROW(wkc::date("32.2.1234"sv), wkc::ParseError);
}

TEST(TypeTest, TimeType) {
  // Check basic handling of the `time` type
  wkc::time time{23, 49, 30, 987654321};
  auto tpl_time = time.ToTuple();
  EXPECT_EQ(time.hour, std::get<0>(tpl_time));
  EXPECT_EQ(time.minute, std::get<1>(tpl_time));
  EXPECT_EQ(time.second, std::get<2>(tpl_time));
  EXPECT_EQ(time.nanosecond, std::get<3>(tpl_time));

  // Printing a time
  EXPECT_EQ("23:49:30.987654321", time.ToString());
  std::ostringstream str;
  str << time;
  EXPECT_EQ("23:49:30.987654321", str.str());

  time.nanosecond = 100000000;
  EXPECT_EQ("23:49:30.100", time.ToString());
  time.nanosecond = 120000000;
  EXPECT_EQ("23:49:30.120", time.ToString());
  time.nanosecond = 123000000;
  EXPECT_EQ("23:49:30.123", time.ToString());
  time.nanosecond = 123400000;
  EXPECT_EQ("23:49:30.123400", time.ToString());
  time.nanosecond = 123450000;
  EXPECT_EQ("23:49:30.123450", time.ToString());
  time.nanosecond = 123456000;
  EXPECT_EQ("23:49:30.123456", time.ToString());
  time.nanosecond = 123456700;
  EXPECT_EQ("23:49:30.123456700", time.ToString());
  time.nanosecond = 123456780;
  EXPECT_EQ("23:49:30.123456780", time.ToString());
  time.nanosecond = 123456789;
  EXPECT_EQ("23:49:30.123456789", time.ToString());

  // Invalid value ranges.
  EXPECT_NO_THROW(wkc::time(0, 0, 0));
  EXPECT_NO_THROW(wkc::time(0, 10, 22));
  EXPECT_NO_THROW(wkc::time(8, 10, 22));

  EXPECT_NO_THROW(wkc::time(23, 10, 22));
  EXPECT_THROW(wkc::time(24, 10, 22), wkc::ValueError);

  EXPECT_NO_THROW(wkc::time(8, 59, 22));
  EXPECT_THROW(wkc::time(8, 60, 22), wkc::ValueError);

  EXPECT_NO_THROW(wkc::time(8, 10, 59));
  EXPECT_THROW(wkc::time(8, 10, 60), wkc::ValueError);

  // Overloaded operators.
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
  // Check time parsing in detail
  auto time = wkc::time{8, 10, 32, 123456789};
  auto parsed = wkc::time{time.ToString()};
  EXPECT_EQ(time, parsed);

  EXPECT_EQ(wkc::time(10, 11), wkc::time("10:11"sv));
  EXPECT_EQ(wkc::time(10, 11, 12), wkc::time("10:11:12"sv));

  // White space is not allowed
  EXPECT_THROW(wkc::time(" 10:11:12"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time(" 10:11:12 "sv), wkc::ParseError);
  EXPECT_THROW(wkc::time(" 10: 11:12"sv), wkc::ParseError);

  // Sub-second component must explicitly contain either 1, 2, 3 (ms), 6 (us)
  // or 9 (ns) digits.
  EXPECT_EQ(wkc::time(10, 11, 12, 100000000), wkc::time("10:11:12.1"sv));
  EXPECT_EQ(wkc::time(10, 11, 12, 120000000), wkc::time("10:11:12.12"sv));
  EXPECT_EQ(wkc::time(10, 11, 12, 123000000), wkc::time("10:11:12.123"sv));
  EXPECT_THROW(wkc::time("10:11:12.1 3"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time("10:11:12. 12"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time("10:11:12.12 "sv), wkc::ParseError);
  EXPECT_THROW(wkc::time("10:11:12. 123"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time("10:11:12.123 "sv), wkc::ParseError);
  EXPECT_THROW(wkc::time("10:11:12.1 23"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time("10:11:12.1234"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time("10:11:12.12345"sv), wkc::ParseError);
  EXPECT_EQ(wkc::time(10, 11, 12, 123456000), wkc::time("10:11:12.123456"sv));
  EXPECT_THROW(wkc::time("10:11:12.1234567"sv), wkc::ParseError);
  EXPECT_THROW(wkc::time("10:11:12.12345678"sv), wkc::ParseError);
  EXPECT_EQ(wkc::time(10, 11, 12, 123456789),
            wkc::time("10:11:12.123456789"sv));

  // Second/fraction delimiter can be '.' or ','
  EXPECT_EQ(wkc::time(10, 11, 12, 1000000), wkc::time("10:11:12.001"sv));
  EXPECT_EQ(wkc::time(10, 11, 12, 1000000), wkc::time("10:11:12,001"sv));
  EXPECT_EQ(wkc::time(10, 11, 12, 1002000), wkc::time("10:11:12.001002"sv));
  EXPECT_EQ(wkc::time(10, 11, 12, 1002000), wkc::time("10:11:12,001002"sv));
  EXPECT_EQ(wkc::time(10, 11, 12, 1002003), wkc::time("10:11:12.001002003"sv));
  EXPECT_THROW(wkc::time("10:11:12,001.002003"sv), wkc::ParseError);

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
  // Check basic handling of the `time_offset` type
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

TEST(TypeTest, DateTime) {
  // Check basic handling of the `date_time` type, which encapsulates
  // the separately tested date, time and offset types.

  wkc::time time{8, 10, 32, 123456789};
  wkc::date date{2000, 11, 04};

  wkc::date_time dt{date, time};
  EXPECT_EQ(date, dt.date);
  EXPECT_EQ(time, dt.time);
  EXPECT_FALSE(dt.offset.has_value());

  EXPECT_EQ(dt, wkc::date_time(dt.ToString()));
  EXPECT_EQ("2000-11-04T08:10:32.123456789", dt.ToString());

  dt.offset = wkc::time_offset{};
  EXPECT_EQ(dt, wkc::date_time(dt.ToString()));
  EXPECT_EQ("2000-11-04T08:10:32.123456789Z", dt.ToString());

  dt.time.nanosecond = 0;
  EXPECT_EQ("2000-11-04T08:10:32Z", dt.ToString());

  wkc::time_offset offset{-1, -12};
  dt.offset = offset;
  EXPECT_EQ(dt, wkc::date_time(dt.ToString()));
  EXPECT_EQ("2000-11-04T08:10:32-01:12", dt.ToString());

  // Parsing valid formats according to RFC 3339.
  dt = wkc::date_time{wkc::date{2023, 2, 14}, wkc::time{21, 8, 23}};
  // Offset has not been set.
  EXPECT_EQ(dt, wkc::date_time{"2023-02-14T21:08:23"sv});
  EXPECT_NE(dt, wkc::date_time{"2023-02-14T21:08:23Z"sv});
  dt.offset = wkc::time_offset{0};
  EXPECT_NE(dt, wkc::date_time{"2023-02-14T21:08:23"sv});
  EXPECT_EQ(dt, wkc::date_time{"2023-02-14T21:08:23Z"sv});

  // For readability, the delimiter between date and time can
  // also be a space.
  EXPECT_EQ(dt, wkc::date_time{"2023-02-14 21:08:23Z"sv});
  // Underscore is also commonly used.
  EXPECT_EQ(dt, wkc::date_time{"2023-02-14_21:08:23Z"sv});

  // Uppercase and lowercase letters T/Z are valid.
  EXPECT_EQ(dt, wkc::date_time{"2023-02-14T21:08:23Z"sv});
  EXPECT_EQ(dt, wkc::date_time{"2023-02-14T21:08:23z"sv});
  EXPECT_EQ(dt, wkc::date_time{"2023-02-14t21:08:23z"sv});
  EXPECT_EQ(dt, wkc::date_time{"2023-02-14t21:08:23Z"sv});

  // 'Z' is equal to an offset of +/-00:00
  EXPECT_EQ(dt, wkc::date_time{"2023-02-14T21:08:23-00:00"sv});
  EXPECT_EQ(dt, wkc::date_time{"2023-02-14T21:08:23+00:00"sv});
  EXPECT_EQ(dt, wkc::date_time{"2023-02-14 21:08:23Z"sv});

  // Parsing with nanoseconds.
  dt.time.nanosecond = 880000000;
  EXPECT_EQ(dt, wkc::date_time{"2023-02-14_21:08:23.880Z"sv});
  EXPECT_EQ(dt, wkc::date_time{"2023-02-14 21:08:23.880Z"sv});
  EXPECT_EQ(dt, wkc::date_time{"2023-02-14T21:08:23.880Z"sv});

  // Parsing with additional offset.
  EXPECT_EQ(dt, wkc::date_time{"2023-02-14T22:08:23.880+01:00"sv});
  EXPECT_EQ(dt, wkc::date_time{"2023-02-14T20:08:23.880-01:00"sv});
  EXPECT_EQ(dt, wkc::date_time{"2023-02-14t22:08:23.880+01:00"sv});
  EXPECT_EQ(dt, wkc::date_time{"2023-02-14T22:08:23.880+01:00"sv});
  EXPECT_NE(dt, wkc::date_time{"2023-02-14T22:08:23+01:00"sv});  // ns differ

  EXPECT_EQ(wkc::date_time{"2024-02-29 00:45:12.123+01:00"sv},
            wkc::date_time{"2024-02-28 23:45:12.123Z"sv});
  EXPECT_EQ(wkc::date_time("2024-02-29 00:45:12.123+01:00"sv).UTC(),
            wkc::date_time("2024-02-28 23:45:12.123Z"sv).UTC());

  EXPECT_NE(dt, wkc::date_time{"2023-02-14T21:08:23"sv});

  dt.time = wkc::time{"22:30:03"sv};
  dt.offset = wkc::time_offset{60};
  EXPECT_EQ(dt, wkc::date_time{"2023-02-14T22:30:03+01:00"sv});
  EXPECT_NE(dt, wkc::date_time{"2023-02-14T22:30:03.8+01:00"sv});
  dt.time.nanosecond = 800000000;
  EXPECT_EQ(dt, wkc::date_time{"2023-02-14T22:30:03.8+01:00"sv});
  EXPECT_NE(dt, wkc::date_time{"2023-02-14T22:30:03.88+01:00"sv});
  dt.time.nanosecond = 880000000;
  EXPECT_EQ(dt, wkc::date_time{"2023-02-14T22:30:03.88+01:00"sv});
  dt.time.nanosecond = 884000000;
  EXPECT_EQ(dt, wkc::date_time{"2023-02-14T22:30:03.884+01:00"sv});

  // RFC 3339 Examples, p. 10
  dt = wkc::date_time{{1985, 4, 12}, {23, 20, 50, 520000000}};
  EXPECT_TRUE(dt.IsLocal());
  dt.offset = wkc::time_offset{0};
  EXPECT_FALSE(dt.IsLocal());
  EXPECT_EQ(dt, wkc::date_time{"1985-04-12T23:20:50.52Z"sv});

  dt = wkc::date_time{{1996, 12, 19}, {16, 39, 57}, {-8, 0}};
  EXPECT_FALSE(dt.IsLocal());
  EXPECT_EQ(dt, wkc::date_time{"1996-12-20T00:39:57Z"sv});

  // Invalid strings
  EXPECT_THROW(wkc::date_time("invalid"sv), wkc::ParseError);
  EXPECT_THROW(wkc::date_time("now"sv), wkc::ParseError);
  EXPECT_THROW(wkc::date_time("today"sv), wkc::ParseError);
  EXPECT_THROW(wkc::date_time("tomorrow"sv), wkc::ParseError);
  EXPECT_THROW(wkc::date_time("yesterday"sv), wkc::ParseError);

  // TODO Comparison operators
  // TODO EXPECT_LT(dt, wkc::date_time{"2023-02-14_21:08:23.880Z"sv});
}

// NOLINTEND
