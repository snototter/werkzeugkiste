#ifndef WERKZEUGKISTE_CONFIG_CONFIGURATION_H
#define WERKZEUGKISTE_CONFIG_CONFIGURATION_H

#include <werkzeugkiste/config/config_export.h>
#include <werkzeugkiste/config/keymatcher.h>
#include <werkzeugkiste/config/types.h>

#include <cmath>
#include <initializer_list>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

/// @brief Utilities to handle configurations.
namespace werkzeugkiste::config {

/// @brief Encapsulates configuration data.
///
/// This class provides access to configuration data in a format compatible
/// to TOML.
/// Supported data types:
/// * Booleans
/// * Integer (internally stored as 64-bit signed integer).
/// * Floating point numbers (internally stored as double precision values).
/// * Strings
/// * Dates, times and date-times following RFC 3339,
///   https://www.rfc-editor.org/rfc/rfc3339
/// * Lists of configuration parameters (may be of any type).
/// * Groups, aka tables (TOML), i.e. a collection of named parameters. Similar
///   to objects in JSON, dicts (with keys of type `str`) in Python, groups in
///   `libconfig`.
///
/// TODO clean up doc / extend by examples:
/// * Explicit method names are preferred over a templated "Get<>".
/// * Get("unknown-key") throws a KeyError if the parameter does not exist.
/// * GetOptional returns an optional scalar.
/// * Get..Or returns a default value if the parameter does not exist.
/// * Mixed lists:
///   - All entries numeric -> can be looked up as floating point list.
///   - Can be created programmatically via CreateList + Append, as well as by
///     loading a mixed list from a TOML/JSON/... string or file
///   - Can be replaced by an empty list, but cannot be replaced by a
///     homogeneous list.
///   - Individual elements can be looked up and set (via their corresponding
///     GetTYPE/SetTYPE).
/// * Integers & floating points will be implicitly converted if the value can
///   be represented *exactly* by the target type. For example, double(2.0)
///   can be looked up as int, but 2.5 cannot (will raise a `TypeError`).
class WERKZEUGKISTE_CONFIG_EXPORT Configuration {
 public:
  /// @brief Constructs an empty configuration.
  Configuration();

  /// @brief Destructor.
  ~Configuration();

  /// @brief Copy constructor - creates a deep copy.
  Configuration(const Configuration &other);

  /// @brief Copy assignment - creates a deep copy.
  Configuration &operator=(const Configuration &other);

  /// @brief Move constructor.
  Configuration(Configuration &&other) noexcept;

  /// @brief Move assignment.
  Configuration &operator=(Configuration &&other) noexcept;

  /// @brief Loads a TOML configuration from a string.
  /// @param toml_string String representation of the TOML config.
  static Configuration LoadTOMLString(std::string_view toml_string);

  /// @brief Loads a TOML configuration from the given file.
  /// @param filename Path to the `.toml` file.
  static Configuration LoadTOMLFile(std::string_view filename);

  /// @brief Returns true if this configuration has no parameters set.
  bool Empty() const;

  /// @brief Returns true if all configuration keys and values match exactly.
  bool Equals(const Configuration &other) const;

  /// @brief Returns true if all configuration keys and values match exactly.
  bool operator==(const Configuration &other) const;

  /// @brief Returns true if any configuration key or value differs.
  bool operator!=(const Configuration &other) const;

  /// @brief Checks if the given key exists in this configuration.
  /// @param key Fully-qualified identifier of the parameter.
  bool Contains(std::string_view key) const;

  /// @brief Returns the length of the list `key`.
  ///
  /// Raises a `KeyError` if the parameter does not exist.
  /// Raises a `TypeError` if the parameter is not a list or a group.
  ///
  /// @param key Fully-qualified identifier of the parameter.
  std::size_t Size(std::string_view key) const;

  /// @brief Returns the number of parameters (key-value pairs) in this
  /// configuration.
  inline std::size_t Size() const { return Size(""); }

  /// @brief Returns the type of the parameter at the given key.
  ///
  /// Raises a `KeyError` if the parameter does not exist.
  ///
  /// @param key Fully-qualified identifier of the parameter.
  ConfigType Type(std::string_view key) const;

  /// @brief Deletes the parameter with the given key.
  ///
  /// Can be used to delete a scalar, list or (sub-)group of the configuration.
  /// Cannot be used to delete a specific element of a list (`arr[0]`). For the
  /// latter, you need to delete (then recreate) the whole list.
  ///
  /// Raises a `KeyError` if the parameter does not exist or refers to an
  /// element of a list.
  ///
  /// @param key Fully-qualified parameter name.
  void Delete(std::string_view key);

  /// @brief Checks if a list parameter contains only scalars of the same type.
  ///
  /// Raises a `KeyError` if the parameter does not exist.
  /// Raises a `TypeError` if the parameter is not a list.
  ///
  /// @param key Fully-qualified name of the parameter.
  /// @return True if the list is empty or contains only scalars of the same
  ///   type. False otherwise.
  bool IsHomogeneousScalarList(std::string_view key) const;

  /// @brief Returns a list of (fully-qualified) parameter names below the
  ///   given key.
  ///
  /// @param key Fully-qualified name of the parameter.
  /// @param include_array_entries If true, the name of each parameter will
  ///   be returned, *i.e.* each array element will be included. Otherwise,
  ///   only named parameters (*e.g.* a dictionary/table within an array, such
  ///   as `arr[3].name`) will be included.
  /// @param recursive If true, the names of all parameters "below" this
  ///   configuration/group will be returned. Otherwise, only the first-level
  ///   child parameters will be returned.
  std::vector<std::string> ListParameterNames(std::string_view key,
      bool include_array_entries,
      bool recursive) const;

