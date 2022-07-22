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
// CircleFrom3Points
// IntersectionCircleCircle
// IsPointInCircle
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


/// Represents a line/segment in 2d Euclidean space.
template <typename _Tp>
class Line2d_ {
public:
  using vec_type = Vec<_Tp, 2>;
  using vec3_type = Vec<_Tp, 3>;

  /// Default constructor yields an invalid line/segment
  Line2d_() : pt_from_(0, 0), pt_to_(0, 0) {}


  /// Construct a line from 2 real valued points. In case of a segment, these
  /// denote the start and end points.
  Line2d_(const vec_type &from, const vec_type &to)
    : pt_from_(from), pt_to_(to) {}


  /// Returns a line with flipped direction vector.
  Line2d_ Flipped() const { return Line2d_<_Tp>(pt_to_, pt_from_); }


  /// For a segment, this returns the start point. For a line, it's simply one
  /// of the given points to construct the line in the first place.
  const vec_type &From() const { return pt_from_; }


  /// For a segment, this returns the end point. For a line, it's simply one
  /// of the given points to construct the line in the first place.
  const vec_type &To() const { return pt_to_; }


  /// Returns the length from the start to the end point. As such, it's only
  /// meaningful for a line segment.
  double Length() const { return Direction().Length(); }


  /// Returns the non-normalized direction vector from the start point to the
  /// end point.
  vec_type Direction() const { return pt_from_.DirectionVector(pt_to_); }


  /// Returns the unit direction vector from the start point to the end point.
  vec_type UnitDirection() const { return Direction().UnitVector(); }


  /// Returns the point halfway between from and to.
  vec_type MidPoint() const { return 0.5 * (pt_from_ + pt_to_); }


  /// Returns true if the line object is valid, *i.e.* start and end points
  /// are not the same.
  bool IsValid() const { return !eps_equal(pt_from_, pt_to_); }


  /// Returns the angle between the line and the given vector (not point(!)).
  double Angle(const vec_type &v) const {
    return std::acos(std::max(-1.0, std::min(1.0,
        static_cast<double>(UnitDirection().Dot(v.UnitVector())))));
  }


  /// Returns the line in homogeneous coordinates, *i.e.* a 3-element vector
  /// in P^2 (Projective 2-space). For more details on lines in projective
  /// space, refer to
  /// `Bob Fisher's CVonline <http://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/BEARDSLEY/node2.html>`__,
  /// or
  /// `Stan Birchfield's notes <http://robotics.stanford.edu/~birch/projective/node4.html>`__.
  vec3_type HomogeneousForm() const {
    return vec3_type{pt_from_[0], pt_from_[1], 1}.Cross(
          vec3_type{pt_to_[0], pt_to_[1], 1});
  }


  /// Returns the closes point on the line, i.e. the projection of the given
  /// point onto this line.
  vec_type ClosestPointOnLine(const vec_type &point) const {
    // Vector from line start to point:
    const vec_type v = pt_from_.DirectionVector(point);
    // Project onto line and get closest point on line:
    const vec_type unit_dir = UnitDirection();
    const _Tp lambda = unit_dir.Dot(v);
    return pt_from_ + lambda * unit_dir;
  }


