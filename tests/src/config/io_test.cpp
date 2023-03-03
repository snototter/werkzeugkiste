#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/files/filesys.h>
#include <werkzeugkiste/strings/strings.h>

#include <sstream>

#include "../test_utils.h"

namespace wkc = werkzeugkiste::config;
namespace wkf = werkzeugkiste::files;
namespace wks = werkzeugkiste::strings;

// NOLINTBEGIN

using namespace std::string_view_literals;

TEST(ConfigIOTest, NestedTOML) {
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
  EXPECT_THROW(
      config.LoadNestedTOMLConfiguration("no-such-key"sv), wkc::KeyError);
  EXPECT_THROW(config.LoadNestedTOMLConfiguration("bool"sv), wkc::TypeError);
  EXPECT_THROW(config.LoadNestedTOMLConfiguration("integer"sv), wkc::TypeError);
  EXPECT_THROW(config.LoadNestedTOMLConfiguration("float"sv), wkc::TypeError);
  EXPECT_THROW(config.LoadNestedTOMLConfiguration("lst"sv), wkc::TypeError);
  EXPECT_THROW(config.LoadNestedTOMLConfiguration("date"sv), wkc::TypeError);
  EXPECT_THROW(config.LoadNestedTOMLConfiguration("time"sv), wkc::TypeError);
  EXPECT_THROW(
      config.LoadNestedTOMLConfiguration("datetime"sv), wkc::TypeError);
  EXPECT_THROW(config.LoadNestedTOMLConfiguration("lvl1"sv), wkc::TypeError);
  EXPECT_THROW(
      config.LoadNestedTOMLConfiguration("lvl1.lvl2"sv), wkc::TypeError);
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
  EXPECT_THROW(
      config.LoadNestedTOMLConfiguration("lvl1.arr[2]"sv), wkc::TypeError);

  // One could abuse it, however, to load a nested configuration into a table
  // that is inside an array... Just because you can doesn't mean you should...
  EXPECT_NO_THROW(
      config.LoadNestedTOMLConfiguration("lvl1.another_arr[1].nested"sv));
  EXPECT_DOUBLE_EQ(
      2.3, config.GetDouble("lvl1.another_arr[1].nested.value2"sv));
  EXPECT_EQ("this/is/a/relative/path",
      config.GetString("lvl1.another_arr[1].nested.section1.rel_path"sv));
}

