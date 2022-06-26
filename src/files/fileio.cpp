#include <fstream>
#include <sstream>
#include <exception>
#include <werkzeugkiste/files/fileio.h>

namespace werkzeugkiste {
namespace files {

std::vector<std::string> ReadAsciiFile(const char *filename) {
  std::ifstream ifs(filename, std::ios::in);
  if (!ifs.is_open()) {
    std::string s("Cannot open file - check path: ");
    s += filename;
    throw std::logic_error(s);
  }

  std::vector<std::string> lines;
  std::string line;
  while (std::getline(ifs, line)) {
    lines.push_back(line);
  }
  ifs.close();
  return lines;
}


std::string CatAsciiFile(const char *filename) {
  std::ifstream ifs(filename, std::ios::in);
  if (!ifs.is_open()) {
    std::string s("Cannot open file - check path: ");
    s += filename;
    throw std::logic_error(s);
  }

  std::stringstream sstr;
  sstr << ifs.rdbuf();
  ifs.close();
  return sstr.str();
}

} // namespace files
} // namespace werkzeugkiste
