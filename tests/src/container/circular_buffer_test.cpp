#include <chrono>
#include <thread>
#include <string>
#include <map>
#include <vector>
#include <array>
#include <list>

#include <werkzeugkiste/container/sort.h>
#include <werkzeugkiste/container/circular_buffer.h>
#include <werkzeugkiste/container/math.h>

#include "../test_utils.h"

namespace wkc = werkzeugkiste::container;

// NOLINTBEGIN

TEST(ContainerUtilsTest, CircularBuffer) {
// You can either use:
 //circular_buffer<int> buffer(3);
 wkc::circular_buffer<int,3> buffer;

 EXPECT_TRUE(buffer.empty());
 EXPECT_EQ(buffer.capacity(), 3);
 buffer.push_back(0);
 EXPECT_FALSE(buffer.empty());
 EXPECT_EQ(buffer.capacity(), 3);
 EXPECT_EQ(buffer.size(), 1);

 buffer.push_back(1);
 buffer.push_back(2);
 // 0, 1, 2
 EXPECT_EQ(buffer[0], 0);
 EXPECT_EQ(buffer[1], 1);
 EXPECT_EQ(buffer[2], 2);

 // 3, 1, 2 => order: (F)1, 2, (B)3
 buffer.push_back(3);
 EXPECT_EQ(buffer[0], 1);
 EXPECT_EQ(buffer[1], 2);
 EXPECT_EQ(buffer[2], 3);

 EXPECT_EQ(buffer.front(), 1);
 EXPECT_EQ(buffer.back(), 3);

 buffer.pop_front();
 // (b)3, x, (f)2
 EXPECT_EQ(buffer.size(), 2);
 EXPECT_EQ(buffer[0], 2);
 EXPECT_EQ(buffer[1], 3);
 EXPECT_EQ(buffer.front(), 2);
 EXPECT_EQ(buffer.back(), 3);

 buffer.push_back(5);
 // 3, (b)5, (f)2
 EXPECT_EQ(buffer[0], 2);
 EXPECT_EQ(buffer[1], 3);
 EXPECT_EQ(buffer[2], 5);
 EXPECT_EQ(buffer.front(), 2);
 EXPECT_EQ(buffer.back(), 5);

 // Reset buffer
 buffer.clear();
 EXPECT_TRUE(buffer.empty());
 EXPECT_EQ(buffer.size(), 0);
 EXPECT_EQ(buffer.capacity(), 3);

 buffer.push_back(23);
 buffer.push_back(42);
 buffer.push_back(77);
 EXPECT_EQ(buffer[0], 23);
 EXPECT_EQ(buffer[1], 42);
 EXPECT_EQ(buffer[2], 77);

 EXPECT_EQ(buffer.front(), 23);
 buffer.pop_front();
 EXPECT_EQ(buffer.front(), 42);
 buffer.pop_front();
 EXPECT_EQ(buffer.front(), 77);
 buffer.pop_front();
 EXPECT_TRUE(buffer.empty());

 buffer.push_back(123);
 buffer.push_back(234);
 buffer.push_back(345);
 EXPECT_EQ(buffer[0], 123);
 EXPECT_EQ(buffer[1], 234);
 EXPECT_EQ(buffer[2], 345);
 EXPECT_EQ(buffer.back(), 345);
 buffer.pop_back();
 EXPECT_EQ(buffer.back(), 234);
 buffer.pop_back();
 EXPECT_EQ(buffer.back(), 123);
 buffer.pop_back();
 EXPECT_TRUE(buffer.empty());
}

//TODO test buffer with complex type (memory allocation and cleanup!) --> create a mock class instead
//#ifdef WITH_OPENCV
//TEST(CircularBuffer, Mat) {
// circular_buffer<cv::Mat> cb(2);
// cv::Mat tmp1 = cv::Mat::zeros(10, 20, CV_8UC3);
// cv::Mat tmp2 = tmp1.clone();
// cv::Mat tmp3 = cv::Mat::zeros(30,75, CV_8UC1);

// cb.push_back(tmp1);
// cb.push_back(tmp2);
// cb.push_back(tmp3);
// EXPECT_EQ(cb.size(), cb.capacity());

// EXPECT_EQ(tmp3.cols, cb.back().cols);
// EXPECT_EQ(tmp2.cols, cb.front().cols);
//}

//typedef struct {
// size_t id;
// cv::Mat mat;
// std::vector<size_t> vec;
//} mat_struct;

//TEST(CircularBuffer, MatStruct) {
// circular_buffer<mat_struct> cb(2);

// cv::Mat tmp1 = cv::Mat::zeros(10, 20, CV_8UC3);
// cv::Mat tmp2 = tmp1.clone();
// cv::Mat tmp3 = cv::Mat::zeros(30,75, CV_8UC1);

