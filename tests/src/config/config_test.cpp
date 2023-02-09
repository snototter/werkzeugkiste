#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/files/filesys.h>
#include <werkzeugkiste/geometry/vector.h>
#include <werkzeugkiste/strings/strings.h>

#include <cmath>
#include <exception>
#include <limits>
#include <sstream>

#include "../test_utils.h"

namespace wkc = werkzeugkiste::config;
namespace wkf = werkzeugkiste::files;
namespace wkg = werkzeugkiste::geometry;
namespace wks = werkzeugkiste::strings;

// NOLINTBEGIN

using namespace std::string_view_literals;

TEST(ConfigTest, TypeUtils) {
  EXPECT_EQ("bool", wkc::TypeName<bool>());
  EXPECT_EQ("int32", wkc::TypeName<int32_t>());
  EXPECT_EQ("int64", wkc::TypeName<int64_t>());
  EXPECT_EQ("float", wkc::TypeName<float>());
  EXPECT_EQ("double", wkc::TypeName<double>());
  EXPECT_EQ("string", wkc::TypeName<std::string>());
  EXPECT_EQ("string_view", wkc::TypeName<std::string_view>());

  EXPECT_NE("ushort", wkc::TypeName<unsigned short>());
}

TEST(ConfigTest, Integers) {
  const auto config = wkc::Configuration::LoadTOMLString(R"toml(
    int32_1 = -123456
    int32_2 = +987654
    int32_max = 2147483647
    int32_max_overflow = 2147483648
    int32_min = -2147483648
    int32_min_overflow = -2147483649
    )toml");
  EXPECT_EQ(-123456, config->GetInteger32("int32_1"));
  EXPECT_EQ(987654, config->GetInteger32("int32_2"));
  EXPECT_EQ(2147483647, config->GetInteger32("int32_max"));
  EXPECT_EQ(-2147483648, config->GetInteger32("int32_min"));
  EXPECT_THROW(config->GetInteger32("int32_min_overflow"), std::range_error);
  EXPECT_THROW(config->GetInteger32("int32_max_overflow"), std::range_error);

  EXPECT_EQ(-1, config->GetInteger32OrDefault("test", -1));
  EXPECT_EQ(17, config->GetInteger32OrDefault("another", 17));
  EXPECT_THROW(config->GetInteger32("test"), wkc::KeyError);

  EXPECT_EQ(-123456, config->GetInteger64("int32_1"));
  EXPECT_EQ(+987654, config->GetInteger64("int32_2"));
  EXPECT_EQ(-2147483649, config->GetInteger64("int32_min_overflow"));
  EXPECT_EQ(+2147483648, config->GetInteger64("int32_max_overflow"));
  EXPECT_EQ(-1, config->GetInteger64OrDefault("test", -1));
  EXPECT_EQ(17, config->GetInteger64OrDefault("another", 17));
  EXPECT_THROW(config->GetInteger64("test"), wkc::KeyError);
}

TEST(ConfigTest, FloatingPoint) {
  const auto config = wkc::Configuration::LoadTOMLString(R"toml(
    int = 32

    flt1 = +1.0
    flt2 = -3.1415
    flt3 = 5e+22

    spec1 = inf
    spec2 = -inf
    spec3 = nan
    )toml");
  // An integer cannot be loaded as double (there's no safe cast from
  // 64-bit int to double).
  EXPECT_THROW(config->GetDouble("int"), std::runtime_error);

  // Similarly, a double can't be loaded as another type.
  EXPECT_THROW(config->GetInteger32("flt1"), std::runtime_error);
  EXPECT_THROW(config->GetInteger64("flt1"), std::runtime_error);

  EXPECT_DOUBLE_EQ(+1.0, config->GetDouble("flt1"));
  EXPECT_DOUBLE_EQ(-3.1415, config->GetDouble("flt2"));
  EXPECT_DOUBLE_EQ(+5e22, config->GetDouble("flt3"));

  EXPECT_DOUBLE_EQ(+std::numeric_limits<double>::infinity(),
                   config->GetDouble("spec1"));
  EXPECT_DOUBLE_EQ(-std::numeric_limits<double>::infinity(),
                   config->GetDouble("spec2"));
  EXPECT_TRUE(std::isnan(config->GetDouble("spec3")));

  EXPECT_EQ(-16.0, config->GetDoubleOrDefault("test", -16));
  EXPECT_THROW(config->GetDouble("test"), wkc::KeyError);
}

