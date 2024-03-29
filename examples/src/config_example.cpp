#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/files/filesys.h>
#include <werkzeugkiste/version.h>

#include <iostream>
#include <ostream>
#include <string>
#include <vector>

// TODO
#include <werkzeugkiste/config/casts.h>
#include <werkzeugkiste/logging.h>

template <typename T, typename S>
void CastingCheck(S val) {
  namespace wkc = werkzeugkiste::config;
  std::cout << "Casting check:"
            << "\n* from " << wkc::TypeName<S>() << "(" << std::to_string(val)
            << ") to " << wkc::TypeName<T>() << "\n* sizeof from(" << sizeof(S)
            << ") vs sizeof to(" << sizeof(T)
            << ")\n* is promotable: " << wkc::is_promotable<S, T>()
            << "\n* cast " << std::to_string(val) << " = ";
  try {
    if constexpr (std::is_same_v<int8_t, T>) {
      std::cout << std::to_string(wkc::checked_numcast<T>(val)) << std::endl;
    } else {
      std::cout << wkc::checked_numcast<T>(val) << std::endl << std::endl;
    }
  } catch (const std::domain_error &e) {
    WZKLOG_CRITICAL("Caught exception during checked_numcast:\n{}\n", e.what());
  }
}

// NOLINTBEGIN(*magic-numbers)
int main(int /* argc */, char ** /* argv */) {
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

  std::cout << "Query a double: " << config.GetDouble("a_float") << std::endl;
  try {
    config.GetDouble("a_str");
  } catch (const wkc::TypeError &e) {
    std::cout << "Can't convert a string to double: " << e.what() << std::endl;
  }

  // config.GetDouble("an_int"); //TODO this also throws
  try {
    config.GetDouble("no.such.key");
  } catch (const wkc::KeyError &e) {
    std::cout << "Can't look up a non-existing key: " << e.what() << std::endl;
  }
  std::cout << "But it can be replaced with a default value: "
            << config.GetDoubleOr("no.such.key", 42) << std::endl;

  try {
    config = wkc::LoadTOMLFile("no-such-file.toml");
  } catch (const wkc::ParseError &e) {
    std::cout << e.what() << std::endl;
  }

  config =
      wkc::LoadTOMLFile(wkf::FullFile(wkf::DirName(__FILE__), "tomlspec.toml"));
  const auto params = config.ListParameterNames(
      /*include_array_entries=*/false, /*recursive=*/true);
  std::cout << "Parameter names:\n";
  for (const auto &name : params) {
    std::cout << "  " << name << std::endl;
  }

  // config.AdjustRelativePaths("CUSTOM/BASE/PATH",
  //                             {"*name"sv, "*.regex*"sv, "strings.str5"sv,
  //                              "strings.str[1-3|7]?"sv,
  //                              "products[2].color"sv});

  // config.GetDouble("an_int"); //TODO this also throws
  try {
    config.GetDouble("date-time-params.local-date.ld1");
  } catch (const wkc::TypeError &e) {
    std::cout << "Tried wrong type: " << e.what() << std::endl;
  }
  try {
    config.GetDouble("date-time-params.local-date");
  } catch (const wkc::TypeError &e) {
    std::cout << "Tried wrong type: " << e.what() << std::endl;
  }

  try {
    std::cout << "Query int32_max: "
              << config.GetInt32("integral-numbers.int32_max") << std::endl;
    std::cout << "Query int64 as int32 (should throw exception): "
              << config.GetInt32("integral-numbers.int64") << std::endl;
  } catch (const wkc::TypeError &e) {
    std::cout << "Caught exception: " << e.what() << std::endl;
    std::cout << "Query int64 correctly: "
              << config.GetInt64("integral-numbers.int64") << std::endl;
  }

  const auto list_config = wkc::Configuration::LoadTOMLString(R"toml(
    ints32 = [1, 2, 3, 4, 5, 6, -7, -8]

    ints64 = [0, 2147483647, 2147483648, -2147483648, -2147483649]

    floats = [0.5, 1.0, 1.0e23]

    strings = ["abc", "Foo", "Frobmorten", "Test String"]

    # Type mix
    mixed_types = [1, 2, "framboozle"]

    [not-a-list]
    name = "test"
    )toml");

  try {
    list_config.GetInt32List("no-such-key");
    throw std::logic_error("Shouldn't be here");
  } catch (const wkc::KeyError &e) {
    std::cout << "Tried invalid key, got exception: " << e.what() << std::endl;
  }

  try {
    list_config.GetInt32List("not-a-list");
    throw std::logic_error("Shouldn't be here");
  } catch (const wkc::TypeError &e) {
    std::cout << "Tried loading a table as a list, got exception: " << e.what()
              << std::endl;
  }

  try {
    list_config.GetInt32List("mixed_types");
    throw std::logic_error("Shouldn't be here");
  } catch (const wkc::TypeError &e) {
    std::cout << "Tried loading an inhomogeneous array as scalar list, got "
                 "exception: "
              << e.what() << std::endl;
    ;
  }

  auto list_str = list_config.GetStringList("strings");
  std::cout << "Loaded string list: {";
  for (const auto &s : list_str) {
    std::cout << '"' << s << "\", ";
  }
  std::cout << "}" << std::endl;

  //---------------------------------------------------------------------------
  // Exemplary type casts
  CastingCheck<int>(/*val=*/true);
  CastingCheck<char>(/*val=*/false);
  CastingCheck<bool>(0);
  CastingCheck<bool>(1);
  CastingCheck<bool>(2);
  CastingCheck<int8_t>(127L);
  CastingCheck<int8_t>(128L);
  CastingCheck<uint8_t>(128L);
  CastingCheck<uint8_t>(255L);
  CastingCheck<uint8_t>(256L);

  CastingCheck<int>(static_cast<int16_t>(42));
  CastingCheck<int>(static_cast<uint16_t>(42));
  CastingCheck<uint>(static_cast<int8_t>(0));
  CastingCheck<uint>(static_cast<int8_t>(-42));

  CastingCheck<double>(0.2F);
  CastingCheck<double>(0.1F);
  CastingCheck<long double>(0.2F);

  CastingCheck<float>(1.0);
  CastingCheck<float>(0.0);
  CastingCheck<float>(0.5);
  CastingCheck<float>(-24.0);
  CastingCheck<float>(0.2);
  CastingCheck<float>(3.141592653589793238462643383279502884L);
  CastingCheck<float>(1.0005);

  //---------------------------------------------------------------------------
  // Type example
  std::cout << "Parsing time representations:\n"
            << wkc::time{"08:30"sv} << "\n"
            << wkc::time{"23:59:59"sv} << "\n"
            << wkc::time{"23:59:59.123"sv} << "\n"
            << wkc::time{"23:59:59.123456"sv} << "\n"
            << wkc::time{"23:59:59.123456789"sv} << std::endl;

  return 0;
}
// NOLINTEND(*magic-numbers)
