#ifndef WERKZEUGKISTE_GEOMETRY_PRIMITIVES_H
#define WERKZEUGKISTE_GEOMETRY_PRIMITIVES_H

#include <werkzeugkiste/geometry/geometry_export.h>
#include <werkzeugkiste/geometry/utils.h>
#include <werkzeugkiste/geometry/vector.h>

#include <cmath>
#include <limits>
#include <ostream>
#include <type_traits>
#include <vector>

namespace werkzeugkiste::geometry {

// TODO line2d, line3d, plane

// TODO Triangles

// TODO (Rotated) Rectangles IoU/Area
// TODO Polygon Inside/Outside/Distance, ConvexHull
// TODO RDP

/// Returns true if the given point is within the rectangle.
template <typename T>
inline bool IsPointInsideRectangle(const Vec<T, 2>& pt,
    const Vec<T, 2>& top_left,
    const Vec<T, 2>& size) {
  return (pt.X() >= top_left.X()) && (pt.X() < (top_left.X() + size.Width())) &&
         (pt.Y() >= top_left.Y()) && (pt.Y() < (top_left.Y() + size.Height()));
}

//----------------------------------------------------
// Circle

// Forward declaration needed by Circle_.
template <typename T>
class Line2d_;

template <typename T>
class Circle_ {  // NOLINT(readability-identifier-naming)
 public:
  static_assert(std::is_floating_point<T>::value,
      "Circle type must be float or double!");

  using vec_type = Vec<T, 2>;  // NOLINT(readability-identifier-naming)

  /// Constructs an invalid circle.
  Circle_() = default;

  /// Circle from center and radius.
  Circle_(vec_type center, T radius)
      : center_{std::move(center)}, radius_{radius} {}

  /// Circle from 3 points. If they are collinear, the
  /// circle will be invalid. Check via `IsValid()` afterwards.
  Circle_(const vec_type& p, const vec_type& q, const vec_type& r);

  /// Returns true if this is a valid circle.
  inline bool IsValid() const { return Sign(radius_) > 0; }

  /// Returns the center point as 2d vector.
  vec_type Center() const { return center_; }

  /// Returns the x-coordinate of the center point.
  T CenterX() const { return center_.X(); }

  /// Returns the y-coordinate of the center point.
  T CenterY() const { return center_.Y(); }

  /// Returns the radius.
  T Radius() const { return radius_; }

  /// Returns the area.
  T Area() const { return constants::pi_tpl<T> * (radius_ * radius_); }

  /// Returns true if the point is inside the circle. If you need to
  /// distinguish inside and exactly on the circle, pass a valid
  /// `is_on_circle` pointer.
  bool IsPointInCircle(const vec_type& pt, bool* is_on_circle = nullptr) const;

  /// Returns the number of points of tangency (i.e. 0, 1, or 2) between this
  /// circle and the given point. Optionally sets the points of tangency.
  int PointsOfTangency(const vec_type& pt,
      vec_type* pot1 = nullptr,
      vec_type* pot2 = nullptr) const;

  /// Returns the number of direct common tangents (German "aeussere
  /// Tangenten") between the two circles.
  /// Optionally sets the tangent lines.
  int DirectCommonTangents(const Circle_<T>& other,
      Line2d_<T>* tangent1 = nullptr,
      Line2d_<T>* tangent2 = nullptr) const;

  /// Returns the number of transverse common tangents (German "innere
  /// Tangenten") between the two circles. These are 0 if the circles
  /// overlap, 1 if they intersect in exactly one point, and 2 if the
  /// circles do not touch.
  /// Optionally sets the tangent lines.
  int TransverseCommonTangents(const Circle_<T>& other,
      Line2d_<T>* tangent1 = nullptr,
      Line2d_<T>* tangent2 = nullptr) const;

