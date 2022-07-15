#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <ostream>
#include <utility>
#include <initializer_list>

// Only needed to query the library version:
#include <werkzeugkiste/version.h>

#include <werkzeugkiste/geometry/vector.h>
#include <werkzeugkiste/geometry/projection.h>


//template <typename _V>
//std::string PrettyPrint(const _V &vec) {
//  std::ostringstream s;
//  for (int i = 0; i < _V::ndim; ++i) {
//    if (i > 0) {
//      s << "\n";
//    }
//    s << vec[i];
//  }
//  return s.str();
//}


template <typename _V>
std::string PrettyPrint(
    std::initializer_list<_V> vecs,
    std::size_t indent_first_row = 0,
    std::size_t indent_others = 0) {
  std::ostringstream s;
  s << "[ ";

  const auto lst_it = vecs.begin();
  for (int dim = 0; dim < _V::ndim; ++dim) {
    if (dim > 0) {
      s << "\n";
      for (std::size_t idx = 0; idx < indent_others + 2; ++idx) {
        s << ' ';
      }
    } else {
      for (std::size_t idx = 0; idx < indent_first_row; ++idx) {
        s << ' ';
      }
    }

    for (std::size_t idx = 0; idx < vecs.size(); ++idx) {
      s << std::fixed << std::setw(10) << std::setprecision(3) << lst_it[idx][dim];
      if (idx < vecs.size() - 1) {
        s << ", ";
      }
    }
  }
  s << " ]";
  return s.str();
}


int main(int /* argc */, char ** /* argv */) {
  namespace wkg = werkzeugkiste::geometry;
  std::cout << "--------------------------------------------------\n"
            << "    Werkzeugkiste v" << WERKZEUGKISTE_VERSION << "\n"
            << "    Geometry utilities demo\n"
            << "--------------------------------------------------\n"
            << std::endl;

  //TODO
  wkg::Vec2d v1{17, 42}, v2{9, -3}, v3{0, 0.01};

  std::cout << "Vector:      " << v1
          << "\n--> Length:  " << v1.Length()
          << "\nUnit vector: " << v1.UnitVector()
          << "\n--> Length:  " << v1.UnitVector().Length() << std::endl;

//  auto v2 = wkg::Vec2i{3, 9};
//  std::cout << "Conversion: " << wkg::VecToEigenCol(v1)
//            << "\nHomogeneous: " << wkg::VecToEigenHomogeneous(v1)
//            << "\nMat(3 vecs): " << wkg::VecsToEigenMat(v1, v1, v1)
//            << std::endl;

//  std::cout << "Conversion: " << wkg::VecToEigenCol(v2)
//            << "\nHomogeneous: " << wkg::VecToEigenHomogeneous(v2)
//            << "\nMat(3 vecs): " << wkg::VecsToEigenMat(v1, static_cast<wkg::Vec2d>(v2), v1)
//            << std::endl;

  wkg::Matrix<double, 4, 2> M;
  M << 1, 2, 3, 4,
       5, 6, 7, 8;
  wkg::Matrix<double, 4, 3> mat_res = wkg::TransformToMat(M, v1, v2, v3);

  wkg::Vec4d a, b, c;
  std::tie(a, b, c) = wkg::EigenMatToVecTuple(mat_res);//, std::make_index_sequence<3>{});
  std::cout << "Transformation v1:\n" << M << " * " << PrettyPrint({v1, v2, v3}, 0, 6)
            << " =\n" << PrettyPrint({a, b, c}) << std::endl;

  std::tie(a, b, c) = wkg::TransformToVecs(M, v1, v2, v3);
  std::cout << "Transformation v2:\n" << M << " * " << PrettyPrint({v1, v2, v3}, 0, 6)
            << " =\n" << PrettyPrint({a, b, c}) << std::endl;

  std::tie(c) = wkg::TransformToVecs(M, v2);
  std::cout << "Transformation single vec:\n" << M << " * " << PrettyPrint({v2}, 0, 6)
            << " =\n" << PrettyPrint({c}) << std::endl;

  wkg::Matrix<double, 3, 3> P;
  P << 1, 2, 3,
       4, 5, 6,
       7, 8, 9;
  wkg::Vec2d p1, p2, p3;
  std::tie(p1, p2) = wkg::ProjectInhomogeneousToVecs(P, v1, v2);

  std::cout << "Projection:\n" << P << " * " << PrettyPrint({v1, v2}, 0, 8)
            << " =\n" << PrettyPrint({p1, p2}) << std::endl;

  std::tie(p3) = wkg::ProjectInhomogeneousToVecs(P, v3);
  std::cout << "Projection single vec:\n" << P << " * " << PrettyPrint({v3}, 0, 8)
            << " =\n" << PrettyPrint({p3}) << std::endl;

 return 0;
}
