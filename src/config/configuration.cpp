#include <toml++/toml.h>  // NOLINT
#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/files/fileio.h>
#include <werkzeugkiste/logging.h>
#include <werkzeugkiste/strings/strings.h>

#include <functional>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace werkzeugkiste::config {

// Forward declarations
std::vector<std::string> ListTableKeys(const toml::table &tbl,
                                       std::string_view path);

std::vector<std::string> ListArrayKeys(const toml::array &arr,
                                       std::string_view path);

inline std::string FullyQualifiedPath(const toml::key &key,
                                      std::string_view parent_path) {
  if (parent_path.length() > 0) {
    std::string fqn{parent_path};
    fqn += '.';
    fqn += key.str();
    return fqn;
  }
  return std::string(key.str());
}

inline std::string FullyQualifiedArrayElementPath(std::size_t array_index,
                                                  std::string_view path) {
  std::string fqn{path};
  fqn += '[';
  fqn += std::to_string(array_index);
  fqn += ']';
  return fqn;
}

std::vector<std::string> ListArrayKeys(const toml::array &arr,
                                       std::string_view path) {
  std::vector<std::string> keys;
  std::size_t array_index = 0;
  for (auto &&value : arr) {
    if (value.is_table()) {
      const toml::table &tbl = *value.as_table();
      const auto subkeys =
          ListTableKeys(tbl, FullyQualifiedArrayElementPath(array_index, path));
      keys.insert(keys.end(), subkeys.begin(), subkeys.end());
    }
    if (value.is_array()) {
      const toml::array &nested_arr = *value.as_array();
      const auto subkeys = ListArrayKeys(
          nested_arr, FullyQualifiedArrayElementPath(array_index, path));
      keys.insert(keys.end(), subkeys.begin(), subkeys.end());
    }
    ++array_index;
  }
  return keys;
}

std::vector<std::string> ListTableKeys(const toml::table &tbl,
                                       std::string_view path) {
  // auto fq_path = [path](const toml::key &key) -> std::string {
  //   if (path.length() > 0) {
  //     std::string fqn{path};
  //     fqn += '.';
  //     fqn += key.str();
  //     return fqn;
  //   }
  //   return std::string(key.str());
  // };

  // // TODO group.name.array[idx]
  // auto fq_array_element = [fq_path](const toml::key &key, std::size_t idx) ->
  // std::string {
  //   std::string fqn = fq_path(key);
  //   fqn += '[';
  //   fqn += std::to_string(idx);
  //   fqn += ']';
  //   return fqn;
  // };

  std::vector<std::string> keys;
  for (auto &&[key, value] : tbl) {
    keys.emplace_back(FullyQualifiedPath(key, path));
    if (value.is_array()) {
      // TODO remove
      if (value.is_array_of_tables()) {
        WKZLOG_CRITICAL("TODO array is also array of tables: {:s}",
                        FullyQualifiedPath(key, path));
      }
      // TODO separate function as we need to recurse!!
      const auto subkeys =
          ListArrayKeys(*value.as_array(), FullyQualifiedPath(key, path));
      keys.insert(keys.end(), subkeys.begin(), subkeys.end());
    }
    if (value.is_table()) {
      const auto subkeys =
          ListTableKeys(*value.as_table(), FullyQualifiedPath(key, path));
      keys.insert(keys.end(), subkeys.begin(), subkeys.end());
    }
  }
  return keys;
}

void Traverse(toml::node &node, std::string_view path,
              const std::function<void(toml::node &, std::string_view)> &func) {
  // Iterate container nodes (i.e. table and array) and call the given functor
  // for each node.
  if (node.is_table()) {
    toml::table &tbl = *node.as_table();
    for (auto &&[key, value] : tbl) {
      const std::string fqn = FullyQualifiedPath(key, path);
      func(value, fqn);

      if (value.is_array() || value.is_table()) {
        Traverse(value, fqn, func);
      }
    }
  } else if (node.is_array()) {
    toml::array &arr = *node.as_array();
    std::size_t index = 0;
    for (auto &value : arr) {
      const std::string fqn = FullyQualifiedArrayElementPath(index, path);
      func(value, fqn);

      if (value.is_array() || value.is_table()) {
        Traverse(value, fqn, func);
      }
      ++index;
    }
  } else {
    std::string msg{
        "Traverse() can only be invoked with either `table` or "
        "`array` nodes. Check setting `"};
    msg += path;
    msg += "`!";
    WKZLOG_ERROR(msg);
    throw std::logic_error(msg);
  }
}

