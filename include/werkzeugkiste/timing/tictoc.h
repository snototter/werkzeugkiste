#ifndef __WERKZEUGKISTE_TIMING_TICTOC_H__
#define __WERKZEUGKISTE_TIMING_TICTOC_H__

#include <chrono>  // NOLINT [build/c++11]
#include <string>

namespace werkzeugkiste {
namespace timing {
/**
 * @brief Set output format for the toc_xxx() functions.
 *
 * @param print_labels_aligned If true, all timer labels
 *            will be displayed at a fixed width (which
 *            is determined by the longest label).
 *
 * @param fixed_number_width If > 0, numbers are displayed
 *            using this fixed width.
 *
 * @param number_precision If > 0, defines the decimal
 *            precision for displaying time measurements.
 */
void set_toc_fmt(bool print_labels_aligned=false,
                 int fixed_number_width=0,
                 int number_precision=0);


/**
 * @brief Mute future toc_xxx calls, i.e. no output will be displayed.
 *
 * Users of the C++ library should usually prefer the TIC/TOC macros
 * and #define NO_TICTOC instead. The mute/unmute calls are
 * intended for the Python interface where it's less convenient
 * to rebuild the library.
 * With mute() you still "suffer" from unnecessary function call
 * overheads, whereas with NO_TICTOC all function calls would be
 * eliminated.
 */
void mute_toc();


/**
 * @brief Unmute, i.e. future toc_xxx calls will display the elapsed time again.
 */
void unmute_toc();


/**
 * @brief Starts (or restarts) a @see StopWatch
 *
 * @param label You can specify a label to differentiate
 *              multiple StopWatches
 */
void tic(const std::string &label = std::string());


/**
 * @brief Displays the elapsed time in seconds.
 *
 * @param label Label of the corresponding @see tic() call.
 */
void toc_sec(const std::string &label = std::string());


/**
 * @brief Displays the elapsed time in milliseconds.
 *
 * @param label Label of the corresponding @see tic() call.
 */
void toc_ms(const std::string &label = std::string());


/**
 * @brief Displays the elapsed time in microseconds.
 *
 * @param label Label of the corresponding @see tic() call.
 */
void toc_us(const std::string &label = std::string());


/**
 * @brief Displays the elapsed time in nanoseconds.
 *
 * @param label Label of the corresponding @see tic() call.
 */
void toc_ns(const std::string &label = std::string());


/**
 * @brief Returns the elapsed time in seconds.
 *
 * @param label Label of the corresponding @see tic() call.
 */
double ttoc_sec(const std::string &label = std::string());


/**
 * @brief Returns the elapsed time in milliseconds.
 *
 * @param label Label of the corresponding @see tic() call.
 */
double ttoc_ms(const std::string &label = std::string());


/**
 * @brief Returns the elapsed time in microseconds.
 *
 * @param label Label of the corresponding @see tic() call.
 */
double ttoc_us(const std::string &label = std::string());


/**
 * @brief Returns the elapsed time in nanoseconds.
 *
 * @param label Label of the corresponding @see tic() call.
 */
double ttoc_ns(const std::string &label = std::string());

}  // namespace timing
}  // namespace werkzeugkiste


/** Preprocessor macros to a) easily use TIC/TOC and
 *  b) easily disable all call overheads (by simply defining
 *  NO_TICTOC before including tictoc.h)
 */
#ifdef NO_TICTOC

#define TIC(...)

#define TOC_SEC(...)
#define TTOC_SEC(...) -1

#define TOC_MS(...)
#define TTOC_MS(...) -1

#define TOC_US(...)
#define TTOC_US(...) -1

#define TOC_NS(...)
#define TTOC_NS(...) -1

#else  // NO_TICTOC

#define TIC(...) werkzeugkiste::timing::tic(__VA_ARGS__)

#define TOC_SEC(...)  werkzeugkiste::timing::toc_sec(__VA_ARGS__)
#define TTOC_SEC(...) werkzeugkiste::timing::ttoc_sec(__VA_ARGS__)

#define TOC_MS(...)   werkzeugkiste::timing::toc_ms(__VA_ARGS__)
#define TTOC_MS(...)  werkzeugkiste::timing::ttoc_ms(__VA_ARGS__)

#define TOC_US(...)   werkzeugkiste::timing::toc_us(__VA_ARGS__)
#define TTOC_US(...)  werkzeugkiste::timing::ttoc_us(__VA_ARGS__)

#define TOC_NS(...)   werkzeugkiste::timing::toc_ns(__VA_ARGS__)
#define TTOC_NS(...)  werkzeugkiste::timing::ttoc_ns(__VA_ARGS__)

#endif  // NO_TICTOC

#endif  // __WERKZEUGKISTE_TIMING_TICTOC_H__
