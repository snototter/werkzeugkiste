#include <exception>
#include <initializer_list>
#include <cmath>
#include <vector>
#include <list>

#include <werkzeugkiste/geometry/utils.h>
#include <werkzeugkiste/geometry/vector.h>

#include "../test_utils.h"

namespace wkg = werkzeugkiste::geometry;

// NOLINTBEGIN

//TODO FIXME - make proper tests & change all assertions (1st param is expected; 2nd is actual value)


template<typename T>
void TestVec2dSpecialties(wkg::Vec<T, 2> &vec) {
  auto perpendicular = vec.PerpendicularClockwise();
  EXPECT_TRUE(wkg::IsEpsEqual(perpendicular.X(), vec.Y()));
  EXPECT_TRUE(wkg::IsEpsEqual(perpendicular.Y(), -vec.X()));

  perpendicular = vec.PerpendicularCounterClockwise();
  EXPECT_TRUE(wkg::IsEpsEqual(perpendicular.X(), -vec.Y()));
  EXPECT_TRUE(wkg::IsEpsEqual(perpendicular.Y(), vec.X()));
}


template<typename T, std::size_t Dim>
void Test2dSpecials(wkg::Vec<T, Dim> &vec) {
  if constexpr (Dim == 2) {
    TestVec2dSpecialties<T>(vec);
  }
}



