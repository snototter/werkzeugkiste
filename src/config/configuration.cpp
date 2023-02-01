#include <toml++/toml.h>  // NOLINT
#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/files/fileio.h>
#include <werkzeugkiste/logging.h>

#include <functional>
#include <limits>
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

/// @brief Visits all child nodes of the given TOML configuration in a
/// depth-first search style and invokes the function handle for each node.
/// @param node Container node from which to start the traversal. The initial
/// node (usually the configuration/document root) will **not** be passed to the
/// function handle
/// @param path Path identifier/key of the given node. Must be empty for the
/// root node.
/// @param visit_func Function handle to be invoked for each node (except for
/// the initial `node`).
void Traverse(
    toml::node &node, std::string_view path,
    const std::function<void(toml::node &, std::string_view)> &visit_func) {
  // Iterate container nodes (i.e. table and array) and call the given functor
  // for each node.
  if (node.is_table()) {
    toml::table &tbl = *node.as_table();
    for (auto &&[key, value] : tbl) {
      const std::string fqn = FullyQualifiedPath(key, path);
      visit_func(value, fqn);

      if (value.is_array() || value.is_table()) {
        Traverse(value, fqn, visit_func);
      }
    }
  } else if (node.is_array()) {
    toml::array &arr = *node.as_array();
    std::size_t index = 0;
    for (auto &value : arr) {
      const std::string fqn = FullyQualifiedArrayElementPath(index, path);
      visit_func(value, fqn);

      if (value.is_array() || value.is_table()) {
        Traverse(value, fqn, visit_func);
      }
      ++index;
    }
  } else {
    std::string msg{
        "Traverse() can only be invoked with either `table` or "
        "`array` nodes, but `"};
    msg += path;
    msg += "` is neither!";
    WKZLOG_ERROR(msg);
    throw std::logic_error(msg);
  }
}

class SingleKeyMatcherImpl : public SingleKeyMatcher {
 public:
  explicit SingleKeyMatcherImpl(std::string &&pattern)
      : SingleKeyMatcher(), pattern_(std::move(pattern)) {
    // TODO https://toml.io/en/v1.0.0
    // dotted is allowed (and needed to support paths!)
    // add documentation that quoted keys are not supported! - maybe add a check
    // and raise an exception if needed?
    auto it = std::find_if_not(pattern_.begin(), pattern_.end(), [](char c) {
      return ((isalnum(c) != 0) || (c == '.') || (c == '_') || (c == '-'));
    });

    is_regex_ = (it != pattern_.end());

    if (is_regex_) {
      BuildRegex();
    }
  }

  ~SingleKeyMatcherImpl() override = default;

