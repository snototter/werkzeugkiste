#include <cmath>

#include <werkzeugkiste/geometry/primitives.h>
#include <werkzeugkiste/geometry/utils.h>

namespace werkzeugkiste::geometry
{

//----------------------------------------------------
// Circle
template<typename T>
Circle_<T>::Circle_(const vec_type& p, const vec_type& q, const vec_type& r)
    : Circle_<T>()
{
  // Check if the points are collinear
  const Line2d_<T> segment1(p, q);
  const Line2d_<T> segment2(q, r);

  if (!segment1.IsCollinear(segment2)) {
    // Compute center as the intersection point of the segment bisections:
    // First, we need the bisecting lines
    const vec_type mid1 = segment1.MidPoint();
    const vec_type dir1 = segment1.Direction();
    const vec_type orth1 {dir1.val[1], -dir1.val[0]};
    const Line2d_<T> bisect1 {mid1, mid1 + orth1};
    // ... same for the second one
    const vec_type mid2 = segment2.MidPoint();
    const vec_type dir2 = segment2.Direction();
    const vec_type orth2 {dir2.Y(), -dir2.X()};
    const Line2d_<T> bisect2 {mid2, mid2 + orth2};
    // Intersect them
    const bool exists = bisect1.IntersectionLineLine(bisect2, &center_);
    if (exists) {
      // FIXME warn that intersection of the bisecting lines does not exist
      // (most
      //   likely due to numerical issue)
      radius_ = p.DistanceEuclidean(center_);
    }
  }
}

template<typename T>
bool Circle_<T>::IsPointInCircle(const vec_type& pt, bool* is_on_circle) const
{
  const double distance = pt.DistanceEuclidean(center_);

  bool in_circle {false};

  if (IsEpsEqual(distance, radius_)) {
    if (is_on_circle != nullptr) {
      *is_on_circle = true;
    }
    in_circle = true;
  } else {
    if (is_on_circle != nullptr) {
      *is_on_circle = false;
    }

    in_circle = (distance < radius_);
  }

  return in_circle;
}

template<typename T>
int Circle_<T>::PointsOfTangency(const vec_type& pt,
                                 vec_type* pot1,
                                 vec_type* pot2) const
{
  bool is_on_circle {false};
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
  //    |                   (+)---------------------------------------------(+)
  //    pt |                                        .'    hypotenuse | C |
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
    const double hypotenuse = pt.DistanceEuclidean(center_);
    const double radius_sqr = radius_ * radius_;
    const double distance = std::sqrt((hypotenuse * hypotenuse) - radius_sqr);

    // Euclid (German "Kathetensatz"):
    const double q = radius_sqr / hypotenuse;
    const double p = hypotenuse - q;
    // Euclid (German "Hoehensatz"):
    const double h = std::sqrt(p * q);
    // Trigonometry: tan(alpha) = h/p = a/b
    const double alpha = std::atan2(h, p);

    const Line2d hypo(pt, center_);
    const Vec2d to_rotate = distance * hypo.UnitDirection();

    Vec2d dir = to_rotate.RotateRadians(alpha);
    if (pot1) {
      *pot1 = pt + dir;
    }

    dir = to_rotate.RotateRadians(-alpha);
    if (pot2) {
      *pot2 = pt + dir;
    }
  }

  return 2;
}

