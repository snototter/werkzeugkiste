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
// * Use fixed width integer types - switch back to intX_t instead of
//   least/fast, or change to int for simplicity (requires additional error
//   handling). We're not targeting any special platform & the fixed width
//   types are promoted during arithmetic operations (causing GCC warnings),
//   https://stackoverflow.com/questions/39060852/why-does-the-addition-of-two-shorts-return-an-int/39061103#39061103

// NOLINTBEGIN(*magic-numbers)
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
    msg << "Invalid number " << +(VAL) << " while parsing a `" << (DT) \
        << "`. Value must be in [" << +(MI) << ", " << +(MA) << "]!";  \
    throw ParseError(msg.str());                                       \
  } while (false)

//-----------------------------------------------------------------------------
// Internal utility namespace
namespace detail {
// Collection of low-level date algorithms, along with an excellent analysis
// of their performance: http://howardhinnant.github.io/date_algorithms.html
constexpr bool IsLeapYear(uint_least16_t year) noexcept {
  return (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0));
}

constexpr uint_least8_t LastDayOfMonthCommonYear(uint_least8_t m) noexcept {
  // NOLINTNEXTLINE(*avoid-c-arrays)
  constexpr uint_least8_t days[] = {
      31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  return days[m - 1];
}

constexpr uint_least8_t LastDayOfMonth(uint_least16_t year,
    uint_least8_t month) noexcept {
  return ((month != 2) || !IsLeapYear(year)) ? LastDayOfMonthCommonYear(month)
                                             : static_cast<uint_least8_t>(29);
}

constexpr bool IsValidDate(uint_least16_t year,
    uint_least8_t month,
    uint_least8_t day) noexcept {
  return ((month < 1) || (month > 12))
           ? false
           : ((day > 0) && (day <= LastDayOfMonth(year, month)));
}

constexpr bool IsValidTime(uint_fast8_t hour,
    uint_fast8_t minute,
    uint_fast8_t second,
    uint_fast32_t nanosecond) {
  return (hour < 24) && (minute < 60) && (second < 60) &&
         (nanosecond < 1000000000);
}
}  // namespace detail

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

    case ConfigType::DateTime:
      return "date_time";

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

std::ostream &operator<<(std::ostream &os, const ConfigType &ct) {
  os << ConfigTypeToString(ct);
  return os;
}

//-----------------------------------------------------------------------------
// Number parsing for date & time types

/// @brief Returns true if the input string is a valid integer, i.e.
///   "[+-]?[0-9]+".
bool IsInteger(std::string_view str) {
  if (str.empty()) {
    return false;
  }

  const bool sign = (str[0] == '-' || str[0] == '+');
  const auto begin = sign ? str.begin() + 1 : str.begin();
  const auto pos = std::find_if(
      begin, str.end(), [](char c) -> bool { return std::isdigit(c) == 0; });
  if (pos != str.end()) {
    return false;
  }

  return sign ? (str.length() > 2) : true;
}

template <typename T>
T ParseDateTimeNumber(const std::string &str,
    int min_val,
    int max_val,
    std::string_view type_str) {
  try {
    // We can't use stoi out of the box:
    // * it accepts (and truncates) floating point numbers, and
    // * it takes as many characters as possible (and won't throw an exception
    //   if the "rest" of the string is invalid).
    // Thus, we need to make sure that only [+-][0-9]+ inputs are provided as
    // inputs to stoi. We do not allow white space.
    if (!IsInteger(str)) {
      WZK_RAISE_DATETIME_PARSE_ERROR(str, type_str);
    }

    const int parsed = std::stoi(str);
    if ((parsed < min_val) || (parsed > max_val)) {
      WZK_RAISE_NUMBER_PARSE_ERROR(parsed, min_val, max_val, type_str);
    }
    return checked_numcast<T, int, ParseError>(parsed);
  } catch (const std::logic_error &e) {
    // LCOV_EXCL_START
    // All exceptions thrown within stoi are derived from logic_error. This
    // branch, however, is currently unreachable, as the "IsInteger" check
    // already filters out anything that does not resemble "[+-][0-9]+".
    throw ParseError{e.what()};
    // LCOV_EXCL_STOP
  }
}

//-----------------------------------------------------------------------------
// Date

