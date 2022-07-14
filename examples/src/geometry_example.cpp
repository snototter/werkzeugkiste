#include <iostream>
#include <string>
#include <vector>
#include <ostream>
#include <utility>

// Only needed to query the library version:
#include <werkzeugkiste/version.h>

#include <werkzeugkiste/geometry/vector.h>
#include <werkzeugkiste/geometry/projection.h>


int main(int /* argc */, char ** /* argv */) {
  namespace wkg = werkzeugkiste::geometry;
  std::cout << "--------------------------------------------------\n"
            << "    Werkzeugkiste v" << WERKZEUGKISTE_VERSION << "\n"
            << "    Geometry utilities demo\n"
            << "--------------------------------------------------\n"
            << std::endl;

  //TODO
  auto v1 = wkg::Vec2d{17, 42};

  std::cout << "Vector:      " << v1
          << "\n--> Length:  " << v1.Length()
          << "\nUnit vector: " << v1.UnitVector()
          << "\n--> Length:  " << v1.UnitVector().Length() << std::endl;

  auto v2 = wkg::Vec2i{3, 9};
  std::cout << "Conversion: " << wkg::VecToEigen(v1)
            << "\nHomogeneous: " << wkg::VecToEigenHomogeneous(v1)
            << "\nMat(3 vecs): " << wkg::VecsToEigen(v1, v1, v1)
            << std::endl;

  std::cout << "Conversion: " << wkg::VecToEigen(v2)
            << "\nHomogeneous: " << wkg::VecToEigenHomogeneous(v2)
            << "\nMat(3 vecs): " << wkg::VecsToEigen(v1, static_cast<wkg::Vec2d>(v2), v1)
            << std::endl;

  wkg::Vec2d a, b;
  auto trafo = wkg::VecsToEigen(v1, static_cast<wkg::Vec2d>(v2), v1);
  std::tie(a, b) = wkg::TransformVecs(trafo, v1, v1);
  //std::tie(a,b);

 return 0;
}
