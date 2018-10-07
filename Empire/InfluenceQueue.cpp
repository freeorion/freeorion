#include "InfluenceQueue.h"

#include "Empire.h"
#include "../universe/ValueRef.h"
#include "../util/AppInterface.h"
#include "../util/GameRules.h"
#include "../util/ScopedTimer.h"

#include <boost/range/numeric.hpp>
#include <boost/range/adaptor/map.hpp>


namespace {
    const float EPSILON = 0.01f;

    void AddRules(GameRules& rules) {
    }
    bool temp_bool = RegisterGameRules(&AddRules);

    float CalculateNewInfluenceStockpile(int empire_id, float starting_stockpile, float project_transfer_to_stockpile,
                                         float available_IP, float allocated_IP, float allocated_stockpile_IP)
    {
        TraceLogger() << "CalculateNewInfluenceStockpile for empire " << empire_id;
        const Empire* empire = GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "CalculateNewInfluenceStockpile() passed null empire.  doing nothing.";
            return 0.0f;
        }
        TraceLogger() << " ... stockpile used: " << allocated_stockpile_IP;
        float new_contributions = available_IP - allocated_IP;
        return starting_stockpile + new_contributions + project_transfer_to_stockpile - allocated_stockpile_IP;
    }

    /** Sets the allocated_IP value for each Element in the passed
      * InfluenceQueue \a queue. Elements are allocated IP based on their need,
      * the limits they can be given per turn, and the amount available to the
      * empire. Also checks if elements will be completed this turn. */
    void SetInfluenceQueueElementSpending(
        float available_IP, float available_stockpile,
        InfluenceQueue::QueueType& queue,
        float& allocated_IP, float& allocated_stockpile_IP,
        int& projects_in_progress, bool simulating)
    {
        projects_in_progress = 0;
        allocated_IP = 0.0f;
        allocated_stockpile_IP = 0.0f;

        float dummy_IP_source = 0.0f;
        float stockpile_transfer = 0.0f;

        //DebugLogger() << "queue size: " << queue.size();
        int i = 0;
        for (auto& queue_element : queue) {
            queue_element.allocated_ip = 0.0f;  // default, to be updated below...
            if (queue_element.paused) {
                TraceLogger() << "allocation: " << queue_element.allocated_ip
                              << "  to: " << queue_element.name
                              << "  due to it being paused";
                ++i;
                continue;
            }

            ++i;
        }
    }
}


/////////////////////////////
// InfluenceQueue::Element //
/////////////////////////////
InfluenceQueue::Element::Element()
{}

InfluenceQueue::Element::Element(InfluenceType influence_type_, int empire_id_, bool paused_) :
    influence_type(influence_type_),
    empire_id(empire_id_),
    paused(paused_)
{
    name = "";  // todo, depending on influence_type
}

InfluenceQueue::Element::Element(InfluenceType influence_type_, int empire_id_, std::string name_, bool paused_) :
    influence_type(influence_type_),
    name(name_),
    empire_id(empire_id_),
    paused(paused_)
{}

std::string InfluenceQueue::Element::Dump() const {
    std::stringstream retval;
    retval << "InfluenceQueue::Element: name: " << name << "  empire id: " << empire_id;
    retval << "  allocated: " << allocated_ip << "  turns left: " << turns_left;
    if (paused)
        retval << "  (paused)";
    retval << "\n";
    return retval.str();
}


////////////////////
// InfluenceQueue //
////////////////////
InfluenceQueue::InfluenceQueue(int empire_id) :
    m_projects_in_progress(0),
    m_expected_new_stockpile_amount(0),
    m_empire_id(empire_id)
{}

int InfluenceQueue::ProjectsInProgress() const
{ return m_projects_in_progress; }

float InfluenceQueue::TotalIPsSpent() const
{ return m_total_IPs_spent; }

float InfluenceQueue::AllocatedStockpileIP() const
{ return 0.0f; } // todo

bool InfluenceQueue::empty() const
{ return !m_queue.size(); }

unsigned int InfluenceQueue::size() const
{ return m_queue.size(); }

InfluenceQueue::const_iterator InfluenceQueue::begin() const
{ return m_queue.begin(); }

InfluenceQueue::const_iterator InfluenceQueue::end() const
{ return m_queue.end(); }

InfluenceQueue::const_iterator InfluenceQueue::find(const std::string& item_name) const {
    for (auto it = begin(); it != end(); ++it) {
        if (it->name == item_name)
            return it;
    }
    return end();
}

const InfluenceQueue::Element& InfluenceQueue::operator[](int i) const {
    assert(0 <= i && i < static_cast<int>(m_queue.size()));
    return m_queue[i];
}

void InfluenceQueue::Update() {
    const Empire* empire = GetEmpire(m_empire_id);
    if (!empire) {
        ErrorLogger() << "InfluenceQueue::Update passed null empire.  doing nothing.";
        m_projects_in_progress = 0;
        return;
    }

    ScopedTimer update_timer("InfluenceQueue::Update");

    float available_IP = empire->ResourceOutput(RE_INFLUENCE);
    float stockpiled_IP = empire->ResourceStockpile(RE_INFLUENCE);
    m_total_IPs_spent = 0.0f;

    m_expected_new_stockpile_amount = stockpiled_IP + available_IP - m_total_IPs_spent;

    //std::cout << "available IP: " << available_IP << "  stockpiled: " << stockpiled_IP << "  new expected: " << m_expected_new_stockpile_amount << std::endl;


    // cache Influence item costs and times
    // initialize Influence queue item completion status to 'never'


    boost::posix_time::ptime sim_time_start;
    boost::posix_time::ptime sim_time_end;

    DebugLogger() << "InfluenceQueue::Update: Projections took "
                  << ((sim_time_end - sim_time_start).total_microseconds()) << " microseconds with "
                  << empire->ResourceOutput(RE_INFLUENCE) << " influence output";
    InfluenceQueueChangedSignal();
}

void InfluenceQueue::push_back(const Element& element)
{ m_queue.push_back(element); }

void InfluenceQueue::insert(iterator it, const Element& element)
{ m_queue.insert(it, element); }

void InfluenceQueue::erase(int i) {
    assert(i <= static_cast<int>(size()));
    m_queue.erase(begin() + i);
}

InfluenceQueue::iterator InfluenceQueue::erase(iterator it) {
    assert(it != end());
    return m_queue.erase(it);
}

InfluenceQueue::iterator InfluenceQueue::begin()
{ return m_queue.begin(); }

InfluenceQueue::iterator InfluenceQueue::end()
{ return m_queue.end(); }

InfluenceQueue::iterator InfluenceQueue::find(const std::string& item_name) {
    for (auto it = begin(); it != end(); ++it) {
        if (it->name == item_name)
            return it;
    }
    return end();
}

InfluenceQueue::Element& InfluenceQueue::operator[](int i) {
    assert(0 <= i && i < static_cast<int>(m_queue.size()));
    return m_queue[i];
}

void InfluenceQueue::clear() {
    m_queue.clear();
    m_projects_in_progress = 0;
    InfluenceQueueChangedSignal();
}

