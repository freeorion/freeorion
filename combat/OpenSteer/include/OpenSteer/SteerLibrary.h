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
// SteerLibraryMixin
//
// This mixin (class with templated superclass) adds the "steering library"
// functionality to a given base class.  SteerLibraryMixin assumes its base
// class supports the AbstractVehicle interface.
//
// 10-04-04 bk:  put everything into the OpenSteer namespace
// 02-06-03 cwr: create mixin (from "SteerMass")
// 06-03-02 cwr: removed TS dependencies
// 11-21-01 cwr: created
//
//
// ----------------------------------------------------------------------------


#ifndef OPENSTEER_STEERLIBRARY_H
#define OPENSTEER_STEERLIBRARY_H


#include "OpenSteer/AbstractVehicle.h"
#include "OpenSteer/Pathway.h"
#include "OpenSteer/Obstacle.h"
#include "OpenSteer/Utilities.h"

// Include OpenSteer::Color, OpenSteer::gBlack, ...
#include "Color.h"

namespace OpenSteer {

    // ----------------------------------------------------------------------------


    template <class Super>
    class SteerLibraryMixin : public Super
    {
    public:
        using Super::velocity;
        using Super::maxSpeed;
        using Super::speed;
        using Super::radius;
        using Super::maxForce;
        using Super::forward;
        using Super::position;
        using Super::side;
        using Super::up;
        using Super::predictFuturePosition;
        
    public:

        // Constructor: initializes state
        SteerLibraryMixin ()
        {
            // set inital state
            reset ();
        }

        // reset state
        void reset (void)
        {
            // initial state of wander behavior
            WanderSide = 0;
            WanderUp = 0;

            // default to non-gaudyPursuitAnnotation
            gaudyPursuitAnnotation = false;
        }

        // -------------------------------------------------- steering behaviors

        // Wander behavior
        float WanderSide;
        float WanderUp;
        Vec3 steerForWander (float dt);

        // Seek behavior
        Vec3 steerForSeek (const Vec3& target);

        // Flee behavior
        Vec3 steerForFlee (const Vec3& target);

        // xxx proposed, experimental new seek/flee [cwr 9-16-02]
        Vec3 xxxsteerForFlee (const Vec3& target);
        Vec3 xxxsteerForSeek (const Vec3& target);

        // Path Following behaviors
        Vec3 steerToFollowPath (const int direction,
                                const float predictionTime,
                                Pathway& path);
        Vec3 steerToStayOnPath (const float predictionTime, Pathway& path);

        // ------------------------------------------------------------------------
        // Obstacle Avoidance behavior
        //
        // Returns a steering force to avoid a given obstacle.  The purely
        // lateral steering force will turn our vehicle towards a silhouette edge
        // of the obstacle.  Avoidance is required when (1) the obstacle
        // intersects the vehicle's current path, (2) it is in front of the
        // vehicle, and (3) is within minTimeToCollision seconds of travel at the
        // vehicle's current velocity.  Returns a zero vector value (Vec3::zero)
        // when no avoidance is required.


        Vec3 steerToAvoidObstacle (const float minTimeToCollision,
                                   const Obstacle& obstacle);


        // avoids all obstacles in an ObstacleGroup

        Vec3 steerToAvoidObstacles (const float minTimeToCollision,
                                    const ObstacleGroup& obstacles);

        template <typename Iter>
        Vec3 steerToAvoidObstacles (const float minTimeToCollision,
                                    Iter first, Iter last);


        // ------------------------------------------------------------------------
        // Unaligned collision avoidance behavior: avoid colliding with other
        // nearby vehicles moving in unconstrained directions.  Determine which
        // (if any) other other vehicle we would collide with first, then steers
        // to avoid the site of that potential collision.  Returns a steering
        // force vector, which is zero length if there is no impending collision.


        Vec3 steerToAvoidNeighbors (const float minTimeToCollision,
                                    const AVGroup& others);


