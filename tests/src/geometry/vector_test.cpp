#include <werkzeugkiste/geometry/utils.h>
#include <werkzeugkiste/geometry/vector.h>

#include <cmath>
#include <exception>
#include <initializer_list>
#include <iomanip>
#include <list>
#include <vector>

#include "../test_utils.h"

namespace wkg = werkzeugkiste::geometry;

// NOLINTBEGIN

template <typename Tp, std::size_t Dim>
void TestConversion(wkg::Vec<Tp, Dim> vec) {
  wkg::Vec<double, Dim> offset_dbl;
  wkg::Vec<int32_t, Dim> offset_int;

  if constexpr (std::numeric_limits<Tp>::is_integer) {
    auto result_dbl = vec.ToDouble() + offset_dbl;
    EXPECT_TRUE(CheckVectorEqual(result_dbl.ToInteger(), vec));

    offset_dbl.SetX(-17);
    offset_dbl.SetY(4.2);
    result_dbl = vec.ToDouble() + offset_dbl;

    EXPECT_DOUBLE_EQ(result_dbl[0], vec[0] - 17.0);
    EXPECT_DOUBLE_EQ(result_dbl[1], vec[1] + 4.2);

    auto result_int = result_dbl.ToInteger();
    EXPECT_EQ(vec[0] - 17, result_int[0]);
    EXPECT_EQ(vec[1] + 4, result_int[1]);
  } else {
    auto result_int = vec.ToInteger() + offset_int;
    EXPECT_TRUE(CheckVectorEqual(result_int, vec.ToInteger()));

    offset_dbl.SetX(-17);
    offset_dbl.SetY(4.2);
    auto result_dbl = vec + offset_dbl;
    EXPECT_EQ(vec[0] - 17, result_dbl[0]);
    EXPECT_EQ(vec[1] + 4.2, result_dbl[1]);

    offset_int = offset_dbl.ToInteger();
    EXPECT_EQ(-17, offset_int[0]);
    EXPECT_EQ(4, offset_int[1]);

    result_dbl = vec + offset_int.ToDouble();
    EXPECT_EQ(vec[0] - 17, result_dbl[0]);
    EXPECT_EQ(vec[1] + 4, result_dbl[1]);
  }
}

template <typename Tp, std::size_t Dim>
void TestIndexing(wkg::Vec<Tp, Dim> vec) {
  // Negative indexing with range-checked access:
  int neg_idx = -1;
  for (std::size_t pos_idx = 0; pos_idx < Dim - 1; ++pos_idx, --neg_idx) {
    EXPECT_DOUBLE_EQ(vec[neg_idx], vec[Dim - pos_idx - 1]);
  }

  // Test out-of-bounds access
  constexpr int DimInt = static_cast<int>(Dim);
  EXPECT_NO_THROW(vec[DimInt - 1]);
  EXPECT_THROW(vec[DimInt], std::out_of_range);
  EXPECT_THROW(vec[DimInt + 1], std::out_of_range);

  EXPECT_NO_THROW(vec[-DimInt]);
  EXPECT_THROW(vec[-DimInt - 1], std::out_of_range);
  EXPECT_THROW(vec[-DimInt - 2], std::out_of_range);

  // Negative indexing with unchecked access:
  for (int idx = 0; idx < DimInt; ++idx) {
    EXPECT_EQ(vec.val[DimInt - idx - 1], vec[-(idx + 1)]);
  }

  // Mutable access:
  for (int idx = 0; idx < DimInt; ++idx) {
    vec[idx] = 42 * idx;
    EXPECT_DOUBLE_EQ(vec.val[idx], 42 * idx);
  }
}

