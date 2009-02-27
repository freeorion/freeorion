// ----------------------------------------------------------------------------
//
//
// OpenSteer -- Steering Behaviors for Autonomous Characters
//
// Copyright (c) 2002-2005, Sony Computer Entertainment America
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
// Obstacles for use with obstacle avoidance
//
// 10-04-04 bk:  put everything into the OpenSteer namespace
// 09-05-02 cwr: created
//
//
// ----------------------------------------------------------------------------


#ifndef OPENSTEER_OBSTACLE_H
#define OPENSTEER_OBSTACLE_H


#include "Vec3.h"
#include "LocalSpace.h"
#include "AbstractVehicle.h"


namespace OpenSteer {

    
    // Forward declaration.
    class Color;
    

    // ----------------------------------------------------------------------------
    // AbstractObstacle: a pure virtual base class for an abstract shape in
    // space, to be used with obstacle avoidance.  (Oops, its not "pure" since
    // I added a concrete method to PathIntersection 11-04-04 -cwr).


    class AbstractObstacle
    {
    public:

        virtual ~AbstractObstacle() { /* Nothing to do. */ }
        
        
        // compute steering for a vehicle to avoid this obstacle, if needed
        virtual Vec3 steerToAvoid (const AbstractVehicle& v,
                                   const float minTimeToCollision) const = 0;

        // PathIntersection object: used internally to analyze and store
        // information about intersections of vehicle paths and obstacles.
        class PathIntersection
        {
        public:
            bool intersect; // was an intersection found?
            float distance; // how far was intersection point from vehicle?
            Vec3 surfacePoint; // position of intersection
            Vec3 surfaceNormal; // unit normal at point of intersection
            Vec3 steerHint; // where to steer away from intersection
            bool vehicleOutside; // is the vehicle outside the obstacle?
            const AbstractObstacle* obstacle; // obstacle the path intersects

            // determine steering based on path intersection tests
            Vec3 steerToAvoidIfNeeded (const AbstractVehicle& vehicle,
                                       const float minTimeToCollision) const;

        };

        // find first intersection of a vehicle's path with this obstacle
        // (this must be specialized for each new obstacle shape class)
        virtual void
        findIntersectionWithVehiclePath (const AbstractVehicle& vehicle,
                                         PathIntersection& pi)
            const
            = 0 ;

        // virtual function for drawing -- normally does nothing, can be
        // specialized by derived types to provide graphics for obstacles
        virtual void draw (const bool filled,
                           const Color& color,
                           const Vec3& viewpoint)
            const
            = 0 ;

        // seenFrom (eversion): does this obstacle contrain vehicle to stay
        // inside it or outside it (or both)?  "Inside" describes a clear space
        // within a solid (for example, the interior of a room inside its
        // walls). "Ouitside" describes a solid chunk in the midst of clear
        // space.
        enum seenFromState {outside, inside, both};
        virtual seenFromState seenFrom (void) const = 0;
        virtual void setSeenFrom (seenFromState s) = 0;
    };


    // an STL vector of AbstractObstacle pointers and an iterator for it:
    typedef std::vector<AbstractObstacle*> ObstacleGroup;
    typedef ObstacleGroup::const_iterator ObstacleIterator;


    // ----------------------------------------------------------------------------
    // Obstacle is a utility base class providing some shared functionality


    class Obstacle : public AbstractObstacle
    {
    public:

        Obstacle (void) : _seenFrom (outside) {}

        virtual ~Obstacle() { /* Nothing to do. */ }
        
        // compute steering for a vehicle to avoid this obstacle, if needed 
        Vec3 steerToAvoid (const AbstractVehicle& v,
                           const float minTimeToCollision)
            const;

        // static method to apply steerToAvoid to nearest obstacle in an
        // ObstacleGroup
        static Vec3 steerToAvoidObstacles (const AbstractVehicle& vehicle,
                                           const float minTimeToCollision,
                                           const ObstacleGroup& obstacles);

        // static method to find first vehicle path intersection in an
        // ObstacleGroup
        static void
        firstPathIntersectionWithObstacleGroup (const AbstractVehicle& vehicle,
                                                const ObstacleGroup& obstacles,
                                                PathIntersection& nearest,
                                                PathIntersection& next);

        // static method to apply steerToAvoid to nearest obstacle in an
        // ObstacleGroup
        template <typename Iter>
        static Vec3 steerToAvoidObstacles (const AbstractVehicle& vehicle,
                                           const float minTimeToCollision,
                                           Iter first, Iter last);

        // static method to find first vehicle path intersection in an
        // ObstacleGroup
        template <typename Iter>
        static void
        firstPathIntersectionWithObstacleGroup (const AbstractVehicle& vehicle,
                                                Iter first, Iter last,
                                                PathIntersection& nearest,
                                                PathIntersection& next);

        // default do-nothing draw function (derived class can overload this)
        void draw (const bool, const Color&, const Vec3&) const {}

        seenFromState seenFrom (void) const {return _seenFrom;}
        void setSeenFrom (seenFromState s) {_seenFrom = s;}
    private:
        seenFromState _seenFrom;
    };


    // ----------------------------------------------------------------------------
    // SphereObstacle a simple ball-shaped obstacle


    class SphereObstacle : public Obstacle
    {
    public:
        float radius;
        Vec3 center;

        // constructors
        SphereObstacle (float r, Vec3 c) : radius(r), center (c) {}
        SphereObstacle (void) : radius(1), center (Vec3::zero) {}

        virtual ~SphereObstacle() { /* Nothing to do. */ }
        
