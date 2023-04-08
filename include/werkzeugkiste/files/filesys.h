#ifndef WERKZEUGKISTE_FILES_FILESYS_H
#define WERKZEUGKISTE_FILES_FILESYS_H

#include <werkzeugkiste/files/files_export.h>

#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

/// @brief File system utilities (mainly targeted for GNU/Linux systems).
namespace werkzeugkiste::files {

WERKZEUGKISTE_FILES_EXPORT
bool Exists(const std::string &name);

WERKZEUGKISTE_FILES_EXPORT
bool IsDir(const std::string &path);

WERKZEUGKISTE_FILES_EXPORT
std::optional<std::string> Basename(std::string_view path);

WERKZEUGKISTE_FILES_EXPORT
std::optional<std::string> Extension(std::string_view path);

WERKZEUGKISTE_FILES_EXPORT
std::string FullFile(std::string_view p1, std::string_view p2);

WERKZEUGKISTE_FILES_EXPORT
std::string FullFile(const std::vector<std::string> &path_tokens);

WERKZEUGKISTE_FILES_EXPORT
std::string FullFile(std::initializer_list<std::string_view> path_tokens);

/// Splits the string by the system's path delimiter (fwd or bwd slash)
/// and returns the parent entry.
/// For example:
/// * /path/to/foo --> /path/to
/// * /path/to/foo.h --> /path/to
WERKZEUGKISTE_FILES_EXPORT
std::string Parent(std::string_view path);

/// TODO doc
WERKZEUGKISTE_FILES_EXPORT
std::string DirName(const std::string &path);

/// @brief Returns true if the given path is absolute.
///
/// Only supported on Linux, Unix & Apple systems.
WERKZEUGKISTE_FILES_EXPORT
bool IsAbsolute(std::string_view path);

}  // namespace werkzeugkiste::files

#endif  // WERKZEUGKISTE_FILES_FILEIO_H