        // Given two vehicles, based on their current positions and velocities,
        // determine the time until nearest approach
        float predictNearestApproachTime (AbstractVehicle& otherVehicle);

        // Given the time until nearest approach (predictNearestApproachTime)
        // determine position of each vehicle at that time, and the distance
        // between them
        float computeNearestApproachPositions (AbstractVehicle& otherVehicle,
                                               float time);


        /// XXX globals only for the sake of graphical annotation
        Vec3 hisPositionAtNearestApproach;
        Vec3 ourPositionAtNearestApproach;


        // ------------------------------------------------------------------------
        // avoidance of "close neighbors" -- used only by steerToAvoidNeighbors
        //
        // XXX  Does a hard steer away from any other agent who comes withing a
        // XXX  critical distance.  Ideally this should be replaced with a call
        // XXX  to steerForSeparation.


        Vec3 steerToAvoidCloseNeighbors (const float minSeparationDistance,
                                         const AVGroup& others);


        // ------------------------------------------------------------------------
        // used by boid behaviors


        bool inBoidNeighborhood (const AbstractVehicle& otherVehicle,
                                 const float minDistance,
                                 const float maxDistance,
                                 const float cosMaxAngle);


        // ------------------------------------------------------------------------
        // Separation behavior -- determines the direction away from nearby boids


        Vec3 steerForSeparation (const float maxDistance,
                                 const float cosMaxAngle,
                                 const AVGroup& flock);


        // ------------------------------------------------------------------------
        // Alignment behavior

        Vec3 steerForAlignment (const float maxDistance,
                                const float cosMaxAngle,
                                const AVGroup& flock);


        // ------------------------------------------------------------------------
        // Cohesion behavior


        Vec3 steerForCohesion (const float maxDistance,
                               const float cosMaxAngle,
                               const AVGroup& flock);


        // ------------------------------------------------------------------------
        // pursuit of another vehicle (& version with ceiling on prediction time)


        Vec3 steerForPursuit (const AbstractVehicle& quarry);

        Vec3 steerForPursuit (const AbstractVehicle& quarry,
                              const float maxPredictionTime);

        // for annotation
        bool gaudyPursuitAnnotation;


        // ------------------------------------------------------------------------
        // evasion of another vehicle


        Vec3 steerForEvasion (const AbstractVehicle& menace,
                              const float maxPredictionTime);


        // ------------------------------------------------------------------------
        // tries to maintain a given speed, returns a maxForce-clipped steering
        // force along the forward/backward axis


        Vec3 steerForTargetSpeed (const float targetSpeed);


        // ----------------------------------------------------------- utilities
        // XXX these belong somewhere besides the steering library
        // XXX above AbstractVehicle, below SimpleVehicle
        // XXX ("utility vehicle"?)

        // xxx cwr experimental 9-9-02 -- names OK?
        bool isAhead (const Vec3& target) const {return isAhead (target, 0.707f);};
        bool isAside (const Vec3& target) const {return isAside (target, 0.707f);};
        bool isBehind (const Vec3& target) const {return isBehind (target, -0.707f);};

        bool isAhead (const Vec3& target, float cosThreshold) const
        {
            const Vec3 targetDirection = (target - position ()).normalize ();
            return forward().dot(targetDirection) > cosThreshold;
        };
        bool isAside (const Vec3& target, float cosThreshold) const
        {
            const Vec3 targetDirection = (target - position ()).normalize ();
            const float dp = forward().dot(targetDirection);
            return (dp < cosThreshold) && (dp > -cosThreshold);
        };
        bool isBehind (const Vec3& target, float cosThreshold) const
        {
            const Vec3 targetDirection = (target - position()).normalize ();
            return forward().dot(targetDirection) < cosThreshold;
        };


        // ------------------------------------------------ graphical annotation
        // (parameter names commented out to prevent compiler warning from "-W")


        // called when steerToAvoidObstacles decides steering is required
        // (default action is to do nothing, layered classes can overload it)
        virtual void annotateAvoidObstacle (const float /*minDistanceToCollision*/)
        {
        }