  /// Returns the number of intersections (0, 1, or 2) of the two circles.
  /// Returns -1 if the circles are equal and thus every point on the circles
  /// is an intersection point.
  /// Optionally sets the intersection points.
  int IntersectionCircleCircle(const Circle_<T>& other,
      vec_type* intersection1 = nullptr,
      vec_type* intersection2 = nullptr) const;

  /// Returns the number of intersections (0, 1, or 2) of this circle and the
  /// line. Optionally sets the intersection points.
  int IntersectionCircleLine(const Line2d_<T>& line,
      vec_type* intersection1 = nullptr,
      vec_type* intersection2 = nullptr) const;

  /// Returns the number of intersections (0, 1, or 2) of this circle and the
  /// line segment(!). Optionally sets the intersection points.
  int IntersectionCircleLineSegment(const Line2d_<T>& segment,
      vec_type* intersection1 = nullptr,
      vec_type* intersection2 = nullptr) const;

 private:
  vec_type center_{0, 0};
  T radius_{0};
};

/// Available double-precision specialization for Circle_.
extern template class WERKZEUGKISTE_GEOMETRY_EXPORT Circle_<double>;
using Circle = Circle_<double>;

//----------------------------------------------------
// Line in 2D

/// Represents a line/segment in 2d Euclidean space.
template <typename T>
class Line2d_ {  // NOLINT(readability-identifier-naming)
 public:
  static_assert(std::is_floating_point<T>::value,
      "2D line type must be float or double!");

  using vec_type = Vec<T, 2>;   // NOLINT(readability-identifier-naming)
  using vec3_type = Vec<T, 3>;  // NOLINT(readability-identifier-naming)

  /// Default constructor yields an invalid line/segment
  Line2d_() = default;

  /// Construct a line from 2 real valued points. In case of a segment, these
  /// denote the start and end points.
  Line2d_(vec_type from, vec_type to)
      : pt_from_{std::move(from)}, pt_to_{std::move(to)} {}

  /// Returns a line with flipped direction vector.
  inline Line2d_ Reversed() const { return Line2d_<T>{pt_to_, pt_from_}; }

  /// Returns a line where from/to are sorted left-to-right. If the line is
  /// vertical, they will be sorted top-to-bottom.
  Line2d_ LeftToRight() const;

  /// For a segment, this returns the start point. For a line, it's simply one
  /// of the given points to construct the line in the first place.
  const vec_type& From() const { return pt_from_; }

  /// Sets the first reference point (starting point of a line segment; or any
  /// point on a line).
  void SetFrom(const vec_type& from) { pt_from_ = from; }

  /// For a segment, this returns the end point. For a line, it's simply one
  /// of the given points to construct the line in the first place.
  const vec_type& To() const { return pt_to_; }

  /// Sets the second reference point (end point of a line segment; or any
  /// point on a line).
  void SetTo(const vec_type& to) { pt_to_ = to; }

  /// Returns the length from the start to the end point. As such, it's only
  /// meaningful for a line segment.
  inline double Length() const { return Direction().Length(); }

  /// Returns the non-normalized direction vector from the start point to the
  /// end point.
  inline vec_type Direction() const { return pt_from_.DirectionVector(pt_to_); }

  /// Returns the unit direction vector from the start point to the end point.
  inline vec_type UnitDirection() const { return Direction().UnitVector(); }

  /// Returns the point halfway between from and to.
  inline vec_type MidPoint() const {
    return 0.5 * (pt_from_ + pt_to_);  // NOLINT
  }

  /// Returns true if the line object is valid, *i.e.* start and end points
  /// are not the same.
  inline bool IsValid() const {
    // Safe to use, because vec_type uses eps_equal for element comparison:
    return pt_from_ != pt_to_;
  }

  /// Returns the angle between the line and the given directional vector.
  /// The angle will be between 0 and Pi.
  inline double AngleRad(const vec_type& v) const {
    // Clamp the dot product to avoid numerical issues. Although unlikely,
    // these can occur (and they are a pain to debug, thus I prefer to err
    // on the safe side).
    return std::acos(std::max(-1.0,
        std::min(
            1.0, static_cast<double>(UnitDirection().Dot(v.UnitVector())))));
  }

