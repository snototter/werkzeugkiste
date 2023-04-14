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
void AppendListItems(const libconfig::Setting &node,
    Configuration &cfg,
    std::string_view fqn);

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

/// @brief Appends or sets a configuration value from a libconfig scalar node.
/// @tparam Tcfg Data type used in werkzeugkiste.
/// @tparam Tlibcfg Data type used in libconfig (see their operator overloads).
/// @param value Value to be converted.
/// @param cfg Configuration to be modified.
/// @param fqn Fully qualified parameter name.
/// @param append If true, fqn is assumed to be a list and the value will be
///   appended. Otherwise, the value will be set, i.e. "cfg[fqn] = value";
template <typename Tcfg, typename Tlibcfg = Tcfg>
void HandleBuiltinScalar(const libconfig::Setting &value,
    Configuration &cfg,
    std::string_view fqn,
    bool append) {
  try {
    Tcfg val{};
    if constexpr (std::is_same_v<Tcfg, Tlibcfg>) {
      val = static_cast<Tcfg>(value);
    } else {
      const Tlibcfg tmp = static_cast<Tlibcfg>(value);
      val = static_cast<Tcfg>(tmp);
    }

    if (append) {
      cfg.Append(fqn, val);
    } else {
      cfg.Set(fqn, val);
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

/// @brief Appends or sets a configuration value from a libconfig node.
void HandleNode(const libconfig::Setting &node,
    Configuration &cfg,
    std::string_view fqn,
    bool append) {
  switch (node.getType()) {
    case libconfig::Setting::TypeInt:
      // NOLINTNEXTLINE(google-runtime-int)
      HandleBuiltinScalar<int64_t, int>(node, cfg, fqn, append);
      break;

    case libconfig::Setting::TypeInt64:
      // NOLINTNEXTLINE(google-runtime-int)
      HandleBuiltinScalar<int64_t, long long>(node, cfg, fqn, append);
      break;

    case libconfig::Setting::TypeFloat:
      HandleBuiltinScalar<double>(node, cfg, fqn, append);
      break;

    case libconfig::Setting::TypeString:
      HandleBuiltinScalar<std::string>(node, cfg, fqn, append);
      break;

    case libconfig::Setting::TypeBoolean:
      HandleBuiltinScalar<bool>(node, cfg, fqn, append);
      break;

    case libconfig::Setting::TypeGroup: {
      if (append) {
        cfg.Append(fqn, FromLibconfigGroup(node));
      } else {
        cfg.Set(fqn, FromLibconfigGroup(node));
      }
      break;
    }

    case libconfig::Setting::TypeArray:
    case libconfig::Setting::TypeList: {
      if (append) {
        const std::size_t lst_sz = cfg.Size(fqn);
        const std::string elem_key =
            Configuration::KeyForListElement(fqn, lst_sz);
        cfg.AppendList(fqn);
        AppendListItems(node, cfg, elem_key);
      } else {
        cfg.CreateList(fqn);
        AppendListItems(node, cfg, fqn);
      }
      break;
    }

    case libconfig::Setting::TypeNone:
      // LCOV_EXCL_START
      // This branch should be unreachable.
      ThrowImplementationError(
          "Internal util `AppendListValue` called with node type `none`", fqn);
      break;
      // LCOV_EXCL_STOP
  }
}

void AppendListItems(const libconfig::Setting &node,
    Configuration &cfg,
    std::string_view fqn) {
  // LCOV_EXCL_START
  if (!node.isList() && !node.isArray()) {
    ThrowImplementationError(
        "Internal util `AppendListItems` called with non-list/array node ",
        fqn);
  }
  // LCOV_EXCL_STOP

  for (int i = 0; i < node.getLength(); ++i) {
    const libconfig::Setting &elem = node[i];
    HandleNode(elem, cfg, fqn, /*append=*/true);
  }
}

/// @brief Converts a libconfig node to a werkzeugkiste configuration group.
/// @param node The libconfig group to be converted.
// NOLINTNEXTLINE(misc-no-recursion)
Configuration FromLibconfigGroup(const libconfig::Setting &node) {
  // LCOV_EXCL_START
  if (!node.isGroup()) {
    // This branch should be unreachable.
    ThrowImplementationError(
        "Internal util `FromLibconfigGroup` invoked with non-group node!", "");
  }
  // LCOV_EXCL_STOP

  Configuration grp{};
  for (int i = 0; i < node.getLength(); ++i) {
    HandleNode(node[i], grp, node[i].getName(), /**append=*/false);
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
      out << std::boolalpha << cfg.GetBool(key);
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
    const std::string elem_key = Configuration::KeyForListElement(key, idx);

    if (include_newline) {
      PrintIndent(out, indent);
    }

    switch (cfg.Type(elem_key)) {
      case ConfigType::Group:
        PrintGroup(cfg.GetGroup(elem_key),
            out,
            indent,
            /*include_brackets=*/true);
        break;

      case ConfigType::List:
        PrintList(cfg, out, elem_key, indent);
        break;

      default:
        PrintScalar(cfg, out, elem_key);
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
