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
  explicit KeyError(std::string msg) : std::invalid_argument{msg} {}
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
  /// Internally, integers are always handled as 64-bits.
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

  /// @brief A date-time following RFC 3339.
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
// Date

/// @brief A local date.
struct WERKZEUGKISTE_CONFIG_EXPORT date {
 public:
  /// The year.
  uint_least16_t year{};

  /// The month, from 1-12.
  uint_least8_t month{};

  /// The day, from 1-31.
  uint_least8_t day{};

  date() = default;

  /// @brief Parses a string representation.
  ///
  /// Supported formats are:
  /// * Y-m-d
  /// * d.m.Y
  explicit date(std::string_view str);

  date(uint_least16_t y, uint_least8_t m, uint_least8_t d);

  /// Returns "YYYY-mm-dd".
  std::string ToString() const;

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

 private:
  // NOLINTBEGIN(*-magic-numbers)
  static constexpr uint_least32_t Pack(const date &d) noexcept {
    return (static_cast<uint_least32_t>(d.year) << 16U) |
           (static_cast<uint_least32_t>(d.month) << 8U) |
           static_cast<uint_least32_t>(d.day);
  }
  // NOLINTEND(*-magic-numbers)
};

//-----------------------------------------------------------------------------
// Time

/// @brief A local time.
struct WERKZEUGKISTE_CONFIG_EXPORT time {
 public:
  /// The hour, from 0-23.
  uint_least8_t hour{};

  /// The minute, from 0-59.
  uint_least8_t minute{};

  /// The second, from 0-59.
  uint_least8_t second{};

  /// The nanoseconds, from 0-999999999.
  uint_least32_t nanosecond{};

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

  time(uint_least8_t h, uint_least8_t m, uint_least8_t s = 0,
       uint_least32_t ns = 0)
      : hour{h}, minute{m}, second{s}, nanosecond{ns} {}

  /// @brief Returns "HH:MM:SS.sssssssss".
  std::string ToString() const;

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

 private:
  // NOLINTBEGIN(*-magic-numbers)
  static constexpr uint_least64_t Pack(const time &t) noexcept {
    return (static_cast<uint_least64_t>(t.hour) << 48U) |
           (static_cast<uint_least64_t>(t.minute) << 40U) |
           (static_cast<uint_least64_t>(t.second) << 32U) |
           static_cast<uint_least64_t>(t.nanosecond);
  }
  // NOLINTEND(*-magic-numbers)
};

//-----------------------------------------------------------------------------
// Time zone offset

// TODO doc
// Caveats:
// time_offset cannot represent the 'unknown local offset convention', i.e.
// distinguishing between -00:00 and +00:00.
// https://www.rfc-editor.org/rfc/rfc3339
// --> this is done via the optional<time_offset> in date_time instead.
struct WERKZEUGKISTE_CONFIG_EXPORT time_offset {
 public:
  /// The offset from UTC+0 in minutes.
  int_least16_t minutes{};

  time_offset() = default;

  explicit time_offset(int_least16_t m) : minutes{m} {}

  /// @brief Parses a string representation.
  ///
  /// Supported formats are:
  /// * Z
  /// * [+-]?HH:MM
  explicit time_offset(std::string_view str);

  // TODO doc + highlight that (-1, 30) is *not* equivalent to "-01:30", but
  // instead "-00:30".
  time_offset(int_least8_t h, int_least8_t m);

  /// Returns "Z" or "+/-HH:MM".
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

  /// @brief Returns the representation in RFC 3339 format.
  std::string ToString() const;

  inline bool IsLocal() const { return !offset.has_value(); }

  // TODO operators
  bool operator==(const date_time &other) const;
  bool operator!=(const date_time &other) const;
  //  bool operator<(const time_offset &other) const;
  //  bool operator<=(const time_offset &other) const;
  //  bool operator>(const time_offset &other) const;
  //  bool operator>=(const time_offset &other) const;

  /// @brief Prints the RFC 3339 representation out to the stream.
  friend std::ostream &operator<<(std::ostream &os, const date_time &t) {
    os << t.ToString();
    return os;
  }
};

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
WZKREG_TNSPEC(short)
WZKREG_TNSPEC(unsigned short)
WZKREG_TNSPEC(int)
WZKREG_TNSPEC(unsigned int)
WZKREG_TNSPEC(long int)
WZKREG_TNSPEC(unsigned long int)
WZKREG_TNSPEC(float)
WZKREG_TNSPEC(double)

WZKREG_TNSPEC(date)
WZKREG_TNSPEC(time)

WZKREG_TNSPEC_STR(std::string, string)
WZKREG_TNSPEC_STR(std::string_view, string_view)

// LCOV_EXCL_STOP

#undef WZKREG_TNSPEC
#undef WZKREG_TNSPEC_STR

}  // namespace werkzeugkiste::config

#endif  // WERKZEUGKISTE_CONFIG_TYPES_H