  /// Returns the angle between the line and the given directional vector.
  /// The angle will be between 0 and 180 degrees.
  inline double AngleDeg(const vec_type& v) const {
    return Rad2Deg(AngleRad(v));
  }

  /// Returns the line in homogeneous coordinates, *i.e.* a 3-element vector
  /// in P^2 (Projective 2-space). For more details on lines in projective
  /// space, refer to
  /// `Bob Fisher's CVonline
  /// <http://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/BEARDSLEY/node2.html>`__,
  /// or
  /// `Stan Birchfield's notes
  /// <http://robotics.stanford.edu/~birch/projective/node4.html>`__.
  inline vec3_type HomogeneousForm() const {
    return vec3_type{pt_from_[0], pt_from_[1], 1}.Cross(
        vec3_type{pt_to_[0], pt_to_[1], 1});
  }

  /// Returns the point which is offset * Direction() away from the line's
  /// starting point, i.e. offset == 0 is the starting point, offset == 1
  /// is the end point.
  vec_type PointAtOffset(double offset_factor) const {
    return pt_from_ + offset_factor * Direction();
  }

  /// Returns the closest point on the line, i.e. the projection of the given
  /// point onto this line.
  vec_type ClosestPointOnLine(const vec_type& point) const;

  /// Returns the point on this line segment(!) which is closest to the
  /// given point.
  vec_type ClosestPointOnSegment(const vec_type& point) const;

  /// Returns the minimum distance between the point and this line.
  inline double DistancePointToLine(const vec_type& point) const {
    const vec_type closest_point = ClosestPointOnLine(point);
    return point.DistanceEuclidean(closest_point);
  }

  /// Returns the minimum distance between the point and this segment.
  inline double DistancePointToSegment(const vec_type& point) const {
    const vec_type closest = ClosestPointOnSegment(point);
    return closest.DistanceEuclidean(point);
  }

  /// Returns true if the two lines are collinear.
  bool IsCollinear(const Line2d_& other) const;

  /// Returns true if the point is left of this line as specified by
  /// pt_from_ --> pt_to_. If you need to distinguish left-of vs. exactly on
  /// the line, pass a valid pointer `is_on_line`.
  bool IsPointLeftOfLine(const vec_type& point,
      bool* is_on_line = nullptr) const;

  /// Returns true if this line intersects the other line and optionally sets
  /// the `intersection_point`.
  bool IntersectionLineLine(const Line2d_& other,
      vec_type* intersection_point = nullptr) const;

  /// Returns true if this line(!) intersects the other line segment(!) and
  /// optionally sets the `intersection_point`.
  bool IntersectionLineLineSegment(const Line2d_& segment,
      vec_type* intersection_point = nullptr) const;

  /// Returns true if this line segment(!) intersects the other line segment(!)
  /// and optionally sets the `intersection_point`.
  bool IntersectionLineSegmentLineSegment(const Line2d_& segment,
      vec_type* intersection_point = nullptr) const;

  /// Returns the number of intersections (0, 1, or 2) of this line and
  /// the circle. Optionally sets the intersection points.
  int IntersectionLineCircle(const Circle_<T>& circle,
      vec_type* intersection1 = nullptr,
      vec_type* intersection2 = nullptr) const;

  /// Returns the number of intersections (0, 1, or 2) of this line segment(!)
  /// and the circle. Optionally sets the intersection points.
  int IntersectionLineSegmentCircle(const Circle_<T>& circle,
      vec_type* intersection1 = nullptr,
      vec_type* intersection2 = nullptr) const;

  /// Clips this line(!) such that only the part within the rectangle
  /// remains, check via `IsValid()` afterwards.
  Line2d_<T> ClipLineByRectangle(const vec_type& top_left,
      const vec_type& size) const;

