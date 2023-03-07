#include <werkzeugkiste/strings/strings.h>
#include <werkzeugkiste_macros.h>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iterator>
#include <sstream>
#include <unordered_map>

namespace werkzeugkiste::strings {

namespace detail {
// TODO should we use char32_t instead?
// TODO # -- nr
const std::unordered_map<std::string, std::string>& Replacements() {
  static const std::unordered_map<std::string, std::string> replacements{
      // TODO expand list:
      // https://unicode-table.com/en/blocks/latin-1-supplement/

      // ASCII
      {"!", ""},
      {"\"", ""},
      {"#", "nr"},
      {"$", "dollar"},
      {"%", "pc"},
      {"&", "and"},
      {"'", ""},
      {",", ""},
      {".", "-"},
      {"/", "-"},
      {":", ""},
      {";", ""},
      {"<", "gt"},
      {"=", "eq"},
      {">", "lt"},
      {"?", ""},
      {"@", "at"},
      {"\\", "-"},
      {"^", ""},
      {"_", "-"},
      {"`", ""},  // Grave accent
      {"~", ""},

      // Latin 1 supplement
      {"´", ""},  // Acute accent

      {"\u00a9", "(c)"},   // Copyright sign
      {"\u00ae", "(r)"},   // Registered sign
      {"\u00b1", "pm"},    // Plus-minus sign
      {"\u2213", "mp"},    // Minus-plus sign
      {"\u00b0", "deg"},   // Degree sign
      {"\u00b5", "mu"},    // Mu/micro sign
      {"\u00b7", "cdot"},  // Middle dot

      {"\u00a3", "pound"},
      {"\u00a5", "yen"},
      {"\u00a7", "par"},  // Paragraph
      {"\u20ac", "euro"},

      {"\u00d7", "x"},  // times

      {"\u00c4", "Ae"},  // Capital A with diaeresis
      {"\u00c5", "A"},   // Capital A with ring
      {"\u00e4", "ae"},  // Small a with diaeresis
      {"\u00cb", "E"},   // Capital E with diaeresis
      {"\u00eb", "e"},   // Small e with diaeresis
      {"\u00d6", "Oe"},  // Capital O with diaeresis
      {"\u00f6", "oe"},  // Small o with diaeresis
      {"\u00dc", "Ue"},  // Capital U with diaeresis
      {"\u00fc", "Ue"},  // Small u with diaeresis
      {"\u00df", "sz"},  // Small sharp s
      {"\u1e9e", "Sz"},  // Capital sharp s
      // TODO continue

      {"\u2026", "..."},  // Ellipsis
  };
  return replacements;
}
}  // namespace detail

void ToLower(std::string& s) {
  std::transform(s.begin(), s.end(), s.begin(), ::tolower);
}

void ToUpper(std::string& s) {
  std::transform(s.begin(), s.end(), s.begin(), ::toupper);
}

std::string LTrim(std::string_view totrim) {
  std::string s(totrim);
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c) {
    return std::isspace(c) == 0;
  }));
  return s;
}

std::string RTrim(std::string_view totrim) {
  std::string s(totrim);
  s.erase(std::find_if(
              s.rbegin(), s.rend(), [](int c) { return std::isspace(c) == 0; })
              .base(),
      s.end());
  return s;
}

std::string Trim(std::string_view s) { return LTrim(RTrim(s)); }

bool IsNumeric(const std::string& s) {
  // TODO replace by `std::stoll`
  bool is_numeric{false};
  if (s.length() > 0) {
    // Check long long
    char* pl_end{};
    // NOLINTNEXTLINE(*-magic-numbers)
    const int64_t dummyl = strtoll(s.c_str(), &pl_end, 10);
    WERKZEUGKISTE_UNUSED_VAR(dummyl);

    // Check double
    char* pd_end{};
    const double dummyd = strtod(s.c_str(), &pd_end);
    WERKZEUGKISTE_UNUSED_VAR(dummyd);

    is_numeric = (*pd_end == 0) || (*pl_end == 0);
  }
  return is_numeric;
}

bool IsInteger(std::string_view str) {
  if (str.empty()) {
    return false;
  }

  const bool has_sign = (str[0] == '-' || str[0] == '+');
  const auto begin = has_sign ? str.begin() + 1 : str.begin();
  const auto pos = std::find_if(
      begin, str.end(), [](char c) -> bool { return std::isdigit(c) == 0; });
  if (pos != str.end()) {
    return false;
  }

  return has_sign ? (str.length() > 1) : true;
}

std::vector<std::string> Split(std::string_view s, char delim) {
  std::vector<std::string> elems;
  std::istringstream ss(std::string(s), std::ios_base::in);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}

std::vector<std::string_view> Tokenize(std::string_view s,
    std::string_view delim) {
  std::vector<std::string_view> tokens;
  const auto* first = s.begin();

  while (first != s.end()) {
    const auto* second = std::find_first_of(
        first, std::cend(s), std::cbegin(delim), std::cend(delim));
    if (first != second) {
      const auto start =
          static_cast<std::size_t>(std::distance(s.begin(), first));
      const auto length =
          static_cast<std::size_t>(std::distance(first, second));
      tokens.emplace_back(s.substr(start, length));
    }

    if (second == s.end()) {
      break;
    }

    first = std::next(second);
  }

  return tokens;
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
    start_pos = result.find(needle, start_pos + replacement.length());
  }

  return result;
}

