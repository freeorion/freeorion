#ifndef _InfluenceQueue_h_
#define _InfluenceQueue_h_

#include "../util/Export.h"

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


struct FO_COMMON_API InfluenceQueue
{
    //! The type of a single element in the Influence queue.
    struct FO_COMMON_API Element
    {
        explicit Element() = default;

        Element(int empire_id_, std::string name_, bool paused_ = false) :
            name(std::move(name_)),
            empire_id(empire_id_),
            paused(paused_)
        {}

        //! Name of influence project
        std::string     name;
        int             empire_id = ALL_EMPIRES;
        //! IP allocated to this InfluenceQueue Element by Empire Influence
        //! update
        float           allocated_ip = 0.0f;
        bool            paused = false;

        std::string Dump() const;

    private:
        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version);
    };

    typedef std::deque<Element> QueueType;

    //! The InfluenceQueue iterator type.  Dereference yields a Element.
    typedef QueueType::iterator iterator;

    //! The const InfluenceQueue iterator type.  Dereference yields a Element.
    typedef QueueType::const_iterator const_iterator;

    explicit InfluenceQueue(int empire_id) :
        m_empire_id(empire_id)
    {}

    auto InQueue(const std::string& name) const -> bool;

    auto ProjectsInProgress() const -> int
    { return m_projects_in_progress; }

    auto TotalIPsSpent() const -> float
    { return m_total_IPs_spent; };

    auto EmpireID() const -> int
    { return m_empire_id; }

    //! Returns amount of stockpile IP allocated to Influence queue elements.
    auto AllocatedStockpileIP() const -> float;

    //! Returns the value expected for the Influence Stockpile for the next
    //! turn, based on the current InfluenceQueue allocations.
    auto ExpectedNewStockpileAmount() const -> float
    { return m_expected_new_stockpile_amount; }

    //! Recalculates the PPs spent on and number of turns left for each project
    //! in the queue.  Also determines the number of projects in progress, and
    //! the industry consumed by projects * in each resource-sharing group of
    //! systems.  Does not actually "spend" the PP; a later call to
    //! empire->CheckInfluenceProgress() will actually spend PP, remove items
    //! from queue and create them in the universe.
    void Update();

    auto empty() const -> bool
    { return m_queue.empty(); }

    auto size() const -> std::size_t
    { return m_queue.size(); }

    auto begin() const -> const_iterator
    { return m_queue.begin(); }

    auto end() const -> const_iterator
    { return m_queue.end(); }

    auto find(const std::string& item_name) const -> const_iterator;

    auto operator[](std::size_t i) const -> const Element&;

    auto operator[](int i) const -> const Element&;

    void push_back(const Element& element)
    { m_queue.push_back(element); }

    void insert(iterator it, const Element& element)
    { m_queue.insert(it, element); }

    void erase(int i);

    auto erase(iterator it) -> iterator
    { return m_queue.erase(it); }

    auto begin() -> iterator
    { return m_queue.begin(); }

    auto end() -> iterator
    { return m_queue.end(); }

    auto find(const std::string& item_name) -> iterator;

    auto operator[](int i) -> Element&;

    void clear();

    mutable boost::signals2::signal<void ()> InfluenceQueueChangedSignal;

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


#endif
