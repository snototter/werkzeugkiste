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


bool EndsWith(const std::string &s, const std::string &suffix) {
  if ((s.length() > 0) && (suffix.length() > 0)
      && (s.length() >= suffix.length())) {
    return (s.compare(s.length() - suffix.length(),
                      suffix.length(), suffix) == 0);
  } else {
    return false;
  }
}


bool EndsWith(const std::string &s, char end) {
  if (s.length() > 0) {
    return (s.at(s.length()-1) == end);
  } else {
    return false;
  }
}


bool StartsWith(const std::string &s, const std::string &prefix) {
  if ((s.length() > 0) && (prefix.length() > 0)
      && (s.length() >= prefix.length())) {
    return s.compare(0, prefix.length(), prefix) == 0;
  } else {
    return false;
  }
}


bool StartsWith(const std::string &s, char first) {
  if (s.length() > 0) {
    return s[0] == first;
  } else {
    return false;
  }
}


void ToLower(std::string &s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 ::tolower);
}


std::string Lower(const std::string &s) {
  std::string tmp(s);
  ToLower(tmp);
  return tmp;
}


void ToUpper(std::string &s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 ::toupper);
}


std::string Upper(const std::string &s) {
  std::string tmp(s);
  ToUpper(tmp);
  return tmp;
}


std::string LTrim(const std::string &totrim) {
  std::string s(totrim);
  s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                  std::not1(std::ptr_fun<int, int>(std::isspace))));
  return s;
}


std::string RTrim(const std::string &totrim) {
  std::string s(totrim);
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
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


void Split(const std::string &s, char delim,
           std::vector<std::string> &elems) {
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
}


std::vector<std::string> Split(const std::string &s, char delim) {
  std::vector<std::string> elems;
  Split(s, delim, elems);
  return elems;
}


std::string Replace(const std::string &str, const std::string &search,
                    const std::string &replacement) {
  if ((str.length() == 0) || (search.length() == 0)) {
    return str;
  }

  std::size_t start_pos = str.find(search);
  if(start_pos == std::string::npos) {
    return str;
  }

  std::string s = str;
  do {
    s.replace(start_pos, search.length(), replacement);
    start_pos = s.find(search);
  } while (start_pos != std::string::npos);

  return s;
}


bool GetUrlProtocol(const std::string &url,
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

//TODO(snototter) feature-request SplitUrl
// extract protocol/schema, userinfo, host, port,
// path, query, and fragment of a URL/URI


std::string ObscureUrlAuthentication(const std::string &url) {
  std::string protocol, clipped;
  const bool has_protocol = GetUrlProtocol(url, protocol, clipped);

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
  const bool has_protocol = GetUrlProtocol(url, protocol, clipped);

  // Special handling of file:// URLs, there's no
  // authentication information
  if (has_protocol && (Lower(protocol).compare("file://") == 0)) {
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
  std::remove_copy(s.begin(), s.end(),
                   std::back_inserter(removed), c);
  return removed;
}


std::string Remove(const std::string &s,
                   std::initializer_list<char> chars) {
  std::string copy(s);
  for (const auto c : chars) {
    copy = Remove(copy, c);
  }
  return copy;
}


std::string Slug(const std::string &s,
                 bool strip_dashes) {
  std::string replaced = Replace(Lower(Trim(s)),
                                 "#", "nr");

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
} // namespace string
} // namespace werkzeug
