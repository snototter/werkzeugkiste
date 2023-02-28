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

TEST(ConfigListTest, SetBooleanList) {
  auto config = wkc::LoadTOMLString(R"toml(
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
  EXPECT_EQ(2, flags.size());
  EXPECT_FALSE(flags[0]);
  EXPECT_TRUE(flags[1]);

  // A mixed list that only contains numbers can be replaced by a homogeneous
  // list. Its type, however, will be floating point afterwards:
  EXPECT_NO_THROW(config.SetInteger32List("mixed_int_flt"sv, {1, 2}));
  const auto ints32 = config.GetInteger32List("mixed_int_flt"sv);
  EXPECT_EQ(2, ints32.size());
  EXPECT_EQ(1, ints32[0]);
  EXPECT_EQ(2, ints32[1]);
  EXPECT_EQ(wkc::ConfigType::FloatingPoint, config.Type("mixed_int_flt[0]"sv));
  EXPECT_EQ(wkc::ConfigType::FloatingPoint, config.Type("mixed_int_flt[1]"sv));

  // ... but for all other types/mixtures, the type cannot be changed.
  EXPECT_THROW(config.SetInteger64List("mixed_types"sv, {1, 3, -17}),
               wkc::TypeError);

  EXPECT_THROW(config.SetInteger32List("flags"sv, {1, 3, -17}), wkc::TypeError);

  EXPECT_THROW(config.SetDoubleList("nested_lst"sv, {1.0, -0.5}),
               wkc::TypeError);
  EXPECT_THROW(config.SetStringList("floats"sv, {"abc"}), wkc::TypeError);
}

TEST(ConfigListTest, NumericList) {
  wkc::Configuration config{};

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
}

TEST(ConfigListTest, StringList) {}

TEST(ConfigListTest, DateTimeLists) {}

TEST(ConfigListTest, MixedList) {}

// NOLINTEND