TEST(ConfigTest, QueryTypes) {
  const auto config = wkc::Configuration::LoadTOMLString(R"toml(
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

    )toml");

  EXPECT_TRUE(config->Contains("bool"));
  EXPECT_FALSE(config->Contains("bool1"));
  EXPECT_EQ(wkc::ConfigType::Boolean, config->Type("bool"));

  EXPECT_TRUE(config->Contains("int"));
  EXPECT_FALSE(config->Contains("in"));
  EXPECT_EQ(wkc::ConfigType::Integer, config->Type("int"));

  EXPECT_TRUE(config->Contains("flt"));
  EXPECT_EQ(wkc::ConfigType::FloatingPoint, config->Type("flt"));

  EXPECT_TRUE(config->Contains("str"));
  EXPECT_EQ(wkc::ConfigType::String, config->Type("str"));

  EXPECT_TRUE(config->Contains("lst"));
  EXPECT_EQ(wkc::ConfigType::List, config->Type("lst"));

  EXPECT_TRUE(config->Contains("lst[0]"));
  EXPECT_EQ(wkc::ConfigType::Integer, config->Type("lst[0]"));
  EXPECT_TRUE(config->Contains("lst[1]"));
  EXPECT_EQ(wkc::ConfigType::Integer, config->Type("lst[1]"));
  EXPECT_TRUE(config->Contains("lst[2]"));
  EXPECT_EQ(wkc::ConfigType::FloatingPoint, config->Type("lst[2]"));
  EXPECT_FALSE(config->Contains("lst[3]"));

  EXPECT_THROW(config->Type("lst[3]"), wkc::KeyError);
  try {
    config->Type("lst[3]");
  } catch (const wkc::KeyError &e) {
    const std::string exp_msg{"Key `lst[3]` does not exist!"};
    EXPECT_EQ(exp_msg, std::string(e.what()));
  }

  EXPECT_TRUE(config->Contains("dates"));
  EXPECT_EQ(wkc::ConfigType::Table, config->Type("dates"));

  // TODO dates
}

TEST(ConfigTest, GetScalarTypes) {
  const std::string toml_str = R"toml(
    bool = true
    int = 42
    flt = 1.0
    str = "A string" #TODO others (date, time, date_time)

    int_list = [1, 2, 3]

    [dates]
    day = 2023-01-01
    time1 = 12:34:56
    time2 = 00:01:02.123456
    date_time = 1912-07-23T08:37:00-08:00

    )toml";
  const auto config = wkc::Configuration::LoadTOMLString(toml_str);

  // Boolean parameter
  EXPECT_EQ(true, config->GetBoolean("bool"));
  EXPECT_THROW(config->GetBoolean("no-such.bool"), wkc::KeyError);
  EXPECT_TRUE(config->GetBooleanOrDefault("no-such.bool", true));
  EXPECT_FALSE(config->GetBooleanOrDefault("no-such.bool", false));

  EXPECT_THROW(config->GetInteger32("bool"), std::runtime_error);
  EXPECT_THROW(config->GetInteger32OrDefault("bool", 0), std::runtime_error);
  EXPECT_THROW(config->GetInteger64("bool"), std::runtime_error);
  EXPECT_THROW(config->GetInteger64OrDefault("bool", 2), std::runtime_error);
  EXPECT_THROW(config->GetDouble("bool"), std::runtime_error);
  EXPECT_THROW(config->GetDoubleOrDefault("bool", 1.0), std::runtime_error);
  EXPECT_THROW(config->GetString("bool"), std::runtime_error);
  EXPECT_THROW(config->GetStringOrDefault("bool", "..."), std::runtime_error);

  // Integer parameter
  EXPECT_EQ(42, config->GetInteger32("int"));
  EXPECT_EQ(42, config->GetInteger64("int"));

  EXPECT_THROW(config->GetBoolean("int"), std::runtime_error);
  EXPECT_THROW(config->GetBooleanOrDefault("int", true), std::runtime_error);
  EXPECT_THROW(config->GetDouble("int"), std::runtime_error);
  EXPECT_THROW(config->GetString("int"), std::runtime_error);
  EXPECT_THROW(config->GetStringOrDefault("int", "..."), std::runtime_error);

  // Double parameter
  EXPECT_DOUBLE_EQ(1.0, config->GetDouble("flt"));

  EXPECT_THROW(config->GetBoolean("flt"), std::runtime_error);
  EXPECT_THROW(config->GetInteger32("flt"), std::runtime_error);
  EXPECT_THROW(config->GetInteger64("flt"), std::runtime_error);
  EXPECT_THROW(config->GetString("flt"), std::runtime_error);
  EXPECT_THROW(config->GetStringOrDefault("flt", "..."), std::runtime_error);

  // String parameter
  const std::string expected{"A string"};
  EXPECT_EQ(expected, config->GetString("str"));

  EXPECT_THROW(config->GetString("no-such-key"), wkc::KeyError);

  EXPECT_EQ("...", config->GetStringOrDefault("no-such-key", "..."));

  EXPECT_THROW(config->GetBoolean("str"), std::runtime_error);
  EXPECT_THROW(config->GetInteger32("str"), std::runtime_error);
  EXPECT_THROW(config->GetInteger64("str"), std::runtime_error);

  // Invalid access
  EXPECT_THROW(config->GetBoolean("int_list"), std::runtime_error);
  EXPECT_THROW(config->GetBoolean("tbl"), wkc::KeyError);
  EXPECT_THROW(config->GetInteger32("int_list"), std::runtime_error);
  EXPECT_THROW(config->GetInteger32("tbl"), wkc::KeyError);
  EXPECT_THROW(config->GetInteger64("int_list"), std::runtime_error);
  EXPECT_THROW(config->GetInteger64("tbl"), wkc::KeyError);
  EXPECT_THROW(config->GetDouble("int_list"), std::runtime_error);
  EXPECT_THROW(config->GetDouble("tbl"), wkc::KeyError);
  EXPECT_THROW(config->GetString("int_list"), std::runtime_error);
  EXPECT_THROW(config->GetString("tbl"), wkc::KeyError);

  EXPECT_THROW(config->GetDouble("dates"), std::runtime_error);
  EXPECT_THROW(config->GetDouble("dates.day"), std::runtime_error);
  EXPECT_THROW(config->GetDouble("dates.time1"), std::runtime_error);
  EXPECT_THROW(config->GetDouble("dates.time2"), std::runtime_error);
  EXPECT_THROW(config->GetDouble("dates.date_time"), std::runtime_error);
  EXPECT_THROW(config->GetString("dates"), std::runtime_error);
  EXPECT_THROW(config->GetString("dates.day"), std::runtime_error);
  EXPECT_THROW(config->GetString("dates.time1"), std::runtime_error);
  EXPECT_THROW(config->GetString("dates.time2"), std::runtime_error);
  EXPECT_THROW(config->GetString("dates.date_time"), std::runtime_error);
}

