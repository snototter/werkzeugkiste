#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/files/fileio.h>
#include <werkzeugkiste/logging.h>  // TODO remove
#include <werkzeugkiste/strings/strings.h>

#include <fstream>
#include <string>
#include <string_view>

// NOLINTBEGIN(*-macro-usage, readability-identifier-naming)

// Ensure that nlohmann/json doesn't include conversions from std::filesystem,
// because they will lead to linkage errors in the CI runners.
#define RYML_SINGLE_HDR_DEFINE_NOW
#include <ryml/ryml_all.hpp>
// NOLINTEND(*-macro-usage, readability-identifier-naming)

namespace werkzeugkiste::config {
namespace detail {
// taken from
// https://github.com/biojppm/rapidyaml/blob/master/samples/quickstart.cpp
struct ErrorHandler {
  // this will be called on error
  void on_error(const char *msg, size_t len, ryml::Location loc) {
    throw ParseError(ryml::formatrs<std::string>("{}:{}:{} ({}B): ERROR: {}",
        loc.name,
        loc.line,
        loc.col,
        loc.offset,
        ryml::csubstr(msg, len)));
  }

  // bridge
  ryml::Callbacks callbacks() {
    return ryml::Callbacks(this, nullptr, nullptr, ErrorHandler::s_error);
  }
  static void s_error(const char *msg,
      size_t len,
      ryml::Location loc,
      void *this_) {
    return ((ErrorHandler *)this_)->on_error(msg, len, loc);
  }

