#ifndef _ResearchQueue_h_
#define _ResearchQueue_h_

#include "../universe/ConstantsFwd.h"
#include "../util/Export.h"

#include <deque>
#include <map>
#include <string>
#include <vector>
#include <boost/serialization/access.hpp>
#include <boost/signals2/signal.hpp>

struct ScriptingContext;

struct FO_COMMON_API ResearchQueue {
    /** The type of a single element in the research queue. */
    struct Element {
        [[nodiscard]] std::string Dump() const;

        std::string name;
        int         empire_id = ALL_EMPIRES;
        float       allocated_rp = 0.0f;
        int         turns_left = 0;
        bool        paused = false;

    private:
        friend class boost::serialization::access;
        template <typename Archive>
        void serialize(Archive& ar, const unsigned int version);
    };

    typedef std::deque<Element> QueueType;

    /** The ResearchQueue iterator type.  Dereference yields an Element. */
    typedef QueueType::iterator iterator;
    /** The const ResearchQueue iterator type.  Dereference yields an Element. */
    typedef QueueType::const_iterator const_iterator;

    ResearchQueue() = default;
    explicit ResearchQueue(int empire_id) :
        m_empire_id(empire_id)
    {}

    [[nodiscard]] bool                     InQueue(const std::string& tech_name) const;///< Returns true iff \a tech is in this queue.
    [[nodiscard]] bool                     Paused(const std::string& tech_name) const; ///< Returns true iff \a tech is in this queue and paused.
    [[nodiscard]] bool                     Paused(int idx) const;                      ///< Returns true iff there are at least \a idx - 1 items in the queue and item with index \a idx is paused
    [[nodiscard]] int                      ProjectsInProgress() const noexcept { return m_projects_in_progress; }
    [[nodiscard]] float                    TotalRPsSpent() const noexcept { return m_total_RPs_spent; }
    [[nodiscard]] int                      EmpireID() const noexcept { return m_empire_id; }
    [[nodiscard]] std::vector<std::string> AllEnqueuedProjects() const;
    [[nodiscard]] std::string              Dump() const;

    // STL container-like interface
    [[nodiscard]] bool           empty() const noexcept { return m_queue.size() > 0; }
    [[nodiscard]] auto           size() const noexcept { return m_queue.size(); }
    [[nodiscard]] const_iterator begin() const noexcept { return m_queue.begin(); }
    [[nodiscard]] const_iterator end() const noexcept { return m_queue.end(); }
    [[nodiscard]] const_iterator find(const std::string& tech_name) const;
    [[nodiscard]] const Element& operator[](int i) const;

    /** Recalculates the RPs spent on and number of turns left for each project
      * in the queue.  Also determines the number of projects in prgress, and
      * the total number of RPs spent on the projects in the queue.  \note A
      * precondition of this function that \a RPs must be greater than some
      * epsilon > 0; see the implementation for the actual value used for
      * epsilon. */
    void Update(float RPs, const std::map<std::string, float>& research_progress,
                const std::vector<std::tuple<std::string_view, double, int>>& costs_times,
                const ScriptingContext& context);

    // STL container-like interface
    void                   push_back(const std::string& tech_name, bool paused = false);
    void                   insert(iterator it, const std::string& tech_name, bool paused = false);
    void                   erase(iterator it);
    [[nodiscard]] iterator begin() noexcept { return m_queue.begin(); }
    [[nodiscard]] iterator end() noexcept { return m_queue.end(); }
    [[nodiscard]] iterator find(const std::string& tech_name);
    void                   clear();

    mutable boost::signals2::signal<void ()> ResearchQueueChangedSignal;

private:
    QueueType   m_queue;
    int         m_projects_in_progress = 0;
    float       m_total_RPs_spent = 0.0f;
    int         m_empire_id = ALL_EMPIRES;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};


#endif