TEST(ConfigTest, SetScalarTypes1) {
  const std::string toml_str = R"toml(
    bool = true
    int = 42
    a.string = "value"
    booleans = [true, false, true]

    array = [0, 1, { int = 2, bool = false }]
    )toml";
  auto config = wkc::Configuration::LoadTOMLString(toml_str);

  // Adjust a boolean parameter
  EXPECT_EQ(true, config->GetBoolean("bool"));
  EXPECT_NO_THROW(config->SetBoolean("bool", false));
  EXPECT_EQ(false, config->GetBoolean("bool"));

  // Cannot change the type of an existing parameter
  EXPECT_THROW(config->SetBoolean("int", true), std::runtime_error);

  // Set a non-existing parameter
  EXPECT_THROW(config->GetBoolean("another_bool"), wkc::KeyError);
  EXPECT_NO_THROW(config->SetBoolean("another_bool", false));
  EXPECT_NO_THROW(config->GetBoolean("another_bool"));
  EXPECT_EQ(false, config->GetBoolean("another_bool"));

  // Set a nested parameter (must create the hierarchy)
  EXPECT_THROW(config->GetBoolean("others.bool"), wkc::KeyError);
  EXPECT_NO_THROW(config->SetBoolean("others.bool", false));
  EXPECT_NO_THROW(config->GetBoolean("others.bool"));
  EXPECT_EQ(false, config->GetBoolean("others.bool"));

  // Test a deeper path hierarchy
  EXPECT_THROW(config->GetBoolean("a.deeper.hierarchy.bool"), wkc::KeyError);
  EXPECT_NO_THROW(config->SetBoolean("a.deeper.hierarchy.bool", false));
  EXPECT_NO_THROW(config->GetBoolean("a.deeper.hierarchy.bool"));
  EXPECT_EQ(false, config->GetBoolean("a.deeper.hierarchy.bool"));

  // Cannot create a path below a scalar type
  EXPECT_THROW(config->SetBoolean("a.string.below.bool", true),
               std::runtime_error);

  // Creating an array is also not supported
  EXPECT_THROW(config->SetBoolean("an_array[3].bool", true),
               std::runtime_error);

  // Creating a table within an existing array is also not supported:
  EXPECT_THROW(config->SetBoolean("array[3].bool", true), std::runtime_error);

  // But setting an existing array element is supported:
  EXPECT_NO_THROW(config->SetBoolean("booleans[1]", true));
  EXPECT_EQ(true, config->GetBoolean("booleans[0]"));
  EXPECT_EQ(true, config->GetBoolean("booleans[1]"));
  EXPECT_EQ(true, config->GetBoolean("booleans[2]"));

  EXPECT_EQ(false, config->GetBoolean("array[2].bool"));
  EXPECT_NO_THROW(config->SetBoolean("array[2].bool", true));
  EXPECT_EQ(true, config->GetBoolean("array[2].bool"));
}

