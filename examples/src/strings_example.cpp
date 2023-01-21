#include <iostream>
#include <string>
#include <vector>
#include <ostream>

// Only needed to query the library version:
#include <werkzeugkiste/version.h>

#include <werkzeugkiste/strings/strings.h>

inline std::string b2s(bool val) {
  if (val) {
    return "true";
  } else {
    return "false";
  }
}


int main(int /* argc */, char ** /* argv */) {
  namespace wks = werkzeugkiste::strings;
  std::cout << "--------------------------------------------------\n"
            << "    Werkzeugkiste v" << werkzeugkiste::Version() << "\n"
            << "    String utilities demo\n"
            << "--------------------------------------------------\n"
            << std::endl;

  //TODO
  std::cout << "Shorten 5: " << wks::Shorten("0123456789", 5)
            << "\nShorten 7: " << wks::Shorten("0123456789", 7)
            << "\nShorten 10: " << wks::Shorten("0123456789", 10)
            << "\nShorten ellipsis center 5: " << wks::Shorten("0123456789", 5, 0)
            << "\nShorten ellipsis center 6: " << wks::Shorten("0123456789", 6, 0)
            << "\nShorten ellipsis center 7: " << wks::Shorten("0123456789", 7, 0)
            << "\nShorten ellipsis right:  " << wks::Shorten("0123456789", 5, 1)
            << "\nShorten 3 custom ellipsis: " << wks::Shorten("0123456789", 3, 0, "*!*")
            << "\nShorten 4 custom ellipsis: " << wks::Shorten("0123456789", 4, 0, "*!*")
            << "\nShorten 5 custom ellipsis: " << wks::Shorten("0123456789", 5, 0, "*!*")
            << std::endl << std::endl;

  std::vector<std::string> examples {"This", "is", "a", "LiSt", "oF", "\"ExAmPlEs\"", "-_1,2;3:'#!"};
  for (const auto &s : examples) {
    std::cout << "Input: " << s << "\n"
              << "+ Upper: " << wks::Upper(s)
              << "\n+ Lower: " << wks::Lower(s)
              << "\n+ Prefix 'a':  " << b2s(wks::StartsWith(s, 'a'))
              << "\n+ Suffix \"is\": " << b2s(wks::EndsWith(s, "is"))
              << std::endl << std::endl;
  }

  //TODO trim
  //TODO replace
  //TODO tokenize
  //TODO slug
  std::cout << "Concatenation: " << wks::Concatenate(examples)
            << "\nSlug:         " << wks::Slug(wks::Concatenate(examples))
            << std::endl;
  return 0;
}
