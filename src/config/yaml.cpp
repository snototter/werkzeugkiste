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

// Forward declarations
namespace werkzeugkiste::config::detail {
void SetScalar(const YAML::Node &node,
    Configuration &cfg,
    std::string_view key);
void AppendListItems(const YAML::Node &node,
    Configuration &cfg,
    std::string_view key);
}  // namespace werkzeugkiste::config::detail

// NOLINTBEGIN(readability-identifier-naming, misc-no-recursion)
namespace YAML {
/// @brief Specialization of the YAML::convert template for a Configuration.
template <>
struct convert<werkzeugkiste::config::Configuration> {
  // Encode skipped on purpose, as we reuse the YAML formatter of toml++.

  static bool decode(const Node &node,
      werkzeugkiste::config::Configuration &rhs) {
    if (!node.IsMap()) {
      return false;
    }

    for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
      const std::string key = it->first.as<std::string>();
      WZKLOG_CRITICAL("decoding config, key: {}", key);
      switch (it->second.Type()) {
        case YAML::NodeType::Null:
          // rhs.Set(key, werkzeugkiste::config::NullValue{});
          // break;
          break;
      }
      if (it->second.IsScalar()) {
        werkzeugkiste::config::detail::SetScalar(it->second, rhs, key);
      } else if (it->second.IsSequence()) {
        rhs.CreateList(key);
        werkzeugkiste::config::detail::AppendListItems(it->second, rhs, key);
      } else if (it->second.IsMap()) {
        rhs.SetGroup(
            key, it->second.as<werkzeugkiste::config::Configuration>());
      } else {
        // TODO undefined or none!
      }
    }
    return true;
  }
};

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
std::optional<Tp> DecodeScalarNode(const YAML::Node &node) {
  if (!node.IsScalar()) {
    WZKLOG_CRITICAL(
        "NODE IS NOT SCALAR! map {}, seq {}, scalar {}, null {}, defined {}",
        node.IsMap(),
        node.IsSequence(),
        node.IsScalar(),
        node.IsNull()),
        node.IsDefined();
    return std::nullopt;
  }

  const std::string &tag = node.Tag();
  const std::string &val = node.Scalar();

  // try {
  WZKLOG_CRITICAL("try decoding node into type {}, value {}, tag {}",
      TypeName<Tp>(),
      val,
      node.Tag());
  Tp typed;
  // Use YAML::convert to avoid YAML::BadConversion being thrown.
  if (YAML::convert<Tp>::decode(node, typed)) {
    WZKLOG_CRITICAL("-----> succeeded");
    return typed;
  }

  return std::nullopt;
}

void SetTaggedScalar(const YAML::Node &node,
    Configuration &cfg,
    std::string_view fqn) {
  // LCOV_EXCL_START
  if (!HasTag(node)) {
    std::string msg{
        "SetTaggedScalar called with untagged node for parameter `"};
    msg += fqn;
    msg += "`!";
    throw std::logic_error(msg);
  }
  // LCOV_EXCL_STOP

  // TODO document supported tags

  const std::string &tag = node.Tag();
  const std::string &value = node.Scalar();
  if ((tag == "tag:yaml.org,2002:str") || (tag == "!")) {
    cfg.SetString(fqn, value);
  } else if (tag == "tag:yaml.org,2002:bool") {
    cfg.SetBool(fqn, node.as<bool>());
  } else if (tag == "tag:yaml.org,2002:int") {
    cfg.SetInt64(fqn, std::stol(value));
    return;
  } else if (tag == "tag:yaml.org,2002:float") {
    cfg.SetDouble(fqn, std::stod(value));
  } else {
    // if (tag == "tag:yaml.org,2002:date") {
    //   return date{val};
    // return;
    // }

    std::string msg{"YAML tag `" + tag + "` is not supported!"};
    throw std::logic_error{msg};
  }
}

void AppendTaggedScalar(const YAML::Node &node,
    Configuration &cfg,
    std::string_view fqn) {
  // LCOV_EXCL_START
  if (!HasTag(node)) {
    std::string msg{
        "AppendTaggedScalar called with untagged node for parameter `"};
    msg += fqn;
    msg += "`!";
    throw std::logic_error(msg);
  }
  // LCOV_EXCL_STOP

  const std::string &tag = node.Tag();
  const std::string &value = node.Scalar();
  if ((tag == "tag:yaml.org,2002:str") || (tag == "!")) {
    cfg.Append(fqn, value);
  } else if (tag == "tag:yaml.org,2002:bool") {
    cfg.Append(fqn, node.as<bool>());
  } else if (tag == "tag:yaml.org,2002:int") {
    cfg.Append(fqn, std::stol(value));
  } else if (tag == "tag:yaml.org,2002:float") {
    cfg.Append(fqn, std::stod(value));
  } else {
    // if (tag == "tag:yaml.org,2002:date") {
    //   return date{val};
    // return;
    // }

    std::string msg{"YAML tag `" + tag + "` is not supported!"};
    throw std::logic_error{msg};
  }
}

