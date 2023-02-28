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

TEST(ConfigIOTest, ParsingInvalidDateTimes) {
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
TEST(ConfigIOTest, LoadingJson) {
  const auto config = wkc::LoadTOMLString(R"toml(
    param1 = "value"
    )toml"sv);
  EXPECT_TRUE(config.ToJSON().length() > 0);
}

// NOLINTEND
