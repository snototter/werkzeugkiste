#ifndef __WERKZEUGKISTE_GEOMETRY_PRIMITIVES_H__
#define __WERKZEUGKISTE_GEOMETRY_PRIMITIVES_H__

#include <cmath>
#include <limits>
#include <ostream>
#include <vector>
#include <type_traits>

#include <werkzeugkiste/geometry/vector.h>
#include <werkzeugkiste/geometry/utils.h>

namespace werkzeugkiste {
namespace geometry {

//TODO line2d, line3d, plane

// TODO Circles
// PointsOfTangencyPointToCircle
// TransverseCommonTangentsBetweenCircles
// DirectCommonTangentsBetweenCircles

// TODO Triangles

// TODO (Rotated) Rectangles IoU/Area
// TODO Polygon Inside/Outside/Distance, ConvexHull
// TODO RDP


/// Returns true if the given point is within the rectangle.
template <typename _Tp> inline
bool IsPointInsideRectangle(
    const Vec<_Tp, 2> &pt, const Vec<_Tp, 2> &top_left,
    const Vec<_Tp, 2> &size) {
  return (pt.x() >= top_left.x()) && (pt.x() < (top_left.x() + size.width()))
      && (pt.y() >= top_left.y()) && (pt.y() < (top_left.y() + size.height()));
}


//----------------------------------------------------
// Circle

// Forward declaration needed by Circle_.
template <typename _Tp> class Line2d_;


template <typename _Tp>
class Circle_ {
public:
  static_assert(
      std::is_floating_point<_Tp>::value,
      "Circle type must be float or double!");

  using vec_type = Vec<_Tp, 2>;


  /// Constructs an invalid circle.
  Circle_()
    : center_(0, 0), radius_(0.0f)
  {}


  /// Circle from center and radius.
  Circle_(const vec_type &center, _Tp radius)
    : center_(center), radius_(radius)
  {}


  /// Circle from 3 points. If they are collinear, the
  /// circle will be invalid. Check via `IsValid()` afterwards.
  Circle_(const vec_type &p, const vec_type &q, const vec_type &r);


  /// Returns true if this is a valid circle.
  inline bool IsValid() const { return radius_ > 0.0f; }


  /// Returns the center point as 2d vector.
  vec_type Center() const { return center_; }


  /// Returns the x-coordinate of the center point.
  _Tp cx() const { return center_.x(); }


  /// Returns the y-coordinate of the center point.
  _Tp cy() const { return center_.y(); }


  /// Returns the radius.
  _Tp Radius() const { return radius_; }


  /// Returns the area.
  _Tp Area() const { return M_PI * radius_ * radius_; }


  /// Returns true if the point is inside the circle. If you need to
  /// distinguish inside and exactly on the circle, pass a valid
  /// `is_on_circle` pointer.
  bool IsPointInCircle(const vec_type &pt, bool *is_on_circle = nullptr) const;


  /// Returns the number of points of tangency (i.e. 0, 1, or 2) between this
  /// circle and the given point. Optionally sets the points of tangency.
  int PointsOfTangency(
      const vec_type &pt,
      vec_type *pot1 = nullptr, vec_type *pot2 = nullptr) const;


  /// Returns the number of direct common tangents (German "aeussere
  /// Tangenten") between the two circles.
  /// Optionally sets the tangent lines.
  int DirectCommonTangents(
      const Circle_<_Tp> &other,
      Line2d_<_Tp> *tangent1 = nullptr, Line2d_<_Tp> *tangent2 = nullptr) const;


  /// Returns the number of transverse common tangents (German "innere
  /// Tangenten") between the two circles. These are 0 if the circles
  /// overlap, 1 if they intersect in exactly one point, and 2 if the
  /// circles do not touch.
  /// Optionally sets the tangent lines.
  int TransverseCommonTangents(
      const Circle_<_Tp> &other,
      Line2d_<_Tp> *tangent1 = nullptr, Line2d_<_Tp> *tangent2 = nullptr) const;


  /// Returns the number of intersections (0, 1, or 2) of the two circles.
  /// Returns -1 if the circles are equal and thus every point on the circles
  /// is an intersection point.
  /// Optionally sets the intersection points.
  int IntersectionCircleCircle(const Circle_<_Tp> &other,
      vec_type *intersection1 = nullptr,
      vec_type *intersection2 = nullptr) const;


  /// Returns the number of intersections (0, 1, or 2) of this circle and the
  /// line. Optionally sets the intersection points.
  int IntersectionCircleLine(
      const Line2d_<_Tp>& line,
      vec_type *intersection1 = nullptr,
      vec_type *intersection2 = nullptr) const;


  /// Returns the number of intersections (0, 1, or 2) of this circle and the
  /// line segment(!). Optionally sets the intersection points.
  int IntersectionCircleLineSegment(
      const Line2d_<_Tp>& segment,
      vec_type *intersection1 = nullptr,
      vec_type *intersection2 = nullptr) const;

private:
  vec_type center_;
  _Tp radius_;
};


/// Available specialization:
typedef Circle_<double> Circle;


//----------------------------------------------------
// Line in 2D

/// Represents a line/segment in 2d Euclidean space.
template <typename _Tp>
class Line2d_ {
public:
  static_assert(
      std::is_floating_point<_Tp>::value,
      "2D line type must be float or double!");