  /// @brief Returns a list of (fully-qualified) parameter names below the
  ///   configuration root.
  ///
  /// @param include_array_entries If true, the name of each parameter will
  /// be returned, *i.e.* each array element will be included. Otherwise,
  /// only named parameters (*e.g.* a dictionary/table within an array, such
  /// as `arr[3].name`) will be included.
  /// @param recursive If true, the names of all parameters "below" this
  /// configuration/group will be returned. Otherwise, only the first-level
  /// child parameters will be returned.
  inline std::vector<std::string> ListParameterNames(bool include_array_entries,
      bool recursive) const {
    using namespace std::string_view_literals;
    return ListParameterNames(""sv, include_array_entries, recursive);
  };

  //---------------------------------------------------------------------------
  // Booleans

  /// @brief Returns the boolean parameter.
  ///
  /// Raises a `KeyError` if the parameter does not exist.
  /// Raises a `TypeError` if the parameter is of a different type.
  ///
  /// @param key Fully-qualified parameter name.
  bool GetBoolean(std::string_view key) const;

  /// @brief Returns the boolean parameter or the `default_val` if it does not
  ///   exist.
  ///
  /// Raises a `TypeError` if the parameter exists but is of a different type.
  ///
  /// @param key Fully-qualified parameter name.
  /// @param default_val Value to return if the parameter does not exist.
  bool GetBooleanOr(std::string_view key, bool default_val) const;

  /// @brief Returns an optional boolean or `std::nullopt` if it does not
  ///   exist.
  ///
  /// Raises a `TypeError` if the parameter exists but is of a different type.
  ///
  /// @param key Fully-qualified parameter name.
  std::optional<bool> GetOptionalBoolean(std::string_view key) const;

  /// @brief Sets a boolean parameter.
  ///
  /// Raises a `TypeError` if the parameter exists and is of a different type.
  /// Raises a `std::logic_error` if setting the value in the underlying TOML
  ///   library failed for unforeseen/not handled reasons.
  ///
  /// @param key Fully-qualified parameter name.
  /// @param value The value to be set.
  void SetBoolean(std::string_view key, bool value);

  /// @brief Returns a list of boolean flags.
  ///
  /// Raises a `KeyError` if the parameter does not exist.
  /// Raises a `TypeError` if the parameter is of a different type.
  ///
  /// @param key Fully-qualified parameter name.
  std::vector<bool> GetBooleanList(std::string_view key) const;

  /// @brief Sets or replaces a list of boolean flags.
  ///
  /// Raises a `TypeError` if the parameter exists but is of a different type.
  ///
  /// @param key Fully-qualified parameter name.
  /// @param values List of flags.
  void SetBooleanList(std::string_view key, const std::vector<bool> &values);

  //---------------------------------------------------------------------------
  // Integers (32-bit)

  /// @brief Returns the 32-bit integer parameter.
  ///
  /// Raises a `KeyError` if the parameter does not exist.
  /// Raises a `TypeError` if the parameter is of a different type, unless it
  /// can be safely cast (e.g. double(2.0) can be exactly represented by a
  /// 32-bit integer, whereas double(1.5) cannot).
  ///
  /// @param key Fully-qualified parameter name.
  int32_t GetInteger32(std::string_view key) const;

  /// @brief Returns the 32-bit integer parameter or the default value.
  ///
  /// Raises a `TypeError` if the parameter is of a different type, unless it
  /// can be safely cast (e.g. double(2.0) can be exactly represented by a
  /// 32-bit integer, whereas double(1.5) cannot).
  ///
  /// @param key Fully-qualified parameter name.
  /// @param default_val Value to be returned if the parameter does not exist.
  int32_t GetInteger32Or(std::string_view key, int32_t default_val) const;

  /// @brief Returns the 32-bit integer parameter or `std::nullopt` if it does
  ///   not exist.
  ///
  /// Raises a `TypeError` if the parameter is of a different type, unless it
  /// can be safely cast (e.g. double(2.0) can be exactly represented by a
  /// 32-bit integer, whereas double(1.5) cannot).
  ///
  /// @param key Fully-qualified parameter name.
  std::optional<int32_t> GetOptionalInteger32(std::string_view key) const;

  /// @brief Sets a 32-bit signed integer parameter.
  ///
  /// Raises a `TypeError` if the parameter exists and is of a different type,
  ///   unless the value is exactly representable by the existing type. For
  ///   example, an integer value can usually be exactly represented as a
  ///   floating point number, thus `SetInteger32("my-float"sv, 2)` will not
  ///   raise an exception.
  /// Raises a `std::logic_error` if setting the value in the underlying TOML
  ///   library failed for unforeseen/not handled reasons.
  ///
  /// @param key Fully-qualified parameter name.
  /// @param value The value to be set.
  void SetInteger32(std::string_view key, int32_t value);

  /// @brief Returns a list of 32-bit integers.
  ///
  /// Raises a `KeyError` if the parameter does not exist.
  /// Raises a `TypeError` if the parameter is of a different type, unless it
  /// can be safely cast. For example, a list like [0.0, -2.0, 100.0, 12345.0]
  /// can be exactly represented by a list of 32-bit integer, whereas
  /// [0.0, 1.5] cannot.
  ///
  /// @param key Fully-qualified parameter name.
  std::vector<int32_t> GetInteger32List(std::string_view key) const;