        // find first intersection of a vehicle's path with this obstacle
        void findIntersectionWithVehiclePath (const AbstractVehicle& vehicle,
                                              PathIntersection& pi)
            const;
    };


    // ----------------------------------------------------------------------------
    // LocalSpaceObstacle: a mixture of LocalSpace and Obstacle methods


     typedef LocalSpaceMixin<Obstacle> LocalSpaceObstacle;


    // ----------------------------------------------------------------------------
    // BoxObstacle: a box-shaped (cuboid) obstacle of a given height, width,
    // depth, position and orientation.  The box is centered on and aligned
    // with a local space.


    class BoxObstacle : public LocalSpaceObstacle
    {
    public:
        float width;  // width  of box centered on local X (side)    axis
        float height; // height of box centered on local Y (up)      axis
        float depth;  // depth  of box centered on local Z (forward) axis

        // constructors
        BoxObstacle (float w, float h, float d) : width(w), height(h), depth(d) {}
        BoxObstacle (void) :  width(1.0f), height(1.0f), depth(1.0f) {}

        virtual ~BoxObstacle() { /* Nothing to do. */ }
        
        
        // find first intersection of a vehicle's path with this obstacle
        void findIntersectionWithVehiclePath (const AbstractVehicle& vehicle,
                                              PathIntersection& pi)
            const;
    };


    // ----------------------------------------------------------------------------
    // PlaneObstacle: a planar obstacle of a given position and orientation.
    // The plane is defined as the XY (aka side/up) plane of a local space.
    // The +Z (forward) half-space is considered "outside" the obstacle.  
    //
    // This is also the base class for several other obstacles which represent
    // 2d shapes (rectangle, triangle, ...) arbitarily oriented and positioned
    // in 2d space.  They specialize this class via xyPointInsideShape which
    // tests if a given point on the XZ plane is inside the obstacle's shape.


    class PlaneObstacle : public LocalSpaceObstacle
    {
    public:
        // constructors
        PlaneObstacle (void) {}
        PlaneObstacle (const Vec3& s,
                       const Vec3& u,
                       const Vec3& f,
                       const Vec3& p)
        : LocalSpaceObstacle( s, u, f, p )
        {
            /*
            setSide (s);
            setUp (u);
            setForward (f);
            setPosition (p);
             */
        }

        // find first intersection of a vehicle's path with this obstacle
        void findIntersectionWithVehiclePath (const AbstractVehicle& vehicle,
                                              PathIntersection& pi)
            const;

        // determines if a given point on XY plane is inside obstacle shape
        virtual bool xyPointInsideShape (const Vec3& /*point*/,
                                         float /*radius*/) const
        {
            return true; // always true for PlaneObstacle
        }
    };


    // ----------------------------------------------------------------------------
    // RectangleObstacle: a rectangular obstacle of a given height, width,
    // position and orientation.  It is a rectangle centered on the XY (aka
    // side/up) plane of a local space.


    class RectangleObstacle : public PlaneObstacle
    {
    public:
        float width;  // width  of rectangle centered on local X (side) axis
        float height; // height of rectangle centered on local Y (up)   axis

        // constructors
        RectangleObstacle (float w, float h) : width(w), height(h) {}
        RectangleObstacle (void) :  width(1.0f), height(1.0f) {}
        RectangleObstacle (float w, float h, const Vec3& s,
                           const Vec3& u, const Vec3& f, const Vec3& p,
                           seenFromState sf) 
            : PlaneObstacle( s, u, f, p ), width(w), height(h)
        {
            /*
            setSide (s);
            setUp (u);
            setForward (f);
            setPosition (p);
             */
            setSeenFrom (sf);
        }
        
        virtual ~RectangleObstacle() { /* Nothing to do. */ }

        // determines if a given point on XY plane is inside obstacle shape
        bool xyPointInsideShape (const Vec3& point, float radius) const;
    };


     // template implementations
     // static method to apply steerToAvoid to nearest obstacle in an
     // ObstacleGroup
     template <typename Iter>
     Vec3 Obstacle::steerToAvoidObstacles (const AbstractVehicle& vehicle,
                                           const float minTimeToCollision,
                                           Iter first, Iter last)
     {
         PathIntersection nearest, next;

         // test all obstacles in group for an intersection with the vehicle's
         // future path, select the one whose point of intersection is nearest
         firstPathIntersectionWithObstacleGroup (vehicle, first, last, nearest, next);

         // if nearby intersection found, steer away from it, otherwise no steering
         return nearest.steerToAvoidIfNeeded (vehicle, minTimeToCollision);
     }

     // static method to find first vehicle path intersection in an
     // ObstacleGroup
     template <typename Iter>
     void
     Obstacle::firstPathIntersectionWithObstacleGroup (const AbstractVehicle& vehicle,
                                                       Iter first, Iter last,
                                                       PathIntersection& nearest,
                                                       PathIntersection& next)
     {
         // test all obstacles in group for an intersection with the vehicle's
         // future path, select the one whose point of intersection is nearest
         next.intersect = false;
         nearest.intersect = false;
         for (Iter o = first; o != last; ++o)
         {
             // find nearest point (if any) where vehicle path intersects obstacle
             // o, storing the results in PathIntersection object "next"
             o->findIntersectionWithVehiclePath (vehicle, next);

             // if this is the first intersection found, or it is the nearest found
             // so far, store it in PathIntersection object "nearest"
             const bool firstFound = !nearest.intersect;
             const bool nearestFound = (next.intersect &&
                                        (next.distance < nearest.distance));
             if (firstFound || nearestFound) nearest = next;
         }
     }


} // namespace OpenSteer
    
    
// ----------------------------------------------------------------------------
#endif // OPENSTEER_OBSTACLE_H
