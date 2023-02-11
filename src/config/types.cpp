#include <werkzeugkiste/config/types.h>

#include <iomanip>
#include <sstream>

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
}

std::string date::ToString() const {
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
  //  if (year == other.year) {
  //    if (month == other.month) {
  //      return day < other.day;
  //    }
  //    return month < other.month;
  //  }

  //  return (year < other.year);
  return Pack(*this) < Pack(other);
}

bool date::operator<=(const date &other) const {
  return Pack(*this) <= Pack(other);
}

bool date::operator>(const date &other) const {
  //  return !((*this == other) || (other < *this));
  return Pack(*this) > Pack(other);
}

bool date::operator>=(const date &other) const {
  return Pack(*this) >= Pack(other);
}

std::string time::ToString() const {
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
  //  if (hour == other.hour) {
  //    if (minute == other.minute) {
  //      if (second == other.second) {
  //        return (nanosecond < other.nanosecond);
  //      }
  //      return (second < other.second);
  //    }
  //    return (minute < other.minute);
  //  }
  //  return (hour < other.hour);
  return Pack(*this) < Pack(other);
}

bool time::operator<=(const time &other) const {
  return Pack(*this) <= Pack(other);
}

bool time::operator>(const time &other) const {
  //  return !((*this == other) || (other < *this));
  return Pack(*this) > Pack(other);
}

bool time::operator>=(const time &other) const {
  return Pack(*this) >= Pack(other);
}

}  // namespace werkzeugkiste::config
