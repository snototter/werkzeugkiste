#include <sys/stat.h>
#include <sys/types.h>
#include <werkzeugkiste/files/filesys.h>
#include <werkzeugkiste/strings/strings.h>

#include <exception>
#include <fstream>
#include <sstream>
#ifdef WZK_HAS_FILESYSTEM
#  include <filesystem>
// FIXME linking to std::filesystem will be a chore
//#else // WZK_HAS_FILESYSTEM
#endif // WZK_HAS_FILESYSTEM

namespace werkzeugkiste::files {

// TODO add:
// filename sorting & filtering
// mkdir & mkpath
// exists
// isdir, isabsolute
// listdircontents
// getextension

#  if defined(WIN32) || defined(_WIN32) || defined(__CYGWIN__)
const char k_file_separator = '\\';
#  else // WIN32
const char k_file_separator = '/';
#  endif // WIN32


bool Exists(const std::string& name) {
#ifdef WKZ_HAS_FILESYSTEM
  std::filesystem::path pth{name};
  return std::filesystem::exists(pth);
#else
   std::ifstream f(name.c_str());
   const bool status = f.good();
   f.close();
   return status;
#endif // WKZ_HAS_FILESYSTEM
}

// taken from
// http://stackoverflow.com/questions/18100097/portable-way-to-check-if-directory-exists-windows-linux-c
bool IsDir(const std::string& path) {
  struct stat info {};
  if (stat(path.c_str(), &info) != 0) {
    return false;
  }

  if (info.st_mode & S_IFDIR) {
    return true;
  }

  return false;
}

std::string FullFile(const std::string& p1, const std::string& p2) {
  std::string path(p1);
  if (!strings::EndsWith(p1, k_file_separator)) {
    path += k_file_separator;
  }
  path += p2;
  return path;
}

std::string FullFile(const std::vector<std::string>& path_tokens) {
  std::string path;
  bool prepend_delim = false;
  for (const auto& token : path_tokens) {
    if (prepend_delim) {
      path += k_file_separator;
    }
    path += token;
    prepend_delim = !strings::EndsWith(token, k_file_separator);
  }
  return path;
}

std::string FullFile(std::initializer_list<std::string> path_tokens) {
  std::string path;
  bool prepend_delim = false;
  for (const auto& token : path_tokens) {
    if (prepend_delim) {
      path += k_file_separator;
    }
    path += token;
    prepend_delim = !strings::EndsWith(token, k_file_separator);
  }
  return path;
}

std::string Parent(const std::string& path) {
  std::vector<std::string> components = strings::Split(path, k_file_separator);
  if (components.size() < 2) {
    std::string res;
    res += k_file_separator;
    return res;
  }

  components.pop_back();
  return FullFile(components);
}

std::string DirName(const std::string& path) {
  if (Exists(path) && IsDir(path)) {
    return path;
  } else {
    return Parent(path);
  }
}

}  // namespace werkzeugkiste::files
