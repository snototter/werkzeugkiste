#include <werkzeugkiste/files/filesys.h>

#include <string>
#include <string_view>

#include "../test_utils.h"

namespace wkf = werkzeugkiste::files;

using namespace std::string_view_literals;

// NOLINTBEGIN

TEST(FileSystemTest, Exists) {
  EXPECT_TRUE(wkf::Exists(__FILE__));
  EXPECT_FALSE(wkf::Exists("no-such-file"sv));

  EXPECT_FALSE(wkf::IsDir(__FILE__));

  const std::string dirname = wkf::DirName(__FILE__);
  EXPECT_TRUE(wkf::Exists(dirname));
  EXPECT_TRUE(wkf::IsDir(dirname));
}

TEST(FileSystemTest, Paths) {
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

// NOLINTEND
