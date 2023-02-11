// NOLINTBEGIN
#define TOML_ENABLE_FORMATTERS 1
#include <toml++/toml.h>
// NOLINTEND

#include <werkzeugkiste/config/casts.h>
#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/files/fileio.h>
#include <werkzeugkiste/files/filesys.h>
#include <werkzeugkiste/logging.h>
#include <werkzeugkiste/strings/strings.h>

#include <array>
#include <functional>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace werkzeugkiste::config {
namespace utils {
// NOLINTNEXTLINE(*macro-usage)
#define WZK_CONFIG_LOOKUP_RAISE_TOML_TYPE_ERROR(KEY, NODE, TYPE) \
  do {                                                           \
    std::string msg{"Invalid type `"};                           \
    msg += TypeName<TYPE>();                                     \
    msg += "` used to query key `";                              \
    msg += (KEY);                                                \
    msg += "`, which is of type `";                              \
    msg += TomlTypeName((NODE), (KEY));                          \
    msg += "`!";                                                 \
    throw TypeError{msg};                                        \
  } while (false)

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

/// Returns the "fully-qualified TOML path" for the given array index.
/// For example, `path = section1.arr` & `array_index = 3` results
/// in `section1.arr[3]`.
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
                                       std::string_view path,
                                       bool include_array_entries);

/// Returns all fully-qualified paths for named parameters within
/// the given TOML array.
// NOLINTNEXTLINE(misc-no-recursion)
std::vector<std::string> ListArrayKeys(const toml::array &arr,
                                       std::string_view path,
                                       bool include_array_entries) {
  std::vector<std::string> keys;
  std::size_t array_index = 0;
  for (auto &&value : arr) {
    const std::string fqn = FullyQualifiedArrayElementPath(array_index, path);
    if (include_array_entries) {
      keys.push_back(fqn);
    }
    if (value.is_table()) {
      const toml::table &tbl = *value.as_table();
      const auto subkeys = ListTableKeys(tbl, fqn, include_array_entries);
      keys.insert(keys.end(), subkeys.begin(), subkeys.end());
    }
    if (value.is_array()) {
      const toml::array &nested_arr = *value.as_array();
      const auto subkeys =
          ListArrayKeys(nested_arr, fqn, include_array_entries);
      keys.insert(keys.end(), subkeys.begin(), subkeys.end());
    }
    ++array_index;
  }
  return keys;
}

/// Returns all fully-qualified paths for named parameters within
/// the given TOML table.
// NOLINTNEXTLINE(misc-no-recursion)
std::vector<std::string> ListTableKeys(const toml::table &tbl,
                                       std::string_view path,
                                       bool include_array_entries) {
  std::vector<std::string> keys;
  for (auto &&[key, value] : tbl) {
    // Each parameter within a table is a "named parameter", i.e. it has
    // a separate name that should always be included.
    keys.emplace_back(FullyQualifiedPath(key, path));
    if (value.is_array()) {
      const auto subkeys =
          ListArrayKeys(*value.as_array(), FullyQualifiedPath(key, path),
                        include_array_entries);
      keys.insert(keys.end(), subkeys.begin(), subkeys.end());
    }
    if (value.is_table()) {
      const auto subkeys =
          ListTableKeys(*value.as_table(), FullyQualifiedPath(key, path),
                        include_array_entries);
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
    // LCOV_EXCL_START
    std::string msg{
        "Traverse() can only be invoked with either `table` or "
        "`array` nodes, but `"};
    msg += path;
    msg += "` is neither!";
    throw std::logic_error{msg};
    // LCOV_EXCL_STOP
  }
}

/// Casts the value if the given 64-bit integer can be safely cast into a
/// 32-bit integer. Otherwise, a std::domain_error will be thrown.
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
    throw TypeError{msg};
  }

  return static_cast<int32_t>(value64);
}