template <typename Tp>
void TestVec2dSizeAccess(wkg::Vec<Tp, 2> vec) {
  // A 2d vector can be used to represent a size/dimension:
  EXPECT_DOUBLE_EQ(vec.X(), vec.Width());
  EXPECT_TRUE(wkg::IsEpsEqual(vec.X(), vec.Width()));

  EXPECT_DOUBLE_EQ(vec.Y(), vec.Height());
  EXPECT_TRUE(wkg::IsEpsEqual(vec.Y(), vec.Height()));

  // Adjust its height/width
  vec.SetWidth(2 * vec.X());
  vec.SetHeight(3 * vec.Y());

  EXPECT_DOUBLE_EQ(vec.X(), vec.Width());
  EXPECT_TRUE(wkg::IsEpsEqual(vec.X(), vec.Width()));

  EXPECT_DOUBLE_EQ(vec.Y(), vec.Height());
  EXPECT_TRUE(wkg::IsEpsEqual(vec.Y(), vec.Height()));
}

template <typename Tp>
void TestVec2dGeometry(wkg::Vec<Tp, 2> vec) {
  // In 2d, we can easily rotate a vector by +/- 90 degrees:
  auto perpendicular = vec.PerpendicularClockwise();
  EXPECT_TRUE(wkg::IsEpsEqual(perpendicular.X(), vec.Y()));
  EXPECT_TRUE(wkg::IsEpsEqual(perpendicular.Y(), -vec.X()));

  perpendicular = vec.PerpendicularCounterClockwise();
  EXPECT_TRUE(wkg::IsEpsEqual(perpendicular.X(), -vec.Y()));
  EXPECT_TRUE(wkg::IsEpsEqual(perpendicular.Y(), vec.X()));

  // TODO extend (determinant, arbitrary rotation...)
}

template <typename Tp, std::size_t Dim>
void TestNegation(wkg::Vec<Tp, Dim> vec) {
  const wkg::Vec<Tp, Dim> copy{vec};
  EXPECT_EQ(copy, vec);

  auto negated = -vec;
  EXPECT_EQ(copy, vec);
  EXPECT_NE(copy, negated);

  EXPECT_TRUE(CheckVectorEqual(-copy, -vec));
  EXPECT_TRUE(CheckVectorEqual(-copy, negated));
  EXPECT_TRUE(CheckVectorEqual(copy, -negated));
  EXPECT_TRUE(CheckVectorEqual(static_cast<Tp>(-1) * copy, -vec));
  EXPECT_TRUE(CheckVectorEqual(-copy, static_cast<Tp>(-1) * vec));

  EXPECT_TRUE(wkg::IsEpsEqual(vec.Length(), negated.Length()));
  EXPECT_TRUE(wkg::IsEpsEqual(vec.LengthSquared(), negated.LengthSquared()));
  EXPECT_TRUE(
      wkg::IsEpsEqual(2 * vec.Length(), vec.DistanceEuclidean(negated)));

  auto abs1 = negated.Absolute();
  auto abs2 = vec.Absolute();
  EXPECT_TRUE(CheckVectorEqual(abs1, abs2));

  for (std::size_t idx = 0; idx < Dim; ++idx) {
    EXPECT_TRUE(abs1[idx] >= static_cast<Tp>(0));
  }
}

template <typename Tp, std::size_t Dim>
void TestScalarAddSub(wkg::Vec<Tp, Dim> vec) {
  using V = wkg::Vec<Tp, Dim>;
  const V copy{vec};
  EXPECT_EQ(copy, vec);

  // Add a scalar (rhs and lhs)
  vec += 2;
  EXPECT_NE(vec, copy);
  EXPECT_TRUE(CheckVectorEqual(copy, vec - 2));
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
  vec -= (-512);
  EXPECT_NE(vec, copy);
  EXPECT_TRUE(CheckVectorEqual(copy, vec - 512));
  EXPECT_TRUE(CheckVectorEqual(copy + 512, vec));
  EXPECT_TRUE(CheckVectorEqual(512 + copy, vec));

  // Subtract vector from a scalar
  vec = copy;
  EXPECT_EQ(copy, vec);
  V subtracted = 2 - vec;
  EXPECT_TRUE(CheckVectorEqual(V::All(2) - vec, subtracted));
  EXPECT_TRUE(CheckVectorEqual(vec.Negate() + 2, subtracted));
}