template<typename T>
int Circle_<T>::DirectCommonTangents(const Circle_<T>& other,
                                     Line2d_<T>* tangent1,
                                     Line2d_<T>* tangent2) const
{
  // Tangents (transverse and/or direct) depending on distance between
  // the circles:
  // * r1 + r2 < center_dist:  4 tangents (2 direct, 2 transverse)
  // * r1 + r2 == center_dist: 3 tangents (2 direct, 1 transverse)
  // * |r1-r2| < center_dist < r1+r2: 2 tangents (direct, no transverse)
  // * abs(r1-r2) == center_dist: 1 tangent
  // * center_dist < abs(r1-r2): no tangent

  if ((center_ == other.center_) && IsEpsEqual(radius_, other.radius_)) {
    // No tangent if the circles are the same
    return 0;
  }

  const double center_distance = center_.DistanceEuclidean(other.center_);
  if (center_distance < std::fabs(radius_ - other.radius_)) {
    // C1 is inside C2 (or vice versa) and they don't touch.
    return 0;
  }

  const Line2d_<T> center_line(center_, other.center_);
  if (IsEpsEqual(center_distance, std::fabs(radius_ - other.radius_))) {
    // C1 is inside C2 (or vice versa) and they intersect in exactly one point.
    if (tangent1 != nullptr) {
      const vec_type unit_dir = center_line.UnitDirection();
      const vec_type intersection = center_ + (radius_ * unit_dir);
      const vec_type ref1 =
          intersection + (radius_ * vec_type {unit_dir.Y(), -unit_dir.X()});
      const vec_type ref2 =
          intersection - (radius_ * vec_type {unit_dir.Y(), -unit_dir.X()});
      *tangent1 = Line2d_<T> {ref1, ref2};
    }
    return 1;
  }

  // In all other cases, there exist two direct common tangents.
  if ((tangent1 != nullptr) || (tangent2 != nullptr)) {
    if (IsEpsEqual(radius_, other.radius_)) {
      // Tangents are parallel to the center-connecting line
      const vec_type dir = radius_ * center_line.UnitDirection();
      const vec_type orth_dir {dir.Y(), -dir.X()};

      // Intersection points are perpendicular to the center line:
      const vec_type t11 = center_ + orth_dir;
      const vec_type t12 = center_ - orth_dir;
      const vec_type t21 = other.center_ + orth_dir;
      const vec_type t22 = other.center_ - orth_dir;

      if (tangent1 != nullptr) {
        *tangent1 = Line2d_<T> {t11, t21};
      }
      if (tangent2 != nullptr) {
        *tangent2 = Line2d_<T> {t12, t22};
      }
    } else {
      // Similar triangles:
      // (distance c1 to intersection) / (distance c2 to intersection) =
      //   radius1 / radius2
      const double distance_intersection = center_distance
          + (center_distance * other.radius_) / (radius_ - other.radius_);
      const vec_type intersection =
          center_ + (distance_intersection * center_line.UnitDirection());

      // Now compute the points of tangency w.r.t. both circles.
      vec_type t11, t12, t21, t22;  // NOLINT

      // TODO warn if we couldn't find POT (would be a numerical issue)
      //          if (!PointsOfTangency(intersection, &t11, &t12))
      PointsOfTangency(intersection, &t11, &t12);
      other.PointsOfTangency(intersection, &t21, &t22);

      if (tangent1 != nullptr) {
        *tangent1 = Line2d_<T> {t11, t21};
      }
      if (tangent2 != nullptr) {
        *tangent2 = Line2d_<T> {t12, t22};
      }
    }
  }
  return 2;
}