/// Utility to print the type name of a toml::node/toml::node_view.
template <typename NodeView>
inline std::string TomlTypeName(const NodeView &node, std::string_view key) {
  if (node.is_table()) {
    return "group";
  }

  if (node.is_array()) {
    return "list";
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
  throw std::logic_error{msg};
  // LCOV_EXCL_STOP
}

/// Returns true if the TOML table contains a valid node at the given,
/// fully-qualified path/key.
inline bool ConfigContainsKey(const toml::table &tbl, std::string_view key) {
  // Needed, because `tbl.contains()` only checks the direct children.
  const auto node = tbl.at_path(key);
  return node.is_value() || node.is_table() || node.is_array();
}

/// Extracts the value from the toml::node or throws an error if the type
/// is not correct.
/// Tries converting numeric types if a lossless cast is feasible.
template <typename T, typename NodeView>
T CastScalar(const NodeView &node, std::string_view key) {
  static_assert(std::is_arithmetic_v<bool>,
                "Boolean must be a number type to use numeric casts for "
                "parameter extraction!");

  if constexpr (std::is_same_v<T, bool>) {
    if (node.is_boolean()) {
      return static_cast<bool>(*node.as_boolean());
    }
  } else if constexpr (std::is_arithmetic_v<T>) {
    if (node.is_integer()) {
      return CheckedCast<T, int64_t, TypeError>(
          static_cast<int64_t>(*node.as_integer()));
    } else if (node.is_floating_point()) {
      return CheckedCast<T, double, TypeError>(
          static_cast<double>(*node.as_floating_point()));
    }
  } else if constexpr (std::is_same_v<T, std::string>) {
    if (node.is_string()) {
      return std::string{*node.as_string()};
    }
  } else {
    // TODO This method could be extended to handle date/time
    throw std::logic_error("Type not yet supported!");
  }
  WZK_CONFIG_LOOKUP_RAISE_TOML_TYPE_ERROR(key, node, T);
}

/// Looks up the value at the given key (fully-qualified TOML path).
/// If the key does not exist, a KeyError will be raised unless
/// `allow_default` is true (in which case the `default_val` will be
/// returned instead).
template <typename T, typename DefaultType = T>
T ConfigLookupScalar(const toml::table &tbl, std::string_view key,
                     bool allow_default = false,
                     DefaultType default_val = DefaultType{}) {
  if (!ConfigContainsKey(tbl, key)) {
    if (allow_default) {
      return T{default_val};
    }

    throw werkzeugkiste::config::KeyError(key);
  }

  const auto node = tbl.at_path(key);
  return CastScalar<T>(node, key);
}

/// Looks up the value at the given key (fully-qualified TOML path).
/// If the key does not exist, a nullopt will be returned.
template <typename T>
std::optional<T> ConfigLookupOptional(const toml::table &tbl,
                                      std::string_view key) {
  if (!ConfigContainsKey(tbl, key)) {
    return std::nullopt;
  }

  const auto node = tbl.at_path(key);
  return CastScalar<T>(node, key);
}

/// Splits a fully-qualified TOML path into <anchestor, child>.
/// This does *not* handle arrays!
inline std::pair<std::string_view, std::string_view> SplitTomlPath(
    std::string_view path) {
  // TODO This doesn't work for array elements. Currently, this is
  // not an issue, because we don't allow creating array elements (as
  // there is no need to do so). If this requirement changes, make sure
  // to support "fancy" paths, such as "arr[3][0][1].internal.array[0]".
  std::size_t pos = path.find_last_of('.');
  if (pos != std::string_view::npos) {
    return std::make_pair(path.substr(0, pos), path.substr(pos + 1));
  }

  return std::make_pair(std::string_view{}, path);
}

/// Creates all missing nodes (tables).
/// For example, if `key = "path.to.a.value"`, then this recursive
/// function will create 4 tables: "path", "path.to", "path.to.a", and
/// "path.to.a.value".
// NOLINTNEXTLINE(misc-no-recursion)
void EnsureContainerPathExists(toml::table &tbl, std::string_view key) {
  if (key.empty()) {
    // We've reached the root node.
    return;
  }

  const auto node = tbl.at_path(key);
  if (node.is_table() || node.is_array()) {
    // Node exists and is a container.
    return;
  }

  if (node.is_value()) {
    // Node exists, but is a scalar value.
    std::string msg{
        "Invalid key: The path anchestors must consist of tables/arrays, but "
        "`"};
    msg += key;
    msg += "` is of type `";
    msg += TomlTypeName(node, key);
    msg += "`!";
    throw TypeError{msg};
  }

  // Parent does not exist. We now have to recursively create the
  // parent path, then create a table here.
  // But first, ensure that we are not asked to create an array:
  const auto path = SplitTomlPath(key);
  if (path.second.find('[') != std::string_view::npos) {
    std::string msg{
        "Cannot create the requested configuration hierarchy. Creating an "
        "array at `"};
    msg += key;
    msg += "` is not supported!";
    throw TypeError{msg};
  }

  if (path.first.empty()) {
    // Create table at root level.
    tbl.emplace(key, toml::table{});
  } else {
    // Create anchestor path.
    EnsureContainerPathExists(tbl, path.first);
    const auto &parent = tbl.at_path(path.first);
    if (parent.is_table()) {
      parent.as_table()->emplace(path.second, toml::table{});
    } else {
      // Would need to parse the array element index from the key.
      // node.as_array()->emplace(path.second, toml::table{});
      std::string msg{
          "Creating a table as a child of an array is not supported! Check "
          "configuration parameter `"};
      msg += key;
      msg += "`!";
      throw TypeError{msg};
    }
  }
}

/// @brief Allows setting a TOML parameter to an int64, double, bool, or string.
/// @tparam T Scalar TOML type to be set.
/// @tparam TMessage Only needed to avoid separate int32 specialization (as
/// internally, all integers are stored as 64-bit; if there would be an error,
/// we don't want to show a confusing "user provided 64-bit" error message).
/// @param tbl The "root" table.
/// @param key The fully-qualified "TOML path".
/// @param value The value to be set.
template <typename T, typename TMessage = T, typename TValue>
void ConfigSetScalar(toml::table &tbl, std::string_view key, TValue value) {
  const auto path = SplitTomlPath(key);
  if (ConfigContainsKey(tbl, key)) {
    const auto node = tbl.at_path(key);
    if (!node.is<T>()) {
      std::string msg{"Changing the type is not allowed. Parameter `"};
      msg += key;
      msg += "` is `";
      msg += TomlTypeName(node, key);
      msg += "`, but scalar is of type `";
      msg += TypeName<TMessage>();
      msg += "`!";
      throw TypeError{msg};
    }

    auto &ref = *node.as<T>();
    ref = T{value};
  } else {
    EnsureContainerPathExists(tbl, path.first);
    toml::table *parent =
        path.first.empty() ? &tbl : tbl.at_path(path.first).as_table();
    if (parent == nullptr) {
      // LCOV_EXCL_START
      std::ostringstream msg;
      msg << "Creating the path hierarchy for `" << key
          << "` completed without failure, but the parent table `" << path.first
          << "` is a nullptr. This must be a bug. Please report at"
             " https://github.com/snototter/werkzeugkiste/issues";
      throw std::logic_error{msg.str()};
      // LCOV_EXCL_STOP
    }

    auto result = parent->insert_or_assign(path.second, value);
    if (!ConfigContainsKey(tbl, key)) {
      // LCOV_EXCL_START
      std::ostringstream msg;
      msg << "Assigning `" << key << "` = `" << value
          << "` completed without failure, but the key cannot be looked up. "
             "The value should have been "
          << (result.second ? "inserted" : "assigned") << " at parent{`"
          << path.first << "`}, current{`" << path.second
          << "`}. This must be a bug. Please report at "
             "https://github.com/snototter/werkzeugkiste/issues";
      throw std::logic_error{msg.str()};
      // LCOV_EXCL_STOP
    }
  }
}

/// Utility to turn a std::array into a std::tuple.
template <typename Array, std::size_t... Idx>
inline auto ArrayToTuple(const Array &arr,
                         std::index_sequence<Idx...> /* indices */) {
  return std::make_tuple(arr[Idx]...);
}

/// Utility to turn a std::array into a std::tuple.
template <typename Type, std::size_t Num>
inline auto ArrayToTuple(const std::array<Type, Num> &arr) {
  return ArrayToTuple(arr, std::make_index_sequence<Num>{});
}

/// Extracts a single Dim-dimensional point from the given
/// toml::array into the `point` std::array.
template <typename Type, std::size_t Dim>
inline void ExtractPointFromTOMLArray(const toml::array &arr,
                                      std::string_view key,
                                      std::array<Type, Dim> &point) {
  // The array must have at least Dim entries - more are also allowed, as
  // they will just be ignored.
  if (arr.size() < Dim) {
    std::ostringstream msg;
    msg << "Invalid parameter `" << key << "`. Cannot extract a " << Dim
        << "D point from a " << arr.size() << "-element array!";
    throw TypeError{msg.str()};
  }

  // The first Dim entries of the array must be of the
  // correct data type in order to extract them for the point.
  for (std::size_t idx = 0; idx < Dim; ++idx) {
    if (arr[idx].is_number()) {
      if (!arr[idx].is<Type>()) {
        std::ostringstream msg;
        msg << "Invalid parameter `" << key << "`. Dimension [" << idx
            << "] is `" << TomlTypeName(arr, key) << "` instead of `"
            << TypeName<Type>() << "`!";
        throw TypeError{msg.str()};
      }
      point[idx] = Type{*arr[idx].as<Type>()};
    }
  }
}

/// Extracts a single Dim-dimensional point as std::tuple from the given
/// toml::array.
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

/// Extracts a single Dim-dimensional point from the given
/// toml::table into the `point` std::array. The table must
/// have "x", "y", ... entries which are used to look up the
/// corresponding point coordinates.
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
      msg << "Invalid parameter `" << key
          << "`. Table entry does not specify the `" << point_keys[idx]
          << "` coordinate!";
      throw TypeError{msg.str()};
    }

    if (!tbl[point_keys[idx]].is<Type>()) {
      std::ostringstream msg;
      msg << "Invalid parameter `" << key << "`. Dimension `" << point_keys[idx]
          << "` is `" << TomlTypeName(tbl[point_keys[idx]], key)
          << "` instead of `" << TypeName<Type>() << "`!";
      throw TypeError{msg.str()};
    }

    point[idx] = Type{*tbl[point_keys[idx]].as<Type>()};
  }
}

