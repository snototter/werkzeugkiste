#include "test_utils.h"

::testing::AssertionResult CheckElapsedTime(double val, double expected,
                                            double pm) {
  if ((val >= (expected - pm)) && (val <= (expected + pm))) {
    return ::testing::AssertionSuccess();
  } else {  // NOLINT
    return ::testing::AssertionFailure()
           << "Elapsed time " << val << " is not within " << expected << " +/- "
           << pm;
  }
}