template<typename _Tp, std::size_t Dim>
void VectorTestHelper(wkg::Vec<_Tp, Dim> &vec) {
  EXPECT_GE(Dim, 2);

  constexpr int DIM_INT = static_cast<int>(Dim);

  // Test negative indexing
  for (int i = 0; i < DIM_INT; ++i) {
    EXPECT_EQ(vec.val[DIM_INT - i - 1], vec[-(i+1)]);
  }

  EXPECT_EQ(-vec, static_cast<_Tp>(-1) * vec); //TODO

  Test2dSpecials(vec);

  // Check usage as 2d size representation
  if constexpr (Dim == 2) {
    EXPECT_DOUBLE_EQ(vec.X(), vec.Width());
    EXPECT_DOUBLE_EQ(vec.Y(), vec.Height());

    vec.SetWidth(2 * vec.X());
    vec.SetHeight(3 * vec.Y());

    EXPECT_DOUBLE_EQ(vec.X(), vec.Width());
    EXPECT_DOUBLE_EQ(vec.Y(), vec.Height());

    // Restore original input vector
    vec.val[0] /= 2;
    vec.val[1] /= 3;

//    EXPECT_THROW(vec.Z(), std::logic_error);
//    EXPECT_THROW(vec.W(), std::logic_error);
  } else {
    //FIXME clean up tests
    // Other dimensional vectors should not support
    // this functionality
//    EXPECT_THROW(vec.Width(), std::logic_error);
//    EXPECT_THROW(vec.Height(), std::logic_error);

//    if constexpr (Dim == 3) {
//      EXPECT_THROW(vec.W(), std::logic_error);
//    }
  }

  // Test out-of-bounds access
  EXPECT_THROW(vec[DIM_INT], std::out_of_range);
  EXPECT_THROW(vec[-DIM_INT - 1], std::out_of_range);

  // Create a copy
  auto copy = wkg::Vec<_Tp, Dim>(vec);
  EXPECT_EQ(vec, copy);

  // Basic arithmetics
  auto vec_twice = vec + vec;
  EXPECT_NE(vec, vec_twice);
  EXPECT_EQ(vec * static_cast<_Tp>(2), vec_twice);
  EXPECT_EQ(static_cast<_Tp>(2) * vec, vec_twice);

  vec *= 2;
  EXPECT_EQ(vec, vec_twice);

  std::vector<wkg::Vec<_Tp, Dim>> poly{vec, vec_twice};
  double poly_len = wkg::LengthPolygon(poly);
  EXPECT_DOUBLE_EQ(poly_len, vec.DistanceEuclidean(vec_twice));

  poly.push_back(vec);
  poly_len = wkg::LengthPolygon(poly);
  EXPECT_DOUBLE_EQ(poly_len, 2 * vec.DistanceEuclidean(vec_twice));

  if constexpr (std::is_integral<_Tp>::value) {
    auto dbl = vec.ToDouble();
    dbl /= 2;
    for (std::size_t idx = 0; idx < Dim; ++idx) {
      vec[idx] = static_cast<_Tp>(dbl[idx]);
    }
  } else {
    vec /= 2;
    EXPECT_EQ(vec_twice / static_cast<_Tp>(2), vec);
  }

  // Assignment operator
  vec_twice = vec;
  EXPECT_EQ(vec_twice, vec);
  EXPECT_TRUE(vec_twice == copy);

  const auto vec_3x = vec + vec_twice + copy;
  EXPECT_EQ(static_cast<_Tp>(3) * vec, vec_3x);

  poly.clear();
  poly_len = wkg::LengthPolygon(poly);
  EXPECT_DOUBLE_EQ(poly_len, 0.0);
  poly.push_back(vec);
  poly_len = wkg::LengthPolygon(poly);
  EXPECT_DOUBLE_EQ(poly_len, 0.0);
  poly.push_back(vec_3x);
  poly_len = wkg::LengthPolygon(poly);
  EXPECT_DOUBLE_EQ(poly_len, vec.DistanceEuclidean(vec_3x));
  poly.push_back(vec);
  poly_len = wkg::LengthPolygon(poly);
  EXPECT_DOUBLE_EQ(poly_len, 2 * vec.DistanceEuclidean(vec_3x));
  poly.push_back(vec_3x);
  poly_len = wkg::LengthPolygon(poly);
  EXPECT_DOUBLE_EQ(poly_len, 3 * vec.DistanceEuclidean(vec_3x));



  // Add 0 vector
  wkg::Vec<_Tp, Dim> zero;
  vec_twice = vec + zero;
  EXPECT_EQ(vec_twice, vec);

  EXPECT_EQ(vec_3x, static_cast<_Tp>(3) * vec);

  // Add/subtract scalars
  const auto add1 = vec.ToDouble() + 17.0;
  const auto sub1 = vec.ToDouble() - 42.0;
  for (std::size_t i = 0; i < Dim; ++i) {
    EXPECT_DOUBLE_EQ(add1[i], vec[i] + 17);
    EXPECT_DOUBLE_EQ(sub1[i], vec[i] - 42);
  }

  // Test negation (unary operator-)
  auto negated = -vec;

  for (std::size_t i = 0; i < Dim; ++i) {
    EXPECT_DOUBLE_EQ(copy[i], vec[i]);
    EXPECT_DOUBLE_EQ(negated[i], -vec[i]);
  }

  // Distance/Length & dot product:
  auto dot1 = vec.Dot(vec);
  auto dot3 = vec.Dot(vec_3x);
  EXPECT_DOUBLE_EQ(3.0 * dot1, 1.0 * dot3);

  auto len = vec.Length();
  EXPECT_DOUBLE_EQ(std::sqrt(1.0 * dot1), 1.0 * len);

  EXPECT_DOUBLE_EQ(1.0 * dot1, vec.LengthSquared());

  auto dist = vec.DistanceEuclidean(zero);
  EXPECT_DOUBLE_EQ(dist, len);

  auto vec_4x = static_cast<_Tp>(4) * vec;
  dist = vec.DistanceEuclidean(vec_4x);
  EXPECT_DOUBLE_EQ(dist, len * 3);


  // Cross product
  if constexpr (Dim == 3) {
    wkg::Vec<_Tp, Dim> other {-3, 15, 21};

    auto cross = vec.Cross(other);
    wkg::Vec<_Tp, Dim> expected{
          vec.Y() * other.Z() - vec.Z() * other.Y(),
          vec.Z() * other.X() - vec.X() * other.Z(),
          vec.X() * other.Y() - vec.Y() * other.X()};
    EXPECT_DOUBLE_EQ(cross.X(), expected.X());
    EXPECT_DOUBLE_EQ(cross.Y(), expected.Y());
    EXPECT_DOUBLE_EQ(cross.Z(), expected.Z());
  }
}


