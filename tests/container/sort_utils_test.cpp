#include <chrono>
#include <thread>
#include <string>
#include <map>
#include <vector>
#include <array>
#include <list>


#include <gtest/gtest.h>

#include <werkzeugkiste/container/sort.h>

namespace wkc = werkzeugkiste::container;

TEST(SortUtilsTest, MapKeys) {
  std::map<int, int> m1{{0, -1}, {17, -3}, {42, 9}, {-100, 3}};
  auto k1 = wkc::GetMapKeys(m1);
  EXPECT_EQ(m1.size(), 4);
  EXPECT_EQ(k1.size(), 4);
  EXPECT_TRUE(wkc::ContainsKey(m1, 0));
  EXPECT_TRUE(wkc::ContainsKey(m1, 17));
  EXPECT_TRUE(wkc::ContainsKey(m1, 42));
  EXPECT_TRUE(wkc::ContainsKey(m1, -100));
  EXPECT_FALSE(wkc::ContainsKey(m1, -1));

  std::map<std::string, int> m2{{"foo", -1}, {"Bar", 10}, {"A B C", 1}};
  auto k2 = wkc::GetMapKeys(m2);
  EXPECT_EQ(m2.size(), 3);
  EXPECT_EQ(k2.size(), 3);
  EXPECT_TRUE(wkc::ContainsKey(m2, "foo"));
  EXPECT_TRUE(wkc::ContainsKey(m2, "Bar"));
  EXPECT_TRUE(wkc::ContainsKey(m2, "A B C"));
  EXPECT_FALSE(wkc::ContainsKey(m2, "Foo"));
  EXPECT_FALSE(wkc::ContainsKey(m2, "bar"));


  std::map<std::pair<int, std::string>,
      int> m3{{{1, "foo"}, -1}, {{99, "Bar"}, 10},
              {{2, "foo"}, 0}};
  auto k3 = wkc::GetMapKeys(m3);
  EXPECT_EQ(m3.size(), 3);
  EXPECT_EQ(k3.size(), 3);
  EXPECT_TRUE(wkc::ContainsKey(m3, std::make_pair(1, "foo")));
  EXPECT_TRUE(wkc::ContainsKey(m3, std::make_pair(2, "foo")));
  EXPECT_FALSE(wkc::ContainsKey(m3, std::make_pair(3, "foo")));
  EXPECT_FALSE(wkc::ContainsKey(m3, std::make_pair(1, "Foo")));
  EXPECT_TRUE(wkc::ContainsKey(m3, std::make_pair(99, "Bar")));
  EXPECT_FALSE(wkc::ContainsKey(m3, std::make_pair(98, "Bar")));
  EXPECT_FALSE(wkc::ContainsKey(m3, std::make_pair(99, "bar")));
}


TEST(SortUtilsTest, Contains) {
  std::map<int, int> m1{{0, -1}, {17, -3}, {42, 9}, {-100, 3}};
  EXPECT_TRUE(wkc::ContainsKey(m1, 0));
  EXPECT_TRUE(wkc::ContainsKey(m1, 17));
  EXPECT_TRUE(wkc::ContainsKey(m1, 42));
  EXPECT_TRUE(wkc::ContainsKey(m1, -100));
  EXPECT_FALSE(wkc::ContainsKey(m1, -1));
  EXPECT_FALSE(wkc::ContainsKey(m1, 3));

  std::map<std::string, int> m2{{"foo", -1}, {"Bar", 10}, {"A B C", 1}};
  auto k2 = wkc::GetMapKeys(m2);
  EXPECT_EQ(m2.size(), 3);
  EXPECT_EQ(k2.size(), 3);
  EXPECT_TRUE(wkc::ContainsKey(m2, "foo"));
  EXPECT_TRUE(wkc::ContainsKey(m2, "Bar"));
  EXPECT_TRUE(wkc::ContainsKey(m2, "A B C"));
  EXPECT_FALSE(wkc::ContainsKey(m2, "Foo"));
  EXPECT_FALSE(wkc::ContainsKey(m2, "bar"));


  std::map<std::pair<int, std::string>,
      int> m3{{{1, "foo"}, -1}, {{99, "Bar"}, 10},
              {{2, "foo"}, 0}};
  auto k3 = wkc::GetMapKeys(m3);
  EXPECT_EQ(m3.size(), 3);
  EXPECT_EQ(k3.size(), 3);
  EXPECT_TRUE(wkc::ContainsKey(m3, std::make_pair(1, "foo")));
  EXPECT_TRUE(wkc::ContainsKey(m3, std::make_pair(2, "foo")));
  EXPECT_FALSE(wkc::ContainsKey(m3, std::make_pair(3, "foo")));
  EXPECT_FALSE(wkc::ContainsKey(m3, std::make_pair(1, "Foo")));
  EXPECT_TRUE(wkc::ContainsKey(m3, std::make_pair(99, "Bar")));
  EXPECT_FALSE(wkc::ContainsKey(m3, std::make_pair(98, "Bar")));
  EXPECT_FALSE(wkc::ContainsKey(m3, std::make_pair(99, "bar")));
}


