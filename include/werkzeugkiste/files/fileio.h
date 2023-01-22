#ifndef WERKZEUGKISTE_FILES_FILEIO_H
#define WERKZEUGKISTE_FILES_FILEIO_H

#include <string>
#include <vector>

namespace werkzeugkiste
{
/// TODO doc
namespace files
{
/// Reads the plain text file into a string.
std::vector<std::string> ReadAsciiFile(const char* filename);
std::string CatAsciiFile(const char* filename);

}  // namespace files
}  // namespace werkzeugkiste

#endif  // WERKZEUGKISTE_FILES_FILEIO_H
