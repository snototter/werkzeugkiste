#include <werkzeugkiste/config/casts.h>
#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/files/filesys.h>
#include <werkzeugkiste/geometry/vector.h>
#include <werkzeugkiste/strings/strings.h>

#include <cmath>
#include <exception>
#include <limits>
#include <sstream>
#include <string_view>

#include "../test_utils.h"

namespace wkc = werkzeugkiste::config;
namespace wkf = werkzeugkiste::files;
namespace wkg = werkzeugkiste::geometry;
namespace wks = werkzeugkiste::strings;

// NOLINTBEGIN

using namespace std::string_view_literals;

TEST(ConfigTest, TypeUtils) {
  EXPECT_EQ("bool", wkc::TypeName<bool>());

  EXPECT_EQ("char", wkc::TypeName<char>());
  EXPECT_EQ("unsigned char", wkc::TypeName<unsigned char>());
  EXPECT_EQ("short", wkc::TypeName<short>());
  EXPECT_EQ("unsigned short", wkc::TypeName<unsigned short>());
  EXPECT_EQ("int", wkc::TypeName<int>());
  EXPECT_EQ("unsigned int", wkc::TypeName<unsigned int>());
  EXPECT_EQ("long int", wkc::TypeName<long int>());
  EXPECT_EQ("unsigned long int", wkc::TypeName<unsigned long int>());

  EXPECT_EQ("float", wkc::TypeName<float>());
  EXPECT_EQ("double", wkc::TypeName<double>());

  EXPECT_EQ("string", wkc::TypeName<std::string>());
  EXPECT_EQ("string_view", wkc::TypeName<std::string_view>());

  EXPECT_EQ("date", wkc::TypeName<wkc::date>());

  EXPECT_NO_THROW(wkc::TypeName<unsigned short>());
  EXPECT_NE("ushort", wkc::TypeName<unsigned short>());
}

TEST(ConfigTest, Integers) {
  const auto config = wkc::LoadTOMLString(R"toml(
    int32_1 = -123456
    int32_2 = +987654
    int32_max = 2147483647
    int32_max_overflow = 2147483648
    int32_min = -2147483648
    int32_min_underflow = -2147483649
    )toml"sv);
  EXPECT_TRUE(config.GetOptionalInteger32("int32_1"sv).has_value());
  EXPECT_EQ(-123456, config.GetOptionalInteger32("int32_1"sv).value());

  EXPECT_EQ(-123456, config.GetInteger32("int32_1"sv));
  EXPECT_EQ(987654, config.GetInteger32("int32_2"sv));

  EXPECT_EQ(2147483647, config.GetInteger32("int32_max"sv));
  EXPECT_EQ(-2147483648, config.GetInteger32("int32_min"sv));

  EXPECT_THROW(config.GetInteger32("int32_min_underflow"sv), wkc::TypeError);
  try {
    config.GetInteger32("int32_min_underflow"sv);
  } catch (const wkc::TypeError &e) {
    EXPECT_TRUE(wks::StartsWith(e.what(), "Underflow"sv));
  }

  EXPECT_THROW(config.GetInteger32("int32_max_overflow"sv), wkc::TypeError);
  try {
    config.GetInteger32("int32_max_overflow"sv);
  } catch (const wkc::TypeError &e) {
    EXPECT_TRUE(wks::StartsWith(e.what(), "Overflow"sv));
  }

  EXPECT_THROW(config.GetOptionalInteger32("int32_min_underflow"sv),
               wkc::TypeError);
  EXPECT_THROW(config.GetOptionalInteger32("int32_max_overflow"sv),
               wkc::TypeError);

  EXPECT_EQ(-1, config.GetInteger32Or("test"sv, -1));
  EXPECT_EQ(17, config.GetInteger32Or("test"sv, 17));
  EXPECT_THROW(config.GetInteger32("test"sv), wkc::KeyError);
  EXPECT_FALSE(config.GetOptionalInteger32("test"sv).has_value());

  EXPECT_EQ(-123456, config.GetInteger64("int32_1"sv));
  EXPECT_TRUE(config.GetOptionalInteger64("int32_1"sv).has_value());
  EXPECT_EQ(-123456, config.GetOptionalInteger64("int32_1"sv).value());

  EXPECT_EQ(+987654, config.GetInteger64("int32_2"sv));

  EXPECT_EQ(-2147483649, config.GetInteger64("int32_min_underflow"sv));
  EXPECT_EQ(-2147483649,
            config.GetOptionalInteger64("int32_min_underflow"sv).value());

  EXPECT_EQ(+2147483648, config.GetInteger64("int32_max_overflow"sv));
  EXPECT_EQ(+2147483648,
            config.GetOptionalInteger64("int32_max_overflow"sv).value());

  EXPECT_EQ(-1, config.GetInteger64Or("test"sv, -1));
  EXPECT_EQ(17, config.GetInteger64Or("test"sv, 17));
  EXPECT_THROW(config.GetInteger64("test"sv), wkc::KeyError);
  EXPECT_FALSE(config.GetOptionalInteger64("test"sv).has_value());

  try {
    config.GetInteger32("int32"sv);
  } catch (const wkc::KeyError &e) {
    const std::string exp_msg{
        "Key `int32` does not exist! Did you mean: `int32_1`, `int32_2`?"};
    EXPECT_EQ(exp_msg, std::string(e.what()));
  }
}