  /// Clips this line segment(!) such that only the part within the rectangle
  /// remains, check via `IsValid()` afterwards.
  Line2d_<T> ClipLineSegmentByRectangle(const vec_type& top_left,
      const vec_type& size) const;

  // TODO doc
  Line2d_<T> TiltRad(double angle_rad) const;

  /// @brief Tilts the line/segment, *i.e.* rotates the end point around the
  ///   start point.
  /// @param angle_deg Tilt angle in degrees.
  /// @return A new line which has been rotated around the start point.
  inline Line2d_<T> TiltDeg(double angle_deg) const {
    return TiltRad(Deg2Rad(angle_deg));
  }

  /// Overloaded output stream operator.
  friend std::ostream& operator<<(std::ostream& stream, const Line2d_& line) {
    constexpr bool include_type = false;
    stream << "Line(" << line.pt_from_.ToString(include_type) << " --> "
           << line.pt_to_.ToString(include_type) << ')';
    return stream;
  }

 private:
  /// Starting point of a line segment or one direction-defining point on a
  /// line.
  vec_type pt_from_{0, 0};

  /// Ending point of a line segment or the second direction-defining point on
  /// a line.
  vec_type pt_to_{0, 0};
};

/// A 2-dimensional line using double precision.
extern template class WERKZEUGKISTE_GEOMETRY_EXPORT Line2d_<double>;
using Line2d = Line2d_<double>;

//----------------------------------------------------
// Line in 3D

// Forward declaration needed by Line3d_.
template <typename T>
class Plane_;

/// Represents a line/segment in 3d Euclidean space.
template <typename T>
class Line3d_ {  // NOLINT(readability-identifier-naming)
 public:
  static_assert(std::is_floating_point<T>::value,
      "3D line type must be float or double!");

  using vec_type = Vec<T, 3>;  // NOLINT(readability-identifier-naming)

  /// Default constructor yields an invalid line/segment
  Line3d_() = default;

  /// Construct a line from 2 real valued points. In case of a segment, these
  /// denote the start and end points.
  Line3d_(vec_type from, vec_type to)
      : pt_from_{std::move(from)}, pt_to_{std::move(to)} {}

  /// Returns a line with flipped direction vector.
  inline Line3d_ Reversed() const { return Line3d_<T>{pt_to_, pt_from_}; }

  /// For a segment, this returns the start point. For a line, it's simply one
  /// of the given points to construct the line in the first place.
  const vec_type& From() const { return pt_from_; }

  /// Sets the first reference point (starting point of a line segment; or any
  /// point on a line).
  void SetFrom(const vec_type& from) { pt_from_ = from; }

  /// For a segment, this returns the end point. For a line, it's simply one
  /// of the given points to construct the line in the first place.
  const vec_type& To() const { return pt_to_; }

  /// Sets the second reference point (end point of a line segment; or any
  /// point on a line).
  void SetTo(const vec_type& to) { pt_to_ = to; }

  /// Returns the length from the start to the end point. As such, it's only
  /// meaningful for a line segment.
  inline double Length() const { return Direction().Length(); }

  /// Returns the non-normalized direction vector from the start point to the
  /// end point.
  inline vec_type Direction() const { return pt_from_.DirectionVector(pt_to_); }

  /// Returns the unit direction vector from the start point to the end point.
  inline vec_type UnitDirection() const { return Direction().UnitVector(); }

  /// Returns the point halfway between from and to.
  inline vec_type MidPoint() const {
    return 0.5 * (pt_from_ + pt_to_);  // NOLINT
  }

  /// Returns true if the line object is valid, *i.e.* start and end points
  /// are not the same.
  inline bool IsValid() const {
    // Safe to use, because vec_type uses eps_equal for element comparison:
    return pt_from_ != pt_to_;
  }

