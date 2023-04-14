#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/files/fileio.h>
#include <werkzeugkiste/strings/strings.h>

// NOLINTBEGIN
#include <yaml-cpp/yaml.h>
// NOLINTEND

#include <fstream>
#include <string>
#include <string_view>

// NOLINTBEGIN(readability-identifier-naming, misc-no-recursion)
namespace YAML {
/// @brief YAML converter to load a YAML node as a date.
template <>
struct convert<werkzeugkiste::config::date> {
  static bool decode(const Node &node, werkzeugkiste::config::date &rhs) {
    if (!node.IsScalar()) {
      return false;
    }

    try {
      rhs = werkzeugkiste::config::date(node.as<std::string>());
      return true;
    } catch (const werkzeugkiste::config::ParseError &) {
      return false;
    }
  }
};

/// @brief YAML converter to load a YAML node as a time.
template <>
struct convert<werkzeugkiste::config::time> {
  static bool decode(const Node &node, werkzeugkiste::config::time &rhs) {
    if (!node.IsScalar()) {
      return false;
    }

    try {
      rhs = werkzeugkiste::config::time(node.as<std::string>());
      return true;
    } catch (const werkzeugkiste::config::ParseError &) {
      return false;
    }
  }
};

/// @brief YAML converter to load a YAML node as a date-time.
template <>
struct convert<werkzeugkiste::config::date_time> {
  static bool decode(const Node &node, werkzeugkiste::config::date_time &rhs) {
    if (!node.IsScalar()) {
      return false;
    }

    try {
      rhs = werkzeugkiste::config::date_time(node.as<std::string>());
      return true;
    } catch (const werkzeugkiste::config::ParseError &) {
      return false;
    }
  }
};
}  // namespace YAML
// NOLINTEND(readability-identifier-naming, misc-no-recursion)

