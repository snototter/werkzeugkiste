#ifndef WERKZEUGKISTE_STRINGS_STRINGS_H
#define WERKZEUGKISTE_STRINGS_STRINGS_H

#include <werkzeugkiste/strings/strings_export.h>

#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

/// Common string manipulation & checks. The kind you've
/// already re-implemented/copied at least a dozen times.
namespace werkzeugkiste::strings {

/// Returns true if the string ends with the given suffix.
inline constexpr bool EndsWith(std::string_view s,
    std::string_view suffix) noexcept {
  bool result{false};
  if ((s.length() > 0) && (suffix.length() > 0) &&
      (s.length() >= suffix.length())) {
    result =
        (s.compare(s.length() - suffix.length(), suffix.length(), suffix) == 0);
  }
  return result;
}

/// Returns true if the string ends with the given character.
inline constexpr bool EndsWith(std::string_view s, char end) noexcept {
  bool result{false};
  if (s.length() > 0) {
    result = (s.at(s.length() - 1) == end);
  }
  return result;
}

/// Returns true if the given string starts with the prefix.
inline constexpr bool StartsWith(std::string_view s,
    std::string_view prefix) noexcept {
  bool result{false};
  if ((s.length() > 0) && (prefix.length() > 0) &&
      (s.length() >= prefix.length())) {
    result = s.compare(0, prefix.length(), prefix) == 0;
  }
  return result;
}

/// Returns true if the string starts with the given character.
inline constexpr bool StartsWith(std::string_view s, char first) noexcept {
  return (s.length() > 0) ? (s[0] == first) : false;
}

/// Converts the string to lower case (in-place).
WERKZEUGKISTE_STRINGS_EXPORT
void ToLower(std::string& s);  // NOLINT

/// Returns a copy, converted to lower case.
inline std::string Lower(std::string_view s) {
  std::string tmp(s);
  ToLower(tmp);
  return tmp;
}

/// Converts the string to upper case (in-place).
WERKZEUGKISTE_STRINGS_EXPORT
void ToUpper(std::string& s);  // NOLINT

/// Returns a copy, converted to upper case.
inline std::string Upper(const std::string& s) {
  std::string tmp(s);
  ToUpper(tmp);
  return tmp;
}

/// @brief Returns the absolute length difference of the two strings.
/// @param str1 First string.
/// @param str2 Second string.
/// @return :math:`|len(str1) - len(str2)|`
inline std::size_t LengthDifference(std::string_view str1,
    std::string_view str2) {
  const std::size_t l1 = str1.length();
  const std::size_t l2 = str2.length();
  if (l1 < l2) {
    return l2 - l1;
  }
  return l1 - l2;
}

/// Returns a copy with leading & trailing
/// white space removed.
WERKZEUGKISTE_STRINGS_EXPORT
std::string Trim(std::string_view s);

/// Returns a copy with leading white space removed.
WERKZEUGKISTE_STRINGS_EXPORT
std::string LTrim(std::string_view totrim);

/// Returns a copy with trailing white space removed.
WERKZEUGKISTE_STRINGS_EXPORT
std::string RTrim(std::string_view totrim);

/// Returns true if the string can be safely cast into
/// either an `int64_t` or a `double` type.
WERKZEUGKISTE_STRINGS_EXPORT
bool IsNumeric(const std::string& s);

/// Tokenizes the string by the given delimiter.
///
/// Note that an empty trailing token will be skipped.
/// For example: Split("a-b-c", '-') returns the same 3 tokens (namely,
/// "a", "b" and "c") as Split("a-b-c-", '-'). For "a-b-c--", however,
/// "a", "b", "c" and "" would be returned.
WERKZEUGKISTE_STRINGS_EXPORT
std::vector<std::string> Split(std::string_view s, char delim);

/// Tokenizes the string by the given delimiter.
///
/// Note that empty tokens will be skipped.
/// For example: Tokenize("a-b-c", "-") returns the same 3 tokens (namely,
/// "a", "b" and "c") as Tokenize("a-b-c-", "-") and also "-a-b-c--".
WERKZEUGKISTE_STRINGS_EXPORT
std::vector<std::string_view> Tokenize(std::string_view s,
    std::string_view delim);

template <typename Container,
    typename Tp = std::decay_t<decltype(*begin(std::declval<Container>()))>>
inline std::string Concatenate(const Container& container,
    std::string_view delimiter = "") {
  std::ostringstream concat;
  bool preprend_delimiter = false;
  for (const auto& str : container) {
    if (preprend_delimiter) {
      concat << delimiter;
    } else {
      preprend_delimiter = true;
    }
    concat << str;
  }
  return concat.str();
}

/// Replaces all occurrences of the given search
/// string `needle` within the `haystack`.
WERKZEUGKISTE_STRINGS_EXPORT
std::string Replace(std::string_view haystack,
    std::string_view needle,
    std::string_view replacement);

/// Replaces all occurrences of the given character.
WERKZEUGKISTE_STRINGS_EXPORT
std::string Replace(std::string_view haystack, char needle, char replacement);

/// Clips the given URL string to include only the
/// protocol and domain, *i.e.* server paths & parameters
/// will be excluded.
WERKZEUGKISTE_STRINGS_EXPORT
std::string ClipUrl(const std::string& url);

/// Sets `protocol` to the URL's protocol, e.g.
/// `https://`, `rtp://`, ...
/// Returns true if the `url` string contained a
/// protocol part.
WERKZEUGKISTE_STRINGS_EXPORT
bool GetUrlProtocol(const std::string& url,
    std::string& protocol,    // NOLINT
    std::string& remainder);  // NOLINT

/// Returns the URL after replacing any plaintext
/// authentication data by the text `<auth>`.
WERKZEUGKISTE_STRINGS_EXPORT
std::string ObscureUrlAuthentication(const std::string& url);

/// Returns a copy where all given characters have been removed.
WERKZEUGKISTE_STRINGS_EXPORT
std::string Remove(std::string_view s, std::initializer_list<char> chars);

/// Returns a copy where the given character has been removed.
WERKZEUGKISTE_STRINGS_EXPORT
std::string Remove(std::string_view s, char c);

/// Returns a slug representation of the string.
///
/// The input will be converted to lower case & trimmed.
/// The number sign/hash will be replaced by "nr". Any
/// other non-alphanumeric symbols will be replaced by
/// dashes.
/// If `strip_dashes` is true, the remaining dashes will
/// then also be stripped: e.g. ` img_dir` would
/// become `imgdir`.
WERKZEUGKISTE_STRINGS_EXPORT
std::string Slug(std::string_view s, bool strip_dashes = false);

/// Returns a string with length <= `desired_length`,
/// where the customizable `ellipsis` has been inserted
/// to indicate that the input string has been clipped.
///
/// Argument ellipsis_position specifies where the ellipsis
/// will be placed:
/// * `< 0`: Left
/// * `0`: Centered
/// * `> 0`: Right
WERKZEUGKISTE_STRINGS_EXPORT
std::string Shorten(std::string_view s,
    std::size_t desired_length,
    int ellipsis_position = -1,
    std::string_view ellipsis = "...");

/// Returns the string indented by n-times the given character.
WERKZEUGKISTE_STRINGS_EXPORT
std::string Indent(std::string_view s, std::size_t n, char character = ' ');

/// @brief Returns the the minimum number of single-character edits (i.e.
/// insertions, deletions or substitutions) required to change one string into
/// the other.
/// @param str1 First string.
/// @param str2 Second string.
/// @return The edit distance >= 0.
WERKZEUGKISTE_STRINGS_EXPORT
std::size_t LevenshteinDistance(std::string_view str1, std::string_view str2);

}  // namespace werkzeugkiste::strings

#endif  // WERKZEUGKISTE_STRINGS_STRINGS_H
