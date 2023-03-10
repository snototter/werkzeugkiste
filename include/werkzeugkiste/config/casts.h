#ifndef WERKZEUGKISTE_CONFIG_CASTS_H
#define WERKZEUGKISTE_CONFIG_CASTS_H

#include <werkzeugkiste/config/configuration.h>

#include <cmath>
#include <limits>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace werkzeugkiste::config {
// NOLINTBEGIN(readability-identifier-naming)

/// @brief Checks whether both types are integral.
template <class A, class B>
struct are_integral : public std::integral_constant<bool,
                          std::is_integral_v<A> && std::is_integral_v<B>> {};

/// @brief Helper variable template to check whether both types are integral.
template <class A, class B>
inline constexpr bool are_integral_v = are_integral<A, B>::value;

/// @brief Checks whether both types are floating point types.
template <class A, class B>
struct are_floating_point
    : public std::integral_constant<bool,
          std::is_floating_point_v<A> && std::is_floating_point_v<B>> {};

/// @brief Helper variable template to check whether both types are floating
///   point types.
template <class A, class B>
inline constexpr bool are_floating_point_v = are_floating_point<A, B>::value;

/// @brief Checks whether source type S is promotable to target type T, i.e.
///   if a type conversion without loss of precision is feasible.
///
/// See implicit conversion rules:
/// https://en.cppreference.com/w/c/language/conversion
template <typename S, typename T>
constexpr bool is_promotable() {
  if constexpr (std::is_same_v<S, T>) {
    return true;
  }

  using src_limits = std::numeric_limits<S>;
  using tgt_limits = std::numeric_limits<T>;

  if constexpr (are_floating_point_v<T, S>) {
    return (tgt_limits::digits >= src_limits::digits);
  }

  // Integral types with same sign.
  if constexpr (are_integral_v<T, S> &&
                (tgt_limits::is_signed == src_limits::is_signed)) {
    return (tgt_limits::digits >= src_limits::digits);
  }

  return false;
}

