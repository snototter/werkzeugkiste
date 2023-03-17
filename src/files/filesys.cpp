#include <sys/stat.h>
#include <sys/types.h>
#include <werkzeugkiste/files/filesys.h>
#include <werkzeugkiste/strings/strings.h>

#include <exception>
#include <fstream>
#include <sstream>
#ifdef WZK_HAS_FILESYSTEM
#include <filesystem>
// FIXME linking to std::filesystem will be a chore
// #else // WZK_HAS_FILESYSTEM
#endif  // WZK_HAS_FILESYSTEM

namespace werkzeugkiste::files {

// TODO add:
// filename sorting & filtering
// mkdir & mkpath
// exists
// isdir, isabsolute
// listdircontents
// getextension

#if defined(WIN32) || defined(_WIN32) || defined(__CYGWIN__)
const char k_file_separator = '\\';
#else   // WIN32
const char k_file_separator = '/';
#endif  // WIN32

bool Exists(const std::string& name) {
#ifdef WZK_HAS_FILESYSTEM
  std::filesystem::path pth{name};
  return std::filesystem::exists(pth);
#else
  std::ifstream f(name.c_str());
  const bool status = f.good();
  f.close();
  return status;
#endif  // WZK_HAS_FILESYSTEM
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

std::optional<std::string> Basename(std::string_view path) {
  const std::string_view separator{&k_file_separator, 1};
  const std::vector<std::string_view> parts =
      strings::Tokenize(path, separator);
  if (parts.empty()) {
    return std::nullopt;
  }

  return std::make_optional(std::string{parts[parts.size() - 1]});
}

std::optional<std::string> Extension(std::string_view path) {
  const auto bname = Basename(path);
  if (!bname.has_value()) {
    return std::nullopt;
  }

  const std::size_t pos = bname.value().find_last_of('.');
  if (pos == std::string::npos) {
    return std::nullopt;
  }

  return std::make_optional(bname.value().substr(pos));
}

std::string FullFile(std::string_view p1, std::string_view p2) {
  std::string path{p1};
  if (!strings::EndsWith(p1, k_file_separator) &&
      !strings::StartsWith(p2, k_file_separator)) {
    path += k_file_separator;
  }
  path += p2;
  return path;
}

std::string FullFile(const std::vector<std::string>& path_tokens) {
  std::string path{};
  bool prepend_delim = false;
  for (const auto& token : path_tokens) {
    if (prepend_delim && !strings::StartsWith(token, k_file_separator)) {
      path += k_file_separator;
    }
    path += token;
    prepend_delim = !strings::EndsWith(token, k_file_separator);
  }
  return path;
}

std::string FullFile(std::initializer_list<std::string_view> path_tokens) {
  std::string path{};
  bool prepend_delim = false;
  for (const auto& token : path_tokens) {
    if (prepend_delim && !strings::StartsWith(token, k_file_separator)) {
      path += k_file_separator;
    }
    path += token;
    prepend_delim = !strings::EndsWith(token, k_file_separator);
  }
  return path;
}

std::string Parent(std::string_view path) {
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

bool IsAbsolute(std::string_view path) {
#if defined(__linux__) || defined(__unix__)
  return (path.length() > 0) && (path[0] == k_file_separator);
#else
  throw std::logic_error(
      "IsAbsolute() is only implemented for Unix-based systems!");
#endif
}

}  // namespace werkzeugkiste::files
