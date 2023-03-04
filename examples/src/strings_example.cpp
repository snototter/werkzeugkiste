#include <iostream>
#include <ostream>
#include <string>
#include <vector>

// Only needed to query the library version:
#include <werkzeugkiste/strings/strings.h>
#include <werkzeugkiste/version.h>

// NOLINTBEGIN(*-magic-numbers)

int main(int /* argc */, char** /* argv */) {
  namespace wks = werkzeugkiste::strings;
  std::cout << "--------------------------------------------------\n"
            << "    Werkzeugkiste v" << werkzeugkiste::Version() << "\n"
            << "    String utilities demo\n"
            << "--------------------------------------------------\n"
            << std::endl;

  // TODO
  std::cout << "Shorten 5: " << wks::Shorten("0123456789", 5)
            << "\nShorten 7: " << wks::Shorten("0123456789", 7)
            << "\nShorten 10: " << wks::Shorten("0123456789", 10)
            << "\nShorten ellipsis center 5: "
            << wks::Shorten("0123456789", 5, 0)
            << "\nShorten ellipsis center 6: "
            << wks::Shorten("0123456789", 6, 0)
            << "\nShorten ellipsis center 7: "
            << wks::Shorten("0123456789", 7, 0)
            << "\nShorten ellipsis right:  " << wks::Shorten("0123456789", 5, 1)
            << "\nShorten 3 custom ellipsis: "
            << wks::Shorten("0123456789", 3, 0, "*!*")
            << "\nShorten 4 custom ellipsis: "
            << wks::Shorten("0123456789", 4, 0, "*!*")
            << "\nShorten 5 custom ellipsis: "
            << wks::Shorten("0123456789", 5, 0, "*!*") << std::endl
            << std::endl;

  std::vector<std::string> examples{"This",
      "is",
      "a",
      "LiSt",
      "oF",
      "\"ExAmPlE\"",
      " Strings ",
      "with_some_",
      "special characters",
      ", e.g.",
      "-_1,2;3:'#!",
      "öÖ#Üß\\-_\t\\"};
  for (const auto& s : examples) {
    std::cout << "Input: " << s << "\n"
              << "+ Upper: " << wks::Upper(s) << "\n+ Lower: " << wks::Lower(s)
              << std::boolalpha
              << "\n+ Prefix 'a':  " << wks::StartsWith(s, 'a')
              << "\n+ Suffix \"is\": " << wks::EndsWith(s, "is") << std::endl
              << std::endl;
  }

  // TODO trim
  // TODO replace
  // TODO tokenize

  // Concatenation & slugification
  std::string concat = wks::Concatenate(examples, " ");
  std::string slug = wks::Slug(concat);
  constexpr std::size_t desired_length = 42;
  using namespace std::literals;
  std::string_view ellipsis{"\u2026"sv};
  std::cout << "Concatenation: " << concat << "\nSlug:          " << slug
            << "\nShortened:     " << wks::Shorten(slug, desired_length)
            << "\n               " << wks::Shorten(slug, desired_length, 0)
            << "\n               " << wks::Shorten(slug, desired_length, 1)
            << "\nEllipsis char: "
            << wks::Shorten(slug, desired_length, 1, ellipsis)
            << " (Length ellipsis unicode char: " << ellipsis.length() << ")"
            << std::endl;
  return 0;
}

// NOLINTEND(*-magic-numbers)
