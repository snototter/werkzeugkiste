#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <sstream>
#include <string>
#include <vector>

// Only needed to query the library version:
#include <werkzeugkiste/version.h>

#include <werkzeugkiste/timing/stopwatch.h>

// If you #define NO_TICTOC before including tictoc.h, all
// TIC/TOC(...) calls will be disabled.
// This should be prefered over werkzeug::timing::mute()/unmute()
//#define NO_TICTOC
#include <werkzeugkiste/timing/tictoc.h>


int main(int /* argc */, char ** /* argv */) {
  namespace wtu = werkzeugkiste::timing;
  wtu::StopWatch watch;
  std::cout << "--------------------------------------------------\n"
            << "    Werkzeugkiste v" << WERKZEUGKISTE_VERSION << "\n"
            << "    Stopwatch demo\n"
            << "--------------------------------------------------\n"
            << "Underlying clock:      " << watch.ClockName()
            << "\nYears before overflow: " << std::fixed
            << std::setprecision(1) << watch.YearsUntilOverflow()
            << "\n--------------------------------------------------"
            << std::endl;

  // If you want to nicely align the TOC output, use set_tictoc_fmt:
  wtu::set_toc_fmt(true, 9, 3);

  // For the demo, we show a "growing label", so you can see the
  // effect of "aligning the labels" via set_tictoc_fmt above:
  std::stringstream label_builder;
  label_builder << "this-label-keeps-growing";

  TIC();  // Starts the default stop watch
  std::vector<std::string> prev_labels;
  for (int i = 0; i < 5; ++i) {
    std::string label = label_builder.str();
    TIC(label);  // Starts a stop watch with the given label

    // Do something
    std::this_thread::sleep_for(std::chrono::milliseconds(120));

    // Display the elapsed time for all active
    // stop watches:
    for (const auto &prev_label : prev_labels)
      TOC_MS(prev_label);
    TOC_MS(label);
    // Alternatively, you could have simply queried the
    // elapsed time, e.g. via
    // double elapsed_microsec = TOC_US(label);

    // Increase the label length for the next iteration:
    label_builder << "...";
    prev_labels.push_back(label);

    std::cout << "--------------------------------------------------\n";
  }
  return 0;
}