TEST(ConfigTest, SetScalarTypes2) {
  auto config = wkc::Configuration::LoadTOMLString(R"toml(
    integer = 12345
    string = "This is a string"

    [section]
    float = 1.5
    string = "value"
    array = [1, true, "a string"]
    )toml");

  // Change integers
  EXPECT_EQ(12345, config->GetInteger32("integer"sv));
  EXPECT_NO_THROW(config->SetInteger32("integer"sv, -123));
  EXPECT_EQ(-123, config->GetInteger32("integer"sv));

  EXPECT_EQ(-123, config->GetInteger64("integer"sv));
  EXPECT_NO_THROW(config->SetInteger64("integer"sv, -2147483649));
  EXPECT_EQ(-2147483649, config->GetInteger64("integer"sv));

  // Change a double
  EXPECT_DOUBLE_EQ(1.5, config->GetDouble("section.float"sv));
  EXPECT_NO_THROW(config->SetDouble("section.float"sv, 0.01));
  EXPECT_DOUBLE_EQ(0.01, config->GetDouble("section.float"sv));

  // We cannot change the type of an existing parameter
  EXPECT_THROW(config->SetDouble("integer"sv, 1.5), std::runtime_error);

  // Set a string:
  EXPECT_EQ("value", config->GetString("section.string"sv));
  EXPECT_NO_THROW(config->SetString("section.string"sv, "frobmorten"sv));
  EXPECT_EQ("frobmorten", config->GetString("section.string"sv));

  // Change a string within an array:
  EXPECT_EQ("a string", config->GetString("section.array[2]"sv));
  EXPECT_NO_THROW(config->SetString("section.array[2]"sv, "foobar"sv));
  EXPECT_EQ("foobar", config->GetString("section.array[2]"sv));

  // Add new scalars:
  EXPECT_NO_THROW(config->SetInteger32("new-values.int32"sv, 3));
  EXPECT_NO_THROW(config->SetInteger64("new-values.int64"sv, 64));
  EXPECT_NO_THROW(config->SetDouble("new-values.float"sv, 1e23));
  EXPECT_NO_THROW(config->SetString("new-values.str", "It works!"));
  EXPECT_EQ(3, config->GetInteger32("new-values.int32"sv));
  EXPECT_EQ(64, config->GetInteger32("new-values.int64"sv));
  EXPECT_DOUBLE_EQ(1e23, config->GetDouble("new-values.float"sv));
  EXPECT_EQ("It works!", config->GetString("new-values.str"sv));
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

  const auto config = wkc::Configuration::LoadTOMLString(toml_str);
  const auto keys = config->ListParameterNames(false);

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
    EXPECT_NE(keys.end(), pos) << "Key `" << key << "` not found!";
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
  const auto config = wkc::Configuration::LoadTOMLString(toml_str);

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
  auto keys = config->ListParameterNames(false);

  EXPECT_EQ(expected_keys.size(), keys.size())
      << "Extracted keys: " << Stringify(keys)
      << "\nExpected keys:  " << Stringify(expected_keys) << "!";

  for (const auto &expected : expected_keys) {
    const auto pos = std::find(keys.begin(), keys.end(), expected);
    EXPECT_NE(keys.end(), pos) << "Key `" << expected << "` not found!";
  }

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

  keys = config->ListParameterNames(true);

  EXPECT_EQ(expected_keys.size(), keys.size())
      << "Extracted keys: " << Stringify(keys)
      << "\nExpected keys:  " << Stringify(expected_keys) << "!";

  for (const auto &expected : expected_keys) {
    const auto pos = std::find(keys.begin(), keys.end(), expected);
    EXPECT_NE(keys.end(), pos) << "Key `" << expected << "` not found!";
  }
}

