#ifndef WERKZEUGKISTE_CONFIG_CONFIGURATION_H
#define WERKZEUGKISTE_CONFIG_CONFIGURATION_H

#include <werkzeugkiste/config/config_export.h>

#include <cmath>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

/// Utilities to handle configurations.
namespace werkzeugkiste::config {

class WERKZEUGKISTE_CONFIG_EXPORT SingleKeyMatcher {
 public:
  static std::unique_ptr<SingleKeyMatcher> Create(std::string_view pattern);
  virtual bool Match(std::string_view key) const = 0;

  virtual ~SingleKeyMatcher() = default;
  SingleKeyMatcher(const SingleKeyMatcher & /* other */) = default;
  SingleKeyMatcher &operator=(const SingleKeyMatcher & /* other */) = default;
  SingleKeyMatcher(SingleKeyMatcher && /* other */) = default;
  SingleKeyMatcher &operator=(SingleKeyMatcher && /* other */) = default;

 protected:
  SingleKeyMatcher() = default;
};

class WERKZEUGKISTE_CONFIG_EXPORT MultiKeyMatcher {
 public:
  static std::unique_ptr<MultiKeyMatcher> Create(
      const std::vector<std::string_view> &patterns);
  virtual ~MultiKeyMatcher() = default;

  virtual bool MatchAny(std::string_view key) const = 0;
};

class WERKZEUGKISTE_CONFIG_EXPORT Configuration {
 public:
  /// Loads the configuration from the given file.
  ///
  /// TODO runtime_error if file doesn't exits or toml is malformed

  // TODO must be a unique_ptr, implement copy/move ctor/assignments!
  static std::unique_ptr<Configuration> LoadTOMLFile(std::string_view filename);

  static std::unique_ptr<Configuration> LoadTOMLString(
      std::string_view toml_string);
  //   static std::unique_ptr<Configuration> LoadJSON(std::string_view
  //   filename);

  virtual ~Configuration() = default;
  Configuration(const Configuration &) = delete;
  Configuration(Configuration &&) = delete;
  Configuration &operator=(const Configuration &) = delete;
  Configuration &operator=(Configuration &&) = delete;

  /// Visits all parameter names and ensures that they hold absolute paths.
  /// Returns true if any parameter has been changed.
  /// TODO wildcard
  /// TODO Example
  virtual bool EnsureAbsolutePaths(
      std::string_view base_path,
      const std::vector<std::string_view> &parameters) = 0;

  /// Visits all string parameters and replaces any occurrence of the given
  /// needle/replacement pairs. Returns True if any parameter has been changed.
  virtual bool ReplaceStringPlaceholders(
      const std::vector<std::pair<std::string_view, std::string_view>>
          &replacements) = 0;

  // keys and values match exactly
  virtual bool Equals(const Configuration *other) const = 0;

  /// Returns a list of all (fully-qualified) parameter names, e.g.
  /// "some_table.param_x".
  virtual std::vector<std::string> ParameterNames() const = 0;

  virtual bool GetBoolean(std::string_view key) const = 0;
  virtual bool GetBooleanOrDefault(std::string_view key,
                                   bool default_val) const = 0;

  virtual double GetDouble(std::string_view key) const = 0;
  virtual double GetDoubleOrDefault(std::string_view key,
                                    double default_val) const = 0;

  virtual int32_t GetInteger32(std::string_view key) const = 0;
  virtual int32_t GetInteger32OrDefault(std::string_view key,
                                        int32_t default_val) const = 0;

  virtual int64_t GetInteger64(std::string_view key) const = 0;
  virtual int64_t GetInteger64OrDefault(std::string_view key,
                                        int64_t default_val) const = 0;

  virtual std::string GetString(std::string_view key) const = 0;
  virtual std::string GetStringOrDefault(
      std::string_view key, std::string_view default_val) const = 0;

  virtual std::pair<double, double> GetDoublePair(
      std::string_view key) const = 0;
  virtual std::vector<double> GetDoubleList(std::string_view key) const = 0;

  virtual std::pair<int32_t, int32_t> GetInteger32Pair(
      std::string_view key) const = 0;
  virtual std::vector<int32_t> GetInteger32List(std::string_view key) const = 0;

  virtual std::pair<int64_t, int64_t> GetInteger64Pair(
      std::string_view key) const = 0;
  virtual std::vector<int64_t> GetInteger64List(std::string_view key) const = 0;

  virtual std::vector<std::string> GetStringList(
      std::string_view key) const = 0;

  virtual std::vector<std::tuple<int32_t, int32_t>> GetPoints2D(
      std::string_view key) const = 0;

  virtual std::vector<std::tuple<int32_t, int32_t, int32_t>> GetPoints3D(
      std::string_view key) const = 0;

  // TODO do we need double-precision points ?
  // TODO do we need nested lists ?

  // TODO should return a copy!
  //    virtual std::unique_ptr<Configuration> GetGroup(std::string_view
  //    group_name) const = 0;

  // TODO doc
  virtual void LoadNestedTOMLConfiguration(std::string_view key) = 0;
  // TODO LoadNestedJSONConfiguration(param_name)
  // TODO LoadNestedLibconfigConfiguration

  // TODO do we need std::map<std::string, std::variant<int64_t, double,
  // std::string>> GetDictionary / GetTable

  /// Returns a TOML-formatted string of this configuration.
  virtual std::string ToTOML() const = 0;

  /// Returns a JSON-formatted string of this configuration.
  virtual std::string ToJSON() const = 0;

 protected:
  Configuration() = default;
};

}  // namespace werkzeugkiste::config

#endif  // WERKZEUGKISTE_CONFIG_CONFIGURATION_H
