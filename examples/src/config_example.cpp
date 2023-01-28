#include <iostream>
#include <ostream>
#include <string>
#include <vector>

// Only needed to query the library version:
#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/version.h>

int main(int /* argc */, char** /* argv */) {
  namespace wkc = werkzeugkiste::config;
  std::cout << "--------------------------------------------------\n"
            << "    Werkzeugkiste v" << werkzeugkiste::Version() << "\n"
            << "    Configuration utilities demo\n"
            << "--------------------------------------------------\n"
            << std::endl;

  auto config = wkc::Configuration::LoadTOML("test.toml");

  config->RegisterPathParameter("foo");
  config->RegisterPathParameter("bar");
  config->RegisterPathParameter("frobnicate.frobmorten");
  config->MakePathsAbsolute("todo");

  config = wkc::Configuration::LoadTOML("no-such-file.toml");

  return 0;
}
