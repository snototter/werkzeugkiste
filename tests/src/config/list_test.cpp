#include <werkzeugkiste/config/configuration.h>

#include "../test_utils.h"

namespace wkc = werkzeugkiste::config;

using namespace std::string_view_literals;

// NOLINTBEGIN

TEST(ConfigListTest, GetEmptyLists) {
  auto config = wkc::LoadTOMLString(R"toml(
    empty = []
    )toml");

  EXPECT_TRUE(config.GetBooleanList("empty"sv).empty());
  EXPECT_TRUE(config.GetInteger32List("empty"sv).empty());
  EXPECT_TRUE(config.GetInteger64List("empty"sv).empty());
  EXPECT_TRUE(config.GetDoubleList("empty"sv).empty());
  EXPECT_TRUE(config.GetStringList("empty"sv).empty());
  EXPECT_TRUE(config.GetDateList("empty"sv).empty());
  EXPECT_TRUE(config.GetTimeList("empty"sv).empty());
  EXPECT_TRUE(config.GetDateTimeList("empty"sv).empty());

  // TODO extend with nested lists

  // An empty list can be set to any type -> it will still have no type.
  EXPECT_NO_THROW(config.SetBooleanList("empty"sv, {}));
  EXPECT_TRUE(config.GetBooleanList("empty"sv).empty());
  EXPECT_TRUE(config.GetDoubleList("empty"sv).empty());
  EXPECT_TRUE(config.GetStringList("empty"sv).empty());

  EXPECT_NO_THROW(config.SetStringList("empty"sv, {}));
  EXPECT_TRUE(config.GetBooleanList("empty"sv).empty());
  EXPECT_TRUE(config.GetDoubleList("empty"sv).empty());
  EXPECT_TRUE(config.GetStringList("empty"sv).empty());

  // But once elements are inserted, the list is typed.
  EXPECT_NO_THROW(config.SetDoubleList("empty"sv, {1.5, 2.0}));
  EXPECT_THROW(config.GetBooleanList("empty"sv), wkc::TypeError);
  EXPECT_THROW(config.GetStringList("empty"sv), wkc::TypeError);
  EXPECT_EQ(2, config.GetDoubleList("empty"sv).size());
}

TEST(ConfigListTest, SetEmptyLists) {
  auto config = wkc::LoadTOMLString(R"toml(
    empty = []
    ints = [1, 2]
    mixed = [1, "two", 3.5]
    )toml");

  // An empty list can be set to any type.
  EXPECT_NO_THROW(config.SetBooleanList("empty"sv, {}));
  EXPECT_NO_THROW(config.SetInteger32List("empty"sv, {}));
  EXPECT_NO_THROW(config.SetInteger64List("empty"sv, {}));
  EXPECT_NO_THROW(config.SetDoubleList("empty"sv, {}));
  EXPECT_NO_THROW(config.SetStringList("empty"sv, {}));
  EXPECT_NO_THROW(config.SetDateList("empty"sv, {}));
  EXPECT_NO_THROW(config.SetTimeList("empty"sv, {}));
  EXPECT_NO_THROW(config.SetDateTimeList("empty"sv, {}));

  // An existing list can be replaced by an empty list of another type,
  // because we assume that an empty list "doesn't have a type". After
  // saving such a configuration to disk, there is no way to distinguish
  // an empty string list from an empty list of dates...
  EXPECT_NO_THROW(config.SetBooleanList("ints"sv, {}));
  EXPECT_TRUE(config.GetBooleanList("ints"sv).empty());
  EXPECT_TRUE(config.GetInteger32List("ints"sv).empty());
  // Restore the integer list to check replacing it by other empty lists.
  config.SetInteger32List("ints"sv, {1, 2});
  EXPECT_NO_THROW(config.SetDoubleList("ints"sv, {}));
  config.SetInteger32List("ints"sv, {1, 2});
  EXPECT_NO_THROW(config.SetStringList("ints"sv, {}));
  config.SetInteger32List("ints"sv, {1, 2});
  EXPECT_NO_THROW(config.SetDateList("ints"sv, {}));
  config.SetInteger32List("ints"sv, {1, 2});
  EXPECT_NO_THROW(config.SetTimeList("ints"sv, {}));
  config.SetInteger32List("ints"sv, {1, 2});
  EXPECT_NO_THROW(config.SetDateTimeList("ints"sv, {}));

  // A mixed list cannot be restored programmatically. Thus, we can only
  // replace it once.
  EXPECT_NO_THROW(config.SetDateList("mixed"sv, {}));
  EXPECT_TRUE(config.GetDateList("mixed"sv).empty());
  EXPECT_TRUE(config.GetBooleanList("mixed"sv).empty());
}