  /// Returns the point on this line segment(!) which is closest to the
  /// given point.
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
      if (lambda > Length()) {
        return pt_to_;
      } else {
        return pt_from_ + lambda * unit_direction;
      }
    }
  }


  /// Returns the minimum distance between the point and this line.
  double DistancePointToLine(const vec_type &point) const {
    const vec_type closest_point = ProjectPointOntoLine(point);
    return point.Distance(closest_point);
  }


  /// Returns the minimum distance between the point and this segment.
  double DistancePointToSegment(const vec_type &point) const {
    const vec_type closest = ClosestPointOnSegment(point);
    return closest.Distance(point);
  }


  /// Returns true if the two lines are collinear.
  bool IsCollinear(const Line2d_ &other) const {
    // Line 1 goes from p to p + r
    const vec_type p = pt_from_;
    const vec_type r = Direction();
    // Line 2 goes from q to q + s
    const vec_type q = other.pt_from_;
    const vec_type s = other.Direction();

    const _Tp rxs = Determinant(r, s);
    const _Tp qmpxr = Determinant((q-p), r);

    return (eps_zero(rxs) && eps_zero(qmpxr));
  }


  /// Returns true if the point is left of this line as specified by
  /// pt_from_ --> pt_to_. If you need to distinguish left-of vs. exactly on
  /// the line, pass a valid pointer `is_on_line`.
  bool IsPointLeftOfLine(const vec_type &point, bool *is_on_line = nullptr) const {
    const _Tp det = Determinant(Direction(), point - pt_to_);

    // If the "2d cross product" (i.e. determinant) is 0, the points are
    // collinear, and thus, would be on the line.
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


  /// Returns true if this line intersects the other line and optionally sets
  /// the `intersection_point`.
  bool IntersectionLineLine(
      const Line2d_ &other, vec_type *intersection_point) const {
    const vec3_type ip = HomogeneousForm().Cross(other.HomogeneousForm());
    if (eps_zero(ip[2])) {
      // Intersection at infinity
      return false;
    } else {
      if (intersection_point) {
        intersection_point->SetX(ip[0] / ip[2]);
        intersection_point->SetY(ip[1] / ip[2]);
      }
      return true;
    }
  }

  /// Returns true if this line(!) intersects the other line segment(!) and
  /// optionally sets the `intersection_point`.
  bool IntersectionLineLineSegment(
      const Line2d_ &segment, vec_type *intersection_point) const {
    // Line 1 goes from p to p + r
    const vec_type p = pt_from_;
    const vec_type r = Direction();
    // Segment 2 goes from q to q + s
    const vec_type q = segment.pt_from_;
    const vec_type s = segment.Direction();

    const _Tp rxs = Determinant(r, s);
    const _Tp qmpxr = Determinant((q-p), r);

    if (eps_zero(rxs) && eps_zero(qmpxr)) {
      // Line and segment are collinear.
      if (intersection_point) {
        *intersection_point = segment.From();
      }
      return true;
    } else {
      if (eps_zero(rxs)) {
        // Parallel and not intersecting
        return false;
      } else {
        // Otherwise, they intersect if the intersection point lies on the
        // segment, i.e. u in [0,1].
        const _Tp u = qmpxr / rxs;

        if ((u >= 0.0f) && (u <= 1.0f)) {
          if (intersection_point) {
            *intersection_point = q + u * s;
          }
          return true;
        } else {
          return false;
        }
      }
    }
  }


  /// Returns true if this line segment(!) intersects the other line segment(!)
  /// and optionally sets the `intersection_point`.
  bool IntersectionLineSegmentLineSegment(
      const Line2d_ &segment, vec_type *intersection_point) const {
    // Based on https://stackoverflow.com/a/565282/400948
    // Line 1 goes from p to p + r
    const vec_type p = pt_from_;
    const vec_type r = Direction();
    // Segment 2 goes from q to q + s
    const vec_type q = segment.pt_from_;
    const vec_type s = segment.Direction();

    const _Tp rxs = Determinant(r, s);
    const _Tp qmpxr = Determinant((q-p), r);

    if (eps_zero(rxs) && eps_zero(qmpxr)) {
      // Lines are collinear. They intersect if there is any overlap.
      const _Tp t0 = (q-p).Dot(r) / r.Dot(r);
      const double t1 = t0 + s.Dot(r) / r.Dot(r);
      if ((t0 >= 0.0f) && (t0 <= 1.0f)) {
        if (intersection_point) {
          *intersection_point = p + t0 * r;
        }
        return true;
      } else {
        if ((t1 >= 0.0f) && (t1 <= 1.0f)) {
          if (intersection_point) {
            *intersection_point = p + t1 * r;
          }
          return true;
        }
        // Otherwise, the segments don't intersect
        return false;
      }
    } else {
      if (eps_zero(rxs)) {
        // Segments are parallel and not intersecting
        return false;
      } else {
        // Otherwise, the segments meet if u in [0,1] and t in [0,1].
        const _Tp u = qmpxr / rxs;
        const _Tp t = Determinant((q-p), s) / Determinant(r, s);

        if ((u >= 0.0f) && (u <= 1.0f) && (t >= 0.0f) && (t <= 1.0f)) {
          if (intersection_point) {
            *intersection_point = p + t * r;
          }
          return true;
        } else {
          return false;
        }
      }
    }
  }

  //TODO(vcp) int IntersectionLineCircle(const vec_type &center, _Tp radius, vec_type *intersection1, vec_type *intersection2) const;
  //TODO(vcp) int IntersectionLineSegmentCircle(const vec_type &center, _Tp radius, vec_type *intersection1, vec_type *intersection2) const;

  /// Clips this line(!) such that only the part within the rectangle
  /// remains, check via `IsValid()` afterwards.
  Line2d_<_Tp> ClipLineByRectangle(
      const vec_type &top_left, const vec_type &size) const {
    const vec_type top_right{top_left.x() + size.width(), top_left.y()};
    const vec_type bottom_right{top_right.x(), top_left.y() + size.height()};
    const vec_type bottom_left{top_left.x(), bottom_right.y()};

    const std::vector<Line2d_> edges {
      Line2d_<_Tp>{top_left, top_right},
      Line2d_<_Tp>{top_right, bottom_right},
      Line2d_<_Tp>{bottom_right, bottom_left},
      Line2d_<_Tp>{bottom_left, top_left}
    };

    std::vector<vec_type> int_points;
    for (std::size_t i = 0; i < 4; ++i) {
      if (IsCollinear(edges[i])) {
        return edges[i];
      }

      vec_type ip;
      if (IntersectionLineLineSegment(edges[i], &ip)) {
        // We iterate the rect edges clockwise. Thus, if an intersection
        // point falls exactly on the corner, we would have found it already
        // when testing the previous edge.
        // If an intersection is the top-left corner, it would still be added
        // twice (once as int_points[0], then as int_points[2]) - however, this
        // can easily be handled by only building the clipped line only from
        // int_points[0] to int_points[1] ;-)
        if (int_points.empty() || (ip != int_points[int_points.size() - 1])) {
          int_points.push_back(ip);
        }
      }
    }

    if (int_points.size() < 2) {
      return Line2d_<_Tp>();
    } else {
      return Line2d_<_Tp>(int_points[0], int_points[1]);
    }
  }


  /// Clips this line segment(!) such that only the part within the rectangle
  /// remains, check via `IsValid()` afterwards.
  Line2d_<_Tp> ClipLineSegmentByRectangle(
      const vec_type &top_left, const vec_type &size) const {
    const bool is_from_inside = IsPointInsideRectangle(
          pt_from_, top_left, size);
    const bool is_to_inside = IsPointInsideRectangle(
          pt_to_, top_left, size);

    if (is_from_inside && is_to_inside) {
      return *this;
    }

    const vec_type top_right{top_left.x() + size.width(), top_left.y()};
    const vec_type bottom_right{top_right.x(), top_left.y() + size.height()};
    const vec_type bottom_left{top_left.x(), bottom_right.y()};

    const std::vector<Line2d_> edges {
      Line2d_<_Tp>{top_left, top_right},
      Line2d_<_Tp>{top_right, bottom_right},
      Line2d_<_Tp>{bottom_right, bottom_left},
      Line2d_<_Tp>{bottom_left, top_left}
    };

    std::vector<vec_type> int_points;
    for (std::size_t i = 0; i < 4; ++i) {
      vec_type ip;
      if (IntersectionLineSegmentLineSegment(edges[i], &ip)) {
        if (int_points.empty() || (ip != int_points[int_points.size() - 1])) {
          int_points.push_back(ip);
        }
      }
    }

    if (int_points.empty()) {
      return Line2d_<_Tp>();
    } else {
      if (is_from_inside != is_to_inside) {
        // One in, one out - there should be only 1 intersection point (unless
        // the intersection falls exactly on a corner, then there may be two).
        if (is_from_inside) {
          return Line2d_<_Tp>(pt_from_, int_points[0]);
        } else {
          return Line2d_<_Tp>(int_points[0], pt_to_);
        }
      } else {
        return Line2d_<_Tp>(int_points[0], int_points[1]);
      }
    }
  }

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
  Plane_(const vec_type &p, const vec_type &q, const vec_type &r)
    : Plane_() {
    const vec_type pq = p.DirectionVector(q);
    const vec_type pr = q.DirectionVector(r);
    const vec_type cross = pq.Cross(pr);

    if (!eps_zero(cross.LengthSquared())) {
      normal_ = cross.UnitVector();
      offset_ = -normal_.Dot(p);
    }
    // TODO: implement logging and warn about 3 collinear points.
  }

  /// Returns true if the plane has a valid normal vector.
  bool IsValid() const {
    return eps_equal(normal_.LengthSquared(), 1.0);
  }


  /// Returns the plane's normal vector.
  vec_type Normal() const { return normal_; }


  /// Returns the plane's offset, i.e. the distance from the plane to
  /// the coordinate system origin, measured along the plane's normal.
  _Tp Offset() const { return offset_; }


  /// Returns the distance from the given point to the plane.
  _Tp DistancePointToPlane(const vec_type &pt) const {
    return (normal_.Dot(pt) + offset_);
  }


  /// Returns true if the given point is in front of the plane, i.e. the
  /// side where the plane's normal points to.
  bool IsPointInFrontOfPlane(const vec_type &pt) const {
    return DistancePointToPlane(pt) >= 0.0f;
  }


  /// Returns true if the given point lies exactly on the plane.
  bool IsPointOnPlane(const vec_type &pt) const {
    return eps_zero(DistancePointToPlane(pt));
  }


  /// Returns the dihedral angle, *i.e.* the angle between the two planes,
  /// in radians.
  double Angle(const Plane_ &other) const {
    // Clamp the dot product to avoid numerical issues (which are a pain to
    // debug).
    return std::acos(std::max(-1.0, std::min(1.0, normal_.Dot(other.normal_))));
  }

  //TODO(vcp) AngleLinePlane
  //TODO(vcp) IntersectionLinePlane
  //TODO(vcp) IntersectionLineSegmentPlane
  //TODO(vcp/viren2d) AddPoly/IsInsidePolygon
  //TODO(vcp/viren2d) Origin/e1/e2


  /// Returns the plane's x-, y- and z-intercepts.
  vec_type XYZIntercepts() const {
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
