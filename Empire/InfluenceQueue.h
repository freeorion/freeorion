#ifndef _InfluenceQueue_h_
#define _InfluenceQueue_h_

#include "../universe/ConstantsFwd.h"
#include "../util/AppInterface.h"
#include "../util/Export.h"

#include <deque>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <boost/serialization/access.hpp>
#include <boost/signals2/signal.hpp>

class ResourcePool;


struct FO_COMMON_API InfluenceQueue {
    /** The type of a single element in the Influence queue. */
    struct FO_COMMON_API Element {
        Element() = default;
        Element(int empire_id_, std::string name_, bool paused_ = false) :
            name(std::move(name_)),
            empire_id(empire_id_),
            paused(paused_)
        {}

        std::string     name;                       ///< name of influence project
        int             empire_id = ALL_EMPIRES;
        float           allocated_ip = 0.0f;        ///< IP allocated to this InfluenceQueue Element by Empire Influence update
        bool            paused = false;

        [[nodiscard]] std::string Dump() const;

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

    explicit InfluenceQueue(int empire_id) :
        m_empire_id(empire_id)
    {}

    [[nodiscard]] bool  InQueue(const std::string& name) const;

    [[nodiscard]] int   ProjectsInProgress() const noexcept { return m_projects_in_progress; }
    [[nodiscard]] float TotalIPsSpent() const noexcept { return m_total_IPs_spent; };
    [[nodiscard]] int   EmpireID() const noexcept { return m_empire_id; }

    /** Returns amount of stockpile IP allocated to Influence queue elements. */
    [[nodiscard]] float AllocatedStockpileIP() const noexcept;

    /** Returns the value expected for the Influence Stockpile for the next
      * turn, based on the current InfluenceQueue allocations. */
    [[nodiscard]] float ExpectedNewStockpileAmount() const noexcept { return m_expected_new_stockpile_amount; }


    // STL container-like interface
    [[nodiscard]] auto empty() const noexcept { return m_queue.empty(); }
    [[nodiscard]] auto size() const noexcept { return m_queue.size(); }
    [[nodiscard]] auto begin() const noexcept { return m_queue.begin(); }
    [[nodiscard]] auto end() const noexcept { return m_queue.end(); }

    [[nodiscard]] const_iterator find(const std::string& item_name) const;
    [[nodiscard]] const Element& operator[](std::size_t i) const;
    [[nodiscard]] const Element& operator[](int i) const;


    /** Recalculates the PPs spent on and number of turns left for each project in the queue.  Also
      * determines the number of projects in progress, and the industry consumed by projects
      * in each resource-sharing group of systems.  Does not actually "spend" the PP; a later call to
      * empire->CheckInfluenceProgress() will actually spend PP, remove items from queue and create them
      * in the universe. */
    void Update(const ScriptingContext& context);


    // STL container-like interface
    void                    push_back(const Element& element) { m_queue.push_back(element); }
    void                    insert(iterator it, const Element& element) { m_queue.insert(it, element); }
    void                    erase(int i);
    iterator                erase(iterator it) { return m_queue.erase(it); }
    [[nodiscard]] iterator  begin() noexcept { return m_queue.begin(); }
    [[nodiscard]] iterator  end() noexcept { return m_queue.end(); }
    [[nodiscard]] iterator  find(const std::string& item_name);
    [[nodiscard]] Element&  operator[](int i);

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
