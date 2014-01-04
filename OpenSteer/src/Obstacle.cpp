// ----------------------------------------------------------------------------
//
//
// OpenSteer -- Steering Behaviors for Autonomous Characters
//
// Copyright (c) 2002-2004, Sony Computer Entertainment America
// Original author: Craig Reynolds <craig_reynolds@playstation.sony.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
//
// ----------------------------------------------------------------------------
//
//
// OpenSteer Obstacle classes
// 
// 10-28-04 cwr: split off from Obstacle.h 
//
//
// ----------------------------------------------------------------------------


#include "OpenSteer/Obstacle.h"


// ----------------------------------------------------------------------------
// Obstacle
// compute steering for a vehicle to avoid this obstacle, if needed 


OpenSteer::Vec3 
OpenSteer::Obstacle::steerToAvoid (const AbstractVehicle& vehicle,
                                   const float minTimeToCollision) const
{
    // find nearest intersection with this obstacle along vehicle's path
    PathIntersection pi;
    findIntersectionWithVehiclePath (vehicle, pi);

    // return steering for vehicle to avoid intersection, or zero if non found
    return pi.steerToAvoidIfNeeded (vehicle, minTimeToCollision);
}


// ----------------------------------------------------------------------------
// Obstacle
// static method to apply steerToAvoid to nearest obstacle in an ObstacleGroup


OpenSteer::Vec3
OpenSteer::Obstacle::
steerToAvoidObstacles (const AbstractVehicle& vehicle,
                       const float minTimeToCollision,
                       const ObstacleGroup& obstacles)
{
    PathIntersection nearest, next;

    // test all obstacles in group for an intersection with the vehicle's
    // future path, select the one whose point of intersection is nearest
    firstPathIntersectionWithObstacleGroup (vehicle, obstacles, nearest, next);

    // if nearby intersection found, steer away from it, otherwise no steering
    return nearest.steerToAvoidIfNeeded (vehicle, minTimeToCollision);
}


// ----------------------------------------------------------------------------
// Obstacle
// static method to find first vehicle path intersection in an ObstacleGroup
//
// returns its results in the PathIntersection argument "nearest",
// "next" is used to store internal state.


void
OpenSteer::Obstacle::
firstPathIntersectionWithObstacleGroup (const AbstractVehicle& vehicle,
                                        const ObstacleGroup& obstacles,
                                        PathIntersection& nearest,
                                        PathIntersection& next)
{
    // test all obstacles in group for an intersection with the vehicle's
    // future path, select the one whose point of intersection is nearest
    next.intersect = false;
    nearest.intersect = false;
    for (ObstacleIterator o = obstacles.begin(); o != obstacles.end(); ++o)
    {
        // find nearest point (if any) where vehicle path intersects obstacle
        // o, storing the results in PathIntersection object "next"
        (**o).findIntersectionWithVehiclePath (vehicle, next);

        // if this is the first intersection found, or it is the nearest found
        // so far, store it in PathIntersection object "nearest"
        const bool firstFound = !nearest.intersect;
        const bool nearestFound = (next.intersect &&
                                   (next.distance < nearest.distance));
        if (firstFound || nearestFound) nearest = next;
    }
}


// ----------------------------------------------------------------------------
// PathIntersection
// determine steering once path intersections have been found


OpenSteer::Vec3 
OpenSteer::Obstacle::PathIntersection::
steerToAvoidIfNeeded (const AbstractVehicle& vehicle,
                      const float minTimeToCollision) const
{
    // if nearby intersection found, steer away from it, otherwise no steering
    const float minDistanceToCollision = minTimeToCollision * vehicle.speed();
    if (intersect && (distance < minDistanceToCollision))
    {
        // compute avoidance steering force: take the component of
        // steerHint which is lateral (perpendicular to vehicle's
        // forward direction), set its length to vehicle's maxForce
        Vec3 lateral = steerHint.perpendicularComponent (vehicle.forward ());
        if (lateral == Vec3::zero)
            lateral = vehicle.side ();
        return lateral.normalize () * vehicle.maxForce ();
    }
    else
    {
        return Vec3::zero;
    }
}


