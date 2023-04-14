#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/files/fileio.h>
#include <werkzeugkiste/logging.h>  // TODO remove
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

/// @brief Returns true if the scalar node is tagged.
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
    std::string msg{
        "HandleTaggedScalar called with untagged node for parameter `"};
    msg += fqn;
    msg += "`!";
    throw std::logic_error(msg);
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
    AppendOrSet(std::stol(value), cfg, fqn, append);
  } else if (tag == "tag:yaml.org,2002:float") {
    AppendOrSet(std::stod(value), cfg, fqn, append);
  } else if ((tag == "tag:yaml.org,2002:date") ||
             (tag == "tag:yaml.org,2002:timestamp") || (tag == "!date")) {
    DecodeDateOrDateTime(node, cfg, fqn, append);
  } else if ((tag == "tag:yaml.org,2002:time") || (tag == "!time")) {
    DecodeTime(node, cfg, fqn, append);
  } else {
    std::string msg{"YAML tag `" + tag + "` is not supported!"};
    throw std::logic_error{msg};
  }
}

// TODO doc
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
  std::string msg{"Could not decode scalar YAML node for parameter `"};
  msg += fqn;
  msg +=
      "`! This is a bug, please report at "
      "https://github.com/snototter/werkzeugkiste/issues";
  throw std::logic_error{msg};
  // LCOV_EXCL_STOP
}

// NOLINTNEXTLINE(misc-no-recursion)
void AppendListItems(const YAML::Node &node,
    Configuration &cfg,
    std::string_view key,
    NullValuePolicy none_policy) {
  // LCOV_EXCL_START
  if (!node.IsSequence() || !cfg.Contains(key)) {
    std::string msg{
        "AppendListItem requires that the YAML node is a sequence ("};
    msg += (node.IsSequence() ? "which it is" : "which it is NOT");
    msg += ") and that the list parameter `";
    msg += key;
    msg += "` has already been created (";
    msg +=
        (cfg.Contains(key) ? "which has been done" : "which has NOT been done");
    msg += ")!";
    throw std::logic_error{msg};
  }
  // LCOV_EXCL_STOP

  for (const auto &item : node) {
    switch (item.Type()) {
      case YAML::NodeType::Null:
        Configuration::HandleNullValue(cfg, key, none_policy, /*append=*/true);
        break;

      case YAML::NodeType::Scalar:
        HandleScalarNode(item, cfg, key, /*append=*/true);
        break;

      case YAML::NodeType::Sequence: {
        const std::size_t lst_size = cfg.Size(key);
        const std::string elem_key =
            Configuration::KeyForListElement(key, lst_size);
        cfg.AppendList(key);
        AppendListItems(item, cfg, elem_key, none_policy);
        break;
      }

      case YAML::NodeType::Map:
        cfg.Append(key, FromYAMLNode(item, none_policy));
        break;

      // LCOV_EXCL_START
      case YAML::NodeType::Undefined: {
        std::string msg{"Undefined YAML node type for parameter `"};
        msg += key;
        msg += "`!";
        throw std::logic_error{msg};
      }
        // LCOV_EXCL_STOP
    }
  }
}

// NOLINTNEXTLINE(misc-no-recursion)
Configuration FromYAMLNode(const YAML::Node &node,
    NullValuePolicy none_policy) {
  // LCOV_EXCL_START
  if (!node.IsMap()) {
    throw std::logic_error{
        "FromYAMLNode requires that the YAML node is a map!"};
  }
  // LCOV_EXCL_STOP

  Configuration cfg{};
  for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
    const std::string key = it->first.as<std::string>();
    switch (it->second.Type()) {
      case YAML::NodeType::Null:
        Configuration::HandleNullValue(cfg, key, none_policy, /*append=*/false);
        break;

      case YAML::NodeType::Scalar:
        HandleScalarNode(it->second, cfg, key, /*append=*/false);
        break;

      case YAML::NodeType::Sequence: {
        cfg.CreateList(key);
        AppendListItems(it->second, cfg, key, none_policy);
        break;
      }

      case YAML::NodeType::Map:
        cfg.SetGroup(key, FromYAMLNode(it->second, none_policy));
        break;

      // LCOV_EXCL_START
      case YAML::NodeType::Undefined: {
        std::string msg{"Undefined YAML node type for parameter `"};
        msg += key;
        msg += "`!";
        throw std::logic_error{msg};
      }
        // LCOV_EXCL_STOP
    }
  }
  return cfg;
}
}  // namespace detail

Configuration LoadYAMLString(const std::string &yaml_string,
    NullValuePolicy none_policy) {
  try {
    // WZKLOG_CRITICAL("INPUT TO PARSER\n{}", yaml_string); // TODO remove
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
