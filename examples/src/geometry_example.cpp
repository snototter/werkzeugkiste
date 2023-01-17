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
#include <werkzeugkiste/geometry/camera.h>
#include <werkzeugkiste/geometry/primitives.h>


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


template <typename Vector>
std::string PrettyPrint(
    std::initializer_list<Vector> vecs,
    std::size_t indent_first_row = 0,
    std::size_t indent_others = 0) {
  std::ostringstream s;
  s << "[ ";

  const auto lst_it = vecs.begin();
  for (int dim = 0; dim < Vector::ndim; ++dim) {
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
            << "    Werkzeugkiste v" << werkzeugkiste::version << "\n"
            << "    Geometry utilities demo\n"
            << "--------------------------------------------------\n"
            << std::endl;

  //TODO
  wkg::Vec2d v1{17, 42}, v2{9, -3}, v3{0, 0.01};

  std::cout << "Vector:      " << v1
          << "\n--> Length:  " << v1.Length()
          << "\nUnit vector: " << v1.UnitVector()
          << "\n--> Length:  " << v1.UnitVector().Length()
          << "\n--> Rot90 CW:  " << v1.PerpendicularClockwise()
          << "\n--> Rot90 CCW: " << v1.PerpendicularCounterClockwise()
          << std::endl;

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
  //TODO remove if we drop transformtomat finally
//  wkg::Matrix<double, 4, 3> mat_res = wkg::TransformToMat(M, v1, v2, v3);

  wkg::Vec4d a, b, c;
//  std::tie(a, b, c) = wkg::EigenMatToVecTuple(mat_res);//, std::make_index_sequence<3>{});
//  std::cout << "Transformation v1:\n" << M << " * " << PrettyPrint({v1, v2, v3}, 0, 6)
//            << " =\n" << PrettyPrint({a, b, c}) << std::endl;

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
  std::tie(p1, p2) = wkg::ProjectToVecs(P, v1, v2);

  std::cout << "Projection (homogeneous coord implicitly added):\n" << P << " * " << PrettyPrint({v1, v2}, 0, 8)
            << " =\n" << PrettyPrint({p1, p2}) << std::endl;

  std::tie(p3) = wkg::ProjectToVecs(P, v3);
  std::cout << "Projection (homogeneous coord implicitly added), single vec:\n" << P << " * " << PrettyPrint({v3}, 0, 8)
            << " =\n" << PrettyPrint({p3}) << std::endl;


  std::tie(p1, p2) = wkg::ProjectToVecs(P, v1.Homogeneous(), v2.Homogeneous());

  std::cout << "Projection (inputs already homogeneous):\n" << P << " * " << PrettyPrint({v1.Homogeneous(), v2.Homogeneous()}, 0, 8)
            << " =\n" << PrettyPrint({p1, p2}) << std::endl;

  std::tie(p3) = wkg::ProjectToVecs(P, v3.Homogeneous());
  std::cout << "Projection (inputs already homogeneous), single vec:\n" << P << " * " << PrettyPrint({v3.Homogeneous()}, 0, 8)
            << " =\n" << PrettyPrint({p3}) << std::endl;


  //TODO refactor demo
  wkg::Mat3x3d K, R;
  wkg::Vec3d t{0.5, 0.3, 0.1};

  K << 400, 0, 300,
       0, 400, 300,
      0, 0, 1;
  R << 1, 0, 0,
       0, 1, 0,
       0, 0, 1;

  wkg::Matrix<double, 3, 4> Rt;
  Rt << R, wkg::VecToEigenMat<3>(t);

  wkg::Mat3x4d cam_prj = wkg::ProjectionMatrixFromKRt(K, R, t);

  std::cout << "Projection matrix:\nK = " << K << ", R = " << R << ", t = " << t << " --> P = \n" << cam_prj << std::endl;

  std::cout << "Line of horizon: " << wkg::GetProjectionOfHorizon(K, R, t) << std::endl;

  std::cout << "GP-2-image:\n" << wkg::GroundplaneToImageHomography(cam_prj)
            << "\nImage-2-GP:\n" << wkg::ImageToGroundplaneHomography(cam_prj)
            << "\n... must equal GP-2-img^(-1):\n" << wkg::GroundplaneToImageHomography(cam_prj).inverse() << std::endl;

  std::cout << "Camera center (R, t): " << wkg::CameraCenterFromRt(R, t) << std::endl
            << "Camera center (Rt):   " << wkg::CameraCenterFromRt(Rt) << std::endl;

  std::cout << "Rotation matrix (float):\n" << wkg::RotationMatrix<float>(10, 20, 30, true) << std::endl
            << "Rotation matrix (double):\n" << wkg::RotationMatrix<double>(10, 20, 30, true) << std::endl;




  wkg::Line2d line1({0.0, 0.0}, {3.0, 0.0});
  wkg::Line2d line2({1.0, -0.6}, {-17.0, -0.6});
  wkg::Line2d line3({-100.0, -0.6}, {-170.0, -0.6});

  std::cout << "Lines: " << line1 << " and " << line2 << std::endl
            << "collinear? " << line1.IsCollinear(line2) << std::endl
            << "collinear " << line2 << " and " << line3 << "? " << line2.IsCollinear(line3) << std::endl
            << "Closest point (" << line1.To() << ")\n  to line2 " << line2.ClosestPointOnLine(line1.To()) << std::endl
            << "  to segment2: " << line2.ClosestPointOnSegment(line1.To()) << std::endl;

  return 0;
}
