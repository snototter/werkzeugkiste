#ifndef __WERKZEUGKISTE_GEOMETRY_PRIMITIVES_H__
#define __WERKZEUGKISTE_GEOMETRY_PRIMITIVES_H__

#include <cmath>
#include <ostream>

#include <werkzeugkiste/geometry/vector.h>
#include <werkzeugkiste/geometry/utils.h>

namespace werkzeugkiste {
namespace geometry {

//TODO line2d, line3d, plane

/// Represents a line/segment in 2d Euclidean space.
template <typename _Tp>
class Line2d_ {
public:
  using vec_type = Vec<_Tp, 2>;

  /// Default constructor yields an invalid line/segment
  Line2d_() : empty_(true), pt_from_(), pt_to_() {}


  /// Construct a line from 2 real valued points. In case of a segment, these
  /// denote the start and end points.
  Line2d_(const vec_type &from, const vec_type &to)
    : empty_(false), pt_from_(from), pt_to_(to) {}


  /// Returns a line with flipped direction vector.
  Line2d_ Flipped() const { return Line2d_<_Tp>(pt_to_, pt_from_); }


  /// For a segment, this returns the start point. For a line, it's simply one
  /// of the given points to construct the line in the first place.
  const vec_type &From() const { return pt_from_; }


  /// For a segment, this returns the end point. For a line, it's simply one
  /// of the given points to construct the line in the first place.
  const vec_type &To() const { return pt_to_; }


  /// For a segment, this returns the length from the start to the end point.
  double SegmentLength() const { return Direction().Length(); }


  /// Returns the non-normalized direction vector from the start point to the
  /// end point.
  vec_type Direction() const { return pt_from_.DirectionVector(pt_to_); }


  /// Returns the unit direction vector from the start point to the end point.
  vec_type UnitDirection() const { return Direction().UnitVector(); }


  /// Returns the point halfway between from and to.
  vec_type MidPoint() const { return 0.5 * (pt_from_ + pt_to_); }


  /// Returns true, if the line object is empty, *i.e.* either start/end have
  /// not been set, or are the same point (to == from).
  bool empty() const { return empty_ || eps_equal(pt_from_, pt_to_); }


  /// Returns the angle between the line and v.
  double Angle(const vec_type &v) const {
    return std::acos(std::max(-1.0, std::min(1.0,
        static_cast<double>(UnitDirection().Dot(v.UnitVector())))));
  }


  /// Returns the line in homogeneous coordinates, *i.e.* a 3-element vector
  /// in P^2 (Projective 2-space). For more details on lines in projective space, refer to
  /// `Bob Fisher's CVonline <http://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/BEARDSLEY/node2.html>`__, or
  /// `Stan Birchfield's notes <http://robotics.stanford.edu/~birch/projective/node4.html>`__.
  Vec<_Tp, 3> HomogeneousForm() const {
    return Vec<_Tp, 3>{pt_from_[0], pt_from_[1], static_cast<_Tp>(1)}.Cross(
          Vec<_Tp, 3>{pt_to_[0], pt_to_[1], static_cast<_Tp>(1)});
  }


  /// Computes the projection of the given point onto this line.
  vec_type ClosestPointOnLine(const vec_type &point) const {
    // Vector from line start to point:
    const vec_type v = pt_from_.DirectionVector(point);
    // Project onto line and get closest point on line:
    const vec_type unit_dir = UnitDirection();
    const _Tp lambda = unit_dir.Dot(v);
    return pt_from_ + lambda * unit_dir;
  }


  /// Returns the point on this line segment(!) which is closest.
  vec_type ClosestPointOnSegment(const vec_type &point) const {
    // Vector from segment start to point:
    const vec_type v = pt_from_.DirectionVector(point);
    // Project v onto segment:
    const vec_type direction = Direction();
    const vec_type unit_direction = direction.UnitVector();
    const double lambda = static_cast<double>(unit_direction.Dot(v));

    if (lambda < 0.0) {
      return pt_from_;
    } else {
      if (lambda > SegmentLength()) {
        return pt_to_;
      } else {
        return pt_from_ + lambda * unit_direction;
      }
    }
  }


  /// Returns the minimum distance between the point and this line.
  double DistancePointLine(const vec_type &point) const {
    const vec_type closest_point = ProjectPointOntoLine(point);
    return point.Distance(closest_point);
  }


//  //TODO
//  /// Returns the minimum distance between the point and this segment.
//  double DistancePointSegment(const vec_type &point) {
//    // Compute lambda s.t. l=0: start point, l=1: end point of the segment.
//    const vec_type direction = Direction();
//    const double lambda = (point - pt_from_).Dot(direction) / direction.LengthSquared();

//    const vec_typecv::Vec2d closest_point = line_segment.From() + std::min(1.0, std::max(0.0, lambda)) * line_direction;
//    return Distance(point, closest_point);
//  }

  /// Returns true if the two lines are collinear.
  bool IsCollinear(const Line2d_ &other) const {
    // See IntersectionLineSegmentLineSegment() for more documentation and further links!
    // Line 1 goes from p to p + r
    const vec_type p = pt_from_;
    const vec_type r = Direction();
    // Line 2 goes from q to q + s
    const vec_type q = other.pt_from_;
    const vec_type s = other.Direction();

    const double rxs = Determinant(r, s);
    const double qmpxr = Determinant((q-p), r);

    return (eps_zero(rxs) && eps_zero(qmpxr));
  }


  bool IsPointLeftOfLine(const vec_type &point, bool *is_on_line = nullptr) const {
    const double det = Determinant(Direction(), point - pt_to_);

    // If the 2d cross product (determinant) is 0, the points are collinear,
    // and thus, would be on the line.
    if (eps_zero(det)) {
      if (is_on_line) {
        *is_on_line = true;
      }
      return true;
    } else {
      if (is_on_line) {
        *is_on_line = false;
      }
      return det > 0.0;
    }
  }


//TODO Intersection Line/line, line/segment, segment/line, segment/segment
  // intersection circle
  // clip line, clip segment

  /// Overloaded output stream operator.
  friend std::ostream &operator<< (std::ostream &stream, const Line2d_ &line) {
    stream << line.pt_from_ << "-->" << line.pt_to_;
    return stream;
  }

private:
  bool empty_;
  vec_type pt_from_;
  vec_type pt_to_;
};


typedef Line2d_<double> Line2d;



} // namespace geometry
} // namespace werkzeugkiste

#endif // __WERKZEUGKISTE_GEOMETRY_UTILS_H__
