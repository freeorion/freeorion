/* GG is a GUI for SDL and OpenGL.
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

#include <GG/EventPump.h>

#include <GG/WndEvent.h>


using namespace GG;

EventPumpState::EventPumpState() :
    last_FPS_time(0),
    last_frame_time(0),
    most_recent_time(0),
    frames(0)
{}


void EventPumpBase::LoopBody(GUI* gui, EventPumpState& state, bool do_non_rendering, bool do_rendering)
{
    if (do_non_rendering) {
        int time = gui->Ticks();

        // send an idle message, so that the gui has timely updates for triggering browse info windows, etc.
        gui->HandleGGEvent(GUI::IDLE, GGK_UNKNOWN, 0, gui->ModKeys(), gui->MousePosition(), Pt());

        // govern FPS speed if needed
        if (double max_FPS = gui->MaxFPS()) {
            double min_ms_per_frame = 1000.0 / max_FPS;
            double ms_to_wait = min_ms_per_frame - (time - state.last_frame_time);
            if (0.0 < ms_to_wait)
                gui->Wait(static_cast<unsigned int>(ms_to_wait));
        }
        state.last_frame_time = time;

        // track FPS if needed
        gui->SetDeltaT(time - state.most_recent_time);
        if (gui->FPSEnabled()) {
            ++state.frames;
            if (1000 < time - state.last_FPS_time) { // calculate FPS at most once a second
                gui->SetFPS(state.frames / ((time - state.last_FPS_time) / 1000.0));
                state.last_FPS_time = time;
                state.frames = 0;
            }
        }
        state.most_recent_time = time;
    }

    if (do_rendering) {
        // do one iteration of the render loop
        gui->RenderBegin();
        gui->Render();
        gui->RenderEnd();
    }
}

EventPumpState& EventPumpBase::State()
{
    static EventPumpState state;
    return state;
}


void EventPump::operator()()
{
    GUI* gui = GUI::GetGUI();
    EventPumpState& state = State();
    while (1) {
        gui->HandleSystemEvents();
        LoopBody(gui, state, true, true);
    }
}


ModalEventPump::ModalEventPump(const bool& done) :
    m_done(done)
{}

void ModalEventPump::operator()()
{
    GUI* gui = GUI::GetGUI();
    EventPumpState& state = State();
    while (!Done()) {
        gui->HandleSystemEvents();
        LoopBody(gui, state, true, true);
    }
}

bool ModalEventPump::Done () const
{ return m_done; }
