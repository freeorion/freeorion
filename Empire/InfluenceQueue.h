#ifndef _InfluenceQueue_h_
#define _InfluenceQueue_h_

#include "../util/Export.h"
#include "../universe/Enums.h"

#include <deque>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <boost/serialization/access.hpp>
#include <boost/signals2/signal.hpp>

class ResourcePool;
FO_COMMON_API extern const int INVALID_DESIGN_ID;
FO_COMMON_API extern const int INVALID_OBJECT_ID;
FO_COMMON_API extern const int ALL_EMPIRES;


struct FO_COMMON_API InfluenceQueue {
    /** The type of a single element in the research queue. */
    struct Element {
        Element() = default;
        Element(const std::string& name_, int empire_id_) :
            name(name_),
            empire_id(empire_id_)
        {}
        std::string Dump() const;

        std::string name = "";
        int         empire_id = ALL_EMPIRES;

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
    InfluenceQueue(int empire_id) :
        m_empire_id(empire_id)
    {}
    //@}

    /** \name Accessors */ //@{
    int   ProjectsInProgress() const;               ///< Returns the number of Influence projects currently (perhaps partially) funded.
    float TotalIPsSpent() const;                    ///< Returns the number of IPs currently spent on the projects in this queue.
    int   EmpireID() const { return m_empire_id; }

    /** Returns the amount of IP per turn the empire generates.
      * Does not include stockpile. */
    float AvailableIP(const std::shared_ptr<ResourcePool>& influence_pool) const;

    /** Returns amount of IP being used by influence activities and costs. */
    float AllocatedIP() const;

    /** Returns amount of IP being consumed from the stockpile. */
    const std::map<std::set<int>, float>& AllocatedStockpileIP() const;

    /** Returns the value expected for the Imperial Stockpile for the next turn, based on the current
    * InfluenceQueue allocations. */
    float ExpectedNewStockpileAmount() const { return m_expected_new_stockpile_amount; }


    // STL container-like interface
    bool            empty() const;
    unsigned int    size() const;
    const_iterator  begin() const;
    const_iterator  end() const;
    const_iterator  find(int i) const;
    const Element&  operator[](int i) const;

    /** \name Mutators */ //@{
    /** Recalculates the IPs spent on projects on the influence queue, whether
      * each project is funded, and the expected next turn stockpile of IP.
      * Does not actually "spend" the IP; a later call to
      * empire->CheckInfluenceProgress() will actually spend IP, remove projects
      * from queue and implement their effects. */
    void Update();

    // STL container-like interface
    void        push_back(const Element& element);
    void        insert(iterator it, const Element& element);
    void        erase(int i);
    iterator    erase(iterator it);

    iterator    begin();
    iterator    end();
    iterator    find(int i);
    Element&    operator[](int i);

    void        clear();

    mutable boost::signals2::signal<void ()> InfluenceQueueChangedSignal;
    //@}

private:
    int     m_empire_id = ALL_EMPIRES;
    float   m_expected_new_stockpile_amount = 0.0f;
    int     m_projects_in_progress = 0;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

#endif //  _InfluenceQueue_h_