/// Extracts a single Dim-dimensional point as std::tuple from the given
/// toml::table.
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

/// Extracts a list of points (a polyline) of integer or double.
template <typename Tuple>
std::vector<Tuple> GetPoints(const toml::table &tbl, std::string_view key) {
  if (!ConfigContainsKey(tbl, key)) {
    throw werkzeugkiste::config::KeyError(key);
  }

  const auto node = tbl.at_path(key);
  if (!node.is_array()) {
    std::string msg{"Invalid point list configuration: `"};
    msg += key;
    msg += "` must be an array, but is of type `";
    msg += TomlTypeName(node, key);
    msg += "`!";
    throw TypeError{msg};
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
      throw TypeError{msg};
    }
    ++arr_index;
  }
  return poly;
}

/// Extracts a list of built-in scalar types (integer, double, bool).
/// This is *not suitable* for TOML++-specific types (date, time, ...)
template <typename T>
std::vector<T> GetScalarList(const toml::table &tbl, std::string_view key) {
  if (!ConfigContainsKey(tbl, key)) {
    throw werkzeugkiste::config::KeyError(key);
  }

  const auto &node = tbl.at_path(key);
  if (!node.is_array()) {
    std::string msg{"Invalid list configuration: Parameter `"};
    msg += key;
    msg += "` must be an array, but is `";
    msg += TomlTypeName(node, key);
    msg += "`!";
    throw TypeError{msg};
  }

  const toml::array &arr = *node.as_array();
  std::size_t arr_index = 0;
  std::vector<T> scalars{};
  for (auto &&value : arr) {
    const auto fqn = FullyQualifiedArrayElementPath(arr_index, key);
    if (value.is_value()) {
      scalars.push_back(CastScalar<T>(value, fqn));
    } else {
      std::string msg{"Invalid list configuration `"};
      msg += key;
      msg += "`: All entries must be of scalar type `";
      msg += TypeName<T>();
      msg += "`, but `";
      msg += fqn;
      msg += "` is not!";
      throw TypeError{msg};
    }
    ++arr_index;
  }
  return scalars;
}