namespace werkzeugkiste::config {
namespace detail {
// Forward declaration
Configuration FromYAMLNode(const YAML::Node &node, NullValuePolicy none_policy);

void AppendListItems(const YAML::Node &node,
    Configuration &cfg,
    std::string_view key,
    NullValuePolicy none_policy);

// LCOV_EXCL_START
/// @brief Throws a std::logic_error with a hint to report an error.
/// @param prefix Error message prefix.
/// @param fqn Fully qualified name of the parameter that caused the error. If
///   provided, " for parameter `fqn`!" will be appended to the error message.
/// @throws std::logic_error
inline void ThrowImplementationError(std::string_view prefix,
    std::string_view fqn) {
  std::string msg{prefix};
  if (!fqn.empty()) {
    msg += " for parameter `";
    msg += fqn;
    msg += "`!";
  }
  msg +=
      " Please report at "
      "https://github.com/snototter/werkzeugkiste/issues";
  throw std::logic_error{msg};
}
// LCOV_EXCL_STOP

/// @brief Appends or sets a configuration value from a decoded scalar value.
/// @tparam Tp Type of the scalar value.
/// @param value The decoded value.
/// @param cfg Configuration to be modified.
/// @param fqn Fully qualified parameter name.
/// @param append If true, fqn is assumed to be a list and the value will be
///   `Append`ed. Otherwise, the value will be `Set`.
template <typename Tp>
void AppendOrSet(Tp value,
    Configuration &cfg,
    std::string_view fqn,
    bool append) {
  if (append) {
    cfg.Append(fqn, value);
  } else {
    cfg.Set(fqn, value);
  }
}

/// @brief Returns true if the scalar node is tagged, i.e. has a
/// non-s
inline bool ScalarHasTag(const YAML::Node &node) {
  if (!node.IsScalar()) {
    // LCOV_EXCL_START
    // This should never happen.
    return false;
    // LCOV_EXCL_STOP
  }
  const std::string &tag = node.Tag();
  return (!tag.empty() && (tag != "?"));
}

template <typename Tp>
std::optional<Tp> DecodeUntaggedScalarNode(const YAML::Node &node) {
  if (!node.IsScalar()) {
    // LCOV_EXCL_START
    // This should never happen.
    return std::nullopt;
    // LCOV_EXCL_STOP
  }

  // Use YAML::convert to avoid YAML::BadConversion being thrown.
  Tp typed;
  if (YAML::convert<Tp>::decode(node, typed)) {
    return typed;
  }

  return std::nullopt;
}

void DecodeDateOrDateTime(const YAML::Node &node,
    Configuration &cfg,
    std::string_view fqn,
    bool append) {
  // A YAML date can be either a date or a date-time.
  date val_date{};
  date_time val_dt{};
  if (YAML::convert<date>::decode(node, val_date)) {
    AppendOrSet(val_date, cfg, fqn, append);
  } else if (YAML::convert<date_time>::decode(node, val_dt)) {
    AppendOrSet(val_dt, cfg, fqn, append);
  } else {
    std::string msg{
        "Failed to parse date from YAML node `" + node.Scalar() + "`!"};
    throw ParseError{msg};
  }
}

void DecodeTime(const YAML::Node &node,
    Configuration &cfg,
    std::string_view fqn,
    bool append) {
  // A YAML time can be either a time or a date-time.
  time val_time{};
  date_time val_dt{};
  if (YAML::convert<time>::decode(node, val_time)) {
    AppendOrSet(val_time, cfg, fqn, append);
  } else if (YAML::convert<date_time>::decode(node, val_dt)) {
    AppendOrSet(val_dt, cfg, fqn, append);
  } else {
    std::string msg{
        "Failed to parse time from YAML node `" + node.Scalar() + "`!"};
    throw ParseError{msg};
  }
}

/// @brief Simplified node handling if the node is tagged.
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void HandleTaggedScalar(const YAML::Node &node,
    Configuration &cfg,
    std::string_view fqn,
    bool append) {
  if (!ScalarHasTag(node)) {
    // LCOV_EXCL_START
    // This should never happen.
    ThrowImplementationError(
        "HandleTaggedScalar called with untagged node", fqn);
    // LCOV_EXCL_STOP
  }

  // TODO document supported tags
  // !!str "some string"
  // !!bool !!int !!float !!date !!timestamp
  // non-standard local tags: !date !time
  // https://yaml.org/type/timestamp.html

  const std::string &tag = node.Tag();
  const std::string &value = node.Scalar();
  if ((tag == "tag:yaml.org,2002:str") || (tag == "!")) {
    // If a node has the "!" (non-specific) tag, it is either a map, sequence
    // or string. Since we already checked for it being a scalar, "!" must
    // indicate it is a string according to the specification:
    // https://yaml.org/spec/1.2.2/
    AppendOrSet(value, cfg, fqn, append);
  } else if (tag == "tag:yaml.org,2002:bool") {
    AppendOrSet(node.as<bool>(), cfg, fqn, append);
  } else if (tag == "tag:yaml.org,2002:int") {
    const int64_t val = std::stol(value);
    AppendOrSet(val, cfg, fqn, append);
  } else if (tag == "tag:yaml.org,2002:float") {
    AppendOrSet(std::stod(value), cfg, fqn, append);
  } else if ((tag == "tag:yaml.org,2002:date") ||
             (tag == "tag:yaml.org,2002:timestamp") || (tag == "!date")) {
    DecodeDateOrDateTime(node, cfg, fqn, append);
  } else if ((tag == "tag:yaml.org,2002:time") || (tag == "!time")) {
    DecodeTime(node, cfg, fqn, append);
  } else {
    std::string msg{"YAML tag `" + tag + "` is not supported"};
    ThrowImplementationError(msg, fqn);
  }
}

/// @brief Tries to decode the scalar node into a value of type Tp. Upon
///   success, the value is set or appended to the configuration.
/// @tparam Tp Type to decode the node into.
/// @param node The YAML node which holds the scalar.
/// @param cfg The configuration.
/// @param fqn The fully qualified parameter name.
/// @param append If true, the value is `Append`ed (assuming that `fqn` is an
///   existing list). Otherwise, the value is `Set`.
/// @return true if the node was decoded successfully, false otherwise.
template <typename Tp>
bool HandleUntaggedScalar(const YAML::Node &node,
    Configuration &cfg,
    std::string_view fqn,
    bool append) {
  const auto val = DecodeUntaggedScalarNode<Tp>(node);
  if (!val.has_value()) {
    return false;
  }

  if (append) {
    cfg.Append(fqn, val.value());
  } else {
    cfg.Set(fqn, val.value());
  }
  return true;
}

/// @brief Sets or appends the scalar value to the configuration.
/// @param node The YAML node which holds the scalar.
/// @param cfg The configuration.
/// @param fqn The fully qualified parameter name.
/// @param append If true, the value is appended (assuming that key is an
///   existing list). Otherwise, the value is Set<T> according to the
///   deduced type.
void HandleScalarNode(const YAML::Node &node,
    Configuration &cfg,
    std::string_view fqn,
    bool append) {
  if (ScalarHasTag(node)) {
    HandleTaggedScalar(node, cfg, fqn, append);
    return;
  }

  // Node is not tagged, so we try to decode it into the supported types.
  if (HandleUntaggedScalar<bool>(node, cfg, fqn, append)) {
    return;
  }

  if (HandleUntaggedScalar<int64_t>(node, cfg, fqn, append)) {
    return;
  }

  if (HandleUntaggedScalar<double>(node, cfg, fqn, append)) {
    return;
  }

  if (HandleUntaggedScalar<date>(node, cfg, fqn, append)) {
    return;
  }

  if (HandleUntaggedScalar<time>(node, cfg, fqn, append)) {
    return;
  }

  // TODO test date types - YAML date examples used a more lenient format; need
  // to check in detail
  if (HandleUntaggedScalar<date_time>(node, cfg, fqn, append)) {
    return;
  }

  // Any scalar can be represented as a string, thus ensure to check it last!
  if (HandleUntaggedScalar<std::string>(node, cfg, fqn, append)) {
    return;
  }

  // LCOV_EXCL_START
  // This should never happen (any scalar should be convertible to a string).
  ThrowImplementationError("Could not decode the YAML scalar", fqn);
  // LCOV_EXCL_STOP
}

/// @brief Appends or sets a configuration value from a parsed YAML node.
/// @param node Value to be added to the configuration.
/// @param cfg Configuration to be modified.
/// @param fqn Fully qualified parameter name.
/// @param append If true, fqn is assumed to be a list and the value will be
///   appended. Otherwise, the value will be set, i.e. "cfg[fqn] = value";
// NOLINTNEXTLINE(misc-no-recursion)
void HandleNode(const YAML::Node &node,
    Configuration &cfg,
    std::string_view fqn,
    NullValuePolicy none_policy,
    bool append) {
  switch (node.Type()) {
    case YAML::NodeType::Null:
      Configuration::HandleNullValue(cfg, fqn, none_policy, append);
      break;

    case YAML::NodeType::Scalar:
      HandleScalarNode(node, cfg, fqn, append);
      break;

    case YAML::NodeType::Sequence: {
      if (append) {
        const std::size_t lst_size = cfg.Size(fqn);
        const std::string elem_key =
            Configuration::KeyForListElement(fqn, lst_size);
        cfg.AppendList(fqn);
        AppendListItems(node, cfg, elem_key, none_policy);
      } else {
        cfg.CreateList(fqn);
        AppendListItems(node, cfg, fqn, none_policy);
      }
      break;
    }

    case YAML::NodeType::Map: {
      if (append) {
        cfg.Append(fqn, FromYAMLNode(node, none_policy));
      } else {
        cfg.Set(fqn, FromYAMLNode(node, none_policy));
      }
      break;
    }

    // LCOV_EXCL_START
    case YAML::NodeType::Undefined: {
      std::string msg{"Undefined YAML node type for parameter `"};
      msg += fqn;
      msg += "`!";
      throw TypeError{msg};
    }
      // LCOV_EXCL_STOP
  }
}

/// @brief Appends all child nodes of the given YAML list to the configuration.
/// @param node The YAML list.
/// @param cfg Configuration to be modified.
/// @param fqn Fully qualified parameter name. This must be a list already
///   existing in the configuration.
/// @param none_policy How to deal with null/none values.
// NOLINTNEXTLINE(misc-no-recursion)
void AppendListItems(const YAML::Node &node,
    Configuration &cfg,
    std::string_view fqn,
    NullValuePolicy none_policy) {
  // LCOV_EXCL_START
  // The following failures should never happen, unless we messed
  //  up the internal logic.
  if (!node.IsSequence()) {
    ThrowImplementationError(
        "Internal util `AppendListItem` called without sequence node", fqn);
  }
  if (!cfg.Contains(fqn)) {
    ThrowImplementationError(
        "Internal util `AppendListItem` called without existing list", fqn);
  }
  // LCOV_EXCL_STOP

  for (const auto &item : node) {
    HandleNode(item, cfg, fqn, none_policy, /*append=*/true);
  }
}

/// @brief Parses a YAML map into a Configuration.
/// @param node The YAML map.
/// @param none_policy How to deal with null/none values.
// NOLINTNEXTLINE(misc-no-recursion)
Configuration FromYAMLNode(const YAML::Node &node,
    NullValuePolicy none_policy) {
  if (!node.IsMap()) {
    // LCOV_EXCL_START
    // This should never happen.
    ThrowImplementationError(
        "Internal util `FromYAMLNode` called without map!", "");
    // LCOV_EXCL_STOP
  }

  Configuration cfg{};
  for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
    const std::string key = it->first.as<std::string>();
    HandleNode(it->second, cfg, key, none_policy, /*append=*/false);
  }
  return cfg;
}
}  // namespace detail

Configuration LoadYAMLString(const std::string &yaml_string,
    NullValuePolicy none_policy) {
  try {
    YAML::Node node = YAML::Load(yaml_string);
    if (node.IsMap()) {
      auto cfg = detail::FromYAMLNode(node, none_policy);
      return cfg;
    }

    if (node.IsSequence()) {
      Configuration cfg{};
      const std::string_view key{"list"};
      cfg.CreateList(key);
      detail::AppendListItems(node, cfg, key, none_policy);
      return cfg;
    }

    // LCOV_EXCL_START
    throw ParseError{
        "Could not parse YAML, because root node is neither a map nor a "
        "sequence!"};
    // LCOV_EXCL_STOP
  } catch (const YAML::Exception &e) {
    throw ParseError(e.what());
  }
}

Configuration LoadYAMLFile(std::string_view filename,
    NullValuePolicy none_policy) {
  try {
    return LoadYAMLString(files::CatAsciiFile(filename), none_policy);
  } catch (const werkzeugkiste::files::IOError &e) {
    throw ParseError(e.what());
  }
}
}  // namespace werkzeugkiste::config
