#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/files/fileio.h>
#include <werkzeugkiste/logging.h>  // TODO remove
#include <werkzeugkiste/strings/strings.h>

#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
using json = nlohmann::json;

namespace werkzeugkiste::config {
namespace detail {
Configuration FromJSON(const json &object, JSONNoneHandling none_handling) {
  if (!object.is_object()) {
    // This branch should be unreachable.
    // ThrowImplementationError(
    //     "Internal util `FromLibconfigGroup` invoked with non-group node!",
    //     node.getName());
  }
  Configuration grp{};
  for (json::const_iterator it = object.begin(); it != object.end(); ++it) {
    WZKLOG_CRITICAL(
        "TODO TODO key: {} is_string {}", it.key(), it.value().is_string());
    const json &value = it.value();
    // value.is_number_integer()
    // value.get<bool>()
  }
  return grp;
}
// void ConstructConfigParam(const json &elem, ConfigParams &params, const
// std::string &root)
// {
//   std::string prefix = root.empty() ? "" : root + ".";

//   for (json::const_iterator it = elem.begin(); it != elem.end(); ++it)
//   {
//     const std::string param_name = prefix + it.key();
//     const auto &v = it.value();
//     if (v.is_boolean())
//     {
//       params.SetBoolean(param_name, v.get<bool>());
//     }
//     else if (v.is_number())
//     {
//       if (IsInteger(v.get<double>()))
//         params.SetInteger(param_name, v.get<int>());
//       else
//         params.SetDouble(param_name, v.get<double>());
//     }
//     else if (v.is_array())
//     {
//       if (IsIntegerArray(v))
//       {
//         std::vector<int> values;
//         for (auto& element : v)
//           values.push_back(element);
//         params.SetIntegerArray(param_name, values);
//       }
//       else if (IsDoubleArray(v))
//       {
//         std::vector<double> values;
//         for (auto& element : v)
//           values.push_back(element);
//         params.SetDoubleArray(param_name, values);
//       }
//       else if (IsStringArray(v))
//       {
//         std::vector<std::string> values;
//         for (auto& element : v)
//           values.push_back(element);
//         params.SetStringArray(param_name, values);
//       }
//       else if (IsTypedKeyValueList<IsString, IsInt>(v))
//       {
//         std::vector<std::pair<std::string, int>> values;
//         for (auto& element : v)
//           values.push_back(std::make_pair<std::string, int>(element[0],
//           element[1]));
//         params.SetIntegerKeyValueList(param_name, values);
//       }
//       else if (IsTypedKeyValueList<IsString, IsDouble>(v))
//       {
//         std::vector<std::pair<std::string, double>> values;
//         for (auto& element : v)
//           values.push_back(std::make_pair<std::string, double>(element[0],
//           element[1]));
//         params.SetDoubleKeyValueList(param_name, values);
//       }
//       else
//       {
//         VCP_ERROR("Only int/double/string arrays are currently supported");
//       }
//     }
//     else if (v.is_string())
//     {
//       params.SetString(param_name, v.get<std::string>());
//     }
//     else if (v.is_object())
//     {
//       ConstructConfigParam(v, params, prefix + it.key());
//     }
//     else
//     {
//       VCP_ERROR("Type of " + root + "." + it.key() + " is not supported");
//     }
//   }
// }
}  // namespace detail

Configuration LoadJSONString(std::string_view json_string,
    JSONNoneHandling none_handling) {
  try {
    return detail::FromJSON(json::parse(json_string), none_handling);
  } catch (const json::parse_error &e) {
    std::string msg{"Parsing JSON input failed: "};
    msg += e.what();
    throw ParseError{msg};
  }
}

Configuration LoadJSONFile(std::string_view filename,
    JSONNoneHandling none_handling) {
  try {
    return LoadJSONString(files::CatAsciiFile(filename), none_handling);
  } catch (const werkzeugkiste::files::IOError &e) {
    throw ParseError(e.what());
  }
}
}  // namespace werkzeugkiste::config
