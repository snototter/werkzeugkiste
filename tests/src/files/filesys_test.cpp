#include <werkzeugkiste/files/filesys.h>

#include <string>
#include <string_view>

#include "../test_utils.h"

namespace wkf = werkzeugkiste::files;

// NOLINTBEGIN

TEST(FileSystemTest, Exists) {
  EXPECT_TRUE(wkf::Exists(__FILE__));
  EXPECT_FALSE(wkf::Exists("no-such-file"));

  EXPECT_FALSE(wkf::IsDir(__FILE__));

  const std::string dirname = wkf::DirName(__FILE__);
  EXPECT_EQ(dirname, wkf::DirName(dirname));
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

TEST(FileSystemTest, FileParts) {
  using namespace std::string_view_literals;
  auto bname = wkf::Basename(""sv);
  EXPECT_FALSE(bname.has_value());

  auto ext = wkf::Extension(""sv);
  EXPECT_FALSE(ext.has_value());

  bname = wkf::Basename("foo"sv);
  EXPECT_TRUE(bname.has_value());
  EXPECT_EQ("foo", bname.value());

  ext = wkf::Extension("foo"sv);
  EXPECT_FALSE(ext.has_value());

  ext = wkf::Extension("archive.tar.gz"sv);
  EXPECT_TRUE(ext.has_value());
  EXPECT_EQ(".gz", ext.value());

#if defined(__linux__) || defined(__unix__)
  bname = wkf::Basename("/foo/bar"sv);
  EXPECT_EQ("bar", bname.value());

  ext = wkf::Extension("/foo/bar"sv);
  EXPECT_FALSE(ext.has_value());

  bname = wkf::Basename("/foo/bar/test.txt"sv);
  EXPECT_EQ("test.txt", bname.value());

  ext = wkf::Extension("/foo/bar/test.txt"sv);
  EXPECT_TRUE(ext.has_value());
  EXPECT_EQ(".txt", ext.value());
#endif
}

// NOLINTEND