/// Extracts a pair of built-in scalar types (integer, double, bool).
/// This is *not suitable* for TOML++-specific types (date, time, ...)
template <typename T>
std::pair<T, T> GetScalarPair(const toml::table &tbl, std::string_view key) {
  if (!ConfigContainsKey(tbl, key)) {
    throw werkzeugkiste::config::KeyError(key);
  }

  const auto &node = tbl.at_path(key);
  if (!node.is_array()) {
    std::string msg{"Invalid pair configuration: Parameter `"};
    msg += key;
    msg += "` must be an array, but is `";
    msg += TomlTypeName(node, key);
    msg += "`!";
    throw TypeError{msg};
  }

  const toml::array &arr = *node.as_array();
  if (arr.size() != 2) {
    std::string msg{"Invalid pair configuration: Parameter `"};
    msg += key;
    msg += "` must be a 2-element array, but has ";
    msg += std::to_string(arr.size());
    msg += ((arr.size() == 1) ? " element!" : " elements!");
    throw TypeError{msg};
  }

  std::size_t arr_index = 0;
  std::array<T, 2> scalars{};
  for (auto &&value : arr) {
    const auto fqn = FullyQualifiedArrayElementPath(arr_index, key);
    if (value.is_value()) {
      scalars[arr_index] = CastScalar<T>(value, fqn);
    } else {
      std::string msg{"Invalid pair configuration `"};
      msg += key;
      msg += "`: Both entries must be of scalar type `";
      msg += TypeName<T>();
      msg += "`, but `";
      msg += fqn;
      msg += "` is not!";
      throw TypeError{msg};
    }
    ++arr_index;
  }
  return std::make_pair(scalars[0], scalars[1]);
}