  /// @brief Sets or replaces a list of 32-bit integers.
  ///
  /// Raises a `TypeError` if the parameter exists but is of a different type.
  ///   If the parameter is a mixed numeric list, however, the values will be
  ///   cast to the corresponding data type. For example, replacing an existing
  ///   `[int, double, int]` list by `[1, 2, 3, 4]` results in a parameter
  ///   list `[int(1), double(2.0), int(3), int(4)]`.
  ///
  /// @param key Fully-qualified parameter name.
  /// @param values List of flags.
  void SetInteger32List(std::string_view key,
      const std::vector<int32_t> &values);

  //---------------------------------------------------------------------------
  // Integers (64-bit)

  /// @brief Returns the 64-bit integer parameter.
  ///
  /// Raises a `KeyError` if the parameter does not exist.
  /// Raises a `TypeError` if the parameter is of a different type, unless it
  /// can be safely cast (e.g. double(2.0) can be exactly represented by a
  /// 64-bit integer, whereas double(1.5) cannot).
  ///
  /// @param key Fully-qualified parameter name.
  int64_t GetInteger64(std::string_view key) const;

  /// @brief Returns the 64-bit integer parameter or the default value.
  ///
  /// Raises a `TypeError` if the parameter is of a different type, unless it
  /// can be safely cast (e.g. double(2.0) can be exactly represented by a
  /// 64-bit integer, whereas double(1.5) cannot).
  ///
  /// @param key Fully-qualified parameter name.
  /// @param default_val Value to be returned if the parameter does not exist.
  int64_t GetInteger64Or(std::string_view key, int64_t default_val) const;

  /// @brief Returns the 64-bit integer parameter or `std::nullopt` if it does
  ///   not exist.
  ///
  /// Raises a `TypeError` if the parameter is of a different type, unless it
  ///   can be safely cast. For example, double(2.0) can be exactly represented
  ///   by a 64-bit integer, whereas double(1.5) cannot.
  ///
  /// @param key Fully-qualified parameter name.
  std::optional<int64_t> GetOptionalInteger64(std::string_view key) const;

  /// @brief Sets a 64-bit signed integer parameter.
  ///
  /// Raises a `TypeError` if the parameter exists and is of a different type,
  ///   unless the value is exactly representable by the existing type. For
  ///   example, an integer value can usually be exactly represented as a
  ///   floating point number, thus `SetInteger64("my-float"sv, 2)` will not
  ///   raise an exception.
  /// Raises a `std::logic_error` if setting the value in the underlying TOML
  ///   library failed for unforeseen/not handled reasons.
  ///
  /// @param key Fully-qualified parameter name.
  /// @param value The value to be set.
  void SetInteger64(std::string_view key, int64_t value);

  /// @brief Returns a list of 64-bit integers.
  ///
  /// Raises a `KeyError` if the parameter does not exist.
  /// Raises a `TypeError` if the parameter is of a different type, unless it
  /// can be safely cast. For example, a list like [0.0, -2.0, 100.0, 12345.0]
  /// can be exactly represented by a list of 64-bit integer, whereas
  /// [0.0, 1.5] cannot.
  ///
  /// @param key Fully-qualified parameter name.
  std::vector<int64_t> GetInteger64List(std::string_view key) const;

  /// @brief Sets or replaces a list of 64-bit integers.
  ///
  /// Raises a `TypeError` if the parameter exists but is of a different type.
  ///   If the parameter is a mixed numeric list, however, the values will be
  ///   cast to the corresponding data type. For example, replacing an existing
  ///   `[int, double, int]` list by `[1, 2, 3, 4]` results in a parameter
  ///   list `[int(1), double(2.0), int(3), int(4)]`.
  ///
  /// @param key Fully-qualified parameter name.
  /// @param values List of flags.
  void SetInteger64List(std::string_view key,
      const std::vector<int64_t> &values);

  /// @brief Returns a 2D point with integer coordinates.
  ///
  /// Interprets a list of numbers as 2D point. If the list contains more than
  /// two elements, only the first two entries are loaded as x and y
  /// coordinate, respectively.
  /// Similarly, a group which holds (at least) `x` and `y` parameters can also
  /// be loaded as a 2D point.
  ///
  /// Raises a `KeyError` if the parameter does not exist.
  /// Raises a `TypeError` if the parameter cannot be converted to a 2D point.
  ///
  /// @param key Fully-qualified parameter name
  point2d<int64_t> GetInteger64Point2D(std::string_view key) const;
  // TODO doc / Get...Or / GetOptional

  /// @brief Returns a 3D point with integer coordinates.
  ///
  /// Interprets a list of numbers as 3D point. If the list contains more than
  /// three elements, only the first three entries are loaded as x, y and z
  /// coordinate, respectively.
  /// Similarly, a group which holds (at least) `x`, `y` and `z` parameters can
  /// also be loaded as a 3D point.
  ///
  /// Raises a `KeyError` if the parameter does not exist.
  /// Raises a `TypeError` if the parameter cannot be converted to a 2D point.
  ///
  /// @param key Fully-qualified parameter name
  point3d<int64_t> GetInteger64Point3D(std::string_view key) const;
  // TODO doc / Get...Or / GetOptional