/// @brief Implementation namespace.
namespace detail {

// NOLINTNEXTLINE(*macro-usage)
#define WZK_CASTS_THROW_OVERFLOW(EXC, SRC, TGT, VAL)                      \
  do {                                                                    \
    std::ostringstream msg;                                               \
    msg << "Overflow when casting `" << std::to_string(VAL) << "` from `" \
        << TypeName<SRC>() << "` to `" << TypeName<TGT>() << "`!";        \
    throw EXC(msg.str());                                                 \
  } while (false);

// NOLINTNEXTLINE(*macro-usage)
#define WZK_CASTS_THROW_UNDERFLOW(EXC, SRC, TGT, VAL)                      \
  do {                                                                     \
    std::ostringstream msg;                                                \
    msg << "Underflow when casting `" << std::to_string(VAL) << "` from `" \
        << TypeName<SRC>() << "` to `" << TypeName<TGT>() << "`!";         \
    throw EXC(msg.str());                                                  \
  } while (false);

// NOLINTNEXTLINE(*macro-usage)
#define WZK_CASTS_THROW_NOT_REPRESENTABLE(EXC, SRC, TGT, VAL)           \
  do {                                                                  \
    std::ostringstream msg;                                             \
    msg << "Error while casting `" << std::to_string(VAL) << "` from `" \
        << TypeName<SRC>() << "` to `" << TypeName<TGT>()               \
        << "`. Value is not exactly representable in target type!";     \
    throw EXC(msg.str());                                               \
  } while (false);

/// @brief Helper to cast from a SIGNED to an UNSIGNED integral value.
template <typename T, typename S, typename E>
std::optional<T> sign_aware_integral_cast(const S value,
    std::false_type /* same_sign */,
    std::true_type /* src_is_signed */,
    bool may_throw) {
  if (value < static_cast<S>(0)) {
    if (may_throw) {
      WZK_CASTS_THROW_UNDERFLOW(E, S, T, value);
    } else {
      return std::nullopt;
    }
  }

  // Source value is positive, can be safely cast to its corresponding
  // (and also a wider) unsigned representation.
  using unsigned_src = std::make_unsigned_t<S>;
  using us_limits = std::numeric_limits<unsigned_src>;

  // Get the wider unsigned type.
  using tgt_limits = std::numeric_limits<T>;
  using wider = std::
      conditional_t<(us_limits::digits > tgt_limits::digits), unsigned_src, T>;

  if (static_cast<wider>(value) > static_cast<wider>(tgt_limits::max())) {
    if (may_throw) {
      WZK_CASTS_THROW_OVERFLOW(E, S, T, value);
    } else {
      return std::nullopt;
    }
  }

  return std::make_optional(static_cast<T>(value));
}

/// @brief Helper to cast from an UNSIGNED to a SIGNED integral value.
template <typename T, typename S, typename E>
std::optional<T> sign_aware_integral_cast(const S value,
    std::false_type /* same_sign */,
    std::false_type /* src_is_signed */,
    bool may_throw) {
  // Casting from unsigned to signed.
  // First, get the maximum target domain value as unsigned.
  using unsigned_tgt = std::make_unsigned_t<T>;
  using ut_limits = std::numeric_limits<unsigned_tgt>;

  // Use the wider (unsigned) type for the range check (because of promotion).
  using src_limits = std::numeric_limits<S>;
  using wider = std::
      conditional_t<(ut_limits::digits > src_limits::digits), unsigned_tgt, S>;

  // The (signed) target limits can now be safely cast to the wider unsigned
  // type for comparison.
  using tgt_limits = std::numeric_limits<T>;
  if (static_cast<wider>(value) > static_cast<wider>(tgt_limits::max())) {
    if (may_throw) {
      WZK_CASTS_THROW_OVERFLOW(E, S, T, value);
    } else {
      return std::nullopt;
    }
  }

  return std::make_optional(static_cast<T>(value));
}

/// @brief Helper/dispatcher to cast between integral types of DIFFERENT SIGNS.
template <typename T, typename S, typename E>
std::optional<T> integral_cast(const S value,
    std::false_type /* same_sign */,
    bool may_throw) {
  using src_limits = std::numeric_limits<S>;
  using tgt_limits = std::numeric_limits<T>;
  static_assert(src_limits::is_signed != tgt_limits::is_signed);

  return sign_aware_integral_cast<T, S, E>(value,
      std::false_type{},
      std::integral_constant<bool, src_limits::is_signed>{},
      may_throw);
}

/// @brief Helper to cast between integral types of the SAME SIGN.
template <typename T, typename S, typename E>
std::optional<T> integral_cast(const S value,
    std::true_type /* same_sign */,
    bool may_throw) {
  using src_limits = std::numeric_limits<S>;
  using tgt_limits = std::numeric_limits<T>;
  static_assert(src_limits::is_signed == tgt_limits::is_signed);
  static_assert(src_limits::digits >= tgt_limits::digits);

  // Same sign, but narrowing the type width. The source is wider,
  // thus we can safely promote the target domain limits:
  if (value < static_cast<S>(tgt_limits::min())) {
    if (may_throw) {
      WZK_CASTS_THROW_UNDERFLOW(E, S, T, value);
    } else {
      return std::nullopt;
    }
  }
  if (value > static_cast<S>(tgt_limits::max())) {
    if (may_throw) {
      WZK_CASTS_THROW_OVERFLOW(E, S, T, value);
    } else {
      return std::nullopt;
    }
  }

  return std::make_optional(static_cast<T>(value));
}

/// @brief Dispatcher to cast between integral types.
template <typename T, typename S, typename E>
constexpr std::optional<T> int_to_int(const S value, bool may_throw) {
  using src_limits = std::numeric_limits<S>;
  using tgt_limits = std::numeric_limits<T>;
  constexpr bool same_sign = src_limits::is_signed == tgt_limits::is_signed;
  return integral_cast<T, S, E>(
      value, std::integral_constant<bool, same_sign>{}, may_throw);
}

/// @brief Helper to cast between floating point types.
template <typename T, typename S, typename E>
std::optional<T> float_to_float(S value, bool may_throw) {
  static_assert(are_floating_point_v<T, S>,
      "Template parameters must be floating point types!");

  using src_limits = std::numeric_limits<S>;
  using tgt_limits = std::numeric_limits<T>;

  // Currently, only IEEE 754 (= IEC559) is supported.
  static_assert(src_limits::is_iec559 && tgt_limits::is_iec559);

  static_assert(src_limits::digits > tgt_limits::digits,
      "Invalid template parameters (not a narrowing floating point cast)!");

  // Handle special floating point values.
  if (std::isnan(value)) {
    // No need to distinguish "signed NaNs" for now.
    return tgt_limits::quiet_NaN();
  }
  if (std::isinf(value)) {
    return (value > S{0}) ? tgt_limits::infinity() : -tgt_limits::infinity();
  }

  // Narrowing from source to target. Thus, we can safely promote the
  // target type's limits:
  if (value < static_cast<S>(tgt_limits::lowest())) {
    if (may_throw) {
      WZK_CASTS_THROW_UNDERFLOW(E, S, T, value);
    } else {
      return std::nullopt;
    }
  }
  if (value > static_cast<S>(tgt_limits::max())) {
    if (may_throw) {
      WZK_CASTS_THROW_OVERFLOW(E, S, T, value);
    } else {
      return std::nullopt;
    }
  }

  // The number is representable in the target type,
  // but it could still be truncated:
  // Promoting the casted value to the source type
  // typically results in a difference which is larger than the
  // **source type's epsilon** (unless the number is exactly
  // representable).
  // For now, this minute "precision loss" is acceptable.
  return std::make_optional(static_cast<T>(value));
  //  const T tgt_val = static_cast<T>(value);
  //  const S src_restored = static_cast<S>(tgt_val);
  //  const S diff = std::fabs(value - src_restored);
  //  if (diff > src_limits::epsilon()) {
  //    // Will (almost) always throw:
  //    throw std::domain_error("Difference exceeds S::epsilon()");
  //  }
  //  return tgt_val;
}

/// @brief A constexpr-compatible computation of $2^exp$.
template <typename T>
constexpr T pow2(int exp) {
  if (exp < 0) {
    throw std::logic_error("Exponent must be >= 0");
  }

  T val{1};
  constexpr T mul{2};
  for (int i{0}; i < exp; ++i) {
    val *= mul;
  }
  return val;
}

/// @brief Returns the value range for the floating point to integer cast.
/// @return Returns the <min,max> pair, such that:
/// * `min` is the lowest floating point value which *will not*
///   underflow when converted to T (integral).
/// * `max` is the lowest floating point value which *will*
///   overflow when converted to T (integral).
template <typename T, typename S>
constexpr std::pair<S, S> float_to_int_range() {
  static_assert(std::is_floating_point_v<S>);
  static_assert(std::is_integral_v<T>);

  using flt_limits = std::numeric_limits<S>;
  using int_limits = std::numeric_limits<T>;

  // Check how many bits we have in the exponent to
  // get the representable powers of 2.
  // This will only work for numbers based on the
  // binary representation (i.e. all the standard
  // float/int implementations).
  static_assert(int_limits::radix == 2);
  constexpr int int_exp_bits = int_limits::digits;

  static_assert(flt_limits::radix == 2);
  static_assert(flt_limits::is_iec559);
  constexpr int flt_exp_bits = flt_limits::max_exponent - 1;

  S min_val{};
  if constexpr (int_limits::is_signed) {
    if constexpr (int_exp_bits < flt_exp_bits) {
      min_val = -pow2<S>(int_exp_bits);
    } else {
      min_val = flt_limits::lowest;
    }
  } else {
    min_val = S{0};
  }

  S max_val{};
  if constexpr (int_exp_bits < flt_exp_bits) {
    max_val = pow2<S>(int_exp_bits);
  } else {
    max_val = flt_limits::infinity;
  }

  return std::make_pair(min_val, max_val);
}

/// @brief Helper to cast from floating point to integral types.
template <typename T, typename S, typename E>
std::optional<T> float_to_int(S value, bool may_throw) {
  static_assert(std::is_floating_point_v<S>);
  static_assert(std::is_integral_v<T>);

  if (std::isinf(value) || std::isnan(value)) {
    if (may_throw) {
      std::ostringstream msg;
      msg << "Cannot represent " << (std::isinf(value) ? "inf" : "NaN")
          << " by " << TypeName<T>() << '!';
      throw E{msg.str()};
    } else {
      return std::nullopt;
    }
  }

  constexpr std::pair<S, S> range = float_to_int_range<T, S>();

  if (value < range.first) {
    if (may_throw) {
      WZK_CASTS_THROW_UNDERFLOW(E, S, T, value);
    } else {
      return std::nullopt;
    }
  }

  if (value >= range.second) {
    if (may_throw) {
      WZK_CASTS_THROW_OVERFLOW(E, S, T, value);
    } else {
      return std::nullopt;
    }
  }

  // It is within the range, but it could still be a fractional
  // number. Thus, we convert and check the result.
  const T cast = static_cast<T>(value);
  const S check = static_cast<S>(cast);

  const S diff = std::fabs(value - check);
  if (diff > std::numeric_limits<S>::epsilon()) {
    if (may_throw) {
      WZK_CASTS_THROW_NOT_REPRESENTABLE(E, S, T, value);
    } else {
      return std::nullopt;
    }
  }

  return std::make_optional(cast);
}

/// @brief Helper to cast from integral to floating point types.
template <typename T, typename S, typename E>
std::optional<T> int_to_float(S value, bool may_throw) {
  static_assert(std::is_integral_v<S>);
  static_assert(std::is_floating_point_v<T>);

  // TODO perform range check first
  using flt_limits = std::numeric_limits<T>;
  using int_limits = std::numeric_limits<S>;

  // Check how many bits we have in the exponent to
  // get the representable powers of 2.
  // This will only work for numbers based on the
  // binary representation (i.e. all the standard
  // float/int implementations).
  static_assert(int_limits::radix == 2);
  constexpr int int_exp_bits = int_limits::digits;

  static_assert(flt_limits::radix == 2);
  static_assert(flt_limits::is_iec559);
  constexpr int flt_exp_bits = flt_limits::max_exponent - 1;

  if constexpr (int_exp_bits > flt_exp_bits) {
    // The integer type can represent a larger range than the floating point
    // type.
    constexpr S min_val = int_limits::is_signed ? -pow2<S>(flt_exp_bits) : 0;
    constexpr S max_val = pow2<S>(flt_exp_bits);

    if (value < min_val) {
      if (may_throw) {
        WZK_CASTS_THROW_UNDERFLOW(E, S, T, value);
      }
      return std::nullopt;
    }

    if (value > max_val) {
      if (may_throw) {
        WZK_CASTS_THROW_OVERFLOW(E, S, T, value);
      }
      return std::nullopt;
    }
  }

  // Check if cast is lossless
  const T cast = static_cast<T>(value);
  const std::optional<S> check =
      float_to_int<S, T, E>(cast, /*may_throw=*/false);

  if (!check.has_value() || (check.value() != value)) {
    if (may_throw) {
      std::ostringstream msg;
      msg << "Cannot perform lossless cast from " << TypeName<S>() << " value "
          << value << " to " << TypeName<T>() << '!';
      if (check.has_value()) {
        msg << " Result would be " << check.value() << '.';
      }
      throw E{msg.str()};
    }
    return std::nullopt;
  }
  return std::make_optional(cast);
}

/// @brief Dispatcher to cast a number from source type S to target type T.
/// @tparam T Target type.
/// @tparam S Source type.
/// @tparam E Exception type to be thrown if `may_throw` is true.
/// @param value The value to be cast.
/// @param may_throw If true, an exception E will be thrown for invalid casts,
///   i.e. for values which are not exactly representable by the target type.
///   If false, no exception will be thrown and the validity of the cast can
///   be checked via the optional return value.
template <typename T, typename S, typename E = std::domain_error>
constexpr std::optional<T> numcast(const S value, const bool may_throw) {
  static_assert(std::is_arithmetic_v<S>);
  static_assert(std::is_arithmetic_v<T>);

  (void)may_throw;  // Silence unused parameter warning

  if constexpr (std::is_same_v<S, T>) {
    return std::make_optional(value);
  }

  if constexpr (std::is_same_v<T, bool>) {
    // Allow C-style cast to boolean, i.e. if a number is (close to) 0,
    // it is interpreted as `false`. Otherwise, it will be cast to `true`.
    if constexpr (std::is_floating_point_v<S>) {
      return std::make_optional(
          std::fabs(value) > std::numeric_limits<S>::epsilon());
    }

    if constexpr (std::is_integral_v<S>) {
      return std::make_optional(value != 0);
    }
  } else if constexpr (std::is_same_v<S, bool>) {
    // A boolean can be converted to 0 or 1 in the target type as we
    // assume that the numeric types include 0. (No support for
    // custom exotic number implementations.)
    return std::make_optional(value ? static_cast<T>(1) : static_cast<T>(0));
  } else {
    if constexpr (is_promotable<S, T>()) {
      return std::make_optional(static_cast<T>(value));
    }

    // int -> int
    if constexpr (!is_promotable<S, T>() && are_integral_v<S, T>) {
      return detail::int_to_int<T, S, E>(value, may_throw);
    }

    // float -> float
    if constexpr (!is_promotable<S, T>() && are_floating_point_v<S, T>) {
      return detail::float_to_float<T, S, E>(value, may_throw);
    }

    // int -> float
    if constexpr (std::is_integral_v<S> && std::is_floating_point_v<T>) {
      return detail::int_to_float<T, S, E>(value, may_throw);
    }

    // float -> int
    if constexpr (std::is_floating_point_v<S> && std::is_integral_v<T>) {
      return detail::float_to_int<T, S, E>(value, may_throw);
    }
  }

  // LCOV_EXCL_START
  return std::nullopt;
  // LCOV_EXCL_STOP
}

#undef WZK_CASTS_THROW_OVERFLOW
#undef WZK_CASTS_THROW_UNDERFLOW

}  // namespace detail

