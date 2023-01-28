#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/logging.h>
#include <vector>
#include <toml++/toml.h>
#include <werkzeugkiste/strings/strings.h>
#include <werkzeugkiste/files/fileio.h>

namespace werkzeugkiste::config {



class ConfigurationImpl : public Configuration {
public:
  ~ConfigurationImpl() {}
  explicit ConfigurationImpl(toml::table &&config)
    : Configuration(), config_(std::move(config))
    {
        const std::string_view key {"visualization.*.save"};
        auto tokens = werkzeugkiste::strings::Tokenize(key, ".");
        for (const auto &token : tokens) {
            WKZLOG_WARN("TODO check key-token '{:s}'", token);
        }
        // auto test = config_[tokens[0]];
        // WKZLOG_WARN("key: {:s}", test[tokens[1]]);
    }
  
  void RegisterPathParameter(std::string_view param_name) override {
    path_parameters_.push_back(std::string(param_name));
  }

  std::size_t MakePathsAbsolute(std::string_view base_path) override {
    for (const auto &param : path_parameters_) {
        WKZLOG_WARN("TODO need to check path parameter: {:s}", param);
    }
  }

  std::string ToTOML() const override {
    return "TODO";
  }

  std::string ToJSON() const override {
    return "TODO";
  }

private:
  toml::table config_{};
  std::vector<std::string> path_parameters_{};
};



std::unique_ptr<Configuration> Configuration::LoadTomlFile(std::string_view filename) {
  const std::string toml = werkzeugkiste::files::CatAsciiFile(filename);
  return Configuration::LoadTomlString(toml);
}

std::unique_ptr<Configuration> Configuration::LoadTomlString(std::string_view toml_string) {
  try
    {
        toml::table tbl = toml::parse(toml_string);
        WKZLOG_INFO("Loaded toml: {:s}", tbl);
        return std::make_unique<ConfigurationImpl>(ConfigurationImpl(std::move(tbl)));
    } catch (const toml::parse_error &err) {
        WKZLOG_ERROR("Error parsing TOML: {:s} ({:s})", err.description(), err.source().begin);
        return nullptr;
    }
}

// std::unique_ptr<Configuration> Configuration::LoadJSON(std::string_view filename) {
//     return std::make_unique<ConfigurationImpl>(ConfigurationImpl()); // TODO
// }
}  // namespace werkzeugkiste::config