  using vec_type = Vec<_Tp, 2>;
  using vec3_type = Vec<_Tp, 3>;

  /// Default constructor yields an invalid line/segment
  Line2d_() : pt_from_(0, 0), pt_to_(0, 0) {}


  /// Construct a line from 2 real valued points. In case of a segment, these
  /// denote the start and end points.
  Line2d_(const vec_type &from, const vec_type &to)
    : pt_from_(from), pt_to_(to) {}


  /// Returns a line with flipped direction vector.
  inline Line2d_ Reversed() const { return Line2d_<_Tp>(pt_to_, pt_from_); }


  /// For a segment, this returns the start point. For a line, it's simply one
  /// of the given points to construct the line in the first place.
  const vec_type &From() const { return pt_from_; }


  /// For a segment, this returns the end point. For a line, it's simply one
  /// of the given points to construct the line in the first place.
  const vec_type &To() const { return pt_to_; }


  /// Returns the length from the start to the end point. As such, it's only
  /// meaningful for a line segment.
  inline double Length() const { return Direction().Length(); }


  /// Returns the non-normalized direction vector from the start point to the
  /// end point.
  inline vec_type Direction() const { return pt_from_.DirectionVector(pt_to_); }


  /// Returns the unit direction vector from the start point to the end point.
  inline vec_type UnitDirection() const { return Direction().UnitVector(); }


  /// Returns the point halfway between from and to.
  inline vec_type MidPoint() const { return 0.5 * (pt_from_ + pt_to_); }


  /// Returns true if the line object is valid, *i.e.* start and end points
  /// are not the same.
  inline bool IsValid() const {
    // Safe to use, because vec_type uses eps_equal for element comparison:
    return pt_from_ != pt_to_;
  }


  /// Returns the angle between the line and the given vector (not point(!)).
  inline double Angle(const vec_type &v) const {
    return std::acos(std::max(-1.0, std::min(1.0,
        static_cast<double>(UnitDirection().Dot(v.UnitVector())))));
  }


  /// Returns the line in homogeneous coordinates, *i.e.* a 3-element vector
  /// in P^2 (Projective 2-space). For more details on lines in projective
  /// space, refer to
  /// `Bob Fisher's CVonline <http://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/BEARDSLEY/node2.html>`__,
  /// or
  /// `Stan Birchfield's notes <http://robotics.stanford.edu/~birch/projective/node4.html>`__.
  inline vec3_type HomogeneousForm() const {
    return vec3_type{pt_from_[0], pt_from_[1], 1}.Cross(
          vec3_type{pt_to_[0], pt_to_[1], 1});
  }


  /// Returns the closest point on the line, i.e. the projection of the given
  /// point onto this line.
  vec_type ClosestPointOnLine(const vec_type &point) const;


  /// Returns the point on this line segment(!) which is closest to the
  /// given point.
  vec_type ClosestPointOnSegment(const vec_type &point) const;


  /// Returns the minimum distance between the point and this line.
  inline double DistancePointToLine(const vec_type &point) const {
    const vec_type closest_point = ClosestPointOnLine(point);
    return point.Distance(closest_point);
  }


  /// Returns the minimum distance between the point and this segment.
  inline double DistancePointToSegment(const vec_type &point) const {
    const vec_type closest = ClosestPointOnSegment(point);
    return closest.Distance(point);
  }


  /// Returns true if the two lines are collinear.
  bool IsCollinear(const Line2d_ &other) const;


  /// Returns true if the point is left of this line as specified by
  /// pt_from_ --> pt_to_. If you need to distinguish left-of vs. exactly on
  /// the line, pass a valid pointer `is_on_line`.
  bool IsPointLeftOfLine(
      const vec_type &point, bool *is_on_line = nullptr) const;


  /// Returns true if this line intersects the other line and optionally sets
  /// the `intersection_point`.
  bool IntersectionLineLine(
      const Line2d_ &other, vec_type *intersection_point = nullptr) const;


  /// Returns true if this line(!) intersects the other line segment(!) and
  /// optionally sets the `intersection_point`.
  bool IntersectionLineLineSegment(
      const Line2d_ &segment, vec_type *intersection_point = nullptr) const;


  /// Returns true if this line segment(!) intersects the other line segment(!)
  /// and optionally sets the `intersection_point`.
  bool IntersectionLineSegmentLineSegment(
      const Line2d_ &segment, vec_type *intersection_point = nullptr) const;


  /// Returns the number of intersections (0, 1, or 2) of this line and
  /// the circle. Optionally sets the intersection points.
  int IntersectionLineCircle(
      const Circle_<_Tp> &circle,
      vec_type *intersection1 = nullptr, vec_type *intersection2 = nullptr) const;


