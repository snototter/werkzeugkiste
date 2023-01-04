#ifndef __TIMEUTILS_TEST_UTILS_H__
#define __TIMEUTILS_TEST_UTILS_H__

#include <gtest/gtest.h>

/** Check if an elapsed time "tick value" is within the range [expected-pm, expected+pm]. */
::testing::AssertionResult CheckElapsedTime(double val, double expected, double pm);

#endif  // __TIMEUTILS_TEST_UTILS_H__
