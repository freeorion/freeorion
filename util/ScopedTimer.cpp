#include "ScopedTimer.h"

#include "OptionsDB.h"
#include "Logger.h"

#include <boost/timer.hpp>


class ScopedTimer::ScopedTimerImpl {
public:
    ScopedTimerImpl(const std::string& timed_name, bool always_output) :
        m_timer(),
        m_name(timed_name),
        m_always_output(always_output)
    {}
    ~ScopedTimerImpl() {
        if (m_timer.elapsed() * 1000.0 > 1 && ( m_always_output || GetOptionsDB().Get<bool>("verbose-logging")))
            Logger().debugStream() << m_name << " time: " << (m_timer.elapsed() * 1000.0);
    }
    boost::timer    m_timer;
    std::string     m_name;
    bool            m_always_output;
};

ScopedTimer::ScopedTimer(const std::string& timed_name, bool always_output) :
    m_impl(new ScopedTimerImpl(timed_name, always_output))
{}

ScopedTimer::~ScopedTimer()
{ delete m_impl; }
