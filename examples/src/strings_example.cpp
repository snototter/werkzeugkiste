#include <iostream>
#include <string>
#include <vector>

// Only needed to query the library version:
#include <werkzeugkiste/version.h>

#include <werkzeugkiste/strings/strings.h>

int main(int /* argc */, char ** /* argv */) {
  namespace wks = werkzeugkiste::strings;
  std::cout << "--------------------------------------------------\n"
            << "    Werkzeugkiste v" << WERKZEUGKISTE_VERSION << "\n"
            << "    String utilites demo\n"
            << "--------------------------------------------------\n"
            << std::endl;

  std::vector<std::string> examples {"This", "is", "a", "LiSt", "oF", "\"ExAmPlEs\"", "-_1,2;3:'#!"};
  for (const auto &s : examples) {
    std::cout << "Input: " << s << "\n"
              << "+ Upper: " << wks::Upper(s)
              << "\n+ Lower: " << wks::Lower(s)
              << "\n+ Prefix 'a':  " << wks::StartsWith(s, 'a')
              << "\n+ Suffix \"is\": " << wks::EndsWith(s, "is")
              << std::endl << std::endl;
  }

 return 0;
}