  /// @brief Returns a list of 2D points (e.g. a polyline or polygon).
  ///
  /// Supports loading nested lists and lists of {x, y} tables as a
  /// list of 2D points. Each point in the configuration must have at least
  /// 2 dimensions, but may also have more (i.e. only loading the x/y
  /// components of 3D points is allowed).
  ///
  /// Raises a `KeyError` if the parameter does not exist.
  /// Raises a `TypeError` if any coordinate is defined as a different type,
  /// unless it can be safely cast.
  ///
  /// @param key Fully-qualified parameter name.
  std::vector<point2d<int64_t>> GetInteger64Points2D(
      std::string_view key) const;

  /// @brief Returns a list of 3D points (e.g. a polyline or polygon).
  ///
  /// Supports loading nested lists and lists of {x, y} tables as a
  /// list of 3D points. Each point in the configuration must have at least
  /// 3 dimensions, but may also have more (i.e. only loading the first 3
  /// components of n-dim points is allowed).
  ///
  /// Raises a `KeyError` if the parameter does not exist.
  /// Raises a `TypeError` if any coordinate is defined as a different type,
  /// unless it can be safely cast.
  ///
  /// @param key Fully-qualified parameter name.
  std::vector<point3d<int64_t>> GetInteger64Points3D(
      std::string_view key) const;

  //---------------------------------------------------------------------------
  // Floating Point

  /// @brief Returns the double-precision floating point parameter.
  ///
  /// Raises a `KeyError` if the parameter does not exist.
  /// Raises a `TypeError` if the parameter is of a different type, unless it
  /// can be safely cast (e.g. integer values can usually be exactly
  /// represented by a double).
  ///
  /// @param key Fully-qualified parameter name.
  double GetDouble(std::string_view key) const;

  /// @brief Returns the double-precision floating point parameter or the
  ///   default value.
  ///
  /// Raises a `TypeError` if the parameter is of a different type, unless it
  /// can be safely cast (e.g. integer values can usually be exactly
  /// represented by a double).
  ///
  /// @param key Fully-qualified parameter name.
  /// @param default_val Value to be returned if the parameter does not exist.
  double GetDoubleOr(std::string_view key, double default_val) const;

  /// @brief Returns the double-precision floating point parameter or
  ///   `std::nullopt` if it does not exist.
  ///
  /// Raises a `TypeError` if the parameter is of a different type, unless it
  ///   can be safely cast. For example, integer values can usually be exactly
  ///   represented by a double.
  ///
  /// @param key Fully-qualified parameter name.
  std::optional<double> GetOptionalDouble(std::string_view key) const;

  /// @brief Sets a double-precision floating point parameter.
  ///
  /// Raises a `TypeError` if the parameter exists and is of a different type,
  ///   unless the value is exactly representable by the existing type. For
  ///   example, `SetDouble("my-integer"sv, 12.0)` will not raise an exception.
  ///   However, the parameter type of `my-integer` would still be `integer`.
  /// Raises a `std::logic_error` if setting the value in the underlying TOML
  ///   library failed for unforeseen/not handled reasons.
  ///
  /// @param key Fully-qualified parameter name.
  /// @param value The value to be set.
  void SetDouble(std::string_view key, double value);

  /// @brief Returns a list of double-precision floating point values.
  ///
  /// Raises a `KeyError` if the parameter does not exist.
  /// Raises a `TypeError` if the parameter is of a different type, unless it
  /// can be safely cast (e.g. integer values can usually be exactly
  /// represented by a double).
  ///
  /// @param key Fully-qualified parameter name.
  std::vector<double> GetDoubleList(std::string_view key) const;

  /// @brief Sets or replaces a list of double-precision floating point values.
  ///
  /// Raises a `TypeError` if the parameter exists but is of a different type.
  ///   If the parameter is a mixed numeric list, however, this call can
  ///   succeed if all values can be exactly represented by the corresponding
  ///   data type. For example, replacing an existing `[int, double, int]`
  ///   list by `[1.0, 2.3, 3e3, 4.5]` results in a parameter list
  ///   `[int(1), double(2.3), int(3000), double(4.5)]`.
  ///
  /// @param key Fully-qualified parameter name.
  /// @param values List of flags.
  void SetDoubleList(std::string_view key, const std::vector<double> &values);

  /// @brief Returns a 2D point with floating point coordinates.
  ///
  /// Interprets a list of numbers as 2D point. If the list contains more than
  /// two elements, only the first two entries are loaded as x and y
  /// coordinate, respectively.
  /// Similarly, a group which holds (at least) `x` and `y` parameters can also
  /// be loaded as a 2D point.
  ///
  /// Raises a `KeyError` if the parameter does not exist.
  /// Raises a `TypeError` if the parameter cannot be converted to a 2D point.
  ///
  /// @param key Fully-qualified parameter name
  point2d<double> GetDoublePoint2D(std::string_view key) const;
  // TODO doc / Get...Or / GetOptional

  /// @brief Returns a 3D point with floating point coordinates.
  ///
  /// Interprets a list of numbers as 3D point. If the list contains more than
  /// three elements, only the first three entries are loaded as x, y and z
  /// coordinate, respectively.
  /// Similarly, a group which holds (at least) `x`, `y` and `z` parameters can
  /// also be loaded as a 3D point.
  ///
  /// Raises a `KeyError` if the parameter does not exist.
  /// Raises a `TypeError` if the parameter cannot be converted to a 2D point.
  ///
  /// @param key Fully-qualified parameter name
  point3d<double> GetDoublePoint3D(std::string_view key) const;
  // TODO doc / Get...Or / GetOptional

