#ifndef WERKZEUGKISTE_FILES_FILEIO_H
#define WERKZEUGKISTE_FILES_FILEIO_H

#include <string>
#include <vector>
#include <string_view>
#include <fstream>

/// TODO doc
namespace werkzeugkiste::files {
/// Reads the plain text file into a string.
std::vector<std::string> ReadAsciiFile(const char* filename);
std::string CatAsciiFile(const char* filename);

class AsciiFileIterator {
public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = std::string;
  using difference_type = std::ptrdiff_t;
  using pointer = value_type const*;
  using reference = value_type const&;

  AsciiFileIterator() {}

  ~AsciiFileIterator();

  explicit AsciiFileIterator(std::string_view filename);

  bool HasLine() const { return !done_; }
//  explicit operator bool() const { return HasLine(); }

  reference operator*() const { return line_; }
  pointer operator->() const { return &line_; }

  reference Next();
  AsciiFileIterator &operator++();

  std::size_t LineNumber() const { return line_number_; }

private:
  std::ifstream ifs_{};
  bool done_{true};
  std::string line_{};
  std::size_t line_number_{0};
};

}  // namespace werkzeugkiste::files

#endif  // WERKZEUGKISTE_FILES_FILEIO_H