TEST(ConfigListTest, GetLists) {
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

    days = [1999-10-11, 2000-01-22]

    times = [23:00:59, 08:30:10]

    dts = [
      2023-02-14T21:08:23,
      1998-02-14T22:08:23.880+01:00,
    ]

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
  EXPECT_THROW(
      config.GetInteger32List("not-a-list.no-such-key"sv), wkc::KeyError);

  EXPECT_THROW(config.GetInteger64List("an_int"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger64List("not-a-list"sv), wkc::TypeError);
  EXPECT_THROW(
      config.GetInteger64List("not-a-list.no-such-key"sv), wkc::KeyError);

  EXPECT_THROW(config.GetDoubleList("an_int"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDoubleList("not-a-list"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDoubleList("not-a-list.no-such-key"sv), wkc::KeyError);

  EXPECT_THROW(config.GetStringList("an_int"sv), wkc::TypeError);
  EXPECT_THROW(config.GetStringList("not-a-list"sv), wkc::TypeError);
  EXPECT_THROW(config.GetStringList("not-a-list.no-such-key"sv), wkc::KeyError);

  // Cannot load inhomogeneous arrays (would need to load each element with its
  // corresponding type separately):
  EXPECT_THROW(config.GetInteger32List("mixed_types"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger64List("mixed_types"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDoubleList("mixed_types"sv), wkc::TypeError);
  EXPECT_THROW(config.GetStringList("mixed_types"sv), wkc::TypeError);
  EXPECT_EQ(1, config.GetInteger32("mixed_types[0]"sv));
  EXPECT_EQ(2, config.GetInteger32("mixed_types[1]"sv));
  EXPECT_EQ("framboozle", config.GetString("mixed_types[2]"sv));

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

  // Load dates:
  const auto days = config.GetDateList("days"sv);
  EXPECT_EQ(2, days.size());
  EXPECT_EQ(wkc::date(1999, 10, 11), days[0]);
  EXPECT_EQ(wkc::date(2000, 1, 22), days[1]);

  const auto times = config.GetTimeList("times"sv);
  EXPECT_EQ(2, times.size());
  EXPECT_EQ(wkc::time(23, 0, 59), times[0]);
  EXPECT_EQ(wkc::time(8, 30, 10), times[1]);

  const auto dts = config.GetDateTimeList("dts"sv);
  EXPECT_EQ(2, dts.size());
  EXPECT_EQ(wkc::date_time{"2023-02-14T21:08:23"sv}, dts[0]);
  EXPECT_NE(wkc::date_time{"2023-02-14T21:08:23"sv}, dts[1]);
  EXPECT_EQ(wkc::date_time{"1998-02-14T22:08:23.880+01:00"sv}, dts[1]);
}

TEST(ConfigListTest, NumericList) {
  auto config = wkc::LoadTOMLString(R"toml(
    mixed_int_flt = [1, 2, 3, 4.5, 5]

    mixed_types = [1, 2, "framboozle"]

    nested_lst = [1, 2, [3, 4], "frobmorten", {name = "fail"}]
    )toml");

  // Create an integer list
  EXPECT_FALSE(config.Contains("ints"sv));
  EXPECT_NO_THROW(config.SetInteger32List("ints"sv, {-3, 0}));
  EXPECT_TRUE(config.Contains("ints"sv));
  auto ints32 = config.GetInteger32List("ints"sv);
  EXPECT_EQ(2, ints32.size());
  EXPECT_EQ(-3, ints32[0]);
  EXPECT_EQ(0, ints32[1]);

  // Update the integer list:
  EXPECT_NO_THROW(config.SetInteger64List("ints"sv, {1, -42, 17}));
  ints32 = config.GetInteger32List("ints"sv);
  EXPECT_EQ(3, ints32.size());
  EXPECT_EQ(1, ints32[0]);
  EXPECT_EQ(-42, ints32[1]);
  EXPECT_EQ(17, ints32[2]);
  // Internally, integers are 64-bit, thus the following
  // (int32::max + 1; int32::min - 1) will not throw.
  EXPECT_NO_THROW(config.SetInteger64List("ints"sv, {2147483648, -2147483649}));
  // But it can no longer be loaded as 32-bit integers.
  EXPECT_THROW(config.GetInteger32List("ints"sv), wkc::TypeError);
  auto ints64 = config.GetInteger64List("ints"sv);
  EXPECT_EQ(2, ints64.size());
  EXPECT_EQ(2147483648L, ints64[0]);
  EXPECT_EQ(-2147483649L, ints64[1]);

  // A list *item* can be replaced by a compatible/convertible value (see
  // ConfigScalarTest.ReplaceListElements for more details).
  EXPECT_THROW(config.SetDouble("ints[0]"sv, 32.8), wkc::TypeError);
  EXPECT_NO_THROW(config.SetDouble("ints[0]"sv, 32.0));
  EXPECT_EQ(32, config.GetInteger32("ints[0]"sv));
  EXPECT_EQ(wkc::ConfigType::Integer, config.Type("ints[0]"sv));

  // A list of integers can be replaced by a list of compatible/convertible
  // floating point values...
  EXPECT_NO_THROW(config.SetDoubleList("ints"sv, {1.0, 5.0}));
  ints64 = config.GetInteger64List("ints"sv);
  EXPECT_EQ(2, ints64.size());
  EXPECT_EQ(1, ints64[0]);
  EXPECT_EQ(5, ints64[1]);
  EXPECT_EQ(wkc::ConfigType::Integer, config.Type("ints[0]"sv));
  // ... but it cannot be replaced by a list of "real" floating points (which
  // are not representable by an int64, as this would require changing the
  // type of the whole list.
  EXPECT_THROW(config.SetDoubleList("ints"sv, {1.5, 5.0}), wkc::TypeError);

  // Sanity checks: can't replace an integer list by bool/string/empty
  // string list:
  EXPECT_THROW(config.SetBooleanList("ints"sv, {true, false}), wkc::TypeError);
  EXPECT_THROW(config.SetStringList("ints"sv, {"test"}), wkc::TypeError);

  // A mixed list that only contains numbers can be replaced by a homogeneous
  // list. Its type, however, will be floating point afterwards:
  EXPECT_NO_THROW(config.SetInteger32List("mixed_int_flt"sv, {1, 2}));
  ints32 = config.GetInteger32List("mixed_int_flt"sv);
  EXPECT_EQ(2, ints32.size());
  EXPECT_EQ(1, ints32[0]);
  EXPECT_EQ(2, ints32[1]);
  EXPECT_EQ(wkc::ConfigType::FloatingPoint, config.Type("mixed_int_flt[0]"sv));
  EXPECT_EQ(wkc::ConfigType::FloatingPoint, config.Type("mixed_int_flt[1]"sv));

  // ... but for all other types/mixtures, the type cannot be changed.
  EXPECT_THROW(
      config.SetInteger64List("mixed_types"sv, {1, 3, -17}), wkc::TypeError);

  EXPECT_NO_THROW(config.SetBooleanList("flags"sv, {true, false}));
  EXPECT_THROW(config.SetInteger32List("flags"sv, {1, 3, -17}), wkc::TypeError);
  EXPECT_THROW(config.SetStringList("flags"sv, {"abc"}), wkc::TypeError);

  EXPECT_THROW(
      config.SetDoubleList("nested_lst"sv, {1.0, -0.5}), wkc::TypeError);
}

TEST(ConfigListTest, SetBooleanList) {
  wkc::Configuration config{};

  // Create a boolean list
  EXPECT_FALSE(config.Contains("flags"sv));
  EXPECT_NO_THROW(config.SetBooleanList("flags"sv, {true, false, true}));
  EXPECT_TRUE(config.Contains("flags"sv));
  auto flags = config.GetBooleanList("flags"sv);
  EXPECT_EQ(3, flags.size());
  EXPECT_TRUE(flags[0]);
  EXPECT_FALSE(flags[1]);
  EXPECT_TRUE(flags[2]);

  EXPECT_THROW(config.GetInteger32List("flags"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger64List("flags"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDoubleList("flags"sv), wkc::TypeError);
  EXPECT_THROW(config.GetStringList("flags"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDateList("flags"sv), wkc::TypeError);
  EXPECT_THROW(config.GetTimeList("flags"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDateTimeList("flags"sv), wkc::TypeError);

  // Update the boolean list
  EXPECT_NO_THROW(config.SetBooleanList("flags"sv, {false, true}));
  flags = config.GetBooleanList("flags"sv);
  EXPECT_EQ(2, config.Size("flags"sv));
  EXPECT_EQ(2, flags.size());
  EXPECT_FALSE(flags[0]);
  EXPECT_TRUE(flags[1]);
}

TEST(ConfigListTest, SetStringList) {
  wkc::Configuration config{};

  EXPECT_NO_THROW(config.SetStringList("strs"sv, {}));
  EXPECT_TRUE(config.GetStringList("strs"sv).empty());

  EXPECT_NO_THROW(config.SetStringList("strs"sv, {"Hello"}));
  EXPECT_EQ(1, config.GetStringList("strs"sv).size());

  EXPECT_NO_THROW(config.SetStringList("strs"sv, {"Hello", "World"}));
  const auto strs = config.GetStringList("strs"sv);
  EXPECT_EQ(2, strs.size());
  EXPECT_EQ(2, config.Size("strs"sv));
  EXPECT_EQ("Hello", strs[0]);
  EXPECT_EQ("Hello", config.GetString("strs[0]"sv));
  EXPECT_EQ("World", strs[1]);
  EXPECT_EQ("World", config.GetString("strs[1]"sv));

  EXPECT_THROW(config.GetBooleanList("strs"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDouble("strs[0]"sv), wkc::TypeError);
}

TEST(ConfigListTest, SetDateList) {
  wkc::Configuration config{};

  // Empty list
  EXPECT_NO_THROW(config.SetDateList("empty"sv, {}));
  EXPECT_TRUE(config.GetDateList("empty"sv).empty());

  // Set/get list
  const std::vector<wkc::date> days = {
      wkc::date{1900, 1, 3}, wkc::date{2000, 2, 29}, wkc::date{2023, 2, 28}};
  EXPECT_NO_THROW(config.SetDateList("days"sv, days));
  const auto lookup = config.GetDateList("days"sv);
  EXPECT_EQ(days.size(), lookup.size());
  EXPECT_EQ(days.size(), config.Size("days"sv));
  for (std::size_t i = 0; i < days.size(); ++i) {
    EXPECT_EQ(days[i], lookup[i]);
  }
  EXPECT_EQ(days[1], config.GetDate("days[1]"sv));

  // Replace a single item
  const wkc::date day{"1234-5-6"sv};
  EXPECT_NO_THROW(config.SetDate("days[1]"sv, day));
  EXPECT_EQ(day, config.GetDate("days[1]"sv));

  // Invalid access
  EXPECT_THROW(config.GetTimeList("days"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDateTimeList("days"sv), wkc::TypeError);
  EXPECT_THROW(config.GetBooleanList("days"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDouble("days[0]"sv), wkc::TypeError);
  EXPECT_THROW(config.GetTime("days[0]"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDateTime("days[0]"sv), wkc::TypeError);
}

TEST(ConfigListTest, SetTimeList) {
  wkc::Configuration config{};

  // Empty list
  EXPECT_NO_THROW(config.SetTimeList("empty"sv, {}));
  EXPECT_TRUE(config.GetTimeList("empty"sv).empty());

  // Set/get list
  const std::vector<wkc::time> times = {
      wkc::time{0, 0}, wkc::time{12, 0}, wkc::time{23, 59, 59}};
  EXPECT_NO_THROW(config.SetTimeList("times"sv, times));
  const auto lookup = config.GetTimeList("times"sv);
  EXPECT_EQ(times.size(), lookup.size());
  EXPECT_EQ(times.size(), config.Size("times"sv));
  for (std::size_t i = 0; i < times.size(); ++i) {
    EXPECT_EQ(times[i], lookup[i]);
  }
  EXPECT_EQ(times[1], config.GetTime("times[1]"sv));

  // Replace a single item
  const wkc::time tm{"13:37"sv};
  EXPECT_NO_THROW(config.SetTime("times[2]"sv, tm));
  EXPECT_EQ(tm, config.GetTime("times[2]"sv));

  // Invalid access
  EXPECT_THROW(config.GetDateList("times"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDateTimeList("times"sv), wkc::TypeError);
  EXPECT_THROW(config.GetBooleanList("times"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDouble("times[0]"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDate("times[0]"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDateTime("times[0]"sv), wkc::TypeError);
}

TEST(ConfigListTest, SetDateTimeList) {
  wkc::Configuration config{};

  // Empty list
  EXPECT_NO_THROW(config.SetDateTimeList("empty"sv, {}));
  EXPECT_TRUE(config.GetDateTimeList("empty"sv).empty());

  // Set/get list
  const std::vector<wkc::date_time> dts = {
      wkc::date_time{"2023-02-14T21:08:23Z"sv},
      wkc::date_time{"2023-02-14_21:08:23.880Z"sv},
      wkc::date_time{"2024-02-29 00:45:12.123+01:00"sv},
      wkc::date_time{"2024-02-28 23:45:12.123Z"sv}};
  EXPECT_NO_THROW(config.SetDateTimeList("dts"sv, dts));
  const auto lookup = config.GetDateTimeList("dts"sv);
  EXPECT_EQ(dts.size(), lookup.size());
  EXPECT_EQ(dts.size(), config.Size("dts"sv));
  for (std::size_t i = 0; i < dts.size(); ++i) {
    EXPECT_EQ(dts[i], lookup[i]);
  }
  EXPECT_EQ(dts[1], config.GetDateTime("dts[1]"sv));

  // Replace a single item
  EXPECT_NO_THROW(config.SetDateTime("dts[0]"sv, dts[3]));
  EXPECT_EQ(dts[3], config.GetDateTime("dts[0]"sv));

  // Invalid access
  EXPECT_THROW(config.GetDateList("dts"sv), wkc::TypeError);
  EXPECT_THROW(config.GetTimeList("dts"sv), wkc::TypeError);
  EXPECT_THROW(config.GetBooleanList("dts"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDouble("dts[0]"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDate("dts[0]"sv), wkc::TypeError);
  EXPECT_THROW(config.GetTime("dts[0]"sv), wkc::TypeError);
}

TEST(ConfigListTest, MixedList) {
  auto config = wkc::LoadTOMLString(R"toml(
    numbers = [1, 2.5]

    types = [true, -42, 4.2, "foo", 2011-09-10]
    )toml");

  EXPECT_EQ(2, config.Size("numbers"sv));
  EXPECT_EQ(5, config.Size("types"sv));

  EXPECT_THROW(config.GetBooleanList("numbers"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDateList("numbers"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger32List("numbers"sv), wkc::TypeError);
  const auto flts = config.GetDoubleList("numbers"sv);
  EXPECT_EQ(2, flts.size());
  EXPECT_DOUBLE_EQ(1.0, flts[0]);
  EXPECT_DOUBLE_EQ(2.5, flts[1]);

  // The mixed list cannot be loaded as a homogeneous type.
  EXPECT_THROW(config.GetBooleanList("types"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDateList("types"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger32List("types"sv), wkc::TypeError);

  // But each element can be looked up individually.
  EXPECT_TRUE(config.GetBoolean("types[0]"sv));
  EXPECT_EQ(-42, config.GetInteger64("types[1]"sv));
  EXPECT_DOUBLE_EQ(4.2, config.GetDouble("types[2]"sv));
  EXPECT_EQ("foo", config.GetString("types[3]"sv));
  EXPECT_EQ(wkc::date{"2011-09-10"sv}, config.GetDate("types[4]"sv));

  // Individual elements can be replaced (but only by a compatible/convertible
  // value)
  EXPECT_THROW(config.SetString("types[0]"sv, ""sv), wkc::TypeError);
  EXPECT_NO_THROW(config.SetBoolean("types[0]"sv, false));
  EXPECT_FALSE(config.GetBoolean("types[0]"sv));

  EXPECT_THROW(config.SetDouble("types[1]"sv, 1.23), wkc::TypeError);
  EXPECT_NO_THROW(config.SetDouble("types[1]"sv, 17.0));
  EXPECT_EQ(17, config.GetInteger32("types[1]"sv));

  EXPECT_THROW(
      config.SetTime("types[4]"sv, wkc::time{"08:00"sv}), wkc::TypeError);
  const wkc::date day{"31.12.1234"};
  EXPECT_NO_THROW(config.SetDate("types[4]"sv, day));
  EXPECT_EQ(day, config.GetDate("types[4]"sv));

  // The mixed list cannot be replaced by a homogeneous list.
  EXPECT_THROW(config.SetBooleanList("types"sv, {true}), wkc::TypeError);
  EXPECT_THROW(config.SetStringList("types"sv, {"test"}), wkc::TypeError);
  // But it can be replaced by an empty list of any type.
  EXPECT_NO_THROW(config.SetDateList("types"sv, {}));
  EXPECT_TRUE(config.GetDateList("types"sv).empty());
  EXPECT_TRUE(config.GetStringList("types"sv).empty());
}

TEST(ConfigListTest, Size) {
  const auto config = wkc::LoadTOMLString(R"toml(
    mixed_int_flt = [1, 2, 3, 4.5, 5]

    mixed_types = [1, 2, "framboozle"]

    nested_lst = [1, 2, [3, 4], "frobmorten", {name = "fail"}]

    str = "value"

    [scalars]
    flt1 = 1.0
    flt2 = 2.0
    )toml");

  EXPECT_EQ(5, config.Size());
  EXPECT_EQ(2, config.Size("scalars"sv));

  EXPECT_EQ(5, config.Size("mixed_int_flt"sv));
  EXPECT_EQ(3, config.Size("mixed_types"sv));
  EXPECT_EQ(5, config.Size("nested_lst"sv));

  EXPECT_THROW(config.Size("no-such-key"sv), wkc::KeyError);
  EXPECT_THROW(config.Size("str"sv), wkc::TypeError);

  EXPECT_THROW(config.Size("nested_lst[0]"sv), wkc::TypeError);
  EXPECT_EQ(2, config.Size("nested_lst[2]"sv));
  EXPECT_EQ(1, config.Size("nested_lst[4]"sv));
}

TEST(ConfigListTest, CreateMixedList) {
  auto config = wkc::LoadTOMLString(R"toml(
    empty = []
    mixed = [1, "two", 3.5]
    str = "value"
    )toml");

  // "CreateList" cannot replace an existing parameter
  EXPECT_THROW(config.CreateList("empty"sv), wkc::KeyError);
  EXPECT_THROW(config.CreateList("mixed"sv), wkc::KeyError);

  // We cannot "create" an element of a list, only "append"
  EXPECT_THROW(config.SetBoolean("empty[0]"sv, true), wkc::KeyError);
  EXPECT_THROW(config.SetDouble("empty[5]"sv, 1.0), wkc::KeyError);

  EXPECT_THROW(config.Append("empty[0]"sv, 1.0), wkc::KeyError);
  EXPECT_NO_THROW(config.Append("empty"sv, true));
  EXPECT_EQ(1, config.Size("empty"sv));

  // But we can append any item to an existing list
  // TODO

  // Create a new mixed type list programmatically
  // TODO adjust docs - this is now supported
  EXPECT_NO_THROW(config.CreateList("lst"sv));
  EXPECT_TRUE(config.Contains("lst"sv));
  EXPECT_EQ(0, config.Size("lst"sv));
  EXPECT_EQ(wkc::ConfigType::List, config.Type("lst"sv));

  EXPECT_THROW(config.Append("str"sv, true), wkc::KeyError);
  EXPECT_THROW(config.SetBoolean("lst[0]"sv, true), wkc::KeyError);

  EXPECT_NO_THROW(config.Append("lst"sv, true));
  EXPECT_EQ(1, config.Size("lst"sv));
  EXPECT_TRUE(config.GetBoolean("lst[0]"sv));
  EXPECT_EQ(wkc::ConfigType::Boolean, config.Type("lst[0]"sv));

  EXPECT_THROW(config.Append("str"sv, 42), wkc::KeyError);
  EXPECT_NO_THROW(config.Append("lst"sv, 42));
  EXPECT_EQ(2, config.Size("lst"sv));
  EXPECT_EQ(42, config.GetInteger32("lst[1]"sv));
  EXPECT_EQ(wkc::ConfigType::Integer, config.Type("lst[1]"sv));

  EXPECT_THROW(config.Append("str"sv, 17L), wkc::KeyError);
  EXPECT_NO_THROW(config.Append("lst"sv, 17L));
  EXPECT_EQ(3, config.Size("lst"sv));
  EXPECT_EQ(17, config.GetInteger32("lst[2]"sv));
  EXPECT_EQ(17L, config.GetInteger64("lst[2]"sv));
  EXPECT_EQ(wkc::ConfigType::Integer, config.Type("lst[2]"sv));

  EXPECT_THROW(config.Append("str"sv, 1e-3), wkc::KeyError);
  EXPECT_NO_THROW(config.Append("lst"sv, 1e-3));
  EXPECT_EQ(4, config.Size("lst"sv));
  EXPECT_DOUBLE_EQ(1e-3, config.GetDouble("lst[3]"sv));
  EXPECT_EQ(wkc::ConfigType::FloatingPoint, config.Type("lst[3]"sv));

  EXPECT_THROW(config.Append("str"sv, "invalid"sv), wkc::KeyError);
  EXPECT_NO_THROW(config.Append("lst"sv, "valid"sv));
  EXPECT_EQ(5, config.Size("lst"sv));
  EXPECT_EQ("valid", config.GetString("lst[4]"sv));
  EXPECT_EQ(wkc::ConfigType::String, config.Type("lst[4]"sv));

  // TODO Append/Create a sublist
  // List already exists: instead of creating a separate/new list, we need
  // to append a nested list:
  EXPECT_THROW(config.CreateList("lst[5]"sv), wkc::KeyError);
  EXPECT_THROW(config.AppendNestedList("str"sv), wkc::KeyError);
  EXPECT_THROW(config.AppendNestedList("lst[4]"sv), wkc::KeyError);
  EXPECT_THROW(config.AppendNestedList("lst[5]"sv), wkc::KeyError);
  EXPECT_NO_THROW(config.AppendNestedList("lst"sv));
  EXPECT_EQ(6, config.Size("lst"sv));
  EXPECT_NO_THROW(config.Append("lst[5]"sv, 1));
  EXPECT_NO_THROW(config.Append("lst[5]"sv, -2));
  EXPECT_EQ(6, config.Size("lst"sv));
  EXPECT_EQ(2, config.Size("lst[5]"sv));
  EXPECT_EQ(1, config.GetInteger32("lst[5][0]"sv));
  EXPECT_EQ(-2, config.GetInteger32("lst[5][1]"sv));
  EXPECT_NO_THROW(config.GetInteger32List("lst[5]"sv));
  EXPECT_NO_THROW(config.Append("lst[5]"sv, "three"sv));
  EXPECT_EQ(6, config.Size("lst"sv));
  EXPECT_EQ(3, config.Size("lst[5]"sv));
  EXPECT_EQ("three", config.GetString("lst[5][2]"sv));
  EXPECT_THROW(config.GetInteger32List("lst[5]"sv), wkc::TypeError);
}
// NOLINTEND