// mat_struct s1, s2, s3;
// s1.id = 1;
// tmp1.copyTo(s1.mat);
// s1.vec.push_back(1);
// s2.id = 2;
// tmp2.copyTo(s2.mat);
// s2.vec.push_back(2);
// s3.id = 3;
// s3.mat = tmp3.clone();
// s3.vec.push_back(3);

// cb.push_back(s1);
// cb.push_back(s2);
// EXPECT_EQ(cb.size(), cb.capacity());

// cb[0].id = 23;
// tmp3.copyTo(cb[0].mat);
// EXPECT_EQ(cb.size(), cb.capacity());
// EXPECT_EQ(cb[0].id, 23);
// EXPECT_EQ(cb[0].mat.rows, tmp3.rows);
// EXPECT_EQ(cb[0].mat.cols, tmp3.cols);

// cb.push_back(s3);
// EXPECT_EQ(cb.size(), cb.capacity());
// EXPECT_EQ(cb[0].id, 2);
// EXPECT_EQ(cb[0].mat.rows, tmp2.rows);
// EXPECT_EQ(cb[0].mat.cols, tmp2.cols);
// EXPECT_EQ(cb[1].mat.rows, tmp3.rows);
// EXPECT_EQ(cb[1].mat.cols, tmp3.cols);
// EXPECT_EQ(cb[1].id, 3);

// cb.pop_front();
// EXPECT_EQ(cb.size(), 1);
// EXPECT_EQ(cb[0].id, 3);
//}
//#endif


TEST(ContainerUtilsTest, Iterators) {
 using cb = wkc::circular_buffer<int>;
 cb buffer(3);

 cb::iterator it = buffer.begin();
 EXPECT_EQ(it, buffer.end());

 buffer.push_back(1);
 buffer.push_back(2);

 // Mutable fwd iterator using prefix increment.
 int val = 1;
 for (it = buffer.begin(); it != buffer.end(); ++it, ++val) {
   EXPECT_EQ(*it, val);
   EXPECT_EQ(buffer[val - 1], val);

   *it = val - 1;
   EXPECT_EQ(*it, val - 1);
   EXPECT_EQ(buffer[val - 1], val - 1);
 }
 EXPECT_NO_THROW(buffer[2]);
 EXPECT_THROW(buffer.at(2), std::out_of_range);
 EXPECT_EQ(buffer[0], 0);
 EXPECT_EQ(buffer[1], 1);

 // Mutable fwd iterator using postfix increment.
 val = 0;
 for (it = buffer.begin(); it != buffer.end(); it++, val++) {
   EXPECT_EQ(*it, val);
   EXPECT_EQ(buffer[val], val);

   *it = val + 1;
   EXPECT_EQ(*it, val + 1);
   EXPECT_EQ(buffer[val], val + 1);
 }
 EXPECT_EQ(buffer[0], 1);
 EXPECT_EQ(buffer[1], 2);

 // const_iterator using cbegin/cend.
 val = 1;
 for (cb::const_iterator cit = buffer.cbegin(); cit != buffer.cend(); ++cit) {
   EXPECT_EQ(*cit, val++);
   *it = 3;
 }

 // const_iterator using begin/end.
 val = 1;
 for (cb::const_iterator cit = buffer.begin(); cit != buffer.end(); ++cit) {
   EXPECT_EQ(*cit, val++);
 }
 // foreach syntax.
 val = 1;
 for (const auto &elem : buffer)
   EXPECT_EQ(elem, val++);

 // Replace oldest element.
 buffer.push_back(3);
 buffer.push_back(4);
 // Standard fwd iterator.
 val = 2;
 for (it = buffer.begin(); it != buffer.end(); ++it) {
   EXPECT_EQ(*it, val++);
 }
 // const_iterator using cbegin/cend.
 val = 2;
 for (cb::const_iterator cit = buffer.cbegin(); cit != buffer.cend(); ++cit) {
   EXPECT_EQ(*cit, val++);
 }
 // foreach syntax.
 val = 2;
 for (const auto &elem : buffer) {
   EXPECT_EQ(elem, val++);
 }

 // Reverse iteration using prefix decrement.
 val = 4;
 for (it = buffer.end(); --it != buffer.begin(); /* do nothing */) {
   EXPECT_EQ(*it, val--);
 }
 // Reverse iteration using postfix decrement.
 val = 4;
 for (it = buffer.end(); it-- != buffer.begin(); /* do nothing */) {
   EXPECT_EQ(*it, val--);
 }
}


