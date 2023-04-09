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

TEST(ConfigCompoundTest, GetMatrices) {
  auto config = wkc::LoadTOMLString(R"toml(
    int = 3

    lst-int = [1, 2, 3]
    lst-flt = [0.0, -3.5, 1e6]
    lst-convertible = [42, 20.0, -3.0, 0, 123.0]

    empty = []

    mat-uint8 = [
      [0, 127],
      [10, 100],
      [32, 64]
    ]

    mat-int32 = [
      [-3,  2, 17],
      [ 0, 19, 42],  # Trailing comma is also allowed
    ]

    mat-int64 = [
      [2147483648, -2147483649], # int32 overflow & underflow
      [1, 2]
    ]

    # Not a valid 2d matrix:
    invalid1 = [[1], 3]

    # Contains a non-numeric value:
    invalid2 = [[1, 2], [3, 'four']]

    # Jagged array:
    invalid3 = [[1, 2], [3]]
    )toml"sv);

  // -------------------------------------------------------------
  // ---- 8-bit unsigned integer
  // Invalid queries
  EXPECT_THROW(config.GetMatrixUInt8("no-such-key"sv), wkc::KeyError);
  EXPECT_THROW(config.GetMatrixUInt8("int"sv), wkc::TypeError);
  EXPECT_THROW(config.GetMatrixUInt8("invalid1"sv), wkc::TypeError);
  EXPECT_THROW(config.GetMatrixUInt8("invalid2"sv), wkc::TypeError);
  EXPECT_THROW(config.GetMatrixUInt8("invalid3"sv), wkc::TypeError);

  wkc::Matrix<uint8_t> mat_u8 = config.GetMatrixUInt8("empty"sv);
  EXPECT_EQ(0, mat_u8.size());

  mat_u8 = config.GetMatrixUInt8("lst-int"sv);
  EXPECT_TRUE(mat_u8.IsRowMajor);
  EXPECT_EQ(3, mat_u8.rows());
  EXPECT_EQ(1, mat_u8.cols());
  EXPECT_EQ(1, mat_u8(0, 0));
  EXPECT_EQ(2, mat_u8(1, 0));
  EXPECT_EQ(3, mat_u8(2, 0));

  mat_u8 = config.GetMatrixUInt8("mat-uint8"sv);
  EXPECT_TRUE(mat_u8.IsRowMajor);
  EXPECT_EQ(3, mat_u8.rows());
  EXPECT_EQ(2, mat_u8.cols());
  EXPECT_EQ(0, mat_u8(0, 0));
  EXPECT_EQ(127, mat_u8(0, 1));
  EXPECT_EQ(10, mat_u8(1, 0));
  EXPECT_EQ(100, mat_u8(1, 1));
  EXPECT_EQ(32, mat_u8(2, 0));
  EXPECT_EQ(64, mat_u8(2, 1));

  // mat-int32 contains a negative number
  EXPECT_THROW(config.GetMatrixUInt8("mat-int32"sv), wkc::TypeError);

  // -------------------------------------------------------------
  // ---- 32-bit signed integer
  // Invalid queries
  EXPECT_THROW(config.GetMatrixInt32("no-such-key"sv), wkc::KeyError);
  EXPECT_THROW(config.GetMatrixInt32("int"sv), wkc::TypeError);
  EXPECT_THROW(config.GetMatrixInt32("invalid1"sv), wkc::TypeError);
  EXPECT_THROW(config.GetMatrixInt32("invalid2"sv), wkc::TypeError);
  EXPECT_THROW(config.GetMatrixInt32("invalid3"sv), wkc::TypeError);

  // Retrieve lists as Nx1 matrices:
  wkc::Matrix<int32_t> mat_int32 = config.GetMatrixInt32("empty"sv);
  EXPECT_EQ(0, mat_int32.size());

  mat_int32 = config.GetMatrixInt32("lst-int"sv);
  EXPECT_TRUE(mat_int32.IsRowMajor);
  EXPECT_EQ(3, mat_int32.rows());
  EXPECT_EQ(1, mat_int32.cols());
  EXPECT_EQ(1, mat_int32(0, 0));
  EXPECT_EQ(2, mat_int32(1, 0));
  EXPECT_EQ(3, mat_int32(2, 0));

  mat_int32 = config.GetMatrixInt32("lst-convertible"sv);
  EXPECT_TRUE(mat_int32.IsRowMajor);
  EXPECT_EQ(5, mat_int32.rows());
  EXPECT_EQ(1, mat_int32.cols());
  EXPECT_EQ(42, mat_int32(0, 0));
  EXPECT_EQ(20, mat_int32(1, 0));
  EXPECT_EQ(-3, mat_int32(2, 0));
  EXPECT_EQ(0, mat_int32(3, 0));
  EXPECT_EQ(123, mat_int32(4, 0));

  // lst-flt contains values that can't be represented by an int:
  EXPECT_THROW(config.GetMatrixInt32("lst-flt"sv), wkc::TypeError);

  mat_int32 = config.GetMatrixInt32("mat-int32"sv);
  EXPECT_TRUE(mat_int32.IsRowMajor);
  EXPECT_EQ(2, mat_int32.rows());
  EXPECT_EQ(3, mat_int32.cols());

  // mat-int64 contains values that over- & underflow 32-bit integer
  EXPECT_THROW(config.GetMatrixInt32("mat-int64"sv), wkc::TypeError);

  // -------------------------------------------------------------
  // ---- 64-bit signed integer
  // Invalid queries
  EXPECT_THROW(config.GetMatrixInt64("no-such-key"sv), wkc::KeyError);
  EXPECT_THROW(config.GetMatrixInt64("int"sv), wkc::TypeError);
  EXPECT_THROW(config.GetMatrixInt64("invalid1"sv), wkc::TypeError);
  EXPECT_THROW(config.GetMatrixInt64("invalid2"sv), wkc::TypeError);
  EXPECT_THROW(config.GetMatrixInt64("invalid3"sv), wkc::TypeError);

  // Retrieve lists as Nx1 matrices:
  wkc::Matrix<int64_t> mat_int64 = config.GetMatrixInt64("empty"sv);
  EXPECT_EQ(0, mat_int64.size());

  mat_int64 = config.GetMatrixInt64("lst-int"sv);
  EXPECT_TRUE(mat_int64.IsRowMajor);
  EXPECT_EQ(3, mat_int64.rows());
  EXPECT_EQ(1, mat_int64.cols());

  mat_int64 = config.GetMatrixInt64("lst-convertible"sv);
  EXPECT_TRUE(mat_int64.IsRowMajor);
  EXPECT_EQ(5, mat_int64.rows());
  EXPECT_EQ(1, mat_int64.cols());
  EXPECT_EQ(42, mat_int64(0, 0));
  EXPECT_EQ(20, mat_int64(1, 0));
  EXPECT_EQ(-3, mat_int64(2, 0));
  EXPECT_EQ(0, mat_int64(3, 0));
  EXPECT_EQ(123, mat_int64(4, 0));

  // lst-flt contains values that can't be represented by an int:
  EXPECT_THROW(config.GetMatrixInt64("lst-flt"sv), wkc::TypeError);

  mat_int64 = config.GetMatrixInt64("mat-int32"sv);
  EXPECT_TRUE(mat_int64.IsRowMajor);
  EXPECT_EQ(2, mat_int64.rows());
  EXPECT_EQ(3, mat_int64.cols());

  mat_int64 = config.GetMatrixInt64("mat-int64"sv);
  EXPECT_TRUE(mat_int64.IsRowMajor);
  EXPECT_EQ(2, mat_int64.rows());
  EXPECT_EQ(2, mat_int64.cols());
  EXPECT_EQ(2147483648, mat_int64(0, 0));
  EXPECT_EQ(-2147483649, mat_int64(0, 1));
  EXPECT_EQ(1, mat_int64(1, 0));
  EXPECT_EQ(2, mat_int64(1, 1));

  // -------------------------------------------------------------
  // ---- Double
  // Invalid queries
  EXPECT_THROW(config.GetMatrixDouble("no-such-key"sv), wkc::KeyError);
  EXPECT_THROW(config.GetMatrixDouble("int"sv), wkc::TypeError);
  EXPECT_THROW(config.GetMatrixDouble("invalid1"sv), wkc::TypeError);
  EXPECT_THROW(config.GetMatrixDouble("invalid2"sv), wkc::TypeError);
  EXPECT_THROW(config.GetMatrixDouble("invalid3"sv), wkc::TypeError);

  // Retrieve lists as Nx1 matrices:
  wkc::Matrix<double> mat_dbl = config.GetMatrixDouble("empty"sv);
  EXPECT_EQ(0, mat_dbl.size());

  mat_dbl = config.GetMatrixDouble("lst-int"sv);
  EXPECT_TRUE(mat_dbl.IsRowMajor);
  EXPECT_EQ(3, mat_dbl.rows());
  EXPECT_EQ(1, mat_dbl.cols());
  EXPECT_DOUBLE_EQ(1.0, mat_dbl(0, 0));
  EXPECT_DOUBLE_EQ(2.0, mat_dbl(1, 0));
  EXPECT_DOUBLE_EQ(3.0, mat_dbl(2, 0));

  mat_dbl = config.GetMatrixDouble("lst-convertible"sv);
  EXPECT_TRUE(mat_dbl.IsRowMajor);
  EXPECT_EQ(5, mat_dbl.rows());
  EXPECT_EQ(1, mat_dbl.cols());
  EXPECT_DOUBLE_EQ(42.0, mat_dbl(0, 0));
  EXPECT_DOUBLE_EQ(20.0, mat_dbl(1, 0));
  EXPECT_DOUBLE_EQ(-3.0, mat_dbl(2, 0));
  EXPECT_DOUBLE_EQ(0.0, mat_dbl(3, 0));
  EXPECT_DOUBLE_EQ(123.0, mat_dbl(4, 0));

  mat_dbl = config.GetMatrixDouble("lst-flt"sv);
  EXPECT_TRUE(mat_dbl.IsRowMajor);
  EXPECT_EQ(3, mat_dbl.rows());
  EXPECT_EQ(1, mat_dbl.cols());
  EXPECT_DOUBLE_EQ(0.0, mat_dbl(0, 0));
  EXPECT_DOUBLE_EQ(-3.5, mat_dbl(1, 0));
  EXPECT_DOUBLE_EQ(1e6, mat_dbl(2, 0));
}

