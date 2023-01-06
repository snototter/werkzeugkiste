#include <algorithm>
#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <exception>

#include <werkzeugkiste/timing/tictoc.h>
#include <werkzeugkiste/timing/stopwatch.h>


// TODO(snototter) nice-to-have: investigate potential speed up using string_view or C strings (i.e. const char *)
// However, with newer compilers, at least the C strings version seems obsolete: https://stackoverflow.com/a/21946709

namespace werkzeugkiste::timing {

/// Internal tic/toc utilities not to be publicly exposed.
namespace tictoc_internals {

/// Dictionary holding the active stop watches for tic/toc.
static std::unordered_map<std::string, StopWatch> active_watches;


/// Toggle output in toc_xxx.
static bool display_output = true;


/// Toggle printing labels at fixed width.
static bool print_labels_aligned = false;


/// Keeps track of the longest tic'ed label (for aligned printing).
static int max_label_length = 0;


/// Fixed width display of numbers in toc if > 0.
static int number_width = 0;


/// Decimal precision for displaying time measurements in toc if > 0.
static int number_precision = 0;


/// Template to retrieve the elapsed time for a given precision.
template<typename Period>
double TToc(const std::string &label) {
  auto it = active_watches.find(label);
  if (it == active_watches.end()) {
    std::ostringstream s;
    s << "StopWatch \"" << label
      << "\" has not been started - check for label typo or did you forget tic()?";
    throw std::invalid_argument(s.str());
  }
  return it->second.ElapsedAs<Period>();
}


/// Template to print the elapsed time for a given precision.
template<typename P>
void TocTemplate(const std::string &label) {
  const auto elapsed = TToc<typename P::period>(label);

  if (!display_output) {
    return;
  }

  // Save current stream formatting, as we might add
  // iomanipulators (if the user requested them via
  // @see SetTocFormat())
  std::ios init(nullptr);
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
  if (number_width > 0) {
    std::cout << std::setw(number_width);
  }

  if (number_precision > 0) {
    std::cout << std::fixed << std::setprecision(number_precision);
  }

  std::cout << std::right << elapsed << ' '
            << DurationAbbreviation<P>() << std::endl;

  // Reset stream formatting
  std::cout.copyfmt(init);
}
}  // namespace tictoc_internals


void SetTocFormat(
    bool print_labels_aligned, int fixed_number_width, int number_precision) {
  tictoc_internals::print_labels_aligned = print_labels_aligned;
  tictoc_internals::number_width = fixed_number_width;
  tictoc_internals::number_precision = number_precision;
}


void MuteToc() {
  tictoc_internals::display_output = false;
}


void UnmuteToc() {
  tictoc_internals::display_output = true;
}


void Tic(const std::string &label) {
  auto it = tictoc_internals::active_watches.find(label);
  if (it == tictoc_internals::active_watches.end()) {
    tictoc_internals::active_watches.insert(std::make_pair(label, StopWatch()));
    tictoc_internals::max_label_length = std::max(
          tictoc_internals::max_label_length, static_cast<int>(label.length()));
  } else {
    it->second.Start();
  }
}


void TocSeconds(const std::string &label) {
  tictoc_internals::TocTemplate<std::chrono::seconds>(label);
}


void TocMilliseconds(const std::string &label) {
  tictoc_internals::TocTemplate<std::chrono::milliseconds>(label);
}


void TocMicroseconds(const std::string &label) {
  tictoc_internals::TocTemplate<std::chrono::microseconds>(label);
}


void TocNanoseconds(const std::string &label) {
  tictoc_internals::TocTemplate<std::chrono::nanoseconds>(label);
}


double TTocSeconds(const std::string &label) {
  return tictoc_internals::TToc<std::ratio<1>>(label);
}


double TTocMilliseconds(const std::string &label) {
  return tictoc_internals::TToc<std::milli>(label);
}


double TTocMicroseconds(const std::string &label) {
  return tictoc_internals::TToc<std::micro>(label);
}


double TTocNanoseconds(const std::string &label) {
  return tictoc_internals::TToc<std::nano>(label);
}

}  // namespace werkzeugkiste::timing