template<typename T>
int Circle_<T>::TransverseCommonTangents(const Circle_<T>& other,
                                         Line2d_<T>* tangent1,
                                         Line2d_<T>* tangent2) const
{
  const double distance = center_.DistanceEuclidean(other.center_);
  const double sum_radii = radius_ + other.radius_;

  if (distance < sum_radii) {
    // No transverse common tangents if there's more than 1 intersection point.
    return 0;
  }

  const Line2d_<T> center_line(center_, other.center_);
  if (IsEpsEqual(distance, sum_radii)) {
    if (tangent1 != nullptr) {
      // Compute the single intersection point
      const vec_type unit_dir = center_line.UnitDirection();
      const vec_type intersection_point = center_ + radius_ * unit_dir;

      // Choose any reference point perpendicular to the center line (to create
      // the tangent line):
      const vec_type ref1 = intersection_point
          + (radius_ * vec_type {unit_dir.Y(), -unit_dir.X()});
      const vec_type ref2 = intersection_point
          - (radius_ * vec_type {unit_dir.Y(), -unit_dir.X()});

      *tangent1 = Line2d_<T> {ref1, ref2};
    }
    return 1;
  }

  // Transverse common tangents divide the line between the two centers in
  // the ratio r1/r2 (so we'll use similar triangles to our rescue).
  // Compute distance to intersection point:
  const double distance_intersection = distance * radius_ / sum_radii;
  const vec_type intersection =
      center_ + (distance_intersection * center_line.UnitDirection());

  // Useful visualizations on line sectioning:
  // https://doubleroot.in/lessons/coordinate-geometry-basics/section-formula/

  // Pythagoras
  const double hypotenuse = intersection.DistanceEuclidean(center_);
  const double radius1_sqr = radius_ * radius_;
  // Euclid part 1 (German "Kathetensatz"):
  const double q = radius1_sqr / hypotenuse;
  const double p = hypotenuse - q;
  // Euclid part 2 (German "Hoehensatz"):
  const double h = std::sqrt(p * q);
  // Trigonometry: tan(alpha) = h/p = a/b
  const double alpha = std::atan2(h, p);

  // Compute points of tangency
  // The tangent is also divided by the ratio r1/r2
  const double tangent_length =
      std::sqrt((distance * distance) - (sum_radii * sum_radii));
  const double tangent_length1 =
      std::sqrt((hypotenuse * hypotenuse) - radius1_sqr);
  const double tangent_length2 = tangent_length - tangent_length1;

  const Line2d_<T> hypo(intersection, center_);
  const vec_type hypo_dir = hypo.UnitDirection();

  if ((tangent1 != nullptr) || (tangent2 != nullptr)) {
    vec_type dir = hypo_dir.RotateRadians(alpha);
    const vec_type t11 = intersection + (tangent_length1 * dir);
    const vec_type t12 = intersection - (tangent_length2 * dir);
    if (tangent1) {
      *tangent1 = Line2d_<T> {t11, t12};
    }

    dir = hypo_dir.RotateRadians(-alpha);
    const vec_type t21 = intersection + (tangent_length1 * dir);
    const vec_type t22 = intersection - (tangent_length2 * dir);
    if (tangent2) {
      *tangent2 = Line2d_<T> {t21, t22};
    }
  }
  return 2;
}

