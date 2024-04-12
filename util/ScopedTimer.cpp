#include "ScopedTimer.h"

#include "Logger.h"

#include <boost/chrono.hpp>
#include <boost/unordered_map.hpp>

#include <iomanip>
#include <sstream>

namespace {
    DeclareThreadSafeLogger(timer);
}

class ScopedTimer::Impl {
public:
    std::chrono::high_resolution_clock::time_point m_start;
    std::string                                    m_name;
    std::function<std::string ()>                  m_output_text_fn;
    bool                                           m_enable_output;
    std::chrono::microseconds                      m_threshold;

    static constexpr bool start_now_noexcept = noexcept(decltype(m_start){std::chrono::high_resolution_clock::now()});
    static constexpr bool elapsed_noexcept = noexcept(std::chrono::high_resolution_clock::now() - m_start);
    static constexpr bool elapsed_count_noexcept = elapsed_noexcept &&
        noexcept(std::declval<std::chrono::nanoseconds>().count());

    Impl(std::string timed_name, bool enable_output, std::chrono::microseconds threshold)
        noexcept(start_now_noexcept) :
        m_start(std::chrono::high_resolution_clock::now()),
        m_name(std::move(timed_name)),
        m_enable_output(enable_output),
        m_threshold(threshold)
    {
        static_assert(noexcept(decltype(m_name){std::move(timed_name)}));
        static_assert(noexcept(decltype(m_threshold){threshold}));
    }

    Impl(std::function<std::string ()> output_text_fn, bool enable_output,
         std::chrono::microseconds threshold) noexcept(start_now_noexcept) :
        m_start(std::chrono::high_resolution_clock::now()),
        m_output_text_fn(std::move(output_text_fn)),
        m_enable_output(enable_output),
        m_threshold(threshold)
    {
        static_assert(noexcept(decltype(m_output_text_fn){std::move(output_text_fn)}));
    }

    ~Impl() noexcept {
        if (!m_enable_output)
            return;

        try {
            const auto elapsed = Elapsed();

            if (elapsed < m_threshold)
                return;

            std::stringstream ss;
            if (!m_name.empty())
                ss << m_name << " time: ";
            else if (m_output_text_fn)
                ss << m_output_text_fn() << " time: ";
            else
                ss << "time: ";
            FormatDuration(ss, elapsed);
            DebugLogger(timer) << ss.str();
        } catch (...) {}
    }

    void Restart() noexcept(noexcept(decltype(m_start){} = std::chrono::high_resolution_clock::now()))
    { m_start = std::chrono::high_resolution_clock::now(); }

    std::chrono::nanoseconds Elapsed() const noexcept(elapsed_noexcept)
    { return std::chrono::high_resolution_clock::now() - m_start; }

    template <typename UNITS = std::chrono::seconds>
    double Duration() const noexcept(elapsed_count_noexcept) {
        using nsp = std::chrono::nanoseconds::period;
        using up = typename UNITS::period;
        constexpr auto scale = nsp::den / nsp::num * up::num / up::den;
        const auto elapsed = Elapsed().count();
        static_assert(noexcept(elapsed / scale));
        return static_cast<double>(elapsed / scale);
    }

    std::string DurationString() const {
        std::stringstream ss;
        FormatDuration(ss, Elapsed());
        return ss.str();
    }

    template <typename UNITS = std::chrono::milliseconds>
    static void FormatDurationFixedUnits(std::stringstream& ss, const std::chrono::nanoseconds& duration) {
        ss << std::setw(8) << std::right << std::chrono::duration_cast<UNITS>(duration).count();
        if constexpr (std::is_same<UNITS, std::chrono::seconds>())
            ss  << " s";
        else if constexpr (std::is_same<UNITS, std::chrono::milliseconds>())
            ss << " ms";
        else if constexpr (std::is_same<UNITS, std::chrono::microseconds>())
            ss << " µs";
        else if constexpr (std::is_same<UNITS, std::chrono::nanoseconds>())
            ss << " ns";
    }

