#include <string>

#include "werkzeugkiste/werkzeugkiste.hpp"

auto main() -> int
{
  auto const exported = exported_class {};

  return std::string("werkzeugkiste") == exported.name() ? 0 : 1;
}