  /// Returns the number of intersections (0, 1, or 2) of this line segment(!)
  /// and the circle. Optionally sets the intersection points.
  int IntersectionLineSegmentCircle(
      const Circle_<_Tp> &circle,
      vec_type *intersection1 = nullptr, vec_type *intersection2 = nullptr) const;


  /// Clips this line(!) such that only the part within the rectangle
  /// remains, check via `IsValid()` afterwards.
  Line2d_<_Tp> ClipLineByRectangle(
      const vec_type &top_left, const vec_type &size) const;


  /// Clips this line segment(!) such that only the part within the rectangle
  /// remains, check via `IsValid()` afterwards.
  Line2d_<_Tp> ClipLineSegmentByRectangle(
      const vec_type &top_left, const vec_type &size) const;


  /// Overloaded output stream operator.
  friend std::ostream &operator<< (std::ostream &stream, const Line2d_ &line) {
    stream << "Line(" << line.pt_from_.ToString(false) << " --> "
           << line.pt_to_.ToString(false) << ')';
    return stream;
  }

private:
  /// Starting point of a line segment or one direction-defining point on a
  /// line.
  vec_type pt_from_;

  /// Ending point of a line segment or the second direction-defining point on
  /// a line.
  vec_type pt_to_;
};


/// A 2-dimensional line using double precision.
typedef Line2d_<double> Line2d;


//----------------------------------------------------
// Line in 3D

//FIXME



//----------------------------------------------------
// Plane

/// Represents a plane in 3d Euclidean space.
template <typename _Tp>
class Plane_ {
public:
  static_assert(
      std::is_floating_point<_Tp>::value,
      "Plane type must be float or double!");

  using vec_type = Vec<_Tp, 3>;

  /// Constructs an invalid plane.
  Plane_()
    : normal_(0, 0, 0), offset_(0)
  {}


  /// Constructs a plane from its Hessian form.
  Plane_(const vec_type &normal, _Tp offset)
    : normal_(normal.UnitVector()), offset_(offset)
  {}


  /// Constructs a plane from 3 points. If the points are collinear, the plane
  /// will be invalid.
  Plane_(const vec_type &p, const vec_type &q, const vec_type &r);


  /// Returns true if the plane has a valid normal vector.
  bool IsValid() const {
    return eps_equal(normal_.LengthSquared(), 1.0);
  }


  /// Returns the plane's normal vector.
  vec_type Normal() const { return normal_; }


  /// Returns the plane's offset, i.e. the distance from the plane to
  /// the coordinate system origin, measured along the plane's normal.
  /// Put differently, this is the minimum distance between the plane
  /// and the origin.
  _Tp Offset() const { return offset_; }


  /// Returns the distance from the given point to the plane.
  inline _Tp DistancePointToPlane(const vec_type &pt) const {
    return (normal_.Dot(pt) + offset_);
  }


  /// Returns true if the given point is in front of the plane, i.e. the
  /// side where the plane's normal points to.
  inline bool IsPointInFrontOfPlane(const vec_type &pt) const {
    return DistancePointToPlane(pt) >= 0.0f;
  }


  /// Returns true if the given point lies exactly on the plane.
  inline bool IsPointOnPlane(const vec_type &pt) const {
    return eps_zero(DistancePointToPlane(pt));
  }


  /// Returns the dihedral angle, *i.e.* the angle between the two planes,
  /// in radians.
  inline double Angle(const Plane_ &other) const {
    // Clamp the dot product to avoid numerical issues. Although unlikely,
    // these can occur (and they are a pain to debug, thus I prefer to err
    // on the safe side).
    return std::acos(
          std::max(-1.0, std::min(1.0, normal_.Dot(other.normal_))));
  }

  //TODO(vcp) AngleLinePlane
  //TODO(vcp) IntersectionLinePlane
  //TODO(vcp) IntersectionLineSegmentPlane
  //TODO(vcp/viren2d) AddPoly/IsInsidePolygon
  //TODO(vcp/viren2d) Origin/e1/e2


  /// Returns the plane's x-, y- and z-intercepts.
  inline vec_type XYZIntercepts() const {
    return vec_type(
          eps_zero(normal_.x())
            ? std::numeric_limits<_Tp>::infinity() : (-offset_ / normal_.x()),
          eps_zero(normal_.y())
            ? std::numeric_limits<_Tp>::infinity() : (-offset_ / normal_.y()),
          eps_zero(normal_.z())
            ? std::numeric_limits<_Tp>::infinity() : (-offset_ / normal_.z()));
  }


  /// Overloaded output stream operator.
  friend std::ostream &operator<< (std::ostream &stream, const Plane_ &plane) {
    stream << "Plane(" << plane.normal_.ToString(false) << ", " << plane.offset_ << ')';
    return stream;
  }

private:
  /// The plane's normal vector.
  vec_type normal_;

  /// Distance between plane and origin of the coordinate system.
  _Tp offset_;
};


/// The default plane type which uses double precision.
typedef Plane_<double> Plane;


} // namespace geometry
} // namespace werkzeugkiste

#endif // __WERKZEUGKISTE_GEOMETRY_UTILS_H__