template<typename T>
int Circle_<T>::IntersectionCircleCircle(const Circle_<T>& other,
                                         vec_type* intersection1,
                                         vec_type* intersection2) const
{
  const double distance = center_.DistanceEuclidean(other.center_);

  // Are the circles the same?
  if (IsEpsZero(distance) && IsEpsEqual(radius_, other.radius_)) {
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
  const double c =
      std::sqrt(2.0 * (r1_sqr + r2_sqr) / dist_sqr
                - (r1smr2s * r1smr2s) / (dist_sqr * dist_sqr) - 1.0);

  const double fx = (center_.X() + other.center_.X()) / 2.0
      + a * (other.center_.X() - center_.X());
  const double gx = c * (other.center_.Y() - center_.Y()) / 2.0;
  const double ix1 = fx + gx;
  const double ix2 = fx - gx;

  const double fy = (center_.Y() + other.center_.Y()) / 2.0
      + a * (other.center_.Y() - center_.Y());
  const double gy = c * (center_.X() - other.center_.X()) / 2.0;
  const double iy1 = fy + gy;
  const double iy2 = fy - gy;

  if (intersection1 != nullptr) {
    *intersection1 = vec_type {ix1, iy1};
  }

  if (IsEpsZero(gx) && IsEpsZero(gy)) {
    return 1;
  }

  if (intersection2 != nullptr) {
    *intersection2 = vec_type {ix2, iy2};
  }
  return 2;
}

template<typename T>
int Circle_<T>::IntersectionCircleLine(const Line2d_<T>& line,
                                       vec_type* intersection1,
                                       vec_type* intersection2) const
{
  return line.IntersectionLineCircle(*this, intersection1, intersection2);
}

template<typename T>
int Circle_<T>::IntersectionCircleLineSegment(const Line2d_<T>& segment,
                                              vec_type* intersection1,
                                              vec_type* intersection2) const
{
  return segment.IntersectionLineSegmentCircle(
      *this, intersection1, intersection2);
}

// Explicit instantiation:
template class Circle_<double>;

//----------------------------------------------------
// Line2d

template<typename T>
Line2d_<T> Line2d_<T>::LeftToRight() const
{
  if (!IsValid()) {
    return Line2d_<T> {};
  }

  // NOLINTBEGIN(llvm-else-after-return)

  if (IsEpsEqual(pt_from_.X(), pt_to_.X())) {
    // A vertical line will be sorted top-to-bottom:
    if (pt_from_.Y() < pt_to_.Y()) {
      return Line2d_<T> {pt_from_, pt_to_};
    } else {
      return Line2d_<T> {pt_to_, pt_from_};
    }
  } else {
    // A horizontal line will be sorted left-to-right:
    if (pt_from_.X() < pt_to_.X()) {
      return Line2d_<T> {pt_from_, pt_to_};
    } else {
      return Line2d_<T> {pt_to_, pt_from_};
    }
  }

  // NOLINTEND(llvm-else-after-return)
}

template<typename T>
typename Line2d_<T>::vec_type Line2d_<T>::ClosestPointOnLine(
    const vec_type& point) const
{
  // Vector from line start to point:
  const vec_type v = pt_from_.DirectionVector(point);
  // Project onto line and get closest point on line:
  const vec_type unit_dir = UnitDirection();
  const T lambda = unit_dir.Dot(v);
  return pt_from_ + lambda * unit_dir;
}

template<typename T>
typename Line2d_<T>::vec_type Line2d_<T>::ClosestPointOnSegment(
    const vec_type& point) const
{
  // Vector from segment start to point:
  const vec_type v = pt_from_.DirectionVector(point);
  // Project v onto segment:
  const vec_type direction = Direction();
  const vec_type unit_direction = direction.UnitVector();
  const double lambda = std::min(
      Length(), std::max(0.0, static_cast<double>(unit_direction.Dot(v))));

  return pt_from_ + lambda * unit_direction;
}

template<typename T>
bool Line2d_<T>::IsCollinear(const Line2d_& other) const
{
  // Line 1 goes from p to p + r
  const vec_type p = pt_from_;
  const vec_type r = Direction();
  // Line 2 goes from q to q + s
  const vec_type q = other.pt_from_;
  const vec_type s = other.Direction();

  const T rxs = Determinant(r, s);
  const T qmpxr = Determinant((q - p), r);

  return (IsEpsZero(rxs) && IsEpsZero(qmpxr));
}

template<typename T>
bool Line2d_<T>::IsPointLeftOfLine(const vec_type& point,
                                   bool* is_on_line) const
{
  const T det = Determinant(Direction(), point - pt_to_);

  // If the "2d cross product" (i.e. determinant) is 0, the points are
  // collinear, and thus, would be on the line.
  if (IsEpsZero(det)) {
    if (is_on_line != nullptr) {
      *is_on_line = true;
    }
    return true;
  }

  if (is_on_line != nullptr) {
    *is_on_line = false;
  }
  return Sign(det) > 0;
}

template<typename T>
bool Line2d_<T>::IntersectionLineLine(const Line2d_& other,
                                      vec_type* intersection_point) const
{
  const vec3_type ip = HomogeneousForm().Cross(other.HomogeneousForm());
  if (IsEpsZero(ip[2])) {
    // Intersection point is at infinity
    return false;
  }

  if (intersection_point != nullptr) {
    intersection_point->SetX(ip[0] / ip[2]);
    intersection_point->SetY(ip[1] / ip[2]);
  }
  return true;
}

template<typename T>
bool Line2d_<T>::IntersectionLineLineSegment(const Line2d_& segment,
                                             vec_type* intersection_point) const
{
  // Line 1 passes through p and (p + r)
  const vec_type p = pt_from_;
  const vec_type r = Direction();

  // Segment 2 goes from q to (q + s)
  const vec_type q = segment.pt_from_;
  const vec_type s = segment.Direction();

  const T rxs = Determinant(r, s);
  const T qmpxr = Determinant((q - p), r);

  if (IsEpsZero(rxs) && IsEpsZero(qmpxr)) {
    // Line and segment are collinear, pick any point on the line.
    if (intersection_point != nullptr) {
      *intersection_point = segment.From();
    }
    return true;
  }

  if (IsEpsZero(rxs)) {
    // Line and segment are parallel and not intersecting.
    return false;
  }

  // Otherwise, they intersect if the intersection point lies on the
  // segment, i.e. u in [0,1].
  const double u = static_cast<double>(qmpxr) / static_cast<double>(rxs);
  if ((u >= 0.0) && (u <= 1.0)) {
    if (intersection_point != nullptr) {
      *intersection_point = q + u * s;
    }
    return true;
  }
  return false;
}

template<typename T>
bool Line2d_<T>::IntersectionLineSegmentLineSegment(
    const Line2d_& segment, vec_type* intersection_point) const
{
  // Based on https://stackoverflow.com/a/565282/400948
  // Line 1 goes from p to p + r
  const vec_type p = pt_from_;
  const vec_type r = Direction();
  // Segment 2 goes from q to q + s
  const vec_type q = segment.pt_from_;
  const vec_type s = segment.Direction();

  const T rxs = Determinant(r, s);
  const T qmpxr = Determinant((q - p), r);

  if (IsEpsZero(rxs) && IsEpsZero(qmpxr)) {
    // Segments are collinear. They intersect if there is any overlap.
    const double rdr = static_cast<double>(r.Dot(r));
    const double t0 = static_cast<double>((q - p).Dot(r)) / rdr;

    if ((t0 >= 0.0) && (t0 <= 1.0)) {
      if (intersection_point != nullptr) {
        *intersection_point = p + t0 * r;
      }
      return true;
    }

    const double t1 = t0 + static_cast<double>(s.Dot(r)) / rdr;
    if ((t1 >= 0.0) && (t1 <= 1.0)) {
      if (intersection_point) {
        *intersection_point = p + t1 * r;
      }
      return true;
    }

    // Otherwise, these collinear segments don't intersect
    return false;
  }

  if (IsEpsZero(rxs)) {
    // Segments are parallel and not intersecting
    return false;
  }

  // Otherwise, the segments meet if u in [0,1] and t in [0,1].
  const double u = static_cast<double>(qmpxr) / static_cast<double>(rxs);
  const double t =
      Determinant((q - p), s) / static_cast<double>(Determinant(r, s));

  if ((u >= 0.0) && (u <= 1.0) && (t >= 0.0) && (t <= 1.0)) {
    if (intersection_point != nullptr) {
      *intersection_point = p + t * r;
    }
    return true;
  }

  return false;
}

template<typename T>
int Line2d_<T>::IntersectionLineCircle(const Circle_<T>& circle,
                                       vec_type* intersection1,
                                       vec_type* intersection2) const
{
  // Interesting further read on collision detection via projections:
  // https://stackoverflow.com/a/1084899/400948
  // This implementation is based on:
  // http://mathworld.wolfram.com/Circle-LineIntersection.html
  // which assumes that the circle is centered at (0,0)
  const double x1 = pt_from_.X() - circle.CenterX();
  const double x2 = pt_to_.X() - circle.CenterX();
  const double y1 = pt_from_.Y() - circle.CenterY();
  const double y2 = pt_to_.Y() - circle.CenterY();

  const double dx = x2 - x1;
  const double dy = y2 - y1;
  const double dr = std::sqrt((dx * dx) + (dy * dy));
  const double dr_sqr = dr * dr;
  const double D = (x1 * y2) - (x2 * y1);  // NOLINT

  const double discriminant =
      (circle.Radius() * circle.Radius() * dr_sqr) - (D * D);
  const double discriminant_sqrt = std::sqrt(discriminant);
  const double sgn = (dy < 0.0) ? -1.0 : 1.0;

  int num_poi {0};

  if (IsEpsZero(discriminant)) {
    // discriminant == 0: line is a tangent
    if (intersection1 != nullptr) {
      intersection1->SetX(((D * dy) + (sgn * dx * discriminant_sqrt)) / dr_sqr
                          + circle.CenterX());
      intersection1->SetY(((-D * dx) + (std::fabs(dy) * discriminant_sqrt))
                              / dr_sqr
                          + circle.CenterY());
    }
    num_poi = 1;
  } else {
    if (Sign(discriminant) > 0) {
      // 2 intersection points
      if (intersection1 != nullptr) {
        intersection1->SetX(((D * dy) + (sgn * dx * discriminant_sqrt)) / dr_sqr
                            + circle.CenterX());
        intersection1->SetY(((-D * dx) + (std::fabs(dy) * discriminant_sqrt))
                                / dr_sqr
                            + circle.CenterY());
      }
      if (intersection2 != nullptr) {
        intersection2->SetX(((D * dy) - (sgn * dx * discriminant_sqrt)) / dr_sqr
                            + circle.CenterX());
        intersection2->SetY(((-D * dx) - (std::fabs(dy) * discriminant_sqrt))
                                / dr_sqr
                            + circle.CenterY());
      }
      num_poi = 2;
    } else {
      // No intersection
      num_poi = 0;
    }
  }
  return num_poi;
}

template<typename T>
int Line2d_<T>::IntersectionLineSegmentCircle(const Circle_<T>& circle,
                                              vec_type* intersection1,
                                              vec_type* intersection2) const
{
  // Compute intersection points with the line(!) and then check if they're
  // on the segment.
  vec_type line_intersect1;
  vec_type line_intersect2;
  const int line_intersections =
      IntersectionLineCircle(circle, &line_intersect1, &line_intersect2);

  if (line_intersections == 0) {
    return 0;
  }

  const double dist1 = DistancePointToSegment(line_intersect1);
  const bool on_segment1 = IsEpsZero(dist1);

  if (line_intersections == 1) {
    // If there is only a single intersection point with the line, it will be
    // stored in line_intersect1, so we only need to check this.
    if (on_segment1) {
      if (intersection1 != nullptr) {
        *intersection1 = line_intersect1;
      }
      return 1;
    }
    // The single intersection point is outside the segment.
    return 0;
  }

  // We have two line-circle intersection points and need to check both.
  const double dist2 = DistancePointToSegment(line_intersect2);
  const bool on_segment2 = IsEpsZero(dist2);

  int num_intersections = 0;
  if (on_segment1) {
    if (intersection1 != nullptr) {
      *intersection1 = line_intersect1;
    }
    ++num_intersections;

    if (on_segment2) {
      if (intersection2 != nullptr) {
        *intersection2 = line_intersect2;
      }
      ++num_intersections;
    }
  } else {
    if (on_segment2) {
      // Only the second intersection point lies on the segment. Thus, we need
      // to assign it to first(!) output parameter.
      if (intersection1 != nullptr) {
        *intersection1 = line_intersect2;
      }
      ++num_intersections;
    }
  }
  return num_intersections;
}

template<typename T>
Line2d_<T> Line2d_<T>::ClipLineByRectangle(const vec_type& top_left,
                                           const vec_type& size) const
{
  const vec_type top_right {top_left.X() + size.Width(), top_left.Y()};
  const vec_type bottom_right {top_right.X(), top_left.Y() + size.Height()};
  const vec_type bottom_left {top_left.X(), bottom_right.Y()};

  const std::vector<Line2d_> edges {Line2d_<T> {top_left, top_right},
                                    Line2d_<T> {top_right, bottom_right},
                                    Line2d_<T> {bottom_right, bottom_left},
                                    Line2d_<T> {bottom_left, top_left}};

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
    return Line2d_<T> {};
  }

  return Line2d_<T> {int_points[0], int_points[1]};
}