  /// @brief Returns a list of 2D points (e.g. a polyline or polygon).
  ///
  /// Supports loading nested lists and lists of {x, y} tables as a
  /// list of 2D points. Each point in the configuration must have at least
  /// 2 dimensions, but may also have more (i.e. only loading the x/y
  /// components of 3D points is allowed).
  ///
  /// Raises a `KeyError` if the parameter does not exist.
  /// Raises a `TypeError` if any coordinate is defined as a different type,
  /// unless it can be safely cast.
  ///
  /// @param key Fully-qualified parameter name.
  std::vector<point2d<double>> GetDoublePoints2D(std::string_view key) const;

  /// @brief Returns a list of 3D points (e.g. a polyline or polygon).
  ///
  /// Supports loading nested lists and lists of {x, y} tables as a
  /// list of 3D points. Each point in the configuration must have at least
  /// 3 dimensions, but may also have more (i.e. only loading the x/y/z
  /// components of n-dim points is allowed).
  ///
  /// Raises a `KeyError` if the parameter does not exist.
  /// Raises a `TypeError` if any coordinate is defined as a different type,
  /// unless it can be safely cast.
  ///
  /// @param key Fully-qualified parameter name.
  std::vector<point3d<double>> GetDoublePoints3D(std::string_view key) const;

  //---------------------------------------------------------------------------
  // Strings

  /// @brief Returns the string parameter.
  ///
  /// Raises a `KeyError` if the parameter does not exist.
  /// Raises a `TypeError` if the parameter is of a different type.
  ///
  /// @param key Fully-qualified parameter name.
  std::string GetString(std::string_view key) const;

  /// @brief Returns the string parameter or the `default_val` if it does not
  ///   exist.
  ///
  /// Raises a `TypeError` if the parameter exists but is of a different type.
  ///
  /// @param key Fully-qualified parameter name.
  /// @param default_val Value to return if the parameter does not exist.
  std::string GetStringOr(std::string_view key,
      std::string_view default_val) const;

  /// @brief Returns an optional string or `std::nullopt` if it does not exist.
  ///
  /// Raises a `TypeError` if the parameter exists but is of a different type.
  ///
  /// @param key Fully-qualified parameter name.
  std::optional<std::string> GetOptionalString(std::string_view key) const;

  /// @brief Sets a string parameter.
  ///
  /// Raises a `TypeError` if the parameter exists and is of a different type.
  /// Raises a `std::logic_error` if setting the value in the underlying TOML
  ///   library failed for unforeseen/not handled reasons.
  ///
  /// @param key Fully-qualified parameter name.
  /// @param value The value to be set.
  void SetString(std::string_view key, std::string_view value);

  /// @brief Returns a list of strings.
  ///
  /// Raises a `KeyError` if the parameter does not exist.
  /// Raises a `TypeError` if the parameter is of a different type.
  ///
  /// @param key Fully-qualified parameter name.
  std::vector<std::string> GetStringList(std::string_view key) const;

  /// @brief Creates or replaces a parameter holding a list of strings.
  ///
  /// Raises a `TypeError` if the parameter exists but is of a different
  ///   type, *i.e.* this method can *not* be used to change the type of
  ///   an existing parameter.
  ///
  /// @param key Fully-qualified parameter name.
  /// @param values List of strings to be set.
  void SetStringList(std::string_view key,
      const std::vector<std::string_view> &values);

  //---------------------------------------------------------------------------
  // Date

  /// @brief Returns the date parameter.
  ///
  /// Raises a `KeyError` if the parameter does not exist.
  /// Raises a `TypeError` if the parameter is of a different type.
  ///
  /// @param key Fully-qualified parameter name.
  date GetDate(std::string_view key) const;

  /// @brief Returns the date parameter or the `default_val` if it does not
  ///   exist.
  ///
  /// Raises a `TypeError` if the parameter exists but is of a different type.
  ///
  /// @param key Fully-qualified parameter name.
  /// @param default_val Value to return if the parameter does not exist.
  date GetDateOr(std::string_view key, const date &default_val) const;

  /// @brief Returns an optional date or `std::nullopt` if it does not
  ///   exist.
  ///
  /// Raises a `TypeError` if the parameter exists but is of a different type.
  ///
  /// @param key Fully-qualified parameter name.
  std::optional<date> GetOptionalDate(std::string_view key) const;

  /// @brief Sets a local date parameter.
  ///
  /// Raises a `TypeError` if the parameter exists and is of a different type.
  /// Raises a `std::logic_error` if setting the value in the underlying TOML
  ///   library failed for unforeseen/not handled reasons.
  ///
  /// @param key Fully-qualified parameter name.
  /// @param value The value to be set.
  void SetDate(std::string_view key, const date &value);

  /// @brief Returns a list of date parameters.
  ///
  /// Raises a `KeyError` if the parameter does not exist.
  /// Raises a `TypeError` if the parameter is of a different type.
  ///
  /// @param key Fully-qualified parameter name.
  std::vector<date> GetDateList(std::string_view key) const;

  /// @brief Sets or replaces a list of date parameters.
  ///
  /// Raises a `TypeError` if the parameter exists but is of a different type.
  ///
  /// @param key Fully-qualified parameter name.
  /// @param values List of dates.
  void SetDateList(std::string_view key, const std::vector<date> &values);

