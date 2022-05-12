#include <chrono>
#include <thread>
#include <string>
#include <exception>

#include <gtest/gtest.h>

#include <werkzeugkiste/timing/tictoc.h>

#include "test_utils.h"

namespace wkt = werkzeugkiste::timing;

TEST(TicTocTest, ElapsedTimes) {
  wkt::tic();
  std::this_thread::sleep_for(std::chrono::milliseconds(120));
  double elapsed = wkt::ttoc_sec();
  EXPECT_TRUE(CheckElapsedTime(elapsed, 0.12, 0.002));

  wkt::tic();
  std::this_thread::sleep_for(std::chrono::milliseconds(120));
  elapsed = wkt::ttoc_ms();
  EXPECT_TRUE(CheckElapsedTime(elapsed, 120, 2));

  wkt::tic();
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  elapsed = wkt::ttoc_us();
  EXPECT_TRUE(CheckElapsedTime(elapsed, 50000, 2000));

  wkt::tic();
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  elapsed = wkt::ttoc_ns();
  EXPECT_TRUE(CheckElapsedTime(elapsed, 50000000,
                                         2000000));
}


TEST(TicTocTest, TocOutput) {
  wkt::tic();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Output seconds
  testing::internal::CaptureStdout();
  wkt::toc_sec();
  std::string output = Trim(testing::internal::GetCapturedStdout());
  EXPECT_TRUE(StartsWith(output, "Elapsed time: 0.1"));
  EXPECT_TRUE(EndsWith(output, " sec"));

  // Output milliseconds
  testing::internal::CaptureStdout();
  wkt::toc_ms();
  output = Trim(testing::internal::GetCapturedStdout());
  EXPECT_TRUE(StartsWith(output, "Elapsed time: "));
  EXPECT_TRUE(EndsWith(output, " ms"));

  // Output microseconds
  const std::string label("wat!ch");
  wkt::tic(label);
  testing::internal::CaptureStdout();
  wkt::toc_us(label);
  output = Trim(testing::internal::GetCapturedStdout());
  EXPECT_TRUE(StartsWith(output, label));
  EXPECT_TRUE(EndsWith(output, " us"));

  // Output nanoseconds
  testing::internal::CaptureStdout();
  wkt::toc_ns(label);
  output = Trim(testing::internal::GetCapturedStdout());
  EXPECT_TRUE(StartsWith(output, label));
  EXPECT_TRUE(EndsWith(output, " ns"));
}


TEST(TicTocTest, TocMuted) {
  wkt::tic();
  // By default, we should see some output
  testing::internal::CaptureStdout();
  wkt::toc_sec();
  std::string output = Trim(testing::internal::GetCapturedStdout());
  EXPECT_TRUE(StartsWith(output, "Elapsed time: "));
  EXPECT_TRUE(EndsWith(output, " sec"));

  wkt::mute_toc();
  testing::internal::CaptureStdout();
  wkt::toc_sec();
  output = testing::internal::GetCapturedStdout();
  EXPECT_EQ(output.length(), 0);

  wkt::unmute_toc();
  testing::internal::CaptureStdout();
  wkt::toc_sec();
  output = Trim(testing::internal::GetCapturedStdout());
  EXPECT_TRUE(StartsWith(output, "Elapsed time: "));
  EXPECT_TRUE(EndsWith(output, " sec"));
}


TEST(TicTocTest, TocFormat) {
  // Default output
  testing::internal::CaptureStdout();
  wkt::tic();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  wkt::toc_sec();
  std::string output = Trim(testing::internal::GetCapturedStdout());
  EXPECT_TRUE(StartsWith(output, "Elapsed time: 0.1"));
  EXPECT_TRUE(EndsWith(output, " sec"));

  // 2 digits after the comma
  wkt::set_toc_fmt(false, 0, 2);
  testing::internal::CaptureStdout();
  wkt::tic();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  wkt::toc_sec();
  output = Trim(testing::internal::GetCapturedStdout());
  EXPECT_TRUE(StartsWith(output, "Elapsed time: 0.1"));
  EXPECT_TRUE(EndsWith(output, " sec"));
  EXPECT_EQ(output.length(), 22);  // 2nd digit after comma might be arbitrary

  // 1 digit after the comma, number width should be 9
  wkt::set_toc_fmt(false, 9, 1);
  testing::internal::CaptureStdout();
  wkt::tic();
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  wkt::toc_sec();
  output = Trim(testing::internal::GetCapturedStdout());
  EXPECT_EQ(output, "Elapsed time:       0.2 sec");

  // 1 digit after the comma, number width should be 5, custom labels
  // should be aligned
  wkt::set_toc_fmt(true, 5, 1);
  wkt::tic();
  wkt::tic("lbl 1");
  wkt::tic("label 2");
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  testing::internal::CaptureStdout();
  wkt::toc_sec();
  output = Trim(testing::internal::GetCapturedStdout());
  testing::internal::CaptureStdout();
  wkt::toc_sec("lbl 1");
  std::string out1 = Trim(testing::internal::GetCapturedStdout());
  testing::internal::CaptureStdout();
  wkt::toc_sec("label 2");
  std::string out2 = Trim(testing::internal::GetCapturedStdout());
  EXPECT_EQ(output, "Elapsed time:   0.2 sec");
  EXPECT_EQ(out1, "lbl 1:     0.2 sec");
  EXPECT_EQ(out2, "label 2:   0.2 sec");

  // Querying an unknown label should throw an exception, for both toc...
  EXPECT_THROW(wkt::toc_ms("foo"), std::invalid_argument);
  // ... and ttoc:
  EXPECT_THROW(wkt::ttoc_us("bar"), std::invalid_argument);
}
