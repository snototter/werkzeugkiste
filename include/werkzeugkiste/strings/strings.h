#ifndef WERKZEUGKISTE_STRINGS_STRINGS_H
#define WERKZEUGKISTE_STRINGS_STRINGS_H

#include <sstream>
#include <iomanip>
#include <string>
#include <string_view>
#include <vector>


namespace werkzeugkiste {

/// Common string manipulation & checks. The kind you've
/// already re-implemented/copied at least a dozen times.
namespace strings {

/// Returns true if the string ends with the given suffix.
inline constexpr
bool EndsWith(
    std::string_view s,
    std::string_view suffix) noexcept {
  bool result {false};
  if ((s.length() > 0)
      && (suffix.length() > 0)
      && (s.length() >= suffix.length())) {
    result = (s.compare(
                s.length() - suffix.length(), suffix.length(),
                suffix) == 0);
  }
  return result;
}


/// Returns true if the string ends with the given character.
inline constexpr
bool EndsWith(
    std::string_view s,
    char end) noexcept {
  bool result {false};
  if (s.length() > 0) {
    result = (s.at(s.length()-1) == end);
  }
  return result;
}


/// Returns true if the given string starts with the prefix.
inline constexpr
bool StartsWith(
    std::string_view s,
    std::string_view prefix) noexcept {
  bool result {false};
  if ((s.length() > 0)
      && (prefix.length() > 0)
      && (s.length() >= prefix.length())) {
    result = s.compare(0, prefix.length(), prefix) == 0;
  }
  return result;
}


/// Returns true if the string starts with the given character.
inline constexpr
bool StartsWith(
    std::string_view s,
    char first) noexcept {
  return (s.length() > 0) ? (s[0] == first) : false;
}


/// Converts the string to lower case (in-place).
void ToLower(std::string &s);  // NOLINT


/// Returns a copy, converted to lower case.
inline std::string Lower(std::string_view s) {
  std::string tmp(s);
  ToLower(tmp);
  return tmp;
}


/// Converts the string to upper case (in-place).
void ToUpper(std::string &s);  // NOLINT


/// Returns a copy, converted to upper case.
inline std::string Upper(const std::string &s) {
  std::string tmp(s);
  ToUpper(tmp);
  return tmp;
}


/// Returns a copy with leading & trailing
/// white space removed.
std::string Trim(std::string_view s);


/// Returns a copy with leading white space removed.
std::string LTrim(std::string_view totrim);


/// Returns a copy with trailing white space removed.
std::string RTrim(std::string_view totrim);


/// Returns true if the string can be safely cast into
/// eiter an `int64_t` or a `double` type.
bool IsNumeric(const std::string &s);


/// Tokenizes the string by the given delimiter.
std::vector<std::string> Split(
    std::string_view s,
    char delim);


/// Replaces all occurences of the given search
/// string `needle` within the `haystack`.
std::string Replace(
    std::string_view haystack,
    std::string_view needle,
    std::string_view replacement);


/// Replaces all occurences of the given character.
std::string Replace(
    std::string_view haystack,
    char needle,
    char replacement);


/// Clips the given URL string to include only the
/// protocol and domain, *i.e.* server paths & parameters
/// will be excluded.
std::string ClipUrl(const std::string &url);


/// Sets `protocol` to the URL's protocol, e.g.
/// `https://`, `rtp://`, ...
/// Returns true if the `url` string contained a
/// protocol part.
bool GetUrlProtocol(
    const std::string &url,
    std::string &protocol,   // NOLINT
    std::string &remainder); // NOLINT


/// Returns the URL after replacing any plaintext
/// authentication data by the text `<auth>`.
std::string ObscureUrlAuthentication(const std::string &url);


/// Returns a copy where all given characters have been removed.
std::string Remove(
    std::string_view s,
    std::initializer_list<char> chars);


/// Returns a copy where the given character has been removed.
std::string Remove(
    std::string_view s,
    char c);


/// Returns a slug representation of the string.
///
/// The input will be converted to lower case & trimmed.
/// The number sign/hash will be replaced by "nr". Any
/// other non-alphanumeric symbols will be replaced by
/// dashes.
/// If `strip_dashes` is true, the remaining dashes will
/// then also be stripped: e.g. ` img_dir` would
/// become `imgdir`.
std::string Slug(
    std::string_view s,
    bool strip_dashes = false);


/// Returns a string with length <= `desired_length`,
/// where the customizable `ellipsis` has been inserted
/// to indicate that the input string has been clipped.
///
/// Argument ellipsis_position specifies where the ellipsis
/// will be placed:
/// * `< 0`: Left
/// * `0`: Centered
/// * `> 0`: Right
std::string Shorten(
    std::string_view s,
    std::size_t desired_length,
    int ellipsis_position = -1,
    std::string_view ellipsis = "...");


/// Returns the string indented by n-times the given character.
std::string Indent(
    std::string_view s,
    std::size_t n,
    char character=' ');

} // namespace strings
} // namespace werkzeugkiste

#endif  // WERKZEUGKISTE_STRINGS_STRINGS_H
