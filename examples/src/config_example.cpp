#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/files/filesys.h>
#include <werkzeugkiste/version.h>

#include <iostream>
#include <ostream>
#include <string>
#include <vector>

// NOLINTBEGIN(*magic-numbers)
int main(int /* argc */, char** /* argv */) {
  namespace wkc = werkzeugkiste::config;
  namespace wkf = werkzeugkiste::files;
  using namespace std::string_view_literals;

  std::cout << "--------------------------------------------------\n"
            << "    Werkzeugkiste v" << werkzeugkiste::Version() << "\n"
            << "    Configuration utilities demo\n"
            << "--------------------------------------------------\n"
            << std::endl;

  auto config = wkc::Configuration::LoadTOMLString(R"toml(
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

  // config->EnsureAbsolutePaths("TODO", {"path", "*.folder"});
  std::cout << "Query a double: " << config->GetDouble("a_float") << std::endl;
  try {
    config->GetDouble("a_str");
  } catch (const std::runtime_error& e) {
    std::cout << "Can't convert a string to double: " << e.what() << std::endl;
  }

  // config->GetDouble("an_int"); //TODO this also throws
  try {
    config->GetDouble("no.such.key");
  } catch (const std::runtime_error& e) {
    std::cout << "Can't look up a non-existing key: " << e.what() << std::endl;
  }
  std::cout << "But it can be replaced with a default value: "
            << config->GetDoubleOrDefault("no.such.key", 42) << std::endl;

  try {
    config = wkc::Configuration::LoadTOMLFile("no-such-file.toml");
  } catch (const std::runtime_error&) {
    // TODO
  }

  config = wkc::Configuration::LoadTOMLFile(
      wkf::FullFile(wkf::DirName(__FILE__), "tomlspec.toml"));
  const auto params = config->ListParameterNames(false);
  std::cout << "Parameter names:\n";
  for (const auto& name : params) {
    std::cout << "  " << name << std::endl;
  }

  // config->AdjustRelativePaths("CUSTOM/BASE/PATH",
  //                             {"*name"sv, "*.regex*"sv, "strings.str5"sv,
  //                              "strings.str[1-3|7]?"sv,
  //                              "products[2].color"sv});

  // config->GetDouble("an_int"); //TODO this also throws
  try {
    // TODO header lookup returns contains() -> false! ??
    //  config->GetDouble("date-time-params.local-date");
    config->GetDouble("date-time-params.local-date.ld1");
  } catch (const std::runtime_error& e) {
    std::cout << "Tried wrong type: " << e.what() << std::endl;
  }
  try {
    config->GetDouble("date-time-params.local-date");
  } catch (const std::runtime_error& e) {
    std::cout << "Tried wrong type: " << e.what() << std::endl;
  }

  try {
    std::cout << "Query int32_max: "
              << config->GetInteger32("integral-numbers.int32_max")
              << std::endl;
    std::cout << "Query int64 as int32 (should throw exception): "
              << config->GetInteger32("integral-numbers.int64") << std::endl;
  } catch (const std::runtime_error& e) {
    std::cout << "Caught exception: " << e.what() << std::endl;
    std::cout << "Query int64 correctly: "
              << config->GetInteger64("integral-numbers.int64") << std::endl;
  }

  const auto list_config = wkc::Configuration::LoadTOMLString(R"toml(
    ints32 = [1, 2, 3, 4, 5, 6, -7, -8]

    ints64 = [0, 2147483647, 2147483648, -2147483648, -2147483649]

    floats = [0.5, 1.0, 1.0e23]

    strings = ["abc", "Foo", "Frobmorten", "Test String"]

    # Type mix
    invalid_int_flt = [1, 2, 3, 4.5, 5]

    mixed_types = [1, 2, "framboozle"]

    [not-a-list]
    name = "test"
    )toml");

  try {
    list_config->GetInteger32List("no-such-key");
    throw std::logic_error("Shouldn't be here");
  } catch (const std::runtime_error& e) {
    std::cout << "Tried invalid key, got exception: " << e.what() << std::endl;
    ;
  }

  try {
    list_config->GetInteger32List("not-a-list");
    throw std::logic_error("Shouldn't be here");
  } catch (const std::runtime_error& e) {
    std::cout << "Tried loading a table as a list, got exception: " << e.what()
              << std::endl;
    ;
  }

  try {
    list_config->GetInteger32List("mixed_types");
    throw std::logic_error("Shouldn't be here");
  } catch (const std::runtime_error& e) {
    std::cout << "Tried loading an inhomogeneous array as scalar list, got "
                 "exception: "
              << e.what() << std::endl;
    ;
  }

  auto list_str = list_config->GetStringList("strings");
  std::cout << "Loaded string list: {";
  for (const auto& s : list_str) {
    std::cout << '"' << s << "\", ";
  }
  std::cout << "}" << std::endl;

  return 0;
}
// NOLINTEND(*magic-numbers)