template <typename Tp, std::size_t Dim>
void TestVectorAddSub(wkg::Vec<Tp, Dim> vec) {
  const wkg::Vec<Tp, Dim> copy{vec};
  EXPECT_EQ(copy, vec);

  wkg::Vec<Tp, Dim> offset{};
  const auto zero = wkg::Vec<Tp, Dim>::All(0);
  EXPECT_EQ(offset, zero);

  vec += offset;
  EXPECT_EQ(copy, vec);

  for (std::size_t idx = 0; idx < Dim; ++idx) {
    offset[idx] = static_cast<Tp>(42 * idx);
  }

  // Add positive offset vector
  vec += offset;
  EXPECT_NE(vec, copy);
  EXPECT_TRUE(CheckVectorEqual(copy, vec - offset));
  EXPECT_TRUE(CheckVectorEqual(copy + offset, vec));

  // Subtract positive offset vector
  vec -= offset;
  EXPECT_TRUE(CheckVectorEqual(copy, vec));

  vec -= offset;
  EXPECT_NE(vec, copy);
  EXPECT_TRUE(CheckVectorEqual(copy, vec + offset));
  EXPECT_TRUE(CheckVectorEqual(copy - offset, vec));

  for (std::size_t idx = 0; idx < Dim; ++idx) {
    offset[idx] = static_cast<Tp>(420 * (idx + 1));
  }

  // Add negated vector
  vec = copy;
  EXPECT_TRUE(CheckVectorEqual(copy, vec));
  vec += (-offset);
  EXPECT_NE(vec, copy);
  EXPECT_TRUE(CheckVectorEqual(copy - offset, vec));
  EXPECT_TRUE(CheckVectorEqual(copy, vec + offset));

  // Subtract negated vector
  vec = copy;
  EXPECT_EQ(vec, copy);
  vec -= (-offset);
  EXPECT_NE(vec, copy);
  EXPECT_TRUE(CheckVectorEqual(copy + offset, vec));
  EXPECT_TRUE(CheckVectorEqual(copy, vec - offset));
}

template <typename Tp, std::size_t Dim>
::testing::AssertionResult CheckDivisionResult(
    const wkg::Vec<Tp, Dim>& dividend,
    const wkg::Vec<Tp, Dim>& divisor,
    const wkg::Vec<Tp, Dim>& expected_non_special,
    const wkg::Vec<Tp, Dim>& value) {
  std::ostringstream msg;
  msg << "Division error: " << dividend.ToString() << " / "
      << divisor.ToString() << " = " << value
      << ", which is wrong at:" << std::setprecision(20);

  bool success{true};
  for (std::size_t idx = 0; idx < Dim; ++idx) {
    if (wkg::IsEpsZero(divisor[idx])) {
      if (wkg::IsEpsZero(dividend[idx])) {
        // "0/0" = +inf or -inf
        if (!std::isinf(value[idx])) {
          msg << "\n  dim[" << idx << "]: should be +/-inf but is "
              << value[idx];
        }
      } else {
        // "v/0" = +nan or -nan
        if (!std::isnan(value[idx])) {
          msg << "\n  dim[" << idx << "]: should be +/-nan but is "
              << value[idx];
        }
      }
    } else {
      if (!wkg::IsEpsEqual(expected_non_special[idx], value[idx])) {
        success = false;
        msg << "\n  dim[" << idx << "]: " << expected_non_special[idx] << " vs "
            << value[idx];
      }
    }
  }

  if (success) {
    return ::testing::AssertionSuccess();
  }

  return ::testing::AssertionFailure() << msg.str();
}

