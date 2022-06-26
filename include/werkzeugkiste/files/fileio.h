#ifndef __WERKZEUGKISTE_FILES_FILEIO_H__
#define __WERKZEUGKISTE_FILES_FILEIO_H__

#include <string>
#include <vector>

namespace werkzeugkiste {
/// TODO doc
namespace files {
/// Reads the plain text file into a string.
std::vector<std::string> ReadAsciiFile(const char *filename);
std::string CatAsciiFile(const char *filename);

} // namespace files
} // namespace werkzeugkiste

#endif // __WERKZEUGKISTE_FILES_FILEIO_H__