        // called when steerToFollowPath decides steering is required
        // (default action is to do nothing, layered classes can overload it)
        virtual void annotatePathFollowing (const Vec3& /*future*/,
                                            const Vec3& /*onPath*/,
                                            const Vec3& /*target*/,
                                            const float /*outside*/)
        {
        }

        // called when steerToAvoidCloseNeighbors decides steering is required
        // (default action is to do nothing, layered classes can overload it)
        virtual void annotateAvoidCloseNeighbor (const AbstractVehicle& /*other*/,
                                                 const float /*additionalDistance*/)
        {
        }

        // called when steerToAvoidNeighbors decides steering is required
        // (default action is to do nothing, layered classes can overload it)
        virtual void annotateAvoidNeighbor (const AbstractVehicle& /*threat*/,
                                            const float /*steer*/,
                                            const Vec3& /*ourFuture*/,
                                            const Vec3& /*threatFuture*/)
        {
        }
    };

    
} // namespace OpenSteer

// ----------------------------------------------------------------------------


template<class Super>
OpenSteer::Vec3
OpenSteer::SteerLibraryMixin<Super>::
steerForWander (float dt)
{
    // random walk WanderSide and WanderUp between -1 and +1
    const float speed = 12.0f * dt; // maybe this (12) should be an argument?
    WanderSide = scalarRandomWalk (WanderSide, speed, -1, +1);
    WanderUp   = scalarRandomWalk (WanderUp,   speed, -1, +1);

    // return a pure lateral steering vector: (+/-Side) + (+/-Up)
    return (side() * WanderSide) + (up() * WanderUp);
}


// ----------------------------------------------------------------------------
// Seek behavior


template<class Super>
OpenSteer::Vec3
OpenSteer::SteerLibraryMixin<Super>::
steerForSeek (const Vec3& target)
{
    const Vec3 desiredVelocity = target - position();
    return desiredVelocity - velocity();
}


// ----------------------------------------------------------------------------
// Flee behavior


template<class Super>
OpenSteer::Vec3
OpenSteer::SteerLibraryMixin<Super>::
steerForFlee (const Vec3& target)
{
    const Vec3 desiredVelocity = position() - target;
    return desiredVelocity - velocity();
}


// ----------------------------------------------------------------------------
// xxx proposed, experimental new seek/flee [cwr 9-16-02]


template<class Super>
OpenSteer::Vec3
OpenSteer::SteerLibraryMixin<Super>::
xxxsteerForFlee (const Vec3& target)
{
//  const Vec3 offset = position - target;
    const Vec3 offset = position() - target;
    const Vec3 desiredVelocity = offset.truncateLength (maxSpeed ()); //xxxnew
    return desiredVelocity - velocity();
}


template<class Super>
OpenSteer::Vec3
OpenSteer::SteerLibraryMixin<Super>::
xxxsteerForSeek (const Vec3& target)
{
//  const Vec3 offset = target - position;
    const Vec3 offset = target - position();
    const Vec3 desiredVelocity = offset.truncateLength (maxSpeed ()); //xxxnew
    return desiredVelocity - velocity();
}


// ----------------------------------------------------------------------------
// Path Following behaviors


template<class Super>
OpenSteer::Vec3
OpenSteer::SteerLibraryMixin<Super>::
steerToStayOnPath (const float predictionTime, Pathway& path)
{
    // predict our future position
    const Vec3 futurePosition = predictFuturePosition (predictionTime);

    // find the point on the path nearest the predicted future position
    Vec3 tangent;
    float outside;
    const Vec3 onPath = path.mapPointToPath (futurePosition,
                                             tangent,     // output argument
                                             outside);    // output argument

    if (outside < 0)
    {
        // our predicted future position was in the path,
        // return zero steering.
        return Vec3::zero;
    }
    else
    {
        // our predicted future position was outside the path, need to
        // steer towards it.  Use onPath projection of futurePosition
        // as seek target
        annotatePathFollowing (futurePosition, onPath, onPath, outside);
        return steerForSeek (onPath);
    }
}