template <typename Tp, std::size_t Dim>
void TestScalarMulDiv(wkg::Vec<Tp, Dim> vec) {
  const wkg::Vec<Tp, Dim> copy{vec};
  EXPECT_EQ(copy, vec);

  // Multiplication is supported for any vector type.
  vec *= 1;
  EXPECT_EQ(copy, vec);

  vec *= 2;
  EXPECT_NE(copy, vec);
  EXPECT_TRUE(CheckVectorEqual(copy + copy, vec));

  vec = copy;
  vec = vec * 3;
  EXPECT_TRUE(CheckVectorEqual(copy + copy + copy, vec));
  EXPECT_TRUE(CheckVectorEqual(copy + copy, vec - (1 * copy)));
  EXPECT_TRUE(CheckVectorEqual(copy, vec - (2 * copy)));

  vec = copy;
  vec = 4 * copy;
  EXPECT_TRUE(CheckVectorEqual(copy + (2 * copy) + copy, vec));

  vec *= 0;
  const wkg::Vec<Tp, Dim> zero{};
  EXPECT_TRUE(CheckVectorEqual(zero, vec));
  EXPECT_TRUE(CheckVectorEqual(0 * copy, vec));

  // Division is only supported for floating point vectors.
  auto vec_dbl = vec.ToDouble();

  // (1/x) * V = V / x
  EXPECT_TRUE(CheckVectorEqual(vec_dbl * 0.5, vec_dbl / 2.0));
  EXPECT_TRUE(CheckVectorEqual(0.5 * vec_dbl, vec_dbl / 2.0));

  EXPECT_TRUE(CheckVectorEqual(vec_dbl * 0.1, vec_dbl / 10.0));
  EXPECT_TRUE(CheckVectorEqual(0.1 * vec_dbl, vec_dbl / 10.0));
}

template <typename Tp, std::size_t Dim>
void TestVectorMulDiv(wkg::Vec<Tp, Dim> vec) {
  const wkg::Vec<Tp, Dim> copy{vec};
  EXPECT_EQ(copy, vec);

  // Element-wise multiplication
  auto mul = vec * copy;
  EXPECT_EQ(copy, vec);
  for (std::size_t idx = 0; idx < Dim; ++idx) {
    EXPECT_DOUBLE_EQ(copy[idx] * copy[idx], mul[idx]);
  }

  mul *= vec;
  EXPECT_EQ(copy, vec);
  EXPECT_TRUE(CheckVectorEqual(copy * copy * copy, mul));

  vec *= vec;
  EXPECT_NE(copy, vec);
  EXPECT_TRUE(CheckVectorEqual(copy * copy, vec));

  // Element-wise division
  // V / V = 1
  auto vec_dbl = vec.ToDouble();
  auto ones = wkg::Vec<double, Dim>::All(1.0);

  auto result = vec_dbl / vec_dbl;
  EXPECT_TRUE(CheckDivisionResult(vec_dbl, vec_dbl, ones, result));

  auto another_copy{vec_dbl};
  EXPECT_EQ(vec_dbl, another_copy);
  another_copy /= another_copy;
  EXPECT_TRUE(CheckDivisionResult(vec_dbl, vec_dbl, ones, result));

  // V * (1 / V) = 1
  auto vec_inv_mul = 1.0 / vec_dbl;
  auto expected = vec_dbl * vec_inv_mul;
  EXPECT_TRUE(CheckDivisionResult(ones, vec_dbl, expected, result));
  // (1 / V) * V = 1
  expected = vec_inv_mul * vec_dbl;
  EXPECT_TRUE(CheckDivisionResult(ones, vec_dbl, expected, result));

  // (1 / (V / 2)) * V = 1 / 2
  wkg::Vec<double, Dim> dividend{vec_inv_mul};
  vec_inv_mul /= 2;
  result = vec_inv_mul * vec_dbl;
  EXPECT_TRUE(CheckDivisionResult(ones, vec_dbl / 2, ones / 2, result));
  EXPECT_TRUE(CheckDivisionResult(ones, vec_dbl / 2, ones * 0.5, result));
}

