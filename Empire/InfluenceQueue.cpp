#include "InfluenceQueue.h"

#include "Empire.h"
#include "Government.h"
#include "../universe/Planet.h"
#include "../universe/Species.h"
#include "../universe/ValueRef.h"
#include "../util/AppInterface.h"
#include "../util/GameRules.h"
#include "../util/ScopedTimer.h"

#include <utility>
#if !defined(__cpp_lib_integer_comparison_functions)
namespace std {
    inline auto cmp_less(auto&& lhs, auto&& rhs) { return lhs < rhs; }
}
#endif

namespace {
#if defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 12) || (__GNUC__ == 12 && __GNUC_MINOR__ >= 2))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934))) && ((!defined(__clang_major__) || (__clang_major__ >= 17)))
    constexpr const InfluenceQueue::Element EMPTY_ELEMENT;
#else
    const InfluenceQueue::Element EMPTY_ELEMENT;
#endif
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

    const float spending_on_policy_adoption_IP = [&empire, &context, &policy_costs]() {
        float spending_on_policy_adoption_IP = 0.0f;
        for (const auto& [policy_name, adoption_turn] : empire->TurnsPoliciesAdopted()) {
            if (adoption_turn != context.current_turn)
                continue;
            const auto ct_it = std::find_if(policy_costs.begin(), policy_costs.end(),
                                            [pn{policy_name}](const auto& ct) { return ct.first == pn; });
            if (ct_it == policy_costs.end()) {
                ErrorLogger() << "Missing policy " << policy_name << " cost time in InfluenceQueue::Update!";
                continue;
            }
            spending_on_policy_adoption_IP += static_cast<float>(ct_it->second);
        }
        return spending_on_policy_adoption_IP;
    }();


    const auto annexed_by_this_empire = [this, curturn{context.current_turn}](const Planet& planet) {
        return planet.OrderedAnnexedByEmpire() == m_empire_id ||
            (planet.LastAnnexedByEmpire() == m_empire_id && planet.TurnsSinceLastAnnexed(curturn) == 0);
    };
    const auto annexed_planets = context.ContextObjects().findIDs<Planet>(annexed_by_this_empire);

    const float spending_on_annexation_IP = [&annexed_planets, &annex_costs]() {
        float spending_on_annexation_IP = 0.0f;
        for (const auto planet_id : annexed_planets) {
            const auto p_it = std::find_if(annex_costs.begin(), annex_costs.end(),
                                           [planet_id](const auto& pac) { return pac.first == planet_id; });
            if (p_it == annex_costs.end()) {
                ErrorLogger() << "Missing annexation cost for plaent " << planet_id << " in InfluenceQueue::Update!";
                continue;
            }
            spending_on_annexation_IP += static_cast<float>(p_it->second);
        }
        return spending_on_annexation_IP;
    }();


    m_total_IPs_spent = spending_on_policy_adoption_IP + spending_on_annexation_IP;
    m_expected_new_stockpile_amount = stockpiled_IP + available_IP - m_total_IPs_spent;

    DebugLogger() << "InfluenceQueue::Update : available IP: " << available_IP << "  stockpiled: "
                  << stockpiled_IP << "  new expected: " << m_expected_new_stockpile_amount << "\n";

    InfluenceQueueChangedSignal();
}

void InfluenceQueue::erase(int i) {
    if (i > 0 && std::cmp_less(i, m_queue.size()))
        m_queue.erase(begin() + i);
}

InfluenceQueue::iterator InfluenceQueue::find(const std::string& item_name)
{ return std::find_if(begin(), end(), [&](const auto& e) { return e.name == item_name; }); }

InfluenceQueue::Element& InfluenceQueue::operator[](int i) {
    assert(0 <= i && std::cmp_less(i, m_queue.size()));
    return m_queue[i];
}

void InfluenceQueue::clear() {
    m_queue.clear();
    m_projects_in_progress = 0;
    InfluenceQueueChangedSignal();
}

