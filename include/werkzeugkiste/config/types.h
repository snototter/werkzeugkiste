#ifndef WERKZEUGKISTE_CONFIG_TYPES_H
#define WERKZEUGKISTE_CONFIG_TYPES_H

#include <werkzeugkiste/config/config_export.h>

#include <cstdint>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <typeinfo>

namespace werkzeugkiste::config {
//-----------------------------------------------------------------------------
// Exceptions

/// @brief Indicates a failure during parsing a configuration string/file.
class WERKZEUGKISTE_CONFIG_EXPORT ParseError : public std::runtime_error {
 public:
  explicit ParseError(const std::string &msg) : std::runtime_error{msg} {}
};

/// @brief Indicates that an invalid key was provided to access a parameter.
class WERKZEUGKISTE_CONFIG_EXPORT KeyError : public std::invalid_argument {
 public:
  explicit KeyError(const std::string &msg) : std::invalid_argument{msg} {}
};

/// @brief Indicates that an invalid type was used to query or set a parameter.
class WERKZEUGKISTE_CONFIG_EXPORT TypeError : public std::invalid_argument {
 public:
  explicit TypeError(const std::string &msg) : std::invalid_argument{msg} {}
};

/// @brief Indicates invalid input values.
class WERKZEUGKISTE_CONFIG_EXPORT ValueError : public std::invalid_argument {
 public:
  explicit ValueError(const std::string &msg) : std::invalid_argument{msg} {}
};

//-----------------------------------------------------------------------------
// Supported parameters

/// @brief Available configuration parameters.
enum class ConfigType : unsigned char {
  /// @brief A boolean flag.
  Boolean,

  /// @brief A 32- or 64-bit integer.
  ///
  /// Internally, integers are represented by 64-bit.
  Integer,

  /// @brief A single- or double-precision floating point number.
  ///
  /// Internally, floating point numbers are always represented by a double.
  FloatingPoint,

  /// @brief A string.
  String,

  /// @brief A local date.
  Date,

  /// @brief A local time.
  Time,

  /// @brief A date-time following <a
  /// href="https://www.rfc-editor.org/rfc/rfc3339">RFC 3339</a>.
  DateTime,

  /// @brief A list/array of unnamed parameters.
  List,

  /// @brief A group/collection of named parameters.
  Group
};

/// @brief Returns the string representation.
WERKZEUGKISTE_CONFIG_EXPORT
std::string ConfigTypeToString(const ConfigType &ct);

/// @brief Prints the string representation of a `ConfigType` out to the stream.
WERKZEUGKISTE_CONFIG_EXPORT
std::ostream &operator<<(std::ostream &os, const ConfigType &ct);

//-----------------------------------------------------------------------------
// Null value handling (e.g. when loading JSON & YAML)

/// @brief How to handle Null/None values (e.g. when loading JSON).
enum class NullValuePolicy : unsigned char {
  /// @brief Null values will be skipped, i.e. not loaded into the
  ///   configuration.
  Skip,

  /// @brief Null values will be **replaced** by the string "null".
  NullString,

  /// @brief Null values will be **replaced** by an empty list.
  EmptyList,

  /// @brief A `werkzeugkiste::config::ParseError` will be thrown.
  Fail
};

//-----------------------------------------------------------------------------
// Date

/// @brief Represents a local date.
struct WERKZEUGKISTE_CONFIG_EXPORT date {
 public:
  /// The year.
  uint32_t year{};

  /// The month, from 1-12.
  uint32_t month{};

  /// The day, from 1-31.
  uint32_t day{};

  date() = default;

  /// @brief Parses a string representation.
  ///
  /// Supported formats are:
  /// * Y-m-d
  /// * d.m.Y
  explicit date(std::string_view str);

  date(uint32_t y, uint32_t m, uint32_t d);

  /// Returns "YYYY-mm-dd".
  std::string ToString() const;

  /// @brief Returns true if this is a valid date.
  bool IsValid() const;

  bool operator==(const date &other) const;
  bool operator!=(const date &other) const;
  bool operator<(const date &other) const;
  bool operator<=(const date &other) const;
  bool operator>(const date &other) const;
  bool operator>=(const date &other) const;

