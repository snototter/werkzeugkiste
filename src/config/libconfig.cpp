#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/files/fileio.h>
#include <werkzeugkiste/logging.h>

#include <cstdint>
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
// Forward declaration
Configuration FromLibconfigGroup(const libconfig::Setting &node);

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
    std::string msg{"Cannot convert libconfig setting `"};
    msg += value.getName();
    msg += "` to type `";
    msg += TypeName<Tlibcfg>();
    msg += "`: ";
    msg += e.what();
    throw TypeError{msg};
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
      ThrowImplementationError(
          "Internal util `AppendNamedSetting` called with node type `none`!",
          node.getName());
      break;
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
      ThrowImplementationError(
          "Internal util `AppendNamedSetting` called with node type `none`!",
          node.getName());
      break;
  }
}

Configuration FromLibconfigGroup(const libconfig::Setting &node) {
  if (!node.isGroup()) {
    // This branch should be unreachable.
    ThrowImplementationError(
        "Internal util `FromLibconfigGroup` invoked with non-group node!",
        node.getName());
  }

  Configuration grp{};
  for (int i = 0; i < node.getLength(); ++i) {
    AppendNamedSetting(grp, node[i]);
  }
  return grp;
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

#else   // WERKZEUGKISTE_WITH_LIBCONFIG
Configuration Configuration::LoadLibconfigString(std::string_view toml_string) {
  throw std::logic_error{
      "werkzeugkiste::config has been built without libconfig support. "
      "Please install libconfig++ and rebuilt the library with "
      "`werkzeugkiste_WITH_LIBCONFIG` enabled"};
}

Configuration Configuration::LoadLibconfigFile(std::string_view filename) {
  return LoadLibconfigString("");
}
#endif  // WERKZEUGKISTE_WITH_LIBCONFIG
}  // namespace werkzeugkiste::config
