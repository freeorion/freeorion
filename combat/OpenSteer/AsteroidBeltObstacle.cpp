#include "AsteroidBeltObstacle.h"


namespace {
    const float SQUARE_APPROX_FACTOR = 0.83486238532110091;
}

AsteroidBeltObstacle::AsteroidBeltObstacle(float r, float tr) :
    m_radius(r),
    m_tube_radius(tr)
{}

AsteroidBeltObstacle::AsteroidBeltObstacle() :
    m_radius(1),
    m_tube_radius(0.1)
{}

void AsteroidBeltObstacle::findIntersectionWithVehiclePath(
    const OpenSteer::AbstractVehicle& vehicle,
    PathIntersection& pi) const
{
    assert(seenFrom() == outside);

    pi = PathIntersection();

    OpenSteer::Vec3 direction = vehicle.forward().normalize();
    OpenSteer::Vec3 position = vehicle.position();
    if (direction.y && direction.x) {
        // HACK: Since a true solution to a line-torus intersection is
        // incredibly hairy (requiring the solution to a quartic), we
        // instead do intersection of a line with two cylinder
        // sections.  The volume between the cylinder sections is
        // approximately the same as the volume of the torus in which
        // the asteroid belt lies.  The radius of the torus tube can't
        // be used though, without the corners of the
        // cylinders-approxmiation sticking out too far.  Our solution
        // here is to make the radius smaller by the
        // SQUARE_APPROX_FACTOR value above, which was empirically
        // chosen to be a reasonable fit.

        /* Parametric form of the equation of a line in the x-y
           plane is:
           x = x0 + u * (x1 - x0)
           y = y0 + u * (y1 - y0)

           And the analytic equation for a cylinder segment in the
           x-y plane is:
           (x - x2)^2 + (y - y2)^2 = r^2

           Substituting the line equations into the circle
           equation and solving for u yields a quadratic equation
           a * u^2 + b * u + c = 0, where:
           a = (x1 - x0)^2 + (y1 - y0)^2
           b = 2 * ((x1 - x0) * (x0 - x2) + (y1 - y0) * (y0 - y2))
           c = x2^2 + y2^2 + x0^2 + y0^2 - 2 * (x2 * x0 + y2 * y0) - r^2

           Note that since our direction vector is a unit vector,
           a = 1.  Also, since the asteroid torus is centered at
           the origin, b = 2 * (x0 + y0) and c = x0^2 + y0^2 - r^2.

           The solutions to the equation are:
           (-b +/- std::sqrt(b * b - 4 * a * c)) / (2 * a)

           Note that given the particulars above, this becomes:
           (-b +/- std::sqrt(b * b - 4 * c)) / 2

           If the b * b - 4 * c expression is:
           < 0, then there is no intersection;
           == 0, then there is one (tangent) intersection at -b / 2; or
           > 0, then there are two intersections
        */

        const float OUTER_CYLINDER_RADIUS =
            m_radius + m_tube_radius * SQUARE_APPROX_FACTOR;
        const float INNER_CYLINDER_RADIUS =
            m_radius - m_tube_radius * SQUARE_APPROX_FACTOR;

        std::set<float> solutions;
        TestCylinderSides(position, direction, OUTER_CYLINDER_RADIUS, solutions);
        TestCylinderSides(position, direction, INNER_CYLINDER_RADIUS, solutions);
        TestBetweenCylinders(position, direction,
                             INNER_CYLINDER_RADIUS, OUTER_CYLINDER_RADIUS,
                             m_tube_radius * SQUARE_APPROX_FACTOR, solutions);
        TestBetweenCylinders(position, direction,
                             INNER_CYLINDER_RADIUS, OUTER_CYLINDER_RADIUS,
                             -m_tube_radius * SQUARE_APPROX_FACTOR, solutions);

        // Find first nonnegative (i.e. front) solution.
        std::set<float>::iterator it = solutions.begin();
        while (it != solutions.end() && *it < 0.0) {
            ++it;
        }

        // An intersection only exists if at least one intersection is
        // in front of us.
        if (it != solutions.end()) {
            pi.intersect = true;
            pi.obstacle = this;
            // If there are one or more negative intersections in
            // addition to the nonnegative one we found, we're inside,
            // so set the distance to 0.
            pi.distance = it != solutions.begin() ? 0.0 : *it;
            pi.surfacePoint = position + direction * *it;
            float theta = std::atan2(pi.surfacePoint.y, pi.surfacePoint.x);
            OpenSteer::Vec3 tube_center_at_theta(
                m_radius * std::cos(theta),
                m_radius * std::sin(theta),
                0.0);
            pi.surfaceNormal =
                (pi.surfacePoint - tube_center_at_theta).normalize();
            pi.steerHint = pi.surfaceNormal;
        }
    }
}

void AsteroidBeltObstacle::TestCylinderSides(
    const OpenSteer::Vec3& position, const OpenSteer::Vec3& direction,
    float cylinder_radius, std::set<float>& solutions) const
{
    //const float a = 1.0;
    const float b = 2.0 * (position.x + position.y);
    const float c =
        position.x * position.x + position.y * position.y -
        cylinder_radius * cylinder_radius;
    const float d = b * b - 4 * c;
    if (!d) {
        InsertSolution(position, direction, -b / 2.0, solutions);
    } else if (0.0 < d) {
        float root = std::sqrt(d);
        InsertSolution(position, direction, (-b + root) / 2.0, solutions);
        InsertSolution(position, direction, (-b - root) / 2.0, solutions);
    }
}

void AsteroidBeltObstacle::TestBetweenCylinders(
    const OpenSteer::Vec3& position, const OpenSteer::Vec3& direction,
    float inner_cylinder_radius, float outer_cylinder_radius,
    float z, std::set<float>& solutions) const
{
    /* We intersect the vehicle path ray with the plane parallel to the X-Y
       plane, at Z=z.  The solution is to find the value of the parameter t in
       t = (N * (p2 - p0)) / (N * (p1 - p0)), where:
       N is the normal of the plane, and p2 is a point in the plane, and the
       line is through p0 and p1.
       In our case, p2 == Vec3(0, 0, z), and (p1 - p0) == direction.  If the
       distance from p2 to the point of intersection is between
       innerer_cylinder_radius and outer_cylinder_radius, we have a hit.
    */
    const OpenSteer::Vec3 NORMAL(0, 0, 1);
    const OpenSteer::Vec3 PT_IN_PLANE(0, 0, z);
    float denominator = NORMAL.dot(direction);
    if (0.0 < denominator) {
        float t = NORMAL.dot(PT_IN_PLANE - position) / denominator;
        OpenSteer::Vec3 intersection = position + direction * t;
        float intersection_radius_squared = (intersection - PT_IN_PLANE).lengthSquared();
        if (inner_cylinder_radius * inner_cylinder_radius < intersection_radius_squared &&
            intersection_radius_squared < outer_cylinder_radius * outer_cylinder_radius) {
            InsertSolution(position, direction, t, solutions);
        }
    }
}

void AsteroidBeltObstacle::InsertSolution(
    const OpenSteer::Vec3& position, const OpenSteer::Vec3& direction,
    float solution, std::set<float>& solutions) const
{
    const float CYLINDER_Z_RADIUS = m_tube_radius * SQUARE_APPROX_FACTOR;
    float solution_z = (position + direction * solution).z;
    if (-CYLINDER_Z_RADIUS < solution_z && solution_z < CYLINDER_Z_RADIUS)
        solutions.insert(solution);
}
