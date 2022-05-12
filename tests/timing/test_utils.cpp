#include <chrono>
#include <thread>
#include <algorithm>
#include <string>
#include <iomanip>

#include <gtest/gtest.h>


//FIXME REMOVE once we include wkz::strings
std::string LTrim(const std::string &totrim)
{
  std::string s(totrim);
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
  return s;
}



std::string RTrim(const std::string &totrim)
{
  std::string s(totrim);
  s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
  return s;
}


std::string Trim(const std::string &s)
{
  return LTrim(RTrim(s));
}



::testing::AssertionResult CheckElapsedTime(double val, double expected, double pm) {
  if((val >= expected - pm) && (val <= expected + pm))
    return ::testing::AssertionSuccess();
  else
    return ::testing::AssertionFailure()
           << "Elapsed time " << val << " is not within " << expected << " +/- " << pm;
}



::testing::AssertionResult StartsWith(const std::string &s, const std::string &prefix) {
  if ((s.length() >= prefix.length()) &&
      (s.compare(0, prefix.length(), prefix) == 0))
    return ::testing::AssertionSuccess();
  else
    return ::testing::AssertionFailure()
           << "String \"" << s << "\" doesn't start with \"" << prefix << "\"";
}


::testing::AssertionResult EndsWith(const std::string &s, const std::string &suffix) {
  if ((s.length() >= suffix.length()) &&
      (s.compare(s.length() - suffix.length(), suffix.length(), suffix) == 0))
    return ::testing::AssertionSuccess();
  else
    return ::testing::AssertionFailure()
           << "String \"" << s << "\" doesn't end with \"" << suffix << "\"";
}

