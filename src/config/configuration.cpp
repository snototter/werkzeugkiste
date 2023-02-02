// NOLINTBEGIN
#include <toml++/toml.h>
#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/files/fileio.h>
#include <werkzeugkiste/logging.h>

#include <array>
#include <functional>
#include <limits>
#include <regex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
// NOLINTEND

// NOLINTNEXTLINE(*macro-usage)
#define WZK_CONFIG_RAISE_KEY_ERROR(KEY) \
  do {                                  \
    std::string msg{"Key `"};           \
    msg += (KEY);                       \
    msg += "` does not exist!";         \
    throw std::runtime_error(msg);      \
  } while (false)

// NOLINTNEXTLINE(*macro-usage)
#define WZK_CONFIG_RAISE_TOML_TYPE_ERROR(KEY, NODE, TYPE) \
  do {                                                    \
    std::string msg{"Invalid type `"};                    \
    msg += BuiltinTypeName<TYPE>();                       \
    msg += "` used to query key `";                       \
    msg += (KEY);                                         \
    msg += "`, which is of type `";                       \
    msg += TomlTypeName((NODE), (KEY));                   \
    msg += "`!";                                          \
    throw std::runtime_error(msg);                        \
  } while (false)

namespace werkzeugkiste::config {
namespace utils {
/// Returns the "fully-qualified TOML path" for the given key and its parent
/// path. For example, `key = param` & `parent_path = lvl1.lvl2` results
/// in `lvl1.lvl2.param`.
inline std::string FullyQualifiedPath(const toml::key &key,
                                      std::string_view parent_path) {
  if (parent_path.length() > 0) {
    std::string fqn{parent_path};
    fqn += '.';
    fqn += key.str();
    return fqn;
  }
  return std::string{key.str()};
}

inline std::string FullyQualifiedArrayElementPath(std::size_t array_index,
                                                  std::string_view path) {
  std::string fqn{path};
  fqn += '[';
  fqn += std::to_string(array_index);
  fqn += ']';
  return fqn;
}

// Forward declaration.
std::vector<std::string> ListTableKeys(const toml::table &tbl,
                                       std::string_view path);

/// Returns all fully-qualified paths for named parameters within
/// the given TOML array. For example, "A = [20, { name = "value", age = 30 }"
/// returns `{ "A[1].name", "A[1].age" }`.
// NOLINTNEXTLINE(misc-no-recursion)
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

// NOLINTNEXTLINE(misc-no-recursion)
std::vector<std::string> ListTableKeys(const toml::table &tbl,
                                       std::string_view path) {
  std::vector<std::string> keys;
  for (auto &&[key, value] : tbl) {
    keys.emplace_back(FullyQualifiedPath(key, path));
    if (value.is_array()) {
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
// NOLINTNEXTLINE(misc-no-recursion)
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
    WZKLOG_ERROR(msg);
    throw std::logic_error(msg);
  }
}

inline int32_t SafeInteger32Cast(int64_t value64, std::string_view param_name) {
  constexpr auto min32 =
      static_cast<int64_t>(std::numeric_limits<int32_t>::min());
  constexpr auto max32 =
      static_cast<int64_t>(std::numeric_limits<int32_t>::max());
  if ((value64 > max32) || (value64 < min32)) {
    std::string msg{"Parameter value `"};
    msg += param_name;
    msg += " = ";
    msg += std::to_string(value64);
    msg += "` exceeds 32-bit integer range!";
    throw std::range_error(msg);
  }

  return static_cast<int32_t>(value64);
}

template <typename T>
inline std::string BuiltinTypeName() {
  // LCOV_EXCL_START
  std::ostringstream msg;
  msg << "Built-in type `" << typeid(T).name()
      << "` not handled in `BuiltinTypeName(). This is a werkzeugkiste "
         "implementation error. Please report at "
         "https://github.com/snototter/werkzeugkiste/issues";
  throw std::logic_error(msg.str());
  // LCOV_EXCL_STOP
}

template <>
inline std::string BuiltinTypeName<bool>() {
  return "bool";
}
template <>
inline std::string BuiltinTypeName<double>() {
  return "double";
}
template <>
inline std::string BuiltinTypeName<int32_t>() {
  return "int32_t";
}
template <>
inline std::string BuiltinTypeName<int64_t>() {
  return "int64_t";
}
template <>
inline std::string BuiltinTypeName<std::string>() {
  return "string";
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

  // LCOV_EXCL_START
  std::string msg{"TOML node type for key `"};
  msg += key;
  msg +=
      "` is not handled in `TomlTypeName`. This is a werkzeugkiste "
      "implementation error. Please report at "
      "https://github.com/snototter/werkzeugkiste/issues";
  throw std::logic_error(msg);
  // LCOV_EXCL_STOP
}

/// Returns true if the TOML table contains a valid node at the given,
/// fully-qualified path/key.
inline bool ConfigContainsKey(const toml::table &tbl, std::string_view key) {
  // Needed, because `tbl.contains()` only checks the direct children.
  const auto node = tbl.at_path(key);
  return node.is_value() || node.is_table() || node.is_array();
}

template <typename T>
T ConfigLookupScalar(const toml::table &tbl, std::string_view key,
                     bool allow_default = false, T default_val = T{}) {
  if (!ConfigContainsKey(tbl, key)) {
    if (allow_default) {
      return default_val;
    }

    WZK_CONFIG_RAISE_KEY_ERROR(key);
  }

  const auto node = tbl.at_path(key);
  if (node.is<T>()) {
    return T{*node.as<T>()};
  }

  WZK_CONFIG_RAISE_TOML_TYPE_ERROR(key, node, T);
}

/// Specialization needed for 32-bit integers, because TOML works with
/// 64-bit integers.
template <>
int32_t ConfigLookupScalar<int32_t>(const toml::table &tbl,
                                    std::string_view key, bool allow_default,
                                    int32_t default_val) {
  const int64_t value64 = ConfigLookupScalar<int64_t>(
      tbl, key, allow_default, static_cast<int64_t>(default_val));
  return SafeInteger32Cast(value64, key);
}

std::string ConfigLookupString(const toml::table &tbl, std::string_view key,
                               bool allow_default = false,
                               std::string_view default_val = {}) {
  if (!ConfigContainsKey(tbl, key)) {
    if (allow_default) {
      return std::string(default_val);
    }

    WZK_CONFIG_RAISE_KEY_ERROR(key);
  }

  const auto node = tbl.at_path(key);
  if (node.is_string()) {
    return std::string{*node.as_string()};
  }

  WZK_CONFIG_RAISE_TOML_TYPE_ERROR(key, node, std::string);
}

template <typename T>
T CastScalar(const toml::node &node, std::string_view key) {
  if (node.is<T>()) {
    return T{*node.as<T>()};
  }

  WZK_CONFIG_RAISE_TOML_TYPE_ERROR(key, node, T);
}

template <>
int32_t CastScalar<int32_t>(const toml::node &node, std::string_view key) {
  const int64_t value64 = CastScalar<int64_t>(node, key);
  return SafeInteger32Cast(value64, key);
}

template <typename Array, std::size_t... Idx>
inline auto ArrayToTuple(const Array &arr,
                         std::index_sequence<Idx...> /* indices */) {
  return std::make_tuple(arr[Idx]...);
}

template <typename Type, std::size_t Num>
inline auto ArrayToTuple(const std::array<Type, Num> &arr) {
  return ArrayToTuple(arr, std::make_index_sequence<Num>{});
}

// template <typename Tuple>
// inline Tuple ExtractPoint(const toml::array &arr, std::string_view key) {
template <typename Type, std::size_t Dim>
inline void ExtractPointFromTOMLArray(const toml::array &arr,
                                      std::string_view key,
                                      std::array<Type, Dim> &point) {
  if (arr.size() < Dim) {
    std::ostringstream msg;
    msg << "Invalid parameter `" << key << "`. Cannot extract a " << Dim
        << "D point from a " << arr.size() << "-element array!";
    throw std::runtime_error(msg.str());
  }
  for (std::size_t idx = 0; idx < Dim; ++idx) {
    if (arr[idx].is_number()) {
      if (!arr[idx].is<Type>()) {
        std::ostringstream msg;
        msg << "Invalid parameter `" << key << "`. Dimension [" << idx
            << "] is `" << TomlTypeName(arr, key) << "` instead of `"
            << BuiltinTypeName<Type>() << "`!";
        throw std::runtime_error(msg.str());
      }
      point[idx] = Type(*arr[idx].as<Type>());
    }
  }
}

template <typename Tuple, std::size_t Dim = std::tuple_size_v<Tuple>>
inline Tuple ExtractPoint(const toml::array &arr, std::string_view key) {
  using CoordType = std::tuple_element_t<0, Tuple>;
  static_assert(std::is_same_v<CoordType, int32_t> ||
                    std::is_same_v<CoordType, int64_t> ||
                    std::is_same_v<CoordType, double>,
                "Only integer (32- and 64-bit) and double-precision floating "
                "point types are supported!");

  using LookupType = std::conditional_t<std::is_same_v<CoordType, int32_t>,
                                        int64_t, CoordType>;

  std::array<LookupType, Dim> point{};
  ExtractPointFromTOMLArray(arr, key, point);

  if constexpr (std::is_same_v<CoordType, int32_t>) {
    std::array<CoordType, Dim> cast{};
    for (std::size_t idx = 0; idx < Dim; ++idx) {
      cast[idx] = SafeInteger32Cast(point[idx], key);
    }
    return ArrayToTuple(cast);
  } else {  // NOLINT(*else-after-return)
    return ArrayToTuple(point);
  }
}

template <typename Type, std::size_t Dim>
inline void ExtractPointFromTOMLTable(const toml::table &tbl,
                                      std::string_view key,
                                      std::array<Type, Dim> &point) {
  using namespace std::string_view_literals;
  constexpr std::array<std::string_view, 3> point_keys{"x"sv, "y"sv, "z"sv};
  static_assert(
      Dim <= point_keys.size(),
      "Table keys for higher-dimensional points have not yet been defined!");

  for (std::size_t idx = 0; idx < Dim; ++idx) {
    if (!tbl.contains(point_keys[idx])) {
      std::ostringstream msg;
      msg << "Invalid parameter `" << key << "`. Table entry does not specify `"
          << point_keys[idx] << "`!";
      throw std::runtime_error(msg.str());
    }

    if (!tbl[point_keys[idx]].is<Type>()) {
      std::ostringstream msg;
      msg << "Invalid parameter `" << key << "`. Dimension `" << point_keys[idx]
          << "` is `" << TomlTypeName(tbl[point_keys[idx]], key)
          << "` instead of `" << BuiltinTypeName<Type>() << "`!";
      throw std::runtime_error(msg.str());
    }

    point[idx] = Type(*tbl[point_keys[idx]].as<Type>());
  }
}

template <typename Tuple, std::size_t Dim = std::tuple_size_v<Tuple>>
inline Tuple ExtractPoint(const toml::table &tbl, std::string_view key) {
  using CoordType = std::tuple_element_t<0, Tuple>;
  static_assert(std::is_same_v<CoordType, int32_t> ||
                    std::is_same_v<CoordType, int64_t> ||
                    std::is_same_v<CoordType, double>,
                "Only integer (32- and 64-bit) and double-precision floating "
                "point types are supported!");

  using LookupType = std::conditional_t<std::is_same_v<CoordType, int32_t>,
                                        int64_t, CoordType>;

  std::array<LookupType, Dim> point{};
  ExtractPointFromTOMLTable(tbl, key, point);

  if constexpr (std::is_same_v<LookupType, CoordType>) {
    return ArrayToTuple(point);
  } else {  // NOLINT
    std::array<CoordType, Dim> cast{};
    for (std::size_t idx = 0; idx < Dim; ++idx) {
      cast[idx] = SafeInteger32Cast(point[idx], key);
    }
    return ArrayToTuple(cast);
  }
}

template <typename Tuple>
std::vector<Tuple> GetPoints(const toml::table &tbl, std::string_view key) {
  if (!ConfigContainsKey(tbl, key)) {
    std::string msg{"Key `"};
    msg += key;
    msg += "` does not exist!";
    throw std::runtime_error(msg);
  }

  const auto node = tbl.at_path(key);
  if (!node.is_array()) {
    std::string msg{"Invalid point list configuration: `"};
    msg += key;
    msg += "` must be an array, but is of type `";
    msg += TomlTypeName(node, key);
    msg += "`!";
    throw std::runtime_error(msg);
  }

  const toml::array &arr = *node.as_array();
  std::size_t arr_index = 0;
  std::vector<Tuple> poly;
  for (auto &&value : arr) {
    const auto fqn = FullyQualifiedArrayElementPath(arr_index, key);
    if (value.is_array()) {
      const auto &pt = *value.as_array();
      poly.emplace_back(ExtractPoint<Tuple>(pt, fqn));
    } else if (value.is_table()) {
      const auto &pt = *value.as_table();
      poly.emplace_back(ExtractPoint<Tuple>(pt, fqn));
    } else {
      std::string msg{
          "Invalid polygon. All parameter entries must be either arrays or "
          "tables, but `"};
      msg += fqn;
      msg += "` is not!";
      throw std::runtime_error(msg);
    }
    ++arr_index;
  }
  return poly;
}

/// Extracts a list of built-in scalar types (integer, double, bool).
/// TODO not suitable for TOML++-specific types (date, time, ...)
template <typename T>
std::vector<T> GetScalarList(const toml::table &tbl, std::string_view key) {
  if (!ConfigContainsKey(tbl, key)) {
    std::string msg{"Key `"};
    msg += key;
    msg += "` does not exist!";
    throw std::runtime_error(msg);
  }

  const auto &node = tbl.at_path(key);
  if (!node.is_array()) {
    std::string msg{"Invalid list configuration: Parameter `"};
    msg += key;
    msg += "` must be an array, but is `";
    msg += TomlTypeName(node, key);
    msg += "`!";
    throw std::runtime_error(msg);
  }

  const toml::array &arr = *node.as_array();
  std::size_t arr_index = 0;
  std::vector<T> scalars;
  for (auto &&value : arr) {
    const auto fqn = FullyQualifiedArrayElementPath(arr_index, key);
    if (value.is_value()) {
      scalars.push_back(CastScalar<T>(value, fqn));
    } else {
      std::string msg{
          "Invalid list configuration: All entries must be of scalar type `"};
      msg += BuiltinTypeName<T>();
      msg += "`, but `";
      msg += fqn;
      msg += "` is not!";
      throw std::runtime_error(msg);
    }
    ++arr_index;
  }
  return scalars;
}
}  // namespace utils

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