template <typename Tp, std::size_t Dim>
void TestVectorBasics(wkg::Vec<Tp, Dim> vec) {
  using V = wkg::Vec<Tp, Dim>;
  double length_squared{0.0};

  for (std::size_t idx = 0; idx < Dim; ++idx) {
    length_squared += static_cast<double>(vec[idx] * vec[idx]);
  }

  EXPECT_TRUE(
      IsApproximatelyEqual(length_squared, static_cast<double>(vec.Dot(vec))))
      << "  Dot product <v, v> = " << vec.Dot(vec) << " does not equal "
      << length_squared << "!";

  EXPECT_TRUE(IsApproximatelyEqual(length_squared, vec.LengthSquared()))
      << "  Squared length " << vec.LengthSquared() << " does not equal "
      << length_squared << "!";

  const double length = std::sqrt(length_squared);
  EXPECT_TRUE(IsApproximatelyEqual(length, vec.Length()))
      << "  Length " << vec.Length() << " does not equal " << length_squared
      << "!";

  // <v, 1> = <1, v>
  // and <v, 0> = <0, v>
  V ones = V::All(1);
  V zeros{};
  Tp zero{0};
  if constexpr (std::is_integral<Tp>::value) {
    EXPECT_EQ(zero, vec.Dot(zeros));
    EXPECT_EQ(zero, zeros.Dot(vec));

    EXPECT_EQ(vec.Sum(), vec.Dot(ones));
    EXPECT_EQ(vec.Sum(), ones.Dot(vec));
  } else {
    EXPECT_TRUE(IsApproximatelyEqual(zero, vec.Dot(zeros)));
    EXPECT_TRUE(IsApproximatelyEqual(zero, zeros.Dot(vec)));

    EXPECT_TRUE(IsApproximatelyEqual(vec.Sum(), vec.Dot(ones)));
    EXPECT_TRUE(IsApproximatelyEqual(vec.Sum(), ones.Dot(vec)));
  }
}

template <typename Tp>
void TestVec3dGeometry(wkg::Vec<Tp, 3> vec) {
  // Cross product ------------------------------------------------------------
  wkg::Vec<Tp, 3> other{-3, 15, 21};

  // Sanity check
  wkg::Vec<Tp, 3> zeros{};
  EXPECT_TRUE(CheckVectorEqual(zeros, vec.Cross(zeros)));

  // Actual cross product
  auto cross = vec.Cross(other);
  wkg::Vec<Tp, 3> expected{vec.Y() * other.Z() - vec.Z() * other.Y(),
      vec.Z() * other.X() - vec.X() * other.Z(),
      vec.X() * other.Y() - vec.Y() * other.X()};

  EXPECT_TRUE(CheckVectorEqual(expected, cross));

  // A x B = -(B x A)
  EXPECT_TRUE(CheckVectorEqual(cross, -(other.Cross(vec))));

  // (A + B) x C = A x C + B x C
  wkg::Vec<Tp, 3> another{47, -23, -1023};
  EXPECT_TRUE(CheckVectorEqual(
      (vec + other).Cross(another), vec.Cross(another) + other.Cross(another)));
}

template <typename Tp, std::size_t Dim>
void TestHomogeneous(wkg::Vec<Tp, Dim> vec) {
  auto vh = vec.Homogeneous();
  EXPECT_EQ(Dim + 1, vh.ndim);

  for (std::size_t idx = 0; idx < Dim; ++idx) {
    EXPECT_TRUE(wkg::IsEpsEqual(vec[idx], vh[idx]));
  }
  EXPECT_TRUE(wkg::IsEpsEqual(static_cast<Tp>(1), vh[Dim]));
}

