#ifndef WERKZEUGKISTE_CONFIG_CASTS_H
#define WERKZEUGKISTE_CONFIG_CASTS_H

#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/logging.h>  //TODO remove

#include <cmath>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

// NOLINTNEXTLINE(*macro-usage)
#define WZK_CASTS_THROW_OVERFLOW(SRC, TGT, VAL)                        \
  do {                                                                 \
    std::ostringstream msg;                                            \
    msg << "Overflow when casting " << std::to_string(VAL) << " from " \
        << TypeName<SRC>() << " to " << TypeName<TGT>() << '!';        \
    throw std::domain_error(msg.str());                                \
  } while (false);

// NOLINTNEXTLINE(*macro-usage)
#define WZK_CASTS_THROW_UNDERFLOW(SRC, TGT, VAL)                        \
  do {                                                                  \
    std::ostringstream msg;                                             \
    msg << "Underflow when casting " << std::to_string(VAL) << " from " \
        << TypeName<SRC>() << " to " << TypeName<TGT>() << '!';         \
    throw std::domain_error(msg.str());                                 \
  } while (false);

// NOLINTNEXTLINE(*macro-usage)
#define WZK_CASTS_THROW_NOT_REPRESENTABLE(SRC, TGT, VAL)             \
  do {                                                               \
    std::ostringstream msg;                                          \
    msg << "Error while casting " << std::to_string(VAL) << " from " \
        << TypeName<SRC>() << " to " << TypeName<TGT>()              \
        << ". Value is not exactly representable in target type!";   \
    throw std::runtime_error(msg.str());                             \
  } while (false);

