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

/// @brief Matches keys (fully qualified parameter names) against user-defined
/// patterns.
class WERKZEUGKISTE_CONFIG_EXPORT KeyMatcher {
 public:
  /// @brief Default constructor.
  KeyMatcher();

  /// @brief Constructor which accepts a list of patterns to match against.
  KeyMatcher(std::initializer_list<std::string_view> keys);

  /// @brief Constructor which accepts a list of patterns to match against.
  explicit KeyMatcher(const std::vector<std::string_view> &keys);

  /// @brief Destructor.
  ~KeyMatcher();

  /// @brief Copy constructor.
  KeyMatcher(const KeyMatcher &other);

  /// @brief Assignment operator.
  KeyMatcher &operator=(const KeyMatcher &other);

  /// @brief Move constructor.
  KeyMatcher(KeyMatcher &&other) noexcept;

  /// @brief Move assignment operator.
  KeyMatcher &operator=(KeyMatcher &&other) noexcept;

  /// @brief Registers an additional key (pattern) to match against.
  void RegisterKey(std::string_view key);

  /// @brief Checks if the given key matches any of the registered patterns.
  bool Match(std::string_view query) const;

  /// @brief Checks if the matcher is empty (no patterns registered).
  bool Empty() const;

 private:
  /// @brief Internal implementation struct.
  struct Impl;

  /// @brief PIMPL idiom.
  std::unique_ptr<Impl> pimpl_;
};

/// @brief Checks if the given key is a valid parameter name.
///
/// A valid parameter name consists of alpha-numeric characters, dashes and
/// underscores. Optionally, dots are also allowed (if )
///
/// @param key The key to be checked.
/// @param allow_dots If true, dots (as in a key path / fully qualified
///   parameter name) are also allowed.
WERKZEUGKISTE_CONFIG_EXPORT
bool IsValidKey(std::string_view key, bool allow_dots) noexcept;

}  // namespace werkzeugkiste::config

#endif  // WERKZEUGKISTE_CONFIG_KEYMATCHER_H