class KeyMatcher {
 public:
  KeyMatcher(std::string &&pattern) : pattern_(std::move(pattern)) {
    // TODO https://toml.io/en/v1.0.0
    // dotted is allowed (and needed to support paths!)
    // doc that quoted keys are not supported! - maybe add a check and raise an
    // exception if needed?
    auto it = std::find_if(pattern_.begin(), pattern_.end(), [](char c) {
      return !(isalnum(c) || (c == '.') || (c == '_') || (c == '-'));
    });

    is_regex_ = (it != pattern_.end());

    if (is_regex_) {
      BuildRegex();
    }
  }

  bool Matches(std::string_view key) const {
    constexpr auto flags = std::regex_constants::match_default;
    if (std::regex_match(key.begin(), key.end(), regex_, flags)) {
      return true;
    }
  }

 private:
  std::string pattern_{};
  bool is_regex_{false};
  std::regex regex_{};

  inline void BuildRegex() {
    std::string re{"^"};
    for (char c : pattern_) {
      if (c == '*') {
        re += ".*";
      } else if (c == '.') {
        re += "\\.";
      } else {  // TODO test backslash handling!
        re += c;
      }
    }
    re += '$';
    WKZLOG_ERROR("Converted pattern `{:s}` to regex str `{:s}`", pattern_,
                 re);  // TODO remove
    regex_ = std::regex{re};
  }
};

class MultiKeyMatcher {
 public:
  MultiKeyMatcher(const std::vector<std::string_view> &patterns) {
    for (const auto &pattern : patterns) {
      matchers_.emplace_back(KeyMatcher(std::string(pattern)));
    }
  }

  bool MatchesAny(std::string_view key) const {
    for (const auto &m : matchers_) {
      if (m.Matches(key)) {
        return true;
      }
    }
    return false;
  }

 private:
  std::vector<KeyMatcher> matchers_{};
};

class ConfigurationImpl : public Configuration {
 public:
  ~ConfigurationImpl() override = default;

  explicit ConfigurationImpl(toml::table &&config)
      : Configuration(), config_(std::move(config)) {
    // const std::string_view key{"visualization.*.save"};
    // auto tokens = werkzeugkiste::strings::Tokenize(key, ".");
    // for (const auto &token : tokens) {
    //   WKZLOG_WARN("TODO check key-token '{:s}'", token);
    // }
    // auto test = config_[tokens[0]];
    // WKZLOG_WARN("key: {:s}", test[tokens[1]]);
  }

  bool EnsureAbsolutePaths(
      std::string_view base_path,
      const std::vector<std::string_view> &parameters) override {
    using namespace std::string_view_literals;
    MultiKeyMatcher matcher{parameters};
    auto to_replace = [matcher](std::string_view fqn) -> bool {
      return matcher.MatchesAny(fqn);
    };

    bool replaced{false};
    auto func = [replaced, to_replace, base_path](
                    toml::node &node, std::string_view fqn) mutable -> void {
      if (node.is_string() && to_replace(fqn)) {
        auto &str = *node.as_string();
        WKZLOG_ERROR("Will replace param {:s}, was previously {:s}", fqn, str);
        str = "*******"sv;  // TODO fullfile basepath! must link to file utils
        replaced = true;
      }
    };
    Traverse(config_, ""sv, func);
    WKZLOG_ERROR("After replacements:\n{:s}\nreplaced?{}", config_, replaced);
    return replaced;
  }

  std::vector<std::string> ParameterNames() const override {
    return ListTableKeys(config_, "");
  }

  // Configuration &GetGroup(std::string_view group_name) override {
  //   //TODO create a copy & return it
  //   return *this;
  // }

  std::string ToTOML() const override {
    std::ostringstream repr;
    repr << config_;
    return repr.str();
  }

  std::string ToJSON() const override { return "TODO"; }

 private:
  toml::table config_{};

  // TODO registered_string_replacements_{};
  //  std::vector<std::string> path_parameters_{};
};

std::unique_ptr<Configuration> Configuration::LoadTomlFile(
    std::string_view filename) {
  const std::string toml = werkzeugkiste::files::CatAsciiFile(filename);
  return Configuration::LoadTomlString(toml);
}

std::unique_ptr<Configuration> Configuration::LoadTomlString(
    std::string_view toml_string) {
  try {
    toml::table tbl = toml::parse(toml_string);
    WKZLOG_INFO("Loaded toml: {:s}", tbl);  // TODO remove
    return std::make_unique<ConfigurationImpl>(std::move(tbl));
  } catch (const toml::parse_error &err) {
    std::ostringstream msg;
    msg << "Error parsing TOML: " << err.description() << " ("
        << err.source().begin << ")!";
    WKZLOG_ERROR(msg.str());
    throw std::runtime_error(msg.str());
  }
}

// std::unique_ptr<Configuration> Configuration::LoadJSON(std::string_view
// filename) {
//     return std::make_unique<ConfigurationImpl>(ConfigurationImpl()); // TODO
// }
}  // namespace werkzeugkiste::config
