#include "test_utils.h"

::testing::AssertionResult CheckElapsedTime(double val, double expected,
                                            double pm) {
  if ((val >= (expected - pm)) && (val <= (expected + pm))) {
    return ::testing::AssertionSuccess();
  } else {  // NOLINT(*else-after-return)
    // LCOV_EXCL_START
    return ::testing::AssertionFailure()
           << "Elapsed time " << val << " is not within " << expected << " +/- "
           << pm;
    // LCOV_EXCL_STOP
  }
}