TEST(ConfigIOTest, ConfigConstruction) {
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

TEST(ConfigIOTest, LoadingToml) {
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
  EXPECT_THROW(
      wkc::LoadTOMLFile("this-does-not-exist.toml"sv), wkc::ParseError);
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

TEST(ConfigIOTest, ParsingInvalidDateTimes) {
  // Leap seconds are not supported.
  EXPECT_THROW(
      wkc::LoadTOMLString("dt = 1990-12-31T23:59:60Z"), wkc::ParseError);
  EXPECT_THROW(
      wkc::LoadTOMLString("dt = 1990-12-31T15:59:60-08:00"), wkc::ParseError);

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
TEST(ConfigIOTest, LoadingJson) {
  const auto config = wkc::LoadTOMLString(R"toml(
    param1 = "value"
    )toml"sv);
  EXPECT_TRUE(config.ToJSON().length() > 0);
}

#ifdef WERKZEUGKISTE_WITH_LIBCONFIG
TEST(ConfigIOTest, ParseLibconfigFiles) {
  EXPECT_THROW(wkc::LoadLibconfigFile("no-such-file"sv), wkc::ParseError);
  // TODO
}

TEST(ConfigIOTest, ParseLibconfigStrings) {
  // TODO enable if compiled with libconfig
  const auto config = wkc::LoadLibconfigString(R"lcfg(
    int_pos = +987654;
    int_neg = -123456;
    int32_max = 2147483647;
    int32_min = -2147483648;
    // Previous libconfig versions require explicit ..L suffix for long ints:
    int32_max_overflow = 2147483648L;
    int32_min_underflow = -2147483649L;
    flt = -1e3;
    flag = false;
    str = "value";

    ints = [1, 2, 3];
    flts = [1.2, 2.0];

    strings = ["foo", "bar"];

    mixed = (1, true, "string");

    nested = (
      "foo",
      { age = 3; name = "bar"; },
      [-1, 23])

    group = {
      flag = true;
      count = 123;

      subgroup = {
        flag = false;
        threshold = 1e-6;
      }
    };
    )lcfg");

  EXPECT_EQ(987654, config.GetInteger32("int_pos"sv));
  EXPECT_EQ(-123456, config.GetInteger32("int_neg"sv));
  EXPECT_EQ(2147483647, config.GetInteger32("int32_max"sv));
  EXPECT_EQ(-2147483648, config.GetInteger32("int32_min"sv));
  EXPECT_THROW(config.GetInteger32("int32_min_underflow"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger32("int32_max_overflow"sv), wkc::TypeError);
  EXPECT_EQ(-2147483649, config.GetInteger64("int32_min_underflow"sv));
  EXPECT_EQ(+2147483648, config.GetInteger64("int32_max_overflow"sv));

  EXPECT_DOUBLE_EQ(-1000.0, config.GetDouble("flt"sv));
  EXPECT_FALSE(config.GetBoolean("flag"sv));
  EXPECT_EQ("value", config.GetString("str"sv));

  // List of integers
  auto ints = config.GetInteger32List("ints"sv);
  EXPECT_EQ(3, ints.size());
  EXPECT_EQ(1, ints[0]);
  EXPECT_EQ(2, ints[1]);
  EXPECT_EQ(3, ints[2]);

  // List of floating points
  const auto flts = config.GetDoubleList("flts"sv);
  EXPECT_EQ(2, flts.size());
  EXPECT_DOUBLE_EQ(1.2, flts[0]);
  EXPECT_DOUBLE_EQ(2.0, flts[1]);

  // String list
  const auto strings = config.GetStringList("strings"sv);
  EXPECT_EQ(2, strings.size());
  EXPECT_EQ("foo", strings[0]);
  EXPECT_EQ("bar", strings[1]);

  // Mixed list
  EXPECT_EQ(3, config.Size("mixed"sv));
  EXPECT_EQ(wkc::ConfigType::List, config.Type("mixed"sv));
  EXPECT_EQ(wkc::ConfigType::Integer, config.Type("mixed[0]"sv));
  EXPECT_EQ(1, config.GetInteger32("mixed[0]"sv));
  EXPECT_EQ(wkc::ConfigType::Boolean, config.Type("mixed[1]"sv));
  EXPECT_TRUE(config.GetBoolean("mixed[1]"sv));
  EXPECT_EQ(wkc::ConfigType::String, config.Type("mixed[2]"sv));
  EXPECT_EQ("string", config.GetString("mixed[2]"sv));

  // Mixed & nested list
  EXPECT_EQ(3, config.Size("nested"sv));
  EXPECT_EQ(wkc::ConfigType::List, config.Type("nested"sv));

  EXPECT_EQ(wkc::ConfigType::String, config.Type("nested[0]"sv));
  EXPECT_EQ("foo", config.GetString("nested[0]"sv));

  EXPECT_EQ(wkc::ConfigType::Group, config.Type("nested[1]"sv));
  auto subgroup = config.GetGroup("nested[1]"sv);
  EXPECT_EQ(2, subgroup.Size());
  EXPECT_EQ(3, subgroup.GetInteger32("age"sv));
  EXPECT_EQ(3, config.GetInteger32("nested[1].age"sv));
  EXPECT_EQ("bar", subgroup.GetString("name"sv));
  EXPECT_EQ("bar", config.GetString("nested[1].name"sv));

  EXPECT_EQ(wkc::ConfigType::List, config.Type("nested[2]"sv));
  ints = config.GetInteger32List("nested[2]"sv);
  EXPECT_EQ(2, ints.size());
  EXPECT_EQ(-1, ints[0]);
  EXPECT_EQ(23, ints[1]);

  // Subgroup/Table
  EXPECT_EQ(wkc::ConfigType::Group, config.Type("group"sv));
  EXPECT_EQ(3, config.Size("group"sv));
  EXPECT_TRUE(config.GetBoolean("group.flag"sv));
  EXPECT_EQ(123, config.GetInteger32("group.count"sv));

  EXPECT_EQ(wkc::ConfigType::Group, config.Type("group.subgroup"sv));
  EXPECT_FALSE(config.GetBoolean("group.subgroup.flag"sv));
  EXPECT_DOUBLE_EQ(1e-6, config.GetDouble("group.subgroup.threshold"sv));

  // Try parsing an invalid configuration string.
  const auto invalid_str = R"lcfg(valid = true;
    invalid = [1, 2.5];
    )lcfg"sv;
  EXPECT_THROW(wkc::LoadLibconfigString(invalid_str), wkc::ParseError);
  try {
    wkc::LoadLibconfigString(invalid_str);
  } catch (const wkc::ParseError &e) {
    EXPECT_TRUE(wks::StartsWith(e.what(),
        "Parsing libconfig string failed at line `2`: "
        "mismatched element type in array"))
        << "Actual exception message: " << e.what();
  }
}

#else   // WERKZEUGKISTE_WITH_LIBCONFIG
TEST(ConfigIOTest, MissingLibconfigSupport) {
  EXPECT_THROW(wkc::LoadLibconfigFile("no-such-file"sv), std::logic_error);
  EXPECT_THROW(wkc::LoadLibconfigString("foo = 3"sv), std::logic_error);
}
#endif  // WERKZEUGKISTE_WITH_LIBCONFIG

// NOLINTEND