template<typename T>
Line2d_<T> Line2d_<T>::ClipLineSegmentByRectangle(const vec_type& top_left,
                                                  const vec_type& size) const
{
  const bool is_from_inside = IsPointInsideRectangle(pt_from_, top_left, size);
  const bool is_to_inside = IsPointInsideRectangle(pt_to_, top_left, size);

  if (is_from_inside && is_to_inside) {
    return *this;
  }

  const vec_type top_right {top_left.X() + size.Width(), top_left.Y()};
  const vec_type bottom_right {top_right.X(), top_left.Y() + size.Height()};
  const vec_type bottom_left {top_left.X(), bottom_right.Y()};

  const std::vector<Line2d_> edges {Line2d_<T> {top_left, top_right},
                                    Line2d_<T> {top_right, bottom_right},
                                    Line2d_<T> {bottom_right, bottom_left},
                                    Line2d_<T> {bottom_left, top_left}};

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
    return Line2d_<T> {};
  }

  if (is_from_inside != is_to_inside) {
    // One in, one out: There should be only 1 intersection point - unless
    // the intersection falls exactly on a corner, then there may be two (but
    // here we are only interested in clipping the segment).

    if (is_from_inside) {
      return Line2d_<T> {pt_from_, int_points[0]};
    }

    return Line2d_<T> {int_points[0], pt_to_};
  }

  return Line2d_<T> {int_points[0], int_points[1]};
}