  // Pre-increment
  date &operator++();

  // Pre-decrement
  date &operator--();

  /// Overloaded stream operator.
  friend std::ostream &operator<<(std::ostream &os, const date &d) {
    os << d.ToString();
    return os;
  }
};

//-----------------------------------------------------------------------------
// Time

/// @brief Represents a local time.
struct WERKZEUGKISTE_CONFIG_EXPORT time {
 public:
  /// The hour, from 0-23.
  uint32_t hour{};

  /// The minute, from 0-59.
  uint32_t minute{};

  /// The second, from 0-59.
  uint32_t second{};

  /// The nanoseconds, from 0-999999999.
  uint32_t nanosecond{};

  time() = default;

  /// @brief Parses a string representation.
  ///
  /// Supported formats are:
  /// * HH:MM
  /// * HH:MM:SS
  /// * HH:MM:SS.sss (for milliseconds)
  /// * HH:MM:SS.ssssss (for microseconds)
  /// * HH:MM:SS.sssssssss (for nanoseconds)
  explicit time(std::string_view str);

  time(uint32_t h, uint32_t m, uint32_t s = 0, uint32_t ns = 0);

  /// @brief Returns "HH:MM:SS.sssssssss".
  std::string ToString() const;

  /// @brief Returns true if this is a valid time between 00:00 and
  ///   23:59:59.999999999.
  bool IsValid() const;

  bool operator==(const time &other) const;
  bool operator!=(const time &other) const;
  bool operator<(const time &other) const;
  bool operator<=(const time &other) const;
  bool operator>(const time &other) const;
  bool operator>=(const time &other) const;

  /// @brief Overloaded stream operator.
  friend std::ostream &operator<<(std::ostream &os, const time &t) {
    os << t.ToString();
    return os;
  }
};

//-----------------------------------------------------------------------------
// Time zone offset

/// @brief Represents a time zone offset.
///
/// Note that `time_offset` cannot represent the __unknown local offset
/// convention__ (according to
/// <a href="https://www.rfc-editor.org/rfc/rfc3339">RFC 3339</a>, __i.e.__ it
///  cannot distinguish between `-00:00` and `+00:00`.
struct WERKZEUGKISTE_CONFIG_EXPORT time_offset {
 public:
  /// @brief The offset from `UTC+00:00` in minutes.
  int32_t minutes{};

  /// @brief Default c'tor.
  time_offset() = default;

  explicit time_offset(int32_t m) : minutes{m} {}

  /// @brief Parses a string representation.
  ///
  /// Supported formats are:
  /// * `Z`, __i.e.__ the 0 offset.
  /// * `[+-]?HH:MM`
  explicit time_offset(std::string_view str);

  // TODO doc + highlight that (-1, 30) is *not* equivalent to "-01:30", but
  // instead "-00:30".
  time_offset(int32_t h, int32_t m);

  /// @brief Returns `"Z"` or `"+/-HH:MM"`.
  std::string ToString() const;

  bool operator==(const time_offset &other) const;
  bool operator!=(const time_offset &other) const;
  bool operator<(const time_offset &other) const;
  bool operator<=(const time_offset &other) const;
  bool operator>(const time_offset &other) const;
  bool operator>=(const time_offset &other) const;

  /// @brief Overloaded stream operator.
  friend std::ostream &operator<<(std::ostream &os, const time_offset &t) {
    os << t.ToString();
    return os;
  }
};

//-----------------------------------------------------------------------------
// Date-time

/// @brief A date-time specification following
///   <a href="https://www.rfc-editor.org/rfc/rfc3339">RFC 3339</a>.
struct date_time {
 public:
  config::date date{};
  config::time time{};
  std::optional<time_offset> offset{std::nullopt};

  date_time() = default;

  explicit date_time(std::string_view str);

  date_time(const config::date &d, const config::time &t) : date{d}, time{t} {}

  date_time(const config::date &d, const config::time &t, const time_offset &o)
      : date{d}, time{t}, offset{o} {}

