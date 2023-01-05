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

template<typename _Tp, int dim>
void VectorTestHelper(wkg::Vec<_Tp, dim> &vec) {
  EXPECT_GE(dim, 2);

  // Test negative indexing
  for (int i = 0; i < dim; ++i) {
    EXPECT_EQ(vec.val[dim - i - 1], vec[-(i+1)]);
  }

  // Check usage as 2d size representation
  if (dim == 2) {
    EXPECT_DOUBLE_EQ(vec.X(), vec.Width());
    EXPECT_DOUBLE_EQ(vec.Y(), vec.Height());

    vec.SetWidth(2 * vec.X());
    vec.SetHeight(3 * vec.Y());

    EXPECT_DOUBLE_EQ(vec.X(), vec.Width());
    EXPECT_DOUBLE_EQ(vec.Y(), vec.Height());

    // Restore original input vector
    vec.val[0] /= 2;
    vec.val[1] /= 3;

    EXPECT_THROW(vec.Z(), std::logic_error);
    EXPECT_THROW(vec.W(), std::logic_error);
  } else {
    // Other dimensional vectors should not support
    // this functionality
    EXPECT_THROW(vec.Width(), std::logic_error);
    EXPECT_THROW(vec.Height(), std::logic_error);

    if (dim == 3) {
      EXPECT_THROW(vec.W(), std::logic_error);
    }
  }

  // Test out-of-bounds
  EXPECT_THROW(vec[dim], std::out_of_range);
  EXPECT_THROW(vec[-dim-1], std::out_of_range);

  // Create a copy
  auto copy = wkg::Vec<_Tp, dim>(vec);
  EXPECT_EQ(vec, copy);

  // Basic arithmetics
  auto vec2 = vec + vec;
  EXPECT_NE(vec, vec2);
  EXPECT_EQ(2 * vec, vec2);

  vec *= 2;
  EXPECT_EQ(vec, vec2);

  std::vector<wkg::Vec<_Tp, dim>> poly{vec, vec2};
  double poly_len = wkg::LengthPolygon(poly);
  EXPECT_DOUBLE_EQ(poly_len, vec.Distance(vec2));

  poly.push_back(vec);
  poly_len = wkg::LengthPolygon(poly);
  EXPECT_DOUBLE_EQ(poly_len, 2 * vec.Distance(vec2));

  vec /= 2;
  EXPECT_EQ(vec2 / 2, vec);

  vec2 = vec;
  EXPECT_EQ(vec2, vec);
  EXPECT_TRUE(vec2 == copy);

  auto vec3 = vec + vec2 + copy;
  EXPECT_EQ(3 * vec, vec3);

  poly.clear();
  poly_len = wkg::LengthPolygon(poly);
  EXPECT_DOUBLE_EQ(poly_len, 0.0);
  poly.push_back(vec);
  poly_len = wkg::LengthPolygon(poly);
  EXPECT_DOUBLE_EQ(poly_len, 0.0);
  poly.push_back(vec3);
  poly_len = wkg::LengthPolygon(poly);
  EXPECT_DOUBLE_EQ(poly_len, vec.Distance(vec3));
  poly.push_back(vec);
  poly_len = wkg::LengthPolygon(poly);
  EXPECT_DOUBLE_EQ(poly_len, 2 * vec.Distance(vec3));
  poly.push_back(vec3);
  poly_len = wkg::LengthPolygon(poly);
  EXPECT_DOUBLE_EQ(poly_len, 3 * vec.Distance(vec3));



  // Add 0 vector
  wkg::Vec<_Tp, dim> zero;
  vec2 = vec + zero;
  EXPECT_EQ(vec2, vec);

  // Add/subtract scalars
  auto add1 = vec + 17.0;
  auto sub1 = vec - 42.0;
  for (int i = 0; i < dim; ++i) {
    EXPECT_DOUBLE_EQ(add1[i], vec[i] + 17);
    EXPECT_DOUBLE_EQ(sub1[i], vec[i] - 42);
  }

  // Test negation (unary operator-)
  auto negated = -vec;

  for (int i = 0; i < dim; ++i) {
    EXPECT_DOUBLE_EQ(copy[i], vec[i]);
    EXPECT_DOUBLE_EQ(negated[i], -vec[i]);
  }

  // Distance/Length & dot product:
  auto dot1 = vec.Dot(vec);
  auto dot3 = vec.Dot(vec3);
  EXPECT_DOUBLE_EQ(3.0 * dot1, 1.0 * dot3);

  auto len = vec.Length();
  EXPECT_DOUBLE_EQ(std::sqrt(1.0 * dot1), 1.0 * len);

  EXPECT_DOUBLE_EQ(1.0 * dot1, vec.LengthSquared());

  auto dist = vec.Distance(zero);
  EXPECT_DOUBLE_EQ(dist, len);

  vec2 = 4.0 * vec;
  dist = vec.Distance(vec2);
  EXPECT_DOUBLE_EQ(dist, len * 3);


  // Cross product (only for 3d)
  wkg::Vec<_Tp, dim> other;
  for (int i = 0; i < dim; ++i)
    other[i] = static_cast<_Tp>(i);

  if (dim != 3) {
    EXPECT_THROW(vec.Cross(other), std::logic_error);
  } else {
    auto cross = vec.Cross(other);
    wkg::Vec<_Tp, dim> expected{
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
  EXPECT_DOUBLE_EQ(unit2d.Length(), 1.0);
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
