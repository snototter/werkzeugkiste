// NOLINTBEGIN
#define TOML_ENABLE_FORMATTERS 1
#include <toml++/toml.h>
// NOLINTEND

#include <werkzeugkiste/config/casts.h>
#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/config/keymatcher.h>
#include <werkzeugkiste/container/sort.h>
#include <werkzeugkiste/files/fileio.h>
#include <werkzeugkiste/files/filesys.h>
#include <werkzeugkiste/logging.h>
#include <werkzeugkiste/strings/strings.h>

#include <array>
#include <functional>
#include <limits>
#include <optional>
#include <sstream>
#include <utility>
#include <vector>

namespace werkzeugkiste::config {
// NOLINTNEXTLINE(*macro-usage)
#define WZK_CONFIG_LOOKUP_RAISE_PATH_CREATION_ERROR(KEY, PARENT)          \
  do {                                                                    \
    std::ostringstream msg;                                               \
    msg << "Creating the path hierarchy to insert parameter group at `"   \
        << (KEY) << "` completed without failure, but the parent table `" \
        << (PARENT)                                                       \
        << "` is a nullptr. This must be a bug. Please report at "        \
           "https://github.com/snototter/werkzeugkiste/issues";           \
    throw std::logic_error{msg.str()};                                    \
  } while (false)

// NOLINTNEXTLINE(*macro-usage)
#define WZK_CONFIG_LOOKUP_RAISE_ASSIGNMENT_ERROR(                   \
    KEY, INSERTED, PARENT, CURRENT)                                 \
  do {                                                              \
    std::ostringstream msg;                                         \
    msg << "Assigning `" << (KEY)                                   \
        << "` completed without failure, but the key cannot "       \
           "be looked up. The value should have been "              \
        << ((INSERTED) ? "inserted" : "assigned") << " at parent(`" \
        << (PARENT) << "`), child(`" << (CURRENT)                   \
        << "`). This must be a bug. Please report at "              \
           "https://github.com/snototter/werkzeugkiste/issues";     \
    throw std::logic_error{msg.str()};                              \
  } while (false)

namespace detail {
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
  return std::string(key.str());
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
    bool include_array_entries,
    bool recursive);

