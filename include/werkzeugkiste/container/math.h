#ifndef WERKZEUGKISTE_CONTAINER_MATH_H
#define WERKZEUGKISTE_CONTAINER_MATH_H

#include <algorithm>
#include <cstddef>
#include <numeric>
#include <stdexcept>

/// Utility functions for standard containers.
namespace werkzeugkiste::container {

/// Smoothes the given data points.
///
/// Smoothes the points such that each point is the average over a
/// window of "span" values centered on the processed point.
/// The first and last points won't be smoothed, *i.e.* the
/// behavior is similar to MATLAB's ``smooth``.
///
/// Args:
///   data: Any sequence container which can be accessed
///     sequentially. Must provide `size()`, `push_back()`
///     and a `value_type`.
///   window_size: Length of the smoothing window. Must
///     be **odd** and `>=3` or `<=0` (no smoothing).
///     For span 1 and 2, an invalid_argument will be thrown.
///
///  Example for span = 5:
///    output[0] = data[0]
///    output[1] = (data[0] + data[1] + data[2]) / 3
///    output[2] = (data[0] + ... + data[4]) / 5
///    output[3] = (data[1] + ... + data[5]) / 5
template <class Container>
Container SmoothMovingAverage(const Container& data, int window_size) {
  if (window_size <= 0) {
    return data;
  }

  if ((window_size < 3) || ((window_size % 2) == 0)) {
    throw std::invalid_argument("Window size must be `>= 3` and odd!");
  }

  Container smoothed_data;
  const int neighbors = (window_size - 1) / 2;
  for (std::size_t ti = 0; ti < data.size(); ++ti) {
    const int idx_int = static_cast<int>(ti);
    int from = std::max(0, idx_int - neighbors);
    int to = std::min(static_cast<int>(data.size() - 1), idx_int + neighbors);

    // Reduce window size at the beginning/end (where there
    // are less neighbors).
    const int n = std::min(idx_int - from, to - idx_int);
    from = idx_int - n;
    to = idx_int + n;

    // Average all values within the window.
    typename Container::value_type average = data[from];
    for (int win_idx = from + 1; win_idx <= to; ++win_idx) {
      average += data[win_idx];
    }
    average /= static_cast<double>((2 * n) + 1);
    smoothed_data.push_back(average);
  }
  return smoothed_data;
}

/// Computes the sum of the given STL-like container.
/// The container must hold an arithmetic type (integers, floats, etc.)
template <class Container, typename T = typename Container::value_type>
std::enable_if_t<std::is_arithmetic_v<T>, T> Sum(const Container& values) {
  if (values.size() == 0) {
    return T{0};
  }
  return std::accumulate(values.begin(), values.end(), T{0});
}

/// Computes the mean of the given STL-like container.
/// The container must hold an arithmetic type (integers, floats, etc.)
template <class Container, typename T = typename Container::value_type>
std::enable_if_t<std::is_arithmetic_v<T>, double> Mean(
    const Container& values) {
  if (values.size() == 0) {
    return 0.0;
  }
  T sum = std::accumulate(values.begin(), values.end(), T{0});
  return static_cast<double>(sum) / values.size();
}

/// Computes the minimum & maximum of the given
/// STL-like sequence container. Currently, only random access containers
/// are supported. The underlying `value_type` must support
/// comparisons via `operator<`.
template <class Container>
void MinMax(const Container& values,
    typename Container::value_type* val_min = nullptr,
    typename Container::value_type* val_max = nullptr,
    std::size_t* idx_min = nullptr,
    std::size_t* idx_max = nullptr) {
  if (values.empty()) {
    return;
  }
  std::size_t _idx_min = 0;  // NOLINT(readability-identifier-naming)
  std::size_t _idx_max = 0;  // NOLINT(readability-identifier-naming)
  for (std::size_t idx = 1; idx < values.size(); ++idx) {
    if (values[idx] < values[_idx_min]) {
      _idx_min = idx;
    }
    if (values[_idx_max] < values[idx]) {
      _idx_max = idx;
    }
  }

  if (val_min != nullptr) {
    *val_min = values[_idx_min];
  }
  if (idx_min != nullptr) {
    *idx_min = _idx_min;
  }

  if (val_max != nullptr) {
    *val_max = values[_idx_max];
  }
  if (idx_max != nullptr) {
    *idx_max = _idx_max;
  }
}

}  // namespace werkzeugkiste::container

#endif  // WERKZEUGKISTE_CONTAINER_MATH_H
