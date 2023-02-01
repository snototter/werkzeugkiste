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
TEST(ConfigTest, Integers) {
  const auto config = wkc::Configuration::LoadTomlString(R"toml(
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
  EXPECT_THROW(config->GetInteger32("test"), std::runtime_error);

  EXPECT_EQ(-123456, config->GetInteger64("int32_1"));
  EXPECT_EQ(+987654, config->GetInteger64("int32_2"));
  EXPECT_EQ(-2147483649, config->GetInteger64("int32_min_overflow"));
  EXPECT_EQ(+2147483648, config->GetInteger64("int32_max_overflow"));
  EXPECT_EQ(-1, config->GetInteger64OrDefault("test", -1));
  EXPECT_EQ(17, config->GetInteger64OrDefault("another", 17));
  EXPECT_THROW(config->GetInteger64("test"), std::runtime_error);
}

TEST(ConfigTest, FloatingPoint) {
  const auto config = wkc::Configuration::LoadTomlString(R"toml(
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
  EXPECT_THROW(config->GetDouble("test"), std::runtime_error);
}

TEST(ConfigTest, Types) {
  const std::string toml_str = R"toml(
    bool = true
    int = 42
    flt = 1.0
    str = "A string" #TODO others, too!
    )toml";
  const auto config = wkc::Configuration::LoadTomlString(toml_str);

  // Boolean parameter
  EXPECT_EQ(true, config->GetBoolean("bool"));
  EXPECT_THROW(config->GetBoolean("no-such.bool"), std::runtime_error);
  EXPECT_TRUE(config->GetBooleanOrDefault("no-such.bool", true));
  EXPECT_FALSE(config->GetBooleanOrDefault("no-such.bool", false));

  EXPECT_THROW(config->GetInteger32("bool"), std::runtime_error);
  EXPECT_THROW(config->GetInteger32OrDefault("bool", 0), std::runtime_error);
  EXPECT_THROW(config->GetInteger64("bool"), std::runtime_error);
  EXPECT_THROW(config->GetInteger64OrDefault("bool", 2), std::runtime_error);
  EXPECT_THROW(config->GetDouble("bool"), std::runtime_error);
  EXPECT_THROW(config->GetDoubleOrDefault("bool", 1.0), std::runtime_error);

  // Integer parameter
  EXPECT_EQ(42, config->GetInteger32("int"));
  EXPECT_EQ(42, config->GetInteger64("int"));

  EXPECT_THROW(config->GetBoolean("int"), std::runtime_error);
  EXPECT_THROW(config->GetBooleanOrDefault("int", true), std::runtime_error);
  EXPECT_THROW(config->GetDouble("int"), std::runtime_error);

  // Double parameter
  EXPECT_DOUBLE_EQ(1.0, config->GetDouble("flt"));

  EXPECT_THROW(config->GetBoolean("flt"), std::runtime_error);
  EXPECT_THROW(config->GetInteger32("flt"), std::runtime_error);
  EXPECT_THROW(config->GetInteger64("flt"), std::runtime_error);

  // String parameter
  // TODO

  EXPECT_THROW(config->GetBoolean("str"), std::runtime_error);
  EXPECT_THROW(config->GetInteger32("str"), std::runtime_error);
  EXPECT_THROW(config->GetInteger64("str"), std::runtime_error);
}

TEST(ConfigTest, Keys1) {
  const std::string toml_str = R"toml(
    key = "value"
    other-key = 0
    another_key = 1
    1234 = "value"

    tbl1.param1 = "value"
    tbl1.param2 = 'value'
    )toml";
  const auto config = wkc::Configuration::LoadTomlString(toml_str);

  const auto keys = config->ParameterNames();
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

std::string Stringify(const std::vector<std::string> &v) {
  std::ostringstream s;
  s << "{";
  for (std::size_t idx = 0; idx < v.size(); ++idx) {
    if (idx > 0) {
      s << ", ";
    }
    s << v[idx];
  }
  s << "}";
  return s.str();
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

    [[tests]]
    name = "value"

    [[tests]]

    [[tests]]
    param = "value"
    )toml";
  const auto config = wkc::Configuration::LoadTomlString(toml_str);

  // We don't extract "names" of scalar array entries, e.g. "tests[1]" below.
  // Currently, I see no need to change this.
  const std::vector<std::string> expected_keys{
      "arr1",       "arr1[1].first", "arr1[1].second",     "lvl-1",
      "lvl-1.arr2", "lvl-1.lvl-2",   "lvl-1.lvl-2.param1", "lvl-1.lvl-2.param2",
      "tests",      "tests[0].name", "tests[2].param"};
  const auto keys = config->ParameterNames();

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

  // TODO
}

template <typename Vec, typename Tpl>
inline std::vector<Vec> ToGeoVec(const std::vector<Tpl> &tuples) {
  std::vector<Vec> vecs;
  for (const auto &tpl : tuples) {
    vecs.emplace_back(std::make_from_tuple<Vec>(tpl));
  }
  return vecs;
}

