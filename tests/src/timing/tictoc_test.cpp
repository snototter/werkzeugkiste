#include <chrono>
#include <exception>
#include <string>
#include <thread>

#include <werkzeugkiste/strings/strings.h>
#include <werkzeugkiste/timing/tictoc.h>

#include "../test_utils.h"

namespace wkt = werkzeugkiste::timing;
namespace wks = werkzeugkiste::strings;

// NOLINTBEGIN

TEST(TicTocTest, ElapsedTimes)
{
  wkt::Tic();
  std::this_thread::sleep_for(std::chrono::milliseconds(120));
  double elapsed = wkt::TTocSeconds();
  EXPECT_TRUE(CheckElapsedTime(elapsed, 0.12, 0.002));

  wkt::Tic();
  std::this_thread::sleep_for(std::chrono::milliseconds(120));
  elapsed = wkt::TTocMilliseconds();
  EXPECT_TRUE(CheckElapsedTime(elapsed, 120, 2));

  wkt::Tic();
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  elapsed = wkt::TTocMicroseconds();
  EXPECT_TRUE(CheckElapsedTime(elapsed, 50000, 2000));

  wkt::Tic();
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  elapsed = wkt::TTocNanoseconds();
  EXPECT_TRUE(CheckElapsedTime(elapsed, 50000000, 2000000));
}

TEST(TicTocTest, TocOutput)
{
  wkt::Tic();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Output seconds
  testing::internal::CaptureStdout();
  wkt::TocSeconds();
  std::string output = wks::Trim(testing::internal::GetCapturedStdout());
  EXPECT_TRUE(wks::StartsWith(output, "Elapsed time: 0.1"));
  EXPECT_TRUE(wks::EndsWith(output, " sec"));

  // Output milliseconds
  testing::internal::CaptureStdout();
  wkt::TocMilliseconds();
  output = wks::Trim(testing::internal::GetCapturedStdout());
  EXPECT_TRUE(wks::StartsWith(output, "Elapsed time: "));
  EXPECT_TRUE(wks::EndsWith(output, " ms"));

  // Output microseconds
  const std::string label("wat!ch");
  wkt::Tic(label);
  testing::internal::CaptureStdout();
  wkt::TocMicroseconds(label);
  output = wks::Trim(testing::internal::GetCapturedStdout());
  EXPECT_TRUE(wks::StartsWith(output, label));
  EXPECT_TRUE(wks::EndsWith(output, " us"));

  // Output nanoseconds
  testing::internal::CaptureStdout();
  wkt::TocNanoseconds(label);
  output = wks::Trim(testing::internal::GetCapturedStdout());
  EXPECT_TRUE(wks::StartsWith(output, label));
  EXPECT_TRUE(wks::EndsWith(output, " ns"));
}

TEST(TicTocTest, TocMuted)
{
  wkt::Tic();
  // By default, we should see some output
  testing::internal::CaptureStdout();
  wkt::TocSeconds();
  std::string output = wks::Trim(testing::internal::GetCapturedStdout());
  EXPECT_TRUE(wks::StartsWith(output, "Elapsed time: "));
  EXPECT_TRUE(wks::EndsWith(output, " sec"));

  wkt::MuteToc();
  testing::internal::CaptureStdout();
  wkt::TocSeconds();
  output = testing::internal::GetCapturedStdout();
  EXPECT_EQ(output.length(), 0);

  wkt::UnmuteToc();
  testing::internal::CaptureStdout();
  wkt::TocSeconds();
  output = wks::Trim(testing::internal::GetCapturedStdout());
  EXPECT_TRUE(wks::StartsWith(output, "Elapsed time: "));
  EXPECT_TRUE(wks::EndsWith(output, " sec"));
}

TEST(TicTocTest, TocFormat)
{
  // Default output
  testing::internal::CaptureStdout();
  wkt::Tic();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  wkt::TocSeconds();
  std::string output = wks::Trim(testing::internal::GetCapturedStdout());
  EXPECT_TRUE(wks::StartsWith(output, "Elapsed time: 0.1"));
  EXPECT_TRUE(wks::EndsWith(output, " sec"));

  // 2 digits after the comma
  wkt::SetTocFormat(false, 0, 2);
  testing::internal::CaptureStdout();
  wkt::Tic();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  wkt::TocSeconds();
  output = wks::Trim(testing::internal::GetCapturedStdout());
  EXPECT_TRUE(wks::StartsWith(output, "Elapsed time: 0.1"));
  EXPECT_TRUE(wks::EndsWith(output, " sec"));
  EXPECT_EQ(output.length(), 22);  // 2nd digit after comma might be arbitrary

  // 1 digit after the comma, number width should be 9
  wkt::SetTocFormat(false, 9, 1);
  testing::internal::CaptureStdout();
  wkt::Tic();
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  wkt::TocSeconds();
  output = wks::Trim(testing::internal::GetCapturedStdout());
  EXPECT_EQ(output, "Elapsed time:       0.2 sec");

  // 1 digit after the comma, number width should be 5, custom labels
  // should be aligned
  wkt::SetTocFormat(true, 5, 1);
  wkt::Tic();
  wkt::Tic("lbl 1");
  wkt::Tic("label 2");
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  testing::internal::CaptureStdout();
  wkt::TocSeconds();
  output = wks::Trim(testing::internal::GetCapturedStdout());
  testing::internal::CaptureStdout();
  wkt::TocSeconds("lbl 1");
  std::string out1 = wks::Trim(testing::internal::GetCapturedStdout());
  testing::internal::CaptureStdout();
  wkt::TocSeconds("label 2");
  std::string out2 = wks::Trim(testing::internal::GetCapturedStdout());
  EXPECT_EQ(output, "Elapsed time:   0.2 sec");
  EXPECT_EQ(out1, "lbl 1:     0.2 sec");
  EXPECT_EQ(out2, "label 2:   0.2 sec");

  // Querying an unknown label should throw an exception, for both toc...
  EXPECT_THROW(wkt::TocMilliseconds("foo"), std::invalid_argument);
  EXPECT_THROW(wkt::TocMicroseconds("foo"), std::invalid_argument);
  // ... and ttoc:
  EXPECT_THROW(wkt::TTocMilliseconds("bar"), std::invalid_argument);
  EXPECT_THROW(wkt::TTocMicroseconds("bar"), std::invalid_argument);
}

// NOLINTEND
