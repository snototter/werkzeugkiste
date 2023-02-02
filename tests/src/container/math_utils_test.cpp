#include <werkzeugkiste/container/circular_buffer.h>
#include <werkzeugkiste/container/math.h>

#include <array>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "../test_utils.h"

namespace wkc = werkzeugkiste::container;

template <typename Container>
std::string Stringify(const Container &c) {
  std::ostringstream s;
  s << "{";
  for (std::size_t idx = 0; idx < c.size(); ++idx) {
    if (idx > 0) {
      s << ", ";
    }
    s << c[idx];
  }
  s << "}";
  return s.str();
}

// NOLINTBEGIN(*magic-numbers, readability-identifier-naming)
// TODO adjust parameter orders (expected, computed_val)
TEST(ContainerUtilsTest, Smooth) {
  using cb = wkc::circular_buffer<int>;
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
  EXPECT_EQ(7, smooth.size());
  // No smoothing at head & tail:
  EXPECT_EQ(buffer[0], smooth[0]);
  EXPECT_EQ(buffer[6], smooth[6]);
  for (std::size_t i = 1; i < 6; ++i) {
    EXPECT_EQ(smooth[i], (buffer[i - 1] + buffer[i] + buffer[i + 1]) / 3.0);
  }

  smooth = wkc::SmoothMovingAverage(buffer, 5);
  EXPECT_EQ(smooth.size(), 7);
  // No smoothing at head & tail:
  EXPECT_EQ(smooth[0], buffer[0]);
  EXPECT_EQ(smooth[6], buffer[6]);
  // Window size should increase at the head/tail:
  EXPECT_EQ(smooth[1], (buffer[0] + buffer[1] + buffer[2]) / 3.0);
  EXPECT_EQ(smooth[5], (buffer[4] + buffer[5] + buffer[6]) / 3.0);

  for (std::size_t i = 2; i < 5; ++i) {
    EXPECT_EQ(smooth[i], (buffer[i - 2] + buffer[i - 1] + buffer[i] +
                          buffer[i + 1] + +buffer[i + 2]) /
                             5.0);
  }
}

TEST(ContainerUtilsTest, Mean) {
  using cb = wkc::circular_buffer<int>;
  cb buffer(7);

  EXPECT_EQ(0, wkc::Sum(buffer));
  EXPECT_DOUBLE_EQ(0.0, wkc::Mean(buffer))
      << "Buffer was: " << Stringify(buffer);

  buffer.push_back(1);
  EXPECT_EQ(1, wkc::Sum(buffer));
  EXPECT_DOUBLE_EQ(1.0, wkc::Mean(buffer))
      << "Buffer was: " << Stringify(buffer);

  buffer.push_back(2);
  EXPECT_EQ(3, wkc::Sum(buffer));
  EXPECT_DOUBLE_EQ(1.5, wkc::Mean(buffer))
      << "Buffer was: " << Stringify(buffer);

  buffer.push_back(3);
  EXPECT_EQ(6, wkc::Sum(buffer));
  EXPECT_DOUBLE_EQ(2.0, wkc::Mean(buffer))
      << "Buffer was: " << Stringify(buffer);

  buffer.push_back(4);
  EXPECT_DOUBLE_EQ(2.5, wkc::Mean(buffer))
      << "Buffer was: " << Stringify(buffer);

  buffer.push_back(5);
  EXPECT_DOUBLE_EQ(3.0, wkc::Mean(buffer))
      << "Buffer was: " << Stringify(buffer);

  buffer.push_back(6);
  EXPECT_DOUBLE_EQ(3.5, wkc::Mean(buffer))
      << "Buffer was: " << Stringify(buffer);

  buffer.push_back(7);
  EXPECT_DOUBLE_EQ(4.0, wkc::Mean(buffer))
      << "Buffer was: " << Stringify(buffer);

  buffer.push_back(8);  // The first 1 dropped out
  EXPECT_EQ(35, wkc::Sum(buffer));
  EXPECT_DOUBLE_EQ(5.0, wkc::Mean(buffer))
      << "Buffer was: " << Stringify(buffer);

  buffer.push_back(-33);
  EXPECT_EQ(0, wkc::Sum(buffer));
  EXPECT_DOUBLE_EQ(0.0, wkc::Mean(buffer))
      << "Buffer was: " << Stringify(buffer);

  buffer.push_back(-25);
  EXPECT_EQ(-28, wkc::Sum(buffer));
  EXPECT_DOUBLE_EQ(-4.0, wkc::Mean(buffer))
      << "Buffer was: " << Stringify(buffer);
}

TEST(ContainerUtilsTest, MinMax) {
  using cb = wkc::circular_buffer<int>;
  cb buffer(3);
  int min = 17;
  int max = 99;

  EXPECT_NO_THROW(wkc::MinMax(buffer));
  EXPECT_NO_THROW(wkc::MinMax(buffer, nullptr, &max));

  EXPECT_EQ(0, buffer.size());
  EXPECT_EQ(3, buffer.capacity());
  // Querying min/max/locations should not touch the parameters
  // if the container is empty
  std::size_t idx_min{};
  std::size_t idx_max{};
  wkc::MinMax(buffer, &min, &max, &idx_min, &idx_max);
  EXPECT_EQ(17, min);
  EXPECT_EQ(0, idx_min);
  EXPECT_EQ(99, max);
  EXPECT_EQ(0, idx_max);

  buffer.push_back(1);
  wkc::MinMax(buffer, &min, &max, &idx_min, &idx_max);
  EXPECT_EQ(1, min);
  EXPECT_EQ(1, max);
  EXPECT_EQ(0, idx_min);
  EXPECT_EQ(0, idx_max);

  buffer.push_back(0);
  wkc::MinMax(buffer, &min, &max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(1, max);

  buffer.push_back(3);
  wkc::MinMax(buffer, &min, &max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(3, max);

  buffer.push_back(10);  // The first 1 dropped out
  wkc::MinMax(buffer, &min, &max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(10, max);

  buffer.push_back(10);
  wkc::MinMax(buffer, &min, &max);
  EXPECT_EQ(3, min);
  EXPECT_EQ(10, max);

  buffer.push_back(9);
  buffer.push_back(-7);
  buffer.push_back(42);
  wkc::MinMax(buffer, &min, &max, &idx_min, &idx_max);
  EXPECT_EQ(-7, min);
  EXPECT_EQ(1, idx_min);
  EXPECT_EQ(42, max);
  EXPECT_EQ(2, idx_max);
}

// NOLINTEND(*magic-numbers, readability-identifier-naming)