#undef WZK_CONFIG_LOOKUP_RAISE_TOML_TYPE_ERROR
}  // namespace utils

// Abusing the PImpl idiom to hide the internally used TOML table.
struct Configuration::Impl {
  toml::table config_root{};
};

Configuration::Configuration() : pimpl_{new Impl{}} {}

Configuration::~Configuration() = default;

Configuration::Configuration(const Configuration &other)
    : pimpl_{std::make_unique<Impl>(*other.pimpl_)} {}

Configuration::Configuration(Configuration &&other) noexcept {
  std::swap(pimpl_, other.pimpl_);
}

Configuration &Configuration::operator=(const Configuration &other) {
  if (this != &other) {
    pimpl_ = std::make_unique<Impl>(*other.pimpl_);
  }
  return *this;
}

Configuration &Configuration::operator=(Configuration &&other) noexcept {
  if (this != &other) {
    std::swap(pimpl_, other.pimpl_);
  }
  return *this;
}

Configuration Configuration::LoadTOMLString(std::string_view toml_string) {
  try {
    toml::table tbl = toml::parse(toml_string);
    Configuration cfg{};
    cfg.pimpl_->config_root = std::move(tbl);
    return cfg;
  } catch (const toml::parse_error &err) {
    std::ostringstream msg;
    msg << err.description() << " (" << err.source().begin << ")!";
    throw ParseError(msg.str());
  }
}

Configuration Configuration::LoadTOMLFile(std::string_view filename) {
  try {
    return Configuration::LoadTOMLString(files::CatAsciiFile(filename));
  } catch (const werkzeugkiste::files::IOError &e) {
    throw ParseError(e.what());
  }
}

bool Configuration::Empty() const {
  return (pimpl_ == nullptr) || (pimpl_->config_root.empty());
}

bool Configuration::Equals(const Configuration &other) const {
  using namespace std::string_view_literals;
  const auto keys_this = utils::ListTableKeys(pimpl_->config_root, ""sv,
                                              /*include_array_entries=*/true);
  const auto keys_other = utils::ListTableKeys(other.pimpl_->config_root, ""sv,
                                               /*include_array_entries=*/true);

  if (keys_this.size() != keys_other.size()) {
    return false;
  }

  for (const auto &key : keys_this) {
    const auto nv_this = pimpl_->config_root.at_path(key);
    const auto nv_other = other.pimpl_->config_root.at_path(key);
    if (nv_this != nv_other) {
      return false;
    }
  }

  return true;
}

bool Configuration::Contains(std::string_view key) const {
  return utils::ConfigContainsKey(pimpl_->config_root, key);
}

ConfigType Configuration::Type(std::string_view key) const {
  const auto nv = pimpl_->config_root.at_path(key);
  switch (nv.type()) {
    case toml::node_type::none:
      throw werkzeugkiste::config::KeyError(key);

    case toml::node_type::table:
      return ConfigType::Group;

    case toml::node_type::array:
      return ConfigType::List;

    case toml::node_type::string:
      return ConfigType::String;

    case toml::node_type::integer:
      return ConfigType::Integer;

    case toml::node_type::floating_point:
      return ConfigType::FloatingPoint;

    case toml::node_type::boolean:
      return ConfigType::Boolean;

      // TODO date, time, date_time

    default: {
      std::string msg{"TOML node type `"};
      msg += utils::TomlTypeName(nv, key);
      msg += "` is not yet handled in `Configuration::Type`!";
      throw std::logic_error(msg);
    }
  }
}

std::vector<std::string> Configuration::ListParameterNames(
    bool include_array_entries) const {
  using namespace std::string_view_literals;
  return utils::ListTableKeys(pimpl_->config_root, ""sv, include_array_entries);
}

//---------------------------------------------------------------------------
// Scalar data types

bool Configuration::GetBoolean(std::string_view key) const {
  return utils::ConfigLookupScalar<bool>(pimpl_->config_root, key,
                                         /*allow_default=*/false);
}