void SetScalar(const YAML::Node &node,
    Configuration &cfg,
    std::string_view fqn) {
  if (HasTag(node)) {
    SetTaggedScalar(node, cfg, fqn);
    return;
  }

  // Node is not tagged, so we try to decode it into the supported types.
  const auto val_bool = DecodeScalarNode<bool>(node);
  if (val_bool.has_value()) {
    cfg.SetBool(fqn, val_bool.value());
    return;
  }

  const auto val_int = DecodeScalarNode<int64_t>(node);
  if (val_int.has_value()) {
    cfg.SetInt64(fqn, val_int.value());
    return;
  }

  const auto val_double = DecodeScalarNode<double>(node);
  if (val_double.has_value()) {
    cfg.SetDouble(fqn, val_double.value());
    return;
  }

  const auto val_date = DecodeScalarNode<date>(node);
  if (val_date.has_value()) {
    cfg.SetDate(fqn, val_date.value());
    return;
  }

  const auto val_time = DecodeScalarNode<time>(node);
  if (val_time.has_value()) {
    cfg.SetTime(fqn, val_time.value());
    return;
  }

  const auto val_datetime = DecodeScalarNode<date_time>(node);
  if (val_datetime.has_value()) {
    cfg.SetDateTime(fqn, val_datetime.value());
    return;
  }

  const auto val_string = DecodeScalarNode<std::string>(node);
  if (val_string.has_value()) {
    cfg.SetString(fqn, val_string.value());
    return;
  }

  // TODO throw
  // LCOV_EXCL_START
  std::string msg{"Could not decode scalar YAML node for parameter `"};
  msg += fqn;
  msg += "`!";
  throw TypeError{msg};
  // LCOV_EXCL_STOP

  // TODO test date types - YAML date examples used a more lenient format; need
  // to check in detail
}

void AppendScalar(const YAML::Node &node,
    Configuration &cfg,
    std::string_view key) {
  const auto val_bool = DecodeScalarNode<bool>(node);
  if (val_bool.has_value()) {
    cfg.Append(key, val_bool.value());
    return;
  }

  const auto val_int = DecodeScalarNode<int64_t>(node);
  if (val_int.has_value()) {
    cfg.Append(key, val_int.value());
    return;
  }

  const auto val_double = DecodeScalarNode<double>(node);
  if (val_double.has_value()) {
    cfg.Append(key, val_double.value());
    return;
  }

  const auto val_date = DecodeScalarNode<date>(node);
  if (val_date.has_value()) {
    cfg.Append(key, val_date.value());
    return;
  }

  const auto val_time = DecodeScalarNode<time>(node);
  if (val_time.has_value()) {
    cfg.Append(key, val_time.value());
    return;
  }

  const auto val_datetime = DecodeScalarNode<date_time>(node);
  if (val_datetime.has_value()) {
    cfg.Append(key, val_datetime.value());
    return;
  }

  const auto val_string = DecodeScalarNode<std::string>(node);
  if (val_string.has_value()) {
    cfg.Append(key, val_string.value());
    return;
  }

  // TODO throw
  // LCOV_EXCL_START
  std::string msg{"Could not decode scalar YAML node for parameter `"};
  msg += key;
  msg += "`!";
  throw TypeError{msg};
  // LCOV_EXCL_STOP

  // TODO test date types - YAML date examples used a more lenient format; need
  // to check in detail
}

// TODO create list
// TODO append list
void AppendListItems(const YAML::Node &node,
    Configuration &cfg,
    std::string_view key) {
  // LCOV_EXCL_START
  if (!node.IsSequence() || !cfg.Contains(key)) {
    // TODO throw
    // std::string msg{"Could not decode list YAML node for parameter `"};
    // msg += key;
    // msg += "`!";
    // throw TypeError{msg};
  }
  // LCOV_EXCL_STOP

  for (const auto &item : node) {
    if (item.IsScalar()) {
      AppendScalar(item, cfg, key);
    } else if (item.IsSequence()) {
      const std::size_t lst_size = cfg.Size(key);
      const std::string elem_key =
          Configuration::KeyForListElement(key, lst_size);
      cfg.AppendList(key);
      AppendListItems(item, cfg, elem_key);
    } else if (item.IsMap()) {
      cfg.Append(key, item.as<Configuration>());
    }
  }
}

// template <typename Tp>
// std::optional<Tp> DecodeNode(const YAML::Node &node) {
//   if (!node.IsScalar()) {
//     return std::nullopt;
//   }

//   // // TODO check if is numeric
//   // const std::string val = node.as<std::string>();
//   // if (werkzeugkiste::strings::IsInteger(val)) {
//   // }
//   // if (werkzeugkiste::strings::IsNumeric(node.as<std::string>());
//   // return node.as<Tp>();
// }
}  // namespace detail

Configuration LoadYAMLString(const std::string &yaml_string,
    NullValuePolicy none_policy) {
  // TODO ryml segfaults on this input:
  // "{[a: b}"
  // Dive deeper into docs, prepare MWE for bug report
  // Older, possibly related issues:
  // https://github.com/biojppm/rapidyaml/issues/34
  // https://github.com/biojppm/rapidyaml/issues/32

  // detail::ErrorHandler err_hnd;
  // ryml::set_callbacks(err_hnd.callbacks());
  // auto cs = ryml::csubstr(yaml_string.c_str(), yaml_string.length());
  // WZKLOG_CRITICAL("INPUT TO PARSER\n{}", cs);
  // ryml::Tree tree = ryml::parse_in_arena(cs);

  // // Restore default error handler
  // ryml::set_callbacks(err_hnd.defaults);

  // return detail::FromlYAMLRoot(tree.rootref());

  try {
    WZKLOG_CRITICAL("INPUT TO PARSER\n{}", yaml_string);
    YAML::Node config = YAML::Load(yaml_string);
    WZKLOG_CRITICAL("ismap {}, is_seq {}", config.IsMap(), config.IsSequence());
    // TODO check if ismap
    auto cfg = config.as<Configuration>();
    WZKLOG_CRITICAL("parsed into config:\n{}", cfg.ToTOML());
    return cfg;

  } catch (const YAML::Exception &e) {
    throw ParseError(e.what());
  }

  return Configuration{};
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
