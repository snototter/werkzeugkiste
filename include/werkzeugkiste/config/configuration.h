#ifndef WERKZEUGKISTE_CONFIG_CONFIGURATION_H
#define WERKZEUGKISTE_CONFIG_CONFIGURATION_H

#include <werkzeugkiste/config/config_export.h>

#include <cmath>
#include <initializer_list>
#include <limits>
#include <memory>
#include <optional>
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
//-----------------------------------------------------------------------------
// Readable type identifiers to support meaningful error messages

template <typename T>
constexpr const char *TypeName() {
  return typeid(T).name();
}

// NOLINTNEXTLINE(*macro-usage)
#define WZKREG_TNSPEC_STR(T, R)         \
  template <>                           \
  constexpr const char *TypeName<T>() { \
    return #R;                          \
  }

// NOLINTNEXTLINE(*macro-usage)
#define WZKREG_TNSPEC(T) WZKREG_TNSPEC_STR(T, T)

WZKREG_TNSPEC(bool)
WZKREG_TNSPEC(int8_t)
WZKREG_TNSPEC(uint8_t)
WZKREG_TNSPEC(int16_t)
WZKREG_TNSPEC(uint16_t)
WZKREG_TNSPEC(int32_t)
WZKREG_TNSPEC(uint32_t)
WZKREG_TNSPEC(int64_t)
WZKREG_TNSPEC(uint64_t)
WZKREG_TNSPEC(float)
WZKREG_TNSPEC(double)
WZKREG_TNSPEC_STR(std::string, string)
WZKREG_TNSPEC_STR(std::string_view, string_view)

#undef WZKREG_TNSPEC
#undef WZKREG_TNSPEC_STR

//-----------------------------------------------------------------------------
// Supported parameters

enum class ConfigType : unsigned char {
  /// @brief Either true or false.
  Boolean,

  /// @brief A 32- or 64-bit integer.
  ///
  /// Internally, integers are always handled as 64-bits.
  Integer,

  /// @brief A single- or double-precision floating point number.
  ///
  /// Internally, floating point numbers are always represented by a double.
  FloatingPoint,

  /// @brief A string.
  String,

  // TODO date
  // TODO time
  // TODO date_time

  /// @brief A list (vector) of unnamed parameters.
  List,

  /// @brief A group (dictionary) of named parameters.
  Group
};

// TODO ostream/istream overloads for ConfigType
std::string ToString(const ConfigType &ct);

//-----------------------------------------------------------------------------
// Exceptions
// TODO doc: parsing error (syntax, I/O)
class ParseError : public std::exception {
 public:
  explicit ParseError(std::string msg) : msg_{std::move(msg)} {}

  const char *what() const noexcept override { return msg_.c_str(); }

 private:
  std::string msg_{};
};

// TODO doc: config key/parameter name does not exist
class KeyError : public std::exception {
 public:
  // NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
  explicit KeyError(std::string_view key) : msg_{"Key `"} {
    msg_.append(key);
    msg_.append("` does not exist!");
  }

  const char *what() const noexcept override { return msg_.c_str(); }

 private:
  std::string msg_{};
};

// TODO doc: wrong type assumed for getter/setter
class TypeError : public std::exception {
 public:
  explicit TypeError(std::string msg) : msg_{std::move(msg)} {}

  const char *what() const noexcept override { return msg_.c_str(); }

 private:
  std::string msg_{};
};

