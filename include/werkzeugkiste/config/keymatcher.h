#ifndef WERKZEUGKISTE_CONFIG_KEYMATCHER_H
#define WERKZEUGKISTE_CONFIG_KEYMATCHER_H

#include <werkzeugkiste/config/config_export.h>

#include <initializer_list>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace werkzeugkiste::config {
//-----------------------------------------------------------------------------
// Key (parameter name) matching to support access via wildcards

class WERKZEUGKISTE_CONFIG_EXPORT KeyMatcher {
 public:
  KeyMatcher();
  KeyMatcher(std::initializer_list<std::string_view> keys);
  explicit KeyMatcher(const std::vector<std::string_view> &keys);

  ~KeyMatcher();

  KeyMatcher(const KeyMatcher &other);
  KeyMatcher &operator=(const KeyMatcher &other);

  KeyMatcher(KeyMatcher &&other) noexcept;
  KeyMatcher &operator=(KeyMatcher &&other) noexcept;

  void RegisterKey(std::string_view key);

  bool Match(std::string_view query) const;

  bool Empty() const;

 private:
  struct Impl;
  std::unique_ptr<Impl> pimpl_;
};

/// @brief Checks if the given key is a valid parameter name.
///
/// A valid parameter name consists of alpha-numeric characters, dashes and
/// underscores. Optionally, dots are also allowed (if )
///
/// @param key The key to be checked.
/// @param allow_dots If true, dots (as in a key path / fully-qualified
///   parameter name) are also allowed.
WERKZEUGKISTE_CONFIG_EXPORT
bool IsValidKey(std::string_view key, bool allow_dots) noexcept;

}  // namespace werkzeugkiste::config

#endif  // WERKZEUGKISTE_CONFIG_KEYMATCHER_H
