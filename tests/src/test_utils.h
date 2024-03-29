#ifndef WERKZEUGKISTE_TEST_UTILS_H
#define WERKZEUGKISTE_TEST_UTILS_H

#include <gtest/gtest.h>
#include <werkzeugkiste/geometry/utils.h>
#include <werkzeugkiste/geometry/vector.h>

#include <algorithm>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

/// Check if an elapsed time "tick value" is within the
/// range [expected-pm, expected+pm].
::testing::AssertionResult CheckElapsedTime(double val,
    double expected,
    double pm);

template <typename T>
inline bool IsApproximatelyEqual(T x, T y) {
  if constexpr (std::is_same<float, T>::value) {
    // NOLINTNEXTLINE(*-magic-numbers)
    return werkzeugkiste::geometry::IsClose(x, y, 0.00001F, 0.0000001F);
  } else {
    // NOLINTNEXTLINE(*-magic-numbers)
    return werkzeugkiste::geometry::IsClose(x, y, 1e-9, 1e-12);
  }
}

/// Equality check helper which adds an error message at which dimension
/// the vector differs.
template <typename Tp, std::size_t Dim>
inline ::testing::AssertionResult CheckVectorEqual(
    const werkzeugkiste::geometry::Vec<Tp, Dim>& expected,
    const werkzeugkiste::geometry::Vec<Tp, Dim>& value) {
  if constexpr (std::is_integral<Tp>::value) {
    if (value == expected) {
      return ::testing::AssertionSuccess();
    }

    return ::testing::AssertionFailure()
        << value.ToString() << " differs from expected " << expected.ToString();
  } else {
    bool is_close{true};

    std::ostringstream msg;
    msg << value.ToString() << " differs from expected " << expected.ToString()
        << " at:" << std::setprecision(20);  // NOLINT(*-magic-numbers)
    for (std::size_t idx = 0; idx < Dim; ++idx) {
      if (!IsApproximatelyEqual(expected[idx], value[idx])) {
        is_close = false;
        msg << "\n  dim[" << idx << "]: " << expected[idx] << " vs "
            << value[idx];
      }
    }

    if (is_close) {
      return ::testing::AssertionSuccess();
    }
    return ::testing::AssertionFailure() << msg.str();
  }
}

template <typename Container>
std::string Stringify(const Container& v) {
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

template <typename Container>
void CheckMatchingContainers(const Container& expected,
    const Container& values) {
  using namespace std::string_view_literals;
  EXPECT_EQ(expected.size(), values.size())
      << "Got values: " << Stringify(values)
      << "\nExpected:  " << Stringify(expected) << "!";

  for (const auto& e : expected) {
    const auto pos = std::find(values.begin(), values.end(), e);
    EXPECT_NE(values.end(), pos)
        << "Expected value `"sv << e << "` was not found!"sv;
  }
}

#endif  // WERKZEUGKISTE_TEST_UTILS_H
