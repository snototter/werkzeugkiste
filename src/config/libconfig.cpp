#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/files/fileio.h>
#include <werkzeugkiste/logging.h>

#include <cstdint>
#include <iomanip>
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
namespace detail {
// Forward declarations
Configuration FromLibconfigGroup(const libconfig::Setting &node);
void SerializeGroup(const Configuration &cfg,
    std::ostream &out,
    std::size_t indent,
    bool is_standalone);

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

void AppendListSetting(std::string_view lst_key,
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
      cfg.AppendNestedList(lst_key);
      std::string elem_key{lst_key};
      elem_key += '[';
      elem_key += std::to_string(sz);
      elem_key += ']';

      for (int i = 0; i < node.getLength(); ++i) {
        AppendListSetting(elem_key, cfg, node[i]);
      }
      break;
    }

    case libconfig::Setting::TypeNone:
      // LCOV_EXCL_START
      // This branch should be unreachable.
      ThrowImplementationError(
          "Internal util `AppendNamedSetting` called with node type `none`!",
          node.getName());
      break;
      // LCOV_EXCL_STOP
  }
}

void AppendNamedSetting(Configuration &cfg, const libconfig::Setting &node) {
  std::string_view key{node.getName()};
  switch (node.getType()) {
    case libconfig::Setting::TypeInt:
      // NOLINTNEXTLINE(google-runtime-int)
      cfg.SetInteger64(key, CastSetting<int64_t, int>(node));
      break;

    case libconfig::Setting::TypeInt64:
      // NOLINTNEXTLINE(google-runtime-int)
      cfg.SetInteger64(key, CastSetting<int64_t, long long>(node));
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
        AppendListSetting(key, cfg, node[i]);
      }
      break;
    }

    case libconfig::Setting::TypeNone:
      // LCOV_EXCL_START
      // This branch should be unreachable.
      ThrowImplementationError(
          "Internal util `AppendNamedSetting` called with node type `none`!",
          node.getName());
      break;
      // LCOV_EXCL_STOP
  }
}

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
    AppendNamedSetting(grp, node[i]);
  }
  return grp;
}

std::string EscapeString(std::string_view str) {
  const auto pos = str.find_first_of("\"");
  if (pos == std::string_view::npos) {
    std::string quoted{'"'};
    quoted += str;
    quoted += '"';
    return quoted;
  } else {
    // TODO escape quote
    // TODO do we need to escape backslashes?
    return "\"TODO\"";
  }
}

void PrintScalar(const Configuration &cfg,
    std::ostream &out,
    std::string_view key) {
  switch (cfg.Type(key)) {
    case ConfigType::Boolean:
      out << std::boolalpha << cfg.GetBoolean(key);
      break;

    case ConfigType::Integer:
      out << cfg.GetInteger64(key);
      break;

    case ConfigType::FloatingPoint:
      // TODO check if it would be representable by an integer
      // if so, we need to print with fixed precision
      // otherwise, std::defaultfloat is fine
      out << cfg.GetDouble(key);
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

    default:
      // TODO throw (unreachable)
      break;
  }
}

void PrintIndent(std::ostream &out, std::size_t indent) {
  for (std::size_t i = 0; i < indent; ++i) {
    out << "    ";
  }
}

void SerializeList(const Configuration &cfg,
    std::ostream &out,
    std::string_view key,
    std::size_t indent) {
  if (cfg.IsHomogeneousScalarList(key)) {
    out << '[';
  } else {
    out << '(';
  }

  for (std::size_t i = 0; i < cfg.Size(key); ++i) {
    if (i > 0) {
      out << ", ";
    }
    std::ostringstream elem_key;
    elem_key << key << '[' << i << ']';

    switch (cfg.Type(elem_key.str())) {
      case ConfigType::Group:
        SerializeGroup(cfg.GetGroup(elem_key.str()), out, indent + 1, false);
        break;

      case ConfigType::List:
        SerializeList(cfg, out, elem_key.str(), indent + 1);
        break;

      default:
        PrintScalar(cfg, out, elem_key.str());
        break;
    }
  }

  if (cfg.IsHomogeneousScalarList(key)) {
    out << ']';
  } else {
    out << ')';
  }
}

void SerializeGroup(const Configuration &cfg,
    std::ostream &out,
    std::size_t indent,
    bool is_standalone) {
  const std::vector<std::string> keys = cfg.ListParameterNames(
      /*include_array_entries=*/false, /*recursive=*/false);
  for (const auto &key : keys) {
    PrintIndent(out, indent);
    out << key << " = ";

    switch (cfg.Type(key)) {
      case ConfigType::Group: {
        if (cfg.Size(key) > 0) {
          out << "{\n";
          SerializeGroup(cfg.GetGroup(key), out, indent + 1, true);
          PrintIndent(out, indent);
          out << '}';
        } else {
          out << "{}";
        }
        break;
      }

      case ConfigType::List:
        SerializeList(cfg, out, key, indent);
        break;

      default:
        PrintScalar(cfg, out, key);
        break;
    }

    if (is_standalone) {
      out << ";\n";
    }
  }
}
}  // namespace detail

Configuration LoadLibconfigString(std::string_view lcfg_string) {
  try {
    const std::string str{lcfg_string};
    libconfig::Config lcfg;
    lcfg.readString(str);
    return detail::FromLibconfigGroup(lcfg.getRoot());
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
    return detail::FromLibconfigGroup(lcfg.getRoot());
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

std::string DumpLibconfigString(const Configuration &cfg) {
  std::ostringstream str;
  detail::SerializeGroup(cfg, str, 0, true);
  return str.str();
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

std::string DumpLibconfigString(const Configuration& /*cfg*/) {
  WZK_THROW_LIBCONFIG_MISSING;
}

#undef WZK_THROW_LIBCONFIG_MISSING
#endif  // WERKZEUGKISTE_WITH_LIBCONFIG
}  // namespace werkzeugkiste::config