/// @brief Encapsulates configuration data.
///
/// TODO doc:
/// * Internally, a TOML configuration is used to store the parameters.
/// * I prefer explicit method names over a templated "Get<>".
/// * Get("unknown-key") throws a KeyError if the parameter does not exist.
/// * GetOptional returns an optional scalar.
/// * Get..Or returns a default value if the parameter does not exist.
///
/// TODOs:
/// * [x] Numeric casts. Implicitly cast if lossless conversion is possible.
/// * [ ] Do we need double-precision points ?
/// * [ ] LoadNestedJSONConfiguration
/// * [ ] LoadJSONFile
/// * [ ] LoadJSONString
/// * [ ] LoadLibconfigFile   ?
/// * [ ] LoadLibconfigString ?
/// * [ ] ToLibconfigString   ?
/// * [ ] Date
/// * [ ] Time
/// * [ ] NestedLists int & double (for "matrices")
/// * [ ] If eigen3 is available, enable GetMatrix.
///       Static dimensions vs dynamic?
/// * [ ] DateTime
/// * [ ] Setters for ...Pair
/// * [ ] Setters for ...List
/// * [x] SetGroup
/// * [x] Return optional<Scalar>
class WERKZEUGKISTE_CONFIG_EXPORT Configuration {
 public:
  //  //  using date_type = std::tuple<uint16_t, uint8_t, uint8_t>;

  Configuration();
  ~Configuration();

  Configuration(const Configuration &other);
  Configuration &operator=(const Configuration &other);

  Configuration(Configuration &&other) noexcept;
  Configuration &operator=(Configuration &&other) noexcept;

  /// @brief Loads a TOML configuration from the given file.
  /// @param filename Path to the `.toml` file.
  static Configuration LoadTOMLFile(std::string_view filename);

  /// @brief Loads a TOML configuration from a string.
  /// @param toml_string String representation of the TOML config.
  static Configuration LoadTOMLString(std::string_view toml_string);

  /// @brief Returns true if this configuration has no parameters set.
  bool Empty() const;

  /// @brief Returns true if all configuration keys and values match exactly.
  bool Equals(const Configuration &other) const;

  /// @brief Checks if the given key exists in this configuration.
  /// @param key Fully-qualified identifier of the parameter.
  bool Contains(std::string_view key) const;

  /// @brief Returns the type of the parameter at the given key.
  ///
  /// Throws an exception if the key is not found.
  ///
  /// @param key Fully-qualified identifier of the parameter.
  ConfigType Type(std::string_view key) const;

  /// @brief Returns a list of all (fully-qualified) parameter names.
  ///
  /// @param include_array_entries If true, the name of each parameter will
  /// be returned, *i.e.* each array element will be included. Otherwise,
  /// only named parameters (*e.g.* a dictionary/table within an array, such
  /// as `arr[3].name`) will be included.
  std::vector<std::string> ListParameterNames(bool include_array_entries) const;

  //---------------------------------------------------------------------------
  // Scalar data types

  bool GetBoolean(std::string_view key) const;
  bool GetBooleanOr(std::string_view key, bool default_val) const;
  std::optional<bool> GetOptionalBoolean(std::string_view key) const;
  void SetBoolean(std::string_view key, bool value);

  int32_t GetInteger32(std::string_view key) const;
  int32_t GetInteger32Or(std::string_view key, int32_t default_val) const;
  std::optional<int32_t> GetOptionalInteger32(std::string_view key) const;
  void SetInteger32(std::string_view key, int32_t value);

  int64_t GetInteger64(std::string_view key) const;
  int64_t GetInteger64Or(std::string_view key, int64_t default_val) const;
  std::optional<int64_t> GetOptionalInteger64(std::string_view key) const;
  void SetInteger64(std::string_view key, int64_t value);

  double GetDouble(std::string_view key) const;
  double GetDoubleOr(std::string_view key, double default_val) const;
  std::optional<double> GetOptionalDouble(std::string_view key) const;
  void SetDouble(std::string_view key, double value);

  std::string GetString(std::string_view key) const;
  std::string GetStringOr(std::string_view key,
                          std::string_view default_val) const;
  std::optional<std::string> GetOptionalString(std::string_view key) const;
  void SetString(std::string_view key, std::string_view value);

  //---------------------------------------------------------------------------
  // Date/time data types

  //  virtual date_type GetDate(std::string_view key) const = 0;
  //  virtual date_type GetDateOrDefault(std::string_view key,
  //                                     const date_type &default_val) const =
  //                                     0;
  //  virtual void SetDate(std::string_view key, const date_type &value) = 0;

