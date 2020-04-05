// -*- C++ -*-
/* GG is a GUI for OpenGL.
   Copyright (C) 2003-2008 T. Zachary Laine

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1
   of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
    
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA

   If you do not wish to comply with the terms of the LGPL please
   contact the author as other terms are available for a fee.
    
   Zach Laine
   whatwasthataddress@gmail.com */
   
//! @file EventPump.h
//! @brief  Contains the ModalEventPump class and its helper classes.
//! A ModalEventPump encapsulates the behavior of a GG processing loop, such as
//! the one associated with the singleton GUI object, or one associated with
//! a modal Wnd.

#ifndef _GG_EventPump_h_
#define _GG_EventPump_h_

#include <GG/GUI.h>

#include <chrono>


namespace GG {

/** \brief Encapsulates the state of GG event pumping.

    A single state object is shared by all EventPumps, to ensure state
    consistency. */
struct GG_API EventPumpState
{
    EventPumpState();

    std::chrono::high_resolution_clock::time_point last_FPS_time;    ///< The last time an FPS calculation was done.
    std::chrono::high_resolution_clock::time_point last_frame_time;  ///< The time of the last frame rendered.
    std::chrono::high_resolution_clock::time_point most_recent_time; ///< The time recorded on the previous iteration of the event pump loop.
    std::size_t  frames;           ///< The number of frames rendered since \a last_frame_time.
};

//! An EventPump that terminates when the bool reference @a done supplied to
//! the constructor is true.
//!
//! @note Modal Wnds use EventPumps to implement their modality.
class GG_API ModalEventPump
{
public:
    ModalEventPump(const bool& done);

    void operator()();

protected:
    //! Returns true iff the constructor parameter @a done is true.
    bool Done() const;

    //! Executes everything but the system event handling portion of the event
    //! handling and rendering cycle.
    void LoopBody(GUI* gui, EventPumpState& state);

    //! Returns the EventPumpState object shared by all event pump types.
    static EventPumpState& State();

    const bool& m_done;
};

} // namespace GG

#endif
