#ifndef _ScopedTimer_h_
#define _ScopedTimer_h_

#include <memory>
#include <string>

#include "Export.h"

#include <chrono>


//! Logs period during which this object existed.
//!
//! Created in the scope of a function, and passed the appropriate name, it will
//! output to DebugLogger() the time elapsed while the function was executing.
//!
//! If @p enable_output is true and duration is greater than threshold then
//! print output.
class FO_COMMON_API ScopedTimer {
public:
    ScopedTimer(const std::string& timed_name, bool enable_output = false,
                std::chrono::microseconds threshold = std::chrono::milliseconds(1));
    ScopedTimer(const std::string& timed_name, std::chrono::microseconds threshold);
    ~ScopedTimer();

    class Impl;

private:
    std::unique_ptr<Impl> const m_impl;
};


//! Logs period during which a this object or named sub sections existed.
//!
//! Similar to ScopedTimer SectionedScopedTimer times the duration of its own
//! existence.  It also allows the creation of sub section timers.  Timers are
//! created by calling EnterSection().
//!
//! Only one sub-section timer is active at any time.
//!
//! Timers can be re-entered, which is effective to profile looping structures.
//!
//! Any name is valid except an empty string.  Calling EnterSection("") will
//! stop the currently running sub section timer.  The total time will still
//! keep running.
//!
//! When SectionedScopedTimer goes out of scope it will print a table of its
//! elapsed time and the sub sections.
//!
//! It only prints times if the time is greater than the threshold.  Each
//! section is only printed if its time is greater than the threshold.  The
//! whole table is only printed if the total time is greater than the threshold.
//!
//! The following is a usage example.  It shows how to create timer with a
//! section before a loop, two sections in a loop, a section after a loop and a
//! section that will not be timed.
//!
//! ```{.cpp}
//! void function_to_profile () {
//!
//!     SectionedScopedTimer timer("Title", std::chrono::milliseconds(1));
//!     timer.EnterSection("initial section");
//!
//!     profiled_code();
//!
//!     for () {
//!         timer.EnterSection("for loop 1 section");
//!
//!         more_code();
//!
//!         timer.EnterSection("for loop 2 section");
//!
//!         even_more_code();
//!     }
//!
//!     timer.EnterSection(""); // don't time this section
//!
//!     untimed_code();
//!
//!     timer.EnterSection("final section");
//!
//!     final_timed_code()
//! }
//! ```
//!
//! The tables of times will look like the following:
//!
//! ```{.txt}
//! Title - initial section    time: xxxx ms
//! Title - for loop 1 section time: xxxx ms
//! Title - for loop 2 section time: xxxx ms
//! Title - final section      time: xxxx ms
//! Title                      time: xxxx ms
//! ```
class FO_COMMON_API SectionedScopedTimer {
public:
    SectionedScopedTimer(const std::string& timed_name, bool enable_output = false,
                         std::chrono::microseconds threshold = std::chrono::milliseconds(1));
    SectionedScopedTimer(const std::string& timed_name, std::chrono::microseconds threshold);
    ~SectionedScopedTimer();

    //! Start recording times for @p section_name.
    //!
    //! This can be called multiple times to add more time to a previously
    //! created section.  Use this feature to profile separate parts of loop
    //! strucures.
    void EnterSection(const std::string& section_name);

private:
    class Impl;

    std::unique_ptr<Impl> const m_impl;
};

#endif // _ScopedTimer_h_