// Explicit instantiation:
template class Line2d_<double>;

//----------------------------------------------------
// Line3d

template<typename T>
typename Line3d_<T>::vec_type Line3d_<T>::ClosestPointOnLine(
    const vec_type& point) const
{
  // Vector from line start to point:
  const vec_type v = pt_from_.DirectionVector(point);
  // Project onto line and get closest point on line:
  const vec_type unit_dir = UnitDirection();
  const T lambda = unit_dir.Dot(v);
  return pt_from_ + lambda * unit_dir;
}

template<typename T>
typename Line3d_<T>::vec_type Line3d_<T>::ClosestPointOnSegment(
    const vec_type& point) const
{
  // Vector from segment start to point:
  const vec_type v = pt_from_.DirectionVector(point);
  // Project v onto segment:
  const vec_type direction = Direction();
  const vec_type unit_direction = direction.UnitVector();
  const double lambda = std::min(
      Length(), std::max(0.0, static_cast<double>(unit_direction.Dot(v))));

  return pt_from_ + lambda * unit_direction;
}

// Explicit instantiation:
template class Line3d_<double>;

//----------------------------------------------------
// Plane
template<typename T>
Plane_<T>::Plane_(const vec_type& p, const vec_type& q, const vec_type& r)
    : Plane_()
{
  const vec_type pq = p.DirectionVector(q);
  const vec_type pr = q.DirectionVector(r);
  const vec_type cross = pq.Cross(pr);

  if (!IsEpsZero(cross.LengthSquared())) {
    normal_ = cross.UnitVector();
    offset_ = -normal_.Dot(p);
  }
  // TODO: implement logging and warn about 3 collinear points.
}

// Explicit instantiation:
template class Plane_<double>;

}  // namespace werkzeugkiste::geometry
