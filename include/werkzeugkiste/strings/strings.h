#ifndef __WERKZEUGKISTE_STRINGS_STRINGS_H__
#define __WERKZEUGKISTE_STRINGS_STRINGS_H__

#include <sstream>
#include <iomanip>
#include <string>
#include <vector>


namespace werkzeugkiste {
/**
 * Common string manipulation & checks. The kind you've
 * already re-implemented/copied at least a dozen times.
 */
namespace strings {
/**
 * @brief Checks if the string ends with the given suffix.
 */
bool EndsWith(const std::string &s,
              const std::string &suffix);


/**
 * @brief Checks if the string ends with the given character.
 */
bool EndsWith(const std::string &s,
              char end);


/** @brief Checks if the given string starts with the prefix. */
bool StartsWith(const std::string &s,
                const std::string &prefix);


/**
 * @brief Checks if the string starts with the given character.
 */
bool StartsWith(const std::string &s,
              char first);



/**
 * @brief Convert string to lower case in-place.
 */
void ToLower(std::string &s);


/** @brief Returns the string converted to lower case letters. */
std::string Lower(const std::string &s);


/**
 * @brief Convert string to upper case in-place.
 */
void ToUpper(std::string &s);


/** @brief Returns the string converted to upper case letters. */
std::string Upper(const std::string &s);


/**
 * @brief Remove leading and trailing white space
 */
std::string Trim(const std::string &s);


/**
 * @brief Remove leading white space
 */
std::string LTrim(const std::string &totrim);


/**
 * @brief Remove trailing white space
 */
std::string RTrim(const std::string &totrim);


/** @brief Checks whether the string contains a valid number. */
bool IsNumeric(const std::string &s);


/**
 * @brief Tokenize string by given delimiter
 */
std::vector<std::string> Split(const std::string &s, char delim);


/**
 * @brief Tokenize string by given delimiter (return tokens within vector elems)
 */
void Split(const std::string &s, char delim, std::vector<std::string> &elems);


/** @brief Replaces all occurences of the search string
  * @param[in] str The string
  * @param[in] search String to search for
  * @param[in] replacement The replacement string
  * @return the string with the replaced part, or an empty string
  */
std::string Replace(const std::string &str, const std::string &search, const std::string &replacement);


/** @brief Clips the given URL to include only protocol and domain (strips path, etc.). */
std::string ClipUrl(const std::string &url);


/**
 * @brief Sets `protocol` to the URL's protocol, e.g. `https://`, `rtp://`, etc.
 *
 * Returns true if the `url` string contained a protocol part.
 */
bool GetUrlProtocol(const std::string &url,
                    std::string &protocol,
                    std::string &remainder);

/** @brief Returns the given URL after replacing the user's authentication data. */
std::string ObscureUrlAuthentication(const std::string &url);


/** @brief Returns a copy of 's' with all given characters removed. */
std::string Remove(const std::string &s, std::initializer_list<char> chars);


/** @brief Returns a copy of 's' where the given char is removed. */
std::string Remove(const std::string &s, const char c);


/** @brief Returns a slug representation of the string.
 *
 * The input will be converted to lower case & trimmed.
 * The number sign/hash will be replaced by "nr". Any
 * other non-alphanumeric symbols will be replaced by
 * dashes.
 * If `strip_dashes` is true, the remaining dashes will
 * then also be stripped - so " img_dir" would become "imgdir".
 */
std::string Slug(const std::string &s,
                 bool strip_dashes=false);

} // namespace strings
} // namespace werkzeug

#endif  // __WERKZEUGKISTE_STRINGS_STRINGS_H__
