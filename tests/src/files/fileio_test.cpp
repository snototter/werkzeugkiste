#include <werkzeugkiste/files/fileio.h>
#include <werkzeugkiste/strings/strings.h>

#include <filesystem>
#include <string>

namespace wkf = werkzeugkiste::files;
namespace wks = werkzeugkiste::strings;

// These are dummy comments.
// Simply to ensure that this file has 42 lines.
// So the test uses a proper magic number ;)
TEST(FileIOTest, ReadFile) {
  EXPECT_THROW(wkf::CatAsciiFile("no-such-file"), std::invalid_argument);
  EXPECT_THROW(wkf::ReadAsciiFile("no-such-file"), std::invalid_argument);

  std::string content = wks::RTrim(wkf::CatAsciiFile(__FILE__));
  auto lines = wkf::ReadAsciiFile(__FILE__);
  auto concatenated = wks::RTrim(wks::Concatenate(lines, "\n"));
  EXPECT_EQ(content.length(), concatenated.length());
  EXPECT_EQ(content, concatenated);

  EXPECT_EQ(42, lines.size());
}

TEST(FileIOTest, Iterator) {
  EXPECT_THROW(wkf::AsciiFileIterator("no-such-file"), std::invalid_argument);

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