TEST(ConfigTest, KeyMatching) {
  auto mm = wkc::MultiKeyMatcher::Create({"this-is.a-valid.key"});

  EXPECT_FALSE(mm->MatchAny("this.is.a-valid.key"));
  EXPECT_FALSE(mm->MatchAny("this_is.a_valid.key"));
  EXPECT_FALSE(mm->MatchAny("this-is.a-valid.ke"));
  EXPECT_FALSE(mm->MatchAny("this-is.a-valid.key2"));
  EXPECT_TRUE(mm->MatchAny("this-is.a-valid.key"));

  mm = wkc::MultiKeyMatcher::Create({"plain-key", "a.b.c1"});
  EXPECT_FALSE(mm->MatchAny("this-is.a-valid.key"));
  EXPECT_TRUE(mm->MatchAny("plain-key"));
  EXPECT_TRUE(mm->MatchAny("a.b.c1"));
  EXPECT_FALSE(mm->MatchAny("a.b.c"));

  auto single = wkc::SingleKeyMatcher::Create("pattern*");
  EXPECT_TRUE(single->Match("pattern"));
  EXPECT_TRUE(single->Match("pattern-"));
  EXPECT_TRUE(single->Match("pattern1"));
  EXPECT_FALSE(single->Match("a-pattern"));

  single = wkc::SingleKeyMatcher::Create("*pattern*");
  EXPECT_TRUE(single->Match("pattern"));
  EXPECT_TRUE(single->Match("pattern-"));
  EXPECT_TRUE(single->Match("pattern1"));
  EXPECT_TRUE(single->Match("a-pattern"));
  EXPECT_FALSE(single->Match("pAttern"));
  EXPECT_FALSE(single->Match("pat-tern"));

  single = wkc::SingleKeyMatcher::Create("table.*.param");
  EXPECT_FALSE(single->Match("table.param"));
  EXPECT_TRUE(single->Match("table.sub.param"));
  EXPECT_TRUE(single->Match("table.Sub123.param"));
  EXPECT_FALSE(single->Match("table1.sub.param"));
  EXPECT_FALSE(single->Match("table.sub.param1"));

  // We explicitly use only a basic substitution.
  // Yes, this invalid keys matches. No, this is not a problem
  // because the matching is only used internally to select
  // existing nodes (and an invalid key could not have been created
  // to begin with...)
  single = wkc::SingleKeyMatcher::Create("arr[*].*");
  EXPECT_TRUE(single->Match("arr[*].*"));
  EXPECT_FALSE(single->Match("arr*"));
  EXPECT_FALSE(single->Match("arr.name"));
  EXPECT_FALSE(single->Match("arr[]name"));
  EXPECT_TRUE(single->Match("arr[0].name"));
  EXPECT_TRUE(single->Match("arr[1].name"));
  EXPECT_TRUE(single->Match("arr[-10].name"));
  EXPECT_TRUE(single->Match("arr[123].name"));
  EXPECT_TRUE(single->Match("arr[123].*"));
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
  const auto config = wkc::Configuration::LoadTOMLString(R"toml(
    str = "not a point list"

    poly1 = [[1, 2], [3, 4], [5, 6], [-7, -8]]

    poly2 = [{y = 20, x = 10}, {x = 30, y = 40}, {y = 60, x = 50}]

    poly3 = [[1, 2, 3], [4, 5, 6], {x = -9, y = 0, z = -3}]

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

    )toml");

  auto poly = config->GetPoints2D("poly1");
  EXPECT_EQ(4, poly.size());

  auto list = config->GetInteger32List("poly1[0]");
  EXPECT_EQ(2, list.size());
  EXPECT_EQ(1, list[0]);
  EXPECT_EQ(2, list[1]);
  list = config->GetInteger32List("poly1[2]");
  EXPECT_EQ(2, list.size());
  EXPECT_EQ(5, list[0]);
  EXPECT_EQ(6, list[1]);

  auto vec = TuplesToVecs<wkg::Vec2i>(poly);
  EXPECT_EQ(wkg::Vec2i(1, 2), vec[0]);
  EXPECT_EQ(wkg::Vec2i(3, 4), vec[1]);
  EXPECT_EQ(wkg::Vec2i(5, 6), vec[2]);
  EXPECT_EQ(wkg::Vec2i(-7, -8), vec[3]);

  poly = config->GetPoints2D("poly2");
  EXPECT_EQ(3, poly.size());

  vec = TuplesToVecs<wkg::Vec2i>(poly);
  EXPECT_EQ(wkg::Vec2i(10, 20), vec[0]);
  EXPECT_EQ(wkg::Vec2i(30, 40), vec[1]);
  EXPECT_EQ(wkg::Vec2i(50, 60), vec[2]);

  // Cannot load an array of tables as a scalar list:
  EXPECT_THROW(config->GetInteger32List("poly2"), std::runtime_error);

  // An N-dimensional polygon can be looked up from any list of at
  // least N-dimensional points:
  EXPECT_NO_THROW(config->GetPoints2D("poly3"));
  EXPECT_NO_THROW(config->GetPoints3D("poly3"));
  EXPECT_NO_THROW(config->GetPoints2D("poly4"));
  EXPECT_NO_THROW(config->GetPoints3D("poly4"));

  EXPECT_THROW(config->GetPoints2D("no-such-key"), wkc::KeyError);
  EXPECT_THROW(config->GetPoints2D("str"), std::runtime_error);
  EXPECT_THROW(config->GetPoints2D("invalid.p1"), std::runtime_error);
  EXPECT_THROW(config->GetPoints2D("invalid.p2"), std::runtime_error);
  EXPECT_THROW(config->GetPoints2D("invalid.p3"), std::runtime_error);
  EXPECT_THROW(config->GetPoints2D("invalid.p4"), std::runtime_error);
  EXPECT_THROW(config->GetPoints2D("invalid.p5"), std::runtime_error);

  EXPECT_NO_THROW(config->GetPoints2D("invalid.p6"));
  EXPECT_THROW(config->GetPoints3D("invalid.p6"), std::runtime_error);

  EXPECT_NO_THROW(config->GetPoints2D("invalid.p7"));
  EXPECT_THROW(config->GetPoints3D("invalid.p7"), std::runtime_error);

  // 3D polygons
  EXPECT_THROW(config->GetPoints3D("poly1"), std::runtime_error);
  EXPECT_THROW(config->GetPoints3D("poly2"), std::runtime_error);

  auto poly3d = config->GetPoints3D("poly3");
  EXPECT_EQ(3, poly3d.size());
  std::vector<wkg::Vec3i> vec3d = TuplesToVecs<wkg::Vec3i>(poly3d);
  EXPECT_EQ(wkg::Vec3i(1, 2, 3), vec3d[0]);
  EXPECT_EQ(wkg::Vec3i(4, 5, 6), vec3d[1]);
  EXPECT_EQ(wkg::Vec3i(-9, 0, -3), vec3d[2]);
}