template <typename Tp, std::size_t Dim>
void VectorTestHelper(wkg::Vec<Tp, Dim> vec) {
  // Indexing
  TestIndexing(vec);

  // Integral/floating point conversion
  TestConversion(vec);

  // Arithmetic
  TestScalarAddSub(vec);
  TestVectorAddSub(vec);
  TestScalarMulDiv(vec);
  TestVectorMulDiv(vec);

  TestVectorBasics(vec);

  TestHomogeneous(vec);

  // Special features of 2d vectors:
  if constexpr (Dim == 2) {
    TestVec2dSizeAccess(vec);
    TestVec2dGeometry(vec);
  }

  // Special features of 3d vectors:
  if constexpr (Dim == 3) {
    TestVec3dGeometry(vec);
  }

  // TODO to test:
  // Dot
  // Length / LengthSq
  // DirectionVector
  // UnitVector
  // DistanceEuclidean / DistanceManhattan
  // ToString

  // TODO non-class functions
  // LengthPolygon
  // Determinant
  // ScalarProjection / VectorProject
  // AngleRadFromDirectionVec / AngleDegFromDirectionVec
  // DirectionVecFromAngleRad / DirectionVecFromAngleDeg
  // RotateVector
  // MinMaxCoordinates

  // TODO refactor below!

  // Create a copy
  auto copy = wkg::Vec<Tp, Dim>(vec);
  EXPECT_EQ(vec, copy);

  // Basic arithmetic
  auto vec_twice = vec + vec;
  EXPECT_NE(vec, vec_twice);
  EXPECT_EQ(vec * static_cast<Tp>(2), vec_twice);
  EXPECT_EQ(static_cast<Tp>(2) * vec, vec_twice);

  vec *= 2;
  EXPECT_EQ(vec, vec_twice);

  std::vector<wkg::Vec<Tp, Dim>> poly{vec, vec_twice};
  double poly_len = wkg::LengthPolygon(poly);
  EXPECT_DOUBLE_EQ(poly_len, vec.DistanceEuclidean(vec_twice));

  poly.push_back(vec);
  poly_len = wkg::LengthPolygon(poly);
  EXPECT_DOUBLE_EQ(poly_len, 2 * vec.DistanceEuclidean(vec_twice));

  if constexpr (std::is_integral<Tp>::value) {
    auto dbl = vec.ToDouble();
    dbl /= 2;
    for (std::size_t idx = 0; idx < Dim; ++idx) {
      vec[idx] = static_cast<Tp>(dbl[idx]);
    }
  } else {
    vec /= 2;
    EXPECT_EQ(vec_twice / static_cast<Tp>(2), vec);
  }

  // Assignment operator
  vec_twice = vec;
  EXPECT_EQ(vec_twice, vec);
  EXPECT_TRUE(vec_twice == copy);

  const auto vec_3x = vec + vec_twice + copy;
  EXPECT_EQ(static_cast<Tp>(3) * vec, vec_3x);

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
  wkg::Vec<Tp, Dim> zero;
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

  // Distance/Length & dot product:
  auto dot1 = vec.Dot(vec);
  auto dot3 = vec.Dot(vec_3x);
  EXPECT_DOUBLE_EQ(3.0 * dot1, 1.0 * dot3);

  auto len = vec.Length();
  EXPECT_DOUBLE_EQ(std::sqrt(1.0 * dot1), 1.0 * len);

  EXPECT_DOUBLE_EQ(1.0 * dot1, vec.LengthSquared());

  auto dist = vec.DistanceEuclidean(zero);
  EXPECT_DOUBLE_EQ(dist, len);

  auto vec_4x = static_cast<Tp>(4) * vec;
  dist = vec.DistanceEuclidean(vec_4x);
  EXPECT_DOUBLE_EQ(dist, len * 3);
}

template <typename Tp, std::size_t Dim>
void TestCreation(wkg::Vec<Tp, Dim> vec) {
  using V = wkg::Vec<Tp, Dim>;

  V copy{vec};
  EXPECT_EQ(copy, vec);

  auto all1 = V::All(-17);
  auto all2 = V::All(12345);
  auto all3 = V::All(0);
  auto all4 = V::All(static_cast<Tp>(0.001));
  for (std::size_t idx = 0; idx < Dim; ++idx) {
    EXPECT_TRUE(wkg::IsEpsEqual(static_cast<Tp>(-17), all1[idx]));
    EXPECT_TRUE(wkg::IsEpsEqual(static_cast<Tp>(12345), all2[idx]));
    EXPECT_TRUE(wkg::IsEpsZero(all3[idx]));
    EXPECT_TRUE(wkg::IsEpsEqual(static_cast<Tp>(0.001), all4[idx]));
  }

  // Sanity check that assignment operator copies the values
  copy = all1;
  EXPECT_NE(vec, copy);
  EXPECT_EQ(all1, copy);

  copy[0] = 123;
  EXPECT_NE(all1, copy);

  copy = vec;
  EXPECT_EQ(vec, copy);
}