// ----------------------------------------------------------------------------
// SphereObstacle
// find first intersection of a vehicle's path with this obstacle


void 
OpenSteer::
SphereObstacle::
findIntersectionWithVehiclePath (const AbstractVehicle& vehicle,
                                 PathIntersection& pi) const
{
    // This routine is based on the Paul Bourke's derivation in:
    //   Intersection of a Line and a Sphere (or circle)
    //   http://www.swin.edu.au/astronomy/pbourke/geometry/sphereline/
    // But the computation is done in the vehicle's local space, so
    // the line in question is the Z (Forward) axis of the space which
    // simplifies some of the calculations.

    float b, c, d, p, q, s;
    Vec3 lc;

    // initialize pathIntersection object to "no intersection found"
    pi.intersect = false;

    // find sphere's "local center" (lc) in the vehicle's coordinate space
    lc = vehicle.localizePosition (center);
    pi.vehicleOutside = lc.length () > radius;

	// if obstacle is seen from inside, but vehicle is outside, must avoid
	// (noticed once a vehicle got outside it ignored the obstacle 2008-5-20)
	if (pi.vehicleOutside && (seenFrom () == inside))
	{
		pi.intersect = true;
		pi.distance = 0.0f;
		pi.steerHint = (center - vehicle.position()).normalize();
		return;
	}
	
    // compute line-sphere intersection parameters
    const float r = radius + vehicle.radius();
    b = -2 * lc.z;
    c = square (lc.x) + square (lc.y) + square (lc.z) - square (r);
    d = (b * b) - (4 * c);

    // when the path does not intersect the sphere
    if (d < 0) return;

    // otherwise, the path intersects the sphere in two points with
    // parametric coordinates of "p" and "q".  (If "d" is zero the two
    // points are coincident, the path is tangent)
    s = sqrtXXX (d);
    p = (-b + s) / 2;
    q = (-b - s) / 2;

    // both intersections are behind us, so no potential collisions
    if ((p < 0) && (q < 0)) return; 

    // at least one intersection is in front, so intersects our forward
    // path
    pi.intersect = true;
    pi.obstacle = this;
    pi.distance =
        ((p > 0) && (q > 0)) ?
        // both intersections are in front of us, find nearest one
        ((p < q) ? p : q) :
        // otherwise one is ahead and one is behind: we are INSIDE obstacle
        (seenFrom () == outside ?
         // inside a solid obstacle, so distance to obstacle is zero
         0.0f :
         // hollow obstacle (or "both"), pick point that is in front
         ((p > 0) ? p : q));
    pi.surfacePoint =
        vehicle.position() + (vehicle.forward() * pi.distance);
    pi.surfaceNormal = (pi.surfacePoint-center).normalize();
    switch (seenFrom ())
    {
    case outside:
        pi.steerHint = pi.surfaceNormal;
        break;
    case inside:
        pi.steerHint = -pi.surfaceNormal;
        break;
    case both:
        pi.steerHint = pi.surfaceNormal * (pi.vehicleOutside ? 1.0f : -1.0f);
        break;
    }
}


// ----------------------------------------------------------------------------
// BoxObstacle
// find first intersection of a vehicle's path with this obstacle


