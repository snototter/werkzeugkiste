#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/files/fileio.h>
#include <werkzeugkiste/strings/strings.h>

#include <cstdint>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string_view>
#include <type_traits>

#ifdef WERKZEUGKISTE_WITH_LIBCONFIG
#if __has_include(<libconfig.hh>)
#include <libconfig.hh>
#elif __has_include(<libconfig.h++>)
#include <libconfig.h++>
#else
#error \
    "werkzeugkiste_WITH_LIBCONFIG was enabled, but libconfig++ header file cannot be found!"
#endif  // has_include
#endif  // WERKZEUGKISTE_WITH_LIBCONFIG

namespace werkzeugkiste::config {
#ifdef WERKZEUGKISTE_WITH_LIBCONFIG
namespace detail::parsing {
// Forward declarations
Configuration FromLibconfigGroup(const libconfig::Setting &node);

// LCOV_EXCL_START

/// @brief Throws a std::logic_error indicating an implementation error, i.e.
///   that we miss-used the libconfig++ API.
/// @param description Error description.
/// @param node_name Name of the libconfig::Setting / node in the configuration
///   file at which this error occured. Can be NULL/nullptr.
void ThrowImplementationError(std::string_view description,
    const char *node_name) {
  std::string msg{description};
  if (node_name != nullptr) {
    msg += " Node name was `";
    msg += node_name;
    msg += ".";
  }
  msg +=
      " Please report at "
      "https://github.com/snototter/werkzeugkiste/issues";
  throw std::logic_error{msg};
}
// LCOV_EXCL_STOP

/// @brief Casts a libconfig value to our corresponding data type.
/// @tparam Tcfg Data type used in werkzeugkiste.
/// @tparam Tlibcfg Data type used in libconfig (see their operator overlaods).
/// @param value Value to be converted.
template <typename Tcfg, typename Tlibcfg = Tcfg>
Tcfg CastSetting(const libconfig::Setting &value) {
  try {
    if constexpr (std::is_same_v<Tcfg, Tlibcfg>) {
      return static_cast<Tcfg>(value);
    } else {
      const Tlibcfg val = static_cast<Tlibcfg>(value);
      return static_cast<Tcfg>(val);
    }
  } catch (const libconfig::SettingTypeException &e) {
    // LCOV_EXCL_START
    // This branch should be unreachable.
    std::string msg{"Cannot convert libconfig setting `"};
    msg += value.getName();
    msg += "` to type `";
    msg += TypeName<Tlibcfg>();
    msg += "`: ";
    msg += e.what();
    throw TypeError{msg};
    // LCOV_EXCL_STOP
  }
}

/// @brief Appends the libconfig value to a werkzeugkiste list.
/// @param lst_key Fully qualified parameter name of the list.
/// @param cfg werkzeugkiste configuration.
/// @param node Node/Setting holding the libconfig value.
// NOLINTNEXTLINE(misc-no-recursion, google-runtime-references)
void AppendListValue(std::string_view lst_key,
    Configuration &cfg,
    const libconfig::Setting &node) {
  switch (node.getType()) {
    case libconfig::Setting::TypeInt:
      // NOLINTNEXTLINE(google-runtime-int)
      cfg.Append(lst_key, CastSetting<int64_t, int>(node));
      break;

    case libconfig::Setting::TypeInt64:
      // NOLINTNEXTLINE(google-runtime-int)
      cfg.Append(lst_key, CastSetting<int64_t, long long>(node));
      break;

    case libconfig::Setting::TypeFloat:
      cfg.Append(lst_key, CastSetting<double>(node));
      break;

    case libconfig::Setting::TypeString:
      cfg.Append(lst_key, CastSetting<std::string>(node));
      break;

    case libconfig::Setting::TypeBoolean:
      cfg.Append(lst_key, CastSetting<bool>(node));
      break;

    case libconfig::Setting::TypeGroup:
      cfg.Append(lst_key, FromLibconfigGroup(node));
      break;

    case libconfig::Setting::TypeArray:
    case libconfig::Setting::TypeList: {
      std::size_t sz = cfg.Size(lst_key);
      cfg.AppendList(lst_key);
      std::string elem_key{lst_key};
      elem_key += '[';
      elem_key += std::to_string(sz);
      elem_key += ']';

      for (int i = 0; i < node.getLength(); ++i) {
        AppendListValue(elem_key, cfg, node[i]);
      }
      break;
    }

    case libconfig::Setting::TypeNone:
      // LCOV_EXCL_START
      // This branch should be unreachable.
      ThrowImplementationError(
          "Internal util `AppendListValue` called with node type `none`!",
          node.getName());
      break;
      // LCOV_EXCL_STOP
  }
}

/// @brief Appends a key/value pair to the werkzeugkiste configuration.
/// @param cfg The werkzeugkiste configuration.
/// @param node Node/setting holding the libconfig value & parameter name.
// NOLINTNEXTLINE(misc-no-recursion)
void AppendKeyValue(Configuration &cfg, const libconfig::Setting &node) {
  std::string_view key{node.getName()};
  switch (node.getType()) {
    case libconfig::Setting::TypeInt:
      // NOLINTNEXTLINE(google-runtime-int)
      cfg.SetInt64(key, CastSetting<int64_t, int>(node));
      break;

    case libconfig::Setting::TypeInt64:
      // NOLINTNEXTLINE(google-runtime-int)
      cfg.SetInt64(key, CastSetting<int64_t, long long>(node));
      break;

    case libconfig::Setting::TypeFloat:
      cfg.SetDouble(key, CastSetting<double>(node));
      break;

    case libconfig::Setting::TypeString:
      cfg.SetString(key, CastSetting<std::string>(node));
      break;

    case libconfig::Setting::TypeBoolean:
      cfg.SetBoolean(key, CastSetting<bool>(node));
      break;

    case libconfig::Setting::TypeGroup:
      cfg.SetGroup(key, FromLibconfigGroup(node));
      break;

    case libconfig::Setting::TypeArray:
    case libconfig::Setting::TypeList: {
      cfg.CreateList(key);
      for (int i = 0; i < node.getLength(); ++i) {
        AppendListValue(key, cfg, node[i]);
      }
      break;
    }

    case libconfig::Setting::TypeNone:
      // LCOV_EXCL_START
      // This branch should be unreachable.
      ThrowImplementationError(
          "Internal util `AppendKeyValue` called with node type `none`!",
          node.getName());
      break;
      // LCOV_EXCL_STOP
  }
}

/// @brief Converts a libconfig node to a werkzeugkiste configuration group.
/// @param node The libconfig group to be converted.
// NOLINTNEXTLINE(misc-no-recursion)
Configuration FromLibconfigGroup(const libconfig::Setting &node) {
  if (!node.isGroup()) {
    // LCOV_EXCL_START
    // This branch should be unreachable.
    ThrowImplementationError(
        "Internal util `FromLibconfigGroup` invoked with non-group node!",
        node.getName());
    // LCOV_EXCL_STOP
  }

  Configuration grp{};
  for (int i = 0; i < node.getLength(); ++i) {
    AppendKeyValue(grp, node[i]);
  }
  return grp;
}
}  // namespace detail::parsing

Configuration LoadLibconfigString(std::string_view lcfg_string) {
  try {
    const std::string str{lcfg_string};
    libconfig::Config lcfg;
    lcfg.readString(str);
    return detail::parsing::FromLibconfigGroup(lcfg.getRoot());
  } catch (const libconfig::ParseException &e) {
    std::string msg{"Parsing libconfig string failed at line `"};
    msg += std::to_string(e.getLine());
    msg += "`: ";
    msg += e.getError();
    throw ParseError{msg};
  }
}

Configuration LoadLibconfigFile(std::string_view filename) {
  try {
    const std::string sfn{filename};
    libconfig::Config lcfg;
    lcfg.readFile(sfn.c_str());
    return detail::parsing::FromLibconfigGroup(lcfg.getRoot());
  } catch (const libconfig::ParseException &e) {
    std::string msg{"Cannot load libconfig file `"};
    msg += filename;
    msg += "`, error at line ";
    msg += std::to_string(e.getLine());
    msg += "`: ";
    msg += e.getError();
    throw ParseError{msg};
  } catch (const libconfig::FileIOException &e) {
    std::string msg{"I/O error while loading libconfig file `"};
    msg += filename;
    msg += "`!";
    throw ParseError{msg};
  }
}

#else  // WERKZEUGKISTE_WITH_LIBCONFIG
#define WZK_THROW_LIBCONFIG_MISSING                                        \
  do {                                                                     \
    throw std::logic_error{                                                \
        "werkzeugkiste::config has been built without libconfig support. " \
        "Please install libconfig++ and rebuilt the library with "         \
        "`werkzeugkiste_WITH_LIBCONFIG` enabled"};                         \
  } while (false)

Configuration LoadLibconfigString(std::string_view /*lcfg_string*/) {
  WZK_THROW_LIBCONFIG_MISSING;
}

Configuration LoadLibconfigFile(std::string_view /*filename*/) {
  WZK_THROW_LIBCONFIG_MISSING;
}

#undef WZK_THROW_LIBCONFIG_MISSING
#endif  // WERKZEUGKISTE_WITH_LIBCONFIG

/// @brief Custom formatter
namespace detail::formatter {
// Forward declaration.
void PrintGroup(const Configuration &cfg,
    std::ostream &out,
    std::size_t indent,
    bool include_brackets);

/// @brief Returns a libconfig-compatible string representation.
std::string EscapeString(std::string_view str) {
  const auto pos = str.find_first_of('\"');
  std::string quoted{'"'};
  if (pos == std::string_view::npos) {
    quoted += str;
  } else {
    quoted += strings::Replace(str, "\"", "\\\"");
  }
  quoted += '"';
  return quoted;
}

/// @brief Returns a string representation of the floating point value.
///
/// To ensure that the floating point value will be correctly parsed as a
/// libconfig floating point again, it must be either in scientific notation
/// or contain a fractional part.
std::string FloatingPointString(double val) {
  std::string str{std::to_string(val)};
  if (strings::IsInteger(str)) {
    str += ".0";
  }
  return str;
}

/// @brief Returns a string representation of the integral value.
///
/// Although the trailing 'L' for 64-bit numbers is optional since v1.5 of
/// libconfig, we experienced conversion issues (failed test cases), i.e.
/// values were still converted to 32-bit. Thus, we explicilty append the
/// type suffix, if the value exceeds the 32-bit range.
std::string IntegerString(int64_t val) {
  std::string str{std::to_string(val)};
  using Limits = std::numeric_limits<int32_t>;
  if ((val < static_cast<int64_t>(Limits::min())) ||
      (val > static_cast<int64_t>(Limits::max()))) {
    str += 'L';
  }
  return str;
}

/// @brief Prints the libconfig-compatible string representation of the scalar
///   parameter to the given output stream.
/// @param cfg The werkzeugkiste configuration.
/// @param out The output stream.
/// @param key Fully qualified parameter name.
void PrintScalar(const Configuration &cfg,
    std::ostream &out,
    std::string_view key) {
  switch (cfg.Type(key)) {
    case ConfigType::Boolean:
      out << std::boolalpha << cfg.GetBoolean(key);
      break;

    case ConfigType::Integer:
      out << IntegerString(cfg.GetInt64(key));
      break;

    case ConfigType::FloatingPoint:
      out << FloatingPointString(cfg.GetDouble(key));
      break;

    case ConfigType::String:
      out << EscapeString(cfg.GetString(key));
      break;

    case ConfigType::Date:
      out << EscapeString(cfg.GetDate(key).ToString());
      break;

    case ConfigType::Time:
      out << EscapeString(cfg.GetTime(key).ToString());
      break;

    case ConfigType::DateTime:
      out << EscapeString(cfg.GetDateTime(key).ToString());
      break;

    // LCOV_EXCL_START
    default: {
      // This branch should be unreachable.
      std::string msg{
          "`ConfigType` not handled in `PrintScalar` for parameter `"};
      msg += key;
      msg +=
          "`! Please report at"
          "https://github.com/snototter/werkzeugkiste/issues";
      throw std::logic_error{msg};
    }
      // LCOV_EXCL_STOP
  }
}

/// @brief Prints white-space indentation to the output stream.
/// @param out The output stream.
/// @param indent Indentation level.
void PrintIndent(std::ostream &out, std::size_t indentation_level) {
  for (std::size_t i = 0; i < indentation_level; ++i) {
    out << "  ";
  }
}

/// @brief Prints a libconfig-compatible string representation of the given
///   list parameter to the output stream.
/// @param cfg The werkzeugkiste configuration.
/// @param out The output stream.
/// @param key Fully qualified parameter name of the list.
/// @param indent Indentation level.
// NOLINTNEXTLINE(misc-no-recursion)
void PrintList(const Configuration &cfg,
    std::ostream &out,
    std::string_view key,
    std::size_t indent) {
  const bool is_homogeneous = cfg.IsHomogeneousScalarList(key);
  const std::size_t size = cfg.Size(key);
  const bool include_newline = !is_homogeneous && (size > 0);
  if (is_homogeneous) {
    out << '[';
  } else {
    out << '(';
  }

  if (include_newline) {
    out << '\n';
  }

  ++indent;
  for (std::size_t idx = 0; idx < size; ++idx) {
    std::ostringstream elem_key;
    elem_key << key << '[' << idx << ']';

    if (include_newline) {
      PrintIndent(out, indent);
    }

    switch (cfg.Type(elem_key.str())) {
      case ConfigType::Group:
        PrintGroup(cfg.GetGroup(elem_key.str()),
            out,
            indent,
            /*include_brackets=*/true);
        break;

      case ConfigType::List:
        PrintList(cfg, out, elem_key.str(), indent);
        break;

      default:
        PrintScalar(cfg, out, elem_key.str());
        break;
    }

    if (idx < (size - 1)) {
      out << ',' << (include_newline ? '\n' : ' ');
    }
  }
  --indent;

  if (include_newline) {
    out << '\n';
    PrintIndent(out, indent);
  }
  if (is_homogeneous) {
    out << ']';
  } else {
    out << ')';
  }
}

/// @brief Prints a libconfig-compatible string representation of the given
///   parameter group/table to the output stream.
/// @param cfg The werkzeugkiste configuration.
/// @param out The output stream.
/// @param key Fully qualified parameter name of the group/table.
/// @param indent Indentation level.
/// @param include_brackets If set to true, the enclosing curly brackets will
///   also be printed.
// NOLINTNEXTLINE(misc-no-recursion)
void PrintGroup(const Configuration &cfg,
    std::ostream &out,
    std::size_t indent,
    bool include_brackets) {
  const std::vector<std::string> keys = cfg.ListParameterNames(
      /*include_array_entries=*/false, /*recursive=*/false);

  const bool include_newline = !cfg.Empty();
  if (include_brackets) {
    out << '{';
    if (include_newline) {
      ++indent;
      out << '\n';
    }
  }

  for (std::size_t idx = 0; idx < keys.size(); ++idx) {
    const auto &key = keys[idx];

    PrintIndent(out, indent);
    out << key << " = ";

    switch (cfg.Type(key)) {
      case ConfigType::Group:
        PrintGroup(cfg.GetGroup(key), out, indent, /*include_brackets=*/true);
        break;

      case ConfigType::List:
        PrintList(cfg, out, key, indent);
        break;

      default:
        PrintScalar(cfg, out, key);
        break;
    }

    out << ";\n";
  }

  if (include_brackets) {
    if (include_newline) {
      --indent;
      PrintIndent(out, indent);
    }
    out << '}';
  }
}
}  // namespace detail::formatter

std::string DumpLibconfigString(const Configuration &cfg) {
  std::ostringstream str;
  detail::formatter::PrintGroup(cfg, str, 0, /*include_brackets=*/false);
  return str.str();
}
}  // namespace werkzeugkiste::config
