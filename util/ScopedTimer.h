// -*- C++ -*-
#ifndef _ScopedTimer_h_
#define _ScopedTimer_h_

#include <string>

#include "Export.h"

/** Wrapper for boost::timer that outputs time during which this object
  * existed.  Created in the scope of a function, and passed the appropriate
  * name, it will output to DebugLogger() the time elapsed while
  * the function was executing. */
class FO_COMMON_API ScopedTimer {
public:
    ScopedTimer(const std::string& timed_name, bool always_output = false);
    ~ScopedTimer();
private:
    class ScopedTimerImpl;
    ScopedTimerImpl*    m_impl;
};

#endif // _MultiplayerCommon_h_