TEST(SortUtilsTest, Duplicates) {
  const std::vector<int> c1{-3, 0, 10, 17, 0, 4, 6, -3, 32};
  EXPECT_FALSE(wkc::HasUniqueItems(c1));
  EXPECT_TRUE(wkc::ContainsValue(c1, 4));
  EXPECT_TRUE(wkc::ContainsValue(c1, -3));
  EXPECT_FALSE(wkc::ContainsValue(c1, 99));

  auto dup1 = wkc::FindDuplicates(c1);
  EXPECT_TRUE(wkc::HasUniqueItems(dup1));
  EXPECT_EQ(dup1.size(), 2);
  EXPECT_TRUE(wkc::ContainsKey(dup1, -3));
  EXPECT_TRUE(wkc::ContainsKey(dup1, 0));

  const std::vector<std::string> c2{
    "0", "", "10", "foo", "0", "Foo", "foo", "foo", "", "-3"};
  EXPECT_FALSE(wkc::HasUniqueItems(c2));
  auto dup2 = wkc::FindDuplicates(c2);
  EXPECT_TRUE(wkc::HasUniqueItems(dup2));
  EXPECT_EQ(dup2.size(), 3);
  EXPECT_TRUE(wkc::ContainsKey(dup2, ""));
  EXPECT_TRUE(wkc::ContainsKey(dup2, "0"));
  EXPECT_TRUE(wkc::ContainsKey(dup2, "foo"));
  EXPECT_EQ(dup2[""], 2);
  EXPECT_EQ(dup2["0"], 2);
  EXPECT_EQ(dup2["foo"], 3);


  const std::array<short, 7> c3 = {-3, 156, 2, 17, -3, 9, 8};
  EXPECT_FALSE(wkc::HasUniqueItems(c3));
  auto dup3 = wkc::FindDuplicates(c3);
  EXPECT_TRUE(wkc::HasUniqueItems(dup3));
  EXPECT_EQ(dup3.size(), 1);
  EXPECT_TRUE(wkc::ContainsKey(dup3, -3));
  EXPECT_FALSE(wkc::ContainsKey(dup3, 3));
  EXPECT_EQ(dup3[-3], 2);


  const std::list<std::string> c4{
    "bcd", "ABC", "foo", "3", "bar", "bce", "bcd"};
  EXPECT_FALSE(wkc::HasUniqueItems(c4));
  auto dup4 = wkc::FindDuplicates(c4);
  EXPECT_TRUE(wkc::HasUniqueItems(dup4));
  EXPECT_EQ(dup4.size(), 1);
  EXPECT_TRUE(wkc::ContainsKey(dup4, "bcd"));
  EXPECT_FALSE(wkc::ContainsKey(dup4, "foo"));
  EXPECT_EQ(dup4["bcd"], 2);


  const std::vector<int> c5{};
  EXPECT_TRUE(wkc::HasUniqueItems(c5));

  const std::vector<int> c6{-1, 0, 1};
  EXPECT_TRUE(wkc::HasUniqueItems(c6));

}
