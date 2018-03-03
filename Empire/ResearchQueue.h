#ifndef _ResearchQueue_h_
#define _ResearchQueue_h_

#include "../util/Export.h"

#include <deque>
#include <map>
#include <string>
#include <vector>
#include <boost/serialization/access.hpp>
#include <boost/signals2/signal.hpp>

FO_COMMON_API extern const int ALL_EMPIRES;

struct FO_COMMON_API ResearchQueue {
    /** The type of a single element in the research queue. */
    struct Element {
        Element()
        {}
        Element(const std::string& name_, int empire_id_, float spending_, int turns_left_, bool paused_ = false) :
            name(name_),
            empire_id(empire_id_),
            allocated_rp(spending_),
            turns_left(turns_left_),
            paused(paused_)
        {}
        std::string Dump() const;

        std::string name;
        int         empire_id = ALL_EMPIRES;
        float       allocated_rp = 0.0f;
        int         turns_left = 0;
        bool        paused = false;
    private:
        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version);
    };

    typedef std::deque<Element> QueueType;

    /** The ResearchQueue iterator type.  Dereference yields an Element. */
    typedef QueueType::iterator iterator;
    /** The const ResearchQueue iterator type.  Dereference yields an Element. */
    typedef QueueType::const_iterator const_iterator;

    /** \name Structors */ //@{
    ResearchQueue(int empire_id) :
        m_empire_id(empire_id)
    {}
    //@}

    /** \name Accessors */ //@{
    bool                        InQueue(const std::string& tech_name) const;///< Returns true iff \a tech is in this queue.
    bool                        Paused(const std::string& tech_name) const; ///< Returns true iff \a tech is in this queue and paused.
    bool                        Paused(int idx) const;                      ///< Returns true iff there are at least \a idx - 1 items in the queue and item with index \a idx is paused
    int                         ProjectsInProgress() const;                 ///< Returns the number of research projects currently (perhaps partially) funded.
    float                       TotalRPsSpent() const;                      ///< Returns the number of RPs currently spent on the projects in this queue.
    int                         EmpireID() const { return m_empire_id; }
    std::vector<std::string>    AllEnqueuedProjects() const;
    std::string                 Dump() const;

    // STL container-like interface
    bool            empty() const;
    unsigned int    size() const;
    const_iterator  begin() const;
    const_iterator  end() const;
    const_iterator  find(const std::string& tech_name) const;
    const Element&  operator[](int i) const;
    //@}

    /** \name Mutators */ //@{
    /** Recalculates the RPs spent on and number of turns left for each project
      * in the queue.  Also determines the number of projects in prgress, and
      * the total number of RPs spent on the projects in the queue.  \note A
      * precondition of this function that \a RPs must be greater than some
      * epsilon > 0; see the implementation for the actual value used for
      * epsilon. */
    void            Update(float RPs, const std::map<std::string, float>& research_progress);

    // STL container-like interface
    void            push_back(const std::string& tech_name, bool paused = false);
    void            insert(iterator it, const std::string& tech_name, bool paused = false);
    void            erase(iterator it);

    iterator        begin();
    iterator        end();
    iterator        find(const std::string& tech_name);

    void            clear();

    mutable boost::signals2::signal<void ()> ResearchQueueChangedSignal;
    //@}

private:
    QueueType   m_queue;
    int         m_projects_in_progress = 0;
    float       m_total_RPs_spent = 0.0f;
    int         m_empire_id = ALL_EMPIRES;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

#endif // _ResearchQueue_h_
