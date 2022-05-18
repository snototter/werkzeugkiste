#include <exception>
#include <initializer_list>
#include <cmath>

#include <gtest/gtest.h>

#include <werkzeugkiste/geometry/utils.h>
#include <werkzeugkiste/geometry/vector.h>

namespace wkg = werkzeugkiste::geometry;

template<typename _Tp, int dim>
void VectorTestHelper(wkg::Vec<_Tp, dim> &vec) {
  EXPECT_GE(dim, 2);

  // Test negative indexing
  for (int i = 0; i < dim; ++i) {
    EXPECT_EQ(vec.val[dim - i - 1], vec[-(i+1)]);
  }

  // Check usage as 2d size representation
  if (dim == 2) {
    EXPECT_DOUBLE_EQ(vec.x(), vec.width());
    EXPECT_DOUBLE_EQ(vec.y(), vec.height());

    vec.SetWidth(2 * vec.x());
    vec.SetHeight(3 * vec.y());

    EXPECT_DOUBLE_EQ(vec.x(), vec.width());
    EXPECT_DOUBLE_EQ(vec.y(), vec.height());

    // Restore original input vector
    vec.val[0] /= 2;
    vec.val[1] /= 3;

    EXPECT_THROW(vec.z(), std::logic_error);
    EXPECT_THROW(vec.w(), std::logic_error);
  } else {
    // Other dimensional vectors should not support
    // this functionality
    EXPECT_THROW(vec.width(), std::logic_error);
    EXPECT_THROW(vec.height(), std::logic_error);

    if (dim == 3) {
      EXPECT_THROW(vec.w(), std::logic_error);
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

  vec /= 2;
  EXPECT_EQ(vec2 / 2, vec);

  vec2 = vec;
  EXPECT_EQ(vec2, vec);
  EXPECT_TRUE(vec2 == copy);

  auto vec3 = vec + vec2 + copy;
  EXPECT_EQ(3 * vec, vec3);

  // Add 0 vector
  wkg::Vec<_Tp, dim> zero;
  vec2 = vec + zero;
  EXPECT_EQ(vec2, vec);

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
    wkg::Vec<_Tp, dim> expected(
          vec.y() * other.z() - vec.z() * other.y(),
          vec.z() * other.x() - vec.x() * other.z(),
          vec.x() * other.y() - vec.y() * other.x());
    EXPECT_DOUBLE_EQ(cross.x(), expected.x());
    EXPECT_DOUBLE_EQ(cross.y(), expected.y());
    EXPECT_DOUBLE_EQ(cross.z(), expected.z());
  }
}


TEST(VectorTest, All) {
  EXPECT_THROW(wkg::Vec2d(1, 2, 3), std::invalid_argument);
  EXPECT_THROW(wkg::Vec3d(2, 17), std::invalid_argument);
  EXPECT_THROW(wkg::Vec4d(2, 17), std::invalid_argument);
  EXPECT_THROW(wkg::Vec4d(2, 17, 3), std::invalid_argument);

  wkg::Vec2d zero2d;

  wkg::Vec2d v2d_a(23, 17);
  VectorTestHelper(v2d_a);

  auto unit2d = v2d_a.UnitVector();
  EXPECT_DOUBLE_EQ(unit2d.Length(), 1.0);
  EXPECT_TRUE(std::fabs(unit2d.x() - 23 / 28.600699292) < 1e-6);
  EXPECT_TRUE(std::fabs(unit2d.y() - 17 / 28.600699292) < 1e-6);
  EXPECT_EQ(v2d_a.DirectionVector(zero2d), -v2d_a);
  EXPECT_EQ(v2d_a.DirectionVector(v2d_a), zero2d);

  wkg::Vec2d v2d_b(0.01, -9.001);
  EXPECT_DOUBLE_EQ(v2d_b.MaxValue(), 0.01);
  EXPECT_DOUBLE_EQ(v2d_b.MinValue(), -9.001);
  EXPECT_EQ(v2d_b.MaxIndex(), 0);
  EXPECT_EQ(v2d_b.MinIndex(), 1);
  VectorTestHelper(v2d_b);

  wkg::Vec2d v2d_c(-735.008, -0.99);
  EXPECT_DOUBLE_EQ(v2d_c.MaxValue(), -0.99);
  EXPECT_DOUBLE_EQ(v2d_c.MinValue(), -735.008);
  EXPECT_EQ(v2d_c.MaxIndex(), 1);
  EXPECT_EQ(v2d_c.MinIndex(), 0);
  VectorTestHelper(v2d_c);


  wkg::Vec3d v3d_a(1, 2, 3);
  EXPECT_DOUBLE_EQ(v3d_a.MaxValue(), 3);
  EXPECT_DOUBLE_EQ(v3d_a.MinValue(), 1);
  EXPECT_EQ(v3d_a.MaxIndex(), 2);
  EXPECT_EQ(v3d_a.MinIndex(), 0);
  VectorTestHelper(v3d_a);

  wkg::Vec3d v3d_b(-0.1, 99, -15.3);
  VectorTestHelper(v3d_b);

  wkg::Vec3d v3d_c(12.3, -0.42, 77.7);
  VectorTestHelper(v3d_c);


  wkg::Vec2i zero2i;
  EXPECT_DOUBLE_EQ(zero2i.Length(), 0);
  EXPECT_EQ(zero2i.UnitVector(), wkg::Vec2d());

  wkg::Vec2i v2i(9, -2);
  VectorTestHelper(v2i);

  auto unit2i = v2i.UnitVector();
  EXPECT_DOUBLE_EQ(unit2i.Length(), 1.0);
  EXPECT_TRUE(std::fabs(unit2i.x() - 9.0 / 9.219544457) < 1e-6);
  EXPECT_TRUE(std::fabs(unit2i.y() + 2.0 / 9.219544457) < 1e-6);
  EXPECT_EQ(v2i.DirectionVector(zero2i), -v2i);
  EXPECT_EQ(v2i.DirectionVector(v2i), zero2i);
}

//TODO test vector math! projections, angles, etc.
