#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/files/fileio.h>
#include <werkzeugkiste/logging.h>  // TODO remove
#include <werkzeugkiste/strings/strings.h>
#include <yaml-cpp/yaml.h>

#include <fstream>
#include <string>
#include <string_view>

// #define RYML_SINGLE_HDR_DEFINE_NOW
// #include <ryml/ryml_all.hpp>
// NOLINTEND(*-macro-usage, readability-identifier-naming)

// Forward declarations
namespace werkzeugkiste::config::detail {
void SetScalar(const YAML::Node &node,
    Configuration &cfg,
    std::string_view key);
void AppendListItems(const YAML::Node &node,
    Configuration &cfg,
    std::string_view key);
}  // namespace werkzeugkiste::config::detail

namespace YAML {
template <>
struct convert<werkzeugkiste::config::Configuration> {
  // static Node encode(const werkzeugkiste::config::Configuration& rhs) {
  //   Node node;
  //   // node.push_back(rhs.x);
  //   // node.push_back(rhs.y);
  //   // node.push_back(rhs.z);//FIXME
  //   return node;
  // }

  static bool decode(const Node &node,
      werkzeugkiste::config::Configuration &rhs) {
    if (!node.IsMap()) {
      return false;
    }

    for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
      const std::string key = it->first.as<std::string>();
      WZKLOG_CRITICAL("decoding config, key: {}", key);
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

namespace werkzeugkiste::config {
namespace detail {
inline bool HasTag(const YAML::Node &node) {
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
    return;
  }

  if (tag == "tag:yaml.org,2002:bool") {
    cfg.SetBool(fqn, node.as<bool>());
  }

  if (tag == "tag:yaml.org,2002:int") {
    cfg.SetInt64(fqn, std::stol(value));
  }

  if (tag == "tag:yaml.org,2002:float") {
    cfg.SetDouble(fqn, std::stod(value));
  }

  // if (tag == "tag:yaml.org,2002:date") {
  //   return date{val};
  // }

  std::string msg{"YAML tag `" + tag + "` is not supported!"};
  throw std::logic_error{msg};
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
// namespace detail {
// // taken from
// // https://github.com/biojppm/rapidyaml/blob/master/samples/quickstart.cpp
// struct ErrorHandler {
//   // this will be called on error
//   void on_error(const char *msg, size_t len, ryml::Location loc) {
//     throw ParseError(ryml::formatrs<std::string>("{}:{}:{} ({}B): ERROR: {}",
//         loc.name,
//         loc.line,
//         loc.col,
//         loc.offset,
//         ryml::csubstr(msg, len)));
//   }

//   // bridge
//   ryml::Callbacks callbacks() {
//     return ryml::Callbacks(this, nullptr, nullptr, ErrorHandler::s_error);
//   }
//   static void s_error(const char *msg,
//       size_t len,
//       ryml::Location loc,
//       void *this_) {
//     return ((ErrorHandler *)this_)->on_error(msg, len, loc);
//   }

//   // // checking
//   // template<class Fn>
//   // void check_error_occurs(Fn &&fn) const
//   // {
//   //     bool expected_error_occurred = false;
//   //     try { fn(); }
//   //     catch(std::runtime_error const&) { expected_error_occurred = true; }
//   //     CHECK(expected_error_occurred);
//   // }
//   // void check_effect(bool committed) const
//   // {
//   //     ryml::Callbacks const& current = ryml::get_callbacks();
//   //     if(committed)
//   //     {
//   //         CHECK(current.m_error == &s_error);
//   //     }
//   //     else
//   //     {
//   //         CHECK(current.m_error != &s_error);
//   //     }
//   //     CHECK(current.m_allocate == defaults.m_allocate);
//   //     CHECK(current.m_free == defaults.m_free);
//   // }
//   // save the default callbacks for checking
//   ErrorHandler() : defaults(ryml::get_callbacks()) {}
//   ryml::Callbacks defaults;
// };
// // // Forward declaration
// // Configuration FromJSONObject(const json &object, NullValuePolicy
// // none_policy);

// // /// @brief Appends the JSON `value` to an already created list of the
// given
// // ///   Configuration `cfg`.
// // /// @param lst_key Parameter name of the list to be appended to.
// // /// @param cfg The configuration which already holds the list.
// // /// @param value The JSON value to append to the list.
// // /// @param none_policy How to deal with null/none values.
// // // NOLINTNEXTLINE(misc-no-recursion)
// // void AppendListValue(std::string_view lst_key,
// //     Configuration &cfg,
// //     const json &value,
// //     NullValuePolicy none_policy) {
// //   using namespace std::string_view_literals;
// //   if (value.is_null()) {
// //     switch (none_policy) {
// //       case NullValuePolicy::Skip:
// //         break;

// //       case NullValuePolicy::NullString:
// //         cfg.Append(lst_key, "null"sv);
// //         break;

// //       case NullValuePolicy::EmptyList:
// //         cfg.AppendList(lst_key);
// //         break;

// //       case NullValuePolicy::Fail:
// //         std::string msg{"Null/None value occured while parsing JSON list
// `"};
// //         msg += lst_key;
// //         msg += "`!";
// //         throw ParseError{msg};
// //     }
// //   } else if (value.is_boolean()) {
// //     cfg.Append(lst_key, value.get<bool>());
// //   } else if (value.is_number_float()) {
// //     cfg.Append(lst_key, value.get<double>());
// //   } else if (value.is_number_integer()) {
// //     cfg.Append(lst_key, value.get<int64_t>());
// //   } else if (value.is_string()) {
// //     cfg.Append(lst_key, value.get<std::string>());
// //   } else if (value.is_array()) {
// //     std::size_t sz = cfg.Size(lst_key);
// //     cfg.AppendList(lst_key);
// //     std::string elem_key{lst_key};
// //     elem_key += '[';
// //     elem_key += std::to_string(sz);
// //     elem_key += ']';
// //     for (const json &element : value) {
// //       AppendListValue(elem_key, cfg, element, none_policy);
// //     }
// //   } else if (value.is_object()) {
// //     cfg.Append(lst_key, FromJSONObject(value, none_policy));
// //   } else if (value.is_binary()) {
// //     // LCOV_EXCL_START
// //     std::string msg{"Cannot load the binary JSON value for parameter `"};
// //     msg += lst_key;
// //     msg += "`!";
// //     throw ValueError{msg};
// //     // LCOV_EXCL_STOP
// //   }
// // }

// // /// @brief Parses a JSON object (dictionary) into a Configuration.
// // /// @param object The JSON object.
// // /// @param none_policy How to deal with null/none values.
// // // NOLINTNEXTLINE(misc-no-recursion)
// // Configuration FromJSONObject(const json &object, NullValuePolicy
// none_policy)
// // {
// //   using namespace std::string_view_literals;
// //   Configuration grp{};
// //   for (json::const_iterator it = object.begin(); it != object.end(); ++it)
// {
// //     const json &value = it.value();
// //     const std::string_view key = it.key();
// //     if (value.is_null()) {
// //       switch (none_policy) {
// //         case NullValuePolicy::Skip:
// //           break;

// //         case NullValuePolicy::NullString:
// //           grp.SetString(key, "null"sv);
// //           break;

// //         case NullValuePolicy::EmptyList:
// //           grp.CreateList(key);
// //           break;

// //         case NullValuePolicy::Fail:
// //           std::string msg{
// //               "Null/None value occured while parsing JSON parameter `"};
// //           msg += key;
// //           msg += "`!";
// //           throw ParseError{msg};
// //       }
// //     } else if (value.is_boolean()) {
// //       grp.SetBoolean(key, value.get<bool>());
// //     } else if (value.is_number_float()) {
// //       // https://json.nlohmann.me/api/basic_json/number_float_t/
// //       grp.SetDouble(key, value.get<double>());
// //     } else if (value.is_number_integer()) {
// //       // https://json.nlohmann.me/api/basic_json/number_integer_t/
// //       grp.SetInt64(key, value.get<int64_t>());
// //     } else if (value.is_string()) {
// //       grp.SetString(key, value.get<std::string>());
// //     } else if (value.is_array()) {
// //       grp.CreateList(key);
// //       for (const json &element : value) {
// //         AppendListValue(key, grp, element, none_policy);
// //       }
// //     } else if (value.is_object()) {
// //       grp.SetGroup(key, FromJSONObject(value, none_policy));
// //     } else if (value.is_binary()) {
// //       // LCOV_EXCL_START
// //       std::string msg{"Cannot load the binary JSON value for parameter
// `"};
// //       msg += key;
// //       msg += "`!";
// //       throw ValueError{msg};
// //       // LCOV_EXCL_STOP
// //     }
// //   }
// //   return grp;
// // }

// // Configuration FromJSONRoot(const json &object, NullValuePolicy
// none_policy) {
// //   if (object.is_object()) {
// //     return FromJSONObject(object, none_policy);
// //   }
// //   if (object.is_array()) {
// //     Configuration cfg{};
// //     const std::string_view key{"json"};
// //     cfg.CreateList(key);
// //     for (const json &element : object) {
// //       AppendListValue(key, cfg, element, none_policy);
// //     }
// //     return cfg;
// //   }

// //   // LCOV_EXCL_START
// //   // This branch should be unreachable.
// //   throw std::logic_error{
// //       "Internal util `FromJSONRoot` invoked with neither JSON object nor "
// //       "array! Please "
// //       "report at https://github.com/snototter/werkzeugkiste/issues"};
// //   // LCOV_EXCL_STOP
// // }

// void Test(ryml::ConstNodeRef node, Configuration &cfg, std::string_view fqn)
// {
//   // TODO is_keyval
//   std::string fqn_child{fqn};
//   fqn_child += "    ";
//   if (node.is_map()) {
//     for (const ryml::ConstNodeRef &child : node.children()) {
//       if (child.has_key()) {
//         WZKLOG_INFO("{}map child: key {}", fqn, child.key());
//       } else {
//         if (child.has_val()) {
//           WZKLOG_INFO("{}seq child - no key, val {}", fqn, child.val());
//         } else {
//           WZKLOG_INFO("{}seq child - no key, no val", fqn);
//         }
//       }
//       Test(child, cfg, fqn_child);  // TODO fqn
//       // std::string_view key = child.key();
//       // std::string_view new_fqn = fqn;
//       // if (!new_fqn.empty()) {
//       //   new_fqn += '.';
//       // }
//       // new_fqn += key;
//       // Test(child, cfg, new_fqn);
//     }
//   } else if (node.is_seq()) {
//     for (const ryml::ConstNodeRef &child : node.children()) {
//       // Test(child, cfg, fqn);
//       if (child.has_key()) {
//         WZKLOG_INFO("{}seq child: key {}", fqn, child.key());
//       } else {
//         if (child.has_val()) {
//           WZKLOG_INFO("{}seq child - no key, val {}", fqn, child.val());
//         } else {
//           WZKLOG_INFO("{}seq child - no key, no val", fqn);
//         }
//       }
//       Test(child, cfg, fqn_child);  // TODO fqn
//     }
//   } else if (node.is_val()) {
//     WZKLOG_INFO("{}node is value: {}", fqn, node.val());
//     // std::string_view value = node.val();
//     // if (value == "true"sv) {
//     //   cfg.SetBoolean(fqn, true);
//     // } else if (value == "false"sv) {
//     //   cfg.SetBoolean(fqn, false);
//     // } else if (value == "null"sv) {
//     //   cfg.SetNull(fqn);
//     // } else {
//     //   try {
//     //     cfg.SetInt64(fqn, std::stoll(std::string{value}));
//     //   } catch (const std::invalid_argument &) {
//     //     try {
//     //       cfg.SetDouble(fqn, std::stod(std::string{value}));
//     //     } catch (const std::invalid_argument &) {
//     //       cfg.SetString(fqn, std::string{value});
//     //     }
//     //   }
//     // }
//   }
// }

// Configuration FromlYAMLRoot(ryml::ConstNodeRef root) {
//   WZKLOG_CRITICAL("from root, is_map: {}", root.is_map());
//   Configuration cfg{};
//   using namespace std::string_view_literals;
//   if (root.is_seq()) {
//     WZKLOG_CRITICAL("root is seq with {} children", root.num_children());
//     for (const ryml::ConstNodeRef &child : root.children()) {
//       WZKLOG_CRITICAL("seq child");
//       Test(child, cfg, ""sv);
//     }
//   } else if (root.is_map()) {
//     WZKLOG_CRITICAL("root is map");
//     for (const ryml::ConstNodeRef &child : root.children()) {
//       WZKLOG_CRITICAL("map child");
//       Test(child, cfg, ""sv);
//     }
//   }
//   // Test(root, cfg, ""sv);
//   return cfg;
// }

// }  // namespace detail

Configuration LoadYAMLString(const std::string &yaml_string,
    NullValuePolicy none_policy) {
  // TODO ryml segfault:
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
