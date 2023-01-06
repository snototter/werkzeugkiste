#include <sstream>
#include <werkzeugkiste/timing/stopwatch.h>

namespace werkzeugkiste::timing {

// Explicit template instantiation
template class stop_watch<std::chrono::steady_clock>;


std::string SecondsToString(unsigned int seconds) {
  std::ostringstream str;

  constexpr unsigned int sec_per_day = 86400;
  constexpr unsigned int sec_per_hour = 3600;
  constexpr unsigned int sec_per_min = 60;
  const unsigned int days = seconds / sec_per_day;

  bool needs_space = false;
  if (days > 0) {
    str << days
        << ((days > 1) ? " days" : " day");
    needs_space = true;
    seconds -= (sec_per_day * days);
  }

  const unsigned int hours = seconds / sec_per_hour;

  if (hours > 0) {
    if (needs_space) {
      str << " ";
    }
    str << hours
        << ((hours > 1) ? " hours" : " hour");
    needs_space = true;
    seconds -= (hours * sec_per_hour);
  }

  const unsigned int mins = seconds / sec_per_min;
  if (mins > 0) {
    if (needs_space) {
      str << " ";
    }
    str << mins
        << ((mins > 1) ? " minutes" : " minute");
    needs_space = true;
    seconds -= (mins * sec_per_min);
  }

  // Skip seconds if we already reported a larger unit
  if ((hours == 0) && (days == 0)
      && ((seconds > 0) || (mins == 0))) {
    if (needs_space) {
      str << " ";
    }

    str << seconds
        << ((seconds == 1) ? " second" : " seconds");
  }
  return str.str();
}

}  // namespace werkzeugkiste::timing