TEST(VectorTest, All) {
  EXPECT_THROW((wkg::Vec2d{1, 2, 3}), std::invalid_argument);
  EXPECT_THROW((wkg::Vec3d{2, 17}), std::invalid_argument);
  EXPECT_THROW((wkg::Vec4d{2, 17}), std::invalid_argument);
  EXPECT_THROW((wkg::Vec4d{2, 17, 3}), std::invalid_argument);

  wkg::Vec2d zero2d;

  wkg::Vec2d v2d_a{23, 17};
  VectorTestHelper(v2d_a);

  auto unit2d = v2d_a.UnitVector();
  EXPECT_DOUBLE_EQ(1.0, unit2d.Length());
  EXPECT_TRUE(std::fabs(unit2d.X() - 23 / 28.600699292) < 1e-6);
  EXPECT_TRUE(std::fabs(unit2d.Y() - 17 / 28.600699292) < 1e-6);
  EXPECT_EQ(v2d_a.DirectionVector(zero2d), -v2d_a);
  EXPECT_EQ(v2d_a.DirectionVector(v2d_a), zero2d);

  wkg::Vec2d v2d_b{0.01, -9.001};
  EXPECT_DOUBLE_EQ(v2d_b.MaxValue(), 0.01);
  EXPECT_DOUBLE_EQ(v2d_b.MinValue(), -9.001);
  EXPECT_EQ(v2d_b.MaxIndex(), 0);
  EXPECT_EQ(v2d_b.MinIndex(), 1);
  VectorTestHelper(v2d_b);

  wkg::Vec2d v2d_c{-735.008, -0.99};
  EXPECT_DOUBLE_EQ(v2d_c.MaxValue(), -0.99);
  EXPECT_DOUBLE_EQ(v2d_c.MinValue(), -735.008);
  EXPECT_EQ(v2d_c.MaxIndex(), 1);
  EXPECT_EQ(v2d_c.MinIndex(), 0);
  VectorTestHelper(v2d_c);


  wkg::Vec3d v3d_a{1, 2, 3};
  EXPECT_DOUBLE_EQ(v3d_a.MaxValue(), 3);
  EXPECT_DOUBLE_EQ(v3d_a.MinValue(), 1);
  EXPECT_EQ(v3d_a.MaxIndex(), 2);
  EXPECT_EQ(v3d_a.MinIndex(), 0);
  VectorTestHelper(v3d_a);

  wkg::Vec3d v3d_b{-0.1, 99, -15.3};
  VectorTestHelper(v3d_b);

  wkg::Vec3d v3d_c{12.3, -0.42, 77.7};
  VectorTestHelper(v3d_c);

  wkg::Vec2i zero2i;
  EXPECT_DOUBLE_EQ(zero2i.Length(), 0);
  EXPECT_EQ(zero2i.UnitVector(), wkg::Vec2d());

  auto v2d_casted = v2d_c.ToInteger();
  EXPECT_EQ(typeid(zero2i), typeid(v2d_casted));
  EXPECT_EQ(static_cast<int>(v2d_c.X()), v2d_casted.X());
  EXPECT_EQ(static_cast<int>(v2d_c.Y()), v2d_casted.Y());
  EXPECT_EQ(-735, v2d_casted.X());
  EXPECT_EQ(0, v2d_casted.Y());

  wkg::Vec2i v2i{9, -2};
  VectorTestHelper(v2i);

  auto unit2i = v2i.UnitVector();
  EXPECT_DOUBLE_EQ(unit2i.Length(), 1.0);
  EXPECT_TRUE(std::fabs(unit2i.X() - 9.0 / 9.219544457) < 1e-6);
  EXPECT_TRUE(std::fabs(unit2i.Y() + 2.0 / 9.219544457) < 1e-6);
  EXPECT_EQ(v2i.DirectionVector(zero2i), -v2i);
  EXPECT_EQ(v2i.DirectionVector(v2i), zero2i);
}


TEST(VectorTest, MinMaxCoordinates) {
  std::vector<wkg::Vec2i> data2i {
    {1, 0}, {10, -3}, {-15, 1}, {17, 42}};
  wkg::Vec2i min, max;
  wkg::MinMaxCoordinates(data2i, min, max);

  EXPECT_EQ(min.X(), -15);
  EXPECT_EQ(min.Y(), -3);

  EXPECT_EQ(max.X(), 17);
  EXPECT_EQ(max.Y(), 42);


  std::list<wkg::Vec3d> data3d {
    {10, 0, 1}, {100, -3, 17}, {1, 0, -1}};
  wkg::Vec3d min3, max3;
  wkg::MinMaxCoordinates(data3d, min3, max3);

  EXPECT_DOUBLE_EQ(min3.X(), 1.0);
  EXPECT_DOUBLE_EQ(min3.Y(), -3.0);
  EXPECT_DOUBLE_EQ(min3.Z(), -1.0);

  EXPECT_DOUBLE_EQ(max3.X(), 100.0);
  EXPECT_DOUBLE_EQ(max3.Y(), 0.0);
  EXPECT_DOUBLE_EQ(max3.Z(), 17.0);
}

// NOLINTEND
