#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/files/filesys.h>

#include <sstream>

#include "../test_utils.h"

namespace wkc = werkzeugkiste::config;
namespace wkf = werkzeugkiste::files;

using namespace std::string_view_literals;

// NOLINTBEGIN

TEST(ConfigUtilsTest, NestedTOML) {
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
  EXPECT_THROW(config.LoadNestedConfiguration("no-such-key"sv), wkc::KeyError);
  EXPECT_THROW(config.LoadNestedConfiguration("bool"sv), wkc::TypeError);
  EXPECT_THROW(config.LoadNestedConfiguration("integer"sv), wkc::TypeError);
  EXPECT_THROW(config.LoadNestedConfiguration("float"sv), wkc::TypeError);
  EXPECT_THROW(config.LoadNestedConfiguration("lst"sv), wkc::TypeError);
  EXPECT_THROW(config.LoadNestedConfiguration("date"sv), wkc::TypeError);
  EXPECT_THROW(config.LoadNestedConfiguration("time"sv), wkc::TypeError);
  EXPECT_THROW(config.LoadNestedConfiguration("datetime"sv), wkc::TypeError);
  EXPECT_THROW(config.LoadNestedConfiguration("lvl1"sv), wkc::TypeError);
  EXPECT_THROW(config.LoadNestedConfiguration("lvl1.lvl2"sv), wkc::TypeError);
  config.LoadNestedConfiguration("nested_config"sv);

  EXPECT_EQ(1, config.GetInteger32("nested_config.value1"sv));
  EXPECT_DOUBLE_EQ(2.3, config.GetDouble("nested_config.value2"sv));
  EXPECT_EQ("this/is/a/relative/path",
      config.GetString("nested_config.section1.rel_path"sv));

  // When trying to load an invalid TOML file, an exception should be thrown,
  // and the parameter should not change.
  EXPECT_THROW(config.LoadNestedConfiguration("invalid_nested_config"sv),
      wkc::ParseError);
  EXPECT_EQ(fname_invalid_toml, config.GetString("invalid_nested_config"sv));

  // Ensure that loading a nested configuration also works at deeper
  // hierarchy levels.
  EXPECT_NO_THROW(config.LoadNestedConfiguration("lvl1.lvl2.lvl3.nested"sv));
  EXPECT_DOUBLE_EQ(2.3, config.GetDouble("lvl1.lvl2.lvl3.nested.value2"sv));
  EXPECT_EQ("this/is/a/relative/path",
      config.GetString("lvl1.lvl2.lvl3.nested.section1.rel_path"sv));

  // It is not allowed to load a nested configuration directly into an array:
  EXPECT_THROW(config.LoadNestedConfiguration("lvl1.arr[2]"sv), wkc::TypeError);

  // One could abuse it, however, to load a nested configuration into a table
  // that is inside an array... Just because you can doesn't mean you should...
  EXPECT_NO_THROW(
      config.LoadNestedConfiguration("lvl1.another_arr[1].nested"sv));
  EXPECT_DOUBLE_EQ(
      2.3, config.GetDouble("lvl1.another_arr[1].nested.value2"sv));
  EXPECT_EQ("this/is/a/relative/path",
      config.GetString("lvl1.another_arr[1].nested.section1.rel_path"sv));
}

TEST(ConfigUtilsTest, AbsolutePaths) {
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

TEST(ConfigUtilsTest, StringReplacements) {
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
  EXPECT_THROW(
      config.ReplaceStringPlaceholders({{""sv, "replace"sv}}), wkc::ValueError);

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

// NOLINTEND
