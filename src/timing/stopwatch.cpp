#include <sstream>
#include <werkzeugkiste/timing/stopwatch.h>

namespace werkzeugkiste::timing {

// Explicit template instantiation
template class stop_watch<std::chrono::steady_clock>;


std::string SecondsToString(unsigned int seconds) {
  std::ostringstream str;

  const unsigned int days = seconds / 86400;

  bool needs_space = false;
  if (days > 0) {
    str << days
        << ((days > 1) ? " days" : " day");
    needs_space = true;
    seconds -= 86400*days;
  }

  const unsigned int hours = seconds / 3600;

  if (hours > 0) {
    if (needs_space) {
      str << " ";
    }
    str << hours
        << ((hours > 1) ? " hours" : " hour");
    needs_space = true;
    seconds -= hours * 3600;
  }

  const unsigned int mins = seconds / 60;
  if (mins > 0) {
    if (needs_space) {
      str << " ";
    }
    str << mins
        << ((mins > 1) ? " minutes" : " minute");
    needs_space = true;
    seconds -= mins * 60;
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
