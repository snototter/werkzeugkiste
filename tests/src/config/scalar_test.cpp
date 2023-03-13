#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/strings/strings.h>

#include <cmath>
#include <limits>
#include <sstream>

#include "../test_utils.h"

namespace wkc = werkzeugkiste::config;
namespace wks = werkzeugkiste::strings;

using namespace std::string_view_literals;

// NOLINTBEGIN

TEST(ConfigScalarTest, Integer) {
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

  EXPECT_THROW(config.GetInteger32(" int32_1"sv), wkc::KeyError);
  EXPECT_THROW(config.GetInteger32(" int32_1 "sv), wkc::KeyError);
  EXPECT_THROW(config.GetInteger32(" int32_1 "sv), wkc::KeyError);

  EXPECT_EQ(2147483647, config.GetInteger32("int32_max"sv));
  EXPECT_EQ(-2147483648, config.GetInteger32("int32_min"sv));

  EXPECT_THROW(config.GetInteger32("int32_min_underflow"sv), wkc::TypeError);
  try {
    config.GetInteger32("int32_min_underflow"sv);
  } catch (const wkc::TypeError &e) {
    EXPECT_TRUE(wks::StartsWith(e.what(),
        "Cannot convert numeric parameter `int32_min_underflow` to `int32_t`. Underflow"sv))
        << "Actual exception message: " << e.what();
  }

  EXPECT_THROW(config.GetInteger32("int32_max_overflow"sv), wkc::TypeError);
  try {
    config.GetInteger32("int32_max_overflow"sv);
  } catch (const wkc::TypeError &e) {
    EXPECT_TRUE(wks::StartsWith(e.what(),
        "Cannot convert numeric parameter `int32_max_overflow` to `int32_t`. Overflow"sv))
        << "Actual exception message: " << e.what();
  }

  EXPECT_THROW(
      config.GetOptionalInteger32("int32_min_underflow"sv), wkc::TypeError);
  EXPECT_THROW(
      config.GetOptionalInteger32("int32_max_overflow"sv), wkc::TypeError);

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
  EXPECT_EQ(
      +2147483648, config.GetOptionalInteger64("int32_max_overflow"sv).value());

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

TEST(ConfigScalarTest, FloatingPoint) {
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

TEST(ConfigScalarTest, LookupScalars) {
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
  EXPECT_EQ(
      wkc::date(2023, 01, 02), config.GetOptionalDate("dates.day"sv).value());

  EXPECT_THROW(config.GetDate("str"sv), wkc::TypeError);
  EXPECT_THROW(
      config.GetDateOr("str"sv, wkc::date{1234, 12, 30}), wkc::TypeError);
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

TEST(ConfigScalarTest, SetBoolean) {
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

  // White space in keys is not allowed when setting a value
  EXPECT_THROW(config.SetBoolean(""sv, true), wkc::KeyError);
  EXPECT_THROW(config.SetBoolean(" invalid-key"sv, true), wkc::KeyError);
  EXPECT_THROW(config.SetBoolean("invalid-key "sv, true), wkc::KeyError);
  EXPECT_THROW(config.SetBoolean("invalid key"sv, true), wkc::KeyError);

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

  // We can't add another parameter as a "child" of a scalar value
  EXPECT_THROW(config.SetBoolean("a.string.below.bool"sv, true), wkc::KeyError);

  // Similarly, automatically creating an array as (one of the) parent(s) is
  // also not supported (how should we initialize array elements up to the
  // requested index, anyhow?). Instead, we would have to first create a
  // list, and then fill it by ourselves. For this, refer to the
  // `ConfigListTest` test suite (list_test.cpp).
  EXPECT_THROW(config.SetBoolean("no_such_array[3]"sv, true), wkc::KeyError);
  // Creating a table within an existing array is also not supported:
  EXPECT_THROW(config.SetBoolean("array[3].bool"sv, true), wkc::KeyError);
  EXPECT_THROW(
      config.SetBoolean("array[4].another_table.value"sv, true), wkc::KeyError);

  // Changing the type of an existing array item is also not supported:
  EXPECT_THROW(config.SetBoolean("array[2]"sv, true), wkc::TypeError);

  // But setting an existing array element is supported:
  EXPECT_NO_THROW(config.SetBoolean("booleans[1]"sv, true));
  EXPECT_EQ(true, config.GetBoolean("booleans[0]"sv));
  EXPECT_EQ(true, config.GetBoolean("booleans[1]"sv));
  EXPECT_EQ(true, config.GetBoolean("booleans[2]"sv));

  EXPECT_EQ(false, config.GetBoolean("array[2].bool"sv));
  EXPECT_NO_THROW(config.SetBoolean("array[2].bool"sv, true));
  EXPECT_EQ(true, config.GetBoolean("array[2].bool"sv));
}

TEST(ConfigScalarTest, SetNonBooleanScalars) {
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

  // White space in keys is not allowed when setting a value
  EXPECT_THROW(config.SetInteger32(""sv, 1), wkc::KeyError);
  EXPECT_THROW(config.SetInteger32(" invalid-key"sv, 1), wkc::KeyError);
  EXPECT_THROW(config.SetInteger32("invalid-key "sv, 1), wkc::KeyError);
  EXPECT_THROW(config.SetInteger32("invalid key"sv, 1), wkc::KeyError);

  EXPECT_THROW(config.SetInteger64(""sv, 1), wkc::KeyError);
  EXPECT_THROW(config.SetInteger64(" invalid-key"sv, 17), wkc::KeyError);
  EXPECT_THROW(config.SetInteger64("invalid-key "sv, 17), wkc::KeyError);
  EXPECT_THROW(config.SetInteger64("invalid key"sv, 17), wkc::KeyError);

  EXPECT_THROW(config.SetDouble(""sv, 1.0), wkc::KeyError);
  EXPECT_THROW(config.SetDouble(" invalid-key"sv, 0.1), wkc::KeyError);
  EXPECT_THROW(config.SetDouble("invalid-key "sv, 0.1), wkc::KeyError);
  EXPECT_THROW(config.SetDouble("invalid key"sv, 0.1), wkc::KeyError);

  EXPECT_THROW(config.SetString(""sv, "value"sv), wkc::KeyError);
  EXPECT_THROW(config.SetString(" invalid-key"sv, "value"sv), wkc::KeyError);
  EXPECT_THROW(config.SetString("invalid-key "sv, "value"sv), wkc::KeyError);
  EXPECT_THROW(config.SetString("invalid key"sv, "value"sv), wkc::KeyError);

  // Change a double
  EXPECT_DOUBLE_EQ(1.5, config.GetDouble("section.float"sv));
  EXPECT_NO_THROW(config.SetDouble("section.float"sv, 0.01));
  EXPECT_DOUBLE_EQ(0.01, config.GetDouble("section.float"sv));

  // We cannot change the type of an existing parameter
  EXPECT_THROW(config.SetDouble("integer"sv, 1.5), wkc::TypeError);
  // But it can be set if the value is convertible
  EXPECT_NO_THROW(config.SetDouble("integer"sv, 3.0));
  EXPECT_EQ(wkc::ConfigType::Integer, config.Type("integer"sv));
  EXPECT_EQ(3, config.GetInteger32("integer"sv));

  // Set a string:
  EXPECT_EQ("value", config.GetString("section.string"sv));
  EXPECT_NO_THROW(config.SetString("section.string"sv, "frobmorten"sv));
  EXPECT_EQ("frobmorten", config.GetString("section.string"sv));
  EXPECT_THROW(config.SetString("section."sv, "value"sv), wkc::KeyError);

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

  // Update date
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

  // White space in keys is not allowed when setting a value
  EXPECT_THROW(config.SetDate(" invalid-key"sv, day), wkc::KeyError);
  EXPECT_THROW(config.SetTime("invalid-key "sv, tm), wkc::KeyError);
  EXPECT_THROW(config.SetDateTime("invalid key"sv, dt), wkc::KeyError);
}

TEST(ConfigScalarTest, ReplaceListElements) {
  auto config = wkc::LoadTOMLString(R"toml(
    ints = [1, 2, 3, 4]
    strs = ["This", "is", "a", "string"]
    mixed = [1, 2.5, "three"]
    )toml"sv);

  //---------------------------------------------------------------------------
  // Integer list.

  // Replace a single element in the list:
  EXPECT_NO_THROW(config.SetInteger32("ints[2]"sv, -2));
  EXPECT_EQ(-2, config.GetInteger32("ints[2]"sv));

  EXPECT_THROW(config.SetInteger32("ints [2]"sv, -2), wkc::KeyError);

  // A compatible/convertible value can also be used:
  EXPECT_NO_THROW(config.SetDouble("ints[0]"sv, 5.0));
  EXPECT_EQ(5, config.GetInteger32("ints[0]"sv));
  EXPECT_EQ(wkc::ConfigType::Integer, config.Type("ints[0]"sv));

  EXPECT_THROW(config.SetDouble("ints [0] "sv, 5.0), wkc::KeyError);

  EXPECT_THROW(config.SetBoolean("ints[0]"sv, true), wkc::TypeError);
  EXPECT_THROW(config.SetString("ints[1]"sv, "test"sv), wkc::TypeError);

  //---------------------------------------------------------------------------
  // String list.

  EXPECT_EQ("is", config.GetString("strs[1]"sv));
  EXPECT_NO_THROW(config.SetString("strs[1]"sv, "was"sv));
  EXPECT_EQ("was", config.GetString("strs[1]"sv));

  EXPECT_NO_THROW(config.SetString("strs[1]"sv, ""sv));
  EXPECT_EQ("", config.GetString("strs[1]"sv));

  EXPECT_THROW(config.SetBoolean("strs[0]"sv, true), wkc::TypeError);
  EXPECT_THROW(config.SetInteger32("strs[1]"sv, 1), wkc::TypeError);

  //---------------------------------------------------------------------------
  // Mixed value list - such a list cannot be created programmatically, but it
  // can be loaded from an existing TOML/libconfig/JSON configuration.

  // Changing a type is not supported, but we can replace a value by a
  // compatible type.
  EXPECT_THROW(config.SetBoolean("mixed[0]"sv, true), wkc::TypeError);
  EXPECT_THROW(config.SetDouble("mixed[0]"sv, -4.5), wkc::TypeError);
  EXPECT_NO_THROW(config.SetDouble("mixed[0]"sv, -4.0));
  EXPECT_EQ(-4, config.GetInteger32("mixed[0]"sv));
  EXPECT_EQ(wkc::ConfigType::Integer, config.Type("mixed[0]"sv));

  EXPECT_THROW(config.SetBoolean("mixed[1]"sv, true), wkc::TypeError);
  EXPECT_THROW(config.SetString("mixed[1]"sv, "3/2"), wkc::TypeError);
  EXPECT_NO_THROW(config.SetInteger64("mixed[1]"sv, -12345));
  EXPECT_DOUBLE_EQ(-12345.0, config.GetDouble("mixed[1]"sv));
  EXPECT_EQ(wkc::ConfigType::FloatingPoint, config.Type("mixed[1]"sv));

  EXPECT_NO_THROW(config.SetString("mixed[2]"sv, "done"));
  EXPECT_EQ("done", config.GetString("mixed[2]"sv));
  EXPECT_THROW(config.SetBoolean("mixed[2]"sv, true), wkc::TypeError);
  EXPECT_THROW(config.SetDouble("mixed[2]"sv, 3.0), wkc::TypeError);
  EXPECT_EQ(wkc::ConfigType::String, config.Type("mixed[2]"sv));
}

TEST(ConfigScalarTest, Delete) {
  auto config = wkc::LoadTOMLString(R"toml(
    int = 12345
    str = "This is a string"

    [section]
    flt = 1.5
    arr = [1, 2, 3]
    lst = [1, true, "a string"]
    )toml"sv);

  EXPECT_THROW(config.Delete(""sv), wkc::KeyError);
  EXPECT_THROW(config.Delete("no-such-key"sv), wkc::KeyError);
  EXPECT_THROW(config.Delete("section."sv), wkc::KeyError);
  EXPECT_THROW(config.Delete("section.\"\""sv), wkc::KeyError);

  EXPECT_TRUE(config.Contains("int"sv));
  EXPECT_NO_THROW(config.Delete("int"sv));
  EXPECT_FALSE(config.Contains("int"sv));

  EXPECT_TRUE(config.Contains("str"sv));
  EXPECT_NO_THROW(config.Delete("str"sv));
  EXPECT_FALSE(config.Contains("str"sv));

  EXPECT_TRUE(config.Contains("section.flt"sv));
  EXPECT_NO_THROW(config.Delete("section.flt"sv));
  EXPECT_FALSE(config.Contains("section.flt"sv));

  EXPECT_THROW(config.Delete("section.arr[0]"sv), wkc::KeyError);
  EXPECT_EQ(3, config.Size("section.arr"sv));

  EXPECT_NO_THROW(config.Delete("section.arr"sv));
  EXPECT_FALSE(config.Contains("section.arr"sv));

  EXPECT_EQ(1, config.Size("section"sv));
  EXPECT_NO_THROW(config.Delete("section"sv));
  EXPECT_FALSE(config.Contains("section"sv));
  EXPECT_TRUE(config.Empty());
}
// NOLINTEND