bool Configuration::GetBooleanOr(std::string_view key, bool default_val) const {
  return utils::ConfigLookupScalar<bool>(pimpl_->config_root, key,
                                         /*allow_default=*/true, default_val);
}

std::optional<bool> Configuration::GetOptionalBoolean(
    std::string_view key) const {
  return utils::ConfigLookupOptional<bool>(pimpl_->config_root, key);
}

void Configuration::SetBoolean(std::string_view key, bool value) {
  utils::ConfigSetScalar<bool>(pimpl_->config_root, key, value);
}

int32_t Configuration::GetInteger32(std::string_view key) const {
  return utils::ConfigLookupScalar<int32_t>(pimpl_->config_root, key,
                                            /*allow_default=*/false);
}

int32_t Configuration::GetInteger32Or(std::string_view key,
                                      int32_t default_val) const {
  return utils::ConfigLookupScalar<int32_t>(
      pimpl_->config_root, key, /*allow_default=*/true, default_val);
}

std::optional<int32_t> Configuration::GetOptionalInteger32(
    std::string_view key) const {
  return utils::ConfigLookupOptional<int32_t>(pimpl_->config_root, key);
}

void Configuration::SetInteger32(std::string_view key, int32_t value) {
  utils::ConfigSetScalar<int64_t, int32_t>(pimpl_->config_root, key,
                                           static_cast<int64_t>(value));
}

int64_t Configuration::GetInteger64(std::string_view key) const {
  return utils::ConfigLookupScalar<int64_t>(pimpl_->config_root, key,
                                            /*allow_default=*/false);
}

int64_t Configuration::GetInteger64Or(std::string_view key,
                                      int64_t default_val) const {
  return utils::ConfigLookupScalar<int64_t>(
      pimpl_->config_root, key, /*allow_default=*/true, default_val);
}

std::optional<int64_t> Configuration::GetOptionalInteger64(
    std::string_view key) const {
  return utils::ConfigLookupOptional<int64_t>(pimpl_->config_root, key);
}

void Configuration::SetInteger64(std::string_view key, int64_t value) {
  utils::ConfigSetScalar<int64_t>(pimpl_->config_root, key, value);
}

double Configuration::GetDouble(std::string_view key) const {
  return utils::ConfigLookupScalar<double>(pimpl_->config_root, key,
                                           /*allow_default=*/false);
}

double Configuration::GetDoubleOr(std::string_view key,
                                  double default_val) const {
  return utils::ConfigLookupScalar<double>(pimpl_->config_root, key,
                                           /*allow_default=*/true, default_val);
}

std::optional<double> Configuration::GetOptionalDouble(
    std::string_view key) const {
  return utils::ConfigLookupOptional<double>(pimpl_->config_root, key);
}

void Configuration::SetDouble(std::string_view key, double value) {
  utils::ConfigSetScalar<double>(pimpl_->config_root, key, value);
}

std::string Configuration::GetString(std::string_view key) const {
  using namespace std::string_view_literals;
  return utils::ConfigLookupScalar<std::string, std::string_view>(
      pimpl_->config_root, key, /*allow_default=*/false, ""sv);
}

std::string Configuration::GetStringOr(std::string_view key,
                                       std::string_view default_val) const {
  return utils::ConfigLookupScalar<std::string, std::string_view>(
      pimpl_->config_root, key, /*allow_default=*/true, default_val);
}

std::optional<std::string> Configuration::GetOptionalString(
    std::string_view key) const {
  return utils::ConfigLookupOptional<std::string>(pimpl_->config_root, key);
}

void Configuration::SetString(std::string_view key, std::string_view value) {
  utils::ConfigSetScalar<std::string, std::string, std::string_view>(
      pimpl_->config_root, key, value);
}

//---------------------------------------------------------------------------
// Lists/pairs of scalar data types

std::pair<double, double> Configuration::GetDoublePair(
    std::string_view key) const {
  return utils::GetScalarPair<double>(pimpl_->config_root, key);
}

std::vector<double> Configuration::GetDoubleList(std::string_view key) const {
  return utils::GetScalarList<double>(pimpl_->config_root, key);
}

std::pair<int32_t, int32_t> Configuration::GetInteger32Pair(
    std::string_view key) const {
  return utils::GetScalarPair<int32_t>(pimpl_->config_root, key);
}

