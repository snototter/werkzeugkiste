#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/strings/strings.h>

#include <sstream>

#include "../test_utils.h"

namespace wkc = werkzeugkiste::config;
namespace wks = werkzeugkiste::strings;

using namespace std::string_view_literals;

// NOLINTBEGIN

TEST(ConfigKeyTest, ParameterNames1) {
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

TEST(ConfigKeyTest, ParameterNames2) {
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

  CheckMatchingContainers(expected_keys, keys);

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

TEST(ConfigKeyTest, KeyMatching) {
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

// NOLINTEND