TEST(ConfigTest, FloatingPoint) {
  auto config = wkc::LoadTOMLString(R"toml(
    int = 32

    flt1 = +1.0
    flt2 = -3.1415
    flt3 = 5e+22

    spec1 = inf
    spec2 = -inf
    spec3 = nan
    )toml"sv);

  // General access of floating point parameters:
  EXPECT_TRUE(config.GetOptionalDouble("flt1"sv).has_value());
  EXPECT_DOUBLE_EQ(+1.0, config.GetOptionalDouble("flt1"sv).value());
  EXPECT_DOUBLE_EQ(+1.0, config.GetDouble("flt1"sv));
  EXPECT_DOUBLE_EQ(-3.1415, config.GetDouble("flt2"sv));
  EXPECT_TRUE(config.GetOptionalDouble("flt2"sv).has_value());
  EXPECT_DOUBLE_EQ(-3.1415, config.GetOptionalDouble("flt2"sv).value());
  EXPECT_DOUBLE_EQ(+5e22, config.GetDouble("flt3"sv));

  EXPECT_THROW(config.GetDouble("test"sv), wkc::KeyError);
  EXPECT_DOUBLE_EQ(-16.0, config.GetDoubleOr("test"sv, -16));
  EXPECT_FALSE(config.GetOptionalDouble("test"sv).has_value());

  // Querying special numbers:
  using limits = std::numeric_limits<double>;
  EXPECT_DOUBLE_EQ(+limits::infinity(), config.GetDouble("spec1"sv));
  EXPECT_DOUBLE_EQ(-limits::infinity(), config.GetDouble("spec2"sv));
  EXPECT_TRUE(std::isnan(config.GetDouble("spec3"sv)));

  // Setting special numbers:
  config.SetDouble("my-inf"sv, -limits::infinity());
  EXPECT_TRUE(std::isinf(config.GetDouble("my-inf"sv)));
  EXPECT_DOUBLE_EQ(-limits::infinity(), config.GetDouble("my-inf"sv));

  config.SetDouble("my-nan"sv, limits::quiet_NaN());
  EXPECT_TRUE(std::isnan(config.GetDouble("my-nan"sv)));

  // Implicit conversion is possible if the value is exactly representable:
  EXPECT_DOUBLE_EQ(32.0, config.GetDouble("int"sv));
  EXPECT_DOUBLE_EQ(32.0, config.GetOptionalDouble("int"sv).value());
  EXPECT_EQ(1, config.GetInteger32("flt1"sv));
  EXPECT_EQ(1L, config.GetInteger64("flt1"sv));
  // -3.14 is not:
  EXPECT_THROW(config.GetInteger32("flt2"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger64("flt2"sv), wkc::TypeError);
}

TEST(ConfigTest, QueryTypes) {
  const auto config = wkc::LoadTOMLString(R"toml(
    bool = true
    int = 42
    flt = 1.0
    str = "A string"
    lst = [1, 2, 3.5]

    [dates]
    day = 2023-01-01
    time1 = 12:34:56
    time2 = 00:01:02.123456
    date_time = 1912-07-23T08:37:00-08:00

    )toml"sv);

  EXPECT_THROW(config.Type(""sv), wkc::KeyError);

  // Bool, int, float, string
  EXPECT_TRUE(config.Contains("bool"sv));
  EXPECT_FALSE(config.Contains("bool1"sv));
  EXPECT_EQ(wkc::ConfigType::Boolean, config.Type("bool"sv));

  EXPECT_TRUE(config.Contains("int"sv));
  EXPECT_FALSE(config.Contains("in"sv));
  EXPECT_EQ(wkc::ConfigType::Integer, config.Type("int"sv));

  EXPECT_TRUE(config.Contains("flt"sv));
  EXPECT_EQ(wkc::ConfigType::FloatingPoint, config.Type("flt"sv));

  EXPECT_TRUE(config.Contains("str"sv));
  EXPECT_EQ(wkc::ConfigType::String, config.Type("str"sv));

  // List
  EXPECT_TRUE(config.Contains("lst"sv));
  EXPECT_EQ(wkc::ConfigType::List, config.Type("lst"sv));

  EXPECT_TRUE(config.Contains("lst[0]"sv));
  EXPECT_EQ(wkc::ConfigType::Integer, config.Type("lst[0]"sv));
  EXPECT_TRUE(config.Contains("lst[1]"sv));
  EXPECT_EQ(wkc::ConfigType::Integer, config.Type("lst[1]"sv));
  EXPECT_TRUE(config.Contains("lst[2]"sv));
  EXPECT_EQ(wkc::ConfigType::FloatingPoint, config.Type("lst[2]"sv));
  EXPECT_FALSE(config.Contains("lst[3]"sv));

  EXPECT_THROW(config.Type("lst[3]"sv), wkc::KeyError);
  try {
    config.Type("lst[3]"sv);
  } catch (const wkc::KeyError &e) {
    const std::string exp_msg{
        "Key `lst[3]` does not exist! Did you mean: `lst[0]`, `lst[1]`, "
        "`lst[2]`?"};
    EXPECT_EQ(exp_msg, std::string(e.what()));
  }

  // Group/table
  EXPECT_TRUE(config.Contains("dates"sv));
  EXPECT_EQ(wkc::ConfigType::Group, config.Type("dates"sv));

  // Date & time
  EXPECT_TRUE(config.Contains("dates.day"sv));
  EXPECT_EQ(wkc::ConfigType::Date, config.Type("dates.day"sv));

  EXPECT_TRUE(config.Contains("dates.time1"sv));
  EXPECT_EQ(wkc::ConfigType::Time, config.Type("dates.time1"sv));
  EXPECT_TRUE(config.Contains("dates.time2"sv));
  EXPECT_EQ(wkc::ConfigType::Time, config.Type("dates.time2"sv));

  EXPECT_TRUE(config.Contains("dates.date_time"sv));
  EXPECT_EQ(wkc::ConfigType::DateTime, config.Type("dates.date_time"sv));

  EXPECT_TRUE(config.Contains("dates.date_time"sv));
  EXPECT_EQ(wkc::ConfigType::DateTime, config.Type("dates.date_time"sv));

  // Access invalid types
  EXPECT_THROW(config.GetBoolean("lst"sv), wkc::TypeError);
  EXPECT_THROW(config.GetString("bool"sv), wkc::TypeError);
  EXPECT_THROW(config.GetBoolean("dates"sv), wkc::TypeError);
  EXPECT_THROW(config.GetBoolean("dates.day"sv), wkc::TypeError);
  EXPECT_THROW(config.GetBoolean("dates.time1"sv), wkc::TypeError);
  EXPECT_THROW(config.GetBoolean("dates.time2"sv), wkc::TypeError);
  EXPECT_THROW(config.GetBoolean("dates.date_time"sv), wkc::TypeError);

  // Verify the string representation of the custom type enum:
  EXPECT_EQ("boolean", wkc::ConfigTypeToString(wkc::ConfigType::Boolean));
  EXPECT_EQ("integer", wkc::ConfigTypeToString(wkc::ConfigType::Integer));
  EXPECT_EQ("floating_point",
            wkc::ConfigTypeToString(wkc::ConfigType::FloatingPoint));
  EXPECT_EQ("string", wkc::ConfigTypeToString(wkc::ConfigType::String));
  EXPECT_EQ("date", wkc::ConfigTypeToString(wkc::ConfigType::Date));
  EXPECT_EQ("time", wkc::ConfigTypeToString(wkc::ConfigType::Time));
  EXPECT_EQ("date_time", wkc::ConfigTypeToString(wkc::ConfigType::DateTime));
  EXPECT_EQ("list", wkc::ConfigTypeToString(wkc::ConfigType::List));
  EXPECT_EQ("group", wkc::ConfigTypeToString(wkc::ConfigType::Group));

  // Stream operator should be properly overloaded
  std::ostringstream str;
  str << wkc::ConfigType::Date;
  EXPECT_EQ("date", str.str());
  str << '!' << wkc::ConfigType::FloatingPoint;
  EXPECT_EQ("date!floating_point", str.str());
}

TEST(ConfigTest, GetScalarTypes) {
  const auto config = wkc::LoadTOMLString(R"toml(
    bool = true
    int = 42
    flt = 1.0
    str = "A string"

    int_list = [1, 2, 3]

    [dates]
    day = 2023-01-02
    time = 01:02:03.123456
    dt1 = 1912-07-23T08:37:00-08:00
    dt2 = 2004-02-28T23:59:59.999888-01:00

    )toml"sv);

  // Boolean parameter
  EXPECT_EQ(true, config.GetBoolean("bool"sv));
  EXPECT_TRUE(config.GetOptionalBoolean("bool"sv).has_value());
  EXPECT_EQ(true, config.GetOptionalBoolean("bool"sv).value());

  EXPECT_THROW(config.GetBoolean("no-such.bool"sv), wkc::KeyError);
  EXPECT_FALSE(config.GetOptionalBoolean("no-such.bool"sv).has_value());
  EXPECT_TRUE(config.GetBooleanOr("no-such.bool"sv, true));
  EXPECT_FALSE(config.GetBooleanOr("no-such.bool"sv, false));

  EXPECT_THROW(config.GetBooleanList("bool"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger32("bool"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger32Or("bool"sv, 0), wkc::TypeError);
  EXPECT_THROW(config.GetOptionalInteger32("bool"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger32List("bool"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger64("bool"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger64Or("bool"sv, 2), wkc::TypeError);
  EXPECT_THROW(config.GetOptionalInteger64("bool"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger64List("bool"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDouble("bool"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDoubleOr("bool"sv, 1.0), wkc::TypeError);
  EXPECT_THROW(config.GetOptionalDouble("bool"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDoubleList("bool"sv), wkc::TypeError);
  EXPECT_THROW(config.GetString("bool"sv), wkc::TypeError);
  EXPECT_THROW(config.GetStringOr("bool"sv, "..."sv), wkc::TypeError);
  EXPECT_THROW(config.GetOptionalString("bool"sv), wkc::TypeError);
  EXPECT_THROW(config.GetStringList("bool"sv), wkc::TypeError);

  // Integer parameter
  EXPECT_EQ(42, config.GetInteger32("int"sv));
  EXPECT_EQ(42, config.GetInteger64("int"sv));

  EXPECT_THROW(config.GetBoolean("int"sv), wkc::TypeError);
  EXPECT_THROW(config.GetBooleanOr("int"sv, true), wkc::TypeError);
  EXPECT_THROW(config.GetString("int"sv), wkc::TypeError);
  EXPECT_THROW(config.GetStringOr("int"sv, "..."sv), wkc::TypeError);
  // This integer is exactly representable by a double
  EXPECT_DOUBLE_EQ(42.0, config.GetDouble("int"sv));

  // Double parameter
  EXPECT_DOUBLE_EQ(1.0, config.GetDouble("flt"sv));

  EXPECT_THROW(config.GetBoolean("flt"sv), wkc::TypeError);
  EXPECT_THROW(config.GetString("flt"sv), wkc::TypeError);
  EXPECT_THROW(config.GetStringOr("flt"sv, "..."sv), wkc::TypeError);
  // This float is exactly representable by an integer
  EXPECT_EQ(1, config.GetInteger32("flt"sv));
  EXPECT_EQ(1L, config.GetInteger64("flt"sv));

  // String parameter
  const std::string expected{"A string"};
  EXPECT_EQ(expected, config.GetString("str"sv));
  EXPECT_TRUE(config.GetOptionalString("str"sv).has_value());
  EXPECT_EQ("A string", config.GetOptionalString("str"sv).value());

  EXPECT_THROW(config.GetString("no-such-key"sv), wkc::KeyError);
  EXPECT_FALSE(config.GetOptionalBoolean("no-such-key"sv).has_value());

  EXPECT_EQ("...", config.GetStringOr("no-such-key"sv, "..."sv));

  EXPECT_THROW(config.GetBoolean("str"sv), wkc::TypeError);
  EXPECT_THROW(config.GetOptionalBoolean("str"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger32("str"sv), wkc::TypeError);
  EXPECT_THROW(config.GetOptionalInteger32("str"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger64("str"sv), wkc::TypeError);
  EXPECT_THROW(config.GetOptionalInteger64("str"sv), wkc::TypeError);

  // Date parameter
  EXPECT_EQ(wkc::date(2023, 01, 02), config.GetDate("dates.day"sv));
  EXPECT_NE(wkc::date(2022, 01, 02), config.GetDate("dates.day"sv));

  EXPECT_TRUE(config.GetOptionalDate("dates.day"sv).has_value());
  EXPECT_EQ(wkc::date(2023, 01, 02),
            config.GetOptionalDate("dates.day"sv).value());

  EXPECT_THROW(config.GetDate("str"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDateOr("str"sv, wkc::date{1234, 12, 30}),
               wkc::TypeError);
  EXPECT_THROW(config.GetDate("no-such-key"sv), wkc::KeyError);
  EXPECT_EQ(wkc::date(1234, 12, 30),
            config.GetDateOr("no-such-key"sv, wkc::date{1234, 12, 30}));

  // The fractional seconds ".123" will be parsed according to the TOML
  // specification into "123000000" nanoseconds.
  wkc::time time{1, 2, 3, 123456000};
  EXPECT_EQ(time, config.GetTime("dates.time"sv));
  EXPECT_TRUE(config.GetOptionalTime("dates.time"sv).has_value());
  EXPECT_EQ(time, config.GetOptionalTime("dates.time"sv).value());

  EXPECT_THROW(config.GetTime("str"sv), wkc::TypeError);
  EXPECT_THROW(config.GetTime("dates.day"sv), wkc::TypeError);
  EXPECT_THROW(config.GetTimeOr("str"sv, time), wkc::TypeError);

  EXPECT_THROW(config.GetTime("no-such-key"sv), wkc::KeyError);
  EXPECT_EQ(time, config.GetTimeOr("no-such-key"sv, time));
  EXPECT_FALSE(config.GetOptionalTime("no-such-key"sv).has_value());

  // Date-time parameter
  const wkc::date_time dt1{"1912-07-23T08:37:00-08:00"sv};
  wkc::date_time dt2{"2004-02-28T23:59:59.999888-01:00"sv};
  EXPECT_EQ(dt1, config.GetDateTime("dates.dt1"sv));
  EXPECT_EQ(dt2, config.GetDateTime("dates.dt2"sv));
  EXPECT_NE(dt1, dt2);

  EXPECT_TRUE(config.GetOptionalDateTime("dates.dt1"sv).has_value());
  EXPECT_EQ(dt1, config.GetOptionalDateTime("dates.dt1"sv).value());

  EXPECT_THROW(config.GetDateTime("str"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDateTime("dates.day"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDateTimeOr("str"sv, dt1), wkc::TypeError);
  EXPECT_THROW(config.GetDateTime("no-such-key"sv), wkc::KeyError);
  EXPECT_EQ(dt2, config.GetDateTimeOr("no-such-key"sv, dt2));

  dt2.offset = std::nullopt;
  EXPECT_NE(dt2, config.GetDateTime("dates.dt2"sv));
  dt2.offset = wkc::time_offset{-59};
  EXPECT_NE(dt2, config.GetDateTime("dates.dt2"sv));
  dt2.offset = wkc::time_offset{-60};
  EXPECT_EQ(dt2, config.GetDateTime("dates.dt2"sv));

  // Invalid access
  EXPECT_THROW(config.GetBoolean("int_list"sv), wkc::TypeError);
  EXPECT_THROW(config.GetBoolean("tbl"sv), wkc::KeyError);
  EXPECT_THROW(config.GetInteger32("int_list"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger32("tbl"sv), wkc::KeyError);
  EXPECT_THROW(config.GetInteger64("int_list"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger64("tbl"sv), wkc::KeyError);
  EXPECT_THROW(config.GetDouble("int_list"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDouble("tbl"sv), wkc::KeyError);
  EXPECT_THROW(config.GetString("int_list"sv), wkc::TypeError);
  EXPECT_THROW(config.GetString("tbl"sv), wkc::KeyError);
  EXPECT_THROW(config.GetDate("int_list"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDate("tbl"sv), wkc::KeyError);
  EXPECT_THROW(config.GetTime("int_list"sv), wkc::TypeError);
  EXPECT_THROW(config.GetTime("tbl"sv), wkc::KeyError);
  EXPECT_THROW(config.GetDateTime("int_list"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDateTime("tbl"sv), wkc::KeyError);

  EXPECT_THROW(config.GetInteger32("dates"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger32("dates.day"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger32("dates.time"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger32("dates.dt1"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDouble("dates"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDouble("dates.day"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDouble("dates.time"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDouble("dates.dt1"sv), wkc::TypeError);
  EXPECT_THROW(config.GetString("dates"sv), wkc::TypeError);
  EXPECT_THROW(config.GetString("dates.day"sv), wkc::TypeError);
  EXPECT_THROW(config.GetString("dates.time"sv), wkc::TypeError);
  EXPECT_THROW(config.GetString("dates.dt1"sv), wkc::TypeError);
}

TEST(ConfigTest, SetBoolean) {
  auto config = wkc::LoadTOMLString(R"toml(
    bool = true
    int = 42
    a.string = "value"
    booleans = [true, false, true]

    array = [0, 1, { int = 2, bool = false }]
    )toml"sv);

  // Adjust a boolean parameter
  EXPECT_EQ(true, config.GetBoolean("bool"sv));
  EXPECT_NO_THROW(config.SetBoolean("bool"sv, false));
  EXPECT_EQ(false, config.GetBoolean("bool"sv));

  // Cannot change the type of an existing parameter
  EXPECT_THROW(config.SetBoolean("int"sv, true), wkc::TypeError);

  // Set a non-existing parameter
  EXPECT_THROW(config.GetBoolean("another_bool"sv), wkc::KeyError);
  EXPECT_NO_THROW(config.SetBoolean("another_bool"sv, false));
  EXPECT_NO_THROW(config.GetBoolean("another_bool"sv));
  EXPECT_EQ(false, config.GetBoolean("another_bool"sv));

  // Set a nested parameter (must create the hierarchy)
  EXPECT_THROW(config.GetBoolean("others.bool"sv), wkc::KeyError);
  EXPECT_NO_THROW(config.SetBoolean("others.bool"sv, false));
  EXPECT_NO_THROW(config.GetBoolean("others.bool"sv));
  EXPECT_EQ(false, config.GetBoolean("others.bool"sv));

  // Test a deeper path hierarchy
  EXPECT_THROW(config.GetBoolean("a.deeper.hierarchy.bool"sv), wkc::KeyError);
  EXPECT_NO_THROW(config.SetBoolean("a.deeper.hierarchy.bool"sv, false));
  EXPECT_NO_THROW(config.GetBoolean("a.deeper.hierarchy.bool"sv));
  EXPECT_EQ(false, config.GetBoolean("a.deeper.hierarchy.bool"sv));

  // Cannot create a path below a scalar type
  EXPECT_THROW(config.SetBoolean("a.string.below.bool"sv, true),
               wkc::TypeError);

  // Creating an array is also not supported
  EXPECT_THROW(config.SetBoolean("an_array[3].bool"sv, true), wkc::TypeError);

  // Creating a table within an existing array is also not supported:
  EXPECT_THROW(config.SetBoolean("array[3].bool"sv, true), wkc::TypeError);

  // Currently, we don't support replacing/inserting array elements:
  EXPECT_THROW(config.SetBoolean("array[3]"sv, true), std::logic_error);

  // Creating a table as a child of an array is also not supported. There's
  // currently no need for such exotic use cases.
  EXPECT_THROW(config.SetBoolean("array[4].another_table.value"sv, true),
               wkc::TypeError);

  // But setting an existing array element is supported:
  EXPECT_NO_THROW(config.SetBoolean("booleans[1]"sv, true));
  EXPECT_EQ(true, config.GetBoolean("booleans[0]"sv));
  EXPECT_EQ(true, config.GetBoolean("booleans[1]"sv));
  EXPECT_EQ(true, config.GetBoolean("booleans[2]"sv));

  EXPECT_EQ(false, config.GetBoolean("array[2].bool"sv));
  EXPECT_NO_THROW(config.SetBoolean("array[2].bool"sv, true));
  EXPECT_EQ(true, config.GetBoolean("array[2].bool"sv));
}

TEST(ConfigTest, SetOtherScalarTypes) {
  auto config = wkc::LoadTOMLString(R"toml(
    integer = 12345
    string = "This is a string"

    [section]
    float = 1.5
    string = "value"
    array = [1, true, "a string"]
    )toml"sv);

  // Change integers
  EXPECT_EQ(12345, config.GetInteger32("integer"sv));
  EXPECT_NO_THROW(config.SetInteger32("integer"sv, -123));
  EXPECT_EQ(-123, config.GetInteger32("integer"sv));

  EXPECT_EQ(-123, config.GetInteger64("integer"sv));
  EXPECT_NO_THROW(config.SetInteger64("integer"sv, -2147483649));
  EXPECT_EQ(-2147483649, config.GetInteger64("integer"sv));

  // Change a double
  EXPECT_DOUBLE_EQ(1.5, config.GetDouble("section.float"sv));
  EXPECT_NO_THROW(config.SetDouble("section.float"sv, 0.01));
  EXPECT_DOUBLE_EQ(0.01, config.GetDouble("section.float"sv));

  // We cannot change the type of an existing parameter
  EXPECT_THROW(config.SetDouble("integer"sv, 1.5), wkc::TypeError);

  // Set a string:
  EXPECT_EQ("value", config.GetString("section.string"sv));
  EXPECT_NO_THROW(config.SetString("section.string"sv, "frobmorten"sv));
  EXPECT_EQ("frobmorten", config.GetString("section.string"sv));

  // Change a string within an array:
  EXPECT_EQ("a string", config.GetString("section.array[2]"sv));
  EXPECT_NO_THROW(config.SetString("section.array[2]"sv, "foobar"sv));
  EXPECT_EQ("foobar", config.GetString("section.array[2]"sv));

  // Add new scalars:
  EXPECT_NO_THROW(config.SetInteger32("new-values.int32"sv, 3));
  EXPECT_NO_THROW(config.SetInteger64("new-values.int64"sv, 64));
  EXPECT_NO_THROW(config.SetDouble("new-values.float"sv, 1e23));
  EXPECT_NO_THROW(config.SetString("new-values.str", "It works!"));
  EXPECT_EQ(3, config.GetInteger32("new-values.int32"sv));
  EXPECT_EQ(64, config.GetInteger32("new-values.int64"sv));
  EXPECT_DOUBLE_EQ(1e23, config.GetDouble("new-values.float"sv));
  EXPECT_EQ("It works!", config.GetString("new-values.str"sv));

  // Set a date
  EXPECT_FALSE(config.Contains("my-day"sv));
  EXPECT_FALSE(config.GetOptionalDate("my-day"sv).has_value());
  wkc::date day{2023, 9, 3};
  EXPECT_NO_THROW(config.SetDate("my-day"sv, day));
  EXPECT_TRUE(config.Contains("my-day"sv));
  EXPECT_EQ(day, config.GetDate("my-day"sv));
  EXPECT_EQ(day, config.GetOptionalDate("my-day"sv).value());

  ++day;
  EXPECT_NE(day, config.GetDate("my-day"sv));
  EXPECT_NO_THROW(config.SetDate("my-day"sv, day));
  EXPECT_EQ(day, config.GetDate("my-day"sv));

  EXPECT_EQ(day, config.GetDateOr("no-such-key"sv, day));

  EXPECT_THROW(config.SetDate("string"sv, wkc::date{}), wkc::TypeError);

  // Set a time
  EXPECT_FALSE(config.Contains("my-time"sv));
  EXPECT_FALSE(config.GetOptionalTime("my-time"sv).has_value());

  wkc::time tm{10, 42, 59};
  EXPECT_NO_THROW(config.SetTime("my-time"sv, tm));
  EXPECT_TRUE(config.Contains("my-time"sv));
  EXPECT_EQ(tm, config.GetTime("my-time"sv));
  EXPECT_EQ(tm, config.GetOptionalTime("my-time"sv).value());

  tm.hour = 12;
  EXPECT_NE(tm, config.GetTime("my-time"sv));
  EXPECT_NO_THROW(config.SetTime("my-time"sv, tm));
  EXPECT_EQ(tm, config.GetTime("my-time"sv));

  EXPECT_EQ(tm, config.GetTimeOr("no-such-key"sv, tm));

  EXPECT_THROW(config.SetTime("string"sv, wkc::time{}), wkc::TypeError);

  // Set a date_time
  EXPECT_FALSE(config.Contains("my-dt"sv));
  EXPECT_FALSE(config.GetOptionalTime("my-dt"sv).has_value());

  wkc::date_time dt{day, tm};
  EXPECT_THROW(config.SetDateTime("my-day"sv, dt), wkc::TypeError);
  EXPECT_THROW(config.SetDateTime("my-time"sv, dt), wkc::TypeError);
  EXPECT_NO_THROW(config.SetDateTime("my-dt"sv, dt));
  EXPECT_TRUE(config.Contains("my-dt"sv));
  EXPECT_EQ(dt, config.GetDateTime("my-dt"sv));
  EXPECT_EQ(dt, config.GetOptionalDateTime("my-dt"sv).value());

  ++dt.date;
  EXPECT_NE(dt, config.GetDateTime("my-dt"sv));
  EXPECT_NO_THROW(config.SetDateTime("my-dt"sv, dt));
  EXPECT_EQ(dt, config.GetDateTime("my-dt"sv));

  dt.offset = wkc::time_offset{90};
  EXPECT_NE(dt, config.GetDateTime("my-dt"sv));
  EXPECT_NO_THROW(config.SetDateTime("my-dt"sv, dt));
  EXPECT_EQ(dt, config.GetDateTime("my-dt"sv));

  EXPECT_EQ(dt, config.GetDateTimeOr("no-such-key"sv, dt));

  EXPECT_THROW(config.SetDateTime("string"sv, dt), wkc::TypeError);
  EXPECT_THROW(config.GetDateTime("my-day"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDateTime("my-time"sv), wkc::TypeError);
}

TEST(ConfigTest, Keys1) {
  const std::string toml_str = R"toml(
    key = "value"
    other-key = 0
    another_key = 1
    1234 = "value"

    tbl1.param1 = "value"
    tbl1.param2 = 'value'

    tbl2.array = [1, 2, 3]
    )toml";

  const auto config = wkc::LoadTOMLString(toml_str);
  const auto keys = config.ListParameterNames(false);

  std::istringstream iss(toml_str);
  std::string line;
  while (std::getline(iss, line)) {
    const auto tokens = wks::Tokenize(line, "=");
    if (tokens.empty()) {
      continue;
    }

    const auto key = wks::Trim(tokens[0]);
    if (key.empty()) {
      continue;
    }

    const auto pos = std::find(keys.begin(), keys.end(), key);
    EXPECT_NE(keys.end(), pos) << "Key `"sv << key << "` not found!"sv;
  }
}

void CheckExpectedKeys(const std::vector<std::string> &expected_keys,
                       const std::vector<std::string> &keys) {
  EXPECT_EQ(expected_keys.size(), keys.size())
      << "Extracted keys: " << Stringify(keys)
      << "\nExpected keys:  " << Stringify(expected_keys) << "!";

  for (const auto &expected : expected_keys) {
    const auto pos = std::find(keys.begin(), keys.end(), expected);
    EXPECT_NE(keys.end(), pos) << "Key `"sv << expected << "` not found!"sv;
  }
}

TEST(ConfigTest, Keys2) {
  const std::string toml_str = R"toml(
    arr1 = [
      1,
      {first = "value", second = "value"}
    ]

    [lvl-1.lvl-2]
    param1 = "value"
    param2 = "value"

    [lvl-1]
    arr2 = [0, 1, 17.4]
    arr3 = [
      "a", "b", { name = "value", age = 12.3 },
      ["inside", "a nested", { type = "array", value = "abc" }]
    ]

    [[tests]]
    name = "value"

    [[tests]]

    [[tests]]
    param = "value"
    )toml";
  const auto config = wkc::LoadTOMLString(toml_str);

  // First, check without extracting the array keys.
  std::vector<std::string> expected_keys{"arr1",
                                         "arr1[1].first",
                                         "arr1[1].second",
                                         "lvl-1",
                                         "lvl-1.arr2",
                                         "lvl-1.arr3",
                                         "lvl-1.arr3[2].name",
                                         "lvl-1.arr3[2].age",
                                         "lvl-1.arr3[3][2].type",
                                         "lvl-1.arr3[3][2].value",
                                         "lvl-1.lvl-2",
                                         "lvl-1.lvl-2.param1",
                                         "lvl-1.lvl-2.param2",
                                         "tests",
                                         "tests[0].name",
                                         "tests[2].param"};
  auto keys = config.ListParameterNames(false);

  CheckExpectedKeys(expected_keys, keys);

  // Second, test with *all* keys. This should explicitly include each
  // array entry, too.
  expected_keys.push_back("arr1[0]");
  expected_keys.push_back("arr1[1]");
  expected_keys.push_back("lvl-1.arr2[0]");
  expected_keys.push_back("lvl-1.arr2[1]");
  expected_keys.push_back("lvl-1.arr2[2]");
  expected_keys.push_back("lvl-1.arr3[0]");
  expected_keys.push_back("lvl-1.arr3[1]");
  expected_keys.push_back("lvl-1.arr3[2]");
  expected_keys.push_back("lvl-1.arr3[3]");
  expected_keys.push_back("lvl-1.arr3[3][0]");
  expected_keys.push_back("lvl-1.arr3[3][1]");
  expected_keys.push_back("lvl-1.arr3[3][2]");
  expected_keys.push_back("tests[0]");
  expected_keys.push_back("tests[1]");
  expected_keys.push_back("tests[2]");

  keys = config.ListParameterNames(true);

  EXPECT_EQ(expected_keys.size(), keys.size())
      << "Extracted keys: "sv << Stringify(keys) << "\nExpected keys:  "sv
      << Stringify(expected_keys) << "!"sv;

  for (const auto &expected : expected_keys) {
    const auto pos = std::find(keys.begin(), keys.end(), expected);
    EXPECT_NE(keys.end(), pos) << "Key `"sv << expected << "` not found!"sv;
  }
}

TEST(ConfigTest, KeyMatching) {
  // Default construction
  wkc::KeyMatcher empty{};
  EXPECT_TRUE(empty.Empty());

  auto matcher = wkc::KeyMatcher{"this-is.a-valid.key"sv};
  EXPECT_FALSE(matcher.Empty());

  EXPECT_FALSE(matcher.Match("this.is.a-valid.key"sv));
  EXPECT_FALSE(matcher.Match("this_is.a_valid.key"sv));
  EXPECT_FALSE(matcher.Match("this-is.a-valid.ke"sv));
  EXPECT_FALSE(matcher.Match("this-is.a-valid.key2"sv));

  EXPECT_TRUE(matcher.Match("this-is.a-valid.key"sv));
  EXPECT_FALSE(matcher.Match("this-is.a-valid.keY"sv));

  // Force copy construction
  wkc::KeyMatcher copy{matcher};
  EXPECT_FALSE(copy.Empty());
  EXPECT_TRUE(copy.Match("this-is.a-valid.key"sv));
  EXPECT_FALSE(copy.Match("this-is.a-valid.keY"sv));

  EXPECT_TRUE(matcher.Match("this-is.a-valid.key"sv));
  EXPECT_FALSE(matcher.Match("this-is.a-valid.keY"sv));

  // Force move construction
  wkc::KeyMatcher moved{std::move(matcher)};
  EXPECT_FALSE(moved.Empty());
  EXPECT_TRUE(moved.Match("this-is.a-valid.key"sv));
  EXPECT_FALSE(moved.Match("this-is.a-valid.keY"sv));

  // Copy/move assignments are tested after the following
  // multi-key matching tests.
  matcher = wkc::KeyMatcher{{"plain-key"sv, "a.b.c1"sv}};
  EXPECT_FALSE(matcher.Match("this-is.a-valid.key"sv));
  EXPECT_TRUE(matcher.Match("plain-key"sv));
  EXPECT_TRUE(matcher.Match("a.b.c1"sv));
  EXPECT_FALSE(matcher.Match("a.b.c"sv));

  // Wildcard
  matcher = wkc::KeyMatcher{"pattern*"sv};
  EXPECT_TRUE(matcher.Match("pattern"sv));
  EXPECT_TRUE(matcher.Match("pattern-"sv));
  EXPECT_TRUE(matcher.Match("pattern1"sv));
  EXPECT_FALSE(matcher.Match("a-pattern"sv));

  // Test copy assignment
  copy = matcher;
  EXPECT_FALSE(copy.Empty());
  EXPECT_TRUE(copy.Match("pattern"sv));
  EXPECT_TRUE(copy.Match("pattern-"sv));
  EXPECT_TRUE(copy.Match("pattern1"sv));
  EXPECT_FALSE(copy.Match("a-pattern"sv));

  EXPECT_FALSE(matcher.Empty());
  EXPECT_TRUE(matcher.Match("pattern"sv));
  EXPECT_TRUE(matcher.Match("pattern-"sv));
  EXPECT_TRUE(matcher.Match("pattern1"sv));
  EXPECT_FALSE(matcher.Match("a-pattern"sv));

  // Multiple wildcards
  matcher = wkc::KeyMatcher{"*pattern*"sv};
  EXPECT_TRUE(matcher.Match("pattern"sv));
  EXPECT_TRUE(matcher.Match("pattern-"sv));
  EXPECT_TRUE(matcher.Match("pattern1"sv));
  EXPECT_TRUE(matcher.Match("a-pattern"sv));
  EXPECT_FALSE(matcher.Match("pAttern"sv));
  EXPECT_FALSE(matcher.Match("pat-tern"sv));

  // Move assignment
  moved = std::move(matcher);
  EXPECT_FALSE(moved.Empty());
  EXPECT_TRUE(moved.Match("pattern"sv));
  EXPECT_TRUE(moved.Match("pattern-"sv));
  EXPECT_TRUE(moved.Match("pattern1"sv));
  EXPECT_TRUE(moved.Match("a-pattern"sv));
  EXPECT_FALSE(moved.Match("pAttern"sv));
  EXPECT_FALSE(moved.Match("pat-tern"sv));

  // Another wildcard (to match multiple sub-levels)
  matcher = wkc::KeyMatcher{"table.*.param"sv};
  EXPECT_FALSE(matcher.Match("table.param"sv));
  EXPECT_TRUE(matcher.Match("table.sub.param"sv));
  EXPECT_TRUE(matcher.Match("table.Sub123.param"sv));
  EXPECT_TRUE(matcher.Match("table.sub.foo.param"sv));
  EXPECT_TRUE(matcher.Match("table.sub.foo.Bar.param"sv));
  EXPECT_FALSE(matcher.Match("table1.sub.param"sv));
  EXPECT_FALSE(matcher.Match("table.sub.param1"sv));

  // We explicitly use only a basic substitution.
  // Yes, this invalid keys matches. No, this is not a problem
  // because the matching is only used internally to select
  // existing nodes (and an invalid key could not have been created
  // to begin with...)
  matcher = wkc::KeyMatcher{"arr[*].*"sv};
  EXPECT_TRUE(matcher.Match("arr[*].*"sv));
  EXPECT_FALSE(matcher.Match("arr*"sv));
  EXPECT_FALSE(matcher.Match("arr.name"sv));
  EXPECT_FALSE(matcher.Match("arr[]name"sv));
  EXPECT_TRUE(matcher.Match("arr[0].name"sv));
  EXPECT_TRUE(matcher.Match("arr[1].name"sv));
  EXPECT_TRUE(matcher.Match("arr[-10].name"sv));
  EXPECT_TRUE(matcher.Match("arr[123].name"sv));
  EXPECT_TRUE(matcher.Match("arr[123].*"sv));
  EXPECT_TRUE(matcher.Match("arr[0][1].*"sv));
  EXPECT_TRUE(matcher.Match("arr[0][1][2].*"sv));
}

template <typename VecType, typename Tuples>
inline std::vector<VecType> TuplesToVecs(const Tuples &tuples) {
  static_assert(VecType::ndim == 2 || VecType::ndim == 3,
                "This test util is only supported for 2D or 3D vectors.");

  std::vector<VecType> poly;
  for (const auto &tpl : tuples) {
    typename VecType::value_type x =
        static_cast<typename VecType::value_type>(std::get<0>(tpl));
    typename VecType::value_type y =
        static_cast<typename VecType::value_type>(std::get<1>(tpl));
    if constexpr (VecType::ndim == 2) {
      poly.emplace_back(VecType{x, y});
    } else {
      typename VecType::value_type z =
          static_cast<typename VecType::value_type>(std::get<2>(tpl));
      poly.emplace_back(VecType{x, y, z});
    }
  }
  return poly;
}
// template <typename VecType, typename... TupleTypes>
// inline std::vector<typename std::enable_if<VecType::ndim ==
// sizeof...(TupleTypes), VecType>::type> TuplesToVecs(const
// std::vector<std::tuple<TupleTypes...>> &tuples) {
//   std::vector<VecType> poly;
//   for (const auto &tpl : tuples) {
//     poly.emplace_back(std::make_from_tuple<VecType>(tpl));
//   }
//   return poly;
// }

TEST(ConfigTest, PointLists) {
  const auto config = wkc::LoadTOMLString(R"toml(
    str = "not a point list"

    poly1 = [[1, 2], [3, 4], [5, 6], [-7, -8]]

    poly2 = [{y = 20, x = 10}, {x = 30, y = 40}, {y = 60, x = 50}]

    poly3 = [[1, 2, 3], [4, 5, 6], {x = -9, y = 0, z = -3}]

    poly64 = [[-10, 20], [1, 3], [2147483647, 2147483648], [0, 21474836480]]

    [[poly4]]
    x = 100
    y = 200
    z = -5

    [[poly4]]
    x = 300
    y = 400
    z = -5

    [invalid]
    # Missing y dimension (2nd point):
    p1 = [{x = 1, y = 2}, {x = 1, name = 2, param = 3}]

    # Mix data types
    p2 = [{x = 1, y = 2}, {x = 1.5, y = 2}]
    p3 = [[1, 2], [5.5, 1.23]]

    # Mix "points" (nested arrays) and scalars
    p4 = [[1, 2], [3, 4], 5]
    p5 = [[1, 2], [3, 4], [5]]

    # 2D & 3D point (Can be converted to 2D polygon)
    p6 = [{x = 1, y = 2}, {x = 1, y = 2, z = 3}]
    p7 = [[1, 2], [3, 4, 5], [6, 7]]

    )toml"sv);

  // Sanity checks
  EXPECT_THROW(config.GetIndices2D("str"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger32List("str"sv), wkc::TypeError);
  EXPECT_THROW(config.GetBooleanList("str"sv), wkc::TypeError);
  EXPECT_THROW(config.GetBooleanList("poly1"sv), wkc::TypeError);

  // Retrieve a polyline
  auto poly = config.GetIndices2D("poly1"sv);
  EXPECT_EQ(4, poly.size());

  auto list = config.GetInteger32List("poly1[0]"sv);
  EXPECT_EQ(2, list.size());
  EXPECT_EQ(1, list[0]);
  EXPECT_EQ(2, list[1]);
  list = config.GetInteger32List("poly1[2]"sv);
  EXPECT_EQ(2, list.size());
  EXPECT_EQ(5, list[0]);
  EXPECT_EQ(6, list[1]);

  auto vec = TuplesToVecs<wkg::Vec2i>(poly);
  EXPECT_EQ(wkg::Vec2i(1, 2), vec[0]);
  EXPECT_EQ(wkg::Vec2i(3, 4), vec[1]);
  EXPECT_EQ(wkg::Vec2i(5, 6), vec[2]);
  EXPECT_EQ(wkg::Vec2i(-7, -8), vec[3]);

  poly = config.GetIndices2D("poly2"sv);
  EXPECT_EQ(3, poly.size());

  vec = TuplesToVecs<wkg::Vec2i>(poly);
  EXPECT_EQ(wkg::Vec2i(10, 20), vec[0]);
  EXPECT_EQ(wkg::Vec2i(30, 40), vec[1]);
  EXPECT_EQ(wkg::Vec2i(50, 60), vec[2]);

  // Cannot load an array of tables as a scalar list:
  EXPECT_THROW(config.GetInteger32List("poly2"sv), wkc::TypeError);

  // An N-dimensional polygon can be looked up from any list of at
  // least N-dimensional points:
  EXPECT_NO_THROW(config.GetIndices2D("poly3"sv));
  EXPECT_NO_THROW(config.GetIndices3D("poly3"sv));
  EXPECT_NO_THROW(config.GetIndices2D("poly4"sv));
  EXPECT_NO_THROW(config.GetIndices3D("poly4"sv));

  // Points uses 32-bit integers. Cause an overflow:
  EXPECT_THROW(config.GetIndices2D("poly64"sv), wkc::TypeError);

  EXPECT_THROW(config.GetIndices2D("no-such-key"sv), wkc::KeyError);
  EXPECT_THROW(config.GetIndices2D("str"sv), wkc::TypeError);
  EXPECT_THROW(config.GetIndices2D("invalid.p1"sv), wkc::TypeError);
  EXPECT_THROW(config.GetIndices2D("invalid.p2"sv), wkc::TypeError);
  EXPECT_THROW(config.GetIndices2D("invalid.p3"sv), wkc::TypeError);
  EXPECT_THROW(config.GetIndices2D("invalid.p4"sv), wkc::TypeError);
  EXPECT_THROW(config.GetIndices2D("invalid.p5"sv), wkc::TypeError);

  EXPECT_NO_THROW(config.GetIndices2D("invalid.p6"sv));
  EXPECT_THROW(config.GetIndices3D("invalid.p6"sv), wkc::TypeError);

  EXPECT_NO_THROW(config.GetIndices2D("invalid.p7"sv));
  EXPECT_THROW(config.GetIndices3D("invalid.p7"sv), wkc::TypeError);

  // 3D polygons
  EXPECT_THROW(config.GetIndices3D("poly1"sv), wkc::TypeError);
  EXPECT_THROW(config.GetIndices3D("poly2"sv), wkc::TypeError);

  auto poly3d = config.GetIndices3D("poly3"sv);
  EXPECT_EQ(3, poly3d.size());
  std::vector<wkg::Vec3i> vec3d = TuplesToVecs<wkg::Vec3i>(poly3d);
  EXPECT_EQ(wkg::Vec3i(1, 2, 3), vec3d[0]);
  EXPECT_EQ(wkg::Vec3i(4, 5, 6), vec3d[1]);
  EXPECT_EQ(wkg::Vec3i(-9, 0, -3), vec3d[2]);
}

TEST(ConfigTest, GetLists) {
  const auto config = wkc::LoadTOMLString(R"toml(
    flags = [true, false, false]

    ints32 = [1, 2, 3, 4, 5, 6, -7, -8]

    ints64 = [0, 2147483647, 2147483648, -2147483648, -2147483649]

    ints64_castable = [-3000, 68000, 0, 12345678]

    floats = [0.5, 1.0, 1.0e23]

    floats_castable = [0.0, -2.0, 100.0, 12345.0]

    strings = ["abc", "Foo", "Frobmorten", "Test String"]

    # Type mix
    mixed_int_flt = [1, 2, 3, 4.5, 5]

    mixed_types = [1, 2, "framboozle"]

    nested_lst = [1, 2, [3, 4], "frobmorten", {name = "fail"}]

    an_int = 1234

    [not-a-list]
    name = "test"

    [[products]]
    value = 1

    [[products]]
    value = 2

    [[products]]
    value = 3
    )toml"sv);

  // Key error:
  EXPECT_THROW(config.GetInteger32List("no-such-key"sv), wkc::KeyError);
  EXPECT_THROW(config.GetInteger64List("no-such-key"sv), wkc::KeyError);
  EXPECT_THROW(config.GetDoubleList("no-such-key"sv), wkc::KeyError);
  EXPECT_THROW(config.GetStringList("no-such-key"sv), wkc::KeyError);

  // Try to load a wrong data type as list:
  EXPECT_THROW(config.GetBooleanList("an_int"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger32List("flags"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger32List("an_int"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger32List("not-a-list"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger32List("not-a-list.no-such-key"sv),
               wkc::KeyError);

  EXPECT_THROW(config.GetInteger64List("an_int"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger64List("not-a-list"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger64List("not-a-list.no-such-key"sv),
               wkc::KeyError);

  EXPECT_THROW(config.GetDoubleList("an_int"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDoubleList("not-a-list"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDoubleList("not-a-list.no-such-key"sv), wkc::KeyError);

  EXPECT_THROW(config.GetStringList("an_int"sv), wkc::TypeError);
  EXPECT_THROW(config.GetStringList("not-a-list"sv), wkc::TypeError);
  EXPECT_THROW(config.GetStringList("not-a-list.no-such-key"sv), wkc::KeyError);

  // Cannot load inhomogeneous arrays:
  EXPECT_THROW(config.GetInteger32List("mixed_types"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger64List("mixed_types"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDoubleList("mixed_types"sv), wkc::TypeError);
  EXPECT_THROW(config.GetStringList("mixed_types"sv), wkc::TypeError);

  EXPECT_THROW(config.GetInteger32List("nested_lst"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger64List("nested_lst"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDoubleList("nested_lst"sv), wkc::TypeError);
  EXPECT_THROW(config.GetStringList("nested_lst"sv), wkc::TypeError);

  // Cannot load a list of tables:
  EXPECT_THROW(config.GetInteger32List("products"sv), wkc::TypeError);

  // Lists must consist of elements of the same type (unless an
  // implicit & lossless cast is available)
  auto list32 = config.GetInteger32List("ints32"sv);
  EXPECT_EQ(8, list32.size());
  auto list64 = config.GetInteger64List("ints32"sv);
  EXPECT_EQ(8, list64.size());
  EXPECT_EQ(1, list32[0]);
  EXPECT_EQ(6, list32[5]);
  EXPECT_EQ(-8, list32[7]);

  // Integers can be implicitly converted to floating point numbers:
  EXPECT_NO_THROW(config.GetDoubleList("ints32"sv));
  EXPECT_THROW(config.GetStringList("ints32"sv), wkc::TypeError);

  // Implicit type conversion:
  EXPECT_THROW(config.GetInteger32List("ints64"sv), wkc::TypeError);
  EXPECT_NO_THROW(config.GetInteger32List("ints64_castable"sv));

  list64 = config.GetInteger64List("ints64"sv);
  EXPECT_EQ(5, list64.size());

  auto list_dbl = config.GetDoubleList("floats"sv);
  EXPECT_EQ(3, list_dbl.size());
  EXPECT_DOUBLE_EQ(0.5, list_dbl[0]);
  EXPECT_DOUBLE_EQ(1.0, list_dbl[1]);
  EXPECT_DOUBLE_EQ(1e23, list_dbl[2]);

  // As a user, you should assume that a float cannot be
  // queried as another type by default:
  EXPECT_THROW(config.GetInteger32List("floats"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger64List("floats"sv), wkc::TypeError);
  EXPECT_THROW(config.GetStringList("floats"sv), wkc::TypeError);
  // But if an exact representation (i.e. a lossless cast) is
  // possible, we allow implicit type conversion:
  EXPECT_NO_THROW(config.GetInteger32List("floats_castable"sv));
  list32 = config.GetInteger32List("floats_castable"sv);
  EXPECT_EQ(4, list32.size());
  EXPECT_EQ(0, list32[0]);
  EXPECT_EQ(-2, list32[1]);
  EXPECT_EQ(100, list32[2]);
  EXPECT_EQ(12345, list32[3]);
  EXPECT_NO_THROW(config.GetInteger64List("floats_castable"sv));
  EXPECT_THROW(config.GetStringList("floats_castable"sv), wkc::TypeError);

  // Implicit conversion to integers fails for fractional numbers,
  // such as "4.5" in mixed_int_flt:
  EXPECT_THROW(config.GetInteger32List("mixed_int_flt"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger64List("mixed_int_flt"sv), wkc::TypeError);
  EXPECT_NO_THROW(config.GetDoubleList("mixed_int_flt"sv));
}

TEST(ConfigTest, SetLists) {
  auto config = wkc::LoadTOMLString(R"toml(
    flags = [true, false, false]

    ints = [1, 2, 3, 4, 5, 6, -7, -8]

    floats = [0.5, 1.0, 1.0e23]

    strings = ["abc", "Foo", "Frobmorten", "Test String"]

    mixed_int_flt = [1, 2, 3, 4.5, 5]

    mixed_types = [1, 2, "framboozle"]

    nested_lst = [1, 2, [3, 4], "frobmorten", {name = "fail"}]

    an_int = 1234

    [not-a-list]
    name = "test"

    [[products]]
    value = 1

    [[products]]
    value = 2

    [[products]]
    value = 3
    )toml"sv);

  // Cannot change the type of a parameter:
  EXPECT_THROW(config.SetBooleanList("ints"sv, {true, false}), wkc::TypeError);
  EXPECT_THROW(config.SetInteger32List("flags"sv, {1, 3, -17}), wkc::TypeError);
  EXPECT_THROW(config.SetDoubleList("nested_lst"sv, {1.0, -0.5}),
               wkc::TypeError);
  EXPECT_THROW(config.SetStringList("floats"sv, {"abc"}), wkc::TypeError);
}

TEST(ConfigTest, Pairs) {
  const auto config = wkc::LoadTOMLString(R"toml(
    int_list = [1, 2, 3, 4]

    int32_pair = [1024, 768]

    int64_pair = [2147483647, 2147483648]

    float_pair = [0.5, 1.0]

    mixed_types = [1, "framboozle"]

    a_scalar = 1234

    nested_array = [1, [2, [3, 4]]]
    )toml"sv);

  // Key error:
  EXPECT_THROW(config.GetInteger32Pair("no-such-key"sv), wkc::KeyError);
  EXPECT_THROW(config.GetInteger64Pair("no-such-key"sv), wkc::KeyError);
  EXPECT_THROW(config.GetDoublePair("no-such-key"sv), wkc::KeyError);

  // A pair must be an array of 2 elements
  EXPECT_THROW(config.GetInteger32Pair("int_list"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger64Pair("int_list"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDoublePair("int_list"sv), wkc::TypeError);

  EXPECT_THROW(config.GetInteger32Pair("mixed_types"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger64Pair("mixed_types"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDoublePair("mixed_types"sv), wkc::TypeError);

  EXPECT_THROW(config.GetInteger32Pair("a_scalar"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger64Pair("a_scalar"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDoublePair("a_scalar"sv), wkc::TypeError);

  EXPECT_THROW(config.GetInteger32Pair("nested_array"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger64Pair("nested_array"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDoublePair("nested_array"sv), wkc::TypeError);

  // Load a valid pair
  auto p32 = config.GetInteger32Pair("int32_pair"sv);
  EXPECT_EQ(1024, p32.first);
  EXPECT_EQ(768, p32.second);

  EXPECT_THROW(config.GetInteger32Pair("int64_pair"sv), wkc::TypeError);
  auto p64 = config.GetInteger64Pair("int64_pair"sv);
  EXPECT_EQ(2147483647, p64.first);
  EXPECT_EQ(2147483648, p64.second);

  EXPECT_THROW(config.GetInteger32Pair("float_pair"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger64Pair("float_pair"sv), wkc::TypeError);
  auto pdbl = config.GetDoublePair("float_pair"sv);
  EXPECT_DOUBLE_EQ(0.5, pdbl.first);
  EXPECT_DOUBLE_EQ(1.0, pdbl.second);
}

TEST(ConfigTest, GetGroup) {
  const auto config = wkc::LoadTOMLString(R"toml(
    str = "A string"

    [lvl1]
    flt = 1.0

    [lvl1.grp1]
    str = "g1"
    lst = [1, 2]

    [lvl1.grp2]
    str = "g2"
    val = 3

    [lvl1.grp3]

    [dates]
    day = 2023-01-01
    )toml"sv);

  EXPECT_THROW(config.GetGroup("no-such-key"sv), wkc::KeyError);
  EXPECT_THROW(config.GetGroup("str"sv), wkc::TypeError);
  EXPECT_THROW(config.GetGroup("dates.day"sv), wkc::TypeError);

  auto sub = config.GetGroup("lvl1.grp1"sv);
  EXPECT_FALSE(sub.Empty());
  auto keys = sub.ListParameterNames(true);
  CheckExpectedKeys({"str", "lst", "lst[0]", "lst[1]"}, keys);

  sub = config.GetGroup("lvl1.grp2"sv);
  EXPECT_FALSE(sub.Empty());
  keys = sub.ListParameterNames(false);
  CheckExpectedKeys({"str", "val"}, keys);

  sub = config.GetGroup("lvl1"sv);
  EXPECT_FALSE(sub.Empty());
  keys = sub.ListParameterNames(true);
  const std::vector<std::string> expected{
      "flt",         "grp1", "grp1.str", "grp1.lst", "grp1.lst[0]",
      "grp1.lst[1]", "grp2", "grp2.str", "grp2.val", "grp3"};
  CheckExpectedKeys(expected, keys);

  // Empty sub-group
  sub = config.GetGroup("lvl1.grp3"sv);
  EXPECT_TRUE(sub.Empty());
  keys = sub.ListParameterNames(false);
  EXPECT_EQ(0, keys.size());
}

TEST(ConfigTest, SetGroup) {
  auto config = wkc::LoadTOMLString(R"toml(
    str = "A string"

    [lvl1]
    flt = 1.0

    [lvl1.grp1]
    str = "g1"
    lst = [1, 2]

    [lvl1.grp2]
    str = "g2"
    val = 3

    [lvl1.grp3]

    [dates]
    day = 2023-01-01
    )toml"sv);

  wkc::Configuration empty{};

  EXPECT_THROW(config.SetGroup(""sv, empty), wkc::KeyError);
  EXPECT_THROW(config.SetGroup("dates.day"sv, empty), wkc::TypeError);
  EXPECT_NO_THROW(config.SetGroup("empty"sv, empty));

  EXPECT_TRUE(config.Contains("empty"sv));
  auto group = config.GetGroup("empty"sv);
  EXPECT_TRUE(group.Empty());

  empty.SetBoolean("my-bool", true);
  empty.SetInteger32("my-int32", 23);
  empty.SetString("my-str", "value");
  EXPECT_FALSE(empty.Empty());

  // Insert group below an existing group
  EXPECT_NO_THROW(config.SetGroup("lvl1.grp3"sv, empty));
  EXPECT_TRUE(config.Contains("lvl1.grp3.my-bool"sv));
  EXPECT_TRUE(config.Contains("lvl1.grp3.my-int32"sv));
  EXPECT_TRUE(config.Contains("lvl1.grp3.my-str"sv));

  group = config.GetGroup("lvl1.grp3"sv);
  EXPECT_FALSE(group.Empty());

  auto keys = group.ListParameterNames(true);
  CheckExpectedKeys({"my-bool", "my-int32", "my-str"}, keys);

  // Insert group at root level
  EXPECT_NO_THROW(config.SetGroup("my-grp"sv, empty));
  EXPECT_TRUE(config.Contains("my-grp.my-bool"sv));
  EXPECT_TRUE(config.Contains("my-grp.my-int32"sv));
  EXPECT_TRUE(config.Contains("my-grp.my-str"sv));
}

TEST(ConfigTest, NestedTOML) {
  const auto fname_invalid_toml =
      wkf::FullFile(wkf::DirName(__FILE__), "test-invalid.toml"sv);
  std::ostringstream toml_str;
  toml_str << "bool = true\ninteger = 3\nlst = [1, 2]\ndate = 2023-02-21\n"
              "time = 08:30:00\ndatetime = 2023-02-21T11:11:11\n"
              "nested_config = \""sv
           << wkf::FullFile(wkf::DirName(__FILE__), "test-valid1.toml"sv)
           << "\"\n"
              "float = 2.0\n"
              "invalid_nested_config = \""sv
           << fname_invalid_toml
           << "\"\n"
              "lvl1.lvl2.lvl3.nested = \""sv
           << wkf::FullFile(wkf::DirName(__FILE__), "test-valid1.toml"sv)
           << "\"\n"
              "lvl1.arr = [ 1, 2, \""sv
           << wkf::FullFile(wkf::DirName(__FILE__), "test-valid1.toml"sv)
           << "\"]\n"
              "lvl1.another_arr = [1, { name = 'test', nested = \""
           << wkf::FullFile(wkf::DirName(__FILE__), "test-valid1.toml"sv)
           << "\" }]"sv;

  auto config = wkc::LoadTOMLString(toml_str.str());
  EXPECT_THROW(config.LoadNestedTOMLConfiguration("no-such-key"sv),
               wkc::KeyError);
  EXPECT_THROW(config.LoadNestedTOMLConfiguration("bool"sv), wkc::TypeError);
  EXPECT_THROW(config.LoadNestedTOMLConfiguration("integer"sv), wkc::TypeError);
  EXPECT_THROW(config.LoadNestedTOMLConfiguration("float"sv), wkc::TypeError);
  EXPECT_THROW(config.LoadNestedTOMLConfiguration("lst"sv), wkc::TypeError);
  EXPECT_THROW(config.LoadNestedTOMLConfiguration("date"sv), wkc::TypeError);
  EXPECT_THROW(config.LoadNestedTOMLConfiguration("time"sv), wkc::TypeError);
  EXPECT_THROW(config.LoadNestedTOMLConfiguration("datetime"sv),
               wkc::TypeError);
  EXPECT_THROW(config.LoadNestedTOMLConfiguration("lvl1"sv), wkc::TypeError);
  EXPECT_THROW(config.LoadNestedTOMLConfiguration("lvl1.lvl2"sv),
               wkc::TypeError);
  config.LoadNestedTOMLConfiguration("nested_config"sv);

  EXPECT_EQ(1, config.GetInteger32("nested_config.value1"sv));
  EXPECT_DOUBLE_EQ(2.3, config.GetDouble("nested_config.value2"sv));
  EXPECT_EQ("this/is/a/relative/path",
            config.GetString("nested_config.section1.rel_path"sv));

  // When trying to load an invalid TOML file, an exception should be thrown,
  // and the parameter should not change.
  EXPECT_THROW(config.LoadNestedTOMLConfiguration("invalid_nested_config"sv),
               wkc::ParseError);
  EXPECT_EQ(fname_invalid_toml, config.GetString("invalid_nested_config"sv));

  // Ensure that loading a nested configuration also works at deeper
  // hierarchy levels.
  EXPECT_NO_THROW(
      config.LoadNestedTOMLConfiguration("lvl1.lvl2.lvl3.nested"sv));
  EXPECT_DOUBLE_EQ(2.3, config.GetDouble("lvl1.lvl2.lvl3.nested.value2"sv));
  EXPECT_EQ("this/is/a/relative/path",
            config.GetString("lvl1.lvl2.lvl3.nested.section1.rel_path"sv));

  // It is not allowed to load a nested configuration directly into an array:
  EXPECT_THROW(config.LoadNestedTOMLConfiguration("lvl1.arr[2]"sv),
               wkc::TypeError);

  // One could abuse it, however, to load a nested configuration into a table
  // that is inside an array... Just because you can doesn't mean you should...
  EXPECT_NO_THROW(
      config.LoadNestedTOMLConfiguration("lvl1.another_arr[1].nested"sv));
  EXPECT_DOUBLE_EQ(2.3,
                   config.GetDouble("lvl1.another_arr[1].nested.value2"sv));
  EXPECT_EQ("this/is/a/relative/path",
            config.GetString("lvl1.another_arr[1].nested.section1.rel_path"sv));
}

TEST(ConfigTest, AbsolutePaths) {
  const std::string fname =
      wkf::FullFile(wkf::DirName(__FILE__), "test-valid1.toml"sv);
  auto config = wkc::LoadTOMLFile(fname);

  EXPECT_FALSE(config.AdjustRelativePaths("...", {"no-such-key"sv}));
  EXPECT_TRUE(
      config.AdjustRelativePaths(wkf::DirName(__FILE__), {"section1.*path"sv}));

  std::string expected =
      wkf::FullFile(wkf::DirName(__FILE__), "this/is/a/relative/path"sv);
  EXPECT_EQ(expected, config.GetString("section1.rel_path"sv));

  expected =
      "file://" + wkf::FullFile(wkf::DirName(__FILE__), "also/relative"sv);
  EXPECT_EQ(expected, config.GetString("section1.rel_url_path"sv));

  // TODO check special character handling (backslash, umlauts,
  // whitespace)

  EXPECT_THROW(config.AdjustRelativePaths("this-will-throw", {"value1"sv}),
               wkc::TypeError);
  EXPECT_THROW(
      config.AdjustRelativePaths("this-will-throw", {"section1.time"sv}),
      wkc::TypeError);
}

TEST(ConfigTest, StringReplacements) {
  auto config = wkc::LoadTOMLString(R"toml(
    str1 = ""
    str2 = "This is a test"
    str3 = "Hello world!"
    value = 123

    str_list = ["List test", "Frobmorten"]

    [table]
    str1 = "Another test!"
    str2 = "Untouched"

    [[configs]]
    name = "%TOREP%/a"

    [[configs]]
    name = "%TOREP%/b"

    [[configs]]
    name = "%TOREP%/C"

    [[configs]]
    name = "%TOREP%/D"
    )toml"sv);

  EXPECT_FALSE(config.ReplaceStringPlaceholders({}));
  EXPECT_FALSE(config.ReplaceStringPlaceholders({{"no-such-text"sv, "bar"sv}}));
  // Invalid search string
  EXPECT_THROW(config.ReplaceStringPlaceholders({{""sv, "replace"sv}}),
               std::runtime_error);

  // Replace words
  EXPECT_TRUE(config.ReplaceStringPlaceholders(
      {{"test"sv, "123"sv}, {"world"sv, "replacement"sv}}));
  // Already replaced
  EXPECT_FALSE(config.ReplaceStringPlaceholders(
      {{"test"sv, "123"sv}, {"world"sv, "replacement"sv}}));

  EXPECT_EQ("", config.GetString("str1"sv));
  EXPECT_EQ("This is a 123", config.GetString("str2"sv));
  EXPECT_EQ("Hello replacement!", config.GetString("str3"sv));
  EXPECT_EQ(123, config.GetInteger32("value"sv));
  EXPECT_EQ("List 123", config.GetString("str_list[0]"sv));
  EXPECT_EQ("Frobmorten", config.GetString("str_list[1]"sv));
  EXPECT_EQ("Another 123!", config.GetString("table.str1"sv));
  EXPECT_EQ("Untouched", config.GetString("table.str2"sv));
  EXPECT_EQ("%TOREP%/C", config.GetString("configs[2].name"sv));

  EXPECT_TRUE(config.ReplaceStringPlaceholders({{"%TOREP%"sv, "..."sv}}));
  EXPECT_EQ(".../a", config.GetString("configs[0].name"sv));
  EXPECT_EQ(".../b", config.GetString("configs[1].name"sv));
  EXPECT_EQ(".../C", config.GetString("configs[2].name"sv));
  EXPECT_EQ(".../D", config.GetString("configs[3].name"sv));
}

TEST(ConfigTest, ConfigConstruction) {
  const std::string fname =
      wkf::FullFile(wkf::DirName(__FILE__), "test-valid1.toml");

  // Force copy construction
  auto config = wkc::LoadTOMLFile(fname);
  wkc::Configuration copy{config};

  EXPECT_TRUE(config.Equals(copy));
  EXPECT_FALSE(config.Empty());
  EXPECT_FALSE(copy.Empty());
  EXPECT_EQ(1, config.GetInteger32("value1"sv));
  EXPECT_EQ(1, copy.GetInteger32("value1"sv));

  // Force move construction
  wkc::Configuration moved{std::move(config)};
  EXPECT_FALSE(copy.Empty());
  EXPECT_EQ(1, moved.GetInteger32("value1"sv));

  // Test copy assignment
  wkc::Configuration tmp{};
  EXPECT_TRUE(tmp.Empty());
  copy = tmp;
  EXPECT_TRUE(tmp.Empty());
  EXPECT_TRUE(copy.Empty());

  tmp.SetBoolean("tbl.val"sv, true);
  EXPECT_FALSE(tmp.Empty());
  EXPECT_TRUE(copy.Empty());
  copy = tmp;
  EXPECT_FALSE(tmp.Empty());
  EXPECT_FALSE(copy.Empty());
  EXPECT_TRUE(tmp.Contains("tbl.val"sv));
  EXPECT_TRUE(copy.Contains("tbl.val"sv));
  EXPECT_TRUE(tmp.GetBoolean("tbl.val"sv));
  EXPECT_TRUE(copy.GetBoolean("tbl.val"sv));

  // Test move assignment
  moved = std::move(tmp);
  EXPECT_FALSE(moved.Empty());
  EXPECT_TRUE(moved.Contains("tbl.val"sv));
  EXPECT_TRUE(moved.GetBoolean("tbl.val"sv));
  EXPECT_FALSE(moved.Contains("value1"sv));  // Previously contained
}

TEST(ConfigTest, LoadingToml) {
  const std::string fname =
      wkf::FullFile(wkf::DirName(__FILE__), "test-valid1.toml");

  // Load valid TOML, then reload its string representation
  const auto config1 = wkc::LoadTOMLFile(fname);
  const auto reloaded = wkc::LoadTOMLString(config1.ToTOML());
  EXPECT_TRUE(config1.Equals(reloaded));
  EXPECT_TRUE(reloaded.Equals(config1));
  // Also the string representations should be equal
  EXPECT_EQ(config1.ToTOML(), reloaded.ToTOML());

  // Load a different configuration:
  const auto config2 = wkc::LoadTOMLString(R"toml(
    param1 = "value"
    param2 = "value"

    param3 = true
    )toml");
  EXPECT_FALSE(config1.Equals(config2));
  EXPECT_FALSE(config2.Equals(config1));

  // Identity check
  EXPECT_TRUE(config1.Equals(config1));
  EXPECT_TRUE(config2.Equals(config2));

  // White space mustn't affect the equality check
  auto config3 = wkc::LoadTOMLString(R"toml(

    param1 =     "value"


    param2 =  "value"

    param3         = true

    )toml");

  EXPECT_FALSE(config1.Equals(config3));
  EXPECT_TRUE(config2.Equals(config3));
  EXPECT_TRUE(config3.Equals(config2));

  // Change the first string parameter
  config3 = wkc::LoadTOMLString(R"toml(
    param1 = "value!"
    param2 = "value"

    param3 = true
    )toml");
  EXPECT_FALSE(config1.Equals(config3));
  EXPECT_FALSE(config2.Equals(config3));
  EXPECT_FALSE(config3.Equals(config2));

  // Change the 3rd parameter type
  config3 = wkc::LoadTOMLString(R"toml(
    param1 = "value"
    param2 = "value"

    param3 = [1, 2]
    )toml"sv);
  EXPECT_FALSE(config1.Equals(config3));
  EXPECT_FALSE(config2.Equals(config3));
  EXPECT_FALSE(config3.Equals(config2));

  const auto empty = wkc::LoadTOMLString(""sv);
  EXPECT_FALSE(empty.Equals(config1));
  EXPECT_FALSE(config1.Equals(empty));

  const auto def = wkc::Configuration();
  EXPECT_TRUE(def.Empty());
  EXPECT_TRUE(empty.Equals(def));

  // Edge cases for TOML loading:
  EXPECT_THROW(wkc::LoadTOMLFile("this-does-not-exist.toml"sv),
               wkc::ParseError);
  try {
    wkc::LoadTOMLFile("this-does-not-exist.toml"sv);
  } catch (const wkc::ParseError &e) {
    const std::string exp_err{
        "Cannot open file. Check path: \"this-does-not-exist.toml\"."};
    EXPECT_EQ(exp_err, std::string(e.what()));
  }

  const std::string fname_invalid =
      wkf::FullFile(wkf::DirName(__FILE__), "test-invalid.toml"sv);
  EXPECT_THROW(wkc::LoadTOMLFile(fname_invalid), wkc::ParseError);
  try {
    wkc::LoadTOMLFile(fname_invalid);
    EXPECT_TRUE(false);
  } catch (const wkc::ParseError &e) {
    EXPECT_TRUE(wks::StartsWith(e.what(), "Error while parsing value: "sv))
        << "Error message was: "sv << e.what();
  }
}

TEST(ConfigTest, InvalidDateTimes) {
  // Leap seconds are not supported.
  EXPECT_THROW(wkc::LoadTOMLString("dt = 1990-12-31T23:59:60Z"),
               wkc::ParseError);
  EXPECT_THROW(wkc::LoadTOMLString("dt = 1990-12-31T15:59:60-08:00"),
               wkc::ParseError);

  // Leap seconds are not supported.
  EXPECT_THROW(wkc::date_time({1990, 12, 31}, {23, 59, 60}), wkc::ValueError);
  EXPECT_THROW(wkc::date_time{"1990-12-31T23:59:60Z"sv}, wkc::ParseError);

  // Unknown Local Offset Convention (-00:00) is not supported, i.e. it
  // will be silently converted to UTC+0.
  EXPECT_NO_THROW(wkc::date_time{"1990-12-31T23:59:59Z"sv});
  EXPECT_NO_THROW(wkc::date_time{"1990-12-31T23:59:59-00:00"sv});
  EXPECT_NO_THROW(wkc::date_time{"1990-12-31T23:59:59+00:00"sv});
  EXPECT_EQ(wkc::date_time{"1990-12-31T23:59:59-00:00"sv},
            wkc::date_time{"1990-12-31T23:59:59+00:00"sv});
}

// TODO Can be properly tested once we have LoadJSONString()
TEST(ConfigTest, LoadingJson) {
  const auto config = wkc::LoadTOMLString(R"toml(
    param1 = "value"
    )toml"sv);
  EXPECT_TRUE(config.ToJSON().length() > 0);
}

// NOLINTEND
