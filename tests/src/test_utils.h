#ifndef WERKZEUGKISTE_TEST_UTILS_H
#define WERKZEUGKISTE_TEST_UTILS_H

#include <type_traits>
#include <gtest/gtest.h>
#include <werkzeugkiste/geometry/utils.h>
#include <werkzeugkiste/geometry/vector.h>


/// Check if an elapsed time "tick value" is within the
/// range [expected-pm, expected+pm].
::testing::AssertionResult CheckElapsedTime(
    double val, double expected, double pm);


/// Equality check helper which adds an error message at which dimension
/// the vector differs.
template <typename Tp, std::size_t Dim>
::testing::AssertionResult CheckVectorEqual(
    const werkzeugkiste::geometry::Vec<Tp, Dim> &expected,
    const werkzeugkiste::geometry::Vec<Tp, Dim> &value) {
  if (value.EpsEquals(expected)) {
    return ::testing::AssertionSuccess();
  }

  std::ostringstream msg;
  msg << value.ToString() << " differs from expected " << expected.ToString()
      << " at:" << std::setprecision(20);  // NOLINT(*-magic-numbers)
  for (std::size_t idx = 0; idx < Dim; ++idx) {
    if (!werkzeugkiste::geometry::IsEpsEqual(expected[idx], value[idx])) {
      msg << "\n  dim[" << idx << "]: " << expected[idx]
          << " vs " << value[idx];
    }
  }
  return ::testing::AssertionFailure() << msg.str();
}

#endif  // WERKZEUGKISTE_TEST_UTILS_H