TEST(ConfigTest, ScalarLists) {
  const auto config = wkc::Configuration::LoadTOMLString(R"toml(
    ints32 = [1, 2, 3, 4, 5, 6, -7, -8]

    ints64 = [0, 2147483647, 2147483648, -2147483648, -2147483649]

    floats = [0.5, 1.0, 1.0e23]

    strings = ["abc", "Foo", "Frobmorten", "Test String"]

    # Type mix
    invalid_int_flt = [1, 2, 3, 4.5, 5]

    mixed_types = [1, 2, "framboozle"]

    an_int = 1234

    [not-a-list]
    name = "test"
    )toml");

  // Key error:
  EXPECT_THROW(config->GetInteger32List("no-such-key"), wkc::KeyError);
  EXPECT_THROW(config->GetInteger64List("no-such-key"), wkc::KeyError);
  EXPECT_THROW(config->GetDoubleList("no-such-key"), wkc::KeyError);
  EXPECT_THROW(config->GetStringList("no-such-key"), wkc::KeyError);

  // Try to load a wrong data type as list:
  EXPECT_THROW(config->GetInteger32List("an_int"), std::runtime_error);
  EXPECT_THROW(config->GetInteger32List("not-a-list"), std::runtime_error);
  EXPECT_THROW(config->GetInteger32List("not-a-list.no-such-key"),
               wkc::KeyError);
  EXPECT_THROW(config->GetInteger64List("an_int"), std::runtime_error);
  EXPECT_THROW(config->GetInteger64List("not-a-list"), std::runtime_error);
  EXPECT_THROW(config->GetInteger64List("not-a-list.no-such-key"),
               wkc::KeyError);
  EXPECT_THROW(config->GetStringList("an_int"), std::runtime_error);
  EXPECT_THROW(config->GetStringList("not-a-list"), std::runtime_error);
  EXPECT_THROW(config->GetStringList("not-a-list.no-such-key"), wkc::KeyError);

  // Cannot load an inhomogeneous array:
  EXPECT_THROW(config->GetInteger32List("mixed_types"), std::runtime_error);
  EXPECT_THROW(config->GetStringList("mixed_types"), std::runtime_error);

  auto list32 = config->GetInteger32List("ints32");
  EXPECT_EQ(8, list32.size());
  auto list64 = config->GetInteger64List("ints32");
  EXPECT_EQ(8, list64.size());
  EXPECT_EQ(1, list32[0]);
  EXPECT_EQ(6, list32[5]);
  EXPECT_EQ(-8, list32[7]);

  // Cannot load integers as other types:
  EXPECT_THROW(config->GetDoubleList("ints32"), std::runtime_error);
  EXPECT_THROW(config->GetStringList("ints32"), std::runtime_error);

  EXPECT_THROW(config->GetInteger32List("ints64"), std::runtime_error);
  list64 = config->GetInteger64List("ints64");
  EXPECT_EQ(5, list64.size());

  auto list_dbl = config->GetDoubleList("floats");
  EXPECT_EQ(3, list_dbl.size());
  EXPECT_DOUBLE_EQ(0.5, list_dbl[0]);
  EXPECT_DOUBLE_EQ(1.0, list_dbl[1]);
  EXPECT_DOUBLE_EQ(1e23, list_dbl[2]);

  // Cannot load floats as other types:
  EXPECT_THROW(config->GetInteger32List("floats"), std::runtime_error);
  EXPECT_THROW(config->GetInteger64List("floats"), std::runtime_error);
  EXPECT_THROW(config->GetStringList("floats"), std::runtime_error);

  EXPECT_THROW(config->GetInteger32List("invalid_int_flt"), std::runtime_error);
  EXPECT_THROW(config->GetInteger64List("invalid_int_flt"), std::runtime_error);
  EXPECT_THROW(config->GetDoubleList("invalid_int_flt"), std::runtime_error);
}

TEST(ConfigTest, Pairs) {
  const auto config = wkc::Configuration::LoadTOMLString(R"toml(
    int_list = [1, 2, 3, 4]

    int32_pair = [1024, 768]

    int64_pair = [2147483647, 2147483648]

    float_pair = [0.5, 1.0]

    mixed_types = [1, "framboozle"]

    a_scalar = 1234

    nested_array = [1, [2, [3, 4]]]
    )toml");

  // Key error:
  EXPECT_THROW(config->GetInteger32Pair("no-such-key"), wkc::KeyError);
  EXPECT_THROW(config->GetInteger64Pair("no-such-key"), wkc::KeyError);
  EXPECT_THROW(config->GetDoublePair("no-such-key"), wkc::KeyError);

  // A pair must be an array of 2 elements
  EXPECT_THROW(config->GetInteger32Pair("int_list"), std::runtime_error);
  EXPECT_THROW(config->GetInteger64Pair("int_list"), std::runtime_error);
  EXPECT_THROW(config->GetDoublePair("int_list"), std::runtime_error);

  EXPECT_THROW(config->GetInteger32Pair("mixed_types"), std::runtime_error);
  EXPECT_THROW(config->GetInteger64Pair("mixed_types"), std::runtime_error);
  EXPECT_THROW(config->GetDoublePair("mixed_types"), std::runtime_error);

  EXPECT_THROW(config->GetInteger32Pair("a_scalar"), std::runtime_error);
  EXPECT_THROW(config->GetInteger64Pair("a_scalar"), std::runtime_error);
  EXPECT_THROW(config->GetDoublePair("a_scalar"), std::runtime_error);

  EXPECT_THROW(config->GetInteger32Pair("nested_array"), std::runtime_error);
  EXPECT_THROW(config->GetInteger64Pair("nested_array"), std::runtime_error);
  EXPECT_THROW(config->GetDoublePair("nested_array"), std::runtime_error);

  // Load a valid pair
  auto p32 = config->GetInteger32Pair("int32_pair");
  EXPECT_EQ(1024, p32.first);
  EXPECT_EQ(768, p32.second);

  EXPECT_THROW(config->GetInteger32Pair("int64_pair"), std::runtime_error);
  auto p64 = config->GetInteger64Pair("int64_pair");
  EXPECT_EQ(2147483647, p64.first);
  EXPECT_EQ(2147483648, p64.second);

  EXPECT_THROW(config->GetInteger32Pair("float_pair"), std::runtime_error);
  EXPECT_THROW(config->GetInteger64Pair("float_pair"), std::runtime_error);
  auto pdbl = config->GetDoublePair("float_pair");
  EXPECT_DOUBLE_EQ(0.5, pdbl.first);
  EXPECT_DOUBLE_EQ(1.0, pdbl.second);
}

