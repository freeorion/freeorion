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
// camera control for OpenSteerDemo
//
// a camera ("point of view") with various "aiming modes" to track a
// moving vehicle
//
// 10-04-04 bk:  put everything into the OpenSteer namespace
// 06-26-02 cwr: created 
//
//
// ----------------------------------------------------------------------------


#ifndef OPENSTEER_CAMERA_H
#define OPENSTEER_CAMERA_H


#include "OpenSteer/LocalSpace.h"
#include "OpenSteer/AbstractVehicle.h"


// ----------------------------------------------------------------------------


namespace OpenSteer {

    class Camera : public LocalSpace
    {
    public:

        // constructor
        Camera ();
        virtual ~Camera() { /* Nothing to do? */ }

        // reset all camera state to default values
        void reset (void);

        // "look at" point, center of view
        Vec3 target;

        // vehicle being tracked
        const AbstractVehicle* vehicleToTrack;

        // aim at predicted position of vehicleToTrack, this far into thefuture
        float aimLeadTime;

        // per frame simulation update
        void update (const float currentTime,
                     const float elapsedTime,
                     const bool simulationPaused);
        void update (const float currentTime, const float elapsedTime)
        {update (currentTime, elapsedTime, false);};

        // helper function for "drag behind" mode
        Vec3 constDistHelper (const float elapsedTime);

        // Smoothly move camera ...
        void smoothCameraMove (const Vec3& newPosition,
                               const Vec3& newTarget,
                               const Vec3& newUp,
                               const float elapsedTime);

        void doNotSmoothNextMove (void) {smoothNextMove = false;};

        bool smoothNextMove;
        float smoothMoveSpeed;

        // adjust the offset vector of the current camera mode based on a
        // "mouse adjustment vector" from OpenSteerDemo (xxx experiment 10-17-02)
        void mouseAdjustOffset (const Vec3& adjustment);
        Vec3 mouseAdjust2 (const bool polar,
                           const Vec3& adjustment,
                           const Vec3& offsetToAdjust);
        Vec3 mouseAdjustPolar (const Vec3& adjustment,
                               const Vec3& offsetToAdjust)
        {return mouseAdjust2 (true, adjustment, offsetToAdjust);};
        Vec3 mouseAdjustOrtho (const Vec3& adjustment,
                               const Vec3& offsetToAdjust)
        {return mouseAdjust2 (false, adjustment, offsetToAdjust);};

        // xxx since currently (10-21-02) the camera's Forward and Side basis
        // xxx vectors are not being set, construct a temporary local space for
        // xxx the camera view -- so as not to make the camera behave
        // xxx differently (which is to say, correctly) during mouse adjustment.
        LocalSpace ls;
        const LocalSpace& xxxls (void)
        {ls.regenerateOrthonormalBasis (target - position(), up()); return ls;}


        // camera mode selection
        enum cameraMode 
            {
                // marks beginning of list
                cmStartMode,
            
                // fixed global position and aimpoint
                cmFixed,

                // camera position is directly above (in global Up/Y) target
                // camera up direction is target's forward direction
                cmStraightDown,

                // look at subject vehicle, adjusting camera position to be a
                // constant distance from the subject
                cmFixedDistanceOffset,

                // camera looks at subject vehicle from a fixed offset in the
                // local space of the vehicle (as if attached to the vehicle)
                cmFixedLocalOffset,

                // camera looks in the vehicle's forward direction, camera
                // position has a fixed local offset from the vehicle.
                cmOffsetPOV,

                // cmFixedPositionTracking // xxx maybe?

                // marks the end of the list for cycling (to cmStartMode+1)
                cmEndMode
            };

        // current mode for this camera instance
        cameraMode mode;

        // string naming current camera mode, used by OpenSteerDemo
        char* modeName (void);

        // select next camera mode, used by OpenSteerDemo
        void selectNextMode (void);

        // the mode that comes after the given mode (used by selectNextMode)
        cameraMode successorMode (const cameraMode cm) const;

        // "static" camera mode parameters
        Vec3 fixedPosition;
        Vec3 fixedTarget;
        Vec3 fixedUp;

        // "constant distance from vehicle" camera mode parameters
        float fixedDistDistance;             // desired distance from it
        float fixedDistVOffset;              // fixed vertical offset from it

        // "look straight down at vehicle" camera mode parameters
        float lookdownDistance;             // fixed vertical offset from it

        // "fixed local offset" camera mode parameters
        Vec3 fixedLocalOffset;

        // "offset POV" camera mode parameters
        Vec3 povOffset;
    };

} // namespace OpenSteer

// ----------------------------------------------------------------------------
#endif // OPENSTEER_CAMERA_H
