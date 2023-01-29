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

inline bool ConfigPathMatchesPattern(std::string_view path,
                                     std::string_view pattern) {
  if (path.compare(pattern) == 0) {
    return true;
  }

  // if (pattern.find('*') != pattern.npos) {
  // TODO if there are any special characters in the pattern...
  std::string re{"^"};
  for (char c : pattern) {
    if (c == '*') {
      re += ".*";
    } else if (c == '.') {
      re += "\\.";
    } else {
      re += c;
    }
  }
  re += '$';
  WKZLOG_ERROR("Converted pattern `{:s}` to regex str `{:s}`", pattern, re);
  std::regex regex{re};
  const auto flags = std::regex_constants::match_default;
  if (std::regex_match(path.begin(), path.end(), regex, flags)) {
    return true;
  }
  // }
  // TODO asterisk replacements!

  return false;
}

inline bool ConfigPathMatchesAnyPattern(
    std::string_view path, const std::vector<std::string_view> &patterns) {
  for (const auto &ptn : patterns) {
    // WKZLOG_CRITICAL("TODO check {:s} vs pattern: {:s}", path, ptn);
    if (ConfigPathMatchesPattern(path, ptn)) {
      return true;
    }
  }
  return false;
}

class ConfigurationImpl : public Configuration {
 public:
  ~ConfigurationImpl() override = default;

  explicit ConfigurationImpl(toml::table &&config)
      : Configuration(), config_(std::move(config)) {
    const std::string_view key{"visualization.*.save"};
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
    auto to_replace = [parameters](std::string_view fqn) -> bool {
      return ConfigPathMatchesAnyPattern(fqn, parameters);
    };

    bool replaced{false};
    auto func = [replaced, to_replace, base_path](
                    toml::node &node, std::string_view fqn) mutable -> void {
      // WKZLOG_INFO("----Traverse {:s}", fqn);
      // if (node.is_string()) {
      //   auto &str = *node.as_string();
      //   WKZLOG_ERROR("------is a string {:s}", str);
      //   str = "replaced?"sv;
      //   WKZLOG_ERROR("is it replaced?: '{:s}'", str);
      //   replaced = true;
      // }
      if (node.is_string() && to_replace(fqn)) {
        auto &str = *node.as_string();
        WKZLOG_ERROR("Will replace param {:s}, was previously {:s}", fqn, str);
        str = "*******"sv;  // TODO fullfile basepath! must link to file utils
        replaced = true;
      }
    };
    Traverse(config_, "", func);
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

  bool EnsureAbsolutePathHelper(std::string_view base_path,
                                std::string_view key) {
    // TODO tokenize
    //  auto tokens = werkzeugkiste::strings::Tokenize(key, ".");
    //  for (const auto &token : tokens) {
    //    WKZLOG_WARN("TODO check key-token '{:s}'", token);
    //  }
    // TODO iterate names
    bool replaced = false;

    return replaced;
  }
  // std::vector<std::string> path_parameters_{};
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
    WKZLOG_INFO("Loaded toml: {:s}", tbl);
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
