#ifndef WERKZEUGKISTE_FILES_FILEIO_H
#define WERKZEUGKISTE_FILES_FILEIO_H

#include <string>
#include <vector>

/// TODO doc
namespace werkzeugkiste::files {
/// Reads the plain text file into a string.
std::vector<std::string> ReadAsciiFile(const char* filename);
std::string CatAsciiFile(const char* filename);

}  // namespace werkzeugkiste::files

#endif  // WERKZEUGKISTE_FILES_FILEIO_H
