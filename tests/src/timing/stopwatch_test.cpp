#include <chrono>
#include <thread>
#include <string>

#include "../test_utils.h"

#include <werkzeugkiste/timing/stopwatch.h>


namespace wkt = werkzeugkiste::timing;

// NOLINTBEGIN

TEST(StopWatchTest, Watches) {
  auto w1 = wkt::StopWatch();
  EXPECT_EQ(w1.ClockName(), "std::chrono::steady_clock");
  EXPECT_TRUE(w1.IsSteady());
  EXPECT_GT(w1.YearsUntilOverflow(), 292);

  auto w2 = wkt::stop_watch<std::chrono::system_clock>();
  EXPECT_EQ(w2.ClockName(), "std::chrono::system_clock");
  EXPECT_FALSE(w2.IsSteady());
}


TEST(StopWatchTest, Timings) {
  auto w1 = wkt::StopWatch();

  // The watch is started upon construction
  std::this_thread::sleep_for(std::chrono::milliseconds(120));
  double elapsed = w1.ElapsedSeconds();
  EXPECT_TRUE(CheckElapsedTime(elapsed, 0.12, 0.002));

  w1.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(90));
  elapsed = w1.ElapsedMilliseconds();
  EXPECT_TRUE(CheckElapsedTime(elapsed, 90, 2));

  w1.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(70));
  elapsed = w1.ElapsedMicroseconds();
  EXPECT_TRUE(CheckElapsedTime(elapsed, 70000, 2000));

  w1.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(40));
  elapsed = w1.ElapsedNanoseconds();
  EXPECT_TRUE(CheckElapsedTime(elapsed, 40000000, 2000000));
}


TEST(StopWatchTest, DurationAbbreviation) {
  EXPECT_EQ(wkt::DurationAbbreviation<std::chrono::nanoseconds>(), "ns");
  EXPECT_EQ(wkt::DurationAbbreviation<std::chrono::microseconds>(), "us");
  EXPECT_EQ(wkt::DurationAbbreviation<std::chrono::milliseconds>(), "ms");
  EXPECT_EQ(wkt::DurationAbbreviation<std::chrono::seconds>(), "sec");
  EXPECT_EQ(wkt::DurationAbbreviation<std::chrono::minutes>(), "min");
  EXPECT_EQ(wkt::DurationAbbreviation<std::chrono::hours>(), "hrs");
#if __cplusplus >= 202002L
  EXPECT_EQ(wkt::DurationAbbreviation<std::chrono::days>(), "days");
  EXPECT_EQ(wkt::DurationAbbreviation<std::chrono::weeks>(), "wks");
  EXPECT_EQ(wkt::DurationAbbreviation<std::chrono::months>(), "mth");
  EXPECT_EQ(wkt::DurationAbbreviation<std::chrono::years>(), "yrs");
#endif  // C++20
}


TEST(StopWatchTest, PrecisionTypeName) {
  EXPECT_EQ(wkt::PrecisionTypeName<std::chrono::nanoseconds>(), "std::chrono::nanoseconds");
  EXPECT_EQ(wkt::PrecisionTypeName<std::chrono::microseconds>(), "std::chrono::microseconds");
  EXPECT_EQ(wkt::PrecisionTypeName<std::chrono::milliseconds>(), "std::chrono::milliseconds");
  EXPECT_EQ(wkt::PrecisionTypeName<std::chrono::seconds>(), "std::chrono::seconds");
  EXPECT_EQ(wkt::PrecisionTypeName<std::chrono::minutes>(), "std::chrono::minutes");
  EXPECT_EQ(wkt::PrecisionTypeName<std::chrono::hours>(), "std::chrono::hours");
#if __cplusplus >= 202002L
  EXPECT_EQ(wkt::PrecisionTypeName<std::chrono::days>(), "std::chrono::days");
  EXPECT_EQ(wkt::PrecisionTypeName<std::chrono::weeks>(), "std::chrono::weeks");
  EXPECT_EQ(wkt::PrecisionTypeName<std::chrono::months>(), "std::chrono::months");
  EXPECT_EQ(wkt::PrecisionTypeName<std::chrono::years>(), "std::chrono::years");
#endif  // C++20
}


