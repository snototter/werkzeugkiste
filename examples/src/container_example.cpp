#include <iostream>
#include <string>
#include <vector>
#include <ostream>

// Only needed to query the library version:
#include <werkzeugkiste/version.h>

#include <werkzeugkiste/container/sort.h>


int main(int /* argc */, char ** /* argv */) {
  namespace wkc = werkzeugkiste::container;
  std::cout << "--------------------------------------------------\n"
            << "    Werkzeugkiste v" << WERKZEUGKISTE_VERSION << "\n"
            << "    Container utilities demo\n"
            << "--------------------------------------------------\n"
            << std::endl;

//  //TODO
//  auto v1 = wkg::Vec2d{17, 42};
//  std::cout << "Vector:      " << v1
//          << "\n--> Length:  " << v1.Length()
//          << "\nUnit vector: " << v1.UnitVector()
//          << "\n--> Length:  " << v1.UnitVector().Length() << std::endl;

 return 0;
}