std::vector<int32_t> Configuration::GetInteger32List(
    std::string_view key) const {
  return utils::GetScalarList<int32_t>(pimpl_->config_root, key);
}

std::pair<int64_t, int64_t> Configuration::GetInteger64Pair(
    std::string_view key) const {
  return utils::GetScalarPair<int64_t>(pimpl_->config_root, key);
}

std::vector<int64_t> Configuration::GetInteger64List(
    std::string_view key) const {
  return utils::GetScalarList<int64_t>(pimpl_->config_root, key);
}

std::vector<std::string> Configuration::GetStringList(
    std::string_view key) const {
  return utils::GetScalarList<std::string>(pimpl_->config_root, key);
}

std::vector<std::tuple<int32_t, int32_t>> Configuration::GetPoints2D(
    std::string_view key) const {
  return utils::GetPoints<std::tuple<int32_t, int32_t>>(pimpl_->config_root,
                                                        key);
}

std::vector<std::tuple<int32_t, int32_t, int32_t>> Configuration::GetPoints3D(
    std::string_view key) const {
  return utils::GetPoints<std::tuple<int32_t, int32_t, int32_t>>(
      pimpl_->config_root, key);
}

Configuration Configuration::GetGroup(std::string_view key) const {
  if (!Contains(key)) {
    throw werkzeugkiste::config::KeyError(key);
  }

  Configuration cfg;
  const auto nv = pimpl_->config_root.at_path(key);

  if (!nv.is_table()) {
    std::string msg{"Cannot retrieve `"};
    msg += key;
    msg += "` as a group, because it is a`";
    msg += utils::TomlTypeName(nv, key);
    msg += "`!";
    throw TypeError{msg};
  }

  const toml::table &tbl = *nv.as_table();
  cfg.pimpl_->config_root = tbl;
  return cfg;
}

void Configuration::SetGroup(std::string_view key, const Configuration &group) {
  if (key.empty()) {
    throw KeyError{
        "Cannot replace this configuration with a parameter group. Key cannot "
        "be empty in `SetGroup`!"};
  }

  const auto path = utils::SplitTomlPath(key);
  if (utils::ConfigContainsKey(pimpl_->config_root, key)) {
    const auto node = pimpl_->config_root.at_path(key);
    if (!node.is_table()) {
      std::string msg{"Cannot insert parameter group at `"};
      msg += key;
      msg += "`. Existing parameter is of type `";
      msg += utils::TomlTypeName(node, key);
      msg += "`!";
      throw TypeError{msg};
    }

    auto &ref = *node.as_table();
    ref = group.pimpl_->config_root;
  } else {
    utils::EnsureContainerPathExists(pimpl_->config_root, path.first);
    toml::table *parent =
        path.first.empty() ? &pimpl_->config_root
                           : pimpl_->config_root.at_path(path.first).as_table();
    if (parent == nullptr) {
      // LCOV_EXCL_START
      std::ostringstream msg;
      msg << "Creating the path hierarchy to insert parameter group at `" << key
          << "` completed without failure, but the parent table `" << path.first
          << "` is a nullptr. This must be a bug. Please report at"
             " https://github.com/snototter/werkzeugkiste/issues";
      throw std::logic_error{msg.str()};
      // LCOV_EXCL_STOP
    }

    auto result =
        parent->insert_or_assign(path.second, group.pimpl_->config_root);
    if (!utils::ConfigContainsKey(pimpl_->config_root, key)) {
      // LCOV_EXCL_START
      std::ostringstream msg;
      msg << "Assigning parameter group to `" << key
          << "` completed without failure, but the key cannot be looked up. "
             "The value should have been "
          << (result.second ? "inserted" : "assigned")
          << ". This must be a bug. Please report at "
             "https://github.com/snototter/werkzeugkiste/issues";
      throw std::logic_error{msg.str()};
      // LCOV_EXCL_STOP
    }
  }
}

//---------------------------------------------------------------------------
// Special utilities

