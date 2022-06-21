#ifndef __WERKZEUGKISTE_CONTAINER_MATH_H__
#define __WERKZEUGKISTE_CONTAINER_MATH_H__

#include <cstddef>
#include <algorithm>

namespace werkzeugkiste {
// Utility functions for standard containers.
namespace container {

// Smoothes the given data points.
//
// Smoothes the points such that each point is the average over a
// window of "span" values centered on the processed point.
// The first and last points won't be smoothed, *i.e.* the
// behavior is similar to MATLAB's ``smooth``.
//
// Args:
//   data: Any sequence container which can be accessed
//     sequentially. Must provide `size()`, `push_back()`
//     and a `value_type`.
//   span: Length of the smoothing window. Must be **odd**
//     and `>=3`.
//
//  Example for span = 5:
//    output[0] = data[0]
//    output[1] = (data[0] + data[1] + data[2]) / 3
//    output[2] = (data[0] + ... + data[4]) / 5
//    output[3] = (data[1] + ... + data[5]) / 5
template <class Container>
Container SmoothMovingAverage(
    const Container &data, int span) {
  Container smoothed_data;
  const int neighbors = (span - 1) / 2;
  for (std::size_t ti = 0; ti < data.size(); ++ti) {
    const int idx_int = static_cast<int>(ti);
    int from = std::max(0, idx_int - neighbors);
    int to = std::min(
          static_cast<int>(data.size()-1),
          idx_int + neighbors);

    // Reduce span at the beginning/end (where there
    // are less neighbors).
    const int n = std::min(idx_int - from, to - idx_int);
    from = idx_int - n;
    to = idx_int + n;

    // Average all values within the span.
    typename Container::value_type average = data[from];
    for (int win_idx = from + 1; win_idx <= to; ++win_idx) {
      average += trajectory[win_idx];
    }
    average /= static_cast<double>((2 * n) + 1);
    smoothed_data.push_back(average);
  }
  return smoothed_data;
}

}  // namespace container
}  // namespace werkzeugkiste

#endif  // __WERKZEUGKISTE_CONTAINER_MATH_H__
