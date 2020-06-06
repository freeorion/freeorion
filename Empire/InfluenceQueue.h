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
    /** The type of a single element in the Influence queue. */
    struct FO_COMMON_API Element {
        explicit Element() = default;
        Element(int empire_id_, std::string name_, bool paused_ = false) :
            name(name_),
            empire_id(empire_id_),
            paused(paused_)
        {}

        std::string     name;                       ///< name of influence project
        int             empire_id = ALL_EMPIRES;
        float           allocated_ip = 0.0f;        ///< IP allocated to this InfluenceQueue Element by Empire Influence update
        bool            paused = false;

        std::string Dump() const;

    private:
        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version);
    };

    typedef std::deque<Element> QueueType;

    /** The InfluenceQueue iterator type.  Dereference yields a Element. */
    typedef QueueType::iterator iterator;
    /** The const InfluenceQueue iterator type.  Dereference yields a Element. */
    typedef QueueType::const_iterator const_iterator;

    /** \name Structors */ //@{
    explicit InfluenceQueue(int empire_id) :
        m_empire_id(empire_id)
    {}
    //@}

    /** \name Accessors */ //@{
    bool    InQueue(const std::string& name) const; ///< Returns true iff \a name influence project is in this queue.
    int     ProjectsInProgress() const;             ///< Returns the number of Influence projects currently (perhaps partially) funded.
    float   TotalIPsSpent() const;                  ///< Returns the number of IPs currently spent on the projects in this queue.
    int     EmpireID() const { return m_empire_id; }

    /** Returns amount of stockpile IP allocated to Influence queue elements. */
    float AllocatedStockpileIP() const;

    /** Returns the value expected for the Influence Stockpile for the next
      * turn, based on the current InfluenceQueue allocations. */
    float ExpectedNewStockpileAmount() const { return m_expected_new_stockpile_amount; }


    // STL container-like interface
    bool            empty() const;
    unsigned int    size() const;
    const_iterator  begin() const;
    const_iterator  end() const;
    const_iterator  find(const std::string& item_name) const;
    const Element&  operator[](int i) const;

    /** \name Mutators */ //@{
    /** Recalculates the PPs spent on and number of turns left for each project in the queue.  Also
      * determines the number of projects in progress, and the industry consumed by projects
      * in each resource-sharing group of systems.  Does not actually "spend" the PP; a later call to
      * empire->CheckInfluenceProgress() will actually spend PP, remove items from queue and create them
      * in the universe. */
    void Update();

    // STL container-like interface
    void        push_back(const Element& element);
    void        insert(iterator it, const Element& element);
    void        erase(int i);
    iterator    erase(iterator it);

    iterator    begin();
    iterator    end();
    iterator    find(const std::string& item_name);
    Element&    operator[](int i);

    void        clear();

    mutable boost::signals2::signal<void ()> InfluenceQueueChangedSignal;
    //@}

private:
    QueueType   m_queue;
    int         m_projects_in_progress = 0;
    float       m_total_IPs_spent = 0.0f;
    float       m_expected_new_stockpile_amount = 0.0f;
    int         m_empire_id = ALL_EMPIRES;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

#endif //  _InfluenceQueue_h_