  SingleKeyMatcherImpl() = delete;
  SingleKeyMatcherImpl(const SingleKeyMatcherImpl &) = default;
  SingleKeyMatcherImpl(SingleKeyMatcherImpl &&) = default;
  SingleKeyMatcherImpl &operator=(const SingleKeyMatcherImpl &) = default;
  SingleKeyMatcherImpl &operator=(SingleKeyMatcherImpl &&) = default;
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
      } else if ((c == '.') || (c == '[') || (c == ']')) {
        re += '\\';
        re += c;
      } else {
        re += c;
      }
    }
    re += '$';
    WZKLOG_CRITICAL("Built regex for SingleKeyMatcher: {:s}", re);
    regex_ = std::regex{re};
  }
};

class MultiKeyMatcherImpl : public MultiKeyMatcher {
 public:
  explicit MultiKeyMatcherImpl(const std::vector<std::string_view> &patterns) {
    for (const auto &pattern : patterns) {
      matchers_.emplace_back(SingleKeyMatcherImpl(std::string(pattern)));
    }
  }

  MultiKeyMatcherImpl() = delete;
  MultiKeyMatcherImpl(const MultiKeyMatcherImpl &) = default;
  MultiKeyMatcherImpl(MultiKeyMatcherImpl &&) = default;
  MultiKeyMatcherImpl &operator=(const MultiKeyMatcherImpl &) = default;
  MultiKeyMatcherImpl &operator=(MultiKeyMatcherImpl &&) = default;
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

std::unique_ptr<SingleKeyMatcher> SingleKeyMatcher::Create(
    std::string_view pattern) {
  return std::make_unique<SingleKeyMatcherImpl>(std::string(pattern));
}

std::unique_ptr<MultiKeyMatcher> MultiKeyMatcher::Create(
    const std::vector<std::string_view> &patterns) {
  return std::make_unique<MultiKeyMatcherImpl>(patterns);
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
        // WZKLOG_ERROR("Will replace param {:s}, was previously {:s}", fqn,
        // str);
        str = "*******"sv;
        // TODO 1) fullfile basepath! must link to file utils
        // TODO 2) check special character handling (backslash, umlauts,
        // whitespace)
        replaced = true;
      }
    };
    utils::Traverse(config_, ""sv, func);
    return replaced;
  }

  std::vector<std::string> ParameterNames() const override {
    using namespace std::string_view_literals;
    return utils::ListTableKeys(config_, ""sv);
  }

  bool GetBoolean(std::string_view key) const override {
    return utils::ConfigLookupScalar<bool>(config_, key,
                                           /*allow_default=*/false);
  }

  bool GetBooleanOrDefault(std::string_view key,
                           bool default_val) const override {
    return utils::ConfigLookupScalar<bool>(config_, key, /*allow_default=*/true,
                                           default_val);
  }

  double GetDouble(std::string_view key) const override {
    return utils::ConfigLookupScalar<double>(config_, key,
                                             /*allow_default=*/false);
  }

  double GetDoubleOrDefault(std::string_view key,
                            double default_val) const override {
    return utils::ConfigLookupScalar<double>(
        config_, key, /*allow_default=*/true, default_val);
  }

  int32_t GetInteger32(std::string_view key) const override {
    return utils::ConfigLookupScalar<int32_t>(config_, key,
                                              /*allow_default=*/false);
  }

  int32_t GetInteger32OrDefault(std::string_view key,
                                int32_t default_val) const override {
    return utils::ConfigLookupScalar<int32_t>(
        config_, key, /*allow_default=*/true, default_val);
  }

  int64_t GetInteger64(std::string_view key) const override {
    return utils::ConfigLookupScalar<int64_t>(config_, key,
                                              /*allow_default=*/false);
  }

  int64_t GetInteger64OrDefault(std::string_view key,
                                int64_t default_val) const override {
    return utils::ConfigLookupScalar<int64_t>(
        config_, key, /*allow_default=*/true, default_val);
  }

  std::string GetString(std::string_view key) const override {
    return utils::ConfigLookupString(config_, key, /*allow_default=*/false);
  }

  std::string GetStringOrDefault(std::string_view key,
                                 std::string_view default_val) const override {
    return utils::ConfigLookupString(config_, key, /*allow_default=*/true,
                                     default_val);
  }

  std::vector<double> GetDoubleList(std::string_view key) const override {
    return utils::GetScalarList<double>(config_, key);
  }

  std::vector<int32_t> GetInteger32List(std::string_view key) const override {
    return utils::GetScalarList<int32_t>(config_, key);
  }

  std::vector<int64_t> GetInteger64List(std::string_view key) const override {
    return utils::GetScalarList<int64_t>(config_, key);
  }

  std::vector<std::string> GetStringList(std::string_view key) const override {
    return utils::GetScalarList<std::string>(config_, key);
  }

  std::vector<std::tuple<int32_t, int32_t>> GetPoints2D(
      std::string_view key) const override {
    return utils::GetPoints<std::tuple<int32_t, int32_t>>(config_, key);
  }

  std::vector<std::tuple<int32_t, int32_t, int32_t>> GetPoints3D(
      std::string_view key) const override {
    return utils::GetPoints<std::tuple<int32_t, int32_t, int32_t>>(config_,
                                                                   key);
  }

  std::string ToTOML() const override {
    std::ostringstream repr;
    repr << config_;
    return repr.str();
  }

  std::string ToJSON() const override {
    throw std::logic_error("JSON serialization is not yet supported!");
  }

 private:
  toml::table config_{};

  bool EqualsImpl(const ConfigurationImpl *other) const {
    using namespace std::string_view_literals;
    const auto keys_this = utils::ListTableKeys(config_, ""sv);
    const auto keys_other = utils::ListTableKeys(other->config_, ""sv);
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

  // TODO registered_string_replacements_{};
  //  std::vector<std::string> path_parameters_{};
};

std::unique_ptr<Configuration> Configuration::LoadTomlString(
    std::string_view toml_string) {
  try {
    toml::table tbl = toml::parse(toml_string);
    return std::make_unique<ConfigurationImpl>(std::move(tbl));
  } catch (const toml::parse_error &err) {
    std::ostringstream msg;
    msg << "Error parsing TOML: " << err.description() << " ("
        << err.source().begin << ")!";
    WZKLOG_ERROR(msg.str());
    throw std::runtime_error(msg.str());
  }
}

std::unique_ptr<Configuration> Configuration::LoadTomlFile(
    std::string_view filename) {
  const std::string toml = werkzeugkiste::files::CatAsciiFile(filename);
  return Configuration::LoadTomlString(toml);
}

// std::unique_ptr<Configuration> Configuration::LoadJSON(std::string_view
// filename) {
//     return std::make_unique<ConfigurationImpl>(ConfigurationImpl()); // TODO
// }
}  // namespace werkzeugkiste::config