TEST(VectorTest, Initialization) {
  EXPECT_THROW((wkg::Vec2d{1, 2, 3}), std::invalid_argument);
  EXPECT_THROW((wkg::Vec3d{2, 17}), std::invalid_argument);
  EXPECT_THROW((wkg::Vec4d{2, 17}), std::invalid_argument);
  EXPECT_THROW((wkg::Vec4d{2, 17, 3}), std::invalid_argument);

  wkg::Vec2d v2d{-0.1, 23.4};
  TestCreation(v2d);

  wkg::Vec3d v3d{0.001, 1e-4, 1e-6};
  TestCreation(v3d);

  wkg::Vec4d v4d{-20.001, 17.23, -(1e-10), 99.9};
  TestCreation(v4d);

  wkg::Vec2i v2i{-987, -754321};
  TestCreation(v2i);

  wkg::Vec3i v3i{123456, 0, -1234};
  TestCreation(v3i);
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

  wkg::Vec3d v3d_d{1, 0, -0.0001};
  EXPECT_DOUBLE_EQ(v3d_d.MaxValue(), 1);
  EXPECT_DOUBLE_EQ(v3d_d.MinValue(), -0.0001);
  EXPECT_EQ(v3d_d.MaxIndex(), 0);
  EXPECT_EQ(v3d_d.MinIndex(), 2);
  VectorTestHelper(v3d_d);

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

TEST(VectorTest, EpsEqual) {
  std::vector<wkg::Vec2d> data{{1, 0},
      {10, -3},
      {-15, 1},
      {17, 42},
      {0.1, 17.0},
      {0.001, -0.005},
      {1e-5, -(1e-7)}};

  for (const auto& vec : data) {
    EXPECT_EQ(vec, vec);

    // The largest values for these tests are in the tens, i.e. 10 <= x <= 100.
    // Thus, 1e-7 is the smallest offset we can add to all vectors such that
    // the IsClose/ApproximatelyEqual tests will succeed (because of the
    // default relative tolerance of 1e-9).
    for (auto offset :
        {0.1, 1e-2, 1e-3, 1e-4, 1e-5, 1e-6, 1e-7, 0.5, 1.5, 100.0}) {
      auto copy = vec.ToDouble();
      EXPECT_TRUE(CheckVectorEqual(vec, copy));

      copy[0] += offset;
      auto diff = std::fabs(copy[0] - vec[0]);
      EXPECT_FALSE(CheckVectorEqual(vec, copy))
          << "  Adding " << offset << " to x-dimension of " << vec
          << " did not result in a sufficiently different vector!"
             "\n  Difference is "
          << diff;

      copy[0] -= offset;
      EXPECT_TRUE(CheckVectorEqual(vec, copy));

      copy[1] -= offset;
      diff = std::fabs(copy[1] - vec[1]);
      EXPECT_FALSE(CheckVectorEqual(vec, copy))
          << "  Subtracting " << offset << " from y-dimension of " << vec
          << " did not result in a sufficiently different vector!"
             "\n  Difference is "
          << diff;

      copy[1] += offset;
      EXPECT_TRUE(CheckVectorEqual(vec, copy));
    }
  }
}

TEST(VectorTest, MinMaxCoordinates) {
  // Find the minimum/maximum entries in a std::vector of Vec:
  std::vector<wkg::Vec2i> data2i{{1, 0}, {10, -3}, {-15, 1}, {17, 42}};
  wkg::Vec2i min, max;
  wkg::MinMaxCoordinates(data2i, min, max);

  EXPECT_EQ(min.X(), -15);
  EXPECT_EQ(min.Y(), -3);

  EXPECT_EQ(max.X(), 17);
  EXPECT_EQ(max.Y(), 42);

  // Similarly, find min/max entries for double-precision Vec's:
  std::list<wkg::Vec3d> data3d{{10, 0, 1}, {100, -3, 17}, {1, 0, -1}};
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
