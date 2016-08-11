#include "ScopedTimer.h"

#include "OptionsDB.h"
#include "Logger.h"

#include <boost/chrono/io/duration_io.hpp>

class ScopedTimer::ScopedTimerImpl {
public:
    ScopedTimerImpl(const std::string& timed_name, bool enable_output) :
        m_start(boost::chrono::high_resolution_clock::now()),
        m_name(timed_name),
        m_enable_output(enable_output)
    {}
    ~ScopedTimerImpl() {
        boost::chrono::nanoseconds duration = boost::chrono::high_resolution_clock::now() - m_start;
        if (duration >= boost::chrono::milliseconds(1) && (m_enable_output || GetOptionsDB().Get<bool>("verbose-logging")))
            if (duration >= boost::chrono::milliseconds(10))
                DebugLogger() << m_name << " time: "
                              << boost::chrono::symbol_format
                              << boost::chrono::duration_cast<boost::chrono::milliseconds>(duration);
            else if (duration >= boost::chrono::microseconds(10))
                DebugLogger() << m_name << " time: "
                              << boost::chrono::symbol_format
                              << boost::chrono::duration_cast<boost::chrono::microseconds>(duration);
            else
                DebugLogger() << m_name << " time: "
                              << boost::chrono::symbol_format
                              << boost::chrono::duration_cast<boost::chrono::nanoseconds>(duration);
    }
    boost::chrono::high_resolution_clock::time_point m_start;
    std::string                                      m_name;
    bool                                             m_enable_output;
};

ScopedTimer::ScopedTimer(const std::string& timed_name, bool enable_output) :
    pimpl(new ScopedTimerImpl(timed_name, enable_output))
{}

// ~ScopedTimer is required because Impl is defined here.
ScopedTimer::~ScopedTimer()
{ }
