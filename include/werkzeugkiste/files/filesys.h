#ifndef WERKZEUGKISTE_FILES_FILESYS_H
#define WERKZEUGKISTE_FILES_FILESYS_H

#include <string>
#include <vector>

namespace werkzeugkiste::files {

// TODO Need to clarify 2 issues:
//  * Switching to C++17 (std::filesystem) will require major refactoring
//  * ::files relies on ::strings - need to verify it's properly
//  installed/linked

bool Exists(const std::string& name);
bool IsDir(const std::string& path);

std::string FullFile(const std::string& p1, const std::string& p2);
std::string FullFile(const std::vector<std::string>& path_tokens);
std::string FullFile(std::initializer_list<std::string> path_tokens);

/// Splits the string by the system's path delimiter (fwd or bwd slash)
/// and returns the parent entry.
/// For example:
/// * /path/to/foo --> /path/to
/// * /path/to/foo.h --> /path/to
std::string Parent(const std::string& path);

/// TODO doc
std::string DirName(const std::string& path);

}  // namespace werkzeugkiste::files

#endif  // WERKZEUGKISTE_FILES_FILEIO_H
