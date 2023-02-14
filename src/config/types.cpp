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
    msg += (STR);                                            \
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

//-----------------------------------------------------------------------------
// Number parsing for date & time types

template <typename T, typename DT>
T ParseIntegralNumber(const std::string &str, int min_val, int max_val) {
  try {
    // stoi would accept (and truncate) floating point numbers. Thus,
    // we need to make sure that only [+-][0-9] inputs are fed into it.
    // White space is not allowed.
    const auto pos = std::find_if(str.begin(), str.end(), [](char c) -> bool {
      return !(std::isdigit(c) || (c == '-') || (c == '+'));
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

//-----------------------------------------------------------------------------
// Date

// NOLINTBEGIN(*magic-numbers)
date ParseDateString(std::string_view str) {
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

date::date(std::string_view str) {
  const date other = ParseDateString(str);
  year = other.year;
  month = other.month;
  day = other.day;
}

std::string date::ToString() const {
  // The unary operator+ ensures that uint8 values are
  // interpreted as numbers and not ASCII characters.
  std::ostringstream s;
  s << std::setfill('0') << std::setw(4) << +year << '-' << std::setw(2)
    << +month << '-' << std::setw(2) << +day;
  return s.str();
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

//-----------------------------------------------------------------------------
// Time

// NOLINTBEGIN(*magic-numbers)
time::time(std::string_view str) {
  const std::vector<std::string> hms_tokens = strings::Split(str, ':');
  if ((hms_tokens.size() < 2) || (hms_tokens.size() > 3)) {
    WZK_RAISE_DATETIME_PARSE_ERROR(str, time);
  }

  hour = ParseIntegralNumber<uint8_t, time>(hms_tokens[0], 0, 23);
  minute = ParseIntegralNumber<uint8_t, time>(hms_tokens[1], 0, 59);

  if (hms_tokens.size() > 2) {
    std::size_t pos = hms_tokens[2].find_first_of('.');
    if (pos != std::string::npos) {
      second = ParseIntegralNumber<uint8_t, time>(hms_tokens[2].substr(0, pos),
                                                  0, 59);

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
      nanosecond = subsec_val;

    } else {
      second = ParseIntegralNumber<uint8_t, time>(hms_tokens[2], 0, 59);
    }
  }
}

std::string time::ToString() const {
  // The unary operator+ ensures that uint8 values are
  // interpreted as numbers and not ASCII characters.
  std::ostringstream s;
  s << std::setfill('0') << std::setw(2) << +hour << ':' << std::setw(2)
    << +minute << ':' << std::setw(2) << +second;
  if (nanosecond > 0) {
    s << '.' << std::setw(9) << +nanosecond;
  }
  return s.str();
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

//-----------------------------------------------------------------------------
// Time Offset

// NOLINTBEGIN(*magic-numbers)
time_offset::time_offset(std::string_view str) {
  std::size_t pos = str.find_first_of(':');
  if (pos == std::string::npos) {
    if ((str.length() > 0) && !((str[0] == 'z') || (str[0] == 'Z'))) {
      WZK_RAISE_DATETIME_PARSE_ERROR(str, time_offset);
    }
    minutes = 0;
  } else {
    const int16_t hrs = ParseIntegralNumber<int16_t, time_offset>(
        std::string{str.substr(0, pos)}, -23, 23);
    const int16_t mins = ParseIntegralNumber<int16_t, time_offset>(
        std::string{str.substr(pos + 1)}, 0, 59);
    // Offset minutes in RFC 3339 format can only be positive. The
    // string "-01:23", however, corresponds to a total offset of
    // "-83 minutes". Thus, we need to adjust the parsed minutes accordingly.
    minutes = static_cast<int16_t>(hrs * 60);
    if ((hrs < 0) || (str[0] == '-')) {
      minutes -= mins;
    } else {
      minutes += mins;
    }
  }
}

time_offset::time_offset(int8_t h, int8_t m) {
  if ((h < -23) || (h > 23) || (m < -59) || (m > 59)) {
    std::ostringstream msg;
    msg << "Invalid parameters h=" << +h << ", m=" << +m
        << " for time offset. Values must be -24 < h < 24 and "
           "-60 < m < 60!";
    throw TypeError(msg.str());
  }
  minutes = static_cast<int16_t>(h * 60 + m);
}

std::string time_offset::ToString() const {
  if (minutes == 0) {
    return "Z";
  }

  const int hrs = std::abs(minutes) / 60;
  const int mins = std::abs(minutes) - hrs * 60;
  // The unary operator+ ensures that uint8 values are
  // interpreted as numbers and not ASCII characters.
  std::ostringstream s;
  s << ((minutes >= 0) ? '+' : '-') << std::setfill('0') << std::setw(2) << +hrs
    << ':' << std::setw(2) << +mins;
  return s.str();
}
// NOLINTEND(*magic-numbers)

bool time_offset::operator==(const time_offset &other) const {
  return minutes == other.minutes;
}

bool time_offset::operator!=(const time_offset &other) const {
  return !(*this == other);
}

bool time_offset::operator<(const time_offset &other) const {
  return minutes < other.minutes;
}

bool time_offset::operator<=(const time_offset &other) const {
  return minutes <= other.minutes;
}

bool time_offset::operator>(const time_offset &other) const {
  return minutes > other.minutes;
}

bool time_offset::operator>=(const time_offset &other) const {
  return minutes >= other.minutes;
}

//-----------------------------------------------------------------------------
// Date-time

date_time::date_time(std::string_view str) {
  // TODO 19 would be rfc (16 is missing ":SS")
  if (str.length() <= 16) {
    WZK_RAISE_DATETIME_PARSE_ERROR(str, date_time);
  }
  this->date = config::date{str.substr(0, 10)};

  const std::string_view tm_str{str.substr(11)};
  const std::size_t pos_off = tm_str.find_first_of("zZ+-");
  if (pos_off == std::string_view::npos) {
    this->time = config::time{tm_str};
  } else {
    this->time = config::time{tm_str.substr(0, pos_off)};
    // [zZ+-] are needed to parse the offset (in contrast to the
    // delimiter between date and time)
    this->offset = config::time_offset{tm_str.substr(pos_off)};
  }
}

std::string date_time::ToString() const {
  std::ostringstream s;
  s << this->date << 'T' << this->time;
  if (this->offset.has_value()) {
    s << this->offset.value();
  }
  return s.str();
}

bool date_time::operator==(const date_time &other) const {
  return (this->date == other.date) && (this->time == other.time) &&
         (this->offset == other.offset);
}

bool date_time::operator!=(const date_time &other) const {
  return !(*this == other);
}

#undef WZK_RAISE_DATETIME_PARSE_ERROR
#undef WZK_RAISE_NUMBER_PARSE_ERROR
}  // namespace werkzeugkiste::config
