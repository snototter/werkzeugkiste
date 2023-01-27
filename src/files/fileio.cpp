#include <werkzeugkiste/files/fileio.h>

#include <exception>
#include <fstream>
#include <sstream>

namespace werkzeugkiste {
namespace files {

std::vector<std::string> ReadAsciiFile(const char* filename) {
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

std::string CatAsciiFile(const char* filename) {
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


AsciiFileIterator::AsciiFileIterator(std::string_view filename) {
  ifs_.open(std::string(filename), std::ios::in);
  if (!ifs_.is_open()) {
    done_ = true;
    //TODO error if file not found
  } else {
    done_ = false;
    Next();
  }
}


AsciiFileIterator::~AsciiFileIterator() {
  if (ifs_.is_open()) {
    ifs_.close();
  }
}


AsciiFileIterator::reference AsciiFileIterator::Next() {
  if (std::getline(ifs_, line_)) {
    ++line_number_;
  } else {
    done_ = true;
  }
  return line_;
}

AsciiFileIterator &AsciiFileIterator::operator++() {
  Next();
  return *this;
}

}  // namespace files
}  // namespace werkzeugkiste
