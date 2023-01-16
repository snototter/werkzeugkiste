#include <exception>
#include <initializer_list>
#include <cmath>
#include <vector>
#include <list>
#include <sstream>
#include <iomanip>

#include <werkzeugkiste/geometry/utils.h>
#include <werkzeugkiste/geometry/vector.h>

#include "../test_utils.h"

namespace wkg = werkzeugkiste::geometry;

// NOLINTBEGIN


//TODO more articles:
// https://codingnest.com/the-little-things-comparing-floating-point-numbers/
// catch2: https://github.com/catchorg/Catch2/blob/9ac9fb164e5b20ad9e2f59556b75b9e6f1600f68/docs/assertions.md#floating-point-comparisons
// precision calculator: https://johannesugb.github.io/cpu-programming/tools/floating-point-epsilon-calculator/
//py.math.isclose https://github.com/PythonCHB/close_pep/blob/master/is_close.py


/// Equality check helper which adds an error message at which dimension
/// the vector differs.
template <typename Tp, std::size_t Dim>
::testing::AssertionResult CheckVectorEqual(
    const wkg::Vec<Tp, Dim> &expected,
    const wkg::Vec<Tp, Dim> &value) {
  if (value.EpsEquals(expected)) {
    return ::testing::AssertionSuccess();
  }

  std::ostringstream msg;
  msg << value.ToString() << " differs at:" << std::setprecision(20);
  for (std::size_t idx = 0; idx < Dim; ++idx) {
//    if constexpr (std::is_same<Tp, double>::value) {
//      EXPECT_DOUBLE_EQ(expected[idx], value[idx]);
//    }
    if (!wkg::IsEpsEqual(expected[idx], value[idx])) {
      msg << " [" << idx << ": " << expected[idx]
          << " vs " << value[idx] << "]";
    }
  }

  float a = 67329.234;
  float b = 67329.242; // 1 ulp example by https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
  double d1 = 0.2;
  double d2 = 1 / std::sqrt(5) / std::sqrt(5);
  double dbl01a = 0.010000000000000000208;
  double dbl01b = 0.0099999999999997868372;
  msg << "TODO true/true: " << wkg::IsEpsEqual(dbl01a, dbl01b)
      << " ? " << wkg::IsEpsEqual(a, b)
      << "\n  machine epsilon: " << std::numeric_limits<double>::epsilon()
      << "\n  double::min:     " << std::numeric_limits<double>::min();

  msg << "\nFPCLASSify: norm " << FP_NORMAL << ", subnorm " << FP_SUBNORMAL
      << ", d1: " << std::fpclassify(d1)<< ", d2: " << std::fpclassify(d2)
      << ", a: " << std::fpclassify(a)<< ", b: " << std::fpclassify(b)
      << ", 0.1: " << std::fpclassify(dbl01a) << ", 0.099: " << std::fpclassify(dbl01b);

  if constexpr (std::is_floating_point<Tp>::value) {
    auto diff = std::fabs(value[0] - expected[0]) ;
    msg << "\n\nPrecision at vec[0]: " << wkg::ExpectedPrecision(value[0])
        << ", " << wkg::ExpectedPrecision(expected[0])
        << ", sum[0]: " << (value[0] + expected[0]) << ", prec: " << wkg::ExpectedPrecision(value[0] + expected[0])
        << ", diff " << diff << "< expected precision? " << (diff < wkg::ExpectedPrecision(value[0]));
  }

  // TODO binary representation: https://stackoverflow.com/a/16444778/400948
  //TODO https://isocpp.org/wiki/faq/newbie#floating-point-arith
  // py3.9+ comes with math.ulp
  /*>>> math.ulp(0.010000000000000000208)
1.734723475976807e-18
>>> math.ulp(0.0099999999999997868372)
1.734723475976807e-18
>>> 0.01 + 2
2.01
>>> 0.01 + 2 - 2
0.009999999999999787


import math
a = 0.010000000000000000208
b = 0.01 +2 -2
math.fabs(a-b)

import numpy as np
np.finfo(np.float64).eps
np.finfo(float).eps

#https://stackoverflow.com/questions/19141432/python-numpy-machine-epsilon
eps = 7./3 - 4./3 -1


2.1337098754514727e-16

*/


  return ::testing::AssertionFailure() << msg.str();
}


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

