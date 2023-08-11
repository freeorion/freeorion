#ifndef _ScopedTimer_h_
#define _ScopedTimer_h_

#include <memory>
#include <string>
#include <functional>

#include "Export.h"

#include <chrono>
#include <unordered_map>


//! Logs period during which this object existed.
//!
//! Created in the scope of a function, and passed the appropriate name, it will
//! output to DebugLogger() the time elapsed while the function was executing.
//!
//! If @p enable_output is true and duration is greater than threshold then
//! print output.
//!
//! If enable_output is false, duration() and DurationString() can still be
//! queried to produce custom results from the output

#if defined(__GNUC__)
// GCC doesn't seem to like class [[nodiscard]] mixed with FO_COMMON_API
class FO_COMMON_API ScopedTimer {
#else
class FO_COMMON_API [[nodiscard]] ScopedTimer {
#endif
public:
    ScopedTimer(); // defaults to not logging output on destruction
    explicit ScopedTimer(std::string timed_name, bool enable_output = true,
                         std::chrono::microseconds threshold = std::chrono::milliseconds(1));
    ScopedTimer(std::string timed_name, std::chrono::microseconds threshold); // defaults to logging output on destruction
    ScopedTimer(std::function<std::string ()> output_text_fn, std::chrono::microseconds threshold); // defaults to logging output on destruction
    ~ScopedTimer();

    void restart() noexcept;

    [[nodiscard]] double duration() const noexcept;
    [[nodiscard]] std::string DurationString() const;
    [[nodiscard]] std::chrono::nanoseconds Elapsed() const noexcept;

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
//!     SectionedScopedTimer timer("Title");
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
#if defined(__GNUC__)
// GCC doesn't seem to like class [[nodiscard]] mixed with FO_COMMON_API
class FO_COMMON_API SectionedScopedTimer {
#else
class FO_COMMON_API [[nodiscard]] SectionedScopedTimer {
#endif
public:
    explicit SectionedScopedTimer(std::string timed_name,
                                  std::chrono::microseconds threshold = std::chrono::milliseconds(1));
    ~SectionedScopedTimer();

    //! Start recording times for @p section_name.
    //!
    //! This can be called multiple times to add more time to a previously
    //! created section.  Use this feature to profile separate parts of loop
    //! strucures.
    void EnterSection(const std::string& section_name);

    [[nodiscard]] std::chrono::nanoseconds Elapsed() const noexcept;
    [[nodiscard]] std::vector<std::pair<std::string_view, std::chrono::nanoseconds>> SectionsElapsed() const noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> const m_impl;
};


#endif