/// Utilities to handle configurations.
namespace werkzeugkiste::config {

template <class A, class B>
struct are_integral
    : public std::integral_constant<bool, std::is_integral_v<A> &&
                                              std::is_integral_v<B>> {};

template <class A, class B>
inline constexpr bool are_integral_v = are_integral<A, B>::value;

template <class A, class B>
struct are_floating_point
    : public std::integral_constant<bool, std::is_floating_point_v<A> &&
                                              std::is_floating_point_v<B>> {};

template <class A, class B>
inline constexpr bool are_floating_point_v = are_floating_point<A, B>::value;

template <typename S, typename T>
constexpr bool IsPromotable() {
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

  // We allow C-style bool conversion, i.e. 0 is false, any other
  // value is true.
  if constexpr (std::is_same_v<S, bool>) {
    return true;
  }

  return false;
}

namespace detail {

/// Helper to cast from a SIGNED to an UNSIGNED integral value.
template <typename T, typename S>
T CheckedIntegralCast(const S value, std::false_type /* same_sign */,
                      std::true_type /* src_is_signed */) {
  if (value < static_cast<S>(0)) {
    WZK_CASTS_THROW_UNDERFLOW(S, T, value);
  }

  // Source value is positive, can be safely cast to its corresponding
  // (and also a wider) unsigned representation.
  using unsigned_src = std::make_unsigned_t<S>;
  using us_limits = std::numeric_limits<unsigned_src>;

  // Get the wider unsigned type.
  using tgt_limits = std::numeric_limits<T>;
  using wider = std::conditional_t<(us_limits::digits > tgt_limits::digits),
                                   unsigned_src, T>;

  if (static_cast<wider>(value) > static_cast<wider>(tgt_limits::max())) {
    WZK_CASTS_THROW_OVERFLOW(S, T, value);
  }

  return static_cast<T>(value);
}

/// Helper to cast from an UNSIGNED to a SIGNED integral value.
template <typename T, typename S>
T CheckedIntegralCast(const S value, std::false_type /* same_sign */,
                      std::false_type /* src_is_signed */) {
  // Casting from unsigned to signed.
  // First, get the maximum target domain value as unsigned.
  using unsigned_tgt = std::make_unsigned_t<T>;
  using ut_limits = std::numeric_limits<unsigned_tgt>;

  // Use the wider (unsigned) type for the range check (because of promotion).
  using src_limits = std::numeric_limits<S>;
  using wider = std::conditional_t<(ut_limits::digits > src_limits::digits),
                                   unsigned_tgt, S>;

  // The (signed) target limits can now be safely cast to the wider unsigned
  // type for comparison.
  using tgt_limits = std::numeric_limits<T>;
  if (static_cast<wider>(value) > static_cast<wider>(tgt_limits::max())) {
    WZK_CASTS_THROW_OVERFLOW(S, T, value);
  }

  return static_cast<T>(value);
}

/// Helper/dispatcher to cast between DIFFERENTLY SIGNED integral types.
template <typename T, typename S>
T CheckedIntegralCast(const S value, std::false_type /* same_sign */) {
  using src_limits = std::numeric_limits<S>;
  using tgt_limits = std::numeric_limits<T>;
  static_assert(src_limits::is_signed != tgt_limits::is_signed);

  return CheckedIntegralCast<T, S>(
      value, std::false_type{},
      std::integral_constant<bool, src_limits::is_signed>{});
}

/// Helper to cast between integral types of the SAME SIGN.
template <typename T, typename S>
T CheckedIntegralCast(const S value, std::true_type /* same_sign */) {
  using src_limits = std::numeric_limits<S>;
  using tgt_limits = std::numeric_limits<T>;
  static_assert(src_limits::is_signed == tgt_limits::is_signed);
  static_assert(src_limits::digits >= tgt_limits::digits);

  // Same sign, but narrowing the type width. The source is wider,
  // thus we can safely promote the target domain limits:
  if (value < static_cast<S>(tgt_limits::lowest())) {
    WZK_CASTS_THROW_UNDERFLOW(S, T, value);
  }
  if (value > static_cast<S>(tgt_limits::max())) {
    WZK_CASTS_THROW_OVERFLOW(S, T, value);
  }

  return static_cast<T>(value);
}

/// Dispatcher to cast between integral types.
template <typename T, typename S>
constexpr T CheckedIntegralCast(const S value) {
  if constexpr (std::is_same_v<T, bool>) {
    return value != S{0};
  }

  using src_limits = std::numeric_limits<S>;
  using tgt_limits = std::numeric_limits<T>;
  constexpr bool same_sign = src_limits::is_signed == tgt_limits::is_signed;
  return CheckedIntegralCast<T, S>(value,
                                   std::integral_constant<bool, same_sign>{});
}

template <typename T, typename S>
T CheckedFloatingPointCast(S value) {
  static_assert(are_floating_point_v<T, S>,
                "Template parameters must be floating point types!");

  using src_limits = std::numeric_limits<S>;
  using tgt_limits = std::numeric_limits<T>;

  // Currently, only IEEE 754 (= IEC559) is supported.
  static_assert(src_limits::is_iec559 && tgt_limits::is_iec559);

  static_assert(
      src_limits::digits > tgt_limits::digits,
      "Invalid template parameters (not a narrowing floating point cast)!");

  // Handle special floating point values.
  if (std::isnan(value)) {
    return tgt_limits::quiet_NaN();
    // TODO do we need signed NaNs?
  }
  if (std::isinf(value)) {
    return (value > S{0}) ? tgt_limits::infinity() : -tgt_limits::infinity();
  }

  // Narrowing from source to target. Thus, we can safely promote the
  // target type's limits:
  if (value < static_cast<S>(tgt_limits::lowest())) {
    WZK_CASTS_THROW_UNDERFLOW(S, T, value);
  }
  if (value > static_cast<S>(tgt_limits::max())) {
    WZK_CASTS_THROW_OVERFLOW(S, T, value);
  }

  // The number is representable in the target type,
  // but it can still be truncated.
  // Promoting the casted value to the source type
  // usually results in a difference larger than the
  // source type's epsilon (unless the number is exactly
  // representable).
  // For now, this precision loss is acceptable.
  return static_cast<T>(value);
  //  const T tgt_val = static_cast<T>(value);
  //  const S src_restored = static_cast<S>(tgt_val);

  //  const S diff = std::fabs(value - src_restored);
  //  WZKLOG_ERROR(
  //        "TODO check diff: {} --> {} vs restored {}, diff: {}",
  //        value, tgt_val, src_restored, diff);
  //  if (diff > src_limits::epsilon()) {
  //    throw std::domain_error("TODO precision lost");
  //  }

  //  return tgt_val;
}

template <typename T>
constexpr T exp2(int exp) {
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

/// Returns the <min,max> pair, such that:
/// * `min` is the lowest floating point value which *will not*
///   underflow when converted to T (integral).
/// * `max` is the lowest floating point value which *will*
///   overflow when converted to T (integral).
template <typename T, typename S>
constexpr std::pair<S, S> RangeForFloatingToIntegralCast() {
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
      min_val = -exp2<S>(int_exp_bits);
    } else {
      min_val = flt_limits::lowest;
    }
  } else {
    min_val = S{0};
  }