  /// Returns the angle between the line and the given directional vector.
  /// The angle will be between 0 and Pi.
  inline double AngleRad(const vec_type& v) const {
    return std::acos(std::max(-1.0,
        std::min(
            1.0, static_cast<double>(UnitDirection().Dot(v.UnitVector())))));
  }

  /// Returns the angle between the line and the given directional vector.
  /// The angle will be between 0 and 180 degrees.
  inline double AngleDeg(const vec_type& v) const {
    return Rad2Deg(AngleRad(v));
  }

  /// Returns the angle between the line and the plane.
  /// The angle will be between -Pi/2 and Pi/2.
  inline double AngleRad(const Plane_<T>& plane) const {
    return std::asin(std::max(-1.0,
        std::min(
            1.0, static_cast<double>(UnitDirection().Dot(plane.Normal())))));
  }

  /// Returns the angle between the line and the plane.
  /// The angle will be between -90 and +90 degrees.
  inline double AngleDeg(const Plane_<T>& plane) const {
    return Rad2Deg(AngleRad(plane));
  }

  /// Returns the point which is offset * Direction() away from the line's
  /// starting point, i.e. offset == 0 is the starting point, offset == 1
  /// is the end point.
  vec_type PointAtOffset(double offset_factor) const {
    return pt_from_ + offset_factor * Direction();
  }

  /// Returns the closest point on the line, i.e. the projection of the given
  /// point onto this line.
  vec_type ClosestPointOnLine(const vec_type& point) const;

  /// Returns the point on this line segment(!) which is closest to the
  /// given point.
  vec_type ClosestPointOnSegment(const vec_type& point) const;

  /// Returns the minimum distance between the point and this line.
  inline double DistancePointToLine(const vec_type& point) const {
    const vec_type closest_point = ClosestPointOnLine(point);
    return point.DistanceEuclidean(closest_point);
  }

  /// Returns the minimum distance between the point and this segment.
  inline double DistancePointToSegment(const vec_type& point) const {
    const vec_type closest = ClosestPointOnSegment(point);
    return closest.DistanceEuclidean(point);
  }

  //  /// Returns true if the two lines are collinear.
  //  bool IsCollinear(const Line3d_ &other) const;

  // TODO bool IntersectionLinePlane(const Line3d &line, const cv::Vec4d &plane,
  // cv::Vec3d *intersection_point=nullptr);
  // TODO bool IntersectionLineSegmentPlane(const Line3d &line_segment, const
  // cv::Vec4d &plane, cv::Vec3d *intersection_point=nullptr);
  // TODO Line3d ClipLineSegmentByPlane(const Line3d &line_segment, const
  // cv::Vec4d &plane);

  /// Overloaded output stream operator.
  friend std::ostream& operator<<(std::ostream& stream, const Line3d_& line) {
    constexpr bool include_type = false;
    stream << "Line(" << line.pt_from_.ToString(include_type) << " --> "
           << line.pt_to_.ToString(include_type) << ')';
    return stream;
  }

 private:
  /// Starting point of a line segment or one direction-defining point on a
  /// line.
  vec_type pt_from_{0, 0, 0};

  /// Ending point of a line segment or the second direction-defining point on
  /// a line.
  vec_type pt_to_{0, 0, 0};
};

/// A 3-dimensional line using double precision.
extern template class WERKZEUGKISTE_GEOMETRY_EXPORT Line3d_<double>;
using Line3d = Line3d_<double>;

//----------------------------------------------------
// Plane

/// Represents a plane in 3d Euclidean space.
template <typename T>
class Plane_ {  // NOLINT(readability-identifier-naming)
 public:
  static_assert(std::is_floating_point<T>::value,
      "Plane type must be float or double!");

  using vec_type = Vec<T, 3>;  // NOLINT(readability-identifier-naming)

  /// Constructs an invalid plane.
  Plane_() = default;

