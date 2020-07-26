//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

/** \file Timer.h \brief Contains the Timer class, which allows Wnds to
    receive regular notifications of the passage of time. */

#ifndef _GG_Timer_h_
#define _GG_Timer_h_

#include <GG/Base.h>

#include <boost/signals2/signal.hpp>

#include <set>


namespace GG {

class Wnd;

/** \brief Timer provides a means for one or more Wnds to receive periodic
    notifications of the passage of time.

    The rate at which the Timer fires is not realtime.  That is, there are no
    guarantees on the interval between firings other than that a minimum of
    Interval() ms will have elapsed. */
class GG_API Timer
{
public:
    /** Emitted when the timer fires */
    typedef boost::signals2::signal<void (unsigned int, Timer*)> FiredSignalType;

    /** Basic ctor.  Takes an interval and a start time in ms; if the start
        time is ommitted, the start time will be immediate. */
    explicit Timer(unsigned int interval, unsigned int start_time = 0);

    ~Timer();

    unsigned int Interval() const;      ///< Returns the interval in ms between firings of the timer
    bool Running() const;               ///< Returns true iff the timer is operating.  When false, this indicates that no firings will occur until Start() is called.

    void Reset(unsigned int start_time = 0); ///< Resets the last-firing time of the timer to \a start_time (in ms), or the current time if \a start_time is ommitted.
    void SetInterval(unsigned int interval); ///< Sets the interval in ms between firings of the timer
    void Connect(Wnd* wnd);         ///< Connects this timer to \a wnd, meaning that \a wnd will be notified when the timer fires.
    void Disconnect(Wnd* wnd);      ///< Disconnects this timer from \a wnd.
    void Start();                   ///< Starts the timer firing; does not reset the timer.
    void Stop();                    ///< Stops the timer firing until Start() is called.
    void Update(unsigned int ticks); ///< Signals listeners iff the timer is running and the last time it fired is is more than Interval() ms ago.

    mutable FiredSignalType FiredSignal; ///< The fired signal object for this Timer

private:
    typedef std::map<Wnd*, boost::signals2::connection> ConnectionMap;

    Timer();
    Timer(const Timer&); // disabled

    ConnectionMap  m_wnd_connections;
    unsigned int   m_interval;
    bool           m_running;
    unsigned int   m_last_fire;
};

}

#endif