date ParseDateString(std::string_view str) {
  using namespace std::string_view_literals;

  date parsed{};
  std::size_t pos = str.find_first_of('-');
  if (pos != std::string_view::npos) {
    // Expect Y-m-d
    std::vector<std::string> tokens = strings::Split(str, '-');
    if (tokens.size() != 3) {
      WZK_RAISE_DATETIME_PARSE_ERROR(str, date);
    }
    parsed.year =
        ParseDateTimeNumber<uint_least16_t>(tokens[0], 0, 9999, "date"sv);
    parsed.month =
        ParseDateTimeNumber<uint_least8_t>(tokens[1], 1, 12, "date"sv);
    parsed.day = ParseDateTimeNumber<uint_least8_t>(tokens[2],
        1,
        detail::LastDayOfMonth(parsed.year, parsed.month),
        "date"sv);

    return parsed;
  }

  pos = str.find_first_of('.');
  if (pos != std::string_view::npos) {
    // Expect d.m.Y
    std::vector<std::string> tokens = strings::Split(str, '.');
    if (tokens.size() != 3) {
      WZK_RAISE_DATETIME_PARSE_ERROR(str, date);
    }
    parsed.year =
        ParseDateTimeNumber<uint_least16_t>(tokens[2], 0, 9999, "date"sv);
    parsed.month =
        ParseDateTimeNumber<uint_least8_t>(tokens[1], 1, 12, "date"sv);
    parsed.day = ParseDateTimeNumber<uint_least8_t>(tokens[0],
        1,
        detail::LastDayOfMonth(parsed.year, parsed.month),
        "date"sv);

    return parsed;
  }

  WZK_RAISE_DATETIME_PARSE_ERROR(str, date);
}

date::date(std::string_view str) {
  const date other = ParseDateString(str);
  year = other.year;
  month = other.month;
  day = other.day;
}

