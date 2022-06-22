#include <string>
#include <map>
#include <vector>
#include <array>
#include <list>

#include <gtest/gtest.h>

#include <werkzeugkiste/container/circular_buffer.h>
#include <werkzeugkiste/container/math.h>

namespace wkc = werkzeugkiste::container;

TEST(ContainerUtilsTest, Smooth) {
  typedef wkc::circular_buffer<int> cb;
  cb buffer(7);

  buffer.push_back(1);
  buffer.push_back(2);
  buffer.push_back(3);
  buffer.push_back(4);
  buffer.push_back(5);
  buffer.push_back(6);
  buffer.push_back(7);

  EXPECT_THROW(wkc::SmoothMovingAverage(buffer, 1), std::invalid_argument);
  EXPECT_THROW(wkc::SmoothMovingAverage(buffer, 2), std::invalid_argument);
  EXPECT_NO_THROW(wkc::SmoothMovingAverage(buffer, 0));
  EXPECT_NO_THROW(wkc::SmoothMovingAverage(buffer, -1));

  cb smooth = wkc::SmoothMovingAverage(buffer, 3);
  EXPECT_EQ(smooth.size(), 7);
  // No smoothing at head & tail:
  EXPECT_EQ(smooth[0], buffer[0]);
  EXPECT_EQ(smooth[6], buffer[6]);
  for (std::size_t i = 1; i < 6; ++i) {
    EXPECT_EQ(smooth[i],
              (buffer[i-1] + buffer[i] + buffer[i+1]) / 3.0);
  }

  smooth = wkc::SmoothMovingAverage(buffer, 5);
  EXPECT_EQ(smooth.size(), 7);
  // No smoothing at head & tail:
  EXPECT_EQ(smooth[0], buffer[0]);
  EXPECT_EQ(smooth[6], buffer[6]);
  // Window size should increase at the head/tail:
  EXPECT_EQ(smooth[1],
            (buffer[0] + buffer[1] + buffer[2]) / 3.0);
  EXPECT_EQ(smooth[5],
            (buffer[4] + buffer[5] + buffer[6]) / 3.0);

  for (std::size_t i = 2; i < 5; ++i) {
    EXPECT_EQ(smooth[i],
              (buffer[i-2] + buffer[i-1] + buffer[i] + buffer[i+1] + + buffer[i+2]) / 5.0);
  }
}


TEST(ContainerUtilsTest, Mean) {
  typedef wkc::circular_buffer<int> cb;
  cb buffer(7);

  EXPECT_DOUBLE_EQ(wkc::Mean(buffer), 0.0);

  buffer.push_back(1);
  EXPECT_DOUBLE_EQ(wkc::Mean(buffer), 1.0);

  buffer.push_back(2);
  EXPECT_DOUBLE_EQ(wkc::Mean(buffer), 1.5);

  buffer.push_back(3);
  EXPECT_DOUBLE_EQ(wkc::Mean(buffer), 2.0);

  buffer.push_back(4);
  EXPECT_DOUBLE_EQ(wkc::Mean(buffer), 2.5);

  buffer.push_back(5);
  EXPECT_DOUBLE_EQ(wkc::Mean(buffer), 3.0);

  buffer.push_back(6);
  EXPECT_DOUBLE_EQ(wkc::Mean(buffer), 3.5);

  buffer.push_back(7);
  EXPECT_DOUBLE_EQ(wkc::Mean(buffer), 4.0);

  buffer.push_back(8);  // The first 1 dropped out
  EXPECT_DOUBLE_EQ(wkc::Mean(buffer), 5.0);
}


TEST(ContainerUtilsTest, MinMax) {
  typedef wkc::circular_buffer<int> cb;
  cb buffer(3);
  int min = 17;
  int max = 99;

  EXPECT_NO_THROW(wkc::MinMax(buffer));
  EXPECT_NO_THROW(wkc::MinMax(buffer, nullptr, &max));

  wkc::MinMax(buffer, &min, &max);
  EXPECT_EQ(min, 17);
  EXPECT_EQ(max, 99);

  buffer.push_back(1);
  wkc::MinMax(buffer, &min, &max);
  EXPECT_EQ(min, 1);
  EXPECT_EQ(max, 1);

  buffer.push_back(0);
  wkc::MinMax(buffer, &min, &max);
  EXPECT_EQ(min, 0);
  EXPECT_EQ(max, 1);

  buffer.push_back(3);
  wkc::MinMax(buffer, &min, &max);
  EXPECT_EQ(min, 0);
  EXPECT_EQ(max, 3);

  buffer.push_back(10); // The first 1 dropped out
  wkc::MinMax(buffer, &min, &max);
  EXPECT_EQ(min, 0);
  EXPECT_EQ(max, 10);

  buffer.push_back(10);
  wkc::MinMax(buffer, &min, &max);
  EXPECT_EQ(min, 3);
  EXPECT_EQ(max, 10);
}
