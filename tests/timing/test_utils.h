#ifndef __TIMEUTILS_TEST_UTILS_H__
#define __TIMEUTILS_TEST_UTILS_H__

#include <string>
#include <gtest/gtest.h>


//------ String utils copied from my (outdated) vcp library
/** Remove whitespace from the beginning of the string. */
std::string LTrim(const std::string &totrim);

/** Remove whitespace at the end of the string. */
std::string RTrim(const std::string &totrim);

/** Remove whitespace at the beginning and the end of the string. */
std::string Trim(const std::string &s);


/** Check if an elapsed time "tick value" is within the range [expected-pm, expected+pm]. */
::testing::AssertionResult CheckElapsedTime(double val, double expected, double pm);

/** Check if the given string starts with this prefix. */
::testing::AssertionResult StartsWith(const std::string &s, const std::string &prefix);

/** Check if the given string ends with this suffix. */
::testing::AssertionResult EndsWith(const std::string &s, const std::string &suffix);


#endif  // __TIMEUTILS_TEST_UTILS_H__
