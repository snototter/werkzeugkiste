#include <algorithm>
#include <iterator>
#include <fstream>
#include <sstream>
#include <cctype>
#include <string>

#include <werkzeugkiste/strings/strings.h>
#include <werkzeugkiste_macros.h>

namespace werkzeugkiste {
namespace strings {


bool EndsWith(
    const std::string &s,
    const std::string &suffix) {
  if ((s.length() > 0)
      && (suffix.length() > 0)
      && (s.length() >= suffix.length())) {
    return (s.compare(
              s.length() - suffix.length(),
              suffix.length(),
              suffix) == 0);
  } else {
    return false;
  }
}


bool StartsWith(
    const std::string &s,
    const std::string &prefix) {
  if ((s.length() > 0)
      && (prefix.length() > 0)
      && (s.length() >= prefix.length())) {
    return s.compare(0, prefix.length(), prefix) == 0;
  } else {
    return false;
  }
}


void ToLower(std::string &s) {
  std::transform(
        s.begin(), s.end(), s.begin(), ::tolower);
}


void ToUpper(std::string &s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 ::toupper);
}


std::string LTrim(const std::string &totrim) {
  std::string s(totrim);
  s.erase(
        s.begin(),
        std::find_if(
          s.begin(), s.end(),
          std::not1(std::ptr_fun<int, int>(std::isspace))));
  return s;
}


std::string RTrim(const std::string &totrim) {
  std::string s(totrim);
  s.erase(
        std::find_if(
          s.rbegin(), s.rend(),
          std::not1(std::ptr_fun<int, int>(std::isspace))
        ).base(),
        s.end());
  return s;
}


std::string Trim(const std::string &s) {
  return LTrim(RTrim(s));
}


bool IsNumeric(const std::string &s) {
  if (s.length() == 0) {
    return false;
  }

  char *pd, *pl;
  // Check long
  long dummyl = strtol(s.c_str(), &pl, 10);
  WERKZEUGKISTE_UNUSED_VAR(dummyl);
  // Check double
  double dummyd = strtod(s.c_str(), &pd);
  WERKZEUGKISTE_UNUSED_VAR(dummyd);
  return (*pd == 0) || (*pl == 0);
}


std::vector<std::string> Split(
    const std::string &s,
    char delim) {
  std::vector<std::string> elems;
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}


std::string Replace(
    const std::string &haystack,
    const std::string &needle,
    const std::string &replacement) {
  if ((haystack.length() == 0) || (needle.length() == 0)) {
    return haystack;
  }

  std::size_t start_pos = haystack.find(needle);
  if(start_pos == std::string::npos) {
    return haystack;
  }

  std::string s = haystack;
  do {
    s.replace(start_pos, needle.length(), replacement);
    start_pos = s.find(needle);
  } while (start_pos != std::string::npos);

  return s;
}


std::string Replace(
    const std::string &haystack, char needle, char replacement) {
  std::string cp(haystack);
  std::replace(
        cp.begin(), cp.end(),
        needle, replacement);
  return cp;
}


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
  } else {
    return ObscureUrlAuthentication(clipped);
  }
}


std::string Remove(const std::string &s, const char c) {
  std::string removed;
  std::remove_copy(
        s.begin(), s.end(),
        std::back_inserter(removed), c);
  return removed;
}


std::string Remove(
    const std::string &s,
    std::initializer_list<char> chars) {
  std::string copy(s);
  for (const auto c : chars) {
    copy = Remove(copy, c);
  }
  return copy;
}


std::string Slug(
    const std::string &s,
    bool strip_dashes) {
  std::string replaced = Replace(
        Lower(Trim(s)), "#", "nr");

  std::ostringstream out;
  // Start with flag set to return "-" if the string
  // contains exclusively non-alphanumeric characters
  bool prev_alphanum = true;
  for (std::size_t i = 0; i < replaced.length(); ++i) {
    if (std::isalnum(replaced[i])) {
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
    const std::string &s,
    std::size_t desired_length,
    int ellipsis_position,
    const std::string &ellipsis) {
  if (s.empty() || (desired_length >= s.length())) {
    return s;
  }

  if (desired_length == 0) {
    return "";
  }

  if (desired_length < ellipsis.length()) {
    std::ostringstream msg;
    msg << "Desired length ("
        << desired_length
        << ") is shorter than the ellipsis ("
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
    shortened << ellipsis
              << s.substr(s.length() - remaining_text_len);
  } else {
    shortened << s.substr(0, remaining_text_len)
              << ellipsis;
  }
  return shortened.str();
}

} // namespace string
} // namespace werkzeug