  bool Match(std::string_view key) const override {
    // Always check for equality
    if (pattern_.compare(key) == 0) {
      return true;
    }

    if (is_regex_) {
      constexpr auto flags = std::regex_constants::match_default;
      if (std::regex_match(key.begin(), key.end(), regex_, flags)) {
        return true;
      }
    }

    return false;
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

std::unique_ptr<SingleKeyMatcher> SingleKeyMatcher::Create(
    std::string_view pattern) {
  return std::make_unique<SingleKeyMatcherImpl>(std::string(pattern));
}

class MultiKeyMatcherImpl : public MultiKeyMatcher {
 public:
  explicit MultiKeyMatcherImpl(const std::vector<std::string_view> &patterns)
      : MultiKeyMatcher() {
    for (const auto &pattern : patterns) {
      matchers_.emplace_back(SingleKeyMatcherImpl(std::string(pattern)));
    }
  }

  ~MultiKeyMatcherImpl() override = default;

  bool MatchAny(std::string_view key) const override {
    for (const auto &m : matchers_) {
      if (m.Match(key)) {
        return true;
      }
    }
    return false;
  }

 private:
  std::vector<SingleKeyMatcherImpl> matchers_{};
};

std::unique_ptr<MultiKeyMatcher> MultiKeyMatcher::Create(
    const std::vector<std::string_view> &patterns) {
  return std::make_unique<MultiKeyMatcherImpl>(patterns);
}

template <typename T>
inline std::string BuiltinTypeName() {
  if constexpr (std::is_same_v<T, double>) {
    return "double";
  }

  if constexpr (std::is_integral_v<T>) {
    if constexpr (std::is_same_v<T, int32_t>) {
      return "int32";
    }

    if constexpr (std::is_same_v<T, int64_t>) {
      return "int64";
    }
  }

  if constexpr (std::is_same_v<T, bool>) {
    return "bool";
  }

  if constexpr (std::is_same_v<T, std::string>) {
    return "string";
  }

  throw std::logic_error("Type not supported!");
}

template <typename NodeView>
inline std::string TomlTypeName(const NodeView &node, std::string_view key) {
  if (node.is_table()) {
    return "table";
  }

  if (node.is_array()) {
    return "array";
  }

  if (node.is_integer()) {
    return "int";
  }

  if (node.is_floating_point()) {
    return "double";
  }

  if (node.is_string()) {
    return "string";
  }

  if (node.is_boolean()) {
    return "bool";
  }

  if (node.is_date()) {
    return "toml::date";
  }

  if (node.is_time()) {
    return "toml::time";
  }

  if (node.is_date_time()) {
    return "toml::date_time";
  }

  std::string msg{"TOML node type for key `"};
  msg += key;
  msg +=
      "` is not handled in `TomlTypeName`. This is a werkzeugkiste "
      "implementation error. Please open an issue.";
  throw std::logic_error(msg);
}

/// Returns true if the TOML table contains a valid node at the given,
/// fully-qualified path/key.
inline bool ConfigContains(const toml::table &tbl, std::string_view key) {
  // Needed, because `tbl.contains()` only checks the direct children.
  const auto node = tbl.at_path(key);
  return node.is_value() || node.is_table() || node.is_array();
}

template <typename T>
T ConfigLookup(const toml::table &tbl, std::string_view key,
               bool allow_default = false, T default_val = T{}) {
  if (!ConfigContains(tbl, key)) {
    if (allow_default) {
      return default_val;
    }

    std::string msg{"Key `"};
    msg += key;
    msg += "` does not exist!";
    throw std::runtime_error(msg);
  }

  const auto node = tbl.at_path(key);
  // if (!node.is_value()) {
  //   throw std::runtime_error("TODO parameter must be a value, not a
  //   container!");   //TODO
  // }
  if (node.is<T>()) {
    return T(*node.as<T>());
  }

  std::string msg{"Invalid type `"};
  msg += BuiltinTypeName<T>();
  msg += "` used to query key `";
  msg += key;
  msg += "`, which is of type `";
  msg += TomlTypeName(node, key);
  msg += "`!";
  throw std::runtime_error(msg);
}

inline int32_t ConfigLookupInt32(const toml::table &tbl, std::string_view key,
                                 bool allow_default = false,
                                 int32_t default_val = 0) {
  // TOML stores integers as int64_t
  int64_t value64 = ConfigLookup<int64_t>(tbl, key, allow_default,
                                          static_cast<int64_t>(default_val));
  constexpr auto min32 =
      static_cast<int64_t>(std::numeric_limits<int32_t>::min());
  constexpr auto max32 =
      static_cast<int64_t>(std::numeric_limits<int32_t>::max());
  if ((value64 > max32) || (value64 < min32)) {
    std::string msg{"Parameter value `"};
    msg += key;
    msg += " = ";
    msg += std::to_string(value64);
    msg += "` exceeds 32-bit integer range!";
    throw std::range_error(msg);
  }

  return static_cast<int32_t>(value64);
}

template <typename Type, int Dim, typename NodeView>
inline auto ExtractPoint(const NodeView &node, std::string_view key) {
  static_assert((Dim == 2) || (Dim == 3),
                "Only 2D and 3D points are supported!");

  Type x{};
  Type y{};
  Type z{};

  if (node.is_array()) {
    const toml::array &arr = *node.as_array();
    if (arr.size() != Dim) {
      std::ostringstream msg;
      msg << "Invalid parameter `" << key << "`. Cannot extract a " << Dim
          << "D point from a " << arr.size() << "-element array entry!";
      throw std::runtime_error(msg.str());
    }
    if (!arr.is_homogeneous()) {
      throw std::runtime_error("TODO not homogeneous!");
    }
    if (!arr[0].is<Type>()) {
      throw std::runtime_error("TODO wrong type");
    }
    x = Type(*arr[0].as<Type>());  // TODO
    y = Type(*arr[1].as<Type>());  // TODO
    if constexpr (Dim == 3) {
      z = Type(*arr[2].as<Type>());  // TODO
    }
  } else if (node.is_table()) {
    const toml::table &tbl = *node.as_table();
    if constexpr (Dim == 2) {
      if (!tbl.contains("x") || !tbl.contains("y")) {
        std::ostringstream msg;
        msg << "Invalid parameter `" << key
            << "`. Table entry does not contain x, y!";
        throw std::runtime_error(msg.str());
      }
    } else {
      if (!tbl.contains("x") || !tbl.contains("y") || !tbl.contains("z")) {
        std::ostringstream msg;
        msg << "Invalid parameter `" << key
            << "`. Table entry does not contain x, y, z!";
        throw std::runtime_error(msg.str());
      }
    }

    // TODO type checks!
    if (!tbl["x"].is<Type>()) {
      throw std::runtime_error("TODO wrong type");
    }

    x = Type(*tbl["x"].as<Type>());
    y = Type(*tbl["y"].as<Type>());
    if constexpr (Dim == 3) {
      z = Type(*tbl["z"].as<Type>());
    }
  } else {
    throw std::runtime_error("TODO! poly must be table or array");
  }

  if constexpr (Dim == 2) {
    return std::make_tuple(x, y);
  } else {
    return std::make_tuple(x, y, z);
  }
}

class ConfigurationImpl : public Configuration {
 public:
  ~ConfigurationImpl() override = default;

  explicit ConfigurationImpl(toml::table &&config)
      : Configuration(), config_(std::move(config)) {}

  bool Equals(const Configuration *other) const override {
    const ConfigurationImpl *other_impl =
        dynamic_cast<const ConfigurationImpl *>(other);
    if (other_impl == nullptr) {
      return false;
    }
    return EqualsImpl(other_impl);
  }

  bool EnsureAbsolutePaths(
      std::string_view base_path,
      const std::vector<std::string_view> &parameters) override {
    using namespace std::string_view_literals;
    MultiKeyMatcherImpl matcher{parameters};
    auto to_replace = [matcher](std::string_view fqn) -> bool {
      return matcher.MatchAny(fqn);
    };

    bool replaced{false};
    auto func = [replaced, to_replace, base_path](
                    toml::node &node, std::string_view fqn) mutable -> void {
      if (node.is_string() && to_replace(fqn)) {
        auto &str = *node.as_string();
        // WKZLOG_ERROR("Will replace param {:s}, was previously {:s}", fqn,
        // str);
        str = "*******"sv;
        // TODO 1) fullfile basepath! must link to file utils
        // TODO 2) check special character handling (backslash, umlauts,
        // whitespace)
        replaced = true;
      }
    };
    Traverse(config_, ""sv, func);
    // WKZLOG_ERROR("After replacements:\n{:s}\nreplaced?{}", config_,
    // replaced);
    return replaced;
  }

  std::vector<std::string> ParameterNames() const override {
    return ListTableKeys(config_, "");
  }

  bool GetBoolean(std::string_view key) const override {
    return ConfigLookup<bool>(config_, key, false);
  }

  bool GetBooleanOrDefault(std::string_view key,
                           bool default_val) const override {
    return ConfigLookup<bool>(config_, key, true, default_val);
  }

  double GetDouble(std::string_view key) const override {
    return ConfigLookup<double>(config_, key, false);
  }

  double GetDoubleOrDefault(std::string_view key,
                            double default_val) const override {
    return ConfigLookup<double>(config_, key, true, default_val);
  }

  int32_t GetInteger32(std::string_view key) const override {
    return ConfigLookupInt32(config_, key, false);
  }

  int32_t GetInteger32OrDefault(std::string_view key,
                                int32_t default_val) const override {
    return ConfigLookupInt32(config_, key, true, default_val);
  }

  int64_t GetInteger64(std::string_view key) const override {
    return ConfigLookup<int64_t>(config_, key, false);
  }

  int64_t GetInteger64OrDefault(std::string_view key,
                                int64_t default_val) const override {
    return ConfigLookup<int64_t>(config_, key, true, default_val);
  }

  std::vector<std::tuple<int64_t, int64_t>> GetPolygon2D(
      std::string_view key) const override {
    return GetPolygonImpl<std::tuple<int64_t, int64_t>, int64_t, 2>(key);
  }

  std::vector<std::tuple<int64_t, int64_t, int64_t>> GetPolygon3D(
      std::string_view key) const override {
    return GetPolygonImpl<std::tuple<int64_t, int64_t, int64_t>, int64_t, 3>(
        key);
  }

  // Configuration &GetGroup(std::string_view group_name) override {
  //   //TODO create a copy & return it as unique_ptr (can't use ref to pure
  //   // virtual class)
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

  bool EqualsImpl(const ConfigurationImpl *other) const {
    using namespace std::string_view_literals;
    const auto keys_this = ListTableKeys(config_, ""sv);
    const auto keys_other = ListTableKeys(other->config_, ""sv);
    if (keys_this.size() != keys_other.size()) {
      return false;
    }

    for (const auto &key : keys_this) {
      const auto nv_this = config_.at_path(key);
      const auto nv_other = other->config_.at_path(key);
      if (nv_this != nv_other) {
        return false;
      }
    }

    return true;
  }

  template <typename Tuple, typename Type, int Dim>
  std::vector<Tuple> GetPolygonImpl(std::string_view key) const {
    const auto node = config_.at_path(key);
    if (!node.is_array()) {
      std::string msg{"Invalid polygon - parameter `"};
      msg += key;
      msg += "` must be an array!";
      throw std::runtime_error(msg);
    }

    const toml::array &arr = *node.as_array();
    std::size_t array_index = 0;
    std::vector<Tuple> poly;
    for (auto &&value : arr) {
      poly.emplace_back(ExtractPoint<Type, Dim>(value, key));
      ++array_index;
    }
    return poly;
  }

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