template<class Super>
OpenSteer::Vec3
OpenSteer::SteerLibraryMixin<Super>::
steerToFollowPath (const int direction,
                   const float predictionTime,
                   Pathway& path)
{
    // our goal will be offset from our path distance by this amount
    const float pathDistanceOffset = direction * predictionTime * speed();

    // predict our future position
    const Vec3 futurePosition = predictFuturePosition (predictionTime);

    // measure distance along path of our current and predicted positions
    const float nowPathDistance =
        path.mapPointToPathDistance (position ());
    const float futurePathDistance =
        path.mapPointToPathDistance (futurePosition);

    // are we facing in the correction direction?
    const bool rightway = ((pathDistanceOffset > 0) ?
                           (nowPathDistance < futurePathDistance) :
                           (nowPathDistance > futurePathDistance));

    // find the point on the path nearest the predicted future position
    // XXX need to improve calling sequence, maybe change to return a
    // XXX special path-defined object which includes two Vec3s and a 
    // XXX bool (onPath,tangent (ignored), withinPath)
    Vec3 tangent;
    float outside;
    const Vec3 onPath = path.mapPointToPath (futurePosition,
                                             // output arguments:
                                             tangent,
                                             outside);

    // no steering is required if (a) our future position is inside
    // the path tube and (b) we are facing in the correct direction
    if ((outside < 0) && rightway)
    {
        // all is well, return zero steering
        return Vec3::zero;
    }
    else
    {
        // otherwise we need to steer towards a target point obtained
        // by adding pathDistanceOffset to our current path position

        float const targetPathDistance = nowPathDistance + pathDistanceOffset;
        Vec3 const target = path.mapPathDistanceToPoint (targetPathDistance);

        annotatePathFollowing (futurePosition, onPath, target, outside);

        // return steering to seek target on path
        return steerForSeek (target);
    }
}


// ----------------------------------------------------------------------------
// Obstacle Avoidance behavior
//
// Returns a steering force to avoid a given obstacle.  The purely lateral
// steering force will turn our vehicle towards a silhouette edge of the
// obstacle.  Avoidance is required when (1) the obstacle intersects the
// vehicle's current path, (2) it is in front of the vehicle, and (3) is
// within minTimeToCollision seconds of travel at the vehicle's current
// velocity.  Returns a zero vector value (Vec3::zero) when no avoidance is
// required.
//
// XXX The current (4-23-03) scheme is to dump all the work on the various
// XXX Obstacle classes, making them provide a "steer vehicle to avoid me"
// XXX method.  This may well change.
//
// XXX 9-12-03: this routine is probably obsolete: its name is too close to
// XXX the new steerToAvoidObstacles and the arguments are reversed
// XXX (perhaps there should be another version of steerToAvoidObstacles
// XXX whose second arg is "const Obstacle& obstacle" just in case we want
// XXX to avoid a non-grouped obstacle)

template<class Super>
OpenSteer::Vec3
OpenSteer::SteerLibraryMixin<Super>::
steerToAvoidObstacle (const float minTimeToCollision,
                      const Obstacle& obstacle)
{
    const Vec3 avoidance = obstacle.steerToAvoid (*this, minTimeToCollision);

    // XXX more annotation modularity problems (assumes spherical obstacle)
    if (avoidance != Vec3::zero)
        annotateAvoidObstacle (minTimeToCollision * speed());

    return avoidance;
}


// this version avoids all of the obstacles in an ObstacleGroup

template<class Super>
OpenSteer::Vec3
OpenSteer::SteerLibraryMixin<Super>::
steerToAvoidObstacles (const float minTimeToCollision,
                       const ObstacleGroup& obstacles)
{
    const Vec3 avoidance = Obstacle::steerToAvoidObstacles (*this,
                                                            minTimeToCollision,
                                                            obstacles);

    // XXX more annotation modularity problems (assumes spherical obstacle)
    if (avoidance != Vec3::zero)
        annotateAvoidObstacle (minTimeToCollision * speed());

    return avoidance;
}