void 
OpenSteer::
BoxObstacle::
findIntersectionWithVehiclePath (const AbstractVehicle& vehicle,
                                 PathIntersection& pi) const
{
    // abbreviations
    const float w = width; // dimensions
    const float h = height;
    const float d = depth;
    const Vec3 s = side (); // local space
    const Vec3 u = up ();
    const Vec3 f = forward ();
    const Vec3 p = position ();
    const Vec3 hw = s * (0.5f * width); // offsets for face centers
    const Vec3 hh = u * (0.5f * height);
    const Vec3 hd = f * (0.5f * depth);
    const seenFromState sf = seenFrom ();

    // the box's six rectangular faces
    RectangleObstacle r1 (w, h,  s,  u,  f, p + hd, sf); // front
    RectangleObstacle r2 (w, h, -s,  u, -f, p - hd, sf); // back
    RectangleObstacle r3 (d, h, -f,  u,  s, p + hw, sf); // side
    RectangleObstacle r4 (d, h,  f,  u, -s, p - hw, sf); // other side
    RectangleObstacle r5 (w, d,  s, -f,  u, p + hh, sf); // top
    RectangleObstacle r6 (w, d, -s, -f, -u, p - hh, sf); // bottom

    // group the six RectangleObstacle faces together
    ObstacleGroup faces;
    faces.push_back (&r1);
    faces.push_back (&r2);
    faces.push_back (&r3);
    faces.push_back (&r4);
    faces.push_back (&r5);
    faces.push_back (&r6);

    // find first intersection of vehicle path with group of six faces
    PathIntersection next;
    firstPathIntersectionWithObstacleGroup (vehicle, faces, pi, next);

    // when intersection found, adjust PathIntersection for the box case
    if (pi.intersect)
    {
        pi.obstacle = this;
        pi.steerHint = ((pi.surfacePoint - position ()).normalize () *
                        (pi.vehicleOutside ? 1.0f : -1.0f));
    }
}


// ----------------------------------------------------------------------------
// PlaneObstacle
// find first intersection of a vehicle's path with this obstacle


void 
OpenSteer::
PlaneObstacle::
findIntersectionWithVehiclePath (const AbstractVehicle& vehicle,
                                 PathIntersection& pi) const
{
    // initialize pathIntersection object to "no intersection found"
    pi.intersect = false;

    const Vec3 lp =  localizePosition (vehicle.position ());
    const Vec3 ld = localizeDirection (vehicle.forward ());

    // no obstacle intersection if path is parallel to XY (side/up) plane
    if (ld.dot (Vec3::forward) == 0.0f) return;

    // no obstacle intersection if vehicle is heading away from the XY plane
    if ((lp.z > 0.0f) && (ld.z > 0.0f)) return;
    if ((lp.z < 0.0f) && (ld.z < 0.0f)) return;

    // no obstacle intersection if obstacle "not seen" from vehicle's side
    if ((seenFrom () == outside) && (lp.z < 0.0f)) return;
    if ((seenFrom () == inside)  && (lp.z > 0.0f)) return;

    // find intersection of path with rectangle's plane (XY plane)
    const float ix = lp.x - (ld.x * lp.z / ld.z);
    const float iy = lp.y - (ld.y * lp.z / ld.z);
    const Vec3 planeIntersection (ix, iy, 0.0f);

    // no obstacle intersection if plane intersection is outside 2d shape
    if (!xyPointInsideShape (planeIntersection, vehicle.radius ())) return;

    // otherwise, the vehicle path DOES intersect this rectangle
    const Vec3 localXYradial = planeIntersection.normalize ();
    const Vec3 radial = globalizeDirection (localXYradial);
    const float sideSign = (lp.z > 0.0f) ? +1.0f : -1.0f;
    const Vec3 opposingNormal = forward () * sideSign;
    pi.intersect = true;
    pi.obstacle = this;
    pi.distance = (lp - planeIntersection).length ();
    pi.steerHint = opposingNormal + radial; // should have "toward edge" term?
    pi.surfacePoint = globalizePosition (planeIntersection);
    pi.surfaceNormal = opposingNormal;
    pi.vehicleOutside = lp.z > 0.0f;
}


// ----------------------------------------------------------------------------
// RectangleObstacle
// determines if a given point on XY plane is inside obstacle shape


bool 
OpenSteer::
RectangleObstacle::
xyPointInsideShape (const Vec3& point, float radius) const
{
    const float w = radius + (width * 0.5f);
    const float h = radius + (height * 0.5f);
    return !((point.x >  w) || (point.x < -w) || (point.y >  h) || (point.y < -h));
}


// ----------------------------------------------------------------------------
