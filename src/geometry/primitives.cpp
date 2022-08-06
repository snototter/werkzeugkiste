#include <cmath>

#include <werkzeugkiste/geometry/primitives.h>
#include <werkzeugkiste/geometry/utils.h>

namespace werkzeugkiste {
namespace geometry {

//----------------------------------------------------
// Circle
template <typename _Tp>
Circle_<_Tp>::Circle_(
    const vec_type &p, const vec_type &q, const vec_type &r)
  : Circle_<_Tp>() {
  // Check if the points are collinear
  const Line2d_<_Tp> segment1(p, q);
  const Line2d_<_Tp> segment2(q, r);

  if (!segment1.IsCollinear(segment2)) {
    // Compute center as the intersection point of the segment bisections:
    // First, we need the bisecting lines
    const vec_type mid1 = segment1.MidPoint();
    const vec_type dir1 = segment1.Direction();
    const vec_type orth1(dir1.val[1], -dir1.val[0]);
    const Line2d_<_Tp> bisect1(mid1, mid1 + orth1);
    // ... same for the second one
    const vec_type mid2 = segment2.MidPoint();
    const vec_type dir2 = segment2.Direction();
    const vec_type orth2(dir2.val[1], -dir2.val[0]);
    const Line2d_<_Tp> bisect2(mid2, mid2 + orth2);
    // Intersect them
    const bool exists = bisect1.IntersectionLineLine(bisect2, &center_);
    if (exists) {
    //TODO warn that intersection of the bisecting lines does not exist (most
    //  likely due to numerical issue)
      radius_ = p.Distance(center_);
    }
  }
}


template <typename _Tp>
bool Circle_<_Tp>::IsPointInCircle(
    const vec_type &pt, bool *is_on_circle) const {
  const double distance = pt.Distance(center_);

  if (eps_equal(distance, radius_)) {
    if (is_on_circle) {
      *is_on_circle = true;
    }
    return true;
  } else {
    if (is_on_circle) {
      *is_on_circle = false;
    }

    return (distance < radius_);
  }
}


template <typename _Tp>
int Circle_<_Tp>::PointsOfTangency(
    const vec_type &pt, vec_type *pot1, vec_type *pot2) const {
  bool is_on_circle;
  if (IsPointInCircle(pt, &is_on_circle)) {
    return 0;
  }

  if (is_on_circle) {
    if (pot1) {
      *pot1 = pt;
    }
    return 1;
  }

  // Point lies outside the circle. Thus, we have 2 points of tangency which
  // can be computed from good ol' trigonometry:
  //
  //
  //                    __...------__    pot1
  //               _.-''             -(+)
  //            ,-'                   |----
  //          ,'                     ||    ----
  //        ,'                      | |     '  ----
  //       /                       |  |      `     ----
  //      /               radius  |   |  h    `.       ----  distance
  //     /                       |    |        \           ----
  //    |                       |     |         |              ----
  //    |                      |      |          |                ( ----
  //    |                     |   q   |          |    p          (  alpha ----
  //    |                   (+)---------------------------------------------(+) pt
  //    |                                        .'    hypotenuse
  //    |                    C                   |
  //     |                                       '
  //      \                                     /
  //       \                                  ,'
  //        `                                /
  //         '.                            ,'
  //           '-.                      _,'
  //              '-._              _,(+)  pot
  //                  '`--......---'


  if (pot1 || pot2) {
    // Pythagoras
    const double hypotenuse = pt.Distance(center_);
    const double radius_sqr = radius_ * radius_;
    const double distance = std::sqrt((hypotenuse * hypotenuse) - radius_sqr);

    // Euclid (German "Kathetensatz"):
    const double q = radius_sqr / hypotenuse;
    const double p = hypotenuse - q;
    // Euclid (German "Hoehensatz"):
    const double h = std::sqrt(p*q);
    // Trigonometry: tan(alpha) = h/p = a/b
    const double alpha = std::atan2(h, p);


    const Line2d hypo(pt, center_);
    const Vec2d to_rotate = distance * hypo.UnitDirection();

    Vec2d dir = RotateVector(to_rotate, alpha);
    if (pot1) {
      *pot1 = pt + dir;
    }
    dir = RotateVector(to_rotate, -alpha);
    if (pot2) {
      *pot2 = pt + dir;
    }
  }

  return 2;
}


template <typename _Tp>
int Circle_<_Tp>::DirectCommonTangents(
    const Circle_<_Tp> &other, Line2d_<_Tp> *tangent1,
    Line2d_<_Tp> *tangent2) const {

  // Tangents (transverse and/or direct) depending on distance between
  // the circles:
  // * r1 + r2 < center_dist:  4 tangents (2 direct, 2 transverse)
  // * r1 + r2 == center_dist: 3 tangents (2 direct, 1 transverse)
  // * |r1-r2| < center_dist < r1+r2: 2 tangents (direct, no transverse)
  // * abs(r1-r2) == center_dist: 1 tangent
  // * center_dist < abs(r1-r2): no tangent

  if ((center_ == other.center_) && eps_equal(radius_, other.radius_)) {
    // No tangent if the circles are the same
    return 0;
  }

  const double center_distance = center_.Distance(other.center_);
  if (center_distance < std::fabs(radius_ - other.radius_)) {
    // C1 is inside C2 (or vice versa) and they don't touch.
    return 0;
  } else {
    const Line2d_<_Tp> center_line(center_, other.center_);

    if (eps_equal(center_distance, std::fabs(radius_ - other.radius_))) {
      // C1 is inside C2 (or vice versa) and they intersect in exactly one
      // point.
      if (tangent1) {
        const vec_type unit_dir = center_line.UnitDirection();
        const vec_type intersection = center_ + (radius_ * unit_dir);
        const vec_type ref1 = intersection + (radius_ * vec_type(unit_dir.y(), -unit_dir.x()));
        const vec_type ref2 = intersection - (radius_ * vec_type(unit_dir.y(), -unit_dir.x()));
        *tangent1 = Line2d_<_Tp>{ref1, ref2};
      }
      return 1;
    } else {
      // In all other cases, there exist two direct common tangents.
      if (tangent1 || tangent2) {
        if (eps_equal(radius_, other.radius_)) {
          // Tangents are parallel to the center-connecting line
          const vec_type dir = radius_ * center_line.UnitDirection();
          const vec_type orth_dir{dir.y(), -dir.x()};

          // Intersection points are perpendicular to the center line:
          const vec_type t11 = center_ + orth_dir;
          const vec_type t12 = center_ - orth_dir;
          const vec_type t21 = other.center_ + orth_dir;
          const vec_type t22 = other.center_ - orth_dir;

          if (tangent1) {
            *tangent1 = Line2d_<_Tp>{t11, t21};
          }
          if (tangent2) {
            *tangent2 = Line2d_<_Tp>{t12, t22};
          }
        } else {
          // Similar triangles:
          // (distance c1 to intersection) / (distance c2 to intersection) =
          //   radius1 / radius2
          const double distance_intersection =
              center_distance + (center_distance * other.radius_) / (radius_ - other.radius_);
          const vec_type intersection = center_ + (distance_intersection * center_line.UnitDirection());

          // Now compute the points of tangency w.r.t. both circles.
          vec_type t11, t12, t21, t22;
          // TODO warn if we couldn't find POT (would be a numerical issue)
//          if (!PointsOfTangency(intersection, &t11, &t12))
          PointsOfTangency(intersection, &t11, &t12);
          other.PointsOfTangency(intersection, &t21, &t22);

          if (tangent1) {
            *tangent1 = Line2d_<_Tp>{t11, t21};
          }
          if (tangent2) {
            *tangent2 = Line2d_<_Tp>{t12, t22};
          }
        }
      } // end-if (tangent1 || tangent2)
      return 2;
    }
  }
}


template <typename _Tp>
int Circle_<_Tp>::TransverseCommonTangents(
    const Circle_<_Tp> &other, Line2d_<_Tp> *tangent1,
    Line2d_<_Tp> *tangent2) const {
  const double distance = center_.Distance(other.center_);
  const double sum_radii = radius_ + other.radius_;

  if (distance < sum_radii) {
    // No transverse common tangents if there's more than 1 intersection point.
    return 0;
  }

  const Line2d_<_Tp> center_line(center_, other.center_);
  if (eps_equal(distance, sum_radii)) {
    if (tangent1) {
      // Compute the single intersection point
      const vec_type unit_dir = center_line.UnitDirection();
      const vec_type intersection_point = center_ + radius_ * unit_dir;

      // Choose any reference point perpendicular to the center line (to create
      // the tangent line):
      const vec_type ref1 = intersection_point + (radius_ * vec_type(unit_dir.y(), -unit_dir.x()));
      const vec_type ref2 = intersection_point - (radius_ * vec_type(unit_dir.y(), -unit_dir.x()));

      *tangent1 = Line2d_<_Tp>{ref1, ref2};
    }

    return 1;
  } else {
    // Transverse common tangents divide the line between the two centers in
    // the ratio r1/r2 (similar triangles to the rescue).
    // Compute distance to intersection point:
    const double distance_intersection = distance * radius_ / sum_radii;
    const vec_type intersection = center_ + (distance_intersection * center_line.UnitDirection());

    // Useful visualizations on line sectioning:
    // https://doubleroot.in/lessons/coordinate-geometry-basics/section-formula/

    // Pythagoras
    const double hypotenuse = intersection.Distance(center_);
    const double radius1_sqr = radius_ * radius_;
    // Euclid (German "Kathetensatz"):
    const double q = radius1_sqr / hypotenuse;
    const double p = hypotenuse - q;
    // Euclid (German "Hoehensatz"):
    const double h = std::sqrt(p * q);
    // Trigonometry: tan(alpha) = h/p = a/b
    const double alpha = std::atan2(h, p);

    // Compute points of tangency
    // The tangent is also divided by the ratio r1/r2
    const double tangent_length = std::sqrt(
          (distance * distance) - (sum_radii * sum_radii));
    const double tangent_length1 = std::sqrt(
          (hypotenuse * hypotenuse) - radius1_sqr);
    const double tangent_length2 = tangent_length - tangent_length1;

    const Line2d_<_Tp> hypo(intersection, center_);
    const vec_type hypo_dir = hypo.UnitDirection();

    if (tangent1 || tangent2) {
      vec_type dir = RotateVector(hypo_dir, alpha);
      const vec_type t11 = intersection + (tangent_length1 * dir);
      const vec_type t12 = intersection - (tangent_length2 * dir);
      if (tangent1) {
        *tangent1 = Line2d_<_Tp>{t11, t12};
      }

      dir = RotateVector(hypo_dir, -alpha);
      const vec_type t21 = intersection + (tangent_length1 * dir);
      const vec_type t22 = intersection - (tangent_length2 * dir);
      if (tangent2) {
        *tangent2 = Line2d_<_Tp>{t21, t22};
      }
    }

    return 2;
  }
}


template <typename _Tp>
int Circle_<_Tp>::IntersectionCircleCircle(
    const Circle_<_Tp> &other, vec_type *intersection1,
    vec_type *intersection2) const {
  const double distance = center_.Distance(other.center_);

  // Are the circles the same?
  if (eps_zero(distance) && eps_equal(radius_, other.radius_)) {
    return -1;
  }

  // Too far apart?
  if (distance > (radius_ + other.radius_)) {
    return 0;
  }

  // One circle contained within the other?
  if (distance < std::fabs(radius_ - other.radius_)) {
    return 0;
  }

  const double dist_sqr = distance * distance;
  const double r1_sqr = radius_ * radius_;
  const double r2_sqr = other.radius_ * other.radius_;
  const double r1smr2s = r1_sqr - r2_sqr;
  const double a = r1smr2s / (2.0 * dist_sqr);
  const double c = std::sqrt(
        2.0 * (r1_sqr + r2_sqr) / dist_sqr - (r1smr2s * r1smr2s) / (dist_sqr * dist_sqr) - 1.0);

  const double fx = (center_.x() + other.center_.x()) / 2.0 + a * (other.center_.x() - center_.x());
  const double gx = c * (other.center_.y() - center_.y()) / 2.0;
  const double ix1 = fx + gx;
  const double ix2 = fx - gx;

  const double fy = (center_.y() + other.center_.y()) / 2.0 + a * (other.center_.y() - center_.y());
  const double gy = c * (center_.x() - other.center_.x()) / 2.0;
  const double iy1 = fy + gy;
  const double iy2 = fy - gy;

  if (intersection1) {
    *intersection1 = vec_type{ix1, iy1};
  }

  if (eps_zero(gx) && eps_zero(gy)) {
    return 1;
  } else {
    if (intersection2) {
      *intersection2 = vec_type{ix2, iy2};
    }
    return 2;
  }
}


template <typename _Tp>
int Circle_<_Tp>::IntersectionCircleLine(
    const Line2d_<_Tp>& line,
    vec_type *intersection1,
    vec_type *intersection2) const {
  return line.IntersectionLineCircle(*this, intersection1, intersection2);
}


template <typename _Tp>
int Circle_<_Tp>::IntersectionCircleLineSegment(
    const Line2d_<_Tp>& segment,
    vec_type *intersection1,
    vec_type *intersection2) const {
  return segment.IntersectionLineSegmentCircle(
        *this, intersection1, intersection2);
}


// Explicit instantiation:
template class Circle_<double>;



//----------------------------------------------------
// Line2d

template <typename _Tp>
Line2d_<_Tp> Line2d_<_Tp>::LeftToRight() const {
  if (!IsValid()) {
    return Line2d_<_Tp>();
  }

  if (eps_equal(pt_from_.x(), pt_to_.x())) {
    // A vertical line will be sorted top-to-bottom:
    if (pt_from_.y() < pt_to_.y()) {
      return Line2d_<_Tp>(pt_from_, pt_to_);
    } else {
      return Line2d_<_Tp>(pt_to_, pt_from_);
    }
  } else {
    // A horizontal line will be sorted left-to-right:
    if (pt_from_.x() < pt_to_.x()) {
      return Line2d_<_Tp>(pt_from_, pt_to_);
    } else {
      return Line2d_<_Tp>(pt_to_, pt_from_);
    }
  }
}


template <typename _Tp>
typename Line2d_<_Tp>::vec_type Line2d_<_Tp>::ClosestPointOnLine(
    const vec_type &point) const {
  // Vector from line start to point:
  const vec_type v = pt_from_.DirectionVector(point);
  // Project onto line and get closest point on line:
  const vec_type unit_dir = UnitDirection();
  const _Tp lambda = unit_dir.Dot(v);
  return pt_from_ + lambda * unit_dir;
}


template <typename _Tp>
typename Line2d_<_Tp>::vec_type Line2d_<_Tp>::ClosestPointOnSegment(
    const vec_type &point) const {
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


template <typename _Tp>
bool Line2d_<_Tp>::IsCollinear(const Line2d_ &other) const {
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


template <typename _Tp>
bool Line2d_<_Tp>::IsPointLeftOfLine(
    const vec_type &point, bool *is_on_line) const {
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


template <typename _Tp>
bool Line2d_<_Tp>::IntersectionLineLine(
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


template <typename _Tp>
bool Line2d_<_Tp>::IntersectionLineLineSegment(
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


template <typename _Tp>
bool Line2d_<_Tp>::IntersectionLineSegmentLineSegment(
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


template <typename _Tp>
int Line2d_<_Tp>::IntersectionLineCircle(
    const Circle_<_Tp> &circle, vec_type *intersection1,
    vec_type *intersection2) const {
  // Interesting further read on collision detection via projections:
  // https://stackoverflow.com/a/1084899/400948
  // This implementation is based on:
  // http://mathworld.wolfram.com/Circle-LineIntersection.html
  // which assumes that the circle is centered at (0,0)
  const double x1 = pt_from_.x() - circle.cx();
  const double x2 = pt_to_.x()- circle.cx();
  const double y1 = pt_from_.y() - circle.cy();
  const double y2 = pt_to_.y() - circle.cy();

  const double dx = x2 - x1;
  const double dy = y2 - y1;
  const double dr = std::sqrt((dx * dx) + (dy * dy));
  const double dr_sqr = dr * dr;
  const double D = (x1 * y2) - (x2 * y1);

  const double discriminant = (circle.Radius() * circle.Radius() * dr_sqr) - (D * D);
  const double discriminant_sqrt = std::sqrt(discriminant);
  const double sgn = (dy < 0.0) ? -1.0 : 1.0;
  if (eps_zero(discriminant)) {
    // discriminant == 0: line is a tangent
    if (intersection1) {
      intersection1->SetX(
            ((D * dy) + (sgn * dx * discriminant_sqrt)) / dr_sqr + circle.cx());
      intersection1->SetY(
            ((-D * dx) + (std::fabs(dy) * discriminant_sqrt)) / dr_sqr + circle.cy());
    }
    return 1;
  } else {
    if (discriminant > 0.0) {
      // 2 intersection points
      if (intersection1) {
        intersection1->SetX(
              ((D * dy) + (sgn * dx * discriminant_sqrt)) / dr_sqr + circle.cx());
        intersection1->SetY(
              ((-D * dx) + (std::fabs(dy) * discriminant_sqrt)) / dr_sqr + circle.cy());
      }
      if (intersection2) {
        intersection2->SetX(
              ((D * dy) - (sgn * dx * discriminant_sqrt)) / dr_sqr + circle.cx());
        intersection2->SetY(
              ((-D * dx) - (std::fabs(dy) * discriminant_sqrt)) / dr_sqr + circle.cy());
      }
      return 2;
    } else {
      // No intersection
      return 0;
    }
  }
}


template <typename _Tp>
int Line2d_<_Tp>::IntersectionLineSegmentCircle(
    const Circle_<_Tp> &circle, vec_type *intersection1,
    vec_type *intersection2) const {
  // Compute intersection points with the line(!) and then check if they're
  // on the segment.
  vec_type line_intersect1, line_intersect2;
  const int line_intersections = IntersectionLineCircle(
        circle, &line_intersect1, &line_intersect2);

  if (line_intersections == 0) {
    return 0;
  }

  const double dist1 = DistancePointToSegment(line_intersect1);
  const bool on_segment1 = eps_zero(dist1);

  if (line_intersections == 1) {
    // If there is only a single intersection point with the line, it will be
    // stored in line_intersect1, so we only need to check this.
    if (on_segment1) {
      if (intersection1) {
        *intersection1 = line_intersect1;
      }
      return 1;
    }
    return 0;
  } else {
    // We have two line-circle intersection points and need to check both.
    const double dist2 = DistancePointToSegment(line_intersect2);
    const bool on_segment2 = eps_zero(dist2);

    int num_intersections = 0;
    if (on_segment1) {
      if (intersection1) {
        *intersection1 = line_intersect1;
      }
      ++num_intersections;

      if (on_segment2) {
        if (intersection2) {
          *intersection2 = line_intersect2;
        }
        ++num_intersections;
      }
    } else {
      if (on_segment2) {
        if (intersection1) {
          *intersection1 = line_intersect2;
        }
        ++num_intersections;
      }
    }
    return num_intersections;
  }
}


template <typename _Tp>
Line2d_<_Tp> Line2d_<_Tp>::ClipLineByRectangle(
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


template <typename _Tp>
Line2d_<_Tp> Line2d_<_Tp>::ClipLineSegmentByRectangle(
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


// Explicit instantiation:
template class Line2d_<double>;


//----------------------------------------------------
// Line3d

template <typename _Tp>
typename Line3d_<_Tp>::vec_type Line3d_<_Tp>::ClosestPointOnLine(
    const vec_type &point) const {
  // Vector from line start to point:
  const vec_type v = pt_from_.DirectionVector(point);
  // Project onto line and get closest point on line:
  const vec_type unit_dir = UnitDirection();
  const _Tp lambda = unit_dir.Dot(v);
  return pt_from_ + lambda * unit_dir;
}


template <typename _Tp>
typename Line3d_<_Tp>::vec_type Line3d_<_Tp>::ClosestPointOnSegment(
    const vec_type &point) const {
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


// Explicit instantiation:
template class Line3d_<double>;


//----------------------------------------------------
// Plane
template <typename _Tp>
Plane_<_Tp>::Plane_(
    const vec_type &p, const vec_type &q, const vec_type &r)
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


// Explicit instantiation:
template class Plane_<double>;


} // namespace geometry
} // namespace werkzeugkiste
