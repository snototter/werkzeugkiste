#ifndef WERKZEUGKISTE_CONFIG_CASTS_H
#define WERKZEUGKISTE_CONFIG_CASTS_H

#include <cmath>
#include <limits>
#include <string>
#include <type_traits>

/// Utilities to handle configurations.
namespace werkzeugkiste::config {
namespace detail {

template <class S, class T, bool v>
struct are_integral {
  static constexpr bool value = v;
};

}  // namespace detail

template <class S, class T>
// struct are_integral : public detail::are_integral<S, T, std::is_integral_v<S>
// && std::is_integral_v<T>> {};
struct are_integral
    : public std::integral_constant<bool, std::is_integral_v<S> &&
                                              std::is_integral_v<T>> {};

template <class S, class T>
inline constexpr bool are_integral_v = are_integral<S, T>::value;

template <class S, class T>
struct are_floating_point
    : public std::integral_constant<bool, std::is_floating_point_v<S> &&
                                              std::is_floating_point_v<T>> {};

template <class S, class T>
inline constexpr bool are_floating_point_v = are_floating_point<S, T>::value;

// template <typename S, typename T>
// constexpr bool AreBothIntegral() {
//     return std::is_integral_v<S> && std::is_integral_v<T>;
// }

// template <typename S, typename T>
// constexpr bool AreBothFloatingPoint() {
//     return std::is_floating_point_v<S> && std::is_floating_point_v<T>;
// }

// template <typename S, typename T>
// constexpr bool IsPromotion() {
//     if constexpr (!std::is_arithmetic_v<S> || !std::is_arithmetic_v<T>) {
//         return false;
//     }

//     if constexpr (AreBothIntegral) {
//         return false;
//     }

//     if (std::is_floating_point_v<S> && !std::is_floating_point_v<T>) {
//         return false;
//     }

//     constexpr std::size_t src_sz = sizeof(S);
//     constexpr std::size_t tgt_sz = sizeof(T);

//     return tgt_sz > src_sz;
// }

}  // namespace werkzeugkiste::config

#endif  // WERKZEUGKISTE_CONFIG_CASTS_H
