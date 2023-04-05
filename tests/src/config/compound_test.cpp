#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/geometry/vector.h>

#include "../test_utils.h"

namespace wkc = werkzeugkiste::config;

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

TEST(ConfigCompoundTest, Points) {
  const auto config = wkc::LoadTOMLString(R"toml(
    str = "not an index list"

    poly1 = [[1, 2], [3, 4], [5, 6], [-7, -8]]

    poly2 = [{y = 20, x = 10}, {x = 30, y = 40}, {y = 60, x = 50}]

    poly3 = [[1, 2, 3], [4, 5, 6], {x = -9, y = 0, z = -3}]
    )toml"sv);

  // 2D integral point
  EXPECT_THROW(config.GetInteger64Point2D("no-such-key"sv), wkc::KeyError);
  EXPECT_THROW(config.GetInteger64Point2D("str"sv), wkc::TypeError);

  EXPECT_THROW(config.GetInteger64Point2D("poly1"sv), wkc::TypeError);
  auto p2i = config.GetInteger64Point2D("poly1[0]"sv);
  EXPECT_EQ(1, p2i.x);
  EXPECT_EQ(2, p2i.y);

  p2i = config.GetInteger64Point2D("poly2[2]"sv);
  EXPECT_EQ(50, p2i.x);
  EXPECT_EQ(60, p2i.y);

  p2i = config.GetInteger64Point2D("poly3[0]"sv);
  EXPECT_EQ(1, p2i.x);
  EXPECT_EQ(2, p2i.y);

  p2i = config.GetInteger64Point2D("poly3[2]"sv);
  EXPECT_EQ(-9, p2i.x);
  EXPECT_EQ(0, p2i.y);

  // 3D integral point
  EXPECT_THROW(config.GetInteger64Point3D("no-such-key"sv), wkc::KeyError);
  EXPECT_THROW(config.GetInteger64Point3D("str"sv), wkc::TypeError);

  EXPECT_THROW(config.GetInteger64Point3D("poly1"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger64Point3D("poly1[0]"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger64Point3D("poly2[0]"sv), wkc::TypeError);

  auto p3i = config.GetInteger64Point3D("poly3[0]"sv);
  EXPECT_EQ(1, p3i.x);
  EXPECT_EQ(2, p3i.y);
  EXPECT_EQ(3, p3i.z);

  p3i = config.GetInteger64Point3D("poly3[2]"sv);
  EXPECT_EQ(-9, p3i.x);
  EXPECT_EQ(0, p3i.y);
  EXPECT_EQ(-3, p3i.z);

  // Double precision points
  EXPECT_THROW(config.GetDoublePoint2D("no-such-key"sv), wkc::KeyError);
  EXPECT_THROW(config.GetDoublePoint2D("str"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDoublePoint2D("poly1"sv), wkc::TypeError);
  auto p2d = config.GetDoublePoint2D("poly1[0]"sv);
  EXPECT_DOUBLE_EQ(1.0, p2d.x);
  EXPECT_DOUBLE_EQ(2.0, p2d.y);

  p2d = config.GetDoublePoint2D("poly2[2]"sv);
  EXPECT_DOUBLE_EQ(50.0, p2d.x);
  EXPECT_DOUBLE_EQ(60.0, p2d.y);

  EXPECT_THROW(config.GetDoublePoint3D("no-such-key"sv), wkc::KeyError);
  EXPECT_THROW(config.GetDoublePoint3D("str"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDoublePoint3D("poly1"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDoublePoint3D("poly1[0]"sv), wkc::TypeError);

  auto p3d = config.GetDoublePoint3D("poly3[0]"sv);
  EXPECT_DOUBLE_EQ(1.0, p3d.x);
  EXPECT_DOUBLE_EQ(2.0, p3d.y);
  EXPECT_DOUBLE_EQ(3.0, p3d.z);

  p3d = config.GetDoublePoint3D("poly3[2]"sv);
  EXPECT_DOUBLE_EQ(-9.0, p3d.x);
  EXPECT_DOUBLE_EQ(0.0, p3d.y);
  EXPECT_DOUBLE_EQ(-3.0, p3d.z);
}

TEST(ConfigCompoundTest, PointLists) {
  const auto config = wkc::LoadTOMLString(R"toml(
    str = "not an index list"

    poly1 = [[1, 2], [3, 4], [5, 6], [-7, -8]]

    poly2 = [{y = 20, x = 10}, {x = 30, y = 40}, {y = 60, x = 50}]

    poly3 = [[1, 2, 3], [4, 5, 6], {x = -9, y = 0, z = -3}]

    poly64 = [[-10, 20], [1, 3], [2147483647, 2147483648], [0, 21474836480]]

    poly_flt = [[1e3, 2e3], {x = -3.5, y = -4.5}]

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

    # Mix "points" (nested arrays) and scalars
    p2 = [[1, 2], [3, 4], 5]
    p3 = [[1, 2], [3, 4], [5]]

    [mixed_2d_3d]
    # 2D & 3D point (They can be loaded as 2D polygon, but not 3D)
    p1 = [{x = 1, y = 2}, {x = 1, y = 2, z = 3}]
    p2 = [[1, 2], [3, 4, 5], [6, 7]]

    )toml"sv);

  // Sanity checks
  EXPECT_THROW(config.GetInteger64Points2D("str"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger64Points3D("str"sv), wkc::TypeError);

  EXPECT_THROW(config.GetInteger32List("str"sv), wkc::TypeError);
  EXPECT_THROW(config.GetBooleanList("str"sv), wkc::TypeError);
  EXPECT_THROW(config.GetBooleanList("poly1"sv), wkc::TypeError);

  // Retrieve a polyline
  auto poly = config.GetInteger64Points2D("poly1"sv);
  EXPECT_EQ(4, poly.size());

  auto list = config.GetInteger32List("poly1[0]"sv);
  EXPECT_EQ(2, list.size());
  EXPECT_EQ(1, list[0]);
  EXPECT_EQ(2, list[1]);
  list = config.GetInteger32List("poly1[2]"sv);
  EXPECT_EQ(2, list.size());
  EXPECT_EQ(5, list[0]);
  EXPECT_EQ(6, list[1]);

  EXPECT_EQ(1, poly[0].x);
  EXPECT_EQ(2, poly[0].y);

  EXPECT_EQ(3, poly[1].x);
  EXPECT_EQ(4, poly[1].y);

  EXPECT_EQ(5, poly[2].x);
  EXPECT_EQ(6, poly[2].y);

  EXPECT_EQ(-7, poly[3].x);
  EXPECT_EQ(-8, poly[3].y);

  poly = config.GetInteger64Points2D("poly2"sv);
  EXPECT_EQ(3, poly.size());

  EXPECT_EQ(10, poly[0].x);
  EXPECT_EQ(20, poly[0].y);
  EXPECT_EQ(30, poly[1].x);
  EXPECT_EQ(40, poly[1].y);
  EXPECT_EQ(50, poly[2].x);
  EXPECT_EQ(60, poly[2].y);

  // Cannot load an array of tables as a scalar list:
  EXPECT_THROW(config.GetInteger32List("poly2"sv), wkc::TypeError);

  // An N-dimensional polygon can be looked up from any list of at
  // least N-dimensional points:
  EXPECT_NO_THROW(config.GetInteger64Points2D("poly3"sv));
  EXPECT_NO_THROW(config.GetInteger64Points3D("poly3"sv));

  EXPECT_NO_THROW(config.GetDoublePoints2D("poly3"sv));
  EXPECT_NO_THROW(config.GetDoublePoints3D("poly3"sv));

  EXPECT_NO_THROW(config.GetInteger64Points2D("poly4"sv));
  EXPECT_NO_THROW(config.GetInteger64Points3D("poly4"sv));

  EXPECT_NO_THROW(config.GetDoublePoints2D("poly4"sv));
  EXPECT_NO_THROW(config.GetDoublePoints3D("poly4"sv));

  // 'poly64' contains values that would overflow 32-bit ints
  // EXPECT_THROW(config.GetIndices2D("poly64"sv), wkc::TypeError);
  EXPECT_NO_THROW(config.GetInteger64Points2D("poly64"sv));
  EXPECT_NO_THROW(config.GetDoublePoints2D("poly64"sv));

  // TODO p3 and others should be retrievable as Points (i.e. floating points)
  EXPECT_THROW(config.GetInteger64Points2D("no-such-key"sv), wkc::KeyError);
  EXPECT_THROW(config.GetInteger64Points2D("str"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger64Points2D("invalid.p1"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger64Points2D("invalid.p2"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger64Points2D("invalid.p3"sv), wkc::TypeError);

  EXPECT_THROW(config.GetDoublePoints2D("no-such-key"sv), wkc::KeyError);
  EXPECT_THROW(config.GetDoublePoints2D("str"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDoublePoints2D("invalid.p1"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDoublePoints2D("invalid.p2"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDoublePoints2D("invalid.p3"sv), wkc::TypeError);

  // A "point" parameter can have more values than the dimensionality of
  // the queried points (e.g. to load a list of 3D coordinates as 2D points)
  EXPECT_NO_THROW(config.GetInteger64Points2D("mixed_2d_3d.p1"sv));
  EXPECT_THROW(config.GetInteger64Points3D("mixed_2d_3d.p1"sv), wkc::TypeError);
  EXPECT_NO_THROW(config.GetDoublePoints2D("mixed_2d_3d.p1"sv));
  EXPECT_THROW(config.GetDoublePoints3D("mixed_2d_3d.p1"sv), wkc::TypeError);

  EXPECT_NO_THROW(config.GetInteger64Points2D("mixed_2d_3d.p2"sv));
  EXPECT_THROW(config.GetInteger64Points3D("mixed_2d_3d.p2"sv), wkc::TypeError);
  EXPECT_NO_THROW(config.GetDoublePoints2D("mixed_2d_3d.p2"sv));
  EXPECT_THROW(config.GetDoublePoints3D("mixed_2d_3d.p2"sv), wkc::TypeError);

  // 3D polygons
  EXPECT_THROW(config.GetInteger64Points3D("poly1"sv), wkc::TypeError);
  EXPECT_THROW(config.GetInteger64Points3D("poly2"sv), wkc::TypeError);

  auto poly3d = config.GetInteger64Points3D("poly3"sv);
  EXPECT_EQ(3, poly3d.size());

  EXPECT_EQ(1, poly3d[0].x);
  EXPECT_EQ(2, poly3d[0].y);
  EXPECT_EQ(3, poly3d[0].z);

  EXPECT_EQ(4, poly3d[1].x);
  EXPECT_EQ(5, poly3d[1].y);
  EXPECT_EQ(6, poly3d[1].z);

  EXPECT_EQ(-9, poly3d[2].x);
  EXPECT_EQ(0, poly3d[2].y);
  EXPECT_EQ(-3, poly3d[2].z);

  // Load the same point list as floating point
  auto poly3d_dbl = config.GetDoublePoints3D("poly3"sv);
  EXPECT_EQ(3, poly3d_dbl.size());

  EXPECT_DOUBLE_EQ(1.0, poly3d_dbl[0].x);
  EXPECT_DOUBLE_EQ(2.0, poly3d_dbl[0].y);
  EXPECT_DOUBLE_EQ(3.0, poly3d_dbl[0].z);

  EXPECT_DOUBLE_EQ(4.0, poly3d_dbl[1].x);
  EXPECT_DOUBLE_EQ(5.0, poly3d_dbl[1].y);
  EXPECT_DOUBLE_EQ(6.0, poly3d_dbl[1].z);

  EXPECT_DOUBLE_EQ(-9.0, poly3d_dbl[2].x);
  EXPECT_DOUBLE_EQ(0.0, poly3d_dbl[2].y);
  EXPECT_DOUBLE_EQ(-3.0, poly3d_dbl[2].z);

  // Floating point polygon only:
  EXPECT_THROW(config.GetInteger64Points2D("poly_flt"sv), wkc::TypeError);
  EXPECT_THROW(config.GetDoublePoints3D("poly_flt"sv), wkc::TypeError);

  auto poly2d_dbl = config.GetDoublePoints2D("poly_flt"sv);
  EXPECT_EQ(2, poly2d_dbl.size());
  EXPECT_DOUBLE_EQ(1e3, poly2d_dbl[0].x);
  EXPECT_DOUBLE_EQ(2e3, poly2d_dbl[0].y);

  EXPECT_DOUBLE_EQ(-3.5, poly2d_dbl[1].x);
  EXPECT_DOUBLE_EQ(-4.5, poly2d_dbl[1].y);
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

  EXPECT_THROW(
      config.ListParameterNames("lvl1.flt"sv, false, false), wkc::TypeError);

  keys = config.ListParameterNames("lvl1.grp1"sv, false, false);
  CheckMatchingContainers({"str", "lst"}, keys);

  keys = config.ListParameterNames("lvl1"sv, false, false);
  CheckMatchingContainers({"flt", "grp1", "grp2", "grp3"}, keys);

  keys = config.ListParameterNames("lvl1"sv, false, true);
  CheckMatchingContainers({"flt",
                              "grp1",
                              "grp1.str",
                              "grp1.lst",
                              "grp2",
                              "grp2.str",
                              "grp2.val",
                              "grp3"},
      keys);

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
