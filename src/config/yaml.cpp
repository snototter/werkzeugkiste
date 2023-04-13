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
    // LCOV_EXCL_START
    if (!node.IsScalar()) {
      return false;
    }
    // LCOV_EXCL_STOP

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
    // LCOV_EXCL_START
    if (!node.IsScalar()) {
      return false;
    }
    // LCOV_EXCL_STOP

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
    // LCOV_EXCL_START
    if (!node.IsScalar()) {
      return false;
    }
    // LCOV_EXCL_STOP

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

/// @brief Returns true if the scalar node is tagged.
inline bool HasTag(const YAML::Node &node) {
  // LCOV_EXCL_START
  if (!node.IsScalar()) {
    return false;
  }
  // LCOV_EXCL_STOP
  const std::string &tag = node.Tag();
  return (!tag.empty() && (tag != "?"));
}

template <typename Tp>
std::optional<Tp> DecodeUntaggedScalarNode(const YAML::Node &node) {
  // LCOV_EXCL_START
  if (!node.IsScalar()) {
    // WZKLOG_CRITICAL(  // TODO remove
    //     "NODE IS NOT SCALAR! map {}, seq {}, scalar {}, null {}, defined {}",
    //     node.IsMap(),
    //     node.IsSequence(),
    //     node.IsScalar(),
    //     node.IsNull()),
    //     node.IsDefined();
    return std::nullopt;
  }
  // LCOV_EXCL_STOP

  // WZKLOG_CRITICAL("try decoding UNTAGGED node into type {}, value {}, tag
  // {}",
  //     TypeName<Tp>(),
  //     node.Scalar(),
  //     node.Tag());  // TODO remove

  // Use YAML::convert to avoid YAML::BadConversion being thrown.
  Tp typed;
  if (YAML::convert<Tp>::decode(node, typed)) {
    // WZKLOG_CRITICAL("-----> succeeded");  // TODO remove
    return typed;
  }

  return std::nullopt;
}

/// @brief Simplified node handling if the node is tagged.
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void HandleTaggedScalar(const YAML::Node &node,
    Configuration &cfg,
    std::string_view fqn,
    bool append) {
  // LCOV_EXCL_START
  if (!HasTag(node)) {
    std::string msg{
        "HandleTaggedScalar called with untagged node for parameter `"};
    msg += fqn;
    msg += "`!";
    throw std::logic_error(msg);
  }
  // LCOV_EXCL_STOP

  // TODO document supported tags

  const std::string &tag = node.Tag();
  const std::string &value = node.Scalar();
  if ((tag == "tag:yaml.org,2002:str") || (tag == "!")) {
    if (append) {
      cfg.Append(fqn, value);
    } else {
      cfg.SetString(fqn, value);
    }
  } else if (tag == "tag:yaml.org,2002:bool") {
    const bool val = node.as<bool>();
    if (append) {
      cfg.Append(fqn, val);
    } else {
      cfg.SetBool(fqn, val);
    }
  } else if (tag == "tag:yaml.org,2002:int") {
    const int64_t val = std::stol(value);
    if (append) {
      cfg.Append(fqn, val);
    } else {
      cfg.SetInt64(fqn, val);
    }
  } else if (tag == "tag:yaml.org,2002:float") {
    const double val = std::stod(value);
    if (append) {
      cfg.Append(fqn, val);
    } else {
      cfg.SetDouble(fqn, val);
    }
  } else if ((tag == "tag:yaml.org,2002:date") || (tag == "!date")) {
    // A YAML date can be either a date or a date-time.
    date val_date{};
    date_time val_dt{};
    if (YAML::convert<date>::decode(node, val_date)) {
      if (append) {
        cfg.Append(fqn, val_date);
      } else {
        cfg.SetDate(fqn, val_date);
      }
    } else if (YAML::convert<date_time>::decode(node, val_dt)) {
      if (append) {
        cfg.Append(fqn, val_dt);
      } else {
        cfg.SetDateTime(fqn, val_dt);
      }
    } else {
      std::string msg{
          "Failed to parse date from tagged YAML node `" + value + "`!"};
      throw ParseError{msg};
    }
  } else if ((tag == "tag:yaml.org,2002:time") || (tag == "!time")) {
    time val_time{};
    if (YAML::convert<time>::decode(node, val_time)) {
      if (append) {
        cfg.Append(fqn, val_time);
      } else {
        cfg.SetTime(fqn, val_time);
      }
    } else {
      std::string msg{
          "Failed to parse time from tagged YAML node `" + value + "`!"};
      throw ParseError{msg};
    }
  } else {
    std::string msg{"YAML tag `" + tag + "` is not supported!"};
    throw std::logic_error{msg};
  }
}

