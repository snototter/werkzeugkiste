#ifndef __WERKZEUGKISTE_STRINGS_STRINGS_H__
#define __WERKZEUGKISTE_STRINGS_STRINGS_H__

#include <sstream>
#include <iomanip>
#include <string>
#include <vector>


namespace werkzeugkiste {

/// Common string manipulation & checks. The kind you've
/// already re-implemented/copied at least a dozen times.
namespace strings {

/// Returns true if the string ends with the given suffix.
bool EndsWith(
    const std::string &s,
    const std::string &suffix);


/// Returns true if the string ends with the given character.
inline bool EndsWith(
    const std::string &s,
    char end) {
  if (s.length() > 0) {
    return (s.at(s.length()-1) == end);
  } else {
    return false;
  }
}


/// Returns true if the given string starts with the prefix.
bool StartsWith(
    const std::string &s,
    const std::string &prefix);


/// Returns true if the string starts with the given character.
inline bool StartsWith(
    const std::string &s,
    char first) {
  if (s.length() > 0) {
    return s[0] == first;
  } else {
    return false;
  }
}


/// Converts the string to lower case (in-place).
void ToLower(std::string &s);


/// Returns a copy, converted to lower case.
inline std::string Lower(const std::string &s) {
  std::string tmp(s);
  ToLower(tmp);
  return tmp;
}


/// Converts the string to upper case (in-place).
void ToUpper(std::string &s);


/// Returns a copy, converted to upper case.
inline std::string Upper(const std::string &s) {
  std::string tmp(s);
  ToUpper(tmp);
  return tmp;
}


/// Returns a copy with leading & trailing
/// white space removed.
std::string Trim(const std::string &s);


/// Returns a copy with leading white space removed.
std::string LTrim(const std::string &totrim);


/// Returns a copy with trailing white space removed.
std::string RTrim(const std::string &totrim);


/// Returns true if the string can be safely cast into
/// a number (checks for `long` and `double`).
bool IsNumeric(const std::string &s);


/// Tokenizes the string by the given delimiter.
std::vector<std::string> Split(
    const std::string &s,
    char delim);


/// Replaces all occurences of the given search
/// string `needle` within the `haystack`.
std::string Replace(
    const std::string &haystack,
    const std::string &needle,
    const std::string &replacement);


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
    std::string &protocol,
    std::string &remainder);


/// Returns the URL after replacing any plaintext
/// authentication data by the text `<auth>`.
std::string ObscureUrlAuthentication(const std::string &url);


/// Returns a copy where all given characters have been removed.
std::string Remove(
    const std::string &s,
    std::initializer_list<char> chars);


/// Returns a copy where the given character has been removed.
std::string Remove(
    const std::string &s,
    const char c);


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
    const std::string &s,
    bool strip_dashes=false);


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
    const std::string &s,
    std::size_t desired_length,
    int ellipsis_position = -1,
    const std::string &ellipsis = "...");

} // namespace strings
} // namespace werkzeug

#endif  // __WERKZEUGKISTE_STRINGS_STRINGS_H__
