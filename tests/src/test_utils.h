#ifndef WERKZEUGKISTE_TEST_UTILS_H
#define WERKZEUGKISTE_TEST_UTILS_H

#include <gtest/gtest.h>

/// Check if an elapsed time "tick value" is within the
/// range [expected-pm, expected+pm].
::testing::AssertionResult CheckElapsedTime(
    double val, double expected, double pm);

#endif  // WERKZEUGKISTE_TEST_UTILS_H