/// @brief Sets or appends the scalar value to the configuration.
/// @param node The YAML node which holds the scalar.
/// @param cfg The configuration.
/// @param fqn The fully qualified parameter name.
/// @param append If true, the value is appended (assuming that key is an
///   existing list). Otherwise, the value is Set<T> according to the
///   deduced type.
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void HandleScalarNode(const YAML::Node &node,
    Configuration &cfg,
    std::string_view fqn,
    bool append) {
  if (HasTag(node)) {
    HandleTaggedScalar(node, cfg, fqn, append);
    return;
  }

  // Node is not tagged, so we try to decode it into the supported types.
  // Any scalar can be represented as a string, thus ensure to check it last!
  const auto val_bool = DecodeUntaggedScalarNode<bool>(node);
  if (val_bool.has_value()) {
    if (append) {
      cfg.Append(fqn, val_bool.value());
    } else {
      cfg.SetBool(fqn, val_bool.value());
    }
    return;
  }

  const auto val_int = DecodeUntaggedScalarNode<int64_t>(node);
  if (val_int.has_value()) {
    if (append) {
      cfg.Append(fqn, val_int.value());
    } else {
      cfg.SetInt64(fqn, val_int.value());
    }
    return;
  }

  const auto val_double = DecodeUntaggedScalarNode<double>(node);
  if (val_double.has_value()) {
    if (append) {
      cfg.Append(fqn, val_double.value());
    } else {
      cfg.SetDouble(fqn, val_double.value());
    }
    return;
  }

  const auto val_date = DecodeUntaggedScalarNode<date>(node);
  if (val_date.has_value()) {
    if (append) {
      cfg.Append(fqn, val_date.value());
    } else {
      cfg.SetDate(fqn, val_date.value());
    }
    return;
  }

  const auto val_time = DecodeUntaggedScalarNode<time>(node);
  if (val_time.has_value()) {
    if (append) {
      cfg.Append(fqn, val_time.value());
    } else {
      cfg.SetTime(fqn, val_time.value());
    }
    return;
  }

  // TODO test date types - YAML date examples used a more lenient format; need
  // to check in detail
  const auto val_datetime = DecodeUntaggedScalarNode<date_time>(node);
  if (val_datetime.has_value()) {
    if (append) {
      cfg.Append(fqn, val_datetime.value());
    } else {
      cfg.SetDateTime(fqn, val_datetime.value());
    }
    return;
  }

  const auto val_string = DecodeUntaggedScalarNode<std::string>(node);
  if (val_string.has_value()) {
    if (append) {
      cfg.Append(fqn, val_string.value());
    } else {
      cfg.SetString(fqn, val_string.value());
    }
    return;
  }

  // LCOV_EXCL_START
  std::string msg{"Could not decode scalar YAML node for parameter `"};
  msg += fqn;
  msg += "`! This is a bug, please report it!";
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
      // TODO remove
      // WZKLOG_CRITICAL("parsed into config:\n{}", cfg.ToTOML());
      return cfg;
    }

    if (node.IsSequence()) {
      Configuration cfg{};
      const std::string_view key{"list"};
      cfg.CreateList(key);
      detail::AppendListItems(node, cfg, key, none_policy);
      // WZKLOG_CRITICAL("parsed LIST into config:\n{}", cfg.ToTOML());
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
