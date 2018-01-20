/* GG is a GUI for SDL and OpenGL.

   Copyright (C) 2006, 2008-2009 T. Zachary Laine

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

#include <GG/Timer.h>

#include <GG/GUI.h>
#include <GG/Wnd.h>

#include <map>


using namespace GG;

namespace {
    void FiredSignalEcho(unsigned int ticks, Timer* timer)
    {
        std::cerr << "GG SIGNAL : Timer::FiredSignal(ticks=" << ticks
                  << ", timer=" << timer << ")\n";
    }
}

Timer::Timer() :
    m_interval(0),
    m_running(true),
    m_last_fire(0)
{
    GUI::GetGUI()->RegisterTimer(*this);
    if (INSTRUMENT_ALL_SIGNALS)
        FiredSignal.connect(&FiredSignalEcho);
}

Timer::Timer(unsigned int interval, unsigned int start_time/* = 0*/) :
    m_interval(interval),
    m_running(true),
    m_last_fire(start_time ? start_time : GUI::GetGUI()->Ticks())
{
    GUI::GetGUI()->RegisterTimer(*this);
    if (INSTRUMENT_ALL_SIGNALS)
        FiredSignal.connect(&FiredSignalEcho);
}

Timer::~Timer()
{
    if (auto gui = GUI::GetGUI())
        gui->RemoveTimer(*this);
}

unsigned int Timer::Interval() const
{ return m_interval; }

bool Timer::Running() const
{ return m_running; }

void Timer::Reset(unsigned int start_time/* = 0*/)
{ m_last_fire = start_time ? start_time : GUI::GetGUI()->Ticks(); }

void Timer::SetInterval(unsigned int interval)
{ m_interval = interval; }

void Timer::Connect(Wnd* wnd)
{
    Disconnect(wnd);
    m_wnd_connections[wnd] = FiredSignal.connect(
        boost::bind(&Wnd::TimerFiring, wnd, _1, _2));
}

void Timer::Disconnect(Wnd* wnd)
{
    ConnectionMap::iterator it = m_wnd_connections.find(wnd);
    if (it != m_wnd_connections.end()) {
        it->second.disconnect();
        m_wnd_connections.erase(it);
    }
}

void Timer::Start()
{ m_running = true; }

void Timer::Stop()
{ m_running = false; }

void Timer::Update(unsigned int ticks)
{
    if (m_running && m_interval < ticks - m_last_fire)
        FiredSignal(ticks, this);
}
