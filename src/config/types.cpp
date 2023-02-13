#include <werkzeugkiste/config/casts.h>
#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/config/types.h>
#include <werkzeugkiste/logging.h>
#include <werkzeugkiste/strings/strings.h>

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>

// TODOs
// * When upgrading to C++20, we can easily support date/time
//   checks: https://en.cppreference.com/w/cpp/chrono/year_month_day/ok
// * Parsing was also introduced in C++20

namespace werkzeugkiste::config {
// NOLINTNEXTLINE(*macro-usage)
#define WZK_RAISE_DATETIME_PARSE_ERROR(STR, TP)              \
  do {                                                       \
    std::string msg{"Invalid string representation for a "}; \
    msg += (#TP);                                            \
    msg += ": `";                                            \
    msg += STR;                                              \
    msg += "`!";                                             \
    throw ParseError(msg);                                   \
  } while (false)

// NOLINTNEXTLINE(*macro-usage)
#define WZK_RAISE_NUMBER_PARSE_ERROR(VAL, MI, MA, DT)                  \
  do {                                                                 \
    std::ostringstream msg;                                            \
    msg << "Invalid number " << +(VAL) << " while parsing a " << (#DT) \
        << ", value muste be in [" << +(MI) << ", " << +(MA) << "]!";  \
    throw ParseError(msg.str());                                       \
  } while (false)

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

template <typename T, typename DT>
T ParseIntegralNumber(const std::string &str, int min_val, int max_val) {
  try {
    // stoi would accept (and truncate) floating point numbers. Thus,
    // we need to make sure that only [+-][0-9] inputs are fed into it:
    const auto pos = std::find_if(str.begin(), str.end(), [](char c) -> bool {
      return !(std::isalnum(c) || std::isspace(c) || (c == '-') || (c == '+'));
    });
    if (pos != str.end()) {
      WZK_RAISE_DATETIME_PARSE_ERROR(str, DT);
    }

    const int parsed = std::stoi(str);
    if ((parsed < min_val) || (parsed > max_val)) {
      WZK_RAISE_NUMBER_PARSE_ERROR(parsed, min_val, max_val, DT);
    }
    return checked_numcast<T, int, ParseError>(parsed);
  } catch (const std::logic_error &e) {
    // All exceptions thrown within stoi are derived from logic_error
    throw ParseError{e.what()};
  }
}

// NOLINTBEGIN(*magic-numbers)
date date::FromString(std::string_view str) {
  date parsed{};
  std::size_t pos = str.find_first_of('-');
  if (pos != std::string_view::npos) {
    // Expect Y-m-d
    std::vector<std::string> tokens = strings::Split(str, '-');
    if (tokens.size() != 3) {
      WZK_RAISE_DATETIME_PARSE_ERROR(str, date);
    }
    parsed.year = ParseIntegralNumber<uint16_t, date>(tokens[0], 0, 9999);
    parsed.month = ParseIntegralNumber<uint8_t, date>(tokens[1], 1, 12);
    parsed.day = ParseIntegralNumber<uint8_t, date>(tokens[2], 1, 31);

    return parsed;
  }

  pos = str.find_first_of('.');
  if (pos != std::string_view::npos) {
    // Expect d.m.Y
    std::vector<std::string> tokens = strings::Split(str, '.');
    if (tokens.size() != 3) {
      WZK_RAISE_DATETIME_PARSE_ERROR(str, date);
    }
    parsed.day = ParseIntegralNumber<uint8_t, date>(tokens[0], 1, 31);
    parsed.month = ParseIntegralNumber<uint8_t, date>(tokens[1], 1, 12);
    parsed.year = ParseIntegralNumber<uint16_t, date>(tokens[2], 0, 9999);

    return parsed;
  }

  WZK_RAISE_DATETIME_PARSE_ERROR(str, date);
}
// NOLINTEND(*magic-numbers)

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

// NOLINTBEGIN(*magic-numbers)
time time::FromString(std::string_view str) {
  std::vector<std::string> hms_tokens = strings::Split(str, ':');
  if ((hms_tokens.size() < 2) || (hms_tokens.size() > 3)) {
    WZK_RAISE_DATETIME_PARSE_ERROR(str, time);
  }

  time parsed{};
  parsed.hour = ParseIntegralNumber<uint8_t, time>(hms_tokens[0], 0, 23);
  parsed.minute = ParseIntegralNumber<uint8_t, time>(hms_tokens[1], 0, 59);

  if (hms_tokens.size() > 2) {
    std::size_t pos = hms_tokens[2].find_first_of('.');
    if (pos != std::string::npos) {
      parsed.second = ParseIntegralNumber<uint8_t, time>(
          hms_tokens[2].substr(0, pos), 0, 59);

      const auto subsec_str = hms_tokens[2].substr(pos + 1);
      if ((subsec_str.length() != 3) && (subsec_str.length() != 6) &&
          (subsec_str.length() != 9)) {
        std::string msg{"Invalid string representation for a time: `"};
        msg += str;
        msg +=
            "`. Specify sub-second component by 3 (ms), 6 (us) or 9 (ns) "
            "digits!";
        throw ParseError(msg);
      }
      uint32_t subsec_val =
          ParseIntegralNumber<uint32_t, time>(subsec_str, 0, 999999999);

      if (subsec_str.length() == 3) {
        subsec_val *= 1000000;
      } else if (subsec_str.length() == 6) {
        subsec_val *= 1000;
      }
      parsed.nanosecond = subsec_val;

    } else {
      parsed.second = ParseIntegralNumber<uint8_t, time>(hms_tokens[2], 0, 59);
    }
  }
  return parsed;
}
// NOLINTEND(*magic-numbers)

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

#undef WZK_RAISE_DATETIME_PARSE_ERROR
#undef WZK_RAISE_NUMBER_PARSE_ERROR
}  // namespace werkzeugkiste::config
