#include "InfluenceQueue.h"

#include "Empire.h"
#include "Government.h"
#include "../universe/ValueRef.h"
#include "../util/AppInterface.h"
#include "../util/GameRules.h"
#include "../util/ScopedTimer.h"

#include <boost/range/numeric.hpp>
#include <boost/range/adaptor/map.hpp>


namespace {
    const float EPSILON = 0.01f;

    //void AddRules(GameRules& rules)
    //{}
    //bool temp_bool = RegisterGameRules(&AddRules);

    //float CalculateNewInfluenceStockpile(int empire_id, float starting_stockpile, float project_transfer_to_stockpile,
    //                                     float available_IP, float allocated_IP, float allocated_stockpile_IP)
    //{
    //    TraceLogger() << "CalculateNewInfluenceStockpile for empire " << empire_id;
    //    const Empire* empire = GetEmpire(empire_id);
    //    if (!empire) {
    //        ErrorLogger() << "CalculateNewInfluenceStockpile() passed null empire.  doing nothing.";
    //        return 0.0f;
    //    }
    //    TraceLogger() << " ... stockpile used: " << allocated_stockpile_IP;
    //    float new_contributions = available_IP - allocated_IP;
    //    return starting_stockpile + new_contributions + project_transfer_to_stockpile - allocated_stockpile_IP;
    //}

    const InfluenceQueue::Element EMPTY_ELEMENT;

    /** Sets the allocated_IP value for each Element in the passed
      * InfluenceQueue \a queue. Elements are allocated IP based on their need,
      * the limits they can be given per turn, and the amount available to the
      * empire. Also checks if elements will be completed this turn. */
    //void SetInfluenceQueueElementSpending(
    //    float available_IP, float available_stockpile,
    //    InfluenceQueue::QueueType& queue,
    //    float& allocated_IP, float& allocated_stockpile_IP,
    //    int& projects_in_progress, bool simulating)
    //{
    //    projects_in_progress = 0;
    //    allocated_IP = 0.0f;
    //    allocated_stockpile_IP = 0.0f;

    //    float dummy_IP_source = 0.0f;
    //    float stockpile_transfer = 0.0f;

    //    //DebugLogger() << "queue size: " << queue.size();
    //    int i = 0;
    //    for (auto& queue_element : queue) {
    //        queue_element.allocated_ip = 0.0f;  // default, to be updated below...
    //        if (queue_element.paused) {
    //            TraceLogger() << "allocation: " << queue_element.allocated_ip
    //                          << "  to: " << queue_element.name
    //                          << "  due to it being paused";
    //            ++i;
    //            continue;
    //        }

    //        ++i;
    //    }
    //}
}


/////////////////////////////
// InfluenceQueue::Element //
/////////////////////////////
std::string InfluenceQueue::Element::Dump() const {
    std::stringstream retval;
    retval << "InfluenceQueue::Element: name: " << name << "  empire id: " << empire_id;
    retval << "  allocated: " << allocated_ip;
    if (paused)
        retval << "  (paused)";
    retval << "\n";
    return retval.str();
}


////////////////////
// InfluenceQueue //
////////////////////
bool InfluenceQueue::InQueue(const std::string& name) const {
    return std::any_of(m_queue.begin(), m_queue.end(),
                       [name](const Element& e){ return e.name == name; });
}

float InfluenceQueue::AllocatedStockpileIP() const
{ return 0.0f; } // todo

InfluenceQueue::const_iterator InfluenceQueue::find(const std::string& item_name) const
{ return std::find_if(begin(), end(), [&](const auto& e) { return e.name == item_name; }); }

const InfluenceQueue::Element& InfluenceQueue::operator[](std::size_t i) const {
    if (i >= m_queue.size())
        return EMPTY_ELEMENT;
    return m_queue[i];
}

const InfluenceQueue::Element& InfluenceQueue::operator[](int i) const
{ return operator[](static_cast<std::size_t>(i)); }

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

    float spending_on_policy_adoption_ip = 0.0f;
    for (auto policy_turn : empire->TurnsPoliciesAdopted()) {
        if (policy_turn.second != CurrentTurn())
            continue;
        auto policy = GetPolicy(policy_turn.first);
        if (!policy) {
            ErrorLogger() << "InfluenceQueue::Update couldn't get policy supposedly adopted this turn: " << policy_turn.first;
            continue;
        }
        spending_on_policy_adoption_ip += policy->AdoptionCost(m_empire_id);
    }

    m_total_IPs_spent = spending_on_policy_adoption_ip;

    m_expected_new_stockpile_amount = stockpiled_IP + available_IP - m_total_IPs_spent;

    DebugLogger() << "InfluenceQueue::Update : available IP: " << available_IP << "  stockpiled: " << stockpiled_IP << "  new expected: " << m_expected_new_stockpile_amount << std::endl;

    //// cache Influence item costs and times
    //// initialize Influence queue item completion status to 'never'
    //boost::posix_time::ptime sim_time_start;
    //boost::posix_time::ptime sim_time_end;

    //DebugLogger() << "InfluenceQueue::Update: Projections took "
    //              << ((sim_time_end - sim_time_start).total_microseconds()) << " microseconds with "
    //              << empire->ResourceOutput(RE_INFLUENCE) << " influence output";
    InfluenceQueueChangedSignal();
}

void InfluenceQueue::erase(int i) {
    if (i > 0 && i < static_cast<int>(m_queue.size()))
        m_queue.erase(begin() + i);
}

InfluenceQueue::iterator InfluenceQueue::find(const std::string& item_name)
{ return std::find_if(begin(), end(), [&](const auto& e) { return e.name == item_name; }); }

InfluenceQueue::Element& InfluenceQueue::operator[](int i) {
    assert(0 <= i && i < static_cast<int>(m_queue.size()));
    return m_queue[i];
}

void InfluenceQueue::clear() {
    m_queue.clear();
    m_projects_in_progress = 0;
    InfluenceQueueChangedSignal();
}

