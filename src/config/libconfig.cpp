#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/files/fileio.h>
#include <werkzeugkiste/logging.h>

#include <cstdint>
#include <string_view>
#include <type_traits>

// TODO remove
#define WERKZEUGKISTE_WITH_LIBCONFIG

#ifdef WERKZEUGKISTE_WITH_LIBCONFIG
#if __has_include(<libconfig.hh>)
#include <libconfig.hh>
#elif __has_include(<libconfig.h++>)
#include <libconfig.h++>
#else
#error \
    "WZK_WITH_LIBCONFIG was enabled, but libconfig header file cannot be found!"
#endif  // has_include
#endif  // WERKZEUGKISTE_WITH_LIBCONFIG

namespace werkzeugkiste::config {
#ifdef WERKZEUGKISTE_WITH_LIBCONFIG
namespace detail {
// Forward declarations
Configuration FromLibconfigGroup(const libconfig::Setting &node);
void AppendList(Configuration &cfg, const libconfig::Setting &node);

template <typename Tcfg, typename Tlibcfg = Tcfg>
Tcfg CastSetting(const libconfig::Setting &value) {
  try {
    if constexpr (std::is_same_v<Tcfg, Tlibcfg>) {
      return static_cast<Tcfg>(value);
    } else {  // NOLINT
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

void AppendSetting(Configuration &cfg, const libconfig::Setting &node) {
  const char *name = node.getName();
  switch (node.getType()) {
    case libconfig::Setting::TypeInt:
      // NOLINTNEXTLINE(google-runtime-int)
      cfg.SetInteger64(name, CastSetting<int64_t, int>(node));
      break;

    case libconfig::Setting::TypeInt64:
      // NOLINTNEXTLINE(google-runtime-int)
      cfg.SetInteger64(name, CastSetting<int64_t, long long>(node));
      break;

    case libconfig::Setting::TypeFloat:
      cfg.SetDouble(name, CastSetting<double>(node));
      break;

    case libconfig::Setting::TypeString:
      cfg.SetString(name, CastSetting<std::string>(node));
      break;

    case libconfig::Setting::TypeBoolean:
      cfg.SetBoolean(name, CastSetting<bool>(node));
      break;

    case libconfig::Setting::TypeGroup:
      cfg.SetGroup(name, FromLibconfigGroup(node));
      break;

    case libconfig::Setting::TypeArray:
    case libconfig::Setting::TypeList:
      AppendList(cfg, node);
      break;

      // TODO TypeNone / default, throw
  }
}

void AppendList(Configuration &cfg, const libconfig::Setting &node) {
  if (!node.isArray() && !node.isList()) {
    // TODO throw logic_error (should be unreachable, though)
  }

  // TODO Configuration needs to be refactored:
  // * Allow appending any type to an existing list
  // Workflow would be 1) create empty list, 2) append the children of this
  // array/list
  WZKLOG_CRITICAL("TODO need to extract an array/a list: {}", node.getName());
}

Configuration FromLibconfigGroup(const libconfig::Setting &node) {
  if (!node.isGroup()) {
    // TODO throw logic_error (should be unreachable, though)
  }

  const char *name = node.getName();
  WZKLOG_CRITICAL("TODO need to extract a group: `{}`",
                  (name != nullptr) ? name : "ROOT");

  Configuration grp{};
  for (int i = 0; i < node.getLength(); ++i) {
    AppendSetting(grp, node[i]);
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
    throw ParseError(e.what());  // TODO do we need getError instead?
  }
}

#else   // WERKZEUGKISTE_WITH_LIBCONFIG
Configuration Configuration::LoadLibconfigString(std::string_view toml_string) {
  throw std::logic_error{
      "werkzeugkiste::config has been built without libconfig support. "
      "Please install libconfig++ and rebuilt the library with "
      "`WZK_WITH_LIBCONFIG` enabled"};
}

Configuration Configuration::LoadLibconfigFile(std::string_view filename) {
  return LoadLibconfigString("");
}
#endif  // WERKZEUGKISTE_WITH_LIBCONFIG
}  // namespace werkzeugkiste::config
