#include <werkzeugkiste/config/casts.h>
#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/config/types.h>
#include <werkzeugkiste/strings/strings.h>

#include <iomanip>
#include <sstream>

// TODOs
// * When upgrading to C++20, we can easily support date/time
//   checks: https://en.cppreference.com/w/cpp/chrono/year_month_day/ok
// * Parsing was also introduced in C++20

namespace werkzeugkiste::config {
std::string ConfigTypeToString(const ConfigType &ct) {
  switch (ct) {
    case ConfigType::Boolean:
      return "boolean";

    case ConfigType::Integer:
      return "integer";

    case ConfigType::FloatingPoint:
      return "floating_point";

    case ConfigType::String:
      return "string";

    case ConfigType::Date:
      return "date";

    case ConfigType::Time:
      return "time";

    case ConfigType::List:
      return "list";

    case ConfigType::Group:
      return "group";
  }

  // LCOV_EXCL_START
  std::ostringstream msg;
  msg << "ConfigType (" << static_cast<int>(ct)
      << ") is not handled within `ConfigTypeToString`. Please file an issue "
         "at https://github.com/snototter/werkzeugkiste/issues";
  throw std::logic_error(msg.str());
  // LCOV_EXCL_STOP
}

std::string date::ToString() const {
  // The unary operator+ ensures that uint8 values are
  // interpreted as numbers and not ASCII characters.
  std::ostringstream s;
  s << std::setfill('0') << std::setw(4) << +year << '-' << std::setw(2)
    << +month << '-' << std::setw(2) << +day;
  return s.str();
}

// NOLINTNEXTLINE(*macro-usage)
#define WZK_RAISE_DATE_PARSE_ERROR(STR)                             \
  do {                                                              \
    std::string msg{"Invalid string representation for a date: `"}; \
    msg += str;                                                     \
    msg += "`!";                                                    \
    throw ParseError(msg);                                          \
  } while (false)

template <typename T>
T ParseNumber(const std::string &token) {
  try {
    const int parsed = std::stoi(token);
    return CheckedCast<T, int, ParseError>(parsed);
  } catch (const std::invalid_argument &e) {
    throw ParseError{e.what()};
  } catch (const std::out_of_range &e) {
    throw ParseError{e.what()};
  }
}

date date::FromString(std::string_view str) {
  date parsed{};
  std::size_t pos = str.find_first_of('-');
  if (pos != std::string_view::npos) {
    // Expect Y-m-d
    std::vector<std::string> tokens = strings::Split(str, '-');
    if (tokens.size() != 3) {
      WZK_RAISE_DATE_PARSE_ERROR(str);
    }
    parsed.year = ParseNumber<uint16_t>(tokens[0]);
    parsed.month = ParseNumber<uint8_t>(tokens[1]);
    parsed.day = ParseNumber<uint8_t>(tokens[2]);

    return parsed;
  }

  pos = str.find_first_of('.');
  if (pos != std::string_view::npos) {
    // Expect d.m.Y
    std::vector<std::string> tokens = strings::Split(str, '.');
    if (tokens.size() != 3) {
      WZK_RAISE_DATE_PARSE_ERROR(str);
    }
    parsed.day = ParseNumber<uint8_t>(tokens[0]);
    parsed.month = ParseNumber<uint8_t>(tokens[1]);
    parsed.year = ParseNumber<uint16_t>(tokens[2]);

    return parsed;
  }

  WZK_RAISE_DATE_PARSE_ERROR(str);
}

bool date::operator==(const date &other) const {
  return (year == other.year) && (month == other.month) && (day == other.day);
}

bool date::operator!=(const date &other) const { return !(*this == other); }

bool date::operator<(const date &other) const {
  return Pack(*this) < Pack(other);
}

bool date::operator<=(const date &other) const {
  return Pack(*this) <= Pack(other);
}

bool date::operator>(const date &other) const {
  return Pack(*this) > Pack(other);
}

bool date::operator>=(const date &other) const {
  return Pack(*this) >= Pack(other);
}

std::string time::ToString() const {
  // The unary operator+ ensures that uint8 values are
  // interpreted as numbers and not ASCII characters.
  std::ostringstream s;
  s << std::setfill('0') << std::setw(2) << +hour << ':' << std::setw(2)
    << +minute << ':' << std::setw(2) << +second << '.' << std::setw(9)
    << +nanosecond;
  return s.str();
}

bool time::operator==(const time &other) const {
  return (hour == other.hour) && (minute == other.minute) &&
         (second == other.second) && (nanosecond == other.nanosecond);
}

bool time::operator!=(const time &other) const { return !(*this == other); }

bool time::operator<(const time &other) const {
  return Pack(*this) < Pack(other);
}

bool time::operator<=(const time &other) const {
  return Pack(*this) <= Pack(other);
}

bool time::operator>(const time &other) const {
  return Pack(*this) > Pack(other);
}

bool time::operator>=(const time &other) const {
  return Pack(*this) >= Pack(other);
}

}  // namespace werkzeugkiste::config
