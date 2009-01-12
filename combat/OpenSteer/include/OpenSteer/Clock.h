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
// discrete time simulation clock for OpenSteerDemo
//
// Keeps track of real clock time and simulation time.  Encapsulates the time
// API of the underlying operating system.  Can be put in either "as fast as
// possible" variable time step mode (where simulation time steps are based on
// real time elapsed between updates), or in fixed "target FPS" mode where the
// simulation steps are constrained to start on 1/FPS boundaries (e.g. on a 60
// hertz video game console).  Also handles the notion of "pausing" simulation
// time.
//
// Usage: allocate a clock, set its "paused" or "targetFPS" parameters, then
// call updateGlobalSimulationClock before each simulation step.
//
// 10-04-04 bk:  put everything into the OpenSteer namespace
// 11-11-03 cwr: another overhaul: support aniamtion mode, switch to
//               functional API, move smoothed stats inside this class
// 09-24-02 cwr: major overhaul
// 06-26-02 cwr: created
//
//
// ----------------------------------------------------------------------------


#ifndef OPENSTEER_CLOCK_H
#define OPENSTEER_CLOCK_H

#include "OpenSteer/Utilities.h"

#if defined (_XBOX)
	#include <xtl.h>
#elif defined (_WIN32)
	#include <windows.h>
#endif


namespace OpenSteer {

    class Clock
    {
    public:

        // constructor
        Clock ();

        // update this clock, called exactly once per simulation step ("frame")
        void update (void);

        // returns the number of seconds of real time (represented as a float)
        // since the clock was first updated.
        float realTimeSinceFirstClockUpdate (void);

        // force simulation time ahead, ignoring passage of real time.
        // Used for OpenSteerDemo's "single step forward" and animation mode
        float advanceSimulationTimeOneFrame (void);
        void advanceSimulationTime (const float seconds);

        // "wait" until next frame time
        void frameRateSync (void);


        // main clock modes: variable or fixed frame rate, real-time or animation
        // mode, running or paused.
    private:
        // run as fast as possible, simulation time is based on real time
        bool variableFrameRateMode;

        // fixed frame rate (ignored when in variable frame rate mode) in
        // real-time mode this is a "target", in animation mode it is absolute
        int fixedFrameRate;

        // used for offline, non-real-time applications
        bool animationMode;

        // is simulation running or paused?
        bool paused;
    public:
        int getFixedFrameRate (void) {return fixedFrameRate;}
        int setFixedFrameRate (int ffr) {return fixedFrameRate = ffr;}

        bool getAnimationMode (void) {return animationMode;}
        bool setAnimationMode (bool am) {return animationMode = am;}

        bool getVariableFrameRateMode (void) {return variableFrameRateMode;}
        bool setVariableFrameRateMode (bool vfrm)
             {return variableFrameRateMode = vfrm;}

        bool togglePausedState (void) {return (paused = !paused);};
        bool getPausedState (void) {return paused;};
        bool setPausedState (bool newPS) {return paused = newPS;};


        // clock keeps track of "smoothed" running average of recent frame rates.
        // When a fixed frame rate is used, a running average of "CPU load" is
        // kept (aka "non-wait time", the percentage of each frame time (time
        // step) that the CPU is busy).
    private:
        float smoothedFPS;
        float smoothedUsage;
        void updateSmoothedRegisters (void)
        {
            const float rate = getSmoothingRate ();
            if (elapsedRealTime > 0)
                blendIntoAccumulator (rate, 1 / elapsedRealTime, smoothedFPS);
            if (! getVariableFrameRateMode ())
                blendIntoAccumulator (rate, getUsage (), smoothedUsage);
        }
    public:
        float getSmoothedFPS (void) const {return smoothedFPS;}
        float getSmoothedUsage (void) const {return smoothedUsage;}
        float getSmoothingRate (void) const
        {
            if (smoothedFPS == 0) return 1; else return elapsedRealTime * 1.5f;
        }
        float getUsage (void)
        {
            // run time per frame over target frame time (as a percentage)
            return ((100 * elapsedNonWaitRealTime) / (1.0f / fixedFrameRate));
        }


        // clock state member variables and public accessors for them
    private:
        // real "wall clock" time since launch
        float totalRealTime;

        // total time simulation has run
        float totalSimulationTime;

        // total time spent paused
        float totalPausedTime;

        // sum of (non-realtime driven) advances to simulation time
        float totalAdvanceTime;

        // interval since last simulation time
        // (xxx does this need to be stored in the instance? xxx)
        float elapsedSimulationTime;

        // interval since last clock update time 
        // (xxx does this need to be stored in the instance? xxx)
        float elapsedRealTime;

        // interval since last clock update,
        // exclusive of time spent waiting for frame boundary when targetFPS>0
        float elapsedNonWaitRealTime;
    public:
        float getTotalRealTime (void) {return totalRealTime;}
        float getTotalSimulationTime (void) {return totalSimulationTime;}
        float getTotalPausedTime (void) {return totalPausedTime;}
        float getTotalAdvanceTime (void) {return totalAdvanceTime;}
        float getElapsedSimulationTime (void) {return elapsedSimulationTime;}
        float getElapsedRealTime (void) {return elapsedRealTime;}
        float getElapsedNonWaitRealTime (void) {return elapsedNonWaitRealTime;}


    private:
        // "manually" advance clock by this amount on next update
        float newAdvanceTime;

        // "Calendar time" when this clock was first updated
    #ifdef _WIN32
        // from QueryPerformanceCounter on Windows
        LONGLONG basePerformanceCounter;
    #else
        // from gettimeofday on Linux and Mac OS X
        int baseRealTimeSec;
        int baseRealTimeUsec;
    #endif
    };

} // namespace OpenSteer


// ----------------------------------------------------------------------------
#endif // OPENSTEER_CLOCK_H
