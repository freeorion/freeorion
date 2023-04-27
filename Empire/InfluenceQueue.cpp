#include "InfluenceQueue.h"

#include "Empire.h"
#include "Government.h"
#include "../universe/Planet.h"
#include "../universe/Species.h"
#include "../universe/ValueRef.h"
#include "../util/AppInterface.h"
#include "../util/GameRules.h"
#include "../util/ScopedTimer.h"

#include <boost/range/numeric.hpp>
#include <boost/range/adaptor/map.hpp>


namespace {
    //constexpr float EPSILON = 0.01f;

    //void AddRules(GameRules& rules)
    //{}
    //bool temp_bool = RegisterGameRules(&AddRules);
    const InfluenceQueue::Element EMPTY_ELEMENT;
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

float InfluenceQueue::AllocatedStockpileIP() const noexcept
{ return 0.0f; } // TODO: implement this...

InfluenceQueue::const_iterator InfluenceQueue::find(const std::string& item_name) const
{ return std::find_if(begin(), end(), [&](const auto& e) { return e.name == item_name; }); }

const InfluenceQueue::Element& InfluenceQueue::operator[](std::size_t i) const {
    if (i >= m_queue.size())
        return EMPTY_ELEMENT;
    return m_queue[i];
}

const InfluenceQueue::Element& InfluenceQueue::operator[](int i) const
{ return operator[](static_cast<std::size_t>(i)); }

void InfluenceQueue::Update(const ScriptingContext& context,
                            const std::vector<std::pair<int, double>>& annex_costs,
                            const std::vector<std::pair<std::string_view, double>>& policy_costs)
{
    auto empire = context.GetEmpire(m_empire_id);
    if (!empire) {
        ErrorLogger() << "InfluenceQueue::Update passed null empire.  doing nothing.";
        m_projects_in_progress = 0;
        return;
    }

    ScopedTimer update_timer("InfluenceQueue::Update");

    const float available_IP = empire->ResourceOutput(ResourceType::RE_INFLUENCE);
    const float stockpiled_IP = empire->ResourceStockpile(ResourceType::RE_INFLUENCE);


    float spending_on_policy_adoption_ip = 0.0f;
    for (const auto& [policy_name, adoption_turn] : empire->TurnsPoliciesAdopted()) {
        if (adoption_turn != context.current_turn)
            continue;
        const auto policy = GetPolicy(policy_name);
        if (!policy) {
            ErrorLogger() << "InfluenceQueue::Update couldn't get policy supposedly adopted this turn: " << policy_name;
            continue;
        }
        spending_on_policy_adoption_ip += policy->AdoptionCost(m_empire_id, context);
    }


    const auto annexed_by_this_empire = [this](const Planet& planet)
    { return planet.OrderedAnnexedByEmpire() == m_empire_id; };

    float spending_on_annexation = 0.0f;
    // transform_reduce
    for (const auto* planet : context.ContextObjects().findRaw<Planet>(annexed_by_this_empire))
        spending_on_annexation += planet->AnnexationCost(m_empire_id, context);


    m_total_IPs_spent = spending_on_policy_adoption_ip + spending_on_annexation;

    m_expected_new_stockpile_amount = stockpiled_IP + available_IP - m_total_IPs_spent;

    DebugLogger() << "InfluenceQueue::Update : available IP: " << available_IP << "  stockpiled: "
                  << stockpiled_IP << "  new expected: " << m_expected_new_stockpile_amount << "\n";

    //// cache Influence item costs and times
    //// initialize Influence queue item completion status to 'never'
    //boost::posix_time::ptime sim_time_start;
    //boost::posix_time::ptime sim_time_end;

    //DebugLogger() << "InfluenceQueue::Update: Projections took "
    //              << ((sim_time_end - sim_time_start).total_microseconds()) << " microseconds with "
    //              << empire->ResourceOutput(ResourceType::RE_INFLUENCE) << " influence output";
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