TEST(StopWatchTest, ClockTypeName) {
  EXPECT_EQ(wkt::ClockTypeName<std::chrono::system_clock>(), "std::chrono::system_clock");
  EXPECT_EQ(wkt::ClockTypeName<std::chrono::steady_clock>(), "std::chrono::steady_clock");
  // High resolution clock should just be an alias to system or steady clock:
  EXPECT_NE(wkt::ClockTypeName<std::chrono::high_resolution_clock>(), "std::chrono::high_resolution_clock");
  EXPECT_TRUE((wkt::ClockTypeName<std::chrono::high_resolution_clock>().compare("std::chrono::steady_clock") == 0)
              || (wkt::ClockTypeName<std::chrono::high_resolution_clock>().compare("std::chrono::system_clock") == 0));

#if __cplusplus >= 202002L
  EXPECT_EQ(wkt::ClockTypeName<std::chrono::utc_clock>(), "std::chrono::utc_clock");
  EXPECT_EQ(wkt::ClockTypeName<std::chrono::tai_clock>(), "std::chrono::tai_clock");
  EXPECT_EQ(wkt::ClockTypeName<std::chrono::gps_clock>(), "std::chrono::gps_clock");
  EXPECT_EQ(wkt::ClockTypeName<std::chrono::file_clock>(), "std::chrono::file_clock");
  EXPECT_EQ(wkt::ClockTypeName<std::chrono::local_t>(), "std::chrono::local_t");
#endif  // C++20
}


TEST(StopWatchTest, CastToTicks) {
  EXPECT_DOUBLE_EQ(wkt::ToSeconds(std::chrono::hours(21)), 75600.0);
  EXPECT_DOUBLE_EQ(wkt::ToSeconds(std::chrono::minutes(59)), 3540.0);
  EXPECT_DOUBLE_EQ(wkt::ToSeconds(std::chrono::seconds(50)), 50.0);
  EXPECT_DOUBLE_EQ(wkt::ToSeconds(std::chrono::milliseconds(50)), 0.05);
  EXPECT_DOUBLE_EQ(wkt::ToSeconds(std::chrono::milliseconds(1234)), 1.234);
  EXPECT_DOUBLE_EQ(wkt::ToSeconds(std::chrono::nanoseconds(999999999)), 0.999999999);

  EXPECT_DOUBLE_EQ(wkt::ToMilliseconds(std::chrono::seconds(12)), 12000.0);
  EXPECT_DOUBLE_EQ(wkt::ToMilliseconds(std::chrono::milliseconds(1234)), 1234.0);
  EXPECT_DOUBLE_EQ(wkt::ToMilliseconds(std::chrono::microseconds(1234)), 1.234);
  EXPECT_DOUBLE_EQ(wkt::ToMilliseconds(std::chrono::nanoseconds(1000000)), 1.0);

  EXPECT_DOUBLE_EQ(wkt::ToMicroseconds(std::chrono::seconds(12)), 12000000.0);
  EXPECT_DOUBLE_EQ(wkt::ToMicroseconds(std::chrono::milliseconds(3)), 3000.0);
  EXPECT_DOUBLE_EQ(wkt::ToMicroseconds(std::chrono::microseconds(1)), 1.0);
  EXPECT_DOUBLE_EQ(wkt::ToMicroseconds(std::chrono::nanoseconds(12)), 0.012);

  EXPECT_DOUBLE_EQ(wkt::ToNanoseconds(std::chrono::seconds(12)), 12.0e9);
  EXPECT_DOUBLE_EQ(wkt::ToNanoseconds(std::chrono::milliseconds(1234)), 1234.0e6);
  EXPECT_DOUBLE_EQ(wkt::ToNanoseconds(std::chrono::microseconds(789)), 789000.0);
  EXPECT_DOUBLE_EQ(wkt::ToNanoseconds(std::chrono::nanoseconds(951)), 951.0);
}


TEST(StopWatchTest, StringRepresentation) {
  EXPECT_EQ(wkt::SecondsToString(0), "0 seconds");
  EXPECT_EQ(wkt::SecondsToString(1), "1 second");
  EXPECT_EQ(wkt::SecondsToString(59), "59 seconds");
  EXPECT_EQ(wkt::SecondsToString(60), "1 minute");
  EXPECT_EQ(wkt::SecondsToString(61), "1 minute 1 second");
  EXPECT_EQ(wkt::SecondsToString(121), "2 minutes 1 second");
  EXPECT_EQ(wkt::SecondsToString(185), "3 minutes 5 seconds");
  EXPECT_EQ(wkt::SecondsToString(3599), "59 minutes 59 seconds");
  EXPECT_EQ(wkt::SecondsToString(3600), "1 hour");
  EXPECT_EQ(wkt::SecondsToString(3601), "1 hour");
  EXPECT_EQ(wkt::SecondsToString(3666), "1 hour 1 minute");
  EXPECT_EQ(wkt::SecondsToString(86399), "23 hours 59 minutes");
  EXPECT_EQ(wkt::SecondsToString(86400), "1 day");
  EXPECT_EQ(wkt::SecondsToString(86405), "1 day");
  EXPECT_EQ(wkt::SecondsToString(86460), "1 day 1 minute");
  EXPECT_EQ(wkt::SecondsToString(86465), "1 day 1 minute");
  EXPECT_EQ(wkt::SecondsToString(86520), "1 day 2 minutes");
  EXPECT_EQ(wkt::SecondsToString(88650), "1 day 37 minutes");
}

// NOLINTEND
