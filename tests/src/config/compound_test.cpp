#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/geometry/vector.h>

#include "../test_utils.h"

namespace wkc = werkzeugkiste::config;
namespace wkg = werkzeugkiste::geometry;

using namespace std::string_view_literals;

// NOLINTBEGIN

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

TEST(ConfigCompoundTest, IndexLists) {
  const auto config = wkc::LoadTOMLString(R"toml(
    str = "not an index list"

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
  // TODO p3 and others should be retrievable as Points (i.e. floating points)
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

TEST(ConfigCompoundTest, Pairs) {
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

TEST(ConfigCompoundTest, GetGroup) {
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
  auto keys = sub.ListParameterNames(true, false);
  CheckMatchingContainers({"str", "lst"}, keys);
  keys = sub.ListParameterNames(true, true);
  CheckMatchingContainers({"str", "lst", "lst[0]", "lst[1]"}, keys);

  sub = config.GetGroup("lvl1.grp2"sv);
  EXPECT_FALSE(sub.Empty());
  keys = sub.ListParameterNames(false, false);
  CheckMatchingContainers({"str", "val"}, keys);
  keys = sub.ListParameterNames(false, true);
  CheckMatchingContainers({"str", "val"}, keys);

  sub = config.GetGroup("lvl1"sv);
  EXPECT_FALSE(sub.Empty());
  keys = sub.ListParameterNames(true, true);
  const std::vector<std::string> expected{"flt",
      "grp1",
      "grp1.str",
      "grp1.lst",
      "grp1.lst[0]",
      "grp1.lst[1]",
      "grp2",
      "grp2.str",
      "grp2.val",
      "grp3"};
  CheckMatchingContainers(expected, keys);

  // Empty sub-group
  sub = config.GetGroup("lvl1.grp3"sv);
  EXPECT_TRUE(sub.Empty());
  keys = sub.ListParameterNames(false, false);
  EXPECT_EQ(0, keys.size());
  keys = sub.ListParameterNames(false, true);
  EXPECT_EQ(0, keys.size());
}

TEST(ConfigCompoundTest, SetGroup) {
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

  auto keys = group.ListParameterNames(true, true);
  CheckMatchingContainers({"my-bool", "my-int32", "my-str"}, keys);

  // Insert group at root level
  EXPECT_NO_THROW(config.SetGroup("my-grp"sv, empty));
  EXPECT_TRUE(config.Contains("my-grp.my-bool"sv));
  EXPECT_TRUE(config.Contains("my-grp.my-int32"sv));
  EXPECT_TRUE(config.Contains("my-grp.my-str"sv));
}

// NOLINTEND