    static void FormatDuration(std::stringstream& ss, const std::chrono::nanoseconds& duration) {
        ss << std::setw(8) << std::right;
        if (duration >= std::chrono::seconds(10)) {
            ss << std::chrono::duration_cast<std::chrono::seconds>(duration).count() << " s";

        } else if (duration >= std::chrono::seconds(10)) {
            const auto ms{std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()};
            ss << static_cast<double>(ms / 100) / 10.0 << " s";    // round to 10ths of seconds

        } else if (duration >= std::chrono::milliseconds(100)) {
            ss << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << " ms";

        } else if (duration >= std::chrono::milliseconds(10)) {
            const auto ms{std::chrono::duration_cast<std::chrono::microseconds>(duration).count()};
            ss << static_cast<double>(ms / 100) / 10.0 << " ms";    // round to 10ths of milliseconds

        } else if (duration >= std::chrono::microseconds(100)) {
            ss << std::chrono::duration_cast<std::chrono::microseconds>(duration).count() << " µs";

        } else if (duration >= std::chrono::microseconds(10)) {
            const auto ns{std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count()};
            ss << static_cast<double>(ns / 100) / 10.0 << " µs";    // round to 10ths of microseconds

        } else {
            ss << std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() << " ns";
        }
    }
};

ScopedTimer::ScopedTimer() :
    ScopedTimer("", false)
{}

ScopedTimer::ScopedTimer(std::string timed_name, bool enable_output,
                         std::chrono::microseconds threshold) :
    m_impl(std::make_unique<Impl>(std::move(timed_name), enable_output, threshold))
{}

ScopedTimer::ScopedTimer(std::string timed_name, std::chrono::microseconds threshold) :
    ScopedTimer(std::move(timed_name), true, threshold)
{}

ScopedTimer::ScopedTimer(std::function<std::string ()> output_text_fn,
                         std::chrono::microseconds threshold) :
    m_impl(std::make_unique<Impl>(std::move(output_text_fn), true, threshold))
{}


//! @note
//!     ~ScopedTimer is required because Impl is defined here.
ScopedTimer::~ScopedTimer() = default;

void ScopedTimer::restart() noexcept
{ m_impl->Restart(); }

double ScopedTimer::duration() const noexcept
{ return m_impl->Duration(); }

std::string ScopedTimer::DurationString() const
{ return m_impl->DurationString(); }

std::chrono::nanoseconds ScopedTimer::Elapsed() const noexcept
{ return m_impl->Elapsed(); }


class SectionedScopedTimer::Impl : public ScopedTimer::Impl {
    /** Sections store a time and a duration for each section of the elapsed time report.*/
    struct Sections {
        Sections(std::chrono::high_resolution_clock::time_point now,
                 std::chrono::nanoseconds time_from_start) :
            m_table{{"", time_from_start}}, // Create a dummy "" section so that m_curr is always a valid iterator.
            m_section_start(now),
            m_curr{m_table.begin()}
        {}

        /** Add time to the current section and then setup the new section. */
        void Accumulate(std::chrono::high_resolution_clock::time_point now,
                        const std::string& section_name)
        {
            if (m_curr->first == section_name)
                return;

            m_curr->second += (now - m_section_start);

            m_section_start = now;

            // Create a new section if needed and update m_curr.
            bool is_new_section = false;
            std::tie(m_curr, is_new_section) = m_table.try_emplace(
                section_name, std::chrono::high_resolution_clock::duration::zero());

            // Insert succeed, so store the new section name at end of list
            if (is_new_section)
                m_section_names.push_back(section_name);
        }

        //Table of section durations
        using SectionTable = boost::unordered_map<std::string, std::chrono::nanoseconds> ;
        SectionTable m_table;

        // Currently running section start time
        std::chrono::high_resolution_clock::time_point m_section_start;

        // m_curr always points to the section currently accumulating
        // time or to the dummy "" blank section.
        SectionTable::iterator m_curr;

        // Names of the sections in order or creation.
        std::vector<std::string> m_section_names;
    };

    /** CreateSections allow m_sections to only be initialized if it is used.*/
    void CreateSections(std::chrono::high_resolution_clock::time_point now)
    { m_sections = std::make_unique<Sections>(now, now - m_start); }

public:
    Impl(std::string timed_name, std::chrono::microseconds threshold,
         bool enable_output, bool unify_section_duration_units)
        noexcept(noexcept(ScopedTimer::Impl(std::declval<std::string>(), true, std::declval<std::chrono::microseconds>()))) :
        ScopedTimer::Impl(std::move(timed_name), enable_output, threshold),
        m_unify_units(unify_section_duration_units)
    {}