template <class Super>
template <typename Iter>
OpenSteer::Vec3
OpenSteer::SteerLibraryMixin<Super>::
steerToAvoidObstacles (const float minTimeToCollision,
                       Iter first, Iter last)
{
    const Vec3 avoidance = Obstacle::steerToAvoidObstacles (*this,
                                                            minTimeToCollision,
                                                            first, last);

    // XXX more annotation modularity problems (assumes spherical obstacle)
    if (avoidance != Vec3::zero)
        annotateAvoidObstacle (minTimeToCollision * speed());

    return avoidance;
}


// ----------------------------------------------------------------------------
// Unaligned collision avoidance behavior: avoid colliding with other nearby
// vehicles moving in unconstrained directions.  Determine which (if any)
// other other vehicle we would collide with first, then steers to avoid the
// site of that potential collision.  Returns a steering force vector, which
// is zero length if there is no impending collision.


template<class Super>
OpenSteer::Vec3
OpenSteer::SteerLibraryMixin<Super>::
steerToAvoidNeighbors (const float minTimeToCollision,
                       const AVGroup& others)
{
    // first priority is to prevent immediate interpenetration
    const Vec3 separation = steerToAvoidCloseNeighbors (0, others);
    if (separation != Vec3::zero) return separation;

    // otherwise, go on to consider potential future collisions
    float steer = 0;
    AbstractVehicle* threat = NULL;

    // Time (in seconds) until the most immediate collision threat found
    // so far.  Initial value is a threshold: don't look more than this
    // many frames into the future.
    float minTime = minTimeToCollision;

    // xxx solely for annotation
    Vec3 xxxThreatPositionAtNearestApproach;
    Vec3 xxxOurPositionAtNearestApproach;

    // for each of the other vehicles, determine which (if any)
    // pose the most immediate threat of collision.
    for (AVIterator i = others.begin(); i != others.end(); ++i)
    {
        AbstractVehicle& other = **i;
        if (&other != this)
        {	
            // avoid when future positions are this close (or less)
            const float collisionDangerThreshold = radius() * 2;

            // predicted time until nearest approach of "this" and "other"
            const float time = predictNearestApproachTime (other);

            // If the time is in the future, sooner than any other
            // threatened collision...
            if ((time >= 0) && (time < minTime))
            {
                // if the two will be close enough to collide,
                // make a note of it
                if (computeNearestApproachPositions (other, time)
                    < collisionDangerThreshold)
                {
                    minTime = time;
                    threat = &other;
                    xxxThreatPositionAtNearestApproach
                        = hisPositionAtNearestApproach;
                    xxxOurPositionAtNearestApproach
                        = ourPositionAtNearestApproach;
                }
            }
        }
    }

    // if a potential collision was found, compute steering to avoid
    if (threat != NULL)
    {
        // parallel: +1, perpendicular: 0, anti-parallel: -1
        float parallelness = forward().dot(threat->forward());
        float angle = 0.707f;

        if (parallelness < -angle)
        {
            // anti-parallel "head on" paths:
            // steer away from future threat position
            Vec3 offset = xxxThreatPositionAtNearestApproach - position();
            float sideDot = offset.dot(side());
            steer = (sideDot > 0) ? -1.0f : 1.0f;
        }
        else
        {
            if (parallelness > angle)
            {
                // parallel paths: steer away from threat
                Vec3 offset = threat->position() - position();
                float sideDot = offset.dot(side());
                steer = (sideDot > 0) ? -1.0f : 1.0f;
            }
            else
            {
                // perpendicular paths: steer behind threat
                // (only the slower of the two does this)
                if (threat->speed() <= speed())
                {
                    float sideDot = side().dot(threat->velocity());
                    steer = (sideDot > 0) ? -1.0f : 1.0f;
                }
            }
        }

        annotateAvoidNeighbor (*threat,
                               steer,
                               xxxOurPositionAtNearestApproach,
                               xxxThreatPositionAtNearestApproach);
    }

    return side() * steer;
}