bool Configuration::AdjustRelativePaths(
    std::string_view base_path,
    const std::vector<std::string_view> &parameters) {
  using namespace std::string_view_literals;
  KeyMatcher matcher{parameters};
  auto to_replace = [matcher](std::string_view fqn) -> bool {
    return matcher.Match(fqn);
  };

  bool replaced{false};
  bool *rep_ptr = &replaced;
  auto func = [rep_ptr, to_replace, base_path](toml::node &node,
                                               std::string_view fqn) -> void {
    if (to_replace(fqn)) {
      // Ensure that the provided key/pattern did not pick up a wrong node by
      // mistake:
      if (!node.is_string()) {
        std::string msg{"Inside `EnsureAbsolutePaths()`, path parameter `"};
        msg += fqn;
        msg += "` must be a string, but is `";
        msg += utils::TomlTypeName(node, fqn);
        msg += "`!";
        throw TypeError{msg};
      }

      // Check if the path is relative
      const std::string param_str = utils::CastScalar<std::string>(node, fqn);
      const bool is_file_url = strings::StartsWith(param_str, "file://");
      // NOLINTNEXTLINE(*magic-numbers)
      std::string path = is_file_url ? param_str.substr(7) : param_str;

      if (!files::IsAbsolute(path)) {
        auto &str = *node.as_string();
        const std::string abspath = files::FullFile(base_path, path);
        if (is_file_url) {
          str = "file://" + abspath;
        } else {
          str = abspath;
        }
        *rep_ptr = true;
      }
    }
  };
  utils::Traverse(pimpl_->config_root, ""sv, func);
  return replaced;
}

bool Configuration::ReplaceStringPlaceholders(
    const std::vector<std::pair<std::string_view, std::string_view>>
        &replacements) {
  // Sanity check, search string can't be empty
  for (const auto &rep : replacements) {
    if (rep.first.empty()) {
      throw std::runtime_error{
          "Search string within `ReplaceStrings()` must not be empty!"};
    }
  }

  using namespace std::string_view_literals;
  bool replaced{false};
  bool *rep_ptr = &replaced;
  auto func = [rep_ptr, replacements](toml::node &node,
                                      std::string_view fqn) -> void {
    if (node.is_string()) {
      std::string param_str = utils::CastScalar<std::string>(node, fqn);
      bool matched{false};
      for (const auto &rep : replacements) {
        std::size_t pos{0};
        while ((pos = param_str.find(rep.first, pos)) != std::string::npos) {
          param_str.replace(pos, rep.first.length(), rep.second);
          matched = true;
        }
      }

      if (matched) {
        auto &str = *node.as_string();
        str = param_str;
        *rep_ptr = true;
      }
    }
  };
  utils::Traverse(pimpl_->config_root, ""sv, func);
  return replaced;
}

void Configuration::LoadNestedTOMLConfiguration(std::string_view key) {
  // TODO refactor (TOML/JSON --> function handle)

  if (!utils::ConfigContainsKey(pimpl_->config_root, key)) {
    throw werkzeugkiste::config::KeyError(key);
  }

  const auto &node = pimpl_->config_root.at_path(key);
  if (!node.is_string()) {
    std::string msg{"Parameter `"};
    msg += key;
    msg += "` to load a nested configuration must be a string, but is `";
    msg += utils::TomlTypeName(node, key);
    msg += "`!";
    throw TypeError{msg};
  }

  // To replace the node, we first have to remove it.
  std::string fname = std::string(*node.as_string());

  try {
    auto nested_tbl = toml::parse_file(fname);

    const auto path = utils::SplitTomlPath(key);

    toml::table *parent =
        path.first.empty() ? &pimpl_->config_root
                           : pimpl_->config_root.at_path(path.first).as_table();
    if ((parent == nullptr) || (path.second[path.second.length() - 1] == ']')) {
      std::string msg{"The parent of parameter `"};
      msg += key;
      msg +=
          "` to load a nested configuration must be the root or a table "
          "node!";
      throw TypeError{msg};
    }

    parent->erase(path.second);
    const auto result = parent->emplace(path.second, std::move(nested_tbl));

    if (!result.second) {
      // LCOV_EXCL_START
      std::string msg{"Could not insert nested configuration at `"};
      msg += key;
      msg += "`!";
      throw std::runtime_error{msg};
      // LCOV_EXCL_STOP
    }
  } catch (const toml::parse_error &err) {
    std::ostringstream msg;
    msg << "Error parsing TOML from \"" << fname << "\": " << err.description()
        << " (" << err.source().begin << ")!";
    throw ParseError{msg.str()};
  }
}

std::string Configuration::ToTOML() const {
  std::ostringstream repr;
  repr << toml::toml_formatter{pimpl_->config_root};
  return repr.str();
}

std::string Configuration::ToJSON() const {
  std::ostringstream repr;
  repr << toml::json_formatter{pimpl_->config_root};
  return repr.str();
}

}  // namespace werkzeugkiste::config