TEST(ConfigTest, NestedTOML) {
  const auto fname_invalid_toml =
      wkf::FullFile(wkf::DirName(__FILE__), "test-invalid.toml"sv);
  std::ostringstream toml_str;
  toml_str << "integer = 3\n"
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

  auto config = wkc::Configuration::LoadTOMLString(toml_str.str());
  EXPECT_THROW(config->LoadNestedTOMLConfiguration("no-such-key"sv),
               wkc::KeyError);
  EXPECT_THROW(config->LoadNestedTOMLConfiguration("integer"sv),
               std::runtime_error);
  config->LoadNestedTOMLConfiguration("nested_config"sv);

  EXPECT_EQ(1, config->GetInteger32("nested_config.value1"sv));
  EXPECT_DOUBLE_EQ(2.3, config->GetDouble("nested_config.value2"sv));
  EXPECT_EQ("this/is/a/relative/path",
            config->GetString("nested_config.section1.rel_path"sv));

  // When trying to load an invalid TOML file, an exception should be thrown,
  // and the parameter should not change.
  EXPECT_THROW(config->LoadNestedTOMLConfiguration("invalid_nested_config"sv),
               std::runtime_error);
  EXPECT_EQ(fname_invalid_toml, config->GetString("invalid_nested_config"));

  // Ensure that loading a nested configuration also works at deeper
  // hierarchy levels.
  EXPECT_NO_THROW(
      config->LoadNestedTOMLConfiguration("lvl1.lvl2.lvl3.nested"sv));
  EXPECT_DOUBLE_EQ(2.3, config->GetDouble("lvl1.lvl2.lvl3.nested.value2"sv));
  EXPECT_EQ("this/is/a/relative/path",
            config->GetString("lvl1.lvl2.lvl3.nested.section1.rel_path"sv));

  // It is not allowed to load a nested configuration directly into an array:
  EXPECT_THROW(config->LoadNestedTOMLConfiguration("lvl1.arr[2]"),
               std::runtime_error);

  // One could abuse it, however, to load a nested configuration into a table
  // that is inside an array... Just because you can doesn't mean you should...
  EXPECT_NO_THROW(
      config->LoadNestedTOMLConfiguration("lvl1.another_arr[1].nested"sv));
  EXPECT_DOUBLE_EQ(2.3,
                   config->GetDouble("lvl1.another_arr[1].nested.value2"sv));
  EXPECT_EQ(
      "this/is/a/relative/path",
      config->GetString("lvl1.another_arr[1].nested.section1.rel_path"sv));
}

TEST(ConfigTest, AbsolutePaths) {
  const std::string fname =
      wkf::FullFile(wkf::DirName(__FILE__), "test-valid1.toml");
  auto config = wkc::Configuration::LoadTOMLFile(fname);

  EXPECT_FALSE(config->AdjustRelativePaths("...", {"no-such-key"sv}));
  EXPECT_TRUE(config->AdjustRelativePaths(wkf::DirName(__FILE__),
                                          {"section1.*path"sv}));

  std::string expected =
      wkf::FullFile(wkf::DirName(__FILE__), "this/is/a/relative/path"sv);
  EXPECT_EQ(expected, config->GetString("section1.rel_path"sv));

  expected =
      "file://" + wkf::FullFile(wkf::DirName(__FILE__), "also/relative"sv);
  EXPECT_EQ(expected, config->GetString("section1.rel_url_path"sv));

  // TODO check special character handling (backslash, umlauts,
  // whitespace)

  EXPECT_THROW(config->AdjustRelativePaths("this-will-throw", {"value1"sv}),
               std::runtime_error);
  EXPECT_THROW(
      config->AdjustRelativePaths("this-will-throw", {"section1.time"sv}),
      std::runtime_error);
}

