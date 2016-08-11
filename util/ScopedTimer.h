#ifndef _ScopedTimer_h_
#define _ScopedTimer_h_

#include <string>

#include "Export.h"

// TODO change boost to std when C++11 is adopted.
#include <boost/chrono/chrono.hpp>
#include <boost/scoped_ptr.hpp>

/** Wrapper for boost::timer that outputs time during which this object
    existed.  Created in the scope of a function, and passed the appropriate
    name, it will output to DebugLogger() the time elapsed while
    the function was executing.

    If \p enable_output is true and duration is greater than threshold then
    print output.
*/

class FO_COMMON_API ScopedTimer {
public:
    ScopedTimer(const std::string& timed_name, bool enable_output = false,
                boost::chrono::microseconds threshold = boost::chrono::milliseconds(1));
    ScopedTimer(const std::string& timed_name, boost::chrono::microseconds threshold);
    ~ScopedTimer();
private:
    class ScopedTimerImpl;
    // TODO use C++11 unique_ptr
    boost::scoped_ptr<ScopedTimerImpl> const pimpl;
};

#endif // _MultiplayerCommon_h_