std::string Replace(std::string_view haystack, char needle, char replacement) {
  std::string cp(haystack);
  std::replace(cp.begin(), cp.end(), needle, replacement);
  return cp;
}

// TODO continue string_view replacements below
bool GetUrlProtocol(const std::string& url,
    std::string& protocol,
    std::string& remainder) {
  const std::size_t protocol_pos = url.find("://");
  if (protocol_pos == std::string::npos) {
    protocol = "";
    remainder = url;
    return false;
  } else {  // NOLINT
    protocol = url.substr(0, protocol_pos + 3);
    remainder = url.substr(protocol_pos + 3);
    return true;
  }
}

// TODO(feature-request) SplitUrl
//   extract protocol/schema, userinfo, host, port,
//   path, query, and fragment of a URL/URI

std::string ObscureUrlAuthentication(const std::string& url) {
  std::string protocol;
  std::string clipped;
  const bool has_protocol = GetUrlProtocol(url, protocol, clipped);

  const std::size_t at_pos = clipped.find('@');
  if (at_pos == std::string::npos) {
    return url;
  }

  std::string obscured = "<auth>" + clipped.substr(at_pos);
  if (has_protocol) {
    return protocol + obscured;
  }
  return obscured;
}

std::string ClipUrl(const std::string& url) {
  std::string protocol;
  std::string clipped;
  const bool has_protocol = GetUrlProtocol(url, protocol, clipped);

  // Special handling of file:// URLs, as there's no
  // authentication information. But potentially lots
  // of forward slashes:
  if (has_protocol && (Lower(protocol).compare("file://") == 0)) {  // NOLINT
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
  } else {  // NOLINT
    return ObscureUrlAuthentication(clipped);
  }
}

std::string Remove(std::string_view s, char c) {
  std::string removed;
  std::remove_copy(s.begin(), s.end(), std::back_inserter(removed), c);
  return removed;
}

std::string Remove(std::string_view s, std::initializer_list<char> chars) {
  std::string copy(s);
  for (const auto c : chars) {
    copy = Remove(copy, c);
  }
  return copy;
}

std::string ReplaceSpecialCharacters(std::string_view s) {
  std::string replaced(s);
  for (const auto& replacement : detail::Replacements()) {
    replaced = Replace(replaced, replacement.first, replacement.second);
  }
  return replaced;
}

std::string Slug(std::string_view s, bool strip_dashes) {
  // TODO a) inefficient
  // TODO b) update documentation
  std::string replaced = Lower(ReplaceSpecialCharacters(Trim(s)));

  std::ostringstream out;
  // Start with flag set to true, to return "-" if the string
  // contains exclusively non-alphanumeric characters
  bool prev_alphanum = true;
  for (const char c : replaced) {
    if (std::isalnum(c) != 0) {
      out << c;
      prev_alphanum = true;
    } else {
      if (prev_alphanum && (!strip_dashes)) {
        out << '-';
        prev_alphanum = false;
      }
    }
  }

  std::string slug = out.str();
  if (EndsWith(slug, '-')) {
    // Replace the stream with a default constructed:
    std::ostringstream().swap(out);

    const std::size_t last_alnum = slug.find_last_not_of('-');
    for (std::size_t idx = 0; idx <= last_alnum; ++idx) {
      out << slug[idx];
    }
  }
  return out.str();
}

std::string Shorten(std::string_view s,
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
        << ") is shorter than the ellipsis (\"" << ellipsis << "\", length "
        << ellipsis.length() << ")!";
    throw std::invalid_argument(msg.str());
  }

  std::ostringstream shortened;
  const std::size_t remaining_text_len = desired_length - ellipsis.length();
  if (ellipsis_position == 0) {
    // center
    const std::size_t num_left = remaining_text_len / 2;
    const std::size_t num_right = remaining_text_len - num_left;

    shortened << s.substr(0, num_left) << ellipsis
              << s.substr(s.length() - num_right);
  } else if (ellipsis_position < 0) {
    // left
    shortened << ellipsis << s.substr(s.length() - remaining_text_len);
  } else {
    // right
    shortened << s.substr(0, remaining_text_len) << ellipsis;
  }
  return shortened.str();
}

std::string Indent(std::string_view s, std::size_t n, char character) {
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

// NOLINTNEXTLINE(misc-no-recursion)
std::size_t LevenshteinDistance(std::string_view str1, std::string_view str2) {
  const std::size_t min_size = str1.size();
  const std::size_t max_size = str2.size();

  // Ensure first string is longer (or both are equally long).
  if (min_size > max_size) {
    return LevenshteinDistance(str2, str1);
  }

  std::vector<std::size_t> dist(min_size + 1);
  for (std::size_t i = 0; i <= min_size; ++i) {
    dist[i] = i;
  }

  for (std::size_t j = 1; j <= max_size; ++j) {
    std::size_t prev_diagonal = dist[0];
    std::size_t prev_diagonal_save{};
    ++dist[0];

    for (std::size_t i = 1; i <= min_size; ++i) {
      prev_diagonal_save = dist[i];
      if (str1[i - 1] == str2[j - 1]) {
        dist[i] = prev_diagonal;
      } else {
        dist[i] = std::min(std::min(dist[i - 1], dist[i]), prev_diagonal) + 1;
      }
      prev_diagonal = prev_diagonal_save;
    }
  }

  return dist[min_size];
}

}  // namespace werkzeugkiste::strings
