#ifndef WERKZEUGKISTE_CONFIG_TYPES_H
#define WERKZEUGKISTE_CONFIG_TYPES_H

#include <werkzeugkiste/config/config_export.h>

#include <cstdint>
#include <optional>
#include <ostream>
#include <string>
#include <tuple>
#include <type_traits>
#include <typeinfo>

namespace werkzeugkiste::config {
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

  /// @brief A date.
  Date,

  /// @brief A time.
  Time,

  // TODO date_time

  /// @brief A list (vector) of unnamed parameters.
  List,

  /// @brief A group (dictionary) of named parameters.
  Group
};

// TODO ostream/istream overloads for ConfigType
WERKZEUGKISTE_CONFIG_EXPORT
std::string ConfigTypeToString(const ConfigType &ct);

struct WERKZEUGKISTE_CONFIG_EXPORT date {
 public:
  /// The year.
  uint16_t year{};

  /// The month, from 1-12.
  uint8_t month{};

  /// The day, from 1-31.
  uint8_t day{};

  date() = default;

  /// @brief Parses a string representation.
  ///
  /// Supported formats are:
  /// * Y-m-d
  /// * d.m.Y
  explicit date(std::string_view str);

  date(uint16_t y, uint8_t m, uint8_t d) : year{y}, month{m}, day{d} {}

  std::tuple<uint16_t, uint8_t, uint8_t> ToTuple() const {
    return std::make_tuple(year, month, day);
  }

  /// Returns "YYYY-mm-dd".
  std::string ToString() const;

  bool operator==(const date &other) const;
  bool operator!=(const date &other) const;
  bool operator<(const date &other) const;
  bool operator<=(const date &other) const;
  bool operator>(const date &other) const;
  bool operator>=(const date &other) const;

  /// Overloaded stream operator.
  friend std::ostream &operator<<(std::ostream &os, const date &d) {
    os << d.ToString();
    return os;
  }

 private:
  // NOLINTBEGIN(*-magic-numbers)
  static constexpr uint32_t Pack(const date &d) noexcept {
    return (static_cast<uint32_t>(d.year) << 16U) |
           (static_cast<uint32_t>(d.month) << 8U) |
           static_cast<uint32_t>(d.day);
  }
  // NOLINTEND(*-magic-numbers)
};

struct WERKZEUGKISTE_CONFIG_EXPORT time {
 public:
  /// The hour, from 0-23.
  uint8_t hour{};

  /// The minute, from 0-59.
  uint8_t minute{};

  /// The second, from 0-59.
  uint8_t second{};

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

  time(uint8_t h, uint8_t m, uint8_t s = 0, uint32_t ns = 0)
      : hour{h}, minute{m}, second{s}, nanosecond{ns} {}

  std::tuple<uint8_t, uint8_t, uint8_t, uint32_t> ToTuple() const {
    return std::make_tuple(hour, minute, second, nanosecond);
  }

  /// Returns "HH:MM:SS.sssssssss".
  std::string ToString() const;

  bool operator==(const time &other) const;
  bool operator!=(const time &other) const;
  bool operator<(const time &other) const;
  bool operator<=(const time &other) const;
  bool operator>(const time &other) const;
  bool operator>=(const time &other) const;

  /// Overloaded stream operator.
  friend std::ostream &operator<<(std::ostream &os, const time &t) {
    os << t.ToString();
    return os;
  }

 private:
  // NOLINTBEGIN(*-magic-numbers)
  static constexpr uint64_t Pack(const time &t) noexcept {
    return (static_cast<uint64_t>(t.hour) << 48U) |
           (static_cast<uint64_t>(t.minute) << 40U) |
           (static_cast<uint64_t>(t.second) << 32U) |
           static_cast<uint64_t>(t.nanosecond);
  }
  // NOLINTEND(*-magic-numbers)
};

//-----------------------------------------------------------------------------
// Time zone offset

// TODO doc
// Caveats:
// * Cannot represent the 'unknown local offset convention', i.e.
//   distinguishing between -00:00 and +00:00.
// https://www.rfc-editor.org/rfc/rfc3339
struct WERKZEUGKISTE_CONFIG_EXPORT time_offset {
 public:
  /// The offset from UTC+0 in minutes.
  int16_t minutes{};

  time_offset() = default;

  explicit time_offset(int16_t m) : minutes{m} {}

  /// @brief Parses a string representation.
  ///
  /// Supported formats are:
  /// * Z
  /// * [+-]?HH:MM
  explicit time_offset(std::string_view str);

  // TODO doc + highlight that (-1, 30) is *not* equivalent to "-01:30", but
  // instead "-00:30".
  time_offset(int8_t h, int8_t m);

  /// Returns "Z" or "+/-HH:MM".
  std::string ToString() const;

  bool operator==(const time_offset &other) const;
  bool operator!=(const time_offset &other) const;
  bool operator<(const time_offset &other) const;
  bool operator<=(const time_offset &other) const;
  bool operator>(const time_offset &other) const;
  bool operator>=(const time_offset &other) const;

  /// Overloaded stream operator.
  friend std::ostream &operator<<(std::ostream &os, const time_offset &t) {
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

WZKREG_TNSPEC(date)
WZKREG_TNSPEC(time)

WZKREG_TNSPEC_STR(std::string, string)
WZKREG_TNSPEC_STR(std::string_view, string_view)

// LCOV_EXCL_STOP

#undef WZKREG_TNSPEC
#undef WZKREG_TNSPEC_STR

}  // namespace werkzeugkiste::config

#endif  // WERKZEUGKISTE_CONFIG_TYPES_H
