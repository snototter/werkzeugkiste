#include <algorithm>
#include <iterator>
#include <fstream>
#include <sstream>
#include <cctype>
#include <unordered_map>

#include <werkzeugkiste/strings/strings.h>
#include <werkzeugkiste_macros.h>

namespace werkzeugkiste {
namespace strings {

namespace slug_ {
//TODO should we use char32_t instead?
//TODO # -- nr
std::unordered_map<std::string, std::string> replacements {
  //TODO expand list: https://unicode-table.com/en/blocks/latin-1-supplement/

  // ASCII
  {"!", ""}, {"\"", ""}, {"#", "nr"},
  {"$", "dollar"}, {"%", "pc"}, {"&", "and"},
  {"'", ""}, {",", ""}, {".", "-"},
  {"/", "-"}, {":", ""}, {";", ""},
  {"<", "gt"}, {"=", "eq"}, {">", "lt"},
  {"?", ""}, {"@", "at"},
  {"\\", "-"}, {"^", ""},
  {"_", "-"}, {"`", ""}, // Grave accent
  //TODO tilde?

  // Latin 1 supplement
  {"´", ""}, // Acute accent

  {"\u00a9", "(c)"},  // Copyright sign
  {"\u00ae", "(r)"},  // Registered sign
  {"\u00b1", "pm"},   // Plus-minus sign
  {"\u2213", "mp"},   // Minus-plus sign
  {"\u00b0", "deg"},  // Degree sign
  {"\u00b5", "mu"},   // Mu/micro sign
  {"\u00b7", "cdot"},   // Middle dot

  {"\u00a3", "pound"},
  {"\u00a5", "yen"},
  {"\u00a7", "par"}, // Paragraph
  {"\u20ac", "euro"},

  {"\u00d7", "x"}, // times

  {"\u00c5", "Ae"}, // Capital A with diaeresis
  {"\u00e4", "ae"}, // Small a with diaeresis
  {"\u00cb", "E"}, // Capital E with diaeresis
  {"\u00eb", "e"}, // Small e with diaeresis
  {"\u00d6", "Oe"}, // Capital O with diaeresis
  {"\u00f6", "oe"}, // Small o with diaeresis
  {"\u00dc", "Ue"}, // Capital U with diaeresis
  {"\u00fc", "Ue"}, // Small u with diaeresis
  //TODO continue
};
}  // namespace slug_

void ToLower(std::string &s) {
  std::transform(
        s.begin(), s.end(), s.begin(), ::tolower);
}


void ToUpper(std::string &s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 ::toupper);
}


std::string LTrim(std::string_view totrim) {
  std::string s(totrim);
  s.erase(
        s.begin(),
        std::find_if(
          s.begin(), s.end(),
          std::not1(std::ptr_fun<int, int>(std::isspace))));
  return s;
}


std::string RTrim(std::string_view totrim) {
  std::string s(totrim);
  s.erase(
        std::find_if(
          s.rbegin(), s.rend(),
          std::not1(std::ptr_fun<int, int>(std::isspace))
        ).base(),
        s.end());
  return s;
}


std::string Trim(std::string_view s) {
  return LTrim(RTrim(s));
}


bool IsNumeric(const std::string &s) {
  bool is_numeric{false};
  if (s.length() > 0) {
    char *pd, *pl;

    // Check long
    long dummyl = strtol(s.c_str(), &pl, 10);
    WERKZEUGKISTE_UNUSED_VAR(dummyl);

    // Check double
    double dummyd = strtod(s.c_str(), &pd);
    WERKZEUGKISTE_UNUSED_VAR(dummyd);

    is_numeric = (*pd == 0) || (*pl == 0);
  }
  return is_numeric;
}


std::vector<std::string> Split(std::string_view s,
    char delim) {
  std::vector<std::string> elems;
  std::istringstream ss(std::string(s), std::ios_base::in);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}


std::string Replace(std::string_view haystack,
    std::string_view needle,
    std::string_view replacement) {
  std::string result{haystack};
  if ((haystack.length() == 0) || (needle.length() == 0)) {
    return result;
  }

  std::size_t start_pos = result.find(needle);
  while (start_pos != std::string::npos) {
    result.replace(start_pos, needle.length(), replacement);
    start_pos = result.find(needle);
  }

  return result;
}


std::string Replace(
    std::string_view haystack, char needle, char replacement) {
  std::string cp(haystack);
  std::replace(
        cp.begin(), cp.end(),
        needle, replacement);
  return cp;
}

//TODO continue string_view replacements below
bool GetUrlProtocol(
    const std::string &url,
    std::string &protocol,
    std::string &remainder) {
  const std::size_t protocol_pos = url.find("://");
  if (protocol_pos == std::string::npos) {
    protocol = "";
    remainder = url;
    return false;
  } else {
    protocol = url.substr(0, protocol_pos+3);
    remainder = url.substr(protocol_pos+3);
    return true;
  }
}

//TODO(feature-request) SplitUrl
//  extract protocol/schema, userinfo, host, port,
//  path, query, and fragment of a URL/URI


std::string ObscureUrlAuthentication(
    const std::string &url) {
  std::string protocol, clipped;
  const bool has_protocol = GetUrlProtocol(
        url, protocol, clipped);

  const std::size_t at_pos = clipped.find('@');
  if (at_pos == std::string::npos) {
    return url;
  }

  const std::string obscured = "<auth>" + clipped.substr(at_pos);
  if (has_protocol) {
    return protocol + obscured;
  }
  return obscured;
}


std::string ClipUrl(const std::string &url) {
  std::string protocol, clipped;
  const bool has_protocol = GetUrlProtocol(
        url, protocol, clipped);

  // Special handling of file:// URLs, as there's no
  // authentication information. But potentially lots
  // of forward slashes:
  if (has_protocol
      && (Lower(protocol).compare("file://") == 0)) {
    return url;
  }

  // Strip path
  std::size_t path_del_pos = clipped.find('/');
  if (path_del_pos != std::string::npos) {
    clipped = clipped.substr(0, path_del_pos);
  }

  // Strip parameters
  path_del_pos = clipped.find('?');
  if (path_del_pos != std::string::npos) {
    clipped = clipped.substr(0, path_del_pos);
  }

  if (has_protocol) {
    return protocol + ObscureUrlAuthentication(clipped);
  } else { // NOLINT
    return ObscureUrlAuthentication(clipped);
  }
}


std::string Remove(std::string_view s, const char c) {
  std::string removed;
  std::remove_copy(
        s.begin(), s.end(),
        std::back_inserter(removed), c);
  return removed;
}


std::string Remove(
    std::string_view s,
    std::initializer_list<char> chars) {
  std::string copy(s);
  for (const auto c : chars) {
    copy = Remove(copy, c);
  }
  return copy;
}


std::string Slug(
    std::string_view s,
    bool strip_dashes) {
  std::string replaced = Trim(s);

  //TODO a) inefficient
  //TODO b) update documentation

  for (const auto &replacement : slug_::replacements) {
    replaced = Replace(replaced, replacement.first, replacement.second);
  }
  replaced = Lower(replaced);

  std::ostringstream out;
  // Start with flag set to return "-" if the string
  // contains exclusively non-alphanumeric characters
  bool prev_alphanum = true;
  for (std::size_t i = 0; i < replaced.length(); ++i) {
    if (std::isalnum(replaced[i]) != 0) {
      out << replaced[i];
      prev_alphanum = true;
    } else if (prev_alphanum && !strip_dashes) {
      out << '-';
      prev_alphanum = false;
    }
  }
  return out.str();
}


std::string Shorten(
    std::string_view s,
    std::size_t desired_length,
    int ellipsis_position,
    std::string_view ellipsis) {
  if (s.empty() || (desired_length >= s.length())) {
    return std::string(s);
  }

  if (desired_length == 0) {
    return "";
  }

  if (desired_length < ellipsis.length()) {
    std::ostringstream msg;
    msg << "Desired length (" << desired_length
        << ") is shorter than the ellipsis (\""
        << ellipsis << "\", length "
        << ellipsis.length() << ")!";
    throw std::invalid_argument(msg.str());
  }

  std::ostringstream shortened;
  const std::size_t remaining_text_len = desired_length - ellipsis.length();
  if (ellipsis_position == 0) {
    //center
    const std::size_t num_left = remaining_text_len / 2;
    const std::size_t num_right = remaining_text_len - num_left;

    shortened << s.substr(0, num_left)
              << ellipsis
              << s.substr(s.length() - num_right);
  } else if (ellipsis_position < 0){
    // left
    shortened << ellipsis
              << s.substr(s.length() - remaining_text_len);
  } else {
    // right
    shortened << s.substr(0, remaining_text_len)
              << ellipsis;
  }
  return shortened.str();
}


std::string Indent(
    std::string_view s,
    std::size_t n,
    char character) {
  std::string out;
  out.reserve(s.length() + n);
  for (std::size_t idx = 0; idx < n; ++idx) {
    out[idx] = character;
  }

  for (std::size_t idx = 0; idx < s.length(); ++idx) {
    out[idx + n] = s[idx];
  }

  return out;
}

}  // namespace strings
}  // namespace werkzeugkiste