TEST(ConfigTest, StringReplacements) {
  auto config = wkc::Configuration::LoadTOMLString(R"toml(
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
    )toml");

  EXPECT_FALSE(config->ReplaceStringPlaceholders({}));
  EXPECT_FALSE(
      config->ReplaceStringPlaceholders({{"no-such-text"sv, "bar"sv}}));
  // Invalid search string
  EXPECT_THROW(config->ReplaceStringPlaceholders({{""sv, "replace"sv}}),
               std::runtime_error);

  // Replace words
  EXPECT_TRUE(config->ReplaceStringPlaceholders(
      {{"test"sv, "123"sv}, {"world"sv, "replacement"sv}}));
  // Already replaced
  EXPECT_FALSE(config->ReplaceStringPlaceholders(
      {{"test"sv, "123"sv}, {"world"sv, "replacement"sv}}));

  EXPECT_EQ("", config->GetString("str1"));
  EXPECT_EQ("This is a 123", config->GetString("str2"));
  EXPECT_EQ("Hello replacement!", config->GetString("str3"));
  EXPECT_EQ(123, config->GetInteger32("value"));
  EXPECT_EQ("List 123", config->GetString("str_list[0]"));
  EXPECT_EQ("Frobmorten", config->GetString("str_list[1]"));
  EXPECT_EQ("Another 123!", config->GetString("table.str1"));
  EXPECT_EQ("Untouched", config->GetString("table.str2"));
  EXPECT_EQ("%TOREP%/C", config->GetString("configs[2].name"));

  EXPECT_TRUE(config->ReplaceStringPlaceholders({{"%TOREP%", "..."}}));
  EXPECT_EQ(".../a", config->GetString("configs[0].name"));
  EXPECT_EQ(".../b", config->GetString("configs[1].name"));
  EXPECT_EQ(".../C", config->GetString("configs[2].name"));
  EXPECT_EQ(".../D", config->GetString("configs[3].name"));
}

TEST(ConfigTest, LoadingToml) {
  const std::string fname =
      wkf::FullFile(wkf::DirName(__FILE__), "test-valid1.toml");

  // Load valid TOML, then reload its string representation
  const auto config1 = wkc::Configuration::LoadTOMLFile(fname);
  const auto reloaded = wkc::Configuration::LoadTOMLString(config1->ToTOML());
  EXPECT_TRUE(config1->Equals(reloaded.get()));
  EXPECT_TRUE(reloaded->Equals(config1.get()));
  // Also the string representations should be equal
  EXPECT_EQ(config1->ToTOML(), reloaded->ToTOML());

  // Load a different configuration:
  const auto config2 = wkc::Configuration::LoadTOMLString(R"toml(
    param1 = "value"
    param2 = "value"

    param3 = true
    )toml");
  EXPECT_FALSE(config1->Equals(config2.get()));
  EXPECT_FALSE(config2->Equals(config1.get()));

  // Identity check
  EXPECT_TRUE(config1->Equals(config1.get()));
  EXPECT_TRUE(config2->Equals(config2.get()));

  // White space mustn't affect the equality check
  auto config3 = wkc::Configuration::LoadTOMLString(R"toml(

    param1 =     "value"


    param2 =  "value"

    param3         = true

    )toml");

  EXPECT_FALSE(config1->Equals(config3.get()));
  EXPECT_TRUE(config2->Equals(config3.get()));
  EXPECT_TRUE(config3->Equals(config2.get()));

  // Change the first string parameter
  config3 = wkc::Configuration::LoadTOMLString(R"toml(
    param1 = "value!"
    param2 = "value"

    param3 = true
    )toml");
  EXPECT_FALSE(config1->Equals(config3.get()));
  EXPECT_FALSE(config2->Equals(config3.get()));
  EXPECT_FALSE(config3->Equals(config2.get()));

  // Change the 3rd parameter type
  config3 = wkc::Configuration::LoadTOMLString(R"toml(
    param1 = "value"
    param2 = "value"

    param3 = [1, 2]
    )toml");
  EXPECT_FALSE(config1->Equals(config3.get()));
  EXPECT_FALSE(config2->Equals(config3.get()));
  EXPECT_FALSE(config3->Equals(config2.get()));

  // Edge cases for equality comparison:
  EXPECT_FALSE(config1->Equals(nullptr));

  const auto empty = wkc::Configuration::LoadTOMLString("");
  EXPECT_FALSE(empty->Equals(config1.get()));
  EXPECT_FALSE(config1->Equals(empty.get()));

  // Edge cases for TOML loading:
  EXPECT_THROW(wkc::Configuration::LoadTOMLFile("this-does-not-exist.toml"),
               std::runtime_error);

  const std::string fname_invalid =
      wkf::FullFile(wkf::DirName(__FILE__), "test-invalid.toml");
  EXPECT_THROW(wkc::Configuration::LoadTOMLFile(fname_invalid),
               std::runtime_error);
}

// TODO Can be properly tested once we have LoadJSONString()
TEST(ConfigTest, LoadingJson) {
  const auto config = wkc::Configuration::LoadTOMLString(R"toml(
    param1 = "value"
    )toml");
  EXPECT_TRUE(config->ToJSON().length() > 0);
}

// NOLINTEND
