#ifndef WERKZEUGKISTE_TIMING_TICTOC_H
#define WERKZEUGKISTE_TIMING_TICTOC_H

#include <werkzeugkiste/timing/timing_export.h>

#include <chrono>  // NOLINT [build/c++11]
#include <string>

namespace werkzeugkiste::timing {

/// @brief Set output format for the toc_xxx() functions.
///
/// @param print_labels_aligned If true, all timer labels
///            will be displayed at a fixed width (which
///            is determined by the longest label).
///
/// @param fixed_number_width If > 0, numbers are displayed
///            using this fixed width.
///
/// @param number_precision If > 0, defines the decimal
///            precision for displaying time measurements.
WERKZEUGKISTE_TIMING_EXPORT
void SetTocFormat(bool print_labels_aligned = false, int fixed_number_width = 0,
                  int number_precision = 0);

/// @brief Mute future toc_xxx calls, i.e. no output will be displayed.
///
/// Users of the C++ library should usually prefer the TIC/TOC macros
/// and #define NO_TICTOC instead. The mute/unmute calls are
/// intended for a (future) Python interface where it's less convenient
/// to rebuild the library.
/// With mute() you still "suffer" from unnecessary function call
/// overheads, whereas with NO_TICTOC all function calls would be
/// eliminated.
WERKZEUGKISTE_TIMING_EXPORT
void MuteToc();

/// @brief Unmute, i.e. future toc_xxx calls will display the elapsed time
/// again.
WERKZEUGKISTE_TIMING_EXPORT
void UnmuteToc();

/// @brief Starts (or restarts) a @see StopWatch
///
/// @param label You can specify a label to differentiate multiple StopWatches.
WERKZEUGKISTE_TIMING_EXPORT
void Tic(const std::string& label = std::string());

/// @brief Displays the elapsed time in seconds.
///
/// @param label Label of the corresponding @see tic() call.
WERKZEUGKISTE_TIMING_EXPORT
void TocSeconds(const std::string& label = std::string());

/// @brief Displays the elapsed time in milliseconds.
///
/// @param label Label of the corresponding @see tic() call.
WERKZEUGKISTE_TIMING_EXPORT
void TocMilliseconds(const std::string& label = std::string());

/// @brief Displays the elapsed time in microseconds.
///
/// @param label Label of the corresponding @see tic() call.
WERKZEUGKISTE_TIMING_EXPORT
void TocMicroseconds(const std::string& label = std::string());

/// @brief Displays the elapsed time in nanoseconds.
///
/// @param label Label of the corresponding @see tic() call.
WERKZEUGKISTE_TIMING_EXPORT
void TocNanoseconds(const std::string& label = std::string());

/// @brief Returns the elapsed time in seconds.
///
/// @param label Label of the corresponding @see tic() call.
WERKZEUGKISTE_TIMING_EXPORT
double TTocSeconds(const std::string& label = std::string());

/// @brief Returns the elapsed time in milliseconds.
///
/// @param label Label of the corresponding @see tic() call.
WERKZEUGKISTE_TIMING_EXPORT
double TTocMilliseconds(const std::string& label = std::string());

/// @brief Returns the elapsed time in microseconds.
///
/// @param label Label of the corresponding @see tic() call.
WERKZEUGKISTE_TIMING_EXPORT
double TTocMicroseconds(const std::string& label = std::string());

/// @brief Returns the elapsed time in nanoseconds.
///
/// @param label Label of the corresponding @see tic() call.
WERKZEUGKISTE_TIMING_EXPORT
double TTocNanoseconds(const std::string& label = std::string());

}  // namespace werkzeugkiste::timing

// NOLINTBEGIN(*-macro-usage)

/// Preprocessor macros to:
///   a) easily use TIC/TOC by simply defining 'WITH_TICTOC' before including
///      this header:
///      #define WITH_TICTOC
///      #include <werkzeugkiste/timing/tictoc.h>
///   b) easily disable all call overheads (by simply defining
///      NO_TICTOC before including tictoc.h)
#ifdef WITH_TICTOC
#define TIC(...) werkzeugkiste::timing::Tic(__VA_ARGS__)
#define TOC_SEC(...) werkzeugkiste::timing::TocSeconds(__VA_ARGS__)
#define TTOC_SEC(...) werkzeugkiste::timing::TTocSeconds(__VA_ARGS__)
#define TOC_MS(...) werkzeugkiste::timing::TocMilliseconds(__VA_ARGS__)
#define TTOC_MS(...) werkzeugkiste::timing::TTocMilliseconds(__VA_ARGS__)
#define TOC_US(...) werkzeugkiste::timing::TocMicroseconds(__VA_ARGS__)
#define TTOC_US(...) werkzeugkiste::timing::TTocMicroseconds(__VA_ARGS__)
#define TOC_NS(...) werkzeugkiste::timing::TocNanoseconds(__VA_ARGS__)
#define TTOC_NS(...) werkzeugkiste::timing::TTocNanoseconds(__VA_ARGS__)
#else  // WITH_TICTOC
#define TIC(...)
#define TOC_SEC(...)
#define TTOC_SEC(...) (-1)
#define TOC_MS(...)
#define TTOC_MS(...) (-1)
#define TOC_US(...)
#define TTOC_US(...) (-1)
#define TOC_NS(...)
#define TTOC_NS(...) (-1)
#endif  // WITH_TICTOC

// NOLINTEND(*-macro-usage)

#endif  // WERKZEUGKISTE_TIMING_TICTOC_H
