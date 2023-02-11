#include <werkzeugkiste/config/configuration.h>

#include <initializer_list>
#include <optional>
#include <regex>
#include <utility>  // pair, swap
#include <vector>

namespace werkzeugkiste::config {
struct KeyMatcher::Impl {
 public:
  //  Impl() = default;

  //  // Copyable
  //  Impl(const Impl &) = default;
  //  Impl &operator=(const Impl &) = default;

  //  // Moveable
  //  Impl(Impl &&) = default;
  //  Impl &operator=(Impl &&) = default;

  void RegisterKey(std::string_view key) {
    patterns_.emplace_back(std::make_pair(std::string(key), BuildRegex(key)));
  }

  bool Match(std::string_view query) const {
    constexpr auto re_flags = std::regex_constants::match_default;

    for (const auto &pattern : patterns_) {
      if (pattern.first == query) {
        return true;
      }

      if (pattern.second.has_value() &&
          std::regex_match(query.begin(), query.end(), pattern.second.value(),
                           re_flags)) {
        return true;
      }
    }
    return false;
  }

  bool Empty() const { return patterns_.empty(); }

 private:
  std::vector<std::pair<std::string, std::optional<std::regex>>> patterns_{};

  static bool IsRegex(std::string_view key) {
    const auto *it = std::find_if_not(key.begin(), key.end(), [](char c) {
      return ((isalnum(c) != 0) || (c == '.') || (c == '_') || (c == '-'));
    });
    return it != key.end();
  }

  static std::optional<std::regex> BuildRegex(std::string_view key) {
    if (!IsRegex(key)) {
      return std::nullopt;
    }

    std::string re{"^"};
    for (char c : key) {
      if (c == '*') {
        re += ".*";
      } else if ((c == '.') || (c == '[') || (c == ']')) {
        re += '\\';
        re += c;
      } else {
        re += c;
      }
    }
    re += '$';

    return std::make_optional<std::regex>(re);
  }
};

KeyMatcher::KeyMatcher() : pimpl_{new Impl{}} {}

KeyMatcher::KeyMatcher(std::initializer_list<std::string_view> keys)
    : pimpl_{new Impl{}} {
  for (auto key : keys) {
    pimpl_->RegisterKey(key);
  }
}

KeyMatcher::KeyMatcher(const std::vector<std::string_view> &keys)
    : pimpl_{new Impl{}} {
  for (auto key : keys) {
    pimpl_->RegisterKey(key);
  }
}

KeyMatcher::~KeyMatcher() = default;

KeyMatcher::KeyMatcher(const KeyMatcher &other)
    : pimpl_{std::make_unique<Impl>(*other.pimpl_)} {}

KeyMatcher::KeyMatcher(KeyMatcher &&other) noexcept {
  std::swap(pimpl_, other.pimpl_);
}

KeyMatcher &KeyMatcher::operator=(const KeyMatcher &other) {
  if (this != &other) {
    pimpl_ = std::make_unique<Impl>(*other.pimpl_);
  }
  return *this;
}

KeyMatcher &KeyMatcher::operator=(KeyMatcher &&other) noexcept {
  if (this != &other) {
    std::swap(pimpl_, other.pimpl_);
  }
  return *this;
}

void KeyMatcher::RegisterKey(std::string_view key) { pimpl_->RegisterKey(key); }

bool KeyMatcher::Match(std::string_view query) const {
  return pimpl_->Match(query);
}

bool KeyMatcher::Empty() const {
  return (pimpl_ == nullptr) || (pimpl_->Empty());
}

}  // namespace werkzeugkiste::config