/// @brief Returns the value as type T if it can be exactly represented in the
///   target type, or std::nullopt otherwise.
///
/// @tparam T The target type.
/// @tparam S The input type.
/// @param value The value to be casted.
template <typename T, typename S>
std::optional<T> safe_numcast(const S value) noexcept {
  constexpr bool may_throw = false;
  try {
    return detail::numcast<T, S>(value, may_throw);
  } catch (...) {
    return std::nullopt;
  }
}

/// @brief Returns the value as type T iff it can be exactly represented in the
///   target type.
///
/// If the value is not exactly representable in the target type (i.e. the cast
/// is not possible without losing precision/information), an exception of type
/// E will be thrown.
///
/// @tparam T The target type.
/// @tparam S The input type.
/// @tparam E Exception which will be thrown if the cast is not feasible.
/// @param value The value to be casted.
template <typename T, typename S, typename E = std::domain_error>
T checked_numcast(const S value) {
  constexpr bool may_throw = true;

  const auto opt = detail::numcast<T, S, E>(value, may_throw);

  if (opt.has_value()) {
    return opt.value();
  }

  // LCOV_EXCL_START
  std::string msg{"The requested cast from `"};
  msg += TypeName<S>();
  msg += "` to `";
  msg += TypeName<T>();
  msg += "` is not supported!";
  throw std::logic_error{msg};
  // LCOV_EXCL_STOP
}

// NOLINTEND(readability-identifier-naming)
}  // namespace werkzeugkiste::config

#endif  // WERKZEUGKISTE_CONFIG_CASTS_H