// Given two vehicles, based on their current positions and velocities,
// determine the time until nearest approach
//
// XXX should this return zero if they are already in contact?

template<class Super>
float
OpenSteer::SteerLibraryMixin<Super>::
predictNearestApproachTime (AbstractVehicle& otherVehicle)
{
    // imagine we are at the origin with no velocity,
    // compute the relative velocity of the other vehicle
    const Vec3 myVelocity = velocity();
    const Vec3 otherVelocity = otherVehicle.velocity();
    const Vec3 relVelocity = otherVelocity - myVelocity;
    const float relSpeed = relVelocity.length();

    // for parallel paths, the vehicles will always be at the same distance,
    // so return 0 (aka "now") since "there is no time like the present"
    if (relSpeed == 0) return 0;

    // Now consider the path of the other vehicle in this relative
    // space, a line defined by the relative position and velocity.
    // The distance from the origin (our vehicle) to that line is
    // the nearest approach.

    // Take the unit tangent along the other vehicle's path
    const Vec3 relTangent = relVelocity / relSpeed;

    // find distance from its path to origin (compute offset from
    // other to us, find length of projection onto path)
    const Vec3 relPosition = position() - otherVehicle.position();
    const float projection = relTangent.dot(relPosition);

    return projection / relSpeed;
}


// Given the time until nearest approach (predictNearestApproachTime)
// determine position of each vehicle at that time, and the distance
// between them


template<class Super>
float
OpenSteer::SteerLibraryMixin<Super>::
computeNearestApproachPositions (AbstractVehicle& otherVehicle,
                                 float time)
{
    const Vec3    myTravel =       forward () *       speed () * time;
    const Vec3 otherTravel = otherVehicle.forward () * otherVehicle.speed () * time;

    const Vec3    myFinal =       position () +    myTravel;
    const Vec3 otherFinal = otherVehicle.position () + otherTravel;

    // xxx for annotation
    ourPositionAtNearestApproach = myFinal;
    hisPositionAtNearestApproach = otherFinal;

    return Vec3::distance (myFinal, otherFinal);
}



// ----------------------------------------------------------------------------
// avoidance of "close neighbors" -- used only by steerToAvoidNeighbors
//
// XXX  Does a hard steer away from any other agent who comes withing a
// XXX  critical distance.  Ideally this should be replaced with a call
// XXX  to steerForSeparation.


template<class Super>
OpenSteer::Vec3
OpenSteer::SteerLibraryMixin<Super>::
steerToAvoidCloseNeighbors (const float minSeparationDistance,
                            const AVGroup& others)
{
    // for each of the other vehicles...
    for (AVIterator i = others.begin(); i != others.end(); ++i)    
    {
        AbstractVehicle& other = **i;
        if (&other != this)
        {
            const float sumOfRadii = radius() + other.radius();
            const float minCenterToCenter = minSeparationDistance + sumOfRadii;
            const Vec3 offset = other.position() - position();
            const float currentDistance = offset.length();

            if (currentDistance < minCenterToCenter)
            {
                annotateAvoidCloseNeighbor (other, minSeparationDistance);
                return (-offset).perpendicularComponent (forward());
            }
        }
    }

    // otherwise return zero
    return Vec3::zero;
}


// ----------------------------------------------------------------------------
// used by boid behaviors: is a given vehicle within this boid's neighborhood?


template<class Super>
bool
OpenSteer::SteerLibraryMixin<Super>::
inBoidNeighborhood (const AbstractVehicle& otherVehicle,
                    const float minDistance,
                    const float maxDistance,
                    const float cosMaxAngle)
{
    if (&otherVehicle == this)
    {
        return false;
    }
    else
    {
        const Vec3 offset = otherVehicle.position() - position();
        const float distanceSquared = offset.lengthSquared ();

        // definitely in neighborhood if inside minDistance sphere
        if (distanceSquared < (minDistance * minDistance))
        {
            return true;
        }
        else
        {
            // definitely not in neighborhood if outside maxDistance sphere
            if (distanceSquared > (maxDistance * maxDistance))
            {
                return false;
            }
            else
            {
                // otherwise, test angular offset from forward axis
                const Vec3 unitOffset = offset / sqrt (distanceSquared);
                const float forwardness = forward().dot (unitOffset);
                return forwardness > cosMaxAngle;
            }
        }
    }
}


