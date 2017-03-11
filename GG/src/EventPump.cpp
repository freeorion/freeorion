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
    last_FPS_time(std::chrono::high_resolution_clock::now()),
    last_frame_time(std::chrono::high_resolution_clock::now()),
    most_recent_time(std::chrono::high_resolution_clock::now()),
    frames(0)
{}


void EventPumpBase::LoopBody(GUI* gui, EventPumpState& state, bool do_non_rendering, bool do_rendering)
{
    if (do_non_rendering) {
        std::chrono::high_resolution_clock::time_point time = std::chrono::high_resolution_clock::now();

        // send an idle message, so that the gui has timely updates for triggering browse info windows, etc.
        gui->HandleGGEvent(GUI::IDLE, GGK_UNKNOWN, 0, gui->ModKeys(), gui->MousePosition(), Pt());

        // govern FPS speed if needed
        if (double max_FPS = gui->MaxFPS()) {
            std::chrono::microseconds min_us_per_frame = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::duration<double>(1.0 / (max_FPS + 1)));
            std::chrono::microseconds us_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                time - state.last_frame_time);
            std::chrono::microseconds us_to_wait = (min_us_per_frame - us_elapsed);
            if (std::chrono::microseconds(0) < us_to_wait) {
                gui->Wait(us_to_wait);
                time = std::chrono::high_resolution_clock::now();
            }
        }
        state.last_frame_time = time;

        // track FPS if needed
        gui->SetDeltaT(std::chrono::duration_cast<std::chrono::microseconds>(time - state.most_recent_time).count());
        if (gui->FPSEnabled()) {
            ++state.frames;
            if (std::chrono::seconds(1) < time - state.last_FPS_time) { // calculate FPS at most once a second
                double time_since_last_FPS = std::chrono::duration_cast<std::chrono::microseconds>(
                    time - state.last_FPS_time).count() / 1000000.0;
                gui->SetFPS(state.frames / time_since_last_FPS);
                state.last_FPS_time = time;
                state.frames = 0;
            }
        }
        state.most_recent_time = time;
    }

    if (do_rendering) {
        // do one iteration of the render loop
        gui->PreRender();
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
