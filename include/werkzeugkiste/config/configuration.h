#ifndef WERKZEUGKISTE_CONFIG_CONFIGURATION_H
#define WERKZEUGKISTE_CONFIG_CONFIGURATION_H

#include <cmath>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

/// Utilities to handle configurations.
namespace werkzeugkiste::config {

class Configuration {
 public:
  /// Loads the configuration from the given file.
  ///
  /// TODO runtime_error if file doesn't exits or toml is malformed
  static std::unique_ptr<Configuration> LoadTomlFile(std::string_view filename);
  static std::unique_ptr<Configuration> LoadTomlString(
      std::string_view toml_string);
  //   static std::unique_ptr<Configuration> LoadJSON(std::string_view
  //   filename);

  virtual ~Configuration() = default;
  Configuration(const Configuration &) = delete;
  Configuration(Configuration &&) = delete;
  Configuration &operator=(const Configuration &) = delete;
  Configuration &operator=(Configuration &&) = delete;

  /// @brief Registers the given parameter name for future replacement (in
  /// `MakePathsAbsolute`).
  /// @param parameter Name of the parameter, e.g. "storage.folder",
  /// "input_filename", ...
  // virtual void RegisterPathParameter(std::string_view param_name) = 0;

  /// @brief Ensures that all registered path parameters are absolute.
  /// @return Number of adjusted parameters.
  // virtual std::size_t MakePathsAbsolute(std::string_view base_path) = 0;

  // True if anything has been replaced
  virtual bool EnsureAbsolutePaths(
      std::string_view base_path,
      const std::vector<std::string_view> &parameters) = 0;

  /// Returns a list of all (fully-qualified) parameter names, e.g.
  /// "some_table.param_x".
  virtual std::vector<std::string> ParameterNames() const = 0;

  // TODO LoadNestedTOMLConfiguration(param_name)
  // TODO LoadNestedJSONConfiguration(param_name)
  // TODO LoadNestedLibconfigConfiguration
  // TODO MakePathsAbsolute(base, list of params)

  // TODO
  // https://github.com/snototter/vitocpp/blob/master/src/cpp/vcp_config/config_params.h

  // TODO register params which link to nested config files (would need to be
  // loaded afterwards)
  // TODO template T GetCompulsory/GetOptional(param "x.y")

  /// Returns a TOML-formatted string of this configuration.
  virtual std::string ToTOML() const = 0;

  /// Returns a JSON-formatted string of this configuration.
  virtual std::string ToJSON() const = 0;

 protected:
  Configuration() = default;
};

}  // namespace werkzeugkiste::config

#endif  // WERKZEUGKISTE_CONFIG_CONFIGURATION_H