// ----------------------------------------------------------------------------
// Separation behavior: steer away from neighbors


template<class Super>
OpenSteer::Vec3
OpenSteer::SteerLibraryMixin<Super>::
steerForSeparation (const float maxDistance,
                    const float cosMaxAngle,
                    const AVGroup& flock)
{
    // steering accumulator and count of neighbors, both initially zero
    Vec3 steering;

    // for each of the other vehicles...
    AVIterator flockEndIter = flock.end();
    for (AVIterator otherVehicle = flock.begin(); otherVehicle != flockEndIter; ++otherVehicle )
    {
        if (inBoidNeighborhood (**otherVehicle, radius()*3, maxDistance, cosMaxAngle))
        {
            // add in steering contribution
            // (opposite of the offset direction, divided once by distance
            // to normalize, divided another time to get 1/d falloff)
            const Vec3 offset = (**otherVehicle).position() - position();
            if (const float distanceSquared = offset.dot(offset))
                steering += (offset / -distanceSquared);
        }
    }

    return steering.normalize();
}


// ----------------------------------------------------------------------------
// Alignment behavior: steer to head in same direction as neighbors


template<class Super>
OpenSteer::Vec3
OpenSteer::SteerLibraryMixin<Super>::
steerForAlignment (const float maxDistance,
                   const float cosMaxAngle,
                   const AVGroup& flock)
{
    // steering accumulator and count of neighbors, both initially zero
    Vec3 steering;
    int neighbors = 0;

    // for each of the other vehicles...
    for (AVIterator otherVehicle = flock.begin(); otherVehicle != flock.end(); ++otherVehicle)
    {
        if (inBoidNeighborhood (**otherVehicle, radius()*3, maxDistance, cosMaxAngle))
        {
            // accumulate sum of neighbor's heading
            steering += (**otherVehicle).forward();

            // count neighbors
            neighbors++;
        }
    }

    // divide by neighbors, subtract off current heading to get error-
    // correcting direction, then normalize to pure direction
    if (neighbors > 0) steering = ((steering / (float)neighbors) - forward()).normalize();

    return steering;
}


// ----------------------------------------------------------------------------
// Cohesion behavior: to to move toward center of neighbors



template<class Super>
OpenSteer::Vec3
OpenSteer::SteerLibraryMixin<Super>::
steerForCohesion (const float maxDistance,
                  const float cosMaxAngle,
                  const AVGroup& flock)
{
    // steering accumulator and count of neighbors, both initially zero
    Vec3 steering;
    int neighbors = 0;

    // for each of the other vehicles...
    for (AVIterator otherVehicle = flock.begin(); otherVehicle != flock.end(); ++otherVehicle)
    {
        if (inBoidNeighborhood (**otherVehicle, radius()*3, maxDistance, cosMaxAngle))
        {
            // accumulate sum of neighbor's positions
            steering += (**otherVehicle).position();

            // count neighbors
            neighbors++;
        }
    }

    // divide by neighbors, subtract off current position to get error-
    // correcting direction, then normalize to pure direction
    if (neighbors > 0) steering = ((steering / (float)neighbors) - position()).normalize();

    return steering;
}


// ----------------------------------------------------------------------------
// pursuit of another vehicle (& version with ceiling on prediction time)


template<class Super>
OpenSteer::Vec3
OpenSteer::SteerLibraryMixin<Super>::
steerForPursuit (const AbstractVehicle& quarry)
{
    return steerForPursuit (quarry, FLT_MAX);
}


