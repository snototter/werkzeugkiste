#ifndef WERKZEUGKISTE_CONFIG_CONFIGURATION_H
#define WERKZEUGKISTE_CONFIG_CONFIGURATION_H

#include <werkzeugkiste/config/config_export.h>
#include <werkzeugkiste/config/types.h>

#include <cmath>
#include <initializer_list>
#include <limits>
#include <memory>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

/// Utilities to handle configurations.
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

}  // namespace werkzeugkiste::config

#endif  // WERKZEUGKISTE_CONFIG_KEYMATCHER_H