  //---------------------------------------------------------------------------
  // Time

  /// @brief Returns the time parameter.
  ///
  /// Raises a `KeyError` if the parameter does not exist.
  /// Raises a `TypeError` if the parameter is of a different type.
  ///
  /// @param key Fully-qualified parameter name.
  time GetTime(std::string_view key) const;

  /// @brief Returns the time parameter or the `default_val` if it does not
  ///   exist.
  ///
  /// Raises a `TypeError` if the parameter exists but is of a different type.
  ///
  /// @param key Fully-qualified parameter name.
  /// @param default_val Value to return if the parameter does not exist.
  time GetTimeOr(std::string_view key, const time &default_val) const;

  /// @brief Returns an optional time or `std::nullopt` if it does not
  ///   exist.
  ///
  /// Raises a `TypeError` if the parameter exists but is of a different type.
  ///
  /// @param key Fully-qualified parameter name.
  std::optional<time> GetOptionalTime(std::string_view key) const;

  /// @brief Sets a local time parameter.
  ///
  /// Raises a `TypeError` if the parameter exists and is of a different type.
  /// Raises a `std::logic_error` if setting the value in the underlying TOML
  ///   library failed for unforeseen/not handled reasons.
  ///
  /// @param key Fully-qualified parameter name.
  /// @param value The value to be set.
  void SetTime(std::string_view key, const time &value);

  /// @brief Returns a list of time parameters.
  ///
  /// Raises a `KeyError` if the parameter does not exist.
  /// Raises a `TypeError` if the parameter is of a different type.
  ///
  /// @param key Fully-qualified parameter name.
  std::vector<time> GetTimeList(std::string_view key) const;

  /// @brief Sets or replaces a list of time parameters.
  ///
  /// Raises a `TypeError` if the parameter exists but is of a different type.
  ///
  /// @param key Fully-qualified parameter name.
  /// @param values List of times.
  void SetTimeList(std::string_view key, const std::vector<time> &values);

  //---------------------------------------------------------------------------
  // Date-time

  /// @brief Returns the date-time parameter with optional timezone offset.
  ///
  /// Raises a `KeyError` if the parameter does not exist.
  /// Raises a `TypeError` if the parameter is of a different type.
  ///
  /// @param key Fully-qualified parameter name.
  date_time GetDateTime(std::string_view key) const;

  /// @brief Returns the date-time parameter or the `default_val` if it does
  ///   not exist.
  ///
  /// Raises a `TypeError` if the parameter exists but is of a different type.
  ///
  /// @param key Fully-qualified parameter name.
  /// @param default_val Value to return if the parameter does not exist.
  date_time GetDateTimeOr(std::string_view key,
      const date_time &default_val) const;

  /// @brief Returns an optional date_time or `std::nullopt` if it does not
  ///   exist.
  ///
  /// Raises a `TypeError` if the parameter exists but is of a different type.
  ///
  /// @param key Fully-qualified parameter name.
  std::optional<date_time> GetOptionalDateTime(std::string_view key) const;

  /// @brief Sets a date-time parameter.
  ///
  /// A date-time consists of a date, a time and an optional timezone offset,
  /// following RFC 3339, https://www.rfc-editor.org/rfc/rfc3339
  ///
  /// Raises a `TypeError` if the parameter exists and is of a different type.
  /// Raises a `std::logic_error` if setting the value in the underlying TOML
  ///   library failed for unforeseen/not handled reasons.
  ///
  /// @param key Fully-qualified parameter name.
  /// @param value The value to be set.
  void SetDateTime(std::string_view key, const date_time &value);

  /// @brief Returns a list of date-time parameters.
  ///
  /// Raises a `KeyError` if the parameter does not exist.
  /// Raises a `TypeError` if the parameter is of a different type.
  ///
  /// @param key Fully-qualified parameter name.
  std::vector<date_time> GetDateTimeList(std::string_view key) const;

  /// @brief Sets or replaces a list of date-time parameters.
  ///
  /// Raises a `TypeError` if the parameter exists but is of a different type.
  ///
  /// @param key Fully-qualified parameter name.
  /// @param values List of date-times.
  void SetDateTimeList(std::string_view key,
      const std::vector<date_time> &values);

  //---------------------------------------------------------------------------
  // Mixed list support

  /// @brief Creates an empty list with the given name.
  ///
  /// After successful creation, any values can be `Append`ed.
  ///
  /// Raises a `KeyError` if the key already exists or if a "parent" could
  /// not be created.
  ///
  /// @param key Fully-qualified name of the parameter.
  void CreateList(std::string_view key);

  /// @brief Clears an existing list.
  ///
  /// Afterwards, any values can be `Append`ed.
  ///
  /// Raises a `KeyError` if the parameter does not exist.
  /// Raises a `TypeError` if the parameter is not a list.
  ///
  /// @param key Fully-qualified name of the parameter.
  void ClearList(std::string_view key);

  /// @brief Appends an empty list to an existing list in order to supported
  ///   creating nested lists programmatically.
  ///
  /// Raises a `KeyError` if the key does not exist.
  /// Raises a `TypeError` if the key exists, but is not a list.
  ///
  /// @param key Fully-qualified name of the existing list parameter.
  void AppendList(std::string_view key);