  //---------------------------------------------------------------------------
  //  Lists/pairs of scalar data types

  std::pair<int32_t, int32_t> GetInteger32Pair(std::string_view key) const;
  std::vector<int32_t> GetInteger32List(std::string_view key) const;

  std::pair<int64_t, int64_t> GetInteger64Pair(std::string_view key) const;
  std::vector<int64_t> GetInteger64List(std::string_view key) const;

  std::pair<double, double> GetDoublePair(std::string_view key) const;
  std::vector<double> GetDoubleList(std::string_view key) const;

  std::vector<std::string> GetStringList(std::string_view key) const;

  std::vector<std::tuple<int32_t, int32_t>> GetPoints2D(
      std::string_view key) const;

  std::vector<std::tuple<int32_t, int32_t, int32_t>> GetPoints3D(
      std::string_view key) const;

  /// @brief Returns a copy of the sub-group.
  /// @param key Fully-qualified name of the parameter (which must be a
  ///   group, e.g. a JSON dictionary, a TOML table, or a libconfig group).
  Configuration GetGroup(std::string_view key) const;

  /// @brief Inserts (or replaces) the given configuration group.
  ///
  /// If the `key` already exists, it must be a group. Otherwise, the
  /// parameter will be newly created, along with all "parent" in the
  /// fully-qualified name (which defines a "path" through the configuration
  /// table/tree).
  ///
  /// @param key Fully-qualified name of the parameter. If it exists, it must
  ///   already be a group. The empty string is not allowed. To replace the
  ///   "root", create a new `Configuration` instance instead or use the
  ///   overloaded assignment operators.
  /// @param group The group to be inserted.
  void SetGroup(std::string_view key, const Configuration &group);

  //---------------------------------------------------------------------------
  // Special utilities

  /// @brief Adjusts the given parameters to hold either an absolute file path,
  /// or the result of "base_path / <param>" if they initially held a relative
  /// file path.
  /// @param base_path Base path to be prepended to relative file paths.
  /// @param parameters A list of parameter names / patterns. The wildcard '*'
  /// is also supported. For example, valid names are: "my-param",
  /// "files.video1", etc. Valid patterns would be "*path",
  /// "some.nested.*.filename", etc.
  /// @return True if any parameter has been adjusted.
  bool AdjustRelativePaths(std::string_view base_path,
                           const std::vector<std::string_view> &parameters);

  /// @brief Visits all string parameters and replaces any occurrence of the
  /// given needle/replacement pairs.
  /// @param replacements List of `<search, replacement>` pairs.
  /// @return True if any placeholder has actually been replaced.
  bool ReplaceStringPlaceholders(
      const std::vector<std::pair<std::string_view, std::string_view>>
          &replacements);

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
  void LoadNestedTOMLConfiguration(std::string_view key);

  /// @brief Returns a TOML-formatted string of this configuration.
  std::string ToTOML() const;

  /// @brief Returns a JSON-formatted string of this configuration.
  std::string ToJSON() const;

 private:
  struct Impl;
  std::unique_ptr<Impl> pimpl_;
};

//-----------------------------------------------------------------------------
// Key (parameter name) matching to support access via wildcards

class WERKZEUGKISTE_CONFIG_EXPORT KeyMatcher {
 public:
  KeyMatcher();
  KeyMatcher(std::initializer_list<std::string_view> keys);
  explicit KeyMatcher(const std::vector<std::string_view> &keys);

  ~KeyMatcher();

  KeyMatcher(const KeyMatcher &other);
  KeyMatcher &operator=(const KeyMatcher &other);

  KeyMatcher(KeyMatcher &&other) noexcept;
  KeyMatcher &operator=(KeyMatcher &&other) noexcept;

  void RegisterKey(std::string_view key);

  bool Match(std::string_view query) const;

  bool Empty() const;

 private:
  struct Impl;
  std::unique_ptr<Impl> pimpl_;
};

}  // namespace werkzeugkiste::config

#endif  // WERKZEUGKISTE_CONFIG_CONFIGURATION_H
