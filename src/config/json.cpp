#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/files/fileio.h>
#include <werkzeugkiste/strings/strings.h>

#include <fstream>
#include <string>
#include <string_view>

// NOLINTBEGIN(*-macro-usage, readability-identifier-naming)

// Ensure that nlohmann/json doesn't include conversions from std::filesystem,
// because they will lead to linkage errors in the CI runners.
#define JSON_HAS_FILESYSTEM 0
#define JSON_HAS_EXPERIMENTAL_FILESYSTEM 0
#include <nlohmann/json.hpp>
using json = nlohmann::json;
// NOLINTEND(*-macro-usage, readability-identifier-naming)

namespace werkzeugkiste::config {
namespace detail {
// Forward declaration
Configuration FromJSONObject(const json &object, NullValuePolicy none_policy);

void AppendListItems(const json &list,
    Configuration &cfg,
    std::string_view fqn,
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

/// @brief Appends or sets a configuration value from a scalar json value.
/// @tparam Tp Type of the scalar value.
/// @param object The JSON value.
/// @param cfg Configuration to be modified.
/// @param fqn Fully qualified parameter name.
/// @param append If true, fqn is assumed to be a list and the value will be
///   appended. Otherwise, the value will be set, i.e. "cfg[fqn] = value";
template <typename Tp>
void HandleBuiltinScalar(const json &object,
    Configuration &cfg,
    std::string_view fqn,
    bool append) {
  if (append) {
    cfg.Append(fqn, object.get<Tp>());
  } else {
    cfg.Set(fqn, object.get<Tp>());
  }
}

/// @brief Appends or sets a configuration value from a parsed json value.
/// @param value Value to be added to the configuration.
/// @param cfg Configuration to be modified.
/// @param fqn Fully qualified parameter name.
/// @param append If true, fqn is assumed to be a list and the value will be
///   appended. Otherwise, the value will be set, i.e. "cfg[fqn] = value";
// NOLINTNEXTLINE(misc-no-recursion)
void HandleValue(const json &value,
    Configuration &cfg,
    std::string_view fqn,
    NullValuePolicy none_policy,
    bool append) {
  switch (value.type()) {
    case json::value_t::null:
      Configuration::HandleNullValue(cfg, fqn, none_policy, append);
      break;

    case json::value_t::boolean:
      HandleBuiltinScalar<bool>(value, cfg, fqn, append);
      break;

    case json::value_t::number_integer:
    case json::value_t::number_unsigned:
      // https://json.nlohmann.me/api/basic_json/number_integer_t/
      HandleBuiltinScalar<int64_t>(value, cfg, fqn, append);
      break;

    case json::value_t::number_float:
      // https://json.nlohmann.me/api/basic_json/number_float_t/
      HandleBuiltinScalar<double>(value, cfg, fqn, append);
      break;

    case json::value_t::string:
      HandleBuiltinScalar<std::string>(value, cfg, fqn, append);
      break;

    case json::value_t::array: {
      if (append) {
        const std::size_t lst_sz = cfg.Size(fqn);
        const std::string elem_key =
            Configuration::KeyForListElement(fqn, lst_sz);
        cfg.AppendList(fqn);
        AppendListItems(value, cfg, elem_key, none_policy);
      } else {
        cfg.CreateList(fqn);
        AppendListItems(value, cfg, fqn, none_policy);
      }
      break;
    }

    case json::value_t::object: {
      if (append) {
        cfg.Append(fqn, FromJSONObject(value, none_policy));
      } else {
        cfg.Set(fqn, FromJSONObject(value, none_policy));
      }
      break;
    }

    // LCOV_EXCL_START
    case json::value_t::binary: {
      std::string msg{
          "Binary JSON values are not supported, check parameter `"};
      msg += fqn;
      msg += "`!";
      throw ValueError{msg};
    }

    case json::value_t::discarded:
      break;
      // LCOV_EXCL_STOP
  }
}

/// @brief Appends all child nodes of the given JSON list to the configuration.
/// @param list The JSON list.
/// @param cfg Configuration to be modified.
/// @param fqn Fully qualified parameter name. This must be a list already
///   existing in the configuration.
/// @param none_policy How to deal with null/none values.
// NOLINTNEXTLINE(misc-no-recursion)
void AppendListItems(const json &list,
    Configuration &cfg,
    std::string_view fqn,
    NullValuePolicy none_policy) {
  // LCOV_EXCL_START
  if (!list.is_array()) {
    ThrowImplementationError(
        "Internal JSON util `AppendListItems` called with non-list/array node",
        fqn);
  }
  if (!cfg.Contains(fqn)) {
    ThrowImplementationError(
        "Internal JSON util `AppendListItems` requires that list already "
        "exists",
        fqn);
  }
  // LCOV_EXCL_STOP

  for (const json &elem : list) {
    HandleValue(elem, cfg, fqn, none_policy, /*append=*/true);
  }
}

/// @brief Parses a JSON object (dictionary) into a Configuration.
/// @param object The JSON object.
/// @param none_policy How to deal with null/none values.
// NOLINTNEXTLINE(misc-no-recursion)
Configuration FromJSONObject(const json &object, NullValuePolicy none_policy) {
  Configuration grp{};
  for (json::const_iterator it = object.begin(); it != object.end(); ++it) {
    HandleValue(it.value(), grp, it.key(), none_policy, /*append=*/false);
  }
  return grp;
}

}  // namespace detail

Configuration LoadJSONString(std::string_view json_string,
    NullValuePolicy none_policy) {
  try {
    const json &object = json::parse(json_string);
    if (object.is_object()) {
      return detail::FromJSONObject(object, none_policy);
    }

    if (object.is_array()) {
      Configuration cfg{};
      const std::string_view key{"list"};
      cfg.CreateList(key);
      detail::AppendListItems(object, cfg, key, none_policy);
      return cfg;
    }

    // LCOV_EXCL_START
    // This branch should be unreachable.
    throw std::logic_error{
        "Internal util `FromJSONRoot` invoked with neither JSON object nor "
        "array! Please "
        "report at https://github.com/snototter/werkzeugkiste/issues"};
    // LCOV_EXCL_STOP
  } catch (const json::parse_error &e) {
    std::string msg{"Parsing JSON input failed! "};
    msg += e.what();
    throw ParseError{msg};
  }
}

Configuration LoadJSONFile(std::string_view filename,
    NullValuePolicy none_policy) {
  try {
    return LoadJSONString(files::CatAsciiFile(filename), none_policy);
  } catch (const werkzeugkiste::files::IOError &e) {
    throw ParseError(e.what());
  }
}
}  // namespace werkzeugkiste::config