TEST(ConfigTest, Polygon) {
  const auto config = wkc::Configuration::LoadTomlString(R"toml(
    poly1 = [[1, 2], [3, 4], [5, 6], [-7, -8]]

    poly2 = [{y = 20, x = 10}, {x = 30, y = 40}, {y = 60, x = 50}]

    poly3 = [[1, 2, 3], [4, 5, 6]]

    [[poly4]]
    x = 100
    y = 200
    z = -5

    [[poly4]]
    x = 300
    y = 400
    z = -5

    [invalid]
    p1 = [{x = 1, y = 2}, {x = 1, name = 2, param = 3}]
    #p1 = [{x = 1, y = 2}, {x = 1, y = 2, z = 3}] # Dimensionality mix - TODO should be supported
    p2 = [{x = 1, y = 2}, {x = 1.5, y = 2}]      # Type mix
    )toml");

  const auto poly1 = config->GetPolygon2D("poly1");
  EXPECT_EQ(4, poly1.size());

  std::vector<wkg::Vec2i> vec = ToGeoVec<wkg::Vec2i>(poly1);
  EXPECT_EQ(wkg::Vec2i(1, 2), vec[0]);
  EXPECT_EQ(wkg::Vec2i(3, 4), vec[1]);
  EXPECT_EQ(wkg::Vec2i(5, 6), vec[2]);
  EXPECT_EQ(wkg::Vec2i(-7, -8), vec[3]);

  const auto poly2 = config->GetPolygon2D("poly2");
  EXPECT_EQ(3, poly2.size());

  vec = ToGeoVec<wkg::Vec2i>(poly2);
  EXPECT_EQ(wkg::Vec2i(10, 20), vec[0]);
  EXPECT_EQ(wkg::Vec2i(30, 40), vec[1]);
  EXPECT_EQ(wkg::Vec2i(50, 60), vec[2]);

  // const auto poly3 = config->GetPolygon2D("poly3");
  EXPECT_THROW(config->GetPolygon2D("poly3"), std::runtime_error);
  EXPECT_THROW(config->GetPolygon2D("poly4"), std::runtime_error);

  EXPECT_THROW(config->GetPolygon2D("invalid.p1"), std::runtime_error);
  EXPECT_THROW(config->GetPolygon2D("invalid.p2"), std::runtime_error);
  EXPECT_THROW(config->GetPolygon2D("no-such-key"), std::runtime_error);

  // 3D polygons
  EXPECT_THROW(config->GetPolygon3D("poly1"), std::runtime_error);
  EXPECT_THROW(config->GetPolygon3D("poly2"), std::runtime_error);

  const auto poly3 = config->GetPolygon3D("poly3");
  EXPECT_EQ(2, poly3.size());
  std::vector<wkg::Vec3i> vec3 = ToGeoVec<wkg::Vec3i>(poly3);
  EXPECT_EQ(wkg::Vec3i(1, 2, 3), vec3[0]);
  EXPECT_EQ(wkg::Vec3i(4, 5, 6), vec3[1]);
}

TEST(ConfigTest, LoadingToml) {
  const std::string fname =
      wkf::FullFile(wkf::DirName(__FILE__), "test-valid1.toml");

  // Load valid TOML, then reload its string representation
  const auto config1 = wkc::Configuration::LoadTomlFile(fname);
  const auto reloaded = wkc::Configuration::LoadTomlString(config1->ToTOML());
  EXPECT_TRUE(config1->Equals(reloaded.get()));
  EXPECT_TRUE(reloaded->Equals(config1.get()));
  // Also the string representations should be equal
  EXPECT_EQ(config1->ToTOML(), reloaded->ToTOML());

  // Load a different configuration:
  const auto config2 = wkc::Configuration::LoadTomlString(R"toml(
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
  const auto config3 = wkc::Configuration::LoadTomlString(R"toml(

    param1 =     "value"


    param2 =  "value"

    param3         = true

    )toml");

  EXPECT_FALSE(config1->Equals(config3.get()));
  EXPECT_TRUE(config2->Equals(config3.get()));
  EXPECT_TRUE(config3->Equals(config2.get()));

  // Edge cases for equality comparison:
  EXPECT_FALSE(config1->Equals(nullptr));

  const auto empty = wkc::Configuration::LoadTomlString("");
  EXPECT_FALSE(empty->Equals(config1.get()));
  EXPECT_FALSE(config1->Equals(empty.get()));

  // Edge cases for TOML loading:
  EXPECT_THROW(wkc::Configuration::LoadTomlFile("this-does-not-exist.toml"),
               std::runtime_error);

  const std::string fname_invalid =
      wkf::FullFile(wkf::DirName(__FILE__), "test-invalid.toml");
  EXPECT_THROW(wkc::Configuration::LoadTomlFile(fname_invalid),
               std::runtime_error);
}

// NOLINTEND