date::date(uint_least16_t y, uint_least8_t m, uint_least8_t d)
    : year{y}, month{m}, day{d} {
  if (!detail::IsValidDate(y, m, d)) {
    std::ostringstream msg;
    msg << "Cannot create a valid `date` from " << +y << ", " << +m << ", "
        << +d << '!';
    throw ValueError(msg.str());
  }
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

date &date::operator++() {
  if (day == detail::LastDayOfMonth(year, month)) {
    day = 1;
    if (month == 12) {
      month = 1;
      ++year;
    } else {
      ++month;
    }
  } else {
    ++day;
  }
  return *this;
}

date &date::operator--() {
  if (day == 1) {
    if (month == 1) {
      month = 12;
      --year;
    } else {
      --month;
    }
    day = detail::LastDayOfMonth(year, month);
  } else {
    --day;
  }
  return *this;
}

//-----------------------------------------------------------------------------
// Time

time::time(std::string_view str) {
  using namespace std::string_view_literals;
  const std::vector<std::string> hms_tokens = strings::Split(str, ':');
  if ((hms_tokens.size() < 2) || (hms_tokens.size() > 3)) {
    WZK_RAISE_DATETIME_PARSE_ERROR(str, time);
  }

  hour = ParseDateTimeNumber<uint_least8_t>(hms_tokens[0], 0, 23, "time"sv);
  minute = ParseDateTimeNumber<uint_least8_t>(hms_tokens[1], 0, 59, "time"sv);

  if (hms_tokens.size() > 2) {
    // Delimiter between second and fraction component can be '.' or ','.
    const std::size_t pos = hms_tokens[2].find_first_of(".,");
    if (pos != std::string::npos) {
      second = ParseDateTimeNumber<uint_least8_t>(
          hms_tokens[2].substr(0, pos), 0, 59, "time"sv);

      const auto subsec_str = hms_tokens[2].substr(pos + 1);
      if ((subsec_str.length() > 3) &&
          ((subsec_str.length() != 6) && (subsec_str.length() != 9))) {
        std::string msg{"Invalid string representation for a time: `"};
        msg += str;
        msg +=
            "`. Specify sub-second component by 1, 2, 3 (ms), 6 (us) "
            "or 9 (ns) digits!";
        throw ParseError(msg);
      }
      uint_least32_t subsec_val = ParseDateTimeNumber<uint_least32_t>(
          subsec_str, 0, 999999999, "time"sv);

      if (subsec_str.length() == 1) {
        subsec_val *= 100000000;
      } else if (subsec_str.length() == 2) {
        subsec_val *= 10000000;
      } else if (subsec_str.length() == 3) {
        subsec_val *= 1000000;
      } else if (subsec_str.length() == 6) {
        subsec_val *= 1000;
      }
      nanosecond = subsec_val;

    } else {
      second =
          ParseDateTimeNumber<uint_least8_t>(hms_tokens[2], 0, 59, "time"sv);
    }
  }
}

time::time(uint_least8_t h, uint_least8_t m, uint_least8_t s, uint_least32_t ns)
    : hour{h}, minute{m}, second{s}, nanosecond{ns} {
  if (!detail::IsValidTime(h, m, s, ns)) {
    std::ostringstream msg;
    msg << "Cannot create a valid `time` from " << +h << ':' << +m << ':' << +s
        << '.' << +ns << '!';
    throw ValueError(msg.str());
  }
}

std::string time::ToString() const {
  // The unary operator+ ensures that uint8 values are
  // interpreted as numbers and not ASCII characters.
  std::ostringstream s;
  s << std::setfill('0') << std::setw(2) << +hour << ':' << std::setw(2)
    << +minute << ':' << std::setw(2) << +second;
  if (nanosecond > 0) {
    if ((nanosecond % 1000000) == 0) {
      const uint_least32_t ms = nanosecond / 1000000;
      s << '.' << std::setw(3) << +ms;
    } else if ((nanosecond % 1000) == 0) {
      const uint_least32_t us = nanosecond / 1000;
      s << '.' << std::setw(6) << +us;
    } else {
      s << '.' << std::setw(9) << +nanosecond;
    }
  }
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

//-----------------------------------------------------------------------------
// Time Offset

time_offset::time_offset(std::string_view str) {
  using namespace std::string_view_literals;

  std::size_t pos = str.find_first_of(':');
  if (pos == std::string::npos) {
    if ((str.length() > 0) && (str[0] != 'z') && (str[0] != 'Z')) {
      WZK_RAISE_DATETIME_PARSE_ERROR(str, time_offset);
    }
    minutes = 0;
  } else {
    const int_least16_t hrs = ParseDateTimeNumber<int_least16_t>(
        std::string{str.substr(0, pos)}, -23, 23, "time_offset"sv);
    const int_least16_t mins = ParseDateTimeNumber<int_least16_t>(
        std::string{str.substr(pos + 1)}, 0, 59, "time_offset"sv);
    // Offset minutes in RFC 3339 format can only be positive. The
    // string "-01:23", however, corresponds to a total offset of
    // "-83 minutes". Thus, we need to adjust the parsed minutes accordingly.
    minutes = static_cast<int_least16_t>(hrs * 60);
    if ((hrs < 0) || (str[0] == '-')) {
      minutes -= static_cast<int_least16_t>(mins);
    } else {
      minutes += static_cast<int_least16_t>(mins);
    }
  }
}

time_offset::time_offset(int_least8_t h, int_least8_t m) {
  if ((h < -23) || (h > 23) || (m < -59) || (m > 59)) {
    std::ostringstream msg;
    msg << "Invalid parameters h=" << +h << ", m=" << +m
        << " for time offset. Values must be -24 < h < 24 and "
           "-60 < m < 60!";
    throw TypeError(msg.str());
  }
  minutes = static_cast<int_least16_t>(h * 60 + m);
}

std::string time_offset::ToString() const {
  if (minutes == 0) {
    return "Z";
  }

  const int_least16_t hrs = std::abs(minutes) / 60;
  const int_least16_t mins = std::abs(minutes) - hrs * 60;
  // The unary operator+ ensures that uint8 values are
  // interpreted as numbers and not ASCII characters.
  std::ostringstream s;
  s << ((minutes >= 0) ? '+' : '-') << std::setfill('0') << std::setw(2) << +hrs
    << ':' << std::setw(2) << +mins;
  return s.str();
}

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
  // len(19) would be following the rfc, len(16) is missing the seconds
  // component ":SS". For usage convenience, we are lenient here.
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

date_time date_time::UTC() const {
  date_time utc{this->date, this->time};
  constexpr int_least16_t min_per_day = 1440;
  if (this->offset.has_value()) {
    int_least16_t minutes = static_cast<int_least16_t>(utc.time.hour) * 60 +
                            utc.time.minute - offset.value().minutes;
    if (minutes >= min_per_day) {
      ++utc.date;
      minutes -= min_per_day;
    }
    if (minutes < 0) {
      --utc.date;
      minutes += min_per_day;
    }

    utc.time.hour = static_cast<uint_least8_t>(minutes / 60);
    utc.time.minute = static_cast<uint_least8_t>(minutes % 60);
  }

  return utc;
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
  // Currently, we follow the definition strictly: a date-time without
  // an explicitly set offset (i.e. a local time) is not equal to
  // a date-time with offset 0/Z
  if (IsLocal() || other.IsLocal()) {
    return (this->date == other.date) && (this->time == other.time) &&
           (this->offset == other.offset);
  }

  const date_time utc1 = UTC();
  const date_time utc2 = other.UTC();
  return (utc1.date == utc2.date) && (utc1.time == utc2.time);
}

bool date_time::operator!=(const date_time &other) const {
  return !(*this == other);
}

#undef WZK_RAISE_DATETIME_PARSE_ERROR
#undef WZK_RAISE_NUMBER_PARSE_ERROR
}  // namespace werkzeugkiste::config
// NOLINTEND(*magic-numbers)