TEST(ConfigCompoundTest, SetMatrices) {
  auto config = wkc::LoadTOMLString(R"toml(
    int = 3

    lst = [1, 2, 3, 'four']
    )toml");

  // -------------------------------------------------------------
  // ---- Set a parameter from a 2d integer matrix
  Eigen::Matrix2i m2i;
  m2i << 1, 2, 3, 4;

  EXPECT_NO_THROW(config.SetMatrix("m2i"sv, m2i));
  EXPECT_EQ(2, config.Size("m2i"sv));
  EXPECT_EQ(2, config.Size("m2i[0]"sv));
  EXPECT_EQ(2, config.Size("m2i[1]"sv));

  EXPECT_EQ(1, config.GetInteger32("m2i[0][0]"sv));
  EXPECT_EQ(2, config.GetInteger32("m2i[0][1]"sv));
  EXPECT_EQ(3, config.GetInteger32("m2i[1][0]"sv));
  EXPECT_EQ(4, config.GetInteger32("m2i[1][1]"sv));

  EXPECT_FALSE(m2i.IsRowMajor);  // Eigen is column-major by default
  auto m2i_retrieved = config.GetMatrixInt32("m2i"sv);
  EXPECT_TRUE(m2i_retrieved.IsRowMajor);  // But wzk returns row-major
  EXPECT_EQ(m2i, m2i_retrieved);

  // Changing the type of an existing parameter is not allowed
  EXPECT_THROW(config.SetMatrix("int"sv, m2i), wkc::TypeError);

  // But we can replace an existing list by this matrix, as matrices
  // are stored as lists (nested if needed):
  EXPECT_NO_THROW(config.SetMatrix("lst"sv, m2i));
  EXPECT_EQ(2, config.Size("lst"sv));
  EXPECT_EQ(2, config.Size("lst[0]"sv));
  EXPECT_EQ(2, config.Size("lst[1]"sv));

  // -------------------------------------------------------------
  // ---- Set a parameter from a 2d double matrix
  Eigen::Matrix3d m3d;
  m3d << -1, -2, -3, 1, 2, 3, 0.1, 0.2, 0.3;

  EXPECT_NO_THROW(config.SetMatrix("m3d"sv, m3d));
  EXPECT_EQ(3, config.Size("m3d"sv));
  EXPECT_EQ(3, config.Size("m3d[0]"sv));
  EXPECT_EQ(3, config.Size("m3d[1]"sv));
  EXPECT_EQ(3, config.Size("m3d[2]"sv));
  EXPECT_DOUBLE_EQ(m3d(0, 0), config.GetDouble("m3d[0][0]"sv));
  EXPECT_DOUBLE_EQ(m3d(0, 1), config.GetDouble("m3d[0][1]"sv));
  EXPECT_DOUBLE_EQ(m3d(0, 2), config.GetDouble("m3d[0][2]"sv));
  EXPECT_DOUBLE_EQ(m3d(1, 0), config.GetDouble("m3d[1][0]"sv));
  EXPECT_DOUBLE_EQ(m3d(1, 1), config.GetDouble("m3d[1][1]"sv));
  EXPECT_DOUBLE_EQ(m3d(1, 2), config.GetDouble("m3d[1][2]"sv));
  EXPECT_DOUBLE_EQ(m3d(2, 0), config.GetDouble("m3d[2][0]"sv));
  EXPECT_DOUBLE_EQ(m3d(2, 1), config.GetDouble("m3d[2][1]"sv));
  EXPECT_DOUBLE_EQ(m3d(2, 2), config.GetDouble("m3d[2][2]"sv));

  // Changing the type of an existing parameter is not allowed
  EXPECT_THROW(config.SetMatrix("int"sv, m2i), wkc::TypeError);

  // But we can replace an existing list (see m2i test above):
  EXPECT_NO_THROW(config.SetMatrix("lst"sv, m3d));
  EXPECT_EQ(3, config.Size("lst"sv));
  EXPECT_EQ(3, config.Size("lst[0]"sv));
  EXPECT_EQ(3, config.Size("lst[1]"sv));
  EXPECT_EQ(3, config.Size("lst[2]"sv));

  // -------------------------------------------------------------
  // ---- Set a parameter from a column vector
  // Eigen::Vector is a Nx1 matrix
  Eigen::Vector2f v2f;
  v2f << 0.5f, -2.0f;
  EXPECT_EQ(2, v2f.rows());
  EXPECT_EQ(1, v2f.cols());
  EXPECT_NO_THROW(config.SetMatrix("v2f"sv, v2f));
  // Check that the parameter is a 2-element list
  EXPECT_EQ(2, config.Size("v2f"sv));
  EXPECT_DOUBLE_EQ(0.5, config.GetDouble("v2f[0]"sv));
  EXPECT_DOUBLE_EQ(-2.0, config.GetDouble("v2f[1]"sv));
  // Retrieve as 2x1 matrix
  auto m2d = config.GetMatrixDouble("v2f"sv);
  EXPECT_EQ(2, m2d.rows());
  EXPECT_EQ(1, m2d.cols());
  EXPECT_DOUBLE_EQ(0.5, m2d(0, 0));
  EXPECT_DOUBLE_EQ(-2.0, m2d(1, 0));

  // -------------------------------------------------------------
  // ---- Set a parameter from a row vector
  Eigen::MatrixXi mxi(1, 3);
  mxi << -42, 0, 420;
  EXPECT_EQ(1, mxi.rows());
  EXPECT_EQ(3, mxi.cols());
  EXPECT_NO_THROW(config.SetMatrix("mxi"sv, mxi));
  EXPECT_EQ(3, config.Size("mxi"sv));
  EXPECT_EQ(-42, config.GetInteger32("mxi[0]"sv));
  EXPECT_EQ(0, config.GetInteger32("mxi[1]"sv));
  EXPECT_EQ(420, config.GetInteger32("mxi[2]"sv));

  // Querying this row vector will return a column vector:
  mxi = config.GetMatrixInt32("mxi"sv);
  EXPECT_EQ(3, mxi.rows());
  EXPECT_EQ(1, mxi.cols());
  EXPECT_EQ(-42, mxi(0, 0));
  EXPECT_EQ(0, mxi(1, 0));
  EXPECT_EQ(420, mxi(2, 0));
}

// NOLINTEND
