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

/// @brief Appends the JSON `value` to an already created list of the given
///   Configuration `cfg`.
/// @param lst_key Parameter name of the list to be appended to.
/// @param cfg The configuration which already holds the list.
/// @param value The JSON value to append to the list.
/// @param none_policy How to deal with null/none values.
// NOLINTNEXTLINE(misc-no-recursion)
void AppendListValue(std::string_view lst_key,
    Configuration &cfg,
    const json &value,
    NullValuePolicy none_policy) {
  using namespace std::string_view_literals;
  if (value.is_null()) {
    switch (none_policy) {
      case NullValuePolicy::Skip:
        break;

      case NullValuePolicy::NullString:
        cfg.Append(lst_key, "null"sv);
        break;

      case NullValuePolicy::EmptyList:
        cfg.AppendList(lst_key);
        break;

      case NullValuePolicy::Fail:
        std::string msg{"Null/None value occured while parsing JSON list `"};
        msg += lst_key;
        msg += "`!";
        throw ParseError{msg};
    }
  } else if (value.is_boolean()) {
    cfg.Append(lst_key, value.get<bool>());
  } else if (value.is_number_float()) {
    cfg.Append(lst_key, value.get<double>());
  } else if (value.is_number_integer()) {
    cfg.Append(lst_key, value.get<int64_t>());
  } else if (value.is_string()) {
    cfg.Append(lst_key, value.get<std::string>());
  } else if (value.is_array()) {
    std::size_t sz = cfg.Size(lst_key);
    cfg.AppendList(lst_key);
    std::string elem_key{lst_key};
    elem_key += '[';
    elem_key += std::to_string(sz);
    elem_key += ']';
    for (const json &element : value) {
      AppendListValue(elem_key, cfg, element, none_policy);
    }
  } else if (value.is_object()) {
    cfg.Append(lst_key, FromJSONObject(value, none_policy));
  } else if (value.is_binary()) {
    // LCOV_EXCL_START
    std::string msg{"Cannot load the binary JSON value for parameter `"};
    msg += lst_key;
    msg += "`!";
    throw ValueError{msg};
    // LCOV_EXCL_STOP
  }
}

/// @brief Parses a JSON object (dictionary) into a Configuration.
/// @param object The JSON object.
/// @param none_policy How to deal with null/none values.
// NOLINTNEXTLINE(misc-no-recursion)
Configuration FromJSONObject(const json &object, NullValuePolicy none_policy) {
  using namespace std::string_view_literals;
  Configuration grp{};
  for (json::const_iterator it = object.begin(); it != object.end(); ++it) {
    const json &value = it.value();
    const std::string_view key = it.key();
    if (value.is_null()) {
      switch (none_policy) {
        case NullValuePolicy::Skip:
          break;

        case NullValuePolicy::NullString:
          grp.SetString(key, "null"sv);
          break;

        case NullValuePolicy::EmptyList:
          grp.CreateList(key);
          break;

        case NullValuePolicy::Fail:
          std::string msg{
              "Null/None value occured while parsing JSON parameter `"};
          msg += key;
          msg += "`!";
          throw ParseError{msg};
      }
    } else if (value.is_boolean()) {
      grp.SetBool(key, value.get<bool>());
    } else if (value.is_number_float()) {
      // https://json.nlohmann.me/api/basic_json/number_float_t/
      grp.SetDouble(key, value.get<double>());
    } else if (value.is_number_integer()) {
      // https://json.nlohmann.me/api/basic_json/number_integer_t/
      grp.SetInt64(key, value.get<int64_t>());
    } else if (value.is_string()) {
      grp.SetString(key, value.get<std::string>());
    } else if (value.is_array()) {
      grp.CreateList(key);
      for (const json &element : value) {
        AppendListValue(key, grp, element, none_policy);
      }
    } else if (value.is_object()) {
      grp.SetGroup(key, FromJSONObject(value, none_policy));
    } else if (value.is_binary()) {
      // LCOV_EXCL_START
      std::string msg{"Cannot load the binary JSON value for parameter `"};
      msg += key;
      msg += "`!";
      throw ValueError{msg};
      // LCOV_EXCL_STOP
    }
  }
  return grp;
}

Configuration FromJSONRoot(const json &object, NullValuePolicy none_policy) {
  if (object.is_object()) {
    return FromJSONObject(object, none_policy);
  }
  if (object.is_array()) {
    Configuration cfg{};
    const std::string_view key{"json"};
    cfg.CreateList(key);
    for (const json &element : object) {
      AppendListValue(key, cfg, element, none_policy);
    }
    return cfg;
  }

  // LCOV_EXCL_START
  // This branch should be unreachable.
  throw std::logic_error{
      "Internal util `FromJSONRoot` invoked with neither JSON object nor "
      "array! Please "
      "report at https://github.com/snototter/werkzeugkiste/issues"};
  // LCOV_EXCL_STOP
}
}  // namespace detail

Configuration LoadJSONString(std::string_view json_string,
    NullValuePolicy none_policy) {
  try {
    return detail::FromJSONRoot(json::parse(json_string), none_policy);
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