  /// Constructs a plane from its Hessian form.
  Plane_(const vec_type& normal, T offset)
      : normal_{normal.UnitVector()}, offset_{offset} {}

  /// Constructs a plane from 3 points. If the points are collinear, the plane
  /// will be invalid.
  Plane_(const vec_type& p, const vec_type& q, const vec_type& r);

  /// Returns true if the plane has a valid normal vector.
  bool IsValid() const { return IsEpsEqual(normal_.LengthSquared(), 1.0); }

  /// Returns the plane's normal vector.
  vec_type Normal() const { return normal_; }

  /// Returns the plane's offset, i.e. the distance from the plane to
  /// the coordinate system origin, measured along the plane's normal.
  /// Put differently, this is the minimum distance between the plane
  /// and the origin.
  T Offset() const { return offset_; }

  /// Returns the distance from the given point to the plane.
  inline T DistancePointToPlane(const vec_type& pt) const {
    return (normal_.Dot(pt) + offset_);
  }

  /// Returns true if the given point is in front of the plane, i.e. the
  /// side where the plane's normal points to.
  inline bool IsPointInFrontOfPlane(const vec_type& pt) const {
    return Sign(DistancePointToPlane(pt)) >= 0;
  }

  /// Returns true if the given point lies exactly on the plane.
  inline bool IsPointOnPlane(const vec_type& pt) const {
    return IsEpsZero(DistancePointToPlane(pt));
  }

  /// Returns the dihedral angle, *i.e.* the angle between the two planes,
  /// in radians. The angle will be between 0 and Pi.
  inline double AngleRad(const Plane_& other) const {
    return std::acos(std::max(-1.0, std::min(1.0, normal_.Dot(other.normal_))));
  }

  /// Returns the dihedral angle, *i.e.* the angle between the two planes,
  /// in degrees. The angle will be between 0 and 180.
  inline double AngleDeg(const Plane_& other) const {
    return Rad2Deg(AngleRad(other));
  }

  /// Returns the angle between this plane and the line in radians.
  /// The angle will be between -Pi/2 and Pi/2.
  inline double AngleRad(const Line3d_<T>& line) const {
    return line.AngleRad(*this);
  }

  /// Returns the angle between this plane and the line in degrees.
  /// The angle will be between -90 and +90.
  inline double AngleDeg(const Line3d_<T>& line) const {
    return line.AngleDeg(*this);
  }

  // TODO(vcp) IntersectionLinePlane
  // TODO(vcp) IntersectionLineSegmentPlane
  // TODO(vcp/viren2d) AddPoly/IsInsidePolygon
  // TODO(vcp/viren2d) Origin/e1/e2

  /// Returns the plane's x-, y- and z-intercepts.
  inline vec_type XYZIntercepts() const {
    return vec_type{IsEpsZero(normal_.X()) ? std::numeric_limits<T>::infinity()
                                           : (-offset_ / normal_.X()),
        IsEpsZero(normal_.Y()) ? std::numeric_limits<T>::infinity()
                               : (-offset_ / normal_.Y()),
        IsEpsZero(normal_.Z()) ? std::numeric_limits<T>::infinity()
                               : (-offset_ / normal_.Z())};
  }

  /// Overloaded output stream operator.
  friend std::ostream& operator<<(std::ostream& stream, const Plane_& plane) {
    stream << "Plane(" << plane.normal_.ToString(false) << ", " << plane.offset_
           << ')';
    return stream;
  }

 private:
  /// The plane's normal vector.
  vec_type normal_{0, 0, 0};

  /// Distance between plane and origin of the coordinate system.
  T offset_{0};
};

/// The default plane type which uses double precision.
extern template class WERKZEUGKISTE_GEOMETRY_EXPORT Plane_<double>;
using Plane = Plane_<double>;

}  // namespace werkzeugkiste::geometry

#endif  // WERKZEUGKISTE_GEOMETRY_UTILS_H
