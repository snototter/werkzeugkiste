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
            << "    Werkzeugkiste v" << WERKZEUGKISTE_VERSION << "\n"
            << "    String utilities demo\n"
            << "--------------------------------------------------\n"
            << std::endl;

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
  //TODO

 return 0;
}