  // // checking
  // template<class Fn>
  // void check_error_occurs(Fn &&fn) const
  // {
  //     bool expected_error_occurred = false;
  //     try { fn(); }
  //     catch(std::runtime_error const&) { expected_error_occurred = true; }
  //     CHECK(expected_error_occurred);
  // }
  // void check_effect(bool committed) const
  // {
  //     ryml::Callbacks const& current = ryml::get_callbacks();
  //     if(committed)
  //     {
  //         CHECK(current.m_error == &s_error);
  //     }
  //     else
  //     {
  //         CHECK(current.m_error != &s_error);
  //     }
  //     CHECK(current.m_allocate == defaults.m_allocate);
  //     CHECK(current.m_free == defaults.m_free);
  // }
  // save the default callbacks for checking
  ErrorHandler() : defaults(ryml::get_callbacks()) {}
  ryml::Callbacks defaults;
};
// // Forward declaration
// Configuration FromJSONObject(const json &object, NullValuePolicy
// none_policy);

// /// @brief Appends the JSON `value` to an already created list of the given
// ///   Configuration `cfg`.
// /// @param lst_key Parameter name of the list to be appended to.
// /// @param cfg The configuration which already holds the list.
// /// @param value The JSON value to append to the list.
// /// @param none_policy How to deal with null/none values.
// // NOLINTNEXTLINE(misc-no-recursion)
// void AppendListValue(std::string_view lst_key,
//     Configuration &cfg,
//     const json &value,
//     NullValuePolicy none_policy) {
//   using namespace std::string_view_literals;
//   if (value.is_null()) {
//     switch (none_policy) {
//       case NullValuePolicy::Skip:
//         break;

//       case NullValuePolicy::NullString:
//         cfg.Append(lst_key, "null"sv);
//         break;

//       case NullValuePolicy::EmptyList:
//         cfg.AppendList(lst_key);
//         break;

//       case NullValuePolicy::Fail:
//         std::string msg{"Null/None value occured while parsing JSON list `"};
//         msg += lst_key;
//         msg += "`!";
//         throw ParseError{msg};
//     }
//   } else if (value.is_boolean()) {
//     cfg.Append(lst_key, value.get<bool>());
//   } else if (value.is_number_float()) {
//     cfg.Append(lst_key, value.get<double>());
//   } else if (value.is_number_integer()) {
//     cfg.Append(lst_key, value.get<int64_t>());
//   } else if (value.is_string()) {
//     cfg.Append(lst_key, value.get<std::string>());
//   } else if (value.is_array()) {
//     std::size_t sz = cfg.Size(lst_key);
//     cfg.AppendList(lst_key);
//     std::string elem_key{lst_key};
//     elem_key += '[';
//     elem_key += std::to_string(sz);
//     elem_key += ']';
//     for (const json &element : value) {
//       AppendListValue(elem_key, cfg, element, none_policy);
//     }
//   } else if (value.is_object()) {
//     cfg.Append(lst_key, FromJSONObject(value, none_policy));
//   } else if (value.is_binary()) {
//     // LCOV_EXCL_START
//     std::string msg{"Cannot load the binary JSON value for parameter `"};
//     msg += lst_key;
//     msg += "`!";
//     throw ValueError{msg};
//     // LCOV_EXCL_STOP
//   }
// }

// /// @brief Parses a JSON object (dictionary) into a Configuration.
// /// @param object The JSON object.
// /// @param none_policy How to deal with null/none values.
// // NOLINTNEXTLINE(misc-no-recursion)
// Configuration FromJSONObject(const json &object, NullValuePolicy none_policy)
// {
//   using namespace std::string_view_literals;
//   Configuration grp{};
//   for (json::const_iterator it = object.begin(); it != object.end(); ++it) {
//     const json &value = it.value();
//     const std::string_view key = it.key();
//     if (value.is_null()) {
//       switch (none_policy) {
//         case NullValuePolicy::Skip:
//           break;

//         case NullValuePolicy::NullString:
//           grp.SetString(key, "null"sv);
//           break;

//         case NullValuePolicy::EmptyList:
//           grp.CreateList(key);
//           break;

//         case NullValuePolicy::Fail:
//           std::string msg{
//               "Null/None value occured while parsing JSON parameter `"};
//           msg += key;
//           msg += "`!";
//           throw ParseError{msg};
//       }
//     } else if (value.is_boolean()) {
//       grp.SetBoolean(key, value.get<bool>());
//     } else if (value.is_number_float()) {
//       // https://json.nlohmann.me/api/basic_json/number_float_t/
//       grp.SetDouble(key, value.get<double>());
//     } else if (value.is_number_integer()) {
//       // https://json.nlohmann.me/api/basic_json/number_integer_t/
//       grp.SetInt64(key, value.get<int64_t>());
//     } else if (value.is_string()) {
//       grp.SetString(key, value.get<std::string>());
//     } else if (value.is_array()) {
//       grp.CreateList(key);
//       for (const json &element : value) {
//         AppendListValue(key, grp, element, none_policy);
//       }
//     } else if (value.is_object()) {
//       grp.SetGroup(key, FromJSONObject(value, none_policy));
//     } else if (value.is_binary()) {
//       // LCOV_EXCL_START
//       std::string msg{"Cannot load the binary JSON value for parameter `"};
//       msg += key;
//       msg += "`!";
//       throw ValueError{msg};
//       // LCOV_EXCL_STOP
//     }
//   }
//   return grp;
// }

// Configuration FromJSONRoot(const json &object, NullValuePolicy none_policy) {
//   if (object.is_object()) {
//     return FromJSONObject(object, none_policy);
//   }
//   if (object.is_array()) {
//     Configuration cfg{};
//     const std::string_view key{"json"};
//     cfg.CreateList(key);
//     for (const json &element : object) {
//       AppendListValue(key, cfg, element, none_policy);
//     }
//     return cfg;
//   }

//   // LCOV_EXCL_START
//   // This branch should be unreachable.
//   throw std::logic_error{
//       "Internal util `FromJSONRoot` invoked with neither JSON object nor "
//       "array! Please "
//       "report at https://github.com/snototter/werkzeugkiste/issues"};
//   // LCOV_EXCL_STOP
// }

void Test(ryml::ConstNodeRef node, Configuration &cfg, std::string_view fqn) {
  // TODO is_keyval
  std::string fqn_child{fqn};
  fqn_child += "    ";
  if (node.is_map()) {
    for (const ryml::ConstNodeRef &child : node.children()) {
      if (child.has_key()) {
        WZKLOG_INFO("{}map child: key {}", fqn, child.key());
      } else {
        if (child.has_val()) {
          WZKLOG_INFO("{}seq child - no key, val {}", fqn, child.val());
        } else {
          WZKLOG_INFO("{}seq child - no key, no val", fqn);
        }
      }
      Test(child, cfg, fqn_child);  // TODO fqn
      // std::string_view key = child.key();
      // std::string_view new_fqn = fqn;
      // if (!new_fqn.empty()) {
      //   new_fqn += '.';
      // }
      // new_fqn += key;
      // Test(child, cfg, new_fqn);
    }
  } else if (node.is_seq()) {
    for (const ryml::ConstNodeRef &child : node.children()) {
      // Test(child, cfg, fqn);
      if (child.has_key()) {
        WZKLOG_INFO("{}seq child: key {}", fqn, child.key());
      } else {
        if (child.has_val()) {
          WZKLOG_INFO("{}seq child - no key, val {}", fqn, child.val());
        } else {
          WZKLOG_INFO("{}seq child - no key, no val", fqn);
        }
      }
      Test(child, cfg, fqn_child);  // TODO fqn
    }
  } else if (node.is_val()) {
    WZKLOG_INFO("{}node is value: {}", fqn, node.val());
    // std::string_view value = node.val();
    // if (value == "true"sv) {
    //   cfg.SetBoolean(fqn, true);
    // } else if (value == "false"sv) {
    //   cfg.SetBoolean(fqn, false);
    // } else if (value == "null"sv) {
    //   cfg.SetNull(fqn);
    // } else {
    //   try {
    //     cfg.SetInt64(fqn, std::stoll(std::string{value}));
    //   } catch (const std::invalid_argument &) {
    //     try {
    //       cfg.SetDouble(fqn, std::stod(std::string{value}));
    //     } catch (const std::invalid_argument &) {
    //       cfg.SetString(fqn, std::string{value});
    //     }
    //   }
    // }
  }
}

Configuration FromlYAMLRoot(ryml::ConstNodeRef root) {
  WZKLOG_CRITICAL("from root, is_map: {}", root.is_map());
  Configuration cfg{};
  using namespace std::string_view_literals;
  if (root.is_seq()) {
    WZKLOG_CRITICAL("root is seq with {} children", root.num_children());
    for (const ryml::ConstNodeRef &child : root.children()) {
      WZKLOG_CRITICAL("seq child");
      Test(child, cfg, ""sv);
    }
  } else if (root.is_map()) {
    WZKLOG_CRITICAL("root is map");
    for (const ryml::ConstNodeRef &child : root.children()) {
      WZKLOG_CRITICAL("map child");
      Test(child, cfg, ""sv);
    }
  }
  // Test(root, cfg, ""sv);
  return cfg;
}
}  // namespace detail

Configuration LoadYAMLString(const std::string &yaml_string,
    NullValuePolicy none_policy) {
  // TODO ryml segfault:
  // "{[a: b}"
  // Dive deeper into docs, prepare MWE for bug report
  // Older, possibly related issues:
  // https://github.com/biojppm/rapidyaml/issues/34
  // https://github.com/biojppm/rapidyaml/issues/32

  detail::ErrorHandler err_hnd;
  ryml::set_callbacks(err_hnd.callbacks());
  auto cs = ryml::csubstr(yaml_string.c_str(), yaml_string.length());
  WZKLOG_CRITICAL("INPUT TO PARSER\n{}", cs);
  ryml::Tree tree = ryml::parse_in_arena(cs);

  // Restore default error handler
  ryml::set_callbacks(err_hnd.defaults);

  return detail::FromlYAMLRoot(tree.rootref());
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
