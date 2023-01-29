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

  auto config = wkc::Configuration::LoadTomlString(R"toml(
    an_int = 3
    a_str = 'foo'
    a_float = 1.234
    another_float = 1.05e-17
    
    [relative_paths]
    path = "p1"
    another_path = "p2"

    folder = "p3"

    [some.folders.folders]
    folder = "nested-path"
    
    [absolute_paths]
    )toml");

  config->EnsureAbsolutePaths("TODO", {"path", "*.folder"});

  config->ParameterNames();  // TODO

  try {
    config = wkc::Configuration::LoadTomlFile("no-such-file.toml");
  } catch (const std::runtime_error&) {
    // TODO
  }

  return 0;
}
