#include <fstream>
#include <sstream>
#include <exception>

#include <sys/types.h>
#include <sys/stat.h>

#include <werkzeugkiste/files/filesys.h>
#include <werkzeugkiste/strings/strings.h>

namespace werkzeugkiste {
namespace files {

// TODO add:
// filename sorting & filtering
// mkdir & mkpath
// exists
// isdir, isabsolute
// listdircontents
// getextension


#if defined(WIN32) || defined(_WIN32) || defined(__CYGWIN__)
const char kFileSeparator = '\\';
#else
const char kFileSeparator = '/';
#endif



bool Exists(const std::string &name) {
  std::ifstream f(name.c_str());
  const bool status = f.good();
  f.close();
  return status;
}


// taken from http://stackoverflow.com/questions/18100097/portable-way-to-check-if-directory-exists-windows-linux-c
bool IsDir(const std::string &path) {
  struct stat info;
  if (stat(path.c_str(), &info) != 0) {
    return false;
  } else if(info.st_mode & S_IFDIR) {
    return true;
  } else {
    return false;
  }
}


std::string FullFile(const std::string &p1, const std::string &p2) {
  std::string path(p1);
  if (!strings::EndsWith(p1, kFileSeparator)) {
    path += kFileSeparator;
  }
  path += p2;
  return path;
}

std::string FullFile(const std::vector<std::string> &path_tokens) {
  std::string path;
  bool prepend_delim = false;
  for (const auto &token : path_tokens) {
    if (prepend_delim) {
      path += kFileSeparator;
    }
    path += token;
    prepend_delim = !strings::EndsWith(token, kFileSeparator);
  }
  return path;
}


std::string FullFile(std::initializer_list<std::string> path_tokens) {
  std::string path;
  bool prepend_delim = false;
  for (const auto &token : path_tokens) {
    if (prepend_delim) {
      path += kFileSeparator;
    }
    path += token;
    prepend_delim = !strings::EndsWith(token, kFileSeparator);
  }
  return path;
}


std::string Parent(const std::string &path) {
  std::vector<std::string> components = strings::Split(path, kFileSeparator);
  if (components.size() < 2) {
    std::string res;
    res += kFileSeparator;
    return res;
  }

  components.pop_back();
  return FullFile(components);
}


std::string DirName(const std::string &path) {
  if (Exists(path) && IsDir(path)) {
    return path;
  } else {
    return Parent(path);
  }
}

} // namespace files
} // namespace werkzeugkiste