/// Returns all fully-qualified paths for named parameters within
/// the given TOML array.
// NOLINTNEXTLINE(misc-no-recursion)
std::vector<std::string> ListArrayKeys(const toml::array &arr,
    std::string_view path,
    bool include_array_entries,
    bool recursive) {
  std::vector<std::string> keys;
  std::size_t array_index = 0;
  for (auto &&value : arr) {
    const std::string fqn = FullyQualifiedArrayElementPath(array_index, path);
    if (include_array_entries) {
      keys.push_back(fqn);
    }
    if (recursive) {
      if (value.is_table()) {
        const toml::table &tbl = *value.as_table();
        const auto subkeys =
            ListTableKeys(tbl, fqn, include_array_entries, recursive);
        keys.insert(keys.end(), subkeys.begin(), subkeys.end());
      }
      if (value.is_array()) {
        const toml::array &nested_arr = *value.as_array();
        const auto subkeys =
            ListArrayKeys(nested_arr, fqn, include_array_entries, recursive);
        keys.insert(keys.end(), subkeys.begin(), subkeys.end());
      }
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
    bool include_array_entries,
    bool recursive) {
  std::vector<std::string> keys;
  for (auto &&[key, value] : tbl) {
    // Each parameter within a table is a "named parameter", i.e. it has
    // a separate name that should always be included.
    keys.push_back(FullyQualifiedPath(key, path));
    if (recursive) {
      if (value.is_array()) {
        const auto subkeys = ListArrayKeys(*value.as_array(),
            FullyQualifiedPath(key, path),
            include_array_entries,
            recursive);
        keys.insert(keys.end(), subkeys.begin(), subkeys.end());
      }
      if (value.is_table()) {
        const auto subkeys = ListTableKeys(*value.as_table(),
            FullyQualifiedPath(key, path),
            include_array_entries,
            recursive);
        keys.insert(keys.end(), subkeys.begin(), subkeys.end());
      }
    }
  }
  return keys;
}

/// @brief Prepares a KeyError with an alternative key suggestion.
/// @param tbl The configuration root node/table.
/// @param key The key which could not be found.
/// @return KeyError instance to be thrown.
KeyError KeyErrorWithSimilarKeys(const toml::table &tbl, std::string_view key) {
  constexpr std::size_t max_length_difference{5};
  constexpr std::size_t max_levenshtein_distance{3};

  using namespace std::string_view_literals;
  std::string msg{"Key `"};
  msg += key;
  msg += "` does not exist!";

  const std::vector<std::string> keys = ListTableKeys(
      tbl, ""sv, /*include_array_entries=*/true, /*recursive=*/true);
  std::vector<std::pair<std::size_t, std::string_view>> candidates;
  for (const auto &cand : keys) {
    // The edit distance can't be less than the length difference between the
    // two strings. So we can reject unsuitable keys earlier.
    if (strings::LengthDifference(key, cand) < max_length_difference) {
      const std::size_t edit_dist = strings::LevenshteinDistance(key, cand);
      if (edit_dist < max_levenshtein_distance) {
        candidates.emplace_back(edit_dist, cand);
      }
    }
  }
  if (!candidates.empty()) {
    const std::vector<std::size_t> sorted_indices =
        container::GetSortedIndices(candidates,
            [](const std::pair<std::size_t, std::string_view> &a,
                const std::pair<std::size_t, std::string_view> &b) -> bool {
              return a.first < b.first;
            });
    msg += " Did you mean: `";
    const std::size_t num_to_include =
        std::min(sorted_indices.size(), static_cast<std::size_t>(3));
    for (std::size_t idx = 0; idx < num_to_include; ++idx) {
      const auto &cand = candidates[sorted_indices[idx]];
      msg += cand.second;
      if (idx < num_to_include - 1) {
        msg += "`, `";
      }
    }
    msg += "`?";
  }

  return KeyError{msg};
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
void Traverse(toml::node &node,
    std::string_view path,
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

template <typename T>
constexpr const char *TomlTypeName() {
  if constexpr (std::is_same_v<T, toml::table>) {
    return "group";
  }

  if constexpr (std::is_same_v<T, toml::array>) {
    return "list";
  }

  if constexpr (std::is_same_v<T, int64_t>) {
    return "int";
  }

  if constexpr (std::is_same_v<T, double>) {
    return "double";
  }

  if constexpr (std::is_same_v<T, std::string>) {
    return "string";
  }

  if constexpr (std::is_same_v<T, bool>) {
    return "bool";
  }

  if constexpr (std::is_same_v<T, toml::date>) {
    return "date";
  }

  if constexpr (std::is_same_v<T, toml::time>) {
    return "time";
  }

  if constexpr (std::is_same_v<T, toml::date_time>) {
    return "date_time";
  }

  return "unknown";
}

/// Utility to print the type name of a toml::node/toml::node_view.
template <typename NodeView>
inline const char *TomlTypeName(const NodeView &node, std::string_view key) {
  switch (node.type()) {
    case toml::node_type::array:
      return TomlTypeName<toml::array>();

    case toml::node_type::table:
      return TomlTypeName<toml::table>();

    case toml::node_type::string:
      return TomlTypeName<std::string>();

    case toml::node_type::integer:
      return TomlTypeName<int64_t>();

    case toml::node_type::floating_point:
      return TomlTypeName<double>();

    case toml::node_type::boolean:
      return TomlTypeName<bool>();

    case toml::node_type::date:
      return TomlTypeName<toml::date>();

    case toml::node_type::time:
      return TomlTypeName<toml::time>();

    case toml::node_type::date_time:
      return TomlTypeName<toml::date_time>();

    // LCOV_EXCL_START
    case toml::node_type::none: {
      std::string msg{"Internal node type for parameter `"};
      msg += key;
      msg +=
          "` is `none` (not-a-node)! Please report at "
          "https://github.com/snototter/werkzeugkiste/issues";
      throw std::logic_error{msg};
      // or return "none"?
    }
      // LCOV_EXCL_STOP
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
inline bool ContainsKey(const toml::table &tbl, std::string_view key) {
  // Needed, because `tbl.contains()` only checks the direct children.
  const auto node = tbl.at_path(key);
  return node.is_value() || node.is_table() || node.is_array();
}

bool IsScalar(toml::node_type tp) {
  switch (tp) {
    case toml::node_type::none:
    case toml::node_type::table:
    case toml::node_type::array:
      return false;

    case toml::node_type::string:
    case toml::node_type::integer:
    case toml::node_type::floating_point:
    case toml::node_type::boolean:
    case toml::node_type::date:
    case toml::node_type::time:
    case toml::node_type::date_time:
      return true;
  }

  // LCOV_EXCL_START
  std::string msg{"Unhandled TOML node type `"};
  msg += std::to_string(static_cast<int32_t>(tp));
  msg += "` is not yet handled in `IsScalar`!";
  throw std::logic_error(msg);
  // LCOV_EXCL_STOP
}

/// Extracts the value from the toml::node or throws an error if the type
/// is not correct.
/// Tries converting numeric types if a lossless cast is feasible.
template <typename Tcfg, typename NodeView>
Tcfg ConvertTomlToConfigType(const NodeView &node, std::string_view key) {
  if constexpr (std::is_same_v<Tcfg, bool>) {
    if (node.is_boolean()) {
      return static_cast<bool>(*node.as_boolean());
    }
  } else if constexpr (std::is_arithmetic_v<Tcfg>) {
    try {
      if (node.is_integer()) {
        return checked_numcast<Tcfg, int64_t>(
            static_cast<int64_t>(*node.as_integer()));
      }

      if (node.is_floating_point()) {
        return checked_numcast<Tcfg, double>(
            static_cast<double>(*node.as_floating_point()));
      }
    } catch (const std::domain_error &e) {
      // Re-throw with extended error message.
      std::string msg{"Cannot convert numeric parameter `"};
      msg += key;
      msg += "` to `";
      msg += TypeName<Tcfg>();
      msg += "`. ";
      msg += e.what();
      throw TypeError{msg};
    }
  } else if constexpr (std::is_same_v<Tcfg, std::string>) {
    if (node.is_string()) {
      return std::string(*node.as_string());
    }
  } else if constexpr (std::is_same_v<Tcfg, date>) {
    if (node.is_date()) {
      const toml::date &d = node.as_date()->get();
      return date{d.year, d.month, d.day};
    }
  } else if constexpr (std::is_same_v<Tcfg, time>) {
    if (node.is_time()) {
      const toml::time &t = node.as_time()->get();
      return time{t.hour, t.minute, t.second, t.nanosecond};
    }
  } else if constexpr (std::is_same_v<Tcfg, date_time>) {
    if (node.is_date_time()) {
      const toml::date_time &dt = node.as_date_time()->get();
      date_time tmp{};
      tmp.date = date{dt.date.year, dt.date.month, dt.date.day};
      tmp.time = time{
          dt.time.hour, dt.time.minute, dt.time.second, dt.time.nanosecond};
      if (dt.offset.has_value()) {
        tmp.offset = time_offset{dt.offset.value().minutes};
      }
      return tmp;
    }
  }

  std::string msg{"Cannot query `"};
  msg += TomlTypeName(node, key);
  msg += "` parameter `";
  msg += key;
  msg += "` as `";
  msg += TypeName<Tcfg>();
  msg += "`!";
  throw TypeError{msg};
}

/// @brief Converts an interface-exposed type to a TOML type if possible.
/// Throws a type error upon invalid numeric conversion (i.e. the number is not
/// exactly representable by the target type).
/// @tparam Ttoml The TOML type.
/// @tparam Tcfg The type exposed via the werkzeugkiste::config API.
/// @param value The value to be converted.
/// @param key The fully-qualified parameter name.
template <typename Ttoml, typename Tcfg>
Ttoml ConvertConfigTypeToToml(const Tcfg &value, std::string_view key) {
  if constexpr (std::is_same_v<Tcfg, Ttoml>) {
    return value;
  }

  if constexpr (std::is_arithmetic_v<Tcfg> && !std::is_same_v<Tcfg, bool> &&
                (std::is_same_v<Ttoml, int64_t> ||
                    std::is_same_v<Ttoml, double>)) {
    return checked_numcast<Ttoml, Tcfg, TypeError>(value);
  }

  if constexpr (std::is_same_v<Tcfg, std::string_view> &&
                std::is_same_v<Ttoml, std::string>) {
    return std::string(value);
  }

  if constexpr (std::is_same_v<Tcfg, date> &&
                std::is_same_v<Ttoml, toml::date>) {
    return toml::date{value.year, value.month, value.day};
  }

  if constexpr (std::is_same_v<Tcfg, time> &&
                std::is_same_v<Ttoml, toml::time>) {
    return toml::time{value.hour, value.minute, value.second, value.nanosecond};
  }

  if constexpr (std::is_same_v<Tcfg, date_time> &&
                std::is_same_v<Ttoml, toml::date_time>) {
    if (value.IsLocal()) {
      return toml::date_time{
          toml::date{value.date.year, value.date.month, value.date.day},
          toml::time{value.time.hour,
              value.time.minute,
              value.time.second,
              value.time.nanosecond}};
    }
    toml::time_offset offset{};
    offset.minutes = static_cast<int16_t>(value.offset.value().minutes);
    return toml::date_time{
        toml::date{value.date.year, value.date.month, value.date.day},
        toml::time{value.time.hour,
            value.time.minute,
            value.time.second,
            value.time.nanosecond},
        offset};
  }

  std::string msg{"Cannot convert input of type `"};
  msg += TypeName<Tcfg>();
  msg += "` to internal type `";
  msg += TomlTypeName<Ttoml>();
  msg += "` as needed for parameter `";
  msg += key;
  msg += "`!";
  throw TypeError{msg};
}

/// @brief Looks up the value at the given key (fully-qualified TOML path).
///
/// If the key does not exist, a KeyError will be raised unless
/// `allow_default` is true (in which case the `default_val` will be
/// returned instead).
template <typename T, typename DefaultType = T>
T LookupScalar(const toml::table &tbl,
    std::string_view key,
    bool allow_default = false,
    DefaultType default_val = DefaultType{}) {
  if (!ContainsKey(tbl, key)) {
    if (allow_default) {
      return T{default_val};
    }

    throw KeyErrorWithSimilarKeys(tbl, key);
  }

  const auto node = tbl.at_path(key);
  return ConvertTomlToConfigType<T>(node, key);
}

/// @brief Looks up the value at the given key (fully-qualified TOML path).
///
/// If the key does not exist, a nullopt will be returned.
template <typename T>
std::optional<T> LookupOptionalScalar(const toml::table &tbl,
    std::string_view key) {
  if (!ContainsKey(tbl, key)) {
    return std::nullopt;
  }

  const auto node = tbl.at_path(key);
  return ConvertTomlToConfigType<T>(node, key);
}

/// Splits a fully-qualified TOML path into <anchestor, child>.
/// This does *not* handle arrays!
inline std::pair<std::string_view, std::string_view> SplitTomlPath(
    std::string_view path) {
  // Path splitting for nested arrays is not supported. Currently, this is
  // not an issue: We don't allow creating array elements. For example,
  // think of the complexity of handling "fancy" paths such as
  // "arr[3][0][1].table.array[0]".
  // Instead, users can create a list (`CreateList`) and then
  // append scalars, lists and groups/tables (`Append`).
  const std::size_t pos = path.find_last_of('.');
  if (pos != std::string_view::npos) {
    return std::make_pair(path.substr(0, pos), path.substr(pos + 1));
  }

  return std::make_pair(std::string_view{}, path);
}

/// @brief Throws a KeyError if the given key cannot be created, e.g. if it
///   refers to an array element or contains unsupported characters (a quoted
///   key or whitespace, etc.)
/// @param key The bare key or the fully-qualified parameter name (dotted key).
void EnsureDottedOrBareKey(std::string_view key) {
  if (!IsValidKey(key, /*allow_dots=*/true)) {
    std::string msg{
        "Expected a bare key/path (alphanumeric, '-', '_', '.'), but got `"};
    msg += key;
    msg += "`!";
    throw KeyError{msg};
  }
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
    std::string msg{"The path anchestors must consist of tables/arrays, but `"};
    msg += key;
    msg += "` is of type `";
    msg += TomlTypeName(node, key);
    msg += "`!";
    throw KeyError{msg};
  }

  // Parent does not exist. We now have to recursively create the
  // parent path, then create a table here.
  // But first, ensure that we are not asked to create a list:
  const auto path = SplitTomlPath(key);
  EnsureDottedOrBareKey(path.second);

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
      std::string msg{
          "Creating a table as a child of a list is not supported! Check "
          "configuration parameter `"};
      msg += key;
      msg += "`!";
      throw KeyError{msg};
    }
  }
}

template <typename NodeView, typename Tcfg>
void ReplaceScalar(NodeView &node, Tcfg value, std::string_view key) {
  if (node.is_boolean()) {
    *node.as_boolean() = ConvertConfigTypeToToml<bool>(value, key);
  } else if (node.is_integer()) {
    *node.as_integer() = ConvertConfigTypeToToml<int64_t>(value, key);
  } else if (node.is_floating_point()) {
    *node.as_floating_point() = ConvertConfigTypeToToml<double>(value, key);
  } else if (node.is_string()) {
    *node.as_string() = ConvertConfigTypeToToml<std::string>(value, key);
  } else if (node.is_date()) {
    *node.as_date() = ConvertConfigTypeToToml<toml::date>(value, key);
  } else if (node.is_time()) {
    *node.as_time() = ConvertConfigTypeToToml<toml::time>(value, key);
  } else if (node.is_date_time()) {
    *node.as_date_time() = ConvertConfigTypeToToml<toml::date_time>(value, key);
  } else if (node.is_array() || node.is_table()) {
    std::string msg{"Changing the type is not allowed: Cannot replace the "};
    msg += TomlTypeName(node, key);
    msg += " `";
    msg += key;
    msg += "` by a `";
    msg += TypeName<Tcfg>();
    msg += "` scalar!";
    throw TypeError{msg};
  } else {
    // LCOV_EXCL_START
    std::string msg{"`ReplaceScalar` invoked with unhandled TOML type `"};
    msg += TomlTypeName(node, key);
    msg += "`. Check parameter `";
    msg += key;
    msg += "`!";
    throw std::logic_error{msg};
    // LCOV_EXCL_STOP
  }
}

/// @brief Allows setting a scalar TOML parameter.
/// @tparam Ttoml Scalar TOML type to be set.
/// @tparam Tmessage Type needed for error message to avoid separate int32_t
/// specialization. Internally, all integers are stored as 64-bit: If there
/// would be an error while setting a 32-bit value, we don't want to show a
/// confusing "user provided 64-bit" error message.
/// @param tbl The TOML root node.
/// @param key The fully-qualified parameter name.
/// @param value The value to be set.
template <typename Ttoml, typename Tvalue>
void SetScalar(toml::table &tbl, std::string_view key, Tvalue value) {
  const auto path = SplitTomlPath(key);
  if (ContainsKey(tbl, key)) {
    auto node = tbl.at_path(key);
    ReplaceScalar(node, value, key);
  } else {
    EnsureContainerPathExists(tbl, path.first);
    EnsureDottedOrBareKey(path.second);

    toml::table *parent =
        path.first.empty() ? &tbl : tbl.at_path(path.first).as_table();
    if (parent == nullptr) {
      // LCOV_EXCL_START
      WZK_CONFIG_LOOKUP_RAISE_PATH_CREATION_ERROR(key, path.first);
      // LCOV_EXCL_STOP
    }

    auto result = parent->insert_or_assign(
        path.second, ConvertConfigTypeToToml<Ttoml>(value, key));
    if (!ContainsKey(tbl, key)) {
      // LCOV_EXCL_START
      WZK_CONFIG_LOOKUP_RAISE_ASSIGNMENT_ERROR(
          key, result.second, path.first, path.second);
      // LCOV_EXCL_STOP
    }
  }
}

/// @brief Internal helper (no sanity check) to create an array (list of
///   homogeneous elements).
template <typename Ttoml, typename Tcfg>
void CreateList(toml::table &tbl,
    std::string_view key,
    const std::vector<Tcfg> &vec) {
  const auto path = SplitTomlPath(key);
  EnsureContainerPathExists(tbl, path.first);
  EnsureDottedOrBareKey(path.second);

  toml::array arr{};
  for (const auto &value : vec) {
    arr.push_back(ConvertConfigTypeToToml<Ttoml>(value, key));
  }

  toml::table *parent =
      path.first.empty() ? &tbl : tbl.at_path(path.first).as_table();
  if (parent == nullptr) {
    // LCOV_EXCL_START
    WZK_CONFIG_LOOKUP_RAISE_PATH_CREATION_ERROR(key, path.first);
    // LCOV_EXCL_STOP
  }

  auto result = parent->insert_or_assign(path.second, arr);
  if (!ContainsKey(tbl, key)) {
    // LCOV_EXCL_START
    WZK_CONFIG_LOOKUP_RAISE_ASSIGNMENT_ERROR(
        key, result.second, path.first, path.second);
    // LCOV_EXCL_STOP
  }
}

/// @brief Internal helper for `ReplaceList`. Sanity checks are omitted on
///   purpose (they're part of `SetList > ReplaceList`).
/// @tparam Ttoml Element type of existing TOML array.
/// @tparam Tcfg Element type of replacement vector.
/// @param arr List to be replaced.
/// @param key Fully-qualified parameter name.
/// @param vec List of replacement values.
template <typename Ttoml, typename Tcfg>
void ReplaceHomogeneousList(toml::array &arr,
    std::string_view key,
    const std::vector<Tcfg> &vec) {
  toml::array toml_arr{};
  for (const auto &value : vec) {
    toml_arr.push_back(ConvertConfigTypeToToml<Ttoml>(value, key));
  }
  arr = toml_arr;
}

/// @brief Returns true if all elements of this TOML array are either integer
///   or floating point.
bool IsNumericArray(const toml::array &arr) {
  for (auto &&value : arr) {
    if (!value.is_number()) {
      return false;
    }
  }
  return true;
}

template <typename Ttoml, typename Tcfg>
void ReplaceList(toml::table &tbl,
    std::string_view key,
    const std::vector<Tcfg> &vec) {
  auto node = tbl.at_path(key);
  if (!node.is_array()) {
    std::string msg{"Cannot replace `"};
    msg += key;
    msg += "` by a list, because it is of incompatible type `";
    msg += TomlTypeName(node, key);
    msg += "`!";
    throw TypeError{msg};
  }

  toml::array &arr = *node.as_array();

  // * A list can be replaced by an empty list of any type (after saving, we
  //   can't dinstinguish an empty list of strings from an empty list of
  //   floats).
  // * If the existing array is empty, we can simply replace it by inserting
  //   the given values as is.
  // * If the existing array is inhomogeneous, we don't allow replacing it -
  //   unless it holds only numeric values (double <-> ints are interchangable
  //   if they can be exactly represented...)
  // * Otherwise, we need to check for compatible/convertible types.
  if (vec.empty()) {
    arr = toml::array{};
  } else if (arr.empty()) {
    CreateList<Ttoml, Tcfg>(tbl, key, vec);
  } else if (!arr.is_homogeneous()) {
    if (IsNumericArray(arr)) {
      // If all elements are numbers, we have only int64 and double elements.
      // Thus, we can try to convert all inputs to double precision:
      ReplaceHomogeneousList<double>(arr, key, vec);
    } else {
      std::string msg{"Changing the type is not allowed. Parameter `"};
      msg += key;
      msg +=
          "` is an inhomogeneous list/array, but replacement list is of type `";
      msg += TypeName<Tcfg>();
      msg += "`!";
      throw TypeError{msg};
    }
  } else if (arr[0].is_boolean()) {
    ReplaceHomogeneousList<bool>(arr, key, vec);
  } else if (arr[0].is_integer()) {
    ReplaceHomogeneousList<int64_t>(arr, key, vec);
  } else if (arr[0].is_floating_point()) {
    ReplaceHomogeneousList<double>(arr, key, vec);
  } else if (arr[0].is_string()) {
    ReplaceHomogeneousList<std::string>(arr, key, vec);
  } else if (arr[0].is_date()) {
    ReplaceHomogeneousList<toml::date>(arr, key, vec);
  } else if (arr[0].is_time()) {
    ReplaceHomogeneousList<toml::time>(arr, key, vec);
  } else if (arr[0].is_date_time()) {
    ReplaceHomogeneousList<toml::date_time>(arr, key, vec);
  } else {
    // LCOV_EXCL_START
    std::string msg{"Configuration parameter `"};
    msg += key;
    msg += "` is a list of `";
    msg += TomlTypeName(arr[0], key);
    msg +=
        "`, but this type is not yet supported! Please report at "
        "https://github.com/snototter/werkzeugkiste/issues";
    throw std::logic_error{msg};
    // LCOV_EXCL_STOP
  }
}

template <typename Ttoml, typename Tcfg>
void SetList(toml::table &tbl,
    std::string_view key,
    const std::vector<Tcfg> &vec) {
  if (ContainsKey(tbl, key)) {
    ReplaceList<Ttoml>(tbl, key, vec);
  } else {
    CreateList<Ttoml>(tbl, key, vec);
  }
}

toml::array *GetExistingList(toml::table &tbl, std::string_view key) {
  if (!ContainsKey(tbl, key)) {
    throw KeyErrorWithSimilarKeys(tbl, key);
  }

  auto node = tbl.at_path(key);
  if (!node.is_array()) {
    std::string msg{"Cannot look up element `"};
    msg += key;
    msg += "` as a list, because it is of type `";
    msg += TomlTypeName(node, key);
    msg += "`!";
    throw TypeError{msg};
  }

  return node.as_array();
}

template <typename Ttoml, typename Tcfg>
void AppendScalarListElement(toml::table &tbl,
    std::string_view key,
    Tcfg value) {
  toml::array *arr = GetExistingList(tbl, key);
  arr->push_back(ConvertConfigTypeToToml<Ttoml>(value, key));
}

/// @brief Extracts a single pointXd from the given toml::table.
template <typename Pt>
inline Pt ConvertTableToPoint(const toml::table &tbl, std::string_view key) {
  using namespace std::string_view_literals;
  constexpr std::array<std::string_view, 3> point_keys{"x"sv, "y"sv, "z"sv};
  static_assert(Pt::ndim <= point_keys.size(),
      "Table keys for higher-dimensional points have not yet been defined!");

  using CoordType = typename Pt::value_type;

  std::array<CoordType, Pt::ndim> values{};
  for (std::size_t idx = 0; idx < Pt::ndim; ++idx) {
    if (!tbl.contains(point_keys[idx])) {
      std::ostringstream msg;
      msg << "Invalid parameter `" << key
          << "`. Table entry does not specify the `" << point_keys[idx]
          << "` coordinate!";
      throw TypeError{msg.str()};
    }

    values[idx] = ConvertTomlToConfigType<CoordType>(tbl[point_keys[idx]], key);
  }

  Pt point{};
  point.x = values[0];
  point.y = values[1];
  if constexpr (Pt::ndim > 2) {
    point.z = values[2];
  }
  return point;
}

/// @brief Extracts a single pointXd from the given toml::table.
template <typename Pt>
inline Pt ConvertArrayToPoint(const toml::array &arr, std::string_view key) {
  using CoordType = typename Pt::value_type;

  // The array must have at least Dim entries - more are also allowed, as
  // they will just be ignored.
  if (arr.size() < Pt::ndim) {
    std::ostringstream msg;
    msg << "Invalid parameter `" << key << "`. Cannot extract a " << Pt::ndim
        << "D point from a " << arr.size() << "-element array!";
    throw TypeError{msg.str()};
  }

  std::array<CoordType, Pt::ndim> values{};
  for (std::size_t idx = 0; idx < Pt::ndim; ++idx) {
    const auto fqn = FullyQualifiedArrayElementPath(idx, key);
    values[idx] = ConvertTomlToConfigType<CoordType>(arr[idx], fqn);
  }

  Pt point{};
  point.x = values[0];
  point.y = values[1];
  if constexpr (Pt::ndim > 2) {
    point.z = values[2];
  }
  return point;
}

template <typename Pt>
Pt GetPoint(const toml::table &tbl, std::string_view key) {
  static_assert(std::is_arithmetic_v<typename Pt::value_type>);
  static_assert(Pt::ndim == 2 || Pt::ndim == 3);
  if (!ContainsKey(tbl, key)) {
    throw KeyErrorWithSimilarKeys(tbl, key);
  }

  const auto node = tbl.at_path(key);
  if (node.is_array()) {
    const auto &pt = *node.as_array();
    return ConvertArrayToPoint<Pt>(pt, key);
  }
  if (node.is_table()) {
    const auto &pt = *node.as_table();
    return ConvertTableToPoint<Pt>(pt, key);
  }

  std::string msg{"Cannot convert `"};
  msg += key;
  msg += "` to a ";
  msg += std::to_string(Pt::ndim);
  msg += "D point. Expected a group or list, but got `";
  msg += TomlTypeName(node, key);
  msg += "`!";
  throw TypeError{msg};
}

template <typename Pt>
std::vector<Pt> GetPoints(const toml::table &tbl, std::string_view key) {
  static_assert(std::is_arithmetic_v<typename Pt::value_type>);
  static_assert(Pt::ndim == 2 || Pt::ndim == 3);
  if (!ContainsKey(tbl, key)) {
    throw KeyErrorWithSimilarKeys(tbl, key);
  }

  const auto node = tbl.at_path(key);
  if (!node.is_array()) {
    std::string msg{"Invalid point list configuration: `"};
    msg += key;
    msg += "` must be a list, but is of type `";
    msg += TomlTypeName(node, key);
    msg += "`!";
    throw TypeError{msg};
  }

  const toml::array &arr = *node.as_array();
  std::size_t arr_index = 0;
  std::vector<Pt> points;
  for (auto &&value : arr) {
    const auto fqn = FullyQualifiedArrayElementPath(arr_index, key);
    if (value.is_array()) {
      const auto &pt = *value.as_array();
      points.push_back(ConvertArrayToPoint<Pt>(pt, fqn));
    } else if (value.is_table()) {
      const auto &pt = *value.as_table();
      points.push_back(ConvertTableToPoint<Pt>(pt, fqn));
    } else {
      std::string msg{
          "Invalid point list. All parameter entries must be either arrays or "
          "tables, but `"};
      msg += fqn;
      msg += "` is not!";
      throw TypeError{msg};
    }
    ++arr_index;
  }
  return points;
}

template <typename Tcfg>
std::vector<Tcfg> GetList(const toml::array &arr, std::string_view key) {
  std::size_t arr_index = 0;
  std::vector<Tcfg> scalars{};
  for (auto &&value : arr) {
    const auto fqn = FullyQualifiedArrayElementPath(arr_index, key);
    if (value.is_value()) {
      scalars.push_back(ConvertTomlToConfigType<Tcfg>(value, fqn));
    } else {
      std::string msg{"Invalid list configuration `"};
      msg += key;
      msg += "`: All entries must be of scalar type `";
      msg += TypeName<Tcfg>();
      msg += "`, but `";
      msg += fqn;
      msg += "` is `";
      msg += TomlTypeName(value, fqn);
      msg += "`!";
      throw TypeError{msg};
    }
    ++arr_index;
  }
  return scalars;
}
}  // namespace detail

// Abusing the PImpl idiom to hide the internally used TOML table.
struct Configuration::Impl {
  toml::table config_root{};

  const toml::table &Table(std::string_view key) const {
    if (key.empty()) {
      return config_root;
    }

    if (!detail::ContainsKey(config_root, key)) {
      throw detail::KeyErrorWithSimilarKeys(config_root, key);
    }

    const auto &node = config_root.at_path(key);
    if (!node.is_table()) {
      std::string msg{"Cannot lookup parameter `"};
      msg += key;
      msg += "` as group, because it is a `";
      msg += detail::TomlTypeName(node, key);
      msg += "`!";
      throw TypeError{msg};
    }
    return *node.as_table();
  }

  toml::table &Table(std::string_view key) {
    if (key.empty()) {
      return config_root;
    }

    if (!detail::ContainsKey(config_root, key)) {
      throw detail::KeyErrorWithSimilarKeys(config_root, key);
    }

    auto node = config_root.at_path(key);
    if (!node.is_table()) {
      std::string msg{"Cannot lookup parameter `"};
      msg += key;
      msg += "` as group, because it is a `";
      msg += detail::TomlTypeName(node, key);
      msg += "`!";
      throw TypeError{msg};
    }
    return *node.as_table();
  }

  const toml::array &List(std::string_view key) const {
    if (!detail::ContainsKey(config_root, key)) {
      throw detail::KeyErrorWithSimilarKeys(config_root, key);
    }

    const auto &node = config_root.at_path(key);
    if (!node.is_array()) {
      std::string msg{"Cannot lookup parameter `"};
      msg += key;
      msg += "` as list, because it is a `";
      msg += detail::TomlTypeName(node, key);
      msg += "`!";
      throw TypeError{msg};
    }
    return *node.as_array();
  }
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
  const auto keys_this = detail::ListTableKeys(pimpl_->config_root,
      ""sv,
      /*include_array_entries=*/true,
      /*recursive=*/true);
  const auto keys_other = detail::ListTableKeys(other.pimpl_->config_root,
      ""sv,
      /*include_array_entries=*/true,
      /*recursive=*/true);

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

bool Configuration::operator==(const Configuration &other) const {
  return Equals(other);
}

bool Configuration::operator!=(const Configuration &other) const {
  return !(*this == other);
}

bool Configuration::Contains(std::string_view key) const {
  return detail::ContainsKey(pimpl_->config_root, key);
}

std::size_t Configuration::Size(std::string_view key) const {
  if (key.empty()) {
    return pimpl_->config_root.size();
  }

  const auto nv = pimpl_->config_root.at_path(key);

  if (nv.type() == toml::node_type::none) {
    throw detail::KeyErrorWithSimilarKeys(pimpl_->config_root, key);
  }

  if (nv.type() == toml::node_type::array) {
    return nv.as_array()->size();
  }

  if (nv.type() == toml::node_type::table) {
    return nv.as_table()->size();
  }

  std::string msg{"To query its size, the parameter `"};
  msg += key;
  msg += "` must be a list or a group, but it is of type `";
  msg += detail::TomlTypeName(nv, key);
  msg += "`!";
  throw TypeError{msg};
}

ConfigType Configuration::Type(std::string_view key) const {
  if (key.empty()) {
    return ConfigType::Group;
  }

  const auto nv = pimpl_->config_root.at_path(key);
  switch (nv.type()) {
    case toml::node_type::none:
      throw detail::KeyErrorWithSimilarKeys(pimpl_->config_root, key);

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

    case toml::node_type::date:
      return ConfigType::Date;

    case toml::node_type::time:
      return ConfigType::Time;

    case toml::node_type::date_time:
      return ConfigType::DateTime;
  }

  // LCOV_EXCL_START
  std::string msg{"TOML node type `"};
  msg += detail::TomlTypeName(nv, key);
  msg += "` is not yet handled in `Configuration::Type`!";
  throw std::logic_error(msg);
  // LCOV_EXCL_STOP
}

void Configuration::Delete(std::string_view key) {
  if (!detail::ContainsKey(pimpl_->config_root, key)) {
    throw detail::KeyErrorWithSimilarKeys(pimpl_->config_root, key);
  }

  detail::EnsureDottedOrBareKey(key);

  const auto path = detail::SplitTomlPath(key);
  toml::table *parent = path.first.empty()
                          ? &pimpl_->config_root
                          : pimpl_->config_root.at_path(path.first).as_table();
  if (parent == nullptr) {
    // LCOV_EXCL_START
    // Should be unreachable due to the previous dotted/bare key check.
    std::string msg{"Cannot delete parameter `"};
    msg += key;
    msg += "`! Parent must be either a subgroup or the root node.";
    throw KeyError{msg};
    // LCOV_EXCL_STOP
  }

  const std::size_t erased = parent->erase(path.second);
  if (erased == 0) {
    // LCOV_EXCL_START
    // Should be unreachable.
    std::string msg{"Unknown error while deleting parameter `"};
    msg += key;
    msg +=
        "`! Please report at"
        "https://github.com/snototter/werkzeugkiste/issues";
    throw std::runtime_error{msg};
    // LCOV_EXCL_STOP
  }
}

bool Configuration::IsHomogeneousScalarList(std::string_view key) const {
  if (!detail::ContainsKey(pimpl_->config_root, key)) {
    throw detail::KeyErrorWithSimilarKeys(pimpl_->config_root, key);
  }

  auto node = pimpl_->config_root.at_path(key);
  if (!node.is_array()) {
    std::string msg{"Cannot check if `"};
    msg += key;
    msg += "` is homogeneous, because it is of type `";
    msg += detail::TomlTypeName(node, key);
    msg += "` instead of a list!";
    throw TypeError{msg};
  }

  const toml::array &arr = *node.as_array();
  for (const auto &value : arr) {
    if (!detail::IsScalar(value.type()) || (value.type() != arr[0].type())) {
      return false;
    }
  }
  return true;
}

std::vector<std::string> Configuration::ListParameterNames(std::string_view key,
    bool include_array_entries,
    bool recursive) const {
  using namespace std::string_view_literals;
  return detail::ListTableKeys(
      pimpl_->Table(key), ""sv, include_array_entries, recursive);
}

//---------------------------------------------------------------------------
// Boolean

bool Configuration::GetBoolean(std::string_view key) const {
  return detail::LookupScalar<bool>(pimpl_->config_root,
      key,
      /*allow_default=*/false);
}

bool Configuration::GetBooleanOr(std::string_view key, bool default_val) const {
  return detail::LookupScalar<bool>(pimpl_->config_root,
      key,
      /*allow_default=*/true,
      default_val);
}

std::optional<bool> Configuration::GetOptionalBoolean(
    std::string_view key) const {
  return detail::LookupOptionalScalar<bool>(pimpl_->config_root, key);
}

void Configuration::SetBoolean(std::string_view key, bool value) {
  detail::SetScalar<bool>(pimpl_->config_root, key, value);
}

std::vector<bool> Configuration::GetBooleanList(std::string_view key) const {
  return detail::GetList<bool>(pimpl_->List(key), key);
}

void Configuration::SetBooleanList(std::string_view key,
    const std::vector<bool> &values) {
  detail::SetList<bool>(pimpl_->config_root, key, values);
}

//---------------------------------------------------------------------------
// Integer (32-bit)

int32_t Configuration::GetInteger32(std::string_view key) const {
  return detail::LookupScalar<int32_t>(pimpl_->config_root,
      key,
      /*allow_default=*/false);
}

int32_t Configuration::GetInteger32Or(std::string_view key,
    int32_t default_val) const {
  return detail::LookupScalar<int32_t>(pimpl_->config_root,
      key,
      /*allow_default=*/true,
      default_val);
}

std::optional<int32_t> Configuration::GetOptionalInteger32(
    std::string_view key) const {
  return detail::LookupOptionalScalar<int32_t>(pimpl_->config_root, key);
}

void Configuration::SetInteger32(std::string_view key, int32_t value) {
  detail::SetScalar<int64_t>(
      pimpl_->config_root, key, static_cast<int64_t>(value));
}

std::vector<int32_t> Configuration::GetInteger32List(
    std::string_view key) const {
  return detail::GetList<int32_t>(pimpl_->List(key), key);
}

void Configuration::SetInteger32List(std::string_view key,
    const std::vector<int32_t> &values) {
  detail::SetList<int64_t>(pimpl_->config_root, key, values);
}

//---------------------------------------------------------------------------
// Integer (64-bit)

int64_t Configuration::GetInteger64(std::string_view key) const {
  return detail::LookupScalar<int64_t>(pimpl_->config_root,
      key,
      /*allow_default=*/false);
}

int64_t Configuration::GetInteger64Or(std::string_view key,
    int64_t default_val) const {
  return detail::LookupScalar<int64_t>(pimpl_->config_root,
      key,
      /*allow_default=*/true,
      default_val);
}

std::optional<int64_t> Configuration::GetOptionalInteger64(
    std::string_view key) const {
  return detail::LookupOptionalScalar<int64_t>(pimpl_->config_root, key);
}

void Configuration::SetInteger64(std::string_view key, int64_t value) {
  detail::SetScalar<int64_t>(pimpl_->config_root, key, value);
}

std::vector<int64_t> Configuration::GetInteger64List(
    std::string_view key) const {
  return detail::GetList<int64_t>(pimpl_->List(key), key);
}

void Configuration::SetInteger64List(std::string_view key,
    const std::vector<int64_t> &values) {
  detail::SetList<int64_t>(pimpl_->config_root, key, values);
}

point2d<int64_t> Configuration::GetInteger64Point2D(
    std::string_view key) const {
  return detail::GetPoint<point2d<int64_t>>(pimpl_->config_root, key);
}

point3d<int64_t> Configuration::GetInteger64Point3D(
    std::string_view key) const {
  return detail::GetPoint<point3d<int64_t>>(pimpl_->config_root, key);
}

std::vector<point2d<int64_t>> Configuration::GetInteger64Points2D(
    std::string_view key) const {
  return detail::GetPoints<point2d<int64_t>>(pimpl_->config_root, key);
}

std::vector<point3d<int64_t>> Configuration::GetInteger64Points3D(
    std::string_view key) const {
  return detail::GetPoints<point3d<int64_t>>(pimpl_->config_root, key);
}

//---------------------------------------------------------------------------
// Floating Point

double Configuration::GetDouble(std::string_view key) const {
  return detail::LookupScalar<double>(pimpl_->config_root,
      key,
      /*allow_default=*/false);
}

double Configuration::GetDoubleOr(std::string_view key,
    double default_val) const {
  return detail::LookupScalar<double>(pimpl_->config_root,
      key,
      /*allow_default=*/true,
      default_val);
}

std::optional<double> Configuration::GetOptionalDouble(
    std::string_view key) const {
  return detail::LookupOptionalScalar<double>(pimpl_->config_root, key);
}

void Configuration::SetDouble(std::string_view key, double value) {
  detail::SetScalar<double>(pimpl_->config_root, key, value);
}

std::vector<double> Configuration::GetDoubleList(std::string_view key) const {
  return detail::GetList<double>(pimpl_->List(key), key);
}

void Configuration::SetDoubleList(std::string_view key,
    const std::vector<double> &values) {
  detail::SetList<double>(pimpl_->config_root, key, values);
}

point2d<double> Configuration::GetDoublePoint2D(std::string_view key) const {
  return detail::GetPoint<point2d<double>>(pimpl_->config_root, key);
}

point3d<double> Configuration::GetDoublePoint3D(std::string_view key) const {
  return detail::GetPoint<point3d<double>>(pimpl_->config_root, key);
}

std::vector<point2d<double>> Configuration::GetDoublePoints2D(
    std::string_view key) const {
  return detail::GetPoints<point2d<double>>(pimpl_->config_root, key);
}

std::vector<point3d<double>> Configuration::GetDoublePoints3D(
    std::string_view key) const {
  return detail::GetPoints<point3d<double>>(pimpl_->config_root, key);
}

//---------------------------------------------------------------------------
// Strings

std::string Configuration::GetString(std::string_view key) const {
  using namespace std::string_view_literals;
  return detail::LookupScalar<std::string, std::string_view>(
      pimpl_->config_root, key, /*allow_default=*/false, ""sv);
}

std::string Configuration::GetStringOr(std::string_view key,
    std::string_view default_val) const {
  return detail::LookupScalar<std::string, std::string_view>(
      pimpl_->config_root, key, /*allow_default=*/true, default_val);
}

std::optional<std::string> Configuration::GetOptionalString(
    std::string_view key) const {
  return detail::LookupOptionalScalar<std::string>(pimpl_->config_root, key);
}

void Configuration::SetString(std::string_view key, std::string_view value) {
  detail::SetScalar<std::string>(pimpl_->config_root, key, value);
}

std::vector<std::string> Configuration::GetStringList(
    std::string_view key) const {
  return detail::GetList<std::string>(pimpl_->List(key), key);
}

void Configuration::SetStringList(std::string_view key,
    const std::vector<std::string_view> &values) {
  detail::SetList<std::string>(pimpl_->config_root, key, values);
}

//---------------------------------------------------------------------------
// Date

date Configuration::GetDate(std::string_view key) const {
  return detail::LookupScalar<date>(pimpl_->config_root,
      key,
      /*allow_default=*/false);
}

date Configuration::GetDateOr(std::string_view key,
    const date &default_val) const {
  return detail::LookupScalar<date>(pimpl_->config_root,
      key,
      /*allow_default=*/true,
      default_val);
}

std::optional<date> Configuration::GetOptionalDate(std::string_view key) const {
  return detail::LookupOptionalScalar<date>(pimpl_->config_root, key);
}

void Configuration::SetDate(std::string_view key, const date &value) {
  detail::SetScalar<toml::date>(pimpl_->config_root, key, value);
}

std::vector<date> Configuration::GetDateList(std::string_view key) const {
  return detail::GetList<date>(pimpl_->List(key), key);
}

void Configuration::SetDateList(std::string_view key,
    const std::vector<date> &values) {
  detail::SetList<toml::date>(pimpl_->config_root, key, values);
}

//---------------------------------------------------------------------------
// Time

time Configuration::GetTime(std::string_view key) const {
  return detail::LookupScalar<time>(pimpl_->config_root,
      key,
      /*allow_default=*/false);
}

time Configuration::GetTimeOr(std::string_view key,
    const time &default_val) const {
  return detail::LookupScalar<time>(pimpl_->config_root,
      key,
      /*allow_default=*/true,
      default_val);
}

std::optional<time> Configuration::GetOptionalTime(std::string_view key) const {
  return detail::LookupOptionalScalar<time>(pimpl_->config_root, key);
}

void Configuration::SetTime(std::string_view key, const time &value) {
  detail::SetScalar<toml::time>(pimpl_->config_root, key, value);
}

std::vector<time> Configuration::GetTimeList(std::string_view key) const {
  return detail::GetList<time>(pimpl_->List(key), key);
}

void Configuration::SetTimeList(std::string_view key,
    const std::vector<time> &values) {
  detail::SetList<toml::time>(pimpl_->config_root, key, values);
}

//---------------------------------------------------------------------------
// Date-time

date_time Configuration::GetDateTime(std::string_view key) const {
  return detail::LookupScalar<date_time>(pimpl_->config_root,
      key,
      /*allow_default=*/false);
}

date_time Configuration::GetDateTimeOr(std::string_view key,
    const date_time &default_val) const {
  return detail::LookupScalar<date_time>(pimpl_->config_root,
      key,
      /*allow_default=*/true,
      default_val);
}

std::optional<date_time> Configuration::GetOptionalDateTime(
    std::string_view key) const {
  return detail::LookupOptionalScalar<date_time>(pimpl_->config_root, key);
}

void Configuration::SetDateTime(std::string_view key, const date_time &value) {
  detail::SetScalar<toml::date_time>(pimpl_->config_root, key, value);
}

std::vector<date_time> Configuration::GetDateTimeList(
    std::string_view key) const {
  return detail::GetList<date_time>(pimpl_->List(key), key);
}

void Configuration::SetDateTimeList(std::string_view key,
    const std::vector<date_time> &values) {
  detail::SetList<toml::date_time>(pimpl_->config_root, key, values);
}

//---------------------------------------------------------------------------
void Configuration::CreateList(std::string_view key) {
  if (detail::ContainsKey(pimpl_->config_root, key)) {
    std::string msg{"Cannot create an empty list because parameter `"};
    msg += key;
    msg += "` already exists!";
    throw KeyError{msg};
  }

  detail::CreateList<bool, bool>(pimpl_->config_root, key, {});
}

void Configuration::ClearList(std::string_view key) {
  toml::array *arr = detail::GetExistingList(pimpl_->config_root, key);
  arr->clear();
}

void Configuration::AppendList(std::string_view key) {
  toml::array *arr = detail::GetExistingList(pimpl_->config_root, key);
  arr->push_back(toml::array{});
}

void Configuration::Append(std::string_view key, bool value) {
  detail::AppendScalarListElement<bool>(pimpl_->config_root, key, value);
}

void Configuration::Append(std::string_view key, int32_t value) {
  detail::AppendScalarListElement<int64_t>(pimpl_->config_root, key, value);
}

void Configuration::Append(std::string_view key, int64_t value) {
  detail::AppendScalarListElement<int64_t>(pimpl_->config_root, key, value);
}

void Configuration::Append(std::string_view key, double value) {
  detail::AppendScalarListElement<double>(pimpl_->config_root, key, value);
}

void Configuration::Append(std::string_view key, std::string_view value) {
  detail::AppendScalarListElement<std::string>(pimpl_->config_root, key, value);
}

void Configuration::Append(std::string_view key, const date &value) {
  detail::AppendScalarListElement<toml::date>(pimpl_->config_root, key, value);
}

void Configuration::Append(std::string_view key, const time &value) {
  detail::AppendScalarListElement<toml::time>(pimpl_->config_root, key, value);
}

void Configuration::Append(std::string_view key, const date_time &value) {
  detail::AppendScalarListElement<toml::date_time>(
      pimpl_->config_root, key, value);
}

void Configuration::Append(std::string_view key, const Configuration &group) {
  toml::array *arr = detail::GetExistingList(pimpl_->config_root, key);
  arr->push_back(group.pimpl_->config_root);
}

//---------------------------------------------------------------------------
// Group/"Sub-Configuration"

Configuration Configuration::GetGroup(std::string_view key) const {
  const toml::table &tbl = pimpl_->Table(key);
  Configuration cfg;
  cfg.pimpl_->config_root = tbl;
  return cfg;
}

void Configuration::SetGroup(std::string_view key, const Configuration &group) {
  if (key.empty()) {
    throw KeyError{
        "Cannot replace this configuration with a parameter group. Key cannot "
        "be empty in `SetGroup`!"};
  }

  const auto path = detail::SplitTomlPath(key);
  if (detail::ContainsKey(pimpl_->config_root, key)) {
    const auto node = pimpl_->config_root.at_path(key);
    if (!node.is_table()) {
      std::string msg{"Cannot insert parameter group at `"};
      msg += key;
      msg += "`. Existing parameter is of type `";
      msg += detail::TomlTypeName(node, key);
      msg += "`!";
      throw TypeError{msg};
    }

    auto &ref = *node.as_table();
    ref = group.pimpl_->config_root;
  } else {
    detail::EnsureContainerPathExists(pimpl_->config_root, path.first);
    toml::table *parent =
        path.first.empty() ? &pimpl_->config_root
                           : pimpl_->config_root.at_path(path.first).as_table();
    if (parent == nullptr) {
      // LCOV_EXCL_START
      WZK_CONFIG_LOOKUP_RAISE_PATH_CREATION_ERROR(key, path.first);
      // LCOV_EXCL_STOP
    }

    auto result =
        parent->insert_or_assign(path.second, group.pimpl_->config_root);
    if (!detail::ContainsKey(pimpl_->config_root, key)) {
      // LCOV_EXCL_START
      WZK_CONFIG_LOOKUP_RAISE_ASSIGNMENT_ERROR(
          key, result.second, path.first, path.second);
      // LCOV_EXCL_STOP
    }
  }
}

//---------------------------------------------------------------------------
// Convenience utilities

bool Configuration::AdjustRelativePaths(std::string_view key,
    std::string_view base_path,
    const std::vector<std::string_view> &parameters) {
  using namespace std::string_view_literals;
  const KeyMatcher matcher{parameters};
  auto to_replace = [matcher](std::string_view fqn) -> bool {
    return matcher.Match(fqn);
  };

  bool replaced{false};
  bool *rep_ptr = &replaced;
  auto func = [rep_ptr, to_replace, base_path](
                  toml::node &node, std::string_view fqn) -> void {
    if (node.is_string() && to_replace(fqn)) {
      // Check if the path is relative
      const std::string param_str =
          detail::ConvertTomlToConfigType<std::string>(node, fqn);
      const bool is_file_url = strings::StartsWith(param_str, "file://");
      // NOLINTNEXTLINE(*magic-numbers)
      const std::string path = is_file_url ? param_str.substr(7) : param_str;

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
  detail::Traverse(pimpl_->Table(key), ""sv, func);
  return replaced;
}

bool Configuration::ReplaceStringPlaceholders(std::string_view key,
    const std::vector<std::pair<std::string_view, std::string_view>>
        &replacements) {
  // Sanity check, search string can't be empty
  for (const auto &rep : replacements) {
    if (rep.first.empty()) {
      throw ValueError{
          "Search string within `ReplaceStrings()` must not be empty!"};
    }
  }

  using namespace std::string_view_literals;
  bool replaced{false};
  bool *rep_ptr = &replaced;
  auto func = [rep_ptr, replacements](
                  toml::node &node, std::string_view fqn) -> void {
    if (node.is_string()) {
      std::string param_str =
          detail::ConvertTomlToConfigType<std::string>(node, fqn);
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
  detail::Traverse(pimpl_->Table(key), ""sv, func);
  return replaced;
}

void Configuration::LoadNestedConfiguration(std::string_view key) {
  if (!detail::ContainsKey(pimpl_->config_root, key)) {
    throw detail::KeyErrorWithSimilarKeys(pimpl_->config_root, key);
  }

  const auto &node = pimpl_->config_root.at_path(key);
  if (!node.is_string()) {
    std::string msg{"Parameter `"};
    msg += key;
    msg += "` to load a nested configuration must be a string, but is `";
    msg += detail::TomlTypeName(node, key);
    msg += "`!";
    throw TypeError{msg};
  }

  // Overview:
  // * Load the configuration.
  // * Replace the filename parameter by the loaded configuration (for this,
  //   we first have to remove the parameter).

  const std::string fname = std::string(*node.as_string());
  Configuration loaded = LoadFile(fname);

  const auto path = detail::SplitTomlPath(key);

  toml::table *parent = path.first.empty()
                          ? &pimpl_->config_root
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

  const auto result = parent->insert(path.second, loaded.pimpl_->config_root);

  if (!result.second) {
    // LCOV_EXCL_START
    std::string msg{"Could not insert nested configuration at `"};
    msg += key;
    msg += "`!";
    throw std::runtime_error{msg};
    // LCOV_EXCL_STOP
  }
}

//---------------------------------------------------------------------------
// Serialization

std::string Configuration::ToTOML() const {
  std::ostringstream repr;
  repr << toml::toml_formatter{pimpl_->config_root};
  return repr.str();
}

std::string Configuration::ToJSON() const {
  std::ostringstream repr;
  repr << toml::json_formatter{
      pimpl_->config_root, toml::json_formatter::default_flags};
  return repr.str();
}

std::string Configuration::ToYAML() const {
  std::ostringstream repr;
  repr << toml::yaml_formatter{
      pimpl_->config_root, toml::yaml_formatter::default_flags};
  return repr.str();
}

std::string Configuration::ToLibconfig() const {
  return DumpLibconfigString(*this);
}

//---------------------------------------------------------------------------
// Generic loading from file
Configuration LoadFile(std::string_view filename) {
  const std::optional<std::string> ext = files::Extension(filename);
  const std::string lower =
      (ext.has_value()) ? strings::Lower(ext.value()) : "";
  if (lower == ".toml") {
    return LoadTOMLFile(filename);
  }
  if (lower == ".json") {
    return LoadJSONFile(filename);
  }
  if (lower == ".cfg") {
    return LoadLibconfigFile(filename);
  }
  std::string msg{"Cannot deduce configuration type from file name `"};
  msg += filename;
  msg += "`!";
  throw ParseError{msg};
}

#undef WZK_CONFIG_LOOKUP_RAISE_PATH_CREATION_ERROR
#undef WZK_CONFIG_LOOKUP_RAISE_ASSIGNMENT_ERROR
}  // namespace werkzeugkiste::config
