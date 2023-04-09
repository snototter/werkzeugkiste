#include <werkzeugkiste/files/fileio.h>
#include <werkzeugkiste/logging.h>

#include <exception>
#include <fstream>
#include <sstream>

namespace werkzeugkiste::files {

std::vector<std::string> ReadAsciiFile(std::string_view filename) {
  std::ifstream ifs(std::string(filename), std::ios::in);
  if (!ifs.is_open()) {
    std::string msg{"Cannot open file. Check path: \""};
    msg += filename;
    msg += "\".";
    throw IOError(msg);
  }

  std::vector<std::string> lines;
  std::string line;
  while (std::getline(ifs, line)) {
    lines.push_back(line);
  }
  ifs.close();
  return lines;
}

std::string CatAsciiFile(std::string_view filename) {
  std::ifstream ifs(std::string(filename), std::ios::in);
  if (!ifs.is_open()) {
    std::string msg{"Cannot open file. Check path: \""};
    msg += filename;
    msg += "\".";
    throw IOError(msg);
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
    std::string msg{"Cannot open file. Check path: \""};
    msg += filename;
    msg += "\".";
    throw IOError(msg);
  }

  // Load first line
  done_ = false;
  Next();
  line_number_ = 0;
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

AsciiFileIterator& AsciiFileIterator::operator++() {
  Next();
  return *this;
}

}  // namespace werkzeugkiste::files