  /// @brief Appends a boolean flag to an existing list.
  ///
  /// Raises a `KeyError` if the key does not exist.
  /// Raises a `TypeError` if the key exists, but is not a list.
  ///
  /// @param key Fully-qualified name of the existing list parameter.
  /// @param value Value to be appended.
  void Append(std::string_view key, bool value);

  /// @brief Appends a 32-bit integer to an existing list.
  ///
  /// Raises a `KeyError` if the key does not exist.
  /// Raises a `TypeError` if the key exists, but is not a list.
  ///
  /// @param key Fully-qualified name of the existing list parameter.
  /// @param value Value to be appended.
  void Append(std::string_view key, int32_t value);

  /// @brief Appends a 64-bit integer to an existing list.
  ///
  /// Raises a `KeyError` if the key does not exist.
  /// Raises a `TypeError` if the key exists, but is not a list.
  ///
  /// @param key Fully-qualified name of the existing list parameter.
  /// @param value Value to be appended.
  void Append(std::string_view key, int64_t value);

  /// @brief Appends a floating point value to an existing list.
  ///
  /// Raises a `KeyError` if the key does not exist.
  /// Raises a `TypeError` if the key exists, but is not a list.
  ///
  /// @param key Fully-qualified name of the existing list parameter.
  /// @param value Value to be appended.
  void Append(std::string_view key, double value);

  /// @brief Appends a string to an existing list.
  ///
  /// Raises a `KeyError` if the key does not exist.
  /// Raises a `TypeError` if the key exists, but is not a list.
  ///
  /// @param key Fully-qualified name of the existing list parameter.
  /// @param value Value to be appended.
  void Append(std::string_view key, std::string_view value);

  /// @brief Appends a date to an existing list.
  ///
  /// Raises a `KeyError` if the key does not exist.
  /// Raises a `TypeError` if the key exists, but is not a list.
  ///
  /// @param key Fully-qualified name of the existing list parameter.
  /// @param value Value to be appended.
  void Append(std::string_view key, const date &value);

  /// @brief Appends a local time to an existing list.
  ///
  /// Raises a `KeyError` if the key does not exist.
  /// Raises a `TypeError` if the key exists, but is not a list.
  ///
  /// @param key Fully-qualified name of the existing list parameter.
  /// @param value Value to be appended.
  void Append(std::string_view key, const time &value);

  /// @brief Appends a date-time to an existing list.
  ///
  /// Raises a `KeyError` if the key does not exist.
  /// Raises a `TypeError` if the key exists, but is not a list.
  ///
  /// @param key Fully-qualified name of the existing list parameter.
  /// @param value Value to be appended.
  void Append(std::string_view key, const date_time &value);

  /// @brief Appends a group/sub-configuration to an existing list.
  ///
  /// Raises a `KeyError` if the key does not exist.
  /// Raises a `TypeError` if the key exists, but is not a list.
  ///
  /// @param key Fully-qualified name of the existing list parameter.
  /// @param group Group/"Sub-Configuration" to be appended.
  void Append(std::string_view key, const Configuration &group);

  //---------------------------------------------------------------------------
  // Group/"Sub-Configuration"

  /// @brief Returns a copy of the sub-group.
  /// @param key Fully-qualified name of the parameter (which must be a
  ///   group, e.g. a JSON dictionary, a TOML table, or a libconfig group).
  Configuration GetGroup(std::string_view key) const;

  /// @brief Inserts (or replaces) the given configuration group.
  ///
  /// If the `key` already exists, it must be a group. Otherwise, the
  /// parameter will be newly created, along with all "parents" in the
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
  // Convenience utilities

  /// @brief Adjusts the given parameters below the `key` group to hold either
  ///   an absolute file path, or the result of "base_path / <param>" if they
  ///   initially held a relative file path.
  /// @param key Fully-qualified parameter name.
  /// @param base_path Base path to be prepended to relative file paths.
  /// @param parameters A list of parameter names / patterns. The wildcard '*'
  ///   is also supported. For example, valid names are: "my-param",
  ///   "files.video1", etc. Valid patterns would be "*path",
  ///   "some.nested.*.filename", etc. Parameters that match the pattern, but
  ///   are not strings will be skipped.
  /// @return True if any parameter has been adjusted.
  bool AdjustRelativePaths(std::string_view key,
      std::string_view base_path,
      const std::vector<std::string_view> &parameters);

  /// @brief Adjusts the given parameters below the configuration root to hold
  ///   either an absolute file path, or the result of "base_path / <param>" if
  ///   they initially held a relative file path.
  /// @param base_path Base path to be prepended to relative file paths.
  /// @param parameters A list of parameter names / patterns. The wildcard '*'
  ///   is also supported. For example, valid names are: "my-param",
  ///   "files.video1", etc. Valid patterns would be "*path",
  ///   "some.nested.*.filename", etc. Parameters that match the pattern, but
  ///   are not strings will be skipped.
  /// @return True if any parameter has been adjusted.
  inline bool AdjustRelativePaths(std::string_view base_path,
      const std::vector<std::string_view> &parameters) {
    using namespace std::string_view_literals;
    return AdjustRelativePaths(""sv, base_path, parameters);
  }

  /// @brief Visits all string parameters below the given `key` group and
  ///   replaces any occurrence of the given needle/replacement pairs.
  /// @param key Fully-qualified parameter name.
  /// @param replacements List of `<search, replacement>` pairs.
  /// @return True if any placeholder has actually been replaced.
  bool ReplaceStringPlaceholders(std::string_view key,
      const std::vector<std::pair<std::string_view, std::string_view>>
          &replacements);

