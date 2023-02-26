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
#define WZK_CONFIG_LOOKUP_RAISE_ASSIGNMENT_ERROR(KEY, INSERTED, PARENT, \
                                                 CURRENT)               \
  do {                                                                  \
    std::ostringstream msg;                                             \
    msg << "Assigning `" << (KEY)                                       \
        << "` completed without failure, but the key cannot "           \
           "be looked up. The value should have been "                  \
        << ((INSERTED) ? "inserted" : "assigned") << " at parent(`"     \
        << (PARENT) << "`), child(`" << (CURRENT)                       \
        << "`). This must be a bug. Please report at "                  \
           "https://github.com/snototter/werkzeugkiste/issues";         \
    throw std::logic_error{msg.str()};                                  \
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

/// @brief Prepares a KeyError with an alternative key suggestion.
/// @param tbl The configuration root node/table.
/// @param key The key which could not be found.
/// @return KeyError instance to be thrown.
KeyError KeyErrorWithSimilarKeys(const toml::table &tbl, std::string_view key) {
  using namespace std::string_view_literals;
  std::string msg{"Key `"};
  msg.append(key);
  msg.append("` does not exist!");

  const std::vector<std::string> keys =
      ListTableKeys(tbl, ""sv, /*include_array_entries=*/true);
  std::vector<std::pair<std::size_t, std::string_view>> candidates;
  for (const auto &cand : keys) {
    // The edit distance can't be less than the length difference between the
    // two strings. So we can reject unsuitable keys earlier.
    if (strings::LengthDifference(key, cand) < 5) {
      const std::size_t edit_dist = strings::LevenshteinDistance(key, cand);
      if (edit_dist < 3) {
        candidates.emplace_back(edit_dist, cand);
      }
    }
  }
  if (!candidates.empty()) {
    const std::vector<std::size_t> sorted_indices = container::GetSortedIndices(
        candidates,
        [](const std::pair<std::size_t, std::string_view> &a,
           const std::pair<std::size_t, std::string_view> &b) -> bool {
          return a.first < b.first;
        });
    msg.append(" Did you mean: `");
    const std::size_t num_to_include =
        std::min(sorted_indices.size(), static_cast<std::size_t>(3));
    for (std::size_t idx = 0; idx < num_to_include; ++idx) {
      const auto &cand = candidates[sorted_indices[idx]];
      msg.append(cand.second);
      if (idx < num_to_include - 1) {
        msg.append("`, `");
      }
    }
    msg.append("`?");
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
  if (node.is_table()) {
    return TomlTypeName<toml::table>();
  }

  if (node.is_array()) {
    return TomlTypeName<toml::array>();
  }

  if (node.is_integer()) {
    return TomlTypeName<int64_t>();
  }

  if (node.is_floating_point()) {
    return TomlTypeName<double>();
  }

  if (node.is_string()) {
    return TomlTypeName<std::string>();
  }

  if (node.is_boolean()) {
    return TomlTypeName<bool>();
  }

  if (node.is_date()) {
    return TomlTypeName<toml::date>();
  }

  if (node.is_time()) {
    return TomlTypeName<toml::time>();
  }

  if (node.is_date_time()) {
    return TomlTypeName<toml::date_time>();
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

/// Extracts the value from the toml::node or throws an error if the type
/// is not correct.
/// Tries converting numeric types if a lossless cast is feasible.
template <typename T, typename NodeView>
T CastScalar(const NodeView &node, std::string_view fqn) {
  if constexpr (std::is_same_v<T, bool>) {
    if (node.is_boolean()) {
      return static_cast<bool>(*node.as_boolean());
    }
  } else if constexpr (std::is_arithmetic_v<T>) {
    if (node.is_integer()) {
      return checked_numcast<T, int64_t, TypeError>(
          static_cast<int64_t>(*node.as_integer()));
    }

    if (node.is_floating_point()) {
      return checked_numcast<T, double, TypeError>(
          static_cast<double>(*node.as_floating_point()));
    }
  } else if constexpr (std::is_same_v<T, std::string>) {
    if (node.is_string()) {
      return std::string{*node.as_string()};
    }
  } else if constexpr (std::is_same_v<T, date>) {
    if (node.is_date()) {
      const toml::date &d = node.as_date()->get();
      return date{d.year, d.month, d.day};
    }
  } else if constexpr (std::is_same_v<T, time>) {
    if (node.is_time()) {
      const toml::time &t = node.as_time()->get();
      return time{t.hour, t.minute, t.second, t.nanosecond};
    }
  } else if constexpr (std::is_same_v<T, date_time>) {
    if (node.is_date_time()) {
      const toml::date_time &dt = node.as_date_time()->get();
      date_time tmp{};
      tmp.date = date{dt.date.year, dt.date.month, dt.date.day};
      tmp.time = time{dt.time.hour, dt.time.minute, dt.time.second,
                      dt.time.nanosecond};
      if (dt.offset.has_value()) {
        tmp.offset = time_offset{dt.offset.value().minutes};
      }
      return tmp;
    }
  } else {
    throw std::logic_error("Type not yet supported!");
  }
  WZK_CONFIG_LOOKUP_RAISE_TOML_TYPE_ERROR(fqn, node, T);
}

/// @brief Looks up the value at the given key (fully-qualified TOML path).
///
/// If the key does not exist, a KeyError will be raised unless
/// `allow_default` is true (in which case the `default_val` will be
/// returned instead).
template <typename T, typename DefaultType = T>
T LookupScalar(const toml::table &tbl, std::string_view key,
               bool allow_default = false,
               DefaultType default_val = DefaultType{}) {
  if (!ContainsKey(tbl, key)) {
    if (allow_default) {
      return T{default_val};
    }

    throw KeyErrorWithSimilarKeys(tbl, key);
  }

  const auto node = tbl.at_path(key);
  return CastScalar<T>(node, key);
}

/// @brief Looks up the value at the given key (fully-qualified TOML path).
///
/// If the key does not exist, a nullopt will be returned.
template <typename T>
std::optional<T> LookupOptional(const toml::table &tbl, std::string_view key) {
  if (!ContainsKey(tbl, key)) {
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
  const std::size_t pos = path.find_last_of('.');
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

/// @brief Converts an interface-exposed type to a TOML type if possible.
/// Throws a type error upon invalid numeric conversion (i.e. the number is not
/// exactly representable by the target type).
/// @tparam Ttoml The TOML type.
/// @tparam Tcfg The type exposed via the werkzeugkiste::config API.
/// @param value The value to be converted.
template <typename Ttoml, typename Tcfg>
Ttoml ConvertConfigTypeToToml(const Tcfg &value) {
  if constexpr (std::is_same_v<Tcfg, bool>) {
    return value;
  }

  if constexpr (std::is_arithmetic_v<Tcfg>) {
    return checked_numcast<Ttoml, Tcfg, TypeError>(value);
  }

  if constexpr (std::is_same_v<Tcfg, std::string> ||
                std::is_same_v<Tcfg, std::string_view>) {
    return std::string{value};
  }

  if constexpr (std::is_same_v<Tcfg, date>) {
    return toml::date{value.year, value.month, value.day};
  }

  if constexpr (std::is_same_v<Tcfg, time>) {
    return toml::time{value.hour, value.minute, value.second, value.nanosecond};
  }

  if constexpr (std::is_same_v<Tcfg, date_time>) {
    if (value.IsLocal()) {
      return toml::date_time{
          toml::date{value.date.year, value.date.month, value.date.day},
          toml::time{value.time.hour, value.time.minute, value.time.second,
                     value.time.nanosecond}};
    } else {  // NOLINT
      toml::time_offset offset{};
      offset.minutes = static_cast<int16_t>(value.offset.value().minutes);
      return toml::date_time{
          toml::date{value.date.year, value.date.month, value.date.day},
          toml::time{value.time.hour, value.time.minute, value.time.second,
                     value.time.nanosecond},
          offset};
    }
  }

  throw std::logic_error{"TODO Missing implementation - file bug report!"};
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
template <typename Ttoml, typename Tmessage = Ttoml, typename Tvalue>
void SetScalar(toml::table &tbl, std::string_view key, Tvalue value) {
  const auto path = SplitTomlPath(key);
  if (ContainsKey(tbl, key)) {
    const auto node = tbl.at_path(key);

    // TODO replace by "IsCompatible"
    if (!node.is<Ttoml>()) {
      std::string msg{"Changing the type is not allowed. Parameter `"};
      msg += key;
      msg += "` is `";
      msg += TomlTypeName(node, key);
      msg += "`, but replacement value is of type `";
      msg += TypeName<Tmessage>();
      msg += "`!";
      throw TypeError{msg};
    }

    auto &ref = *node.as<Ttoml>();
    // ref = Ttoml{value};
    //  FIXME
    ref = ConvertConfigTypeToToml<Ttoml>(value);
  } else {
    EnsureContainerPathExists(tbl, path.first);
    toml::table *parent =
        path.first.empty() ? &tbl : tbl.at_path(path.first).as_table();
    if (parent == nullptr) {
      // LCOV_EXCL_START
      WZK_CONFIG_LOOKUP_RAISE_PATH_CREATION_ERROR(key, path.first);
      // LCOV_EXCL_STOP
    }

    // auto result = parent->insert_or_assign(path.second, value);
    // FIXME
    auto result = parent->insert_or_assign(
        path.second, ConvertConfigTypeToToml<Ttoml>(value));
    if (!ContainsKey(tbl, key)) {
      // LCOV_EXCL_START
      WZK_CONFIG_LOOKUP_RAISE_ASSIGNMENT_ERROR(key, result.second, path.first,
                                               path.second);
      // LCOV_EXCL_STOP
    }
  }
}

// template <typename Tto, typename Tfrom>
// bool IsExactlyRepresentable(const std::vector<Tfrom> &values) {
//   for (const Tfrom &val : values) {
//     checked_numcast<Tto, Tfrom, TypeError>(val);
//   }
//   return true;
// }

// template<typename Tvalue>
// bool IsCompatibleList(const toml::array &arr, const std::vector<Tvalue>
// &values) {
//   // Empty arrays and inhomogeneous arrays can be replaced by anything.
//   if (arr.empty() || !arr.is_homogeneous()) {
//     return true;
//   }

//   // Otherwise, the replacement values must be compatible to the existing
//   // entries:
//   if (arr[0].is_boolean()) {
//     return std::is_same_v<Tvalue, bool>;
//   }

//   if (arr[0].is_integer()) {
//     return IsExactlyRepresentable<int64_t>(values);
//   }

//   if (arr[0].is_floating_point()) {
//     return IsExactlyRepresentable<double>(values);
//   }

//   if (arr[0].is_string()) {
//     return std::is_same_v<Tvalue, std::string> || std::is_same_v<Tvalue,
//     std::string_view>;
//   }

//   if (arr[0].is_date()) {
//     return std::is_same_v<Tvalue, date>;
//   }

//   if (arr[0].is_time()) {
//     return std::is_same_v<Tvalue, time>;
//   }

//   if (arr[0].is_date_time()) {
//     return std::is_same_v<Tvalue, date_time>;
//   }

//   return false;
// }

// TODO doc
// template <typename Ttoml, typename Tmessage = Ttoml, typename Tvalue>
// void SetList(toml::table &tbl, std::string_view key,
//              const std::vector<Tvalue> &values) {
//   const auto path = SplitTomlPath(key);
//   toml::array arr{};
//   for (const auto &val : values) {
//     arr.push_back(ConvertConfigTypeToToml<Ttoml>(val));
//   }

//   if (ContainsKey(tbl, key)) {
//     auto node = tbl.at_path(key);
//     toml::array &existing = *node.as_array();

//     if (!IsCompatibleList(existing, values)) {
//       throw TypeError{"TODO need list type for error message!"};
//     //   std::string msg{"Changing the type is not allowed. Parameter `"};
//     //   msg += key;
//     //   msg += "` is `";
//     //   msg += TomlTypeName(node, key);
//     //   msg += "`, but scalar is of type `";
//     //   msg += TypeName<TMessage>();
//     //   msg += "`!";
//     //   throw TypeError{msg};
//     }

//     // std::size_t index = 0;
//     // for (auto &value : arr) {
//     //   const std::string fqn = FullyQualifiedArrayElementPath(index, path);
//     //   visit_func(value, fqn);

//     //   if (value.is_array() || value.is_table()) {
//     //     Traverse(value, fqn, visit_func);
//     //   }
//     //   ++index;
//     // }

//     // TODO if type is not compatible, throw exception --> Refactor
//     // compatibility check into separate template (to be used within
//     SetScalar,
//     // too)
//     // TODO For int<->double, try casting, similar to CastScalar/LookupScalar

//     // auto &ref = *node.as<T>();
//     // ref = T{value};
//     throw std::logic_error{"Not yet implemented!"};
//   } else {
//     EnsureContainerPathExists(tbl, path.first);
//     toml::table *parent =
//         path.first.empty() ? &tbl : tbl.at_path(path.first).as_table();
//     if (parent == nullptr) {
//       // LCOV_EXCL_START
//       WZK_CONFIG_LOOKUP_RAISE_PATH_CREATION_ERROR(key, path.first);
//       // LCOV_EXCL_STOP
//     }

//     auto result = parent->insert_or_assign(path.second, arr);
//     if (!ContainsKey(tbl, key)) {
//       // LCOV_EXCL_START
//       WZK_CONFIG_LOOKUP_RAISE_ASSIGNMENT_ERROR(
//           key, result.second, path.first, path.second);
//       // LCOV_EXCL_STOP
//     }
//   }
// }

template <typename Ttoml, typename Tvalue>
void CreateArray(toml::table &tbl, std::string_view key,
                 const std::vector<Tvalue> &values) {
  toml::array arr{};
  for (const auto &val : values) {
    arr.push_back(ConvertConfigTypeToToml<Ttoml>(val));
  }

  const auto path = SplitTomlPath(key);

  EnsureContainerPathExists(tbl, path.first);
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
    WZK_CONFIG_LOOKUP_RAISE_ASSIGNMENT_ERROR(key, result.second, path.first,
                                             path.second);
    // LCOV_EXCL_STOP
  }
}

// TODO remove Ttoml & Tmessage
template <typename Ttoml, typename Tmessage, typename Tvalue,
          typename Texisting>
void ReplaceHomogeneousArray(toml::array &arr, std::string_view key,
                             const std::vector<Tvalue> &values) {
  toml::array tval{};
  for (const auto &val : values) {
    tval.push_back(ConvertConfigTypeToToml<Ttoml>(val));
    // TODO catch exception and rethrow with extended error message
  }

  arr = tval;
  // // TODO int/float checked_cast
  // std::string msg{"Changing the type is not allowed. Parameter `"};
  // msg.append(key);
  // msg.append("` is a homogeneous list of `");
  // msg.append(TomlTypeName<Texisting>());
  // msg.append("`, but the replacement list holds `");
  // msg.append(TypeName<Tvalue>());
  // msg.append("`!");
  // throw TypeError{msg};
}

template <typename Ttoml, typename Tmessage, typename Tvalue>
void ReplaceArray(toml::table &tbl, std::string_view key,
                  const std::vector<Tvalue> &values) {
  auto node = tbl.at_path(key);
  if (!node.is_array()) {
    // TODO
    throw TypeError{"TODO"};
  }

  toml::array &arr = *node.as_array();

  // If the existing array is empty, we can simply replace it by inserting
  // the given values as is.
  // If it is inhomogeneous, we don't allow replacing it.
  // Otherwise, we need to check for compatible/convertible types.
  if (arr.empty()) {
    CreateArray<Ttoml, Tvalue>(tbl, key, values);
  } else if (!arr.is_homogeneous()) {
    throw ValueError{"TODO wrong exception"};
  } else if (arr[0].is_boolean()) {
    ReplaceHomogeneousArray<Ttoml, Tmessage, Tvalue, bool>(arr, key, values);
  } else if (arr[0].is_integer()) {
    ReplaceHomogeneousArray<Ttoml, Tmessage, Tvalue, int64_t>(arr, key, values);
  } else if (arr[0].is_floating_point()) {
    ReplaceHomogeneousArray<Ttoml, Tmessage, Tvalue, double>(arr, key, values);
  } else if (arr[0].is_string()) {
    ReplaceHomogeneousArray<Ttoml, Tmessage, Tvalue, std::string>(arr, key,
                                                                  values);
  } else if (arr[0].is_date()) {
    ReplaceHomogeneousArray<Ttoml, Tmessage, Tvalue, toml::date>(arr, key,
                                                                 values);
  } else if (arr[0].is_time()) {
    ReplaceHomogeneousArray<Ttoml, Tmessage, Tvalue, toml::time>(arr, key,
                                                                 values);
  } else if (arr[0].is_date_time()) {
    ReplaceHomogeneousArray<Ttoml, Tmessage, Tvalue, toml::date_time>(arr, key,
                                                                      values);
  } else {
    throw std::logic_error{"TODO"};
  }
}

template <typename Ttoml, typename Tmessage = Ttoml, typename Tvalue>
void SetList(toml::table &tbl, std::string_view key,
             const std::vector<Tvalue> &values) {
  if (ContainsKey(tbl, key)) {
    ReplaceArray<Ttoml, Tmessage, Tvalue>(tbl, key, values);
  } else {
    CreateArray<Ttoml, Tvalue>(tbl, key, values);
  }
}

/// @brief Utility to turn a std::array into a std::tuple.
template <typename Array, std::size_t... Idx>
inline auto ArrayToTuple(const Array &arr,
                         std::index_sequence<Idx...> /* indices */) {
  return std::make_tuple(arr[Idx]...);
}

/// @brief Utility to turn a std::array into a std::tuple.
template <typename Type, std::size_t Num>
inline auto ArrayToTuple(const std::array<Type, Num> &arr) {
  return ArrayToTuple(arr, std::make_index_sequence<Num>{});
}

/// @brief Extracts a single Dim-dimensional point from the given
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

/// @brief Extracts a single Dim-dimensional point as std::tuple from the
/// given toml::array.
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

/// @brief Extracts a single Dim-dimensional point from the given
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

/// @brief Extracts a single Dim-dimensional point as std::tuple from the
/// given toml::table.
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

/// @brief Extracts a list of tuples: This can be used to query a list of
/// "points" (e.g. a polyline), "pixels", or "indices". Each coordinate must
/// be specified as integer or floating point.
template <typename Tuple>
std::vector<Tuple> GetTuples(const toml::table &tbl, std::string_view key) {
  if (!ContainsKey(tbl, key)) {
    throw KeyErrorWithSimilarKeys(tbl, key);
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

/// @brief Extracts a list of built-in scalar types (integer, double, bool).
/// This is *not suitable* for TOML++-specific types (date, time, ...)
template <typename T>
std::vector<T> GetList(const toml::table &tbl, std::string_view key) {
  if (!ContainsKey(tbl, key)) {
    throw KeyErrorWithSimilarKeys(tbl, key);
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

/// @brief Extracts a pair of built-in scalar types (integer, double, bool).
/// This is *not suitable* for TOML++-specific types (date, time, ...)
template <typename T>
std::pair<T, T> GetScalarPair(const toml::table &tbl, std::string_view key) {
  if (!ContainsKey(tbl, key)) {
    throw KeyErrorWithSimilarKeys(tbl, key);
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
}  // namespace detail

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
  const auto keys_this = detail::ListTableKeys(pimpl_->config_root, ""sv,
                                               /*include_array_entries=*/true);
  const auto keys_other = detail::ListTableKeys(other.pimpl_->config_root, ""sv,
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
  return detail::ContainsKey(pimpl_->config_root, key);
}

ConfigType Configuration::Type(std::string_view key) const {
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

std::vector<std::string> Configuration::ListParameterNames(
    bool include_array_entries) const {
  using namespace std::string_view_literals;
  return detail::ListTableKeys(pimpl_->config_root, ""sv,
                               include_array_entries);
}

//---------------------------------------------------------------------------
// Boolean

bool Configuration::GetBoolean(std::string_view key) const {
  return detail::LookupScalar<bool>(pimpl_->config_root, key,
                                    /*allow_default=*/false);
}

bool Configuration::GetBooleanOr(std::string_view key, bool default_val) const {
  return detail::LookupScalar<bool>(pimpl_->config_root, key,
                                    /*allow_default=*/true, default_val);
}

std::optional<bool> Configuration::GetOptionalBoolean(
    std::string_view key) const {
  return detail::LookupOptional<bool>(pimpl_->config_root, key);
}

void Configuration::SetBoolean(std::string_view key, bool value) {
  detail::SetScalar<bool>(pimpl_->config_root, key, value);
}

std::vector<bool> Configuration::GetBooleanList(std::string_view key) const {
  return detail::GetList<bool>(pimpl_->config_root, key);
}

void Configuration::SetBooleanList(std::string_view key,
                                   const std::vector<bool> &values) {
  detail::SetList<bool>(pimpl_->config_root, key, values);  // TODO test
}

//---------------------------------------------------------------------------
// Integer (32-bit)

int32_t Configuration::GetInteger32(std::string_view key) const {
  return detail::LookupScalar<int32_t>(pimpl_->config_root, key,
                                       /*allow_default=*/false);
}

int32_t Configuration::GetInteger32Or(std::string_view key,
                                      int32_t default_val) const {
  return detail::LookupScalar<int32_t>(pimpl_->config_root, key,
                                       /*allow_default=*/true, default_val);
}

std::optional<int32_t> Configuration::GetOptionalInteger32(
    std::string_view key) const {
  return detail::LookupOptional<int32_t>(pimpl_->config_root, key);
}

void Configuration::SetInteger32(std::string_view key, int32_t value) {
  detail::SetScalar<int64_t, int32_t>(pimpl_->config_root, key,
                                      static_cast<int64_t>(value));
}

std::pair<int32_t, int32_t> Configuration::GetInteger32Pair(
    std::string_view key) const {
  return detail::GetScalarPair<int32_t>(pimpl_->config_root, key);
}

std::vector<int32_t> Configuration::GetInteger32List(
    std::string_view key) const {
  return detail::GetList<int32_t>(pimpl_->config_root, key);
}

void Configuration::SetInteger32List(std::string_view key,
                                     const std::vector<int32_t> &values) {
  detail::SetList<int64_t, int32_t>(pimpl_->config_root, key,
                                    values);  // TODO test
}

std::vector<std::tuple<int32_t, int32_t>> Configuration::GetIndices2D(
    std::string_view key) const {
  return detail::GetTuples<std::tuple<int32_t, int32_t>>(pimpl_->config_root,
                                                         key);
}

std::vector<std::tuple<int32_t, int32_t, int32_t>> Configuration::GetIndices3D(
    std::string_view key) const {
  return detail::GetTuples<std::tuple<int32_t, int32_t, int32_t>>(
      pimpl_->config_root, key);
}

//---------------------------------------------------------------------------
// Integer (64-bit)

int64_t Configuration::GetInteger64(std::string_view key) const {
  return detail::LookupScalar<int64_t>(pimpl_->config_root, key,
                                       /*allow_default=*/false);
}

int64_t Configuration::GetInteger64Or(std::string_view key,
                                      int64_t default_val) const {
  return detail::LookupScalar<int64_t>(pimpl_->config_root, key,
                                       /*allow_default=*/true, default_val);
}

std::optional<int64_t> Configuration::GetOptionalInteger64(
    std::string_view key) const {
  return detail::LookupOptional<int64_t>(pimpl_->config_root, key);
}

void Configuration::SetInteger64(std::string_view key, int64_t value) {
  detail::SetScalar<int64_t>(pimpl_->config_root, key, value);
}

std::pair<int64_t, int64_t> Configuration::GetInteger64Pair(
    std::string_view key) const {
  return detail::GetScalarPair<int64_t>(pimpl_->config_root, key);
}

std::vector<int64_t> Configuration::GetInteger64List(
    std::string_view key) const {
  return detail::GetList<int64_t>(pimpl_->config_root, key);
}

void Configuration::SetInteger64List(std::string_view key,
                                     const std::vector<int64_t> &values) {
  detail::SetList<int64_t>(pimpl_->config_root, key, values);  // TODO test
}

//---------------------------------------------------------------------------
// Floating Point

double Configuration::GetDouble(std::string_view key) const {
  return detail::LookupScalar<double>(pimpl_->config_root, key,
                                      /*allow_default=*/false);
}

double Configuration::GetDoubleOr(std::string_view key,
                                  double default_val) const {
  return detail::LookupScalar<double>(pimpl_->config_root, key,
                                      /*allow_default=*/true, default_val);
}

std::optional<double> Configuration::GetOptionalDouble(
    std::string_view key) const {
  return detail::LookupOptional<double>(pimpl_->config_root, key);
}

void Configuration::SetDouble(std::string_view key, double value) {
  detail::SetScalar<double>(pimpl_->config_root, key, value);
}

std::pair<double, double> Configuration::GetDoublePair(
    std::string_view key) const {
  return detail::GetScalarPair<double>(pimpl_->config_root, key);
}

std::vector<double> Configuration::GetDoubleList(std::string_view key) const {
  return detail::GetList<double>(pimpl_->config_root, key);
}

void Configuration::SetDoubleList(std::string_view key,
                                  const std::vector<double> &values) {
  detail::SetList<double>(pimpl_->config_root, key, values);  // TODO test
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
  return detail::LookupOptional<std::string>(pimpl_->config_root, key);
}

void Configuration::SetString(std::string_view key, std::string_view value) {
  detail::SetScalar<std::string, std::string, std::string_view>(
      pimpl_->config_root, key, value);
}

std::vector<std::string> Configuration::GetStringList(
    std::string_view key) const {
  return detail::GetList<std::string>(pimpl_->config_root, key);
}

void Configuration::SetStringList(std::string_view key,
                                  const std::vector<std::string_view> &values) {
  detail::SetList<std::string, std::string, std::string_view>(
      pimpl_->config_root, key, values);  // TODO test
}

//---------------------------------------------------------------------------
// Date

date Configuration::GetDate(std::string_view key) const {
  return detail::LookupScalar<date>(pimpl_->config_root, key,
                                    /*allow_default=*/false);
}

date Configuration::GetDateOr(std::string_view key,
                              const date &default_val) const {
  return detail::LookupScalar<date>(pimpl_->config_root, key,
                                    /*allow_default=*/true, default_val);
}

std::optional<date> Configuration::GetOptionalDate(std::string_view key) const {
  return detail::LookupOptional<date>(pimpl_->config_root, key);
}

void Configuration::SetDate(std::string_view key, const date &value) {
  // using namespace std::string_view_literals;
  // FIXME
  // detail::SetDateTime<date, toml::date>(pimpl_->config_root, key, value,
  //                                       "date"sv);
  detail::SetScalar<toml::date>(pimpl_->config_root, key, value);
}

std::vector<date> Configuration::GetDateList(std::string_view key) const {
  return detail::GetList<date>(pimpl_->config_root, key);  // TODO test
}

//---------------------------------------------------------------------------
// Time

time Configuration::GetTime(std::string_view key) const {
  return detail::LookupScalar<time>(pimpl_->config_root, key,
                                    /*allow_default=*/false);
}

time Configuration::GetTimeOr(std::string_view key,
                              const time &default_val) const {
  return detail::LookupScalar<time>(pimpl_->config_root, key,
                                    /*allow_default=*/true, default_val);
}

std::optional<time> Configuration::GetOptionalTime(std::string_view key) const {
  return detail::LookupOptional<time>(pimpl_->config_root, key);
}

void Configuration::SetTime(std::string_view key, const time &value) {
  // using namespace std::string_view_literals;
  // detail::SetDateTime<time, toml::time>(pimpl_->config_root, key, value,
  //                                       "time"sv);
  // FIXME
  detail::SetScalar<toml::time>(pimpl_->config_root, key, value);
}

std::vector<time> Configuration::GetTimeList(std::string_view key) const {
  return detail::GetList<time>(pimpl_->config_root, key);  // TODO test
}

//---------------------------------------------------------------------------
// Date-time

date_time Configuration::GetDateTime(std::string_view key) const {
  return detail::LookupScalar<date_time>(pimpl_->config_root, key,
                                         /*allow_default=*/false);
}

date_time Configuration::GetDateTimeOr(std::string_view key,
                                       const date_time &default_val) const {
  return detail::LookupScalar<date_time>(pimpl_->config_root, key,
                                         /*allow_default=*/true, default_val);
}

std::optional<date_time> Configuration::GetOptionalDateTime(
    std::string_view key) const {
  return detail::LookupOptional<date_time>(pimpl_->config_root, key);
}

void Configuration::SetDateTime(std::string_view key, const date_time &value) {
  // using namespace std::string_view_literals;
  // detail::SetDateTime<date_time, toml::date_time>(pimpl_->config_root, key,
  //                                                 value, "date_time"sv);
  // FIXME
  detail::SetScalar<toml::date_time>(pimpl_->config_root, key, value);
}

std::vector<date_time> Configuration::GetDateTimeList(
    std::string_view key) const {
  return detail::GetList<date_time>(pimpl_->config_root,
                                    key);  // TODO test
}

//---------------------------------------------------------------------------
// Group/"Sub-Configuration"

Configuration Configuration::GetGroup(std::string_view key) const {
  if (!Contains(key)) {
    throw detail::KeyErrorWithSimilarKeys(pimpl_->config_root, key);
  }

  Configuration cfg;
  const auto nv = pimpl_->config_root.at_path(key);

  if (!nv.is_table()) {
    std::string msg{"Cannot retrieve `"};
    msg += key;
    msg += "` as a group, because it is a`";
    msg += detail::TomlTypeName(nv, key);
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
      WZK_CONFIG_LOOKUP_RAISE_ASSIGNMENT_ERROR(key, result.second, path.first,
                                               path.second);
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
  const KeyMatcher matcher{parameters};
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
        msg += detail::TomlTypeName(node, fqn);
        msg += "`!";
        throw TypeError{msg};
      }

      // Check if the path is relative
      const std::string param_str = detail::CastScalar<std::string>(node, fqn);
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
  detail::Traverse(pimpl_->config_root, ""sv, func);
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
      std::string param_str = detail::CastScalar<std::string>(node, fqn);
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
  detail::Traverse(pimpl_->config_root, ""sv, func);
  return replaced;
}

void Configuration::LoadNestedTOMLConfiguration(std::string_view key) {
  // TODO refactor (TOML/JSON --> function handle)

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

  // To replace the node, we first have to remove it.
  const std::string fname = std::string(*node.as_string());

  try {
    auto nested_tbl = toml::parse_file(fname);

    const auto path = detail::SplitTomlPath(key);

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

#undef WZK_CONFIG_LOOKUP_RAISE_TOML_TYPE_ERROR
#undef WZK_CONFIG_LOOKUP_RAISE_PATH_CREATION_ERROR
#undef WZK_CONFIG_LOOKUP_RAISE_ASSIGNMENT_ERROR
}  // namespace werkzeugkiste::config
