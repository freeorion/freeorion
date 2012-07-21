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
        ::GG::Connect(FiredSignal, &FiredSignalEcho);
}

Timer::Timer(unsigned int interval, unsigned int start_time/* = 0*/) :
    m_interval(interval),
    m_running(true),
    m_last_fire(start_time ? start_time : GUI::GetGUI()->Ticks())
{
    GUI::GetGUI()->RegisterTimer(*this);
    if (INSTRUMENT_ALL_SIGNALS)
        ::GG::Connect(FiredSignal, &FiredSignalEcho);
}

Timer::~Timer()
{ GUI::GetGUI()->RemoveTimer(*this); }

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
    m_wnd_connections[wnd] = GG::Connect(FiredSignal, &Wnd::TimerFiring, wnd);
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