  /// @brief Visits all string parameters below the root configuration and
  ///   replaces any occurrence of the given needle/replacement pairs.
  /// @param replacements List of `<search, replacement>` pairs.
  /// @return True if any placeholder has actually been replaced.
  inline bool ReplaceStringPlaceholders(
      const std::vector<std::pair<std::string_view, std::string_view>>
          &replacements) {
    using namespace std::string_view_literals;
    return ReplaceStringPlaceholders(""sv, replacements);
  }

  /// @brief Loads a nested configuration.
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
  /// This method deduces the type of the configuration from the file
  /// extension, similar to `LoadFile`.
  ///
  /// Raises a `KeyError` if the parameter does not exist.
  /// Raises a `TypeError` if the parameter is not a string.
  /// Raises a `ParseError` if parsing the external configuration failed.
  /// Raises a `std::runtime_error` if replacing the internal configuration
  ///   failed for unforeseen reasons.
  ///
  /// @param key Parameter name (fully-qualified TOML path) which holds the
  ///     file name of the nested configuration file. The file name must be
  ///     given as string.
  void LoadNestedConfiguration(std::string_view key);

  //---------------------------------------------------------------------------
  // Serialization

  /// @brief Returns a TOML-formatted string of this configuration.
  std::string ToTOML() const;

  /// @brief Returns a JSON-formatted string of this configuration.
  std::string ToJSON() const;

  /// @brief Returns a YAML-formatted string of this configuration.
  std::string ToYAML() const;

  /// @brief Returns a libconfig-formatted string of this configuration.
  std::string ToLibconfig() const;

 private:
  /// Forward declaration of internal implementation struct.
  struct Impl;

  /// Pointer to internal implementation.
  std::unique_ptr<Impl> pimpl_;
};

/// @brief Loads a configuration file.
///
/// The configuration type will be deduced from the file extension, *i.e.*
/// `.toml`, `.json`, or `.cfg`.
/// For JSON files, the default `NullValuePolicy` will be used, see
/// `LoadJSONFile`.
///
/// @param filename Path to the configuration file.
Configuration LoadFile(std::string_view filename);

/// @brief Loads a TOML configuration from the given file.
/// @param filename Path to the `.toml` file.
inline Configuration LoadTOMLFile(std::string_view filename) {
  return Configuration::LoadTOMLFile(filename);
}

/// @brief Loads a TOML configuration from a string.
/// @param toml_string String representation of the TOML config.
inline Configuration LoadTOMLString(std::string_view toml_string) {
  return Configuration::LoadTOMLString(toml_string);
}

/// @brief Loads a libconfig configuration from the given file.
/// @param filename Path to the `.cfg` file.
WERKZEUGKISTE_CONFIG_EXPORT
Configuration LoadLibconfigFile(std::string_view filename);

/// @brief Loads a libconfig configuration from a string.
/// @param lcfg_string String representation of the libconfig configuration.
WERKZEUGKISTE_CONFIG_EXPORT
Configuration LoadLibconfigString(std::string_view lcfg_string);

/// @brief How to handle Null/None values (e.g. when loading JSON).
enum class NullValuePolicy : unsigned char {
  /// @brief Null values will be skipped, i.e. not loaded into the
  ///   configuration.
  Skip,

  /// @brief Null values will be **replaced** by the string "null".
  NullString
};

/// @brief Loads a JSON configuration from a string.
///
/// Because a configuration must consist of key/value pairs, a plain JSON
/// array (e.g. "[1, 2, 3]") will be loaded into the key `json`. Thus, the
/// configuration would have 1 element, and you need to access it via its key.
/// For example, cfg.Size("json"), cfg.GetDouble("json[0]"), etc.
///
/// @param filename Path to the `.json` file.
/// @param none_policy How to deal with None values.
WERKZEUGKISTE_CONFIG_EXPORT
Configuration LoadJSONFile(std::string_view filename,
    NullValuePolicy none_policy = NullValuePolicy::Skip);

/// @brief Loads a JSON configuration from a string.
///
/// Because a configuration must consist of key/value pairs, a plain JSON
/// array (e.g. "[1, 2, 3]") will be loaded into the key `json`. Thus, the
/// configuration would have 1 element, and you need to access it via its key.
/// For example, cfg.Size("json"), cfg.GetDouble("json[0]"), etc.
///
/// @param json_string String representation of the JSON configuration.
/// @param none_policy How to deal with None values.
WERKZEUGKISTE_CONFIG_EXPORT
Configuration LoadJSONString(std::string_view json_string,
    NullValuePolicy none_policy = NullValuePolicy::Skip);

/// @brief Returns a libconfig-formatted string.
WERKZEUGKISTE_CONFIG_EXPORT
std::string DumpLibconfigString(const Configuration &cfg);

/// @brief Returns a TOML-formatted string.
inline std::string DumpTOMLString(const Configuration &cfg) {
  return cfg.ToTOML();
}

/// @brief Returns a JSON-formatted string.
inline std::string DumpJSONString(const Configuration &cfg) {
  return cfg.ToJSON();
}

/// @brief Returns a YAML-formatted string.
inline std::string DumpYAMLString(const Configuration &cfg) {
  return cfg.ToYAML();
}

}  // namespace werkzeugkiste::config

#endif  // WERKZEUGKISTE_CONFIG_CONFIGURATION_H
