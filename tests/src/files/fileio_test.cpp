#include <werkzeugkiste/files/fileio.h>
#include <werkzeugkiste/strings/strings.h>

#include <string>

#include "../test_utils.h"

namespace wkf = werkzeugkiste::files;
namespace wks = werkzeugkiste::strings;

TEST(FileIOTest, ReadFile) {
  EXPECT_THROW(wkf::CatAsciiFile("no-such-file"), wkf::IOError);
  EXPECT_THROW(wkf::ReadAsciiFile("no-such-file"), wkf::IOError);

  std::string content = wks::RTrim(wkf::CatAsciiFile(__FILE__));
  auto lines = wkf::ReadAsciiFile(__FILE__);
  auto concatenated = wks::RTrim(wks::Concatenate(lines, "\n"));
  EXPECT_EQ(content.length(), concatenated.length());
  EXPECT_EQ(content, concatenated);

  // This is a dummy comment to ensure the file ends at 42 lines.
  // This makes for a nice magic number...
  EXPECT_EQ(42, lines.size());
}

TEST(FileIOTest, Iterator) {
  EXPECT_THROW(wkf::AsciiFileIterator("no-such-file"), wkf::IOError);

  std::vector<std::string> lines;
  wkf::AsciiFileIterator iterator{__FILE__};
  while (iterator.HasLine()) {
    lines.push_back(*iterator);
    ++iterator;
  }

  EXPECT_EQ(42, lines.size());

  std::string content = wks::RTrim(wkf::CatAsciiFile(__FILE__));
  auto concatenated = wks::RTrim(wks::Concatenate(lines, "\n"));
  EXPECT_EQ(content.length(), concatenated.length());
  EXPECT_EQ(content, concatenated);
}