    /** The destructor will print the table of accumulated times. */
    ~Impl() {
        if (!m_enable_output || !m_sections)
            return;

        std::chrono::nanoseconds duration = std::chrono::high_resolution_clock::now() - m_start;

        if (duration < m_threshold)
            return;

        EnterSection("");   // Stop the final section

        if (m_sections->m_section_names.size() == 1 && m_sections->m_section_names.begin()->empty())
            return; // Don't print the table if the only section is the default section


        //Print the section times followed by the total time elapsed.

        // Find the longest name to right align the times and longest time to align the units
        std::size_t longest_section_name(0);
        std::chrono::nanoseconds longest_section_duration(0);
        for (const auto& [sec_name, sec_dur] : m_sections->m_table) {
            longest_section_name = std::max(longest_section_name, sec_name.size());
            longest_section_duration = std::max(longest_section_duration, sec_dur);
        }

        // Output section names and times in order they were created
        for (const std::string& section_name : m_sections->m_section_names) {
            auto jt = m_sections->m_table.find(section_name);
            if (jt == m_sections->m_table.end()) {
                ErrorLogger(timer) << "Missing section " << section_name << " in section table.";
                continue;
            }

            // is duration yet long enough to output?
            if (jt->second < m_threshold)
                continue;

            // Create a header with padding, so all times align.
            std::stringstream header, tail;
            if (m_unify_units) {
                if (longest_section_duration < std::chrono::microseconds(10))
                    FormatDurationFixedUnits<std::chrono::nanoseconds>(tail, jt->second);
                else if (longest_section_duration < std::chrono::milliseconds(10))
                    FormatDurationFixedUnits<std::chrono::microseconds>(tail, jt->second);
                else if (longest_section_duration < std::chrono::seconds(10))
                    FormatDurationFixedUnits<std::chrono::milliseconds>(tail, jt->second);
                else
                    FormatDurationFixedUnits<std::chrono::seconds>(tail, jt->second);
            } else {
                FormatDuration(tail, jt->second);
            }
            header << m_name << " - "
                   << std::setw(longest_section_name) << std::left << section_name
                   << std::right << " time: "
                   << tail.str();
            DebugLogger(timer) << header.str();
        }

        // Create a header with padding, so all times align.
        std::stringstream header, tail;
        FormatDuration(tail, duration);
        header << m_name
               << std::setw(longest_section_name + 3 + 7)
               << std::right << " time: "
               << tail.str();
        DebugLogger(timer) << header.str();

        // Prevent the base class from outputting a duplicate total time.
        m_enable_output = false;
    }

    void EnterSection(const std::string& section_name) {
        auto now(std::chrono::high_resolution_clock::now());

        // One time initialization.
        if (!m_sections)
            CreateSections(now);

        m_sections->Accumulate(now, section_name);
    }

    [[nodiscard]]std::chrono::nanoseconds Elapsed() const noexcept
    { return std::chrono::high_resolution_clock::now() - m_start; }

    [[nodiscard]] const Sections::SectionTable* GetSectionTable() const noexcept
    { return m_sections ? &m_sections->m_table : nullptr; }

    [[nodiscard]]const std::vector<std::pair<std::string_view, std::chrono::nanoseconds>> SectionsElapsed() const {
        if (!m_sections)
            return {};
        const auto& t = m_sections->m_table;
        return {t.begin(), t.end()};
    }

private:
    // Pointer to table of sections.
    // Sections are only allocated when the first section is created, to minimize overhead of a
    // section-less timer.
    std::unique_ptr<Sections> m_sections;

    bool m_unify_units = false;
};

SectionedScopedTimer::SectionedScopedTimer(std::string timed_name,
                                           std::chrono::microseconds threshold) :
    m_impl(std::make_unique<Impl>(std::move(timed_name), threshold, true, true))
{}

// ~SectionedScopedTimer is required because Impl is defined here.
SectionedScopedTimer::~SectionedScopedTimer() = default;

std::chrono::nanoseconds SectionedScopedTimer::Elapsed() const noexcept
{ return m_impl->Elapsed(); }

std::vector<std::pair<std::string_view, std::chrono::nanoseconds>> SectionedScopedTimer::SectionsElapsed() const noexcept
{ return m_impl->SectionsElapsed(); }

void SectionedScopedTimer::EnterSection(const std::string& section_name)
{ m_impl->EnterSection(section_name); }