TEST(ContainerUtilsTest, PopIterators) {
  using cb = wkc::circular_buffer<int>;
  cb buffer(5);
  buffer.push_back(1);
  buffer.push_back(2);
  buffer.push_back(3);
  buffer.push_back(4);
  buffer.push_back(5);
  buffer.push_back(6);
  buffer.push_back(7);
  EXPECT_EQ(buffer.size(), 5);

  int val = 7;
  for (auto it = buffer.end(); it-- != buffer.begin(); ) {
    EXPECT_EQ(*it, val--);
  }

  val = 3;
  for (auto it = buffer.begin(); it != buffer.end(); ++it) {
   EXPECT_EQ(*it, val++);
  }

  buffer.pop_back();
  EXPECT_EQ(buffer.size(), 4);
  val = 6;
  for (auto it = buffer.end(); it-- != buffer.begin(); ) {
    EXPECT_EQ(*it, val--);
  }
  EXPECT_EQ(val, 2);

  val = 3;
  for (auto it = buffer.begin(); it != buffer.end(); ++it) {
   EXPECT_EQ(*it, val++);
  }

  buffer.pop_front();
  EXPECT_EQ(buffer.size(), 3);
  val = 6;
  for (auto it = buffer.end(); it-- != buffer.begin(); ) {
    EXPECT_EQ(*it, val--);
  }
  EXPECT_EQ(val, 3);
  val = 4;
  for (auto it = buffer.begin(); it != buffer.end(); ++it) {
   EXPECT_EQ(*it, val++);
  }

  // Pop another one and we should be left with [x x x 4 5].
  buffer.pop_back();
  EXPECT_EQ(buffer.size(), 2);
  auto rit = --(buffer.end());
  EXPECT_EQ(*rit, 5);
  auto it = buffer.begin();
  EXPECT_EQ(*it, 4);
  EXPECT_EQ(buffer.size(), 2);

  buffer.pop_front();
  it = buffer.begin();
  EXPECT_EQ(*it, 5);
  rit = --(buffer.end());
  EXPECT_EQ(*rit, 5);
  EXPECT_EQ(it, rit);
  EXPECT_EQ(*it, *rit);
  EXPECT_EQ(buffer.size(), 1);

  buffer.pop_back();
  it = buffer.begin();
  EXPECT_EQ(it, buffer.end());
  EXPECT_TRUE(buffer.empty());
  EXPECT_EQ(buffer.size(), 0);

  buffer.push_back(23);

  rit = --(buffer.end());
  EXPECT_EQ(*rit, 23);
  it = buffer.begin();
  EXPECT_EQ(*it, 23);
}


TEST(ContainerUtilsTest, CircularBufferSort) {
  using cb = wkc::circular_buffer<int>;
  cb buffer(5);

  buffer.push_back(17);
  buffer.push_back(99);
  buffer.push_back(1);

  std::vector<std::size_t> indices = wkc::GetSortedIndices(buffer, wkc::CmpAsc<int>);
  EXPECT_EQ(indices.size(), 3);
  EXPECT_EQ(indices[0], 2);
  EXPECT_EQ(indices[1], 0);
  EXPECT_EQ(indices[2], 1);

  // Assert that the original buffer didn't change
  EXPECT_EQ(buffer[0], 17);
  EXPECT_EQ(buffer[1], 99);
  EXPECT_EQ(buffer[2], 1);

  indices = wkc::GetSortedIndices(buffer, wkc::CmpDesc<int>);
  EXPECT_EQ(indices.size(), 3);
  EXPECT_EQ(indices[0], 1);
  EXPECT_EQ(indices[1], 0);
  EXPECT_EQ(indices[2], 2);

  // Assert that the original buffer didn't change
  EXPECT_EQ(buffer[0], 17);
  EXPECT_EQ(buffer[1], 99);
  EXPECT_EQ(buffer[2], 1);

  cb remapped = wkc::ApplyIndexLookup(buffer, indices);
  EXPECT_EQ(remapped.size(), 3);
  EXPECT_EQ(remapped[0], 99);
  EXPECT_EQ(remapped[1], 17);
  EXPECT_EQ(remapped[2], 1);

  // Assert that the original buffer didn't change
  EXPECT_EQ(buffer[0], 17);
  EXPECT_EQ(buffer[1], 99);
  EXPECT_EQ(buffer[2], 1);

  buffer.push_back(0);
  buffer.push_back(55);
  buffer.push_back(101);

  EXPECT_EQ(buffer.size(), 5);
  indices = wkc::GetSortedIndices(buffer, wkc::CmpDesc<int>);
  EXPECT_EQ(indices.size(), 5);
  EXPECT_EQ(indices[0], 4);
  EXPECT_EQ(indices[1], 0);
  EXPECT_EQ(indices[2], 3);
  EXPECT_EQ(indices[3], 1);
  EXPECT_EQ(indices[4], 2);
}

// NOLINTEND