  S max_val{};
  if constexpr (flt_exp_bits >= int_exp_bits) {
    max_val = exp2<S>(int_exp_bits);
  } else {
    max_val = flt_limits::infinity;
  }

  return std::make_pair(min_val, max_val);
}

template <typename T, typename S>
T CheckedIntegralToFloatingPointCast(S value) {
  static_assert(std::is_integral_v<S>);
  static_assert(std::is_floating_point_v<T>);

  // TODO perform range check first

  // Check if cast is lossless
  T cast = static_cast<T>(value);
  S check = static_cast<S>(cast);

  if (check != value) {
    std::ostringstream msg;
    msg << "Cannot perform lossless cast from " << TypeName<S>() << " value "
        << value << " to " << TypeName<T>() << ". Result would be " << check
        << '!';
    throw std::domain_error(msg.str());
  }
  return cast;
}

template <typename T, typename S>
T CheckedFloatingPointToIntegralCast(S value) {
  static_assert(std::is_floating_point_v<S>);
  static_assert(std::is_integral_v<T>);

  if (std::isinf(value) || std::isnan(value)) {
    std::ostringstream msg;
    msg << "Cannot represent " << (std::isinf(value) ? "inf" : "NaN") << " by "
        << TypeName<T>() << '!';
    throw std::domain_error(msg.str());
  }

  constexpr std::pair<S, S> range = RangeForFloatingToIntegralCast<T, S>();

  if (value < range.first) {
    WZK_CASTS_THROW_UNDERFLOW(S, T, value);
  }

  if (value >= range.second) {
    WZK_CASTS_THROW_OVERFLOW(S, T, value);
  }

  // It is within the range, but it could still be a fractional
  // number. Thus, we convert and check the result.
  T cast = static_cast<T>(value);
  S check = static_cast<S>(cast);

  const S diff = std::fabs(value - check);
  if (diff > std::numeric_limits<S>::epsilon()) {
    WZK_CASTS_THROW_NOT_REPRESENTABLE(S, T, value);
  }

  return cast;
}
}  // namespace detail

/// Returns the T value if S is exactly representable in the target
/// type. Otherwise, an exception will be thrown.
/// Supported casts:
/// * Casts between numeric types
/// * "Casts" to string (which returns the string representation of
///   the input number)
template <typename T, typename S>
constexpr T CheckedCast(const S value) {
  static_assert(std::is_arithmetic_v<S>);
  static_assert(std::is_arithmetic_v<T> || std::is_same_v<T, std::string>);

  // Special case: Conversion to string.
  if constexpr (std::is_same_v<T, std::string>) {
    return std::to_string(value);
  }

  if constexpr (std::is_same_v<S, T>) {
    return value;
  }

  if constexpr (are_integral_v<S, T>) {
    if constexpr (!IsPromotable<S, T>()) {
      return detail::CheckedIntegralCast<T, S>(value);
    }
    return static_cast<T>(value);
  }

  if constexpr (are_floating_point_v<S, T>) {
    if constexpr (!IsPromotable<S, T>()) {
      return detail::CheckedFloatingPointCast<T, S>(value);
    }
    return static_cast<T>(value);
  }

  if constexpr (std::is_integral_v<S> && std::is_floating_point_v<T>) {
    return detail::CheckedIntegralToFloatingPointCast<T, S>(value);
  }

  if constexpr (std::is_floating_point_v<S> && std::is_integral_v<T>) {
    return detail::CheckedFloatingPointToIntegralCast<T, S>(value);
  }

  throw std::logic_error("The requested cast is not supported!");
}

}  // namespace werkzeugkiste::config

#undef WZK_CASTS_THROW_OVERFLOW
#undef WZK_CASTS_THROW_UNDERFLOW

#endif  // WERKZEUGKISTE_CONFIG_CASTS_H
