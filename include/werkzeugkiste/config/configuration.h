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

/// @brief Encapsulates configuration data.
class WERKZEUGKISTE_CONFIG_EXPORT Configuration {
 public:
  //  using date_type = std::tuple<uint16_t, uint8_t, uint8_t>;

  /// @brief Loads a TOML configuration from the given file.
  /// @param filename Path to the `.toml` file.
  static std::unique_ptr<Configuration> LoadTOMLFile(std::string_view filename);

  /// @brief Loads a TOML configuration from a string.
  /// @param toml_string String representation of the TOML config.
  static std::unique_ptr<Configuration> LoadTOMLString(
      std::string_view toml_string);

  //   static std::unique_ptr<Configuration> LoadJSON(std::string_view
  //   filename);

  virtual ~Configuration() = default;
  Configuration(const Configuration &) = delete;
  Configuration(Configuration &&) = delete;
  Configuration &operator=(const Configuration &) = delete;
  Configuration &operator=(Configuration &&) = delete;

  /// @brief Adjusts the given parameters to hold either an absolute file path,
  /// or the result of "base_path / <param>" if they initially held a relative
  /// file path.
  /// @param base_path Base path to be prepended to relative file paths.
  /// @param parameters A list of parameter names / patterns. The wildcard '*'
  /// is
  ///     also supported. For example, valid names are: "my-param",
  ///     "files.video1", etc. Valid patterns would be "*path",
  ///     "some.nested.*.filename", etc.
  /// @return True if any parameter has been adjusted.
  virtual bool AdjustRelativePaths(
      std::string_view base_path,
      const std::vector<std::string_view> &parameters) = 0;

  /// @brief Visits all string parameters and replaces any occurrence of the
  /// given needle/replacement pairs.
  /// @param replacements List of `<search, replacement>` pairs.
  /// @return True if any placeholder has actually been replaced.
  virtual bool ReplaceStringPlaceholders(
      const std::vector<std::pair<std::string_view, std::string_view>>
          &replacements) = 0;

  /// @brief Returns true if all configuration keys and values match exactly.
  virtual bool Equals(const Configuration *other) const = 0;

  /// @brief Returns a list of all (fully-qualified) parameter names.
  ///
  /// Note that this list does *not* contain a key for each array
  /// element. Only if another dictionary/table is within the array,
  /// the corresponding entry will be included (for example,
  /// `arr[3].name`)
  virtual std::vector<std::string> ParameterNames() const = 0;

  virtual bool GetBoolean(std::string_view key) const = 0;
  virtual bool GetBooleanOrDefault(std::string_view key,
                                   bool default_val) const = 0;
  virtual void SetBoolean(std::string_view key, bool value) = 0;

  virtual double GetDouble(std::string_view key) const = 0;
  virtual double GetDoubleOrDefault(std::string_view key,
                                    double default_val) const = 0;
  virtual void SetDouble(std::string_view key, double value) = 0;

  virtual int32_t GetInteger32(std::string_view key) const = 0;
  virtual int32_t GetInteger32OrDefault(std::string_view key,
                                        int32_t default_val) const = 0;
  virtual void SetInteger32(std::string_view key, int32_t value) = 0;

  virtual int64_t GetInteger64(std::string_view key) const = 0;
  virtual int64_t GetInteger64OrDefault(std::string_view key,
                                        int64_t default_val) const = 0;
  virtual void SetInteger64(std::string_view key, int64_t value) = 0;

  virtual std::string GetString(std::string_view key) const = 0;
  virtual std::string GetStringOrDefault(
      std::string_view key, std::string_view default_val) const = 0;
  virtual void SetString(std::string_view key, std::string_view value) = 0;

  //---------------------------------------------------------------------------
  // Date/time data types

  //  virtual date_type GetDate(std::string_view key) const = 0;
  //  virtual date_type GetDateOrDefault(std::string_view key,
  //                                     const date_type &default_val) const =
  //                                     0;
  //  virtual void SetDate(std::string_view key, const date_type &value) = 0;

  //---------------------------------------------------------------------------
  // Lists/pairs of scalar data types

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

  /// @brief Loads a nested TOML configuration.
  ///
  /// For example, if your configuration had a field "storage", which
  /// should be defined in a separate (e.g. machine-dependent) configuration
  /// file, the "main" config could define it as `storage = "path/to/conf.toml"`
  /// This function will then load this TOML and replace `storage` by the
  /// loaded configuration.
  /// Suppose that `conf.toml` defines `location = ...` and `duration = ...`.
  /// Then, after loading, you can access these as `"storage.location"` and
  /// `"storage.duration"`.
  ///
  /// @param key Parameter name (fully-qualified TOML path) which holds the
  ///     file name of the nested TOML configuration (must be of type
  ///     string)
  virtual void LoadNestedTOMLConfiguration(std::string_view key) = 0;
  // TODO LoadNestedJSONConfiguration(param_name)

  // TODO do we need std::map<std::string, std::variant<int64_t, double,
  // std::string>> GetDictionary / GetTable

  /// @brief Returns a TOML-formatted string of this configuration.
  virtual std::string ToTOML() const = 0;

  /// @brief Returns a JSON-formatted string of this configuration.
  virtual std::string ToJSON() const = 0;

 protected:
  Configuration() = default;
};

}  // namespace werkzeugkiste::config

#endif  // WERKZEUGKISTE_CONFIG_CONFIGURATION_H
