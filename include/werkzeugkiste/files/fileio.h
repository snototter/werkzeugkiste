#ifndef WERKZEUGKISTE_FILES_FILEIO_H
#define WERKZEUGKISTE_FILES_FILEIO_H

#include <werkzeugkiste/files/files_export.h>

#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace werkzeugkiste::files {
/// @brief Indicates an I/O error (e.g. invalid path or missing permissions).
class IOError : public std::runtime_error {
 public:
  explicit IOError(const std::string &msg) : std::runtime_error(msg) {}
};

/// @brief Reads all lines of the plain text file.
std::vector<std::string> WERKZEUGKISTE_FILES_EXPORT ReadAsciiFile(
    std::string_view filename);

/// @brief Reads the plain text file into a single string.
std::string WERKZEUGKISTE_FILES_EXPORT CatAsciiFile(std::string_view filename);

// TODO doc
// TODO test
class WERKZEUGKISTE_FILES_EXPORT AsciiFileIterator {
 public:
  // NOLINTBEGIN(readability-identifier-naming)
  using iterator_category = std::forward_iterator_tag;
  using value_type = std::string;
  using difference_type = std::ptrdiff_t;
  using pointer = value_type const *;
  using reference = value_type const &;
  // NOLINTEND(readability-identifier-naming)

  ~AsciiFileIterator();

  explicit AsciiFileIterator(std::string_view filename);

  AsciiFileIterator() = default;
  AsciiFileIterator(const AsciiFileIterator &) = delete;
  AsciiFileIterator(AsciiFileIterator &&) = delete;
  AsciiFileIterator &operator=(const AsciiFileIterator &) = delete;
  AsciiFileIterator &operator=(AsciiFileIterator &&) = delete;

  /// @brief Returns true if there are still lines left to be read from the
  /// file.
  bool HasLine() const { return !done_; }

  /// @brief Returns the currently read line.
  reference Line() const { return line_; }

  /// @brief Returns the 0-based number of the currently read line.
  std::size_t LineNumber() const { return line_number_; }

  /// @brief Returns the currently read line. Overloaded for convenience.
  reference operator*() const { return Line(); }

  /// @brief Returns a const pointer to the currently read line.
  pointer operator->() const { return &line_; }

  /// @brief Advances this iterator to the next line and returns the read line.
  reference Next();

  /// @brief Advances this iterator to the next line.
  AsciiFileIterator &operator++();

 private:
  /// @brief Input file stream.
  std::ifstream ifs_{};

  /// @brief Indicates whether there are lines left to be read.
  bool done_{true};

  /// @brief Holds the most recently read line.
  std::string line_{};

  /// @brief Holds the 0-based line number of the currently read line.
  std::size_t line_number_{0};
};

}  // namespace werkzeugkiste::files

#endif  // WERKZEUGKISTE_FILES_FILEIO_H
