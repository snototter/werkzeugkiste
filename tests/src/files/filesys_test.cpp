#include <werkzeugkiste/files/filesys.h>

#include <string>
#include <string_view>

#include "../test_utils.h"

namespace wkf = werkzeugkiste::files;

// NOLINTBEGIN

TEST(FileSystemTest, Exists) {
  EXPECT_TRUE(wkf::Exists(__FILE__));
  EXPECT_FALSE(wkf::IsDir(__FILE__));

  const std::string dirname = wkf::DirName(__FILE__);
  EXPECT_TRUE(wkf::Exists(dirname));
  EXPECT_TRUE(wkf::IsDir(dirname));
}

TEST(FileSystemTest, Paths) {
  using namespace std::string_view_literals;
  const std::string fullpath1 = wkf::FullFile({"foo"sv, "bar"sv, "f.ext"sv});
  const std::string fullpath2 = wkf::FullFile(
      {std::string("foo"), std::string("bar"), std::string("f.ext")});
  EXPECT_EQ(fullpath1, fullpath2);

#if defined(__linux__) || defined(__unix__)
  EXPECT_EQ("/", wkf::Parent("foo"sv));
  EXPECT_EQ(".", wkf::Parent("./foo"sv));
  EXPECT_EQ("foo/bar", wkf::Parent("foo/bar/f.ext"sv));
  // TODO extend test case
#endif
}

TEST(FileSystemTest, TODO) {
  //  EXPECT_TRUE(wks::EndsWith("Some string", "string"));
  //  EXPECT_FALSE(wks::EndsWith("Some string", "String"));  // case mismatch
  //  EXPECT_TRUE(wks::EndsWith("Some string", "ing"));
  //  EXPECT_TRUE(wks::EndsWith("Some string", "g"));  // single character
  //  string EXPECT_TRUE(wks::EndsWith("Some string", 'g'));  // character
  //  EXPECT_FALSE(wks::EndsWith("Some string", 'G'));  // character
  //  EXPECT_FALSE(wks::EndsWith("Some string", ""));  // empty string
  //  EXPECT_FALSE(wks::EndsWith("", ""));
  //  EXPECT_FALSE(wks::EndsWith("", "st"));
}

// NOLINTEND