template<class Super>
OpenSteer::Vec3
OpenSteer::SteerLibraryMixin<Super>::
steerForPursuit (const AbstractVehicle& quarry,
                 const float maxPredictionTime)
{
    // offset from this to quarry, that distance, unit vector toward quarry
    const Vec3 offset = quarry.position() - position();
    const float distance = offset.length ();
    const Vec3 unitOffset = offset / distance;

    // how parallel are the paths of "this" and the quarry
    // (1 means parallel, 0 is pependicular, -1 is anti-parallel)
    const float parallelness = forward().dot (quarry.forward());

    // how "forward" is the direction to the quarry
    // (1 means dead ahead, 0 is directly to the side, -1 is straight back)
    const float forwardness = forward().dot (unitOffset);

    const float directTravelTime = distance / speed ();
    const int f = intervalComparison (forwardness,  -0.707f, 0.707f);
    const int p = intervalComparison (parallelness, -0.707f, 0.707f);

    float timeFactor = 0; // to be filled in below
    Color color;           // to be filled in below (xxx just for debugging)

    // Break the pursuit into nine cases, the cross product of the
    // quarry being [ahead, aside, or behind] us and heading
    // [parallel, perpendicular, or anti-parallel] to us.
    switch (f)
    {
    case +1:
        switch (p)
        {
        case +1:          // ahead, parallel
            timeFactor = 4;
            color = gBlack;
            break;
        case 0:           // ahead, perpendicular
            timeFactor = 1.8f;
            color = gGray50;
            break;
        case -1:          // ahead, anti-parallel
            timeFactor = 0.85f;
            color = gWhite;
            break;
        }
        break;
    case 0:
        switch (p)
        {
        case +1:          // aside, parallel
            timeFactor = 1;
            color = gRed;
            break;
        case 0:           // aside, perpendicular
            timeFactor = 0.8f;
            color = gYellow;
            break;
        case -1:          // aside, anti-parallel
            timeFactor = 4;
            color = gGreen;
            break;
        }
        break;
    case -1:
        switch (p)
        {
        case +1:          // behind, parallel
            timeFactor = 0.5f;
            color= gCyan;
            break;
        case 0:           // behind, perpendicular
            timeFactor = 2;
            color= gBlue;
            break;
        case -1:          // behind, anti-parallel
            timeFactor = 2;
            color = gMagenta;
            break;
        }
        break;
    }

    // estimated time until intercept of quarry
    const float et = directTravelTime * timeFactor;

    // xxx experiment, if kept, this limit should be an argument
    const float etl = (et > maxPredictionTime) ? maxPredictionTime : et;

    // estimated position of quarry at intercept
    const Vec3 target = quarry.predictFuturePosition (etl);

    // annotation
    annotationLine (position(),
                    target,
                    gaudyPursuitAnnotation ? color : gGray40);

    return steerForSeek (target);
}

// ----------------------------------------------------------------------------
// evasion of another vehicle


template<class Super>
OpenSteer::Vec3
OpenSteer::SteerLibraryMixin<Super>::
steerForEvasion (const AbstractVehicle& menace,
                 const float maxPredictionTime)
{
    // offset from this to menace, that distance, unit vector toward menace
    const Vec3 offset = menace.position() - position();
    const float distance = offset.length ();

    const float roughTime = distance / menace.speed();
    const float predictionTime = ((roughTime > maxPredictionTime) ?
                                  maxPredictionTime :
                                  roughTime);

    const Vec3 target = menace.predictFuturePosition (predictionTime);

    return steerForFlee (target);
}


// ----------------------------------------------------------------------------
// tries to maintain a given speed, returns a maxForce-clipped steering
// force along the forward/backward axis


template<class Super>
OpenSteer::Vec3
OpenSteer::SteerLibraryMixin<Super>::
steerForTargetSpeed (const float targetSpeed)
{
    const float mf = maxForce ();
    const float speedError = targetSpeed - speed ();
    return forward () * clip (speedError, -mf, +mf);
}


// ----------------------------------------------------------------------------
#endif // OPENSTEER_STEERLIBRARY_H