template<typename Tp, std::size_t Dim>
void TestScalarAddSub(wkg::Vec<Tp, Dim> vec) {
  const wkg::Vec<Tp, Dim> copy{vec};
  EXPECT_EQ(copy, vec);

  // Add a scalar (rhs and lhs)
  vec += 2;
  EXPECT_NE(vec, copy);
  EXPECT_TRUE(CheckVectorEqual(copy, vec - 2)); //breaks with 0.01 vs 0.01+2-2, 123 ULPs apart
  EXPECT_TRUE(CheckVectorEqual(copy, -2 + vec));
  EXPECT_TRUE(CheckVectorEqual(copy + 2, vec));
  EXPECT_TRUE(CheckVectorEqual(copy + 4, vec + 2));

  // Add a negative scalar
  vec = copy;
  EXPECT_EQ(copy, vec);
  vec += (-21);
  EXPECT_NE(vec, copy);
  EXPECT_TRUE(CheckVectorEqual(copy, vec + 21));
  EXPECT_TRUE(CheckVectorEqual(copy, 21 + vec));
  EXPECT_TRUE(CheckVectorEqual(copy - 21, vec));

  // Subtract positive scalar
  vec = copy;
  EXPECT_EQ(copy, vec);
  vec -= 23;
  EXPECT_NE(vec, copy);
  EXPECT_TRUE(CheckVectorEqual(copy, vec + 23));
  EXPECT_TRUE(CheckVectorEqual(copy, 23 + vec));
  EXPECT_TRUE(CheckVectorEqual(copy - 23, vec));


  // Subtract negative scalar
  vec = copy;
  EXPECT_EQ(copy, vec);
  vec -= (-4200);
  EXPECT_NE(vec, copy);
  EXPECT_TRUE(CheckVectorEqual(copy, vec - 4200));
  EXPECT_TRUE(CheckVectorEqual(copy + 4200, vec));
  EXPECT_TRUE(CheckVectorEqual(4200 + copy, vec));
}


template<typename Tp, std::size_t Dim>
void TestVectorAddSub(wkg::Vec<Tp, Dim> vec) {
  const wkg::Vec<Tp, Dim> copy{vec};
  EXPECT_EQ(copy, vec);

  wkg::Vec<Tp, Dim> offset{};
  const auto zero = wkg::Vec<Tp, Dim>::All(0);
  EXPECT_EQ(offset, zero);

  vec += offset;
  EXPECT_EQ(copy, vec);

  vec += (-21);
  EXPECT_NE(vec, copy);
  EXPECT_EQ(copy, vec + 19);
  EXPECT_EQ(copy - 19, vec);

  //TODO extend
}


template<typename Tp, std::size_t Dim>
void TestConversion(wkg::Vec<Tp, Dim> vec) {
  wkg::Vec<double, Dim> offset;

  if constexpr (std::numeric_limits<Tp>::is_integer) {
    auto result = vec.ToDouble() + offset;
    EXPECT_EQ(result.ToInteger(), vec);
  } else {
//    auto result = vec.ToInteger() + offset;
//    EXPECT_EQ(result, vec);
  }
}


template<typename _Tp, std::size_t Dim>
void VectorTestHelper(wkg::Vec<_Tp, Dim> &vec) {
//TODO check https://bitbashing.io/comparing-floats.html
  //
//  EXPECT_DOUBLE_EQ(0.010000000000000000208, 0.0099999999999997868372);
  // Indexing
  //TODO

  // Integral/floating point conversion
  TestConversion(vec);

  // Arithmetics
  TestScalarAddSub(vec);

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

  EXPECT_EQ(vec_3x, 3 * vec);

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

  wkg::Vec2d v2d_a{0.5, 17};
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