  // TODO s.t. offset == +00:00. If no offset is set, it is assumed to be
  // UTC already.
  date_time UTC() const;

  /// @brief Returns the representation in the <a
  /// href="https://www.rfc-editor.org/rfc/rfc3339">RFC 3339</a> format.
  std::string ToString() const;

  /// @brief Returns `true` if this `date_time` has no time zone offset.
  inline bool IsLocal() const { return !offset.has_value(); }

  bool operator==(const date_time &other) const;
  bool operator!=(const date_time &other) const;
  // TODO operators
  //  bool operator<(const time_offset &other) const;
  //  bool operator<=(const time_offset &other) const;
  //  bool operator>(const time_offset &other) const;
  //  bool operator>=(const time_offset &other) const;

  /// @brief Prints the <a href="https://www.rfc-editor.org/rfc/rfc3339">RFC
  /// 3339</a> representation out to the stream.
  friend std::ostream &operator<<(std::ostream &os, const date_time &t) {
    os << t.ToString();
    return os;
  }
};

//-----------------------------------------------------------------------------
// Convenience types to query configuration parameters

/// @brief A point in 2D Euclidean space.
/// @tparam Tp Underlying type, either integral or floating point.
template <typename Tp>
struct point2d {
  static_assert(std::is_arithmetic_v<Tp>,
      "Type of coordinates must be integer or floating point!");

 public:
  // NOLINTNEXTLINE(readability-identifier-naming)
  using value_type = Tp;

  static constexpr std::size_t ndim = 2;
  Tp x{};
  Tp y{};
};

/// @brief A point in 3D Euclidean space.
/// @tparam Tp Underlying type, either integral or floating point.
template <typename Tp>
struct point3d {
  static_assert(std::is_arithmetic_v<Tp>,
      "Only arithmetic types are supported!");

 public:
  // NOLINTNEXTLINE(readability-identifier-naming)
  using value_type = Tp;
  static constexpr std::size_t ndim = 3;
  Tp x{};
  Tp y{};
  Tp z{};
};

// /// @brief Encapsulates a two-dimensional size.
// /// @tparam Tp Underlying type, either integral or floating point.
// template <typename Tp>
// struct size2d {
//   static_assert(std::is_arithmetic_v<Tp>,
//       "Type of dimensions must be integer or floating point!");

//  public:
//   // NOLINTNEXTLINE(readability-identifier-naming)
//   using value_type = Tp;

//   static constexpr std::size_t ndim = 2;
//   Tp width{};
//   Tp height{};
// };

// /// @brief Encapsulates a three-dimensional size.
// /// @tparam Tp Underlying type, either integral or floating point.
// template <typename Tp>
// struct size3d {
//   static_assert(std::is_arithmetic_v<Tp>,
//       "Type of dimensions must be integer or floating point!");

//  public:
//   // NOLINTNEXTLINE(readability-identifier-naming)
//   using value_type = Tp;

//   static constexpr std::size_t ndim = 3;
//   Tp width{};
//   Tp height{};
//   Tp length{};
// };

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

// LCOV_EXCL_START
WZKREG_TNSPEC(bool)
WZKREG_TNSPEC(char)
WZKREG_TNSPEC(unsigned char)
WZKREG_TNSPEC(int16_t)
WZKREG_TNSPEC(uint16_t)
WZKREG_TNSPEC(int32_t)
WZKREG_TNSPEC(uint32_t)
WZKREG_TNSPEC(int64_t)
WZKREG_TNSPEC(uint64_t)
WZKREG_TNSPEC(float)
WZKREG_TNSPEC(double)

WZKREG_TNSPEC(date)
WZKREG_TNSPEC(time)
WZKREG_TNSPEC(time_offset)
WZKREG_TNSPEC(date_time)

WZKREG_TNSPEC_STR(std::string, string)
WZKREG_TNSPEC_STR(std::string_view, string_view)

// LCOV_EXCL_STOP

#undef WZKREG_TNSPEC
#undef WZKREG_TNSPEC_STR

}  // namespace werkzeugkiste::config

#endif  // WERKZEUGKISTE_CONFIG_TYPES_H
