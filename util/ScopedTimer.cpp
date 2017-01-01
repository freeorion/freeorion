#include "ScopedTimer.h"

#include "OptionsDB.h"
#include "Logger.h"

#include <boost/chrono/io/duration_io.hpp>
#include <boost/unordered_map.hpp>

#include <iomanip>

class ScopedTimer::ScopedTimerImpl {
public:
    ScopedTimerImpl(const std::string& timed_name, bool enable_output,
                    boost::chrono::microseconds threshold) :
        m_start(boost::chrono::high_resolution_clock::now()),
        m_name(timed_name),
        m_enable_output(enable_output),
        m_threshold(threshold)
    {}

    ~ScopedTimerImpl() {
        boost::chrono::nanoseconds duration = boost::chrono::high_resolution_clock::now() - m_start;
        if (!ShouldOutput(duration))
            return;

        std::stringstream ss;
        ss << m_name << " time: ";
        FormatDuration(ss, duration);
        DebugLogger() << ss.str();
    }

    bool ShouldOutput(const boost::chrono::nanoseconds & duration) {
        return (duration >= m_threshold && (m_enable_output || GetOptionsDB().Get<bool>("verbose-logging")));
    }

    void FormatDuration(std::stringstream& ss, const boost::chrono::nanoseconds & duration) {
        ss << boost::chrono::symbol_format << std::setw(8) << std::right;
        if (duration >= boost::chrono::milliseconds(10))
            ss << boost::chrono::duration_cast<boost::chrono::milliseconds>(duration);
        else if (duration >= boost::chrono::microseconds(10))
            ss << boost::chrono::duration_cast<boost::chrono::microseconds>(duration);
        else
            ss << boost::chrono::duration_cast<boost::chrono::nanoseconds>(duration);
    }

    boost::chrono::high_resolution_clock::time_point m_start;
    std::string                                      m_name;
    bool                                             m_enable_output;
    boost::chrono::microseconds                      m_threshold;
};

ScopedTimer::ScopedTimer(const std::string& timed_name, bool enable_output,
                         boost::chrono::microseconds threshold) :
    pimpl(new ScopedTimerImpl(timed_name, enable_output, threshold))
{}

ScopedTimer::ScopedTimer(const std::string& timed_name, boost::chrono::microseconds threshold) :
    pimpl(new ScopedTimerImpl(timed_name, true, threshold))
{}

// ~ScopedTimer is required because Impl is defined here.
ScopedTimer::~ScopedTimer()
{}



class SectionedScopedTimer::SectionedScopedTimerImpl : public ScopedTimer::ScopedTimerImpl {

    /** Sections store a time and a duration for each section of the elapsed time report.*/
    struct Sections {
        Sections(const boost::chrono::high_resolution_clock::time_point &now,
                 const boost::chrono::nanoseconds& time_from_start) :
            m_table(), m_section_start(now), m_curr(), m_section_names()
        {
            // Create a dummy "" section so that m_curr is always a valid iterator.
            std::pair<SectionTable::iterator, bool> curr = m_table.insert(std::make_pair("", time_from_start));
            m_curr = curr.first;
        }

        /** Add time to the current section and then setup the new section. */
        void Accumulate(const boost::chrono::high_resolution_clock::time_point &now,
                        const std::string & section_name)
        {
            if (m_curr->first == section_name)
                return;

            m_curr->second += (now - m_section_start);

            m_section_start = now;

            // Create a new section if needed and update m_curr.
            std::pair<SectionTable::iterator, bool> maybe_new = m_table.insert(
                std::make_pair(section_name,
                               boost::chrono::high_resolution_clock::duration::zero()));
            m_curr = maybe_new.first;

            // Insert succeed, so grab the new section name.
            if (maybe_new.second)
                m_section_names.push_back(section_name);

        }

        //Table of section durations
        typedef boost::unordered_map<std::string, boost::chrono::nanoseconds> SectionTable;
        SectionTable m_table;

        // Currently running section start time and iterator
        boost::chrono::high_resolution_clock::time_point m_section_start;

        // m_curr always points to the section currently accumulating
        // time or to the dummy "" blank section.
        SectionTable::iterator m_curr;

        // Names of the sections in order or creation.
        std::vector<std::string> m_section_names;

    };

public:
    SectionedScopedTimerImpl(const std::string& timed_name, bool enable_output,
                             boost::chrono::microseconds threshold) :
        ScopedTimerImpl(timed_name, enable_output, threshold)
    {}

    /** The destructor will print the table of accumulated times. */
    ~SectionedScopedTimerImpl() {
        boost::chrono::nanoseconds duration = boost::chrono::high_resolution_clock::now() - m_start;

        if (!ShouldOutput(duration))
            return;

        // No table so use basic ScopedTimer output.
        if (!m_sections) {
            std::stringstream ss;
            ss << m_name << " time: ";
            ScopedTimerImpl::FormatDuration(ss, duration);
            DebugLogger() << ss.str();
            return;
        }

        //Stop the final section.
        EnterSection("");

        //Print the section times followed by the total time elapsed.

        // Find the longest name to right align the times.
        size_t longest_section_name(0);
        for (const std::string& section_name : m_sections->m_section_names) {
            longest_section_name = std::max(longest_section_name, section_name.size());
        }

        for (const std::string& section_name : m_sections->m_section_names) {
            Sections::SectionTable::const_iterator jt = m_sections->m_table.find(section_name);
            if (jt == m_sections->m_table.end()) {
                ErrorLogger() << "Missing section " << section_name << " in section table.";
                continue;
            }

            if (!ShouldOutput(jt->second))
                continue;

            // Create a header with padding, so all times align.
            std::stringstream header, tail;
            FormatDuration(tail, jt->second);
            header << m_name << " - "
                   << std::setw(longest_section_name) << std::left << section_name
                   << std::right << " time: "
                   << tail.str();
            DebugLogger() << header.str();
        }

        // Create a header with padding, so all times align.
        std::stringstream header, tail;
        FormatDuration(tail, duration);
        header << m_name
               << std::setw(longest_section_name + 3 + 7)
               << std::right << " time: "
               << tail.str();
        DebugLogger() << header.str();

        // Prevent the base class from outputting a duplicate total time.
        m_enable_output = false;
    }

    void EnterSection(const std::string& section_name) {
        boost::chrono::high_resolution_clock::time_point now(boost::chrono::high_resolution_clock::now());

        // One time initialization.
        if (!m_sections)
            CreateSections(now);

        m_sections->Accumulate(now, section_name);
    }

    /** CreateSections allow m_sections to only be initialized if it is used.*/
    Sections* CreateSections(const boost::chrono::high_resolution_clock::time_point &now) {
        m_sections.reset(new Sections(now, now - m_start));
        return m_sections.get();
    }

    // Pointer to table of sections.
    // Sections are only allocated when the first section is created, to minimize overhead of a
    // section-less timer.
    boost::scoped_ptr<Sections> m_sections;

};

SectionedScopedTimer::SectionedScopedTimer(const std::string& timed_name, bool enable_output,
                         boost::chrono::microseconds threshold) :
    pimpl(new SectionedScopedTimerImpl(timed_name, enable_output, threshold))
{}

SectionedScopedTimer::SectionedScopedTimer(const std::string& timed_name, boost::chrono::microseconds threshold) :
    pimpl(new SectionedScopedTimerImpl(timed_name, true, threshold))
{}

// ~SectionedScopedTimer is required because Impl is defined here.
SectionedScopedTimer::~SectionedScopedTimer()
{}

void SectionedScopedTimer::EnterSection(const std::string& section_name)
{ pimpl->EnterSection(section_name);}
