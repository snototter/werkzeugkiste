#include <toml++/toml.h>  // NOLINT [build/c++11]
#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/files/fileio.h>
#include <werkzeugkiste/logging.h>
#include <werkzeugkiste/strings/strings.h>

#include <sstream>
#include <string>
#include <vector>

namespace werkzeugkiste::config {

// std::vector<std::string> ListKeys(const toml::array &array, std::string_view
// path) {

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
  // std::string fqn {FullyQualifiedPath(key, parent_path)};
  fqn += '[';
  fqn += std::to_string(array_index);
  fqn += ']';
  return fqn;
}

// Forward declaration
std::vector<std::string> ListKeys(const toml::table &tbl,
                                  std::string_view path);

std::vector<std::string> ListArrayKeys(const toml::array &arr,
                                       std::string_view path) {
  std::vector<std::string> keys;
  std::size_t array_index = 0;
  for (auto &&value : arr) {
    WKZLOG_INFO("Iterating array path '{:s}'",
                FullyQualifiedArrayElementPath(array_index, path));
    if (value.is_table()) {
      const toml::table &tbl = *value.as_table();
      const auto subkeys =
          ListKeys(tbl, FullyQualifiedArrayElementPath(array_index, path));
      keys.insert(keys.end(), subkeys.begin(), subkeys.end());
    }
    if (value.is_array()) {
      WKZLOG_WARN("TODO --> this is a nested array!");
      // TODO ListKeys, then append
    }
    ++array_index;
  }
  return keys;
}

std::vector<std::string> ListKeys(const toml::table &tbl,
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
      // TODO separate function as we need to recurse!!
      const auto subkeys =
          ListArrayKeys(*value.as_array(), FullyQualifiedPath(key, path));
      // std::size_t array_index = 0;
      // const toml::array &array = *value.as_array();
      // for (auto &&array_value : array) {
      //   WKZLOG_INFO("Iterating array path '{:s}'",
      //   FullyQualifiedArrayElementPath(key, array_index, parent_path)); if
      //   (array_value.is_table()) {
      //     WKZLOG_WARN("--> this is a table!");
      //   }
      //   ++array_index;
      // }
      // TODO iterate (path is [0]...) and recurse into tables
      keys.insert(keys.end(), subkeys.begin(), subkeys.end());
    }
    if (value.is_table()) {
      const auto subkeys =
          ListKeys(*value.as_table(), FullyQualifiedPath(key, path));
      keys.insert(keys.end(), subkeys.begin(), subkeys.end());
    }
  }
  return keys;
}

class ConfigurationImpl : public Configuration {
 public:
  ~ConfigurationImpl() override = default;

  explicit ConfigurationImpl(toml::table &&config)
      : Configuration(), config_(std::move(config)) {
    const std::string_view key{"visualization.*.save"};
    auto tokens = werkzeugkiste::strings::Tokenize(key, ".");
    for (const auto &token : tokens) {
      WKZLOG_WARN("TODO check key-token '{:s}'", token);
    }
    // auto test = config_[tokens[0]];
    // WKZLOG_WARN("key: {:s}", test[tokens[1]]);
  }

  bool EnsureAbsolutePaths(
      std::string_view base_path,
      const std::vector<std::string_view> &parameters) override {
    bool replaced = false;
    for (const auto &param : parameters) {
      const bool result = EnsureAbsolutePathHelper(
          base_path,
          param);  // TODO test to ensure that or doesn't get optimized!
      replaced |= result;
    }
    return replaced;
  }
  // void RegisterPathParameter(std::string_view param_name) override {
  //   path_parameters_.push_back(std::string(param_name));
  // }

  // std::size_t MakePathsAbsolute(std::string_view base_path) override {
  //   for (const auto &param : path_parameters_) {
  //     WKZLOG_WARN("TODO need to check path parameter: {:s}", param);
  //   }
  // }

  std::vector<std::string> ParameterNames() const override {
    return ListKeys(config_, "");
    // std::vector<std::string> names;

    // // for (auto &&[key, value] : config_) {
    // //   WKZLOG_WARN(
    // //       "TODO visiting '{:s}' = is_string({}), is_number({}),
    // is_table({}),  "
    // //       "(value type: {:s})",
    // //       key, value.is_string(), value.is_number(), value.is_table(),
    // //       typeid(decltype(value)).name());
    // // }
    // // config_.for_each([names](auto &&el) {
    // //   WKZLOG_WARN("TODO visiting {:s} (decltype: {:s})", el,
    // //               typeid(decltype(el)).name());      // WKZLOG_WARN("TODO
    // visiting '{:s}' = '{:s}' (value type: {:s})", key,
    // //   //             value, typeid(decltype(value)).name());
    // //   // if constexpr (toml::is_number<decltype(el)>)
    // //   //     (*el)++;
    // //   // else if constexpr (toml::is_string<decltype(el)>)
    // //   //     el = "five"sv;
    // // });
    // names = ListKeys(config_, "");
    // return names;
  }

  std::string ToTOML() const override { return "TODO"; }

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
