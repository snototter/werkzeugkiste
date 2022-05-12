#include <algorithm>
#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <exception>

#include <werkzeugkiste/timing/tictoc.h>
#include <werkzeugkiste/timing/stopwatch.h>


// TODO(snototter) nice-to-have: investigate potential speed up using string_view or C strings (i.e. const char *)
// However, with newer compilers, at least the C strings version seems obsolete: https://stackoverflow.com/a/21946709

namespace werkzeugkiste {
namespace timing {
/**
 * Internal tic/toc utilities not to be
 * publicly exposed.
 */
namespace tictoc_ {
/** Dictionary holding the active stop watches for tic/toc. */
static std::unordered_map<std::string,
                          stop_watch> active_watches;

/** Toggle output in toc_xxx. */
static bool display_output = true;

/** Toggle printing labels at fixed width. */
static bool print_labels_aligned = false;


/** Keeps track of the longest tic'ed label (for aligned printing). */
static int max_label_length = 0;


/** Fixed width display of numbers in toc if > 0. */
static int number_width = 0;


/** Decimal precision for displaying time measurements in toc if > 0. */
static int number_precision = 0;


/** Template to retrieve the elapsed time for a given precision. */
template<typename _period>
double ttoc(const std::string &label) {
  auto it = active_watches.find(label);
  if (it != active_watches.end()) {
    return it->second.ElapsedAs<_period>();
  } else {
    std::stringstream s;
    s << "StopWatch \"" << label
      << "\" has not been started - check for label typo or did you forget tic()?";
    throw std::invalid_argument(s.str());
  }
}


/** Template to print the elapsed time for a given precision. */
template<typename P>
void toc_tpl(const std::string &label) {
  const auto elapsed = ttoc<typename P::period>(label);

  if (!display_output)
    return;

  // Save current stream formatting, as we might add
  // iomanipulators (if the user requested them via
  // @see set_toc_fmt())
  std::ios init(NULL);
  init.copyfmt(std::cout);

  if (!label.empty()) {
    if (print_labels_aligned && max_label_length > 0) {
      std::cout << std::setw(max_label_length + 2)
                << std::left << (label + ": ");
    } else {
      std::cout << label << ": ";
    }
  } else {
    std::cout << "Elapsed time: ";
  }

  // Set stream manipulators if set up by the user
  if (number_width > 0)
    std::cout << std::setw(number_width);

  if (number_precision > 0)
    std::cout << std::fixed << std::setprecision(number_precision);

  std::cout << std::right << elapsed << ' '
            << DurationAbbreviation<P>() << std::endl;

  // Reset stream formatting
  std::cout.copyfmt(init);
}
}  // namespace tictoc_


void set_toc_fmt(bool print_labels_aligned, int fixed_number_width, int number_precision) {
  tictoc_::print_labels_aligned = print_labels_aligned;
  tictoc_::number_width = fixed_number_width;
  tictoc_::number_precision = number_precision;
}


void mute_toc() {
  tictoc_::display_output = false;
}


void unmute_toc() {
  tictoc_::display_output = true;
}


void tic(const std::string &label) {
  auto it = tictoc_::active_watches.find(label);
  if (it == tictoc_::active_watches.end()) {
    tictoc_::active_watches.insert(std::make_pair(label, stop_watch()));
    tictoc_::max_label_length = std::max(tictoc_::max_label_length,
                                        static_cast<int>(label.length()));
  } else {
    it->second.Start();
  }
}


void toc_sec(const std::string &label) {
  tictoc_::toc_tpl<std::chrono::seconds>(label);
}


void toc_ms(const std::string &label) {
  tictoc_::toc_tpl<std::chrono::milliseconds>(label);
}


void toc_us(const std::string &label) {
  tictoc_::toc_tpl<std::chrono::microseconds>(label);
}


void toc_ns(const std::string &label) {
  tictoc_::toc_tpl<std::chrono::nanoseconds>(label);
}


double ttoc_sec(const std::string &label) {
  return tictoc_::ttoc<std::ratio<1>>(label);
}


double ttoc_ms(const std::string &label) {
  return tictoc_::ttoc<std::milli>(label);
}


double ttoc_us(const std::string &label) {
  return tictoc_::ttoc<std::micro>(label);
}


double ttoc_ns(const std::string &label) {
  return tictoc_::ttoc<std::nano>(label);
}

}  // namespace timing
}  // namespace werkzeugkiste
