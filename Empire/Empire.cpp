#include "Empire.h"

#include "../util/i18n.h"
#include "../util/Random.h"
#include "../util/Logger.h"
#include "../util/AppInterface.h"
#include "../util/SitRepEntry.h"
#include "../universe/Building.h"
#include "../universe/BuildingType.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/ShipDesign.h"
#include "../universe/ShipHull.h"
#include "../universe/ShipPart.h"
#include "../universe/Planet.h"
#include "../universe/System.h"
#include "../universe/Tech.h"
#include "../universe/UniverseObject.h"
#include "../universe/UnlockableItem.h"
#include "EmpireManager.h"
#include "Supply.h"
#include "Government.h"

#include <boost/uuid/uuid_io.hpp>


namespace {
    constexpr float EPSILON = 0.01f;
    const std::string EMPTY_STRING;

    std::string operator+(const std::string_view sv, const char* c) {
        std::string retval;
        retval.reserve(sv.size() + std::strlen(c));
        retval.append(sv);
        retval.append(c);
        return retval;
    }

    auto PolicyCategoriesSlotsMeters() {
        std::vector<std::pair<std::string_view, std::string>> retval;
        const auto cats = GetPolicyManager().PolicyCategories();
        retval.reserve(cats.size());
        // derive meters from PolicyManager parsed policies' categories
        std::transform(cats.begin(), cats.end(), std::back_inserter(retval),
                       [](std::string_view cat) -> std::pair<std::string_view, std::string>
                       { return {cat, cat + "_NUM_POLICY_SLOTS"}; });
        return retval;
    }

    DeclareThreadSafeLogger(supply);
}

////////////
// Empire //
////////////
Empire::Empire() :
    m_research_queue(m_id),
    m_production_queue(m_id),
    m_influence_queue(m_id)
{ Init(); }

Empire::Empire(std::string name, std::string player_name,
               int empire_id, EmpireColor color, bool authenticated) :
    m_id(empire_id),
    m_name(std::move(name)),
    m_player_name(std::move(player_name)),
    m_color(color),
    m_meters{{UserStringNop("METER_DETECTION_STRENGTH"), {}}},
    m_research_queue(m_id),
    m_production_queue(m_id),
    m_influence_queue(m_id),
    m_authenticated(authenticated)
{
    //DebugLogger() << "Empire::Empire(" << m_name << ", " << m_player_name << ", " << empire_id << ", colour)";
    Init();
}

void Empire::Init() {
    for (auto& entry : PolicyCategoriesSlotsMeters())
        m_meters.emplace(std::piecewise_construct,
                         std::forward_as_tuple(std::move(entry.second)),
                         std::forward_as_tuple());
}

std::shared_ptr<const UniverseObject> Empire::Source(const ObjectMap& objects) const {
    if (m_eliminated)
        return nullptr;

    // Use the current source if valid
    auto valid_current_source = objects.get(m_source_id);
    if (valid_current_source && valid_current_source->OwnedBy(m_id))
        return valid_current_source;

    // Try the capital
    auto capital_as_source = objects.get(m_capital_id);
    if (capital_as_source && capital_as_source->OwnedBy(m_id)) {
        m_source_id = m_capital_id;
        return capital_as_source;
    }

    // Find any planet / ship owned by the empire
    auto owned = [this](const auto& o) { return o.second->OwnedBy(m_id); };

    const auto& planets = objects.allExisting<Planet>();
    auto p_it = std::find_if(planets.begin(), planets.end(), owned);
    if (p_it != planets.end())
        return p_it->second;

    const auto& ships = objects.allExisting<Ship>();
    auto s_it = std::find_if(ships.begin(), ships.end(), owned);
    if (s_it != ships.end())
        return s_it->second;

    m_source_id = INVALID_OBJECT_ID;
    return nullptr;
}

std::string Empire::Dump() const {
    std::string retval = "Empire name: " + m_name +
                         " ID: " + std::to_string(m_id) +
                         " Capital ID: " + std::to_string(m_capital_id);
    retval += " meters:\n";
    for (const auto& [name, meter] : m_meters) {
        retval += UserString(name) + ": " +
                  std::to_string(meter.Initial()) + "\n";
    }
    return retval;
}

void Empire::SetCapitalID(int id, const ObjectMap& objects) {
    m_capital_id = INVALID_OBJECT_ID;
    m_source_id = INVALID_OBJECT_ID;

    if (id == INVALID_OBJECT_ID)
        return;

    // Verify that the capital exists and is owned by the empire
    auto possible_capital = objects.getExisting(id);
    if (possible_capital && possible_capital->OwnedBy(m_id))
        m_capital_id = id;

    auto possible_source = objects.getRaw(id);
    if (possible_source && possible_source->OwnedBy(m_id))
        m_source_id = id;
}

void Empire::AdoptPolicy(const std::string& name, const std::string& category,
                         const ScriptingContext& context, bool adopt, int slot)
{
    if (adopt && name.empty()) {
        ErrorLogger() << "Empire::AdoptPolicy asked to adopt empty policy name in category " << category << " slot " << slot;
        return;
    } else if (name.empty()) {
        ErrorLogger() << "Empire::AdoptPolicy asked to de-adopt empty policy name";
        return;
    }

    if (!adopt) {
        // revoke policy
        if (m_adopted_policies.erase(name))
            PoliciesChangedSignal();
        return;
    }

    // check that policy is available
    if (!m_available_policies.contains(name)) {
        DebugLogger() << "Policy name: " << name << "  not available to empire with id: " << m_id;
        return;
    }

    // does policy exist?
    const auto policy = GetPolicy(name);
    if (!policy) {
        ErrorLogger() << "Empire::AdoptPolicy can't find policy with name: " << name;
        return;
    }

    // is category appropriate?
    if (!policy->Category().empty() && policy->Category() != category) {
        ErrorLogger() << "Empire::AdoptPolicy asked to handle policy " << name << " in category " << category
                      << " but that policy has category " << policy->Category();
        return;
    }

    // are there conflicts with other policies or missing prerequisite policies?
    if (!PolicyPrereqsAndExclusionsOK(name, context.current_turn)) {
        ErrorLogger() << "Empire::AdoptPolicy asked to adopt policy " << name
                      << " whose prerequisites are not met or which has a conflicting exclusion with already-adopted policies";
        return;
    }

    // check that empire has sufficient influence to adopt policy, after
    // also adopting any other policies that were first adopted this turn
    // add up all other policy adoption costs for this turn
    if (!PolicyAffordable(name, context)) {
        ErrorLogger() << "Empire::AdoptPolicy asked to adopt policy " << name
                      << " which is too expensive to adopt now";
        return;
    }

    // check that policy is not already adopted
    if (m_adopted_policies.contains(name)) {
        ErrorLogger() << "Empire::AdoptPolicy policy " << name << "  already adopted in category "
                      << m_adopted_policies[name].category << "  in slot "
                      << m_adopted_policies[name].slot_in_category << "  on turn "
                      << m_adopted_policies[name].adoption_turn;
        return;
    }

    // get slots for category requested for policy to be adopted in
    const auto total_slots_in_category = TotalPolicySlots()[category];
    if (total_slots_in_category < 1 || slot >= total_slots_in_category) {
        ErrorLogger() << "Empire::AdoptPolicy can't adopt policy: " << name
                      << "  into category: " << category << "  in slot: " << slot
                      << " because category has only " << total_slots_in_category
                      << " slots total";
        return;
    }

    // collect already-adopted policies in category
    std::map<int, std::string> adopted_policies_in_category_map;
    for (auto& [policy_name, adoption_info] : m_adopted_policies) {
        if (adoption_info.category != category)
            continue;
        if (adoption_info.slot_in_category >= total_slots_in_category) {
            ErrorLogger() << "Empire::AdoptPolicy found adopted policy: "
                          << policy_name << "  in category: " << category
                          << "  in slot: " << adoption_info.slot_in_category
                          << "  which is higher than max slot in category: "
                          << (total_slots_in_category - 1);
        }
        if (slot != INVALID_SLOT_INDEX && adoption_info.slot_in_category == slot) {
            ErrorLogger() << "Empire::AdoptPolicy found adopted policy: "
                          << policy_name << "  in category: " << category
                          << "  in slot: " << slot
                          << "  so cannot adopt another policy in that slot";
            return;
        }

        adopted_policies_in_category_map[adoption_info.slot_in_category] = policy_name;
    }
    // convert to vector
    std::vector<std::string> adopted_policies_in_category(total_slots_in_category, "");
    for (auto& [adopted_policy_slot, adopted_policy_name] : adopted_policies_in_category_map) {
        if (adopted_policy_slot < 0 || adopted_policy_slot >= static_cast<int>(adopted_policies_in_category.size())) {
            ErrorLogger() << "AdoptPolicy somehow got slot " << adopted_policy_slot << " of adopted policy " << adopted_policy_name
                          << " outside the suitable range with total slots size: " << adopted_policies_in_category.size();
            continue;
        }
        adopted_policies_in_category[adopted_policy_slot] = std::move(adopted_policy_name);
    }



    // if no particular slot was specified, try to find a suitable slot in category
    if (slot == INVALID_SLOT_INDEX) {
        // search for any suitable empty slot
        for (std::size_t i = 0u; i < adopted_policies_in_category.size(); ++i) {
            if (adopted_policies_in_category[i].empty()) {
                slot = i;
                break;
            }
        }

        if (slot == INVALID_SLOT_INDEX) {
            ErrorLogger() << "Couldn't find empty slot for policy in category: " << category;
            return;
        }
    }

    // adopt policy in requested category on this turn, unless it was already
    // adopted at the start of this turn, in which case restore / keep its
    // previous adtoption turn
    int adoption_turn = context.current_turn;
    auto it = m_initial_adopted_policies.find(name);
    if (it != m_initial_adopted_policies.end())
        adoption_turn = it->second.adoption_turn;
    m_adopted_policies[name] = {adoption_turn, category, slot};

    DebugLogger() << "Empire::AdoptPolicy policy " << name << "  adopted in category "
                  << m_adopted_policies[name].category << "  in slot "
                  << m_adopted_policies[name].slot_in_category << "  on turn "
                  << m_adopted_policies[name].adoption_turn;

    PoliciesChangedSignal();
}

void Empire::RevertPolicies() {
    if (m_adopted_policies != m_initial_adopted_policies) {
        m_adopted_policies = m_initial_adopted_policies;
        PoliciesChangedSignal();
    }
}

void Empire::UpdatePolicies(bool update_cumulative_adoption_time, int current_turn) {
    // TODO: Check and handle policy exclusions in this function...

    // check that there are enough slots for adopted policies in their current slots
    auto total_category_slot_counts = TotalPolicySlots(); // how many slots in each category
    std::set<std::string_view> categories_needing_rearrangement;                 // which categories have a problem
    std::map<std::string_view, std::map<int, int>> category_slot_policy_counts;   // how many policies in each slot of each category
    for (auto& [policy_name, adoption_info] : m_adopted_policies) {
        (void)policy_name; // quiet warning
        const auto& [adoption_turn, slot_in_category, category] = adoption_info;
        (void)adoption_turn; // quiet warning
        const auto& slot_count = category_slot_policy_counts[category][slot_in_category]++; // count how many policies in this slot of this category...
        if (slot_count > 1 || slot_in_category >= total_category_slot_counts[category])     // if multiple policies in a slot, or slot a policy is in is too high, mark category as problematic...
            categories_needing_rearrangement.insert(category);
    }

    // if a category has too many policies or a slot number conflict, rearrange it
    // and remove the excess policies
    for (const auto& cat : categories_needing_rearrangement) {
        DebugLogger() << "Rearranging poilicies in category " << cat << ":";

        auto policies_temp{m_adopted_policies};

        // all adopted policies in this category, sorted by slot and adoption turn (lower first)
        std::vector<std::tuple<int, int, std::string>> slots_turns_policies;
        slots_turns_policies.reserve(m_adopted_policies.size());
        for (auto& [temp_policy_name, temp_adoption_info] : policies_temp) {
            auto& [turn, slot, temp_category] = temp_adoption_info;  // PolicyAdoptionInfo { int adoption_turn; int slot_in_category; std::string category; }
            if (temp_category != cat)
                continue;
            DebugLogger() << "... Policy " << temp_policy_name << " was in slot " << slot;
            m_adopted_policies.erase(temp_policy_name); // remove everything from adopted policies in this category...
            slots_turns_policies.emplace_back(slot, turn, std::move(temp_policy_name));
        }

        auto slot_turn_comp = [](const auto& lhs, const auto& rhs) {
            auto& [l_slot, l_turn, l_ignored] = lhs;
            auto& [r_slot, r_turn, r_ignored] = rhs;
            (void)l_ignored;
            (void)r_ignored;
            return (l_slot < r_slot) || ((l_slot == r_slot) && (l_turn < r_turn));
        };
        std::sort(slots_turns_policies.begin(), slots_turns_policies.end(), slot_turn_comp);

        // re-add in category up to limit, ordered priority by original slot and adoption turn
        int added = 0;
        for (auto& [ignored, turn, policy_name] : slots_turns_policies) {
            (void)ignored;
            if (added >= total_category_slot_counts[cat])
                break;  // can't add more...
            int new_slot = added++;
            DebugLogger() << "... Policy " << policy_name << " re-added in slot " << new_slot;
            m_adopted_policies[std::move(policy_name)] = PolicyAdoptionInfo{turn, std::string{cat}, new_slot};
        }
    }

    // update counters of how many turns each policy has been adopted
    m_policy_adoption_current_duration.clear();
    for (auto& [policy_name, adoption_info] : m_adopted_policies) {
        m_policy_adoption_current_duration[policy_name] = current_turn - adoption_info.adoption_turn;

        if (update_cumulative_adoption_time)
            m_policy_adoption_total_duration[policy_name]++;  // assumes default initialization to 0
    }

    // update initial adopted policies for next turn
    m_initial_adopted_policies = m_adopted_policies;
    PoliciesChangedSignal();
}

int Empire::TurnPolicyAdopted(std::string_view name) const {
    auto it = m_adopted_policies.find(name);
    if (it == m_adopted_policies.end())
        return INVALID_GAME_TURN;
    return it->second.adoption_turn;
}

int Empire::CurrentTurnsPolicyHasBeenAdopted(std::string_view name) const {
    auto it = std::find_if(m_policy_adoption_current_duration.begin(),
                           m_policy_adoption_current_duration.end(),
                           [name](const auto& pacd) { return name == pacd.first; });
    if (it == m_policy_adoption_current_duration.end())
        return 0;
    return it->second;
}

int Empire::CumulativeTurnsPolicyHasBeenAdopted(std::string_view name) const {
    auto it = std::find_if(m_policy_adoption_total_duration.begin(),
                           m_policy_adoption_total_duration.end(),
                           [name](const auto& patd) { return name == patd.first; });
    if (it == m_policy_adoption_total_duration.end())
        return 0;
    return it->second;
}

int Empire::SlotPolicyAdoptedIn(std::string_view name) const {
    if (!PolicyAdopted(name))
        return INVALID_SLOT_INDEX;
    auto it = m_adopted_policies.find(name);
    return it->second.slot_in_category;
}

std::vector<std::string_view> Empire::AdoptedPolicies() const {
    std::vector<std::string_view> retval;
    retval.reserve(m_adopted_policies.size());
    for (const auto& entry : m_adopted_policies)
        retval.push_back(entry.first);
    return retval;
}

std::vector<std::string_view> Empire::InitialAdoptedPolicies() const {
    std::vector<std::string_view> retval;
    retval.reserve(m_initial_adopted_policies.size());
    for (const auto& entry : m_initial_adopted_policies)
        retval.push_back(entry.first);
    return retval;
}

std::map<std::string_view, std::map<int, std::string_view>>
Empire::CategoriesSlotsPoliciesAdopted() const {
    std::map<std::string_view, std::map<int, std::string_view>> retval;
    for (auto& [policy_name, adoption_info] : m_adopted_policies)
        retval[adoption_info.category][adoption_info.slot_in_category] = policy_name;
    return retval;
}

std::map<std::string_view, int, std::less<>> Empire::TurnsPoliciesAdopted() const {
    std::map<std::string_view, int, std::less<>> retval;
    for (auto& [policy_name, adoption_info] : m_adopted_policies)
        retval.emplace_hint(retval.end(), policy_name, adoption_info.adoption_turn);
    return retval;
}

bool Empire::PolicyAvailable(std::string_view name) const
{ return m_available_policies.count(name); }

bool Empire::PolicyPrereqsAndExclusionsOK(std::string_view name, int current_turn) const {
    const Policy* policy_to_adopt = GetPolicy(name);
    if (!policy_to_adopt)
        return false;
    const auto& to_adopt_exclusions = policy_to_adopt->Exclusions();

    // is there an exclusion conflict?
    for (auto& [already_adopted_policy_name, ignored] : m_adopted_policies) {
        (void)ignored; // quiet warning
        if (std::any_of(to_adopt_exclusions.begin(), to_adopt_exclusions.end(),
                        [a{already_adopted_policy_name}](auto& x) { return x == a; }))
        {
            // policy to be adopted has an exclusion with an already-adopted policy
            return false;
        }

        const Policy* already_adopted_policy = GetPolicy(already_adopted_policy_name);
        if (!already_adopted_policy) {
            ErrorLogger() << "Couldn't get already adopted policy: " << already_adopted_policy_name;
            continue;
        }
        const auto& adopted_exclusions = already_adopted_policy->Exclusions();
        if (std::any_of(adopted_exclusions.begin(), adopted_exclusions.end(),
                        [name](const auto& x) { return name == x; }))
        {
            // already-adopted policy has an exclusion with the policy to be adopted
            return false;
        }
    }

    // are there any unmet prerequisites (with the initial adopted policies this turn)
    for (const auto& prereq : policy_to_adopt->Prerequisites()) {
        auto it = m_initial_adopted_policies.find(prereq);
        if (it == m_initial_adopted_policies.end() || it->second.adoption_turn >= current_turn)
            return false;
    }

    return true;
}

bool Empire::PolicyAffordable(std::string_view name, const ScriptingContext& context) const {
    const Policy* policy_to_adopt = GetPolicy(name);
    if (!policy_to_adopt) {
        ErrorLogger() << "Empire::PolicyAffordable couldn't find policy to adopt named " << name;
        return false;
    }

    double other_this_turn_adopted_policies_cost = 0.0;
    for (auto& [adopted_policy_name, adoption_info] : m_adopted_policies) {
        if (adoption_info.adoption_turn != context.current_turn)
            continue;
        auto pre_adopted_policy = GetPolicy(adopted_policy_name);
        if (!pre_adopted_policy) {
            ErrorLogger() << "Empire::PolicyAffordable couldn't find policy named " << adopted_policy_name << " that was supposedly already adopted this turn (" << context.current_turn << ")";
            continue;
        }
        TraceLogger() << "Empire::PolicyAffordable : Already adopted policy this turn: " << adopted_policy_name
                      << " with cost " << pre_adopted_policy->AdoptionCost(m_id, context);
        other_this_turn_adopted_policies_cost += pre_adopted_policy->AdoptionCost(m_id, context);
    }
    TraceLogger() << "Empire::PolicyAffordable : Combined already-adopted policies this turn cost " << other_this_turn_adopted_policies_cost;

    // if policy not already adopted at start of this turn, it costs its adoption cost to adopt on this turn
    // if it was adopted at the start of this turn, it doens't cost anything to re-adopt this turn.
    double adoption_cost = 0.0;
    if (m_initial_adopted_policies.find(name) == m_initial_adopted_policies.end())
        adoption_cost = policy_to_adopt->AdoptionCost(m_id, context);

    if (adoption_cost <= 0) {
        TraceLogger() << "Empire::AdoptPolicy: Zero cost policy ignoring influence available...";
        return true;
    } else {
        double total_this_turn_policy_adoption_cost = adoption_cost + other_this_turn_adopted_policies_cost;
        double available_ip = ResourceStockpile(ResourceType::RE_INFLUENCE);

        if (available_ip < total_this_turn_policy_adoption_cost) {
            TraceLogger() << "Empire::AdoptPolicy insufficient ip: " << available_ip
                          << " / " << total_this_turn_policy_adoption_cost << " to adopt additional policy this turn";
            return false;
        } else {
            TraceLogger() << "Empire::AdoptPolicy sufficient IP: " << available_ip
                          << " / " << total_this_turn_policy_adoption_cost << " to adopt additional policy this turn";
            return true;
        }
    }
}

std::map<std::string_view, int, std::less<>> Empire::TotalPolicySlots() const { // TODO: return flat_map
    std::map<std::string_view, int, std::less<>> retval;
    // collect policy slot category meter values and return
    for (auto& cat_and_slot_strings : PolicyCategoriesSlotsMeters()) {
        auto it = std::find_if(m_meters.begin(), m_meters.end(),
                               [&](const auto& e) { return e.first == cat_and_slot_strings.second; });
        if (it == m_meters.end()) {
            ErrorLogger() << "Empire doesn't have policy category slot meter with name: " << cat_and_slot_strings.second;
            continue;
        }
        retval[cat_and_slot_strings.first] = static_cast<int>(it->second.Initial());
    }
    return retval;
}

std::map<std::string_view, int, std::less<>> Empire::EmptyPolicySlots() const {
    // get total slots empire has available
    auto retval = TotalPolicySlots();

    // subtract used policy categories
    for (auto& [ignored, adoption_info] : m_adopted_policies) {
        (void)ignored; // quiet warning
        retval[adoption_info.category]--;
    }

    // return difference
    return retval;
}

Meter* Empire::GetMeter(std::string_view name) {
    auto it = std::find_if(m_meters.begin(), m_meters.end(), [name](const auto& e) { return e.first == name; });
    if (it != m_meters.end())
        return &(it->second);
    else
        return nullptr;
}

const Meter* Empire::GetMeter(std::string_view name) const {
    auto it = std::find_if(m_meters.begin(), m_meters.end(),
                           [name](const auto& e) { return e.first == name; });
    if (it != m_meters.end())
        return &(it->second);
    else
        return nullptr;
}

void Empire::BackPropagateMeters() {
    for (auto& meter : m_meters)
        meter.second.BackPropagate();
}

bool Empire::ResearchableTech(std::string_view name) const {
    const Tech* tech = GetTech(name);
    if (!tech)
        return false;
    const auto& prereqs = tech->Prerequisites();
    return std::all_of(prereqs.begin(), prereqs.end(),
                       [&](const auto& p) -> bool { return m_techs.contains(p); });
}

bool Empire::HasResearchedPrereqAndUnresearchedPrereq(std::string_view name) const {
    const Tech* tech = GetTech(name);
    if (!tech)
        return false;
    const auto& prereqs = tech->Prerequisites();
    bool one_unresearched = std::any_of(prereqs.begin(), prereqs.end(),
                                        [&](const auto& p) -> bool { return !m_techs.contains(p); });
    bool one_researched = std::any_of(prereqs.begin(), prereqs.end(),
                                      [&](const auto& p) -> bool { return m_techs.contains(p); });
    return one_unresearched && one_researched;
}

float Empire::ResearchProgress(const std::string& name, const ScriptingContext& context) const {
    auto it = m_research_progress.find(name);
    if (it == m_research_progress.end())
        return 0.0f;
    const Tech* tech = GetTech(it->first);
    if (!tech)
        return 0.0f;
    float tech_cost = tech->ResearchCost(m_id, context);
    return it->second * tech_cost;
}

bool Empire::TechResearched(const std::string& name) const
{ return m_techs.contains(name); }

TechStatus Empire::GetTechStatus(const std::string& name) const {
    if (TechResearched(name)) return TechStatus::TS_COMPLETE;
    if (ResearchableTech(name)) return TechStatus::TS_RESEARCHABLE;
    if (HasResearchedPrereqAndUnresearchedPrereq(name)) return TechStatus::TS_HAS_RESEARCHED_PREREQ;
    return TechStatus::TS_UNRESEARCHABLE;
}

const std::string& Empire::TopPriorityEnqueuedTech() const {
    if (m_research_queue.empty())
        return EMPTY_STRING;
    auto it = m_research_queue.begin();
    const std::string& tech = it->name;
    return tech;
}

const std::string& Empire::MostExpensiveEnqueuedTech(const ScriptingContext& context) const {
    if (m_research_queue.empty())
        return EMPTY_STRING;
    float biggest_cost = -99999.9f; // arbitrary small number

    const ResearchQueue::Element* best_elem = nullptr;

    for (const auto& elem : m_research_queue) {
        const Tech* tech = GetTech(elem.name);
        if (!tech)
            continue;
        float tech_cost = tech->ResearchCost(m_id, context);
        if (tech_cost > biggest_cost) {
            biggest_cost = tech_cost;
            best_elem = &elem;
        }
    }

    if (best_elem)
        return best_elem->name;
    return EMPTY_STRING;
}

const std::string& Empire::LeastExpensiveEnqueuedTech(const ScriptingContext& context) const {
    if (m_research_queue.empty())
        return EMPTY_STRING;
    float smallest_cost = 999999.9f; // arbitrary large number

    const ResearchQueue::Element* best_elem = nullptr;

    for (const auto& elem : m_research_queue) {
        const Tech* tech = GetTech(elem.name);
        if (!tech)
            continue;
        float tech_cost = tech->ResearchCost(m_id, context);
        if (tech_cost < smallest_cost) {
            smallest_cost = tech_cost;
            best_elem = &elem;
        }
    }

    if (best_elem)
        return best_elem->name;
    return EMPTY_STRING;
}

const std::string& Empire::MostRPSpentEnqueuedTech() const {
    float most_spent = -999999.9f;  // arbitrary small number
    const std::map<std::string, float>::value_type* best_progress = nullptr;

    for (const auto& progress : m_research_progress) {
        const auto& [tech_name, rp_spent] = progress;
        if (!m_research_queue.InQueue(tech_name))
            continue;
        if (rp_spent > most_spent) {
            best_progress = &progress;
            most_spent = rp_spent;
        }
    }

    if (best_progress)
        return best_progress->first;
    return EMPTY_STRING;
}

const std::string& Empire::MostRPCostLeftEnqueuedTech(const ScriptingContext& context) const {
    float most_left = -999999.9f;  // arbitrary small number
    const std::map<std::string, float>::value_type* best_progress = nullptr;

    for (const auto& progress : m_research_progress) {
        const auto& [tech_name, rp_spent] = progress;
        const Tech* tech = GetTech(tech_name);
        if (!tech)
            continue;

        if (!m_research_queue.InQueue(tech_name))
            continue;

        float rp_total_cost = tech->ResearchCost(m_id, context);
        float rp_left = std::max(0.0f, rp_total_cost - rp_spent);

        if (rp_left > most_left) {
            best_progress = &progress;
            most_left = rp_left;
        }
    }

    if (best_progress)
        return best_progress->first;
    return EMPTY_STRING;
}

const std::string& Empire::TopPriorityResearchableTech() const {
    if (m_research_queue.empty())
        return EMPTY_STRING;
    for (const auto& elem : m_research_queue) {
        if (this->ResearchableTech(elem.name))
            return elem.name;
    }
    return EMPTY_STRING;
}

const std::string& Empire::MostExpensiveResearchableTech() const {
    return EMPTY_STRING;    // TODO: IMPLEMENT THIS
}

const std::string& Empire::LeastExpensiveResearchableTech(const ScriptingContext& context) const {
    return EMPTY_STRING;    // TODO: IMPLEMENT THIS
}

const std::string& Empire::MostRPSpentResearchableTech() const {
    return EMPTY_STRING;    // TODO: IMPLEMENT THIS
}

const std::string& Empire::MostRPCostLeftResearchableTech(const ScriptingContext& context) const {
    return EMPTY_STRING;    // TODO: IMPLEMENT THIS
}

bool Empire::BuildingTypeAvailable(const std::string& name) const
{ return m_available_building_types.contains(name); }

std::vector<int> Empire::AvailableShipDesigns(const Universe& universe) const {
    // create new map containing all ship designs that are available
    std::vector<int> retval;
    retval.reserve(m_known_ship_designs.size());
    std::copy_if(m_known_ship_designs.begin(), m_known_ship_designs.end(),
                 std::back_inserter(retval), [&universe, this](int design_id)
                 { return ShipDesignAvailable(design_id, universe); });
    std::sort(retval.begin(), retval.end());
    auto unique_it = std::unique(retval.begin(), retval.end());
    retval.erase(unique_it, retval.end());
    return retval;
}

bool Empire::ShipDesignAvailable(int ship_design_id, const Universe& universe) const {
    const ShipDesign* design = universe.GetShipDesign(ship_design_id);
    return design ? ShipDesignAvailable(*design) : false;
}

bool Empire::ShipDesignAvailable(const ShipDesign& design) const {
    if (!design.Producible()) return false;

    // design is kept, but still need to verify that it is buildable at this time.  Part or hull tech
    // requirements might prevent it from being built.
    for (const auto& name : design.Parts()) {
        if (name.empty())
            continue;   // empty slot can't be unavailable
        if (!ShipPartAvailable(name))
            return false;
    }
    if (!ShipHullAvailable(design.Hull()))
        return false;

    // if there are no reasons the design isn't available, then by default it is available
    return true;
}

bool Empire::ShipDesignKept(int ship_design_id) const
{ return m_known_ship_designs.contains(ship_design_id); }

bool Empire::ShipPartAvailable(const std::string& name) const
{ return m_available_ship_parts.contains(name); }

bool Empire::ShipHullAvailable(const std::string& name) const
{ return m_available_ship_hulls.contains(name); }

float Empire::ProductionStatus(int i, const ScriptingContext& context) const {
    if (0 > i || i >= static_cast<int>(m_production_queue.size()))
        return -1.0f;
    float item_progress = m_production_queue[i].progress;
    [[maybe_unused]] auto [item_cost, item_time] = m_production_queue[i].ProductionCostAndTime(context);
    (void)item_time; // quiet unused variable warning
    return item_progress * item_cost * m_production_queue[i].blocksize;
}

bool Empire::HasExploredSystem(int ID) const
{ return m_explored_systems.contains(ID); }

bool Empire::ProducibleItem(BuildType build_type, int location_id,
                            const ScriptingContext& context) const
{
    if (build_type == BuildType::BT_SHIP)
        throw std::invalid_argument("Empire::ProducibleItem was passed BuildType BT_SHIP with no further parameters, but ship designs are tracked by number");

    if (build_type == BuildType::BT_BUILDING)
        throw std::invalid_argument("Empire::ProducibleItem was passed BuildType BT_BUILDING with no further parameters, but buildings are tracked by name");

    if (location_id == INVALID_OBJECT_ID)
        return false;

    // must own the production location...
    auto location = context.ContextObjects().getRaw(location_id);
    if (!location) {
        WarnLogger() << "Empire::ProducibleItem for BT_STOCKPILE unable to get location object with id " << location_id;
        return false;
    }

    if (!location->OwnedBy(m_id))
        return false;

    if (location->ObjectType() != UniverseObjectType::OBJ_PLANET)
        return false;

    if (build_type == BuildType::BT_STOCKPILE) {
        return true;

    } else {
        ErrorLogger() << "Empire::ProducibleItem was passed an invalid BuildType";
        return false;
    }
}

bool Empire::ProducibleItem(BuildType build_type, const std::string& name, int location,
                            const ScriptingContext& context) const
{
    // special case to check for ships being passed with names, not design ids
    if (build_type == BuildType::BT_SHIP)
        throw std::invalid_argument("Empire::ProducibleItem was passed BuildType BT_SHIP with a name, but ship designs are tracked by number");

    if (build_type == BuildType::BT_STOCKPILE)
        throw std::invalid_argument("Empire::ProducibleItem was passed BuildType BT_STOCKPILE with a name, but the stockpile does not need an identification");

    if (build_type == BuildType::BT_BUILDING && !BuildingTypeAvailable(name))
        return false;

    const auto* building_type = GetBuildingType(name);
    if (!building_type || !building_type->Producible())
        return false;

    auto build_location = context.ContextObjects().get(location);
    if (!build_location)
        return false;

    if (build_type == BuildType::BT_BUILDING) {
        // specified location must be a valid production location for that building type
        return building_type->ProductionLocation(m_id, location, context);

    } else {
        ErrorLogger() << "Empire::ProducibleItem was passed an invalid BuildType";
        return false;
    }
}

bool Empire::ProducibleItem(BuildType build_type, int design_id, int location,
                            const ScriptingContext& context) const
{
    // special case to check for buildings being passed with ids, not names
    if (build_type == BuildType::BT_BUILDING)
        throw std::invalid_argument("Empire::ProducibleItem was passed BuildType BT_BUILDING with a design id number, but buildings are tracked by name");

    if (build_type == BuildType::BT_STOCKPILE)
        throw std::invalid_argument("Empire::ProducibleItem was passed BuildType BT_STOCKPILE with a design id, but the stockpile does not need an identification");

    if (build_type == BuildType::BT_SHIP && !ShipDesignAvailable(design_id, context.ContextUniverse()))
        return false;

    // design must be known to this empire
    const ShipDesign* ship_design = context.ContextUniverse().GetShipDesign(design_id);
    if (!ship_design || !ship_design->Producible())
        return false;

    const auto build_location = context.ContextObjects().getRaw(location);
    if (!build_location) return false;

    if (build_type == BuildType::BT_SHIP) {
        // specified location must be a valid production location for this design
        return ship_design->ProductionLocation(m_id, location, context);

    } else {
        ErrorLogger() << "Empire::ProducibleItem was passed an invalid BuildType";
        return false;
    }
}

bool Empire::ProducibleItem(const ProductionQueue::ProductionItem& item, int location,
                            const ScriptingContext& context) const
{
    if (item.build_type == BuildType::BT_BUILDING)
        return ProducibleItem(item.build_type, item.name, location, context);
    else if (item.build_type == BuildType::BT_SHIP)
        return ProducibleItem(item.build_type, item.design_id, location, context);
    else if (item.build_type == BuildType::BT_STOCKPILE)
        return ProducibleItem(item.build_type, location, context);
    else
        throw std::invalid_argument("Empire::ProducibleItem was passed a ProductionItem with an invalid BuildType");
}

bool Empire::EnqueuableItem(BuildType build_type, const std::string& name,
                            int location, const ScriptingContext& context) const
{
    if (build_type != BuildType::BT_BUILDING)
        return false;

    const auto* building_type = GetBuildingType(name);
    if (!building_type || !building_type->Producible())
        return false;

    auto build_location = context.ContextObjects().get(location);
    if (!build_location)
        return false;

    // specified location must be a valid production location for that building type
    return building_type->EnqueueLocation(m_id, location, context);
}

bool Empire::EnqueuableItem(const ProductionQueue::ProductionItem& item, int location,
                            const ScriptingContext& context) const
{
    if (item.build_type == BuildType::BT_BUILDING)
        return EnqueuableItem(item.build_type, item.name, location, context);
    else if (item.build_type == BuildType::BT_SHIP)      // ships don't have a distinction between enqueuable and producible
        return ProducibleItem(item.build_type, item.design_id, location, context);
    else if (item.build_type == BuildType::BT_STOCKPILE) // stockpile does not have a distinction between enqueuable and producible
        return ProducibleItem(item.build_type, location, context);
    else
        throw std::invalid_argument("Empire::ProducibleItem was passed a ProductionItem with an invalid BuildType");
}

int Empire::NumSitRepEntries(int turn) const {
    if (turn == INVALID_GAME_TURN)
        return m_sitrep_entries.size();
    int count = 0;
    for (const SitRepEntry& sitrep : m_sitrep_entries)
        if (sitrep.GetTurn() == turn)
            count++;
    return count;
}

void Empire::Eliminate(EmpireManager& empires, int current_turn) {
    m_eliminated = true;

    for (auto& entry : empires)
        entry.second->AddSitRepEntry(CreateEmpireEliminatedSitRep(EmpireID(), current_turn));

    // some Empire data not cleared when eliminating since it might be useful
    // to remember later, and having it doesn't hurt anything (as opposed to
    // the production queue that might actually cause some problems if left
    // uncleared after elimination

    m_capital_id = INVALID_OBJECT_ID;
    // m_newly_researched_techs
    // m_techs
    m_research_queue.clear();
    m_research_progress.clear();
    m_production_queue.clear();
    m_influence_queue.clear();

    // m_available_building_types;
    // m_available_ship_parts;
    // m_available_ship_hulls;
    // m_explored_systems;
    // m_known_ship_designs;
    m_sitrep_entries.clear();
    m_industry_pool.SetObjects({});
    m_research_pool.SetObjects({});
    m_influence_pool.SetObjects({});
    m_population_pool.SetPopCenters({});

    // m_ship_names_used;
    m_supply_system_ranges.clear();
    m_supply_unobstructed_systems.clear();
}

void Empire::Win(const std::string& reason, const EmpireManager::container_type& empires, int current_turn) {
    if (m_victories.insert(reason).second) {
        for (auto& entry : empires)
            entry.second->AddSitRepEntry(CreateVictorySitRep(reason, EmpireID(), current_turn));
    }
}

void Empire::SetReady(bool ready)
{ m_ready = ready; }

void Empire::AutoTurnSetReady() {
    if (m_auto_turn_count > 0) {
        m_auto_turn_count --;
    }
    SetReady(m_auto_turn_count != 0);
}

void Empire::SetAutoTurn(int turns_count)
{ m_auto_turn_count = turns_count; }

void Empire::SetLastTurnReceived(int last_turn_received) noexcept
{ m_last_turn_received = last_turn_received; }

void Empire::UpdateSystemSupplyRanges(const std::set<int>& known_objects, const ObjectMap& objects) {
    TraceLogger(supply) << "Empire::UpdateSystemSupplyRanges() for empire " << this->Name();
    m_supply_system_ranges.clear();

    // as of this writing, only planets can generate supply propagation
    std::vector<const UniverseObject*> owned_planets;
    owned_planets.reserve(known_objects.size());
    for (auto* planet: objects.findRaw<Planet>(known_objects)) {
        if (!planet)
            continue;
        if (planet->OwnedBy(this->EmpireID()))
            owned_planets.push_back(planet);
    }

    //std::cout << "... empire owns " << owned_planets.size() << " planets" << std::endl;
    for (auto* obj : owned_planets) {
        //std::cout << "... considering owned planet: " << obj->Name() << std::endl;

        // ensure object is within a system, from which it can distribute supplies
        int system_id = obj->SystemID();
        if (system_id == INVALID_OBJECT_ID)
            continue;   // TODO: consider future special case if current object is itself a system

        // check if object has a supply meter
        if (obj->GetMeter(MeterType::METER_SUPPLY)) {
            // get resource supply range for next turn for this object
            float supply_range = obj->GetMeter(MeterType::METER_SUPPLY)->Initial();

            // if this object can provide more supply range than the best previously checked object in this system, record its range as the new best for the system
            auto system_it = m_supply_system_ranges.find(system_id);  // try to find a previous entry for this system's supply range
            if (system_it == m_supply_system_ranges.end() || supply_range > system_it->second) {// if there is no previous entry, or the previous entry is shorter than the new one, add or replace the entry
                //std::cout << " ... object " << obj->Name() << " has resource supply range: " << resource_supply_range << std::endl;
                m_supply_system_ranges[system_id] = supply_range;
            }
        }
    }
}

void Empire::UpdateSystemSupplyRanges(const Universe& universe) {
    if (AppEmpireID() != ALL_EMPIRES)
        ErrorLogger() << "Empire::UpdateSystemSupplyRanges unexpectedly called by an App with a specific empire ID";
    const ObjectMap& empire_known_objects{AppEmpireID() == ALL_EMPIRES ?
        universe.EmpireKnownObjects(this->EmpireID()) : universe.Objects()};

    // get ids of objects partially or better visible to this empire.
    const auto& known_destroyed_objects = universe.EmpireKnownDestroyedObjectIDs(this->EmpireID());

    std::set<int> known_objects_set;

    // exclude objects known to have been destroyed (or rather, include ones that aren't known by this empire to be destroyed)
    for (const auto& obj : empire_known_objects.allRaw())
        if (!known_destroyed_objects.contains(obj->ID()))
            known_objects_set.insert(obj->ID());
    UpdateSystemSupplyRanges(known_objects_set, empire_known_objects);
}

void Empire::UpdateUnobstructedFleets(ObjectMap& objects, const std::unordered_set<int>& known_destroyed_objects) {
    for (const auto& system : objects.find<System>(m_supply_unobstructed_systems)) {
        if (!system)
            continue;

        for (auto* fleet : objects.findRaw<Fleet>(system->FleetIDs())) {
            if (known_destroyed_objects.contains(fleet->ID()))
                continue;
            if (fleet->OwnedBy(m_id))
                fleet->SetArrivalStarlane(system->ID());
        }
    }
}

void Empire::UpdateSupplyUnobstructedSystems(const ScriptingContext& context, bool precombat) {
    const Universe& universe = context.ContextUniverse();

    // get ids of systems partially or better visible to this empire.
    // TODO: make a UniverseObjectVisitor for objects visible to an empire at a specified visibility or greater
    const auto& known_destroyed_objects = universe.EmpireKnownDestroyedObjectIDs(this->EmpireID());

    std::set<int> known_systems_set;

    // exclude systems known to have been destroyed (or rather, include ones that aren't known to be destroyed)
    for (const auto& sys : universe.EmpireKnownObjects(this->EmpireID()).allRaw<System>())
        if (!known_destroyed_objects.contains(sys->ID()))
            known_systems_set.insert(sys->ID());
    UpdateSupplyUnobstructedSystems(context, known_systems_set, precombat);
}

void Empire::UpdateSupplyUnobstructedSystems(const ScriptingContext& context,
                                             const std::set<int>& known_systems,
                                             bool precombat)
{
    TraceLogger(supply) << "UpdateSupplyUnobstructedSystems (allowing supply propagation) for empire " << m_id;
    m_supply_unobstructed_systems.clear();

    const Universe& universe{context.ContextUniverse()};
    const ObjectMap& objects{context.ContextObjects()};

    // get systems with historically at least partial visibility
    std::set<int> systems_with_at_least_partial_visibility_at_some_point;
    for (int system_id : known_systems) {
        const auto& vis_turns = universe.GetObjectVisibilityTurnMapByEmpire(system_id, m_id);
        if (vis_turns.contains(Visibility::VIS_PARTIAL_VISIBILITY))
            systems_with_at_least_partial_visibility_at_some_point.insert(system_id);
    }

    // get all fleets, or just those visible to this client's empire
    const auto& known_destroyed_objects = universe.EmpireKnownDestroyedObjectIDs(this->EmpireID());

    // get empire supply ranges
    std::map<int, std::map<int, float>> empire_system_supply_ranges;
    for (const auto& entry : context.Empires()) {
        const auto& empire = entry.second;
        empire_system_supply_ranges[entry.first] = empire->SystemSupplyRanges();
    }

    // find systems that contain fleets that can either maintain supply or block supply.
    // to affect supply in either manner, a fleet must be armed & aggressive, & must be not
    // trying to depart the systme.  Qualifying enemy fleets will blockade if no friendly fleets
    // are present, or if the friendly fleets were already blockade-restricted and the enemy
    // fleets were not (meaning that the enemy fleets were continuing an existing blockade)
    // Friendly fleets can preserve available starlane accesss even if they are trying to leave the system

    // Unrestricted lane access (i.e, (fleet->ArrivalStarlane() == system->ID()) ) is used as a proxy for
    // order of arrival -- if an enemy has unrestricted lane access and you don't, they must have arrived
    // before you, or be in cahoots with someone who did.
    std::set<int> systems_containing_friendly_fleets;
    std::set<int> systems_with_lane_preserving_fleets;
    std::set<int> unrestricted_friendly_systems;
    std::set<int> systems_containing_obstructing_objects;
    std::set<int> unrestricted_obstruction_systems;
    for (auto* fleet : objects.allRaw<Fleet>()) {
        int system_id = fleet->SystemID();
        if (system_id == INVALID_OBJECT_ID) {
            continue;   // not in a system, so can't affect system obstruction
        } else if (known_destroyed_objects.contains(fleet->ID())) {
            continue; //known to be destroyed so can't affect supply, important just in case being updated on client side
        }

        TraceLogger(supply) << "Fleet " << fleet->ID() << " is in system " << system_id
                            << " with next system " << fleet->NextSystemID()
                            << " and is owned by " << fleet->Owner()
                            << " can damage ships: " << fleet->CanDamageShips(context)
                            << " and obstructive: " << fleet->Obstructive();
        if (fleet->CanDamageShips(context) && fleet->Obstructive()) {
            if (fleet->OwnedBy(m_id)) {
                if (fleet->NextSystemID() == INVALID_OBJECT_ID || fleet->NextSystemID() == fleet->SystemID()) {
                    systems_containing_friendly_fleets.insert(system_id);
                    if (fleet->ArrivalStarlane() == system_id)
                        unrestricted_friendly_systems.insert(system_id);
                    else
                        systems_with_lane_preserving_fleets.insert(system_id);
                }
            } else if (fleet->NextSystemID() == INVALID_OBJECT_ID || fleet->NextSystemID() == fleet->SystemID()) {
                int fleet_owner = fleet->Owner();
                bool fleet_at_war = fleet_owner == ALL_EMPIRES ||
                                    context.ContextDiploStatus(m_id, fleet_owner) == DiplomaticStatus::DIPLO_WAR;
                // newly created ships are not allowed to block supply since they have not even potentially gone
                // through a combat round at the present location.  Potential sources for such new ships are monsters
                // created via Effect.  (Ships/fleets constructed by empires are currently created at a later stage of
                // turn processing, but even if such were moved forward they should be similarly restricted.)  For
                // checks after combat and prior to turn advancement, we check against age zero here.  For checks
                // after turn advancement but prior to combat we check against age 1.  Because the
                // fleets themselves may be created and/or destroyed purely as organizational matters, we check ship
                // age not fleet age.
                int cutoff_age = precombat ? 1 : 0;
                if (fleet_at_war && fleet->MaxShipAgeInTurns(objects, context.current_turn) > cutoff_age) {
                    systems_containing_obstructing_objects.insert(system_id);
                    if (fleet->ArrivalStarlane() == system_id)
                        unrestricted_obstruction_systems.insert(system_id);
                }
            }
        }
    }

    TraceLogger(supply) << "Empire::UpdateSupplyUnobstructedSystems systems with obstructing objects for empire " << m_id << " : " << [&]() {
        std::stringstream ss;
        for (int obj_id : systems_containing_obstructing_objects)
            ss << obj_id << ", ";
        return ss.str();
    }();

    DebugLogger() << "Preserved System-Lanes for empire " << m_name << " (" << m_id << ") : " << [&]() {
        std::stringstream ss2;
        for (const auto& sys_lanes : m_preserved_system_exit_lanes) {
            ss2 << "[Sys: " << sys_lanes.first << " : (";
            for (auto lane : sys_lanes.second)
                ss2 << lane << " ";
            ss2 << ")]  ";
        }
        return ss2.str();
    }();

    DebugLogger() << "Systems with lane-preserving fleets for empire " << m_name << " (" << m_id << ") : " << [&]() {
        std::stringstream ss3;
        for (auto sys_id : systems_with_lane_preserving_fleets)
            ss3 << sys_id << ", ";
        return ss3.str();
    }();


    // check each potential supplyable system for whether it can propagate supply.
    for (const auto& sys : objects.find<System>(known_systems)) {
        if (!sys)
            continue;

        // has empire ever seen this system with partial or better visibility?
        if (!systems_with_at_least_partial_visibility_at_some_point.contains(sys->ID())) {
            TraceLogger(supply) << "System " << sys->Name() << " (" << sys->ID() << ") has never been seen";
            continue;
        }

        // if system is explored, then whether it can propagate supply depends
        // on what friendly / enemy ships and planets are in the system

        if (unrestricted_friendly_systems.contains(sys->ID())) {
            // in unrestricted friendly systems, supply can propagate
            m_supply_unobstructed_systems.insert(sys->ID());
            TraceLogger(supply) << "System " << sys->Name() << " (" << sys->ID() << ") +++ is unrestricted and friendly";

        } else if (systems_containing_friendly_fleets.contains(sys->ID())) {
            // if there are unrestricted friendly ships, and no unrestricted enemy fleets, supply can propagate
            if (!unrestricted_obstruction_systems.contains(sys->ID())) {
                m_supply_unobstructed_systems.insert(sys->ID());
                TraceLogger(supply) << "System " << sys->Name() << " (" << sys->ID() << ") +++ has friendly fleets and no obstructions";
            } else {
                TraceLogger(supply) << "System " << sys->Name() << " (" << sys->ID() << ") --- is has friendly fleets but has obstructions";
            }

        } else if (!systems_containing_obstructing_objects.contains(sys->ID())) {
            // if there are no friendly fleets or obstructing enemy fleets, supply can propagate
            m_supply_unobstructed_systems.insert(sys->ID());
            TraceLogger(supply) << "System " << sys->Name() << " (" << sys->ID() << ") +++ has no obstructing objects";

        } else if (!systems_with_lane_preserving_fleets.contains(sys->ID())) {
            // if there are obstructing enemy fleets but no friendly fleets that could maintain
            // lane access, supply cannot propagate and this empire's available system exit
            TraceLogger(supply) << "System " << sys->Name() << " (" << sys->ID() << ") --- has no lane preserving fleets";

            // lanes for this system are cleared
            if (!m_preserved_system_exit_lanes[sys->ID()].empty()) {
                std::stringstream ssca;
                ssca << "Empire::UpdateSupplyUnobstructedSystems clearing preserved lanes for system ("
                     << sys->ID() << "); available lanes were:";
                for (int system_id : m_preserved_system_exit_lanes[sys->ID()])
                    ssca << system_id << ", ";
                TraceLogger(supply) << ssca.str();
            }
            m_preserved_system_exit_lanes[sys->ID()].clear();

        } else {
            TraceLogger(supply) << "Empire::UpdateSupplyUnobstructedSystems : Restricted system " << sys->ID() << " with no friendly fleets, no obustrcting enemy fleets, and no lane-preserving fleets";
        }
    }
}

void Empire::RecordPendingLaneUpdate(int start_system_id, int dest_system_id, const ObjectMap& objects) {
    if (!m_supply_unobstructed_systems.contains(start_system_id)) {
        m_pending_system_exit_lanes[start_system_id].insert(dest_system_id);
    } else { // if the system is unobstructed, mark all its lanes as avilable
        for (const auto& lane : objects.getRaw<System>(start_system_id)->StarlanesWormholes())
            m_pending_system_exit_lanes[start_system_id].insert(lane.first); // will add both starlanes and wormholes
    }
}

void Empire::UpdatePreservedLanes() {
    for (auto& system : m_pending_system_exit_lanes)
        m_preserved_system_exit_lanes[system.first].merge(system.second); //insert(system.second.begin(), system.second.end());
    m_pending_system_exit_lanes.clear();
}

bool Empire::PreservedLaneTravel(int start_system_id, int dest_system_id) const {
    auto find_it = m_preserved_system_exit_lanes.find(start_system_id);
    return find_it != m_preserved_system_exit_lanes.end()
        && find_it->second.contains(dest_system_id);
}

std::set<int> Empire::ExploredSystems() const {
    std::set<int> retval;
    for (const auto& entry : m_explored_systems)
        retval.insert(entry.first);
    return retval;
}

int Empire::TurnSystemExplored(int system_id) const {
    auto it = m_explored_systems.find(system_id);
    if (it == m_explored_systems.end())
        return INVALID_GAME_TURN;
    return it->second;
}

std::map<int, std::set<int>> Empire::KnownStarlanes(const Universe& universe) const {
    // compile starlanes leading into or out of each system
    std::map<int, std::set<int>> retval;

    TraceLogger(supply) << "Empire::KnownStarlanes for empire " << m_id;

    auto& known_destroyed_objects = universe.EmpireKnownDestroyedObjectIDs(this->EmpireID());
    for (const auto& sys : universe.Objects().allRaw<System>()) {
        int start_id = sys->ID();
        TraceLogger(supply) << "system " << start_id << " has up to " << sys->StarlanesWormholes().size() << " lanes / wormholes";

        // exclude lanes starting at systems known to be destroyed
        if (known_destroyed_objects.contains(start_id)) {
            TraceLogger(supply) << "system " << start_id << " known destroyed, so lanes from it are unknown";
            continue;
        }

        for (const auto& lane : sys->StarlanesWormholes()) {
            int end_id = lane.first;
            bool is_wormhole = lane.second;
            if (is_wormhole || known_destroyed_objects.contains(end_id))
                continue;   // is a wormhole, not a starlane, or is connected to a known destroyed system
            retval[start_id].insert(end_id);
            retval[end_id].insert(start_id);
        }

        TraceLogger(supply) << "system " << start_id << " had " << retval[start_id].size() << " known lanes";
    }

    TraceLogger(supply) << "Total of " << retval.size() << " systems had known lanes";
    return retval;
}

std::map<int, std::set<int>> Empire::VisibleStarlanes(const Universe& universe) const {
    std::map<int, std::set<int>> retval;   // compile starlanes leading into or out of each system

    const ObjectMap& objects = universe.Objects();

    for (const auto& sys : objects.allRaw<System>()) {
        int start_id = sys->ID();

        // is system visible to this empire?
        if (universe.GetObjectVisibilityByEmpire(start_id, m_id) <= Visibility::VIS_NO_VISIBILITY)
            continue;

        // get system's visible lanes for this empire
        for (auto& [other_end_id, is_wormhole] : sys->VisibleStarlanesWormholes(m_id, universe)) {
            if (is_wormhole)
                continue;   // is a wormhole, not a starlane
            retval[start_id].insert(other_end_id);
            retval[other_end_id].insert(start_id);
        }
    }

    return retval;
}

float Empire::ProductionPoints() const
{ return ResourceOutput(ResourceType::RE_INDUSTRY); }

const ResourcePool& Empire::GetResourcePool(ResourceType type) const {
    switch (type) {
    case ResourceType::RE_INDUSTRY: return m_industry_pool; break;
    case ResourceType::RE_RESEARCH: return m_research_pool; break;
    case ResourceType::RE_INFLUENCE: return m_influence_pool; break;
    default:
        throw std::invalid_argument("Empire::GetResourcePool passed invalid ResourceType");
    }
}

float Empire::ResourceStockpile(ResourceType type) const
{ return GetResourcePool(type).Stockpile(); }

float Empire::ResourceOutput(ResourceType type) const
{ return GetResourcePool(type).TotalOutput(); }

float Empire::ResourceAvailable(ResourceType type) const
{ return GetResourcePool(type).TotalAvailable(); }

float Empire::Population() const
{ return m_population_pool.Population(); }

void Empire::SetResourceStockpile(ResourceType type, float stockpile) {
    switch (type) {
    case ResourceType::RE_INDUSTRY: m_industry_pool.SetStockpile(stockpile); break;
    case ResourceType::RE_RESEARCH: m_research_pool.SetStockpile(stockpile); break;
    case ResourceType::RE_INFLUENCE: m_influence_pool.SetStockpile(stockpile); break;
    default:
        throw std::invalid_argument("Empire::SetResourceStockpile passed invalid ResourceType");
    }
}

void Empire::PlaceTechInQueue(const std::string& name, int pos) {
    // do not add tech that is already researched
    if (name.empty() || TechResearched(name) || m_techs.contains(name) || m_newly_researched_techs.contains(name))
        return;
    const Tech* tech = GetTech(name);
    if (!tech || !tech->Researchable())
        return;

    auto it = m_research_queue.find(name);

    if (pos < 0 || static_cast<int>(m_research_queue.size()) <= pos) {
        // default to putting at end
        bool paused = false;
        if (it != m_research_queue.end()) {
            paused = it->paused;
            m_research_queue.erase(it);
        }
        m_research_queue.push_back(name, paused);
    } else {
        // put at requested position
        if (it < m_research_queue.begin() + pos)
            --pos;
        bool paused = false;
        if (it != m_research_queue.end()) {
            paused = it->paused;
            m_research_queue.erase(it);
        }
        m_research_queue.insert(m_research_queue.begin() + pos, name, paused);
    }
}

void Empire::RemoveTechFromQueue(const std::string& name) {
    auto it = m_research_queue.find(name);
    if (it != m_research_queue.end())
        m_research_queue.erase(it);
}

void Empire::PauseResearch(const std::string& name) {
    auto it = m_research_queue.find(name);
    if (it != m_research_queue.end())
        it->paused = true;
}

void Empire::ResumeResearch(const std::string& name){
    auto it = m_research_queue.find(name);
    if (it != m_research_queue.end())
        it->paused = false;
}

void Empire::SetTechResearchProgress(const std::string& name, float progress,
                                     const ScriptingContext& context)
{
    const Tech* tech = GetTech(name);
    if (!tech) {
        ErrorLogger() << "Empire::SetTechResearchProgress no such tech as: " << name;
        return;
    }
    if (TechResearched(name))
        return; // can't affect already-researched tech

    // set progress
    float clamped_progress = std::min(1.0f, std::max(0.0f, progress));
    m_research_progress[name] = clamped_progress;

    // if tech is complete, ensure it is on the queue, so it will be researched next turn
    if (clamped_progress >= tech->ResearchCost(m_id, context) &&
        !m_research_queue.InQueue(name))
    { m_research_queue.push_back(name); }

    // don't just give tech to empire, as another effect might reduce its progress before end of turn
}

constexpr unsigned int MAX_PROD_QUEUE_SIZE = 500;

void Empire::PlaceProductionOnQueue(const ProductionQueue::ProductionItem& item,
                                    boost::uuids::uuid uuid, int number,
                                    int blocksize, int location, int pos)
{
    if (m_production_queue.size() >= MAX_PROD_QUEUE_SIZE) {
        ErrorLogger() << "Empire::PlaceProductionOnQueue() : Maximum queue size reached. Aborting enqueue";
        return;
    }

    const ScriptingContext context;

    if (item.build_type == BuildType::BT_BUILDING) {
        // only buildings have a distinction between enqueuable and producible...
        if (!EnqueuableItem(BuildType::BT_BUILDING, item.name, location, context)) {
            ErrorLogger() << "Empire::PlaceProductionOnQueue() : Attempted to place non-enqueuable item in queue: build_type: Building"
                          << "  name: " << item.name << "  location: " << location;
            return;
        }
        if (!ProducibleItem(BuildType::BT_BUILDING, item.name, location, context)) {
            ErrorLogger() << "Empire::PlaceProductionOnQueue() : Placed a non-buildable item in queue: build_type: Building"
                          << "  name: " << item.name << "  location: " << location;
            return;
        }

    } else if (item.build_type == BuildType::BT_SHIP) {
        if (!ProducibleItem(BuildType::BT_SHIP, item.design_id, location, context)) {
            ErrorLogger() << "Empire::PlaceProductionOnQueue() : Placed a non-buildable item in queue: build_type: Ship"
                          << "  design_id: " << item.design_id << "  location: " << location;
            return;
        }

    } else if (item.build_type == BuildType::BT_STOCKPILE) {
        if (!ProducibleItem(BuildType::BT_STOCKPILE, location, context)) {
            ErrorLogger() << "Empire::PlaceProductionOnQueue() : Placed a non-buildable item in queue: build_type: Stockpile"
                          << "  location: " << location;
            return;
        }

    } else {
        throw std::invalid_argument("Empire::PlaceProductionOnQueue was passed a ProductionQueue::ProductionItem with an invalid BuildType");
    }

    ProductionQueue::Element elem{item, m_id, uuid, number, number, blocksize, location};
    if (pos < 0 || static_cast<int>(m_production_queue.size()) <= pos)
        m_production_queue.push_back(std::move(elem));
    else
        m_production_queue.insert(m_production_queue.begin() + pos, std::move(elem));
}

void Empire::SetProductionQuantityAndBlocksize(int index, int quantity, int blocksize) {
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index)
        throw std::runtime_error("Empire::SetProductionQuantity() : Attempted to adjust the quantity of items to be built in a nonexistent production queue item.");
    DebugLogger() << "Empire::SetProductionQuantityAndBlocksize() called for item "<< m_production_queue[index].item.name << "with new quant " << quantity << " and new blocksize " << blocksize;
    if (quantity < 1)
        throw std::runtime_error("Empire::SetProductionQuantity() : Attempted to set the quantity of a build run to a value less than zero.");
    if (m_production_queue[index].item.build_type == BuildType::BT_BUILDING && ((1 < quantity) || ( 1 < blocksize) ))
        throw std::runtime_error("Empire::SetProductionQuantity() : Attempted to build more than one instance of a building in the same build run.");
    int original_quantity = m_production_queue[index].remaining;
    //int original_blocksize = m_production_queue[index].blocksize;
    blocksize = std::max(1, blocksize);
    m_production_queue[index].remaining = quantity;
    m_production_queue[index].ordered += quantity - original_quantity;
    m_production_queue[index].blocksize = blocksize;
    //std::cout << "original block size: " << original_blocksize << "  new blocksize: " << blocksize << "  memory blocksize: " << m_production_queue[index].blocksize_memory << std::endl;
    if (blocksize <= m_production_queue[index].blocksize_memory) {
        // if reducing block size, progress on retained portion is unchanged.
        // if increasing block size, progress is proportionally reduced, unless undoing a recent reduction in block size
        m_production_queue[index].progress = m_production_queue[index].progress_memory;
    } else {
        m_production_queue[index].progress = m_production_queue[index].progress_memory * m_production_queue[index].blocksize_memory / blocksize;
    }
}

void Empire::SplitIncompleteProductionItem(int index, boost::uuids::uuid uuid) {
    DebugLogger() << "Empire::SplitIncompleteProductionItem() called for index " << index;
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index)
        throw std::runtime_error("Empire::SplitIncompleteProductionItem() : Attempted to adjust the quantity of items to be built in a nonexistent production queue item.");
    if (m_production_queue[index].item.build_type == BuildType::BT_BUILDING)
        throw std::runtime_error("Empire::SplitIncompleteProductionItem() : Attempted to split a production item that is not a ship.");

    ProductionQueue::Element& elem = m_production_queue[index];

    // if "splitting" an item with just 1 remaining, do nothing
    if (elem.remaining <= 1)
        return;

    // add duplicate
    int new_item_quantity = elem.remaining - 1;
    elem.remaining = 1; // reduce remaining on specified to 1
    PlaceProductionOnQueue(elem.item, uuid, new_item_quantity, elem.blocksize, elem.location, index + 1);
}

void Empire::DuplicateProductionItem(int index, boost::uuids::uuid uuid) {
    DebugLogger() << "Empire::DuplicateProductionItem() called for index " << index << " with new UUID: " << boost::uuids::to_string(uuid);
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index)
        throw std::runtime_error("Empire::DuplicateProductionItem() : Attempted to adjust the quantity of items to be built in a nonexistent production queue item.");

    auto& elem = m_production_queue[index];
    PlaceProductionOnQueue(elem.item, uuid, elem.remaining, elem.blocksize, elem.location, index + 1);
}

void Empire::SetProductionRallyPoint(int index, int rally_point_id) {
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index)
        throw std::runtime_error("Empire::SetProductionQuantity() : Attempted to adjust the quantity of items to be built in a nonexistent production queue item.");
    m_production_queue[index].rally_point_id = rally_point_id;
}

void Empire::SetProductionQuantity(int index, int quantity) {
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index)
        throw std::runtime_error("Empire::SetProductionQuantity() : Attempted to adjust the quantity of items to be built in a nonexistent production queue item.");
    if (quantity < 1)
        throw std::runtime_error("Empire::SetProductionQuantity() : Attempted to set the quantity of a build run to a value less than zero.");
    if (m_production_queue[index].item.build_type == BuildType::BT_BUILDING && 1 < quantity)
        throw std::runtime_error("Empire::SetProductionQuantity() : Attempted to build more than one instance of a building in the same build run.");
    int original_quantity = m_production_queue[index].remaining;
    m_production_queue[index].remaining = quantity;
    m_production_queue[index].ordered += quantity - original_quantity;
}

void Empire::MoveProductionWithinQueue(int index, int new_index) {
    if (index < new_index)
        --new_index;
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index ||
        new_index < 0 || static_cast<int>(m_production_queue.size()) <= new_index)
    {
        DebugLogger() << "Empire::MoveProductionWithinQueue index: " << index << "  new index: "
                      << new_index << "  queue size: " << m_production_queue.size();
        ErrorLogger() << "Attempted to move a production queue item to or from an invalid index.";
        return;
    }
    auto build = m_production_queue[index];
    m_production_queue.erase(index);
    m_production_queue.insert(m_production_queue.begin() + new_index, build);
}

void Empire::RemoveProductionFromQueue(int index) {
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index) {
        DebugLogger() << "Empire::RemoveProductionFromQueue index: " << index << "  queue size: " << m_production_queue.size();
        ErrorLogger() << "Attempted to delete a production queue item with an invalid index.";
        return;
    }
    m_production_queue.erase(index);
}

void Empire::MarkToBeRemoved(int index) {
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index) {
        DebugLogger() << "Empire::MarkToBeRemoved index: " << index << "  queue size: " << m_production_queue.size();
        ErrorLogger() << "Attempted to mark to be removed a production queue item with an invalid index.";
        return;
    }
    m_production_queue[index].to_be_removed = true;
}

void Empire::MarkNotToBeRemoved(int index) {
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index) {
        DebugLogger() << "Empire::MarkNotToBeRemoved index: " << index << "  queue size: " << m_production_queue.size();
        ErrorLogger() << "Attempted to mark not to be removed a production queue item with an invalid index.";
        return;
    }
    m_production_queue[index].to_be_removed = false;
}

void Empire::PauseProduction(int index) {
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index) {
        DebugLogger() << "Empire::PauseProduction index: " << index << "  queue size: " << m_production_queue.size();
        ErrorLogger() << "Attempted pause a production queue item with an invalid index.";
        return;
    }
    m_production_queue[index].paused = true;
}

void Empire::ResumeProduction(int index) {
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index) {
        DebugLogger() << "Empire::ResumeProduction index: " << index << "  queue size: " << m_production_queue.size();
        ErrorLogger() << "Attempted resume a production queue item with an invalid index.";
        return;
    }
    m_production_queue[index].paused = false;
}

void Empire::AllowUseImperialPP(int index, bool allow) {
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index) {
        DebugLogger() << "Empire::AllowUseImperialPP index: " << index << "  queue size: " << m_production_queue.size();
        ErrorLogger() << "Attempted allow/disallow use of the imperial PP stockpile for a production queue item with an invalid index.";
        return;
    }
    DebugLogger() << "Empire::AllowUseImperialPP allow: " << allow << "  index: " << index << "  queue size: " << m_production_queue.size();
    m_production_queue[index].allowed_imperial_stockpile_use = allow;
}

void Empire::ConquerProductionQueueItemsAtLocation(int location_id, int empire_id, EmpireManager& empires) {
    if (location_id == INVALID_OBJECT_ID) {
        ErrorLogger() << "Empire::ConquerProductionQueueItemsAtLocation: tried to conquer build items located at an invalid location";
        return;
    }

    DebugLogger() << "Empire::ConquerProductionQueueItemsAtLocation: conquering items located at "
                  << location_id << " to empire " << empire_id;

    auto to_empire = empires.GetEmpire(empire_id);    // may be null
    if (!to_empire && empire_id != ALL_EMPIRES) {
        ErrorLogger() << "Couldn't get empire with id " << empire_id;
        return;
    }


    for (auto& [from_empire_id, from_empire] : empires) {
        if (from_empire_id == empire_id) continue;    // skip this empire; can't capture one's own ProductionItems

        ProductionQueue& queue = from_empire->m_production_queue;

        for (auto queue_it = queue.begin(); queue_it != queue.end(); ) {
            const auto& elem = *queue_it;
            if (elem.location != location_id) {
                ++queue_it;
                continue; // skip projects with wrong location
            }

            if (elem.item.build_type == BuildType::BT_BUILDING) {
                const BuildingType* type = GetBuildingType(elem.item.name);
                if (!type) {
                    ErrorLogger() << "ConquerProductionQueueItemsAtLocation couldn't get building with name " << elem.item.name;
                    continue;
                }

                const CaptureResult result = type->GetCaptureResult(from_empire_id, empire_id, location_id, true);

                if (result == CaptureResult::CR_DESTROY) {
                    // item removed from current queue, NOT added to conquerer's queue
                    queue_it = queue.erase(queue_it); // invalidates elem reference

                } else if (result == CaptureResult::CR_CAPTURE) {
                    if (to_empire) {
                        // item removed from current queue, added to conquerer's queue
                        ProductionQueue::Element new_elem(elem.item, empire_id, elem.uuid, elem.ordered,
                                                          elem.remaining, 1, location_id);
                        new_elem.progress = elem.progress;
                        to_empire->m_production_queue.push_back(std::move(new_elem));

                        queue_it = queue.erase(queue_it); // invalidates queue_it and elem reference
                    } else {
                        // else do nothing; no empire can't capure things
                        ++queue_it;
                    }

                } else if (result == CaptureResult::INVALID_CAPTURE_RESULT) {
                    ErrorLogger() << "Empire::ConquerBuildsAtLocationFromEmpire: BuildingType had an invalid CaptureResult";

                } else {
                    ++queue_it;
                }
                // otherwise do nothing: item left on current queue, conquerer gets nothing
            } else {
                ++queue_it;
            }

            // TODO: other types of build item...
        }
    }
}

void Empire::AddNewlyResearchedTechToGrantAtStartOfNextTurn(std::string name) {
    const Tech* tech = GetTech(name);
    if (!tech) {
        ErrorLogger() << "Empire::AddNewlyResearchedTechToGrantAtStartOfNextTurn given an invalid tech: " << name;
        return;
    }

    if (m_techs.contains(name))
        return;

    // Mark given tech to be granted at next turn. If it was already marked, skip writing a SitRep message
    m_newly_researched_techs.insert(std::move(name));
}

void Empire::ApplyNewTechs(Universe& universe, int current_turn) {
    for (const auto& new_tech : m_newly_researched_techs) {
        const Tech* tech = GetTech(new_tech);
        if (!tech) {
            ErrorLogger() << "Empire::ApplyNewTech has an invalid entry in m_newly_researched_techs: " << new_tech;
            continue;
        }

        for (const UnlockableItem& item : tech->UnlockedItems())
            UnlockItem(item, universe, current_turn);  // potential infinite if a tech (in)directly unlocks itself?

        if (!m_techs.contains(new_tech)) {
            m_techs[new_tech] = current_turn;
            AddSitRepEntry(CreateTechResearchedSitRep(new_tech, current_turn));
        }
    }
    m_newly_researched_techs.clear();
}

void Empire::AddPolicy(std::string name, int current_turn) {
    const Policy* policy = GetPolicy(name);
    if (!policy) {
        ErrorLogger() << "Empire::AddPolicy given and invalid policy: " << name;
        return;
    }

    if (m_available_policies.find(name) == m_available_policies.end()) {
        AddSitRepEntry(CreatePolicyUnlockedSitRep(name, current_turn));
        m_available_policies.insert(std::move(name));
    }
}

void Empire::ApplyPolicies(Universe& universe, int current_turn) {
    for (auto& [policy_name, adoption_info] : m_adopted_policies) {
        if (adoption_info.adoption_turn >= current_turn)
            continue; // policy unlock take effect one turn after adoption

        const Policy* policy = GetPolicy(policy_name);
        if (!policy) {
            ErrorLogger() << "Empire::ApplyPolicies couldn't find policy with name  " << policy_name;
            continue;
        }
        for (const UnlockableItem& item : policy->UnlockedItems())
            UnlockItem(item, universe, current_turn);
    }
}

void Empire::UnlockItem(const UnlockableItem& item, Universe& universe, int current_turn) {
    switch (item.type) {
    case UnlockableItemType::UIT_BUILDING:
        AddBuildingType(item.name, current_turn);
        break;
    case UnlockableItemType::UIT_SHIP_PART:
        AddShipPart(item.name, current_turn);
        break;
    case UnlockableItemType::UIT_SHIP_HULL:
        AddShipHull(item.name, current_turn);
        break;
    case UnlockableItemType::UIT_SHIP_DESIGN:
        AddShipDesign(GetPredefinedShipDesignManager().GetDesignID(item.name), universe);
        break;
    case UnlockableItemType::UIT_TECH:
        AddNewlyResearchedTechToGrantAtStartOfNextTurn(item.name);
        break;
    case UnlockableItemType::UIT_POLICY:
        AddPolicy(item.name, current_turn);
        break;
    default:
        ErrorLogger() << "Empire::UnlockItem : passed UnlockableItem with unrecognized UnlockableItemType";
    }
}

void Empire::AddBuildingType(std::string name, int current_turn) {
    const BuildingType* building_type = GetBuildingType(name);
    if (!building_type) {
        ErrorLogger() << "Empire::AddBuildingType given an invalid building type name: " << name;
        return;
    }
    if (!building_type->Producible())
        return;
    if (m_available_building_types.contains(name))
        return;
    m_available_building_types.insert(name);
    AddSitRepEntry(CreateBuildingTypeUnlockedSitRep(std::move(name), current_turn));
}

void Empire::AddShipPart(std::string name, int current_turn) {
    const ShipPart* ship_part = GetShipPart(name);
    if (!ship_part) {
        ErrorLogger() << "Empire::AddShipPart given an invalid ship part name: " << name;
        return;
    }
    if (!ship_part->Producible())
        return;
    m_available_ship_parts.insert(name);
    AddSitRepEntry(CreateShipPartUnlockedSitRep(std::move(name), current_turn));
}

void Empire::AddShipHull(std::string name, int current_turn) {
    const ShipHull* ship_hull = GetShipHull(name);
    if (!ship_hull) {
        ErrorLogger() << "Empire::AddShipHull given an invalid hull type name: " << name;
        return;
    }
    if (!ship_hull->Producible())
        return;
    m_available_ship_hulls.insert(name);
    AddSitRepEntry(CreateShipHullUnlockedSitRep(std::move(name), current_turn));
}

void Empire::AddExploredSystem(int ID, int turn, const ObjectMap& objects) {
    if (objects.getRaw<System>(ID))
        m_explored_systems.emplace(ID, turn);
    else
        ErrorLogger() << "Empire::AddExploredSystem given an invalid system id: " << ID;
}

std::string Empire::NewShipName() {
    static std::vector<std::string> ship_names = UserStringList("SHIP_NAMES");
    if (ship_names.empty())
        ship_names.push_back(UserString("OBJ_SHIP"));

    // select name randomly from list
    const int ship_name_idx = RandInt(0, static_cast<int>(ship_names.size()) - 1);
    std::string retval = ship_names[ship_name_idx];
    const int times_name_used = ++m_ship_names_used[retval];
    if (1 < times_name_used)
        retval += " " + RomanNumber(times_name_used);
    return retval;
}

void Empire::AddShipDesign(int ship_design_id, const Universe& universe, int next_design_id) {
    /* Check if design id is valid.  That is, check that it corresponds to an
     * existing shipdesign in the universe.  On clients, this means that this
     * empire knows about this ship design and the server consequently sent the
     * design to this player.  On the server, all existing ship designs will be
     * valid, so this just adds this design's id to those that this empire will
     * retain as one of it's ship designs, which are those displayed in the GUI
     * list of available designs for human players, and */
    if (ship_design_id == next_design_id)
        return;

    // don't check if design is producible; adding a ship design is useful for more than just producing it

    if (const ShipDesign* ship_design = universe.GetShipDesign(ship_design_id)) {
        // design is valid, so just add the id to empire's set of ids that it knows about
        if (!m_known_ship_designs.contains(ship_design_id)) {
            m_known_ship_designs.insert(ship_design_id);

            ShipDesignsChangedSignal();

            TraceLogger() << "AddShipDesign::  " << ship_design->Name() << " (" << ship_design_id
                          << ") to empire #" << EmpireID();
        }
    } else {
        // design in not valid
        ErrorLogger() << "Empire::AddShipDesign(int ship_design_id) was passed a design id that this empire doesn't know about, or that doesn't exist";
    }
}

int Empire::AddShipDesign(ShipDesign ship_design, Universe& universe) {
    /* check if there already exists this same design in the universe.
     * On clients, this checks whether this empire knows of this exact
     * design and is trying to re-add it.  On the server, this checks
     * whether this exact design exists at all yet */
    const auto it = std::find_if(universe.ShipDesigns().begin(), universe.ShipDesigns().end(),
                                 [&ship_design](const auto& id_design) { return ship_design == id_design.second; });
    if (it != universe.ShipDesigns().end()) {
        // ship design is already present in universe.  just need to add it to the empire's set of ship designs
        const int ship_design_id = it->first;
        AddShipDesign(ship_design_id, universe);
        return ship_design_id;
    }

    const auto new_design_id = universe.InsertShipDesign(std::move(ship_design));

    if (new_design_id == INVALID_DESIGN_ID) {
        ErrorLogger() << "Empire::AddShipDesign Unable to add new design to universe";
        return INVALID_DESIGN_ID;
    }

    AddShipDesign(new_design_id, universe);

    return new_design_id;
}

void Empire::RemoveShipDesign(int ship_design_id) {
    if (m_known_ship_designs.erase(ship_design_id)) {
        ShipDesignsChangedSignal();
    } else {
        DebugLogger() << "Empire::RemoveShipDesign: this empire did not have design with id " << ship_design_id;
    }
}

void Empire::AddSitRepEntry(const SitRepEntry& entry)
{ m_sitrep_entries.push_back(entry); }

void Empire::AddSitRepEntry(SitRepEntry&& entry)
{ m_sitrep_entries.push_back(std::move(entry)); }

void Empire::RemoveTech(const std::string& name)
{ m_techs.erase(name); }

void Empire::RemovePolicy(const std::string& name)
{ m_available_policies.erase(name); }

void Empire::LockItem(const UnlockableItem& item) {
    switch (item.type) {
    case UnlockableItemType::UIT_BUILDING:
        RemoveBuildingType(item.name);
        break;
    case UnlockableItemType::UIT_SHIP_PART:
        RemoveShipPart(item.name);
        break;
    case UnlockableItemType::UIT_SHIP_HULL:
        RemoveShipHull(item.name);
        break;
    case UnlockableItemType::UIT_SHIP_DESIGN:
        RemoveShipDesign(GetPredefinedShipDesignManager().GetDesignID(item.name));
        break;
    case UnlockableItemType::UIT_TECH:
        RemoveTech(item.name);
        break;
    case UnlockableItemType::UIT_POLICY:
        RemovePolicy(item.name);
        break;
    default:
        ErrorLogger() << "Empire::LockItem : passed UnlockableItem with unrecognized UnlockableItemType";
    }
}

void Empire::RemoveBuildingType(const std::string& name) {
    if (!m_available_building_types.contains(name))
        DebugLogger() << "Empire::RemoveBuildingType asked to remove building type " << name << " that was no available to this empire";
    m_available_building_types.erase(name);
}

void Empire::RemoveShipPart(const std::string& name) {
    auto it = m_available_ship_parts.find(name);
    if (it == m_available_ship_parts.end())
        DebugLogger() << "Empire::RemoveShipPart asked to remove part type " << name << " that was no available to this empire";
    m_available_ship_parts.erase(name);
}

void Empire::RemoveShipHull(const std::string& name) {
    auto it = m_available_ship_hulls.find(name);
    if (it == m_available_ship_hulls.end())
        DebugLogger() << "Empire::RemoveShipHull asked to remove hull type " << name << " that was no available to this empire";
    m_available_ship_hulls.erase(name);
}

void Empire::ClearSitRep()
{ m_sitrep_entries.clear(); }

namespace {
    // remove nonexistant / invalid techs from queue
    void SanitizeResearchQueue(ResearchQueue& queue) {
        bool done = false;
        while (!done) {
            auto it = queue.begin();
            while (true) {
                if (it == queue.end()) {
                    done = true;        // got all the way through the queue without finding an invalid tech
                    break;
                } else if (!GetTech(it->name)) {
                    DebugLogger() << "SanitizeResearchQueue for empire " << queue.EmpireID() << " removed invalid tech: " << it->name;
                    queue.erase(it);    // remove invalid tech, end inner loop without marking as finished
                    break;
                } else {
                    ++it;               // check next element
                }
            }
        }
    }
}

std::vector<std::string> Empire::CheckResearchProgress(const ScriptingContext& context) {
    SanitizeResearchQueue(m_research_queue);

    float spent_rp{0.0f};
    float total_rp_available = m_research_pool.TotalAvailable();

    // process items on queue
    std::vector<std::string> to_erase_from_queue_and_grant_next_turn;
    for (auto& elem : m_research_queue) {
        const Tech* tech = GetTech(elem.name);
        if (!tech) {
            ErrorLogger() << "Empire::CheckResearchProgress couldn't find tech on queue, even after sanitizing!";
            continue;
        }
        float& progress = m_research_progress[elem.name];
        float tech_cost = tech->ResearchCost(m_id, context);
        progress += elem.allocated_rp / std::max(EPSILON, tech_cost);
        spent_rp += elem.allocated_rp;
        if (tech_cost - EPSILON <= progress * tech_cost) {
            m_research_progress.erase(elem.name);
            to_erase_from_queue_and_grant_next_turn.push_back(elem.name);
        }
    }

    //DebugLogger() << m_research_queue.Dump();
    float rp_left_to_spend = total_rp_available - spent_rp;
    //DebugLogger() << "leftover RP: " << rp_left_to_spend;
    // auto-allocate any excess RP left over after player-specified queued techs

    // if there are left over RPs, any tech on the queue presumably can't
    // have RP allocated to it
    std::unordered_set<std::string> techs_not_suitable_for_auto_allocation;
    for (auto& elem : m_research_queue)
        techs_not_suitable_for_auto_allocation.insert(elem.name);

    // for all available and suitable techs, store ordered by cost to complete
    std::vector<std::pair<double, std::string>> costs_to_complete_available_unpaused_techs;
    costs_to_complete_available_unpaused_techs.reserve(GetTechManager().size());
    for (const auto& [tech_name, tech] : GetTechManager()) {
        if (techs_not_suitable_for_auto_allocation.contains(tech_name))
            continue;
        if (this->GetTechStatus(tech_name) != TechStatus::TS_RESEARCHABLE)
            continue;
        if (!tech.Researchable())
            continue;
        double progress = this->ResearchProgress(tech_name, context);
        double total_cost = tech.ResearchCost(m_id, context);
        if (progress >= total_cost)
            continue;
        costs_to_complete_available_unpaused_techs.emplace_back(total_cost - progress, tech_name);
    }
    std::sort(costs_to_complete_available_unpaused_techs.begin(),
              costs_to_complete_available_unpaused_techs.end());

    // in order of minimum additional cost to complete, allocate RP to
    // techs up to available RP and per-turn limits
    for (auto const& [tech_cost, tech_name] : costs_to_complete_available_unpaused_techs) {
        if (rp_left_to_spend <= EPSILON)
            break;

        const Tech* tech = GetTech(tech_name);
        if (!tech)
            continue;

        //DebugLogger() << "extra tech: " << cost_tech.second << " needs: " << cost_tech.first << " more RP to finish";

        float RPs_per_turn_limit = tech->PerTurnCost(m_id, context);
        float progress_fraction = m_research_progress[tech_name];

        float progress_fraction_left = 1.0f - progress_fraction;
        float max_progress_per_turn = RPs_per_turn_limit / static_cast<float>(tech_cost);
        float progress_possible_with_available_rp = rp_left_to_spend / static_cast<float>(tech_cost);

        //DebugLogger() << "... progress left: " << progress_fraction_left
        //              << " max per turn: " << max_progress_per_turn
        //              << " progress possible with available rp: " << progress_possible_with_available_rp;

        float progress_increase = std::min(
            progress_fraction_left,
            std::min(max_progress_per_turn, progress_possible_with_available_rp));

        float consumed_rp = progress_increase * static_cast<float>(tech_cost);

        m_research_progress[tech_name] += progress_increase;
        rp_left_to_spend -= consumed_rp;

        if (tech_cost - EPSILON <= m_research_progress[tech_name] * tech_cost)
            to_erase_from_queue_and_grant_next_turn.push_back(tech_name);

        //DebugLogger() << "... allocated: " << consumed_rp << " to increase progress by: " << progress_increase;
    }

    // remove completed items from queue (after consuming extra RP, as that
    // determination uses the contents of the queue as input)
    for (const std::string& tech_name : to_erase_from_queue_and_grant_next_turn) {
        auto temp_it = m_research_queue.find(tech_name);
        if (temp_it != m_research_queue.end())
            m_research_queue.erase(temp_it);
    }

    // can uncomment following line when / if research stockpiling is enabled...
    // m_resource_pools[RE_RESEARCH]->SetStockpile(m_resource_pools[RE_RESEARCH]->TotalAvailable() - m_research_queue.TotalRPsSpent());
    return to_erase_from_queue_and_grant_next_turn;
}

void Empire::CheckProductionProgress(ScriptingContext& context) {
    DebugLogger() << "========Empire::CheckProductionProgress=======";
    // following commented line should be redundant, as previous call to
    // UpdateResourcePools should have generated necessary info
    // m_production_queue.Update(context.ContextUniverse());

    std::map<int, std::vector<Ship*>> system_new_ships;
    std::map<int, int> new_ship_rally_point_ids;

    auto& universe = context.ContextUniverse();


    // preprocess the queue to get all the costs and times of all items
    // at every location at which they are being produced,
    // before doing any generation of new objects or other modifications
    // of the gamestate. this will ensure that the cost of items doesn't
    // change while the queue is being processed, so that if there is
    // sufficent PP to complete an object at the start of a turn,
    // items above it on the queue getting finished don't increase the
    // cost and result in it not being finished that turn.
    struct ItemCostAndTime {
        ItemCostAndTime(int l, BuildType bt, int d, std::string_view n, float c, int t) :
            location_id(l),
            build_type(bt),
            design_id(d),
            name(n),
            cost(c),
            time(t)
        {}
        ItemCostAndTime() = default;

        const int location_id = INVALID_OBJECT_ID;
        const BuildType build_type = BuildType::INVALID_BUILD_TYPE;
        const int design_id = INVALID_DESIGN_ID;
        const std::string_view name = "";
        const float cost = 0.0f;
        const int time = 0;
    };
    std::vector<ItemCostAndTime> queue_item_costs_and_times;
    queue_item_costs_and_times.reserve(m_production_queue.size());

    for (const auto& elem : m_production_queue) {
        // cache unique items and locations costs and time...

        // for items that don't depend on location, only store cost/time once
        const int location_id = (elem.item.CostIsProductionLocationInvariant(universe) ?
                                 INVALID_OBJECT_ID : elem.location);
        const auto& item = elem.item;
        const std::string_view item_name = item.name;
        auto same_item_and_loc = [location_id, bt{item.build_type}, did{item.design_id}, item_name]
            (const auto& q_item_time)
        {
            return location_id == q_item_time.location_id && bt == q_item_time.build_type &&
                did == q_item_time.design_id && item_name == q_item_time.name;
        };
        if (std::any_of(queue_item_costs_and_times.begin(), queue_item_costs_and_times.end(),
                        same_item_and_loc))
        { continue; } // already have this item and location cached

        auto [cost, time] = elem.ProductionCostAndTime(context);
        queue_item_costs_and_times.emplace_back(location_id, item.build_type, item.design_id,
                                                item_name, cost, time);
    }

    auto get_cost_turns = [](const auto& qicat, const ProductionQueue::Element& elem, int location_id)
        -> std::pair<float, int>
    {
        const auto& item = elem.item;
        auto same_item_and_loc = [location_id, bt{item.build_type}, did{item.design_id}, item_name{item.name}]
            (const auto& q_item_time)
        {
            return location_id == q_item_time.location_id && bt == q_item_time.build_type &&
                did == q_item_time.design_id && item_name == q_item_time.name;
        };

        auto it = std::find_if(qicat.begin(), qicat.end(), same_item_and_loc);
        if (it != qicat.end())
            return {it->cost, it->time};
        return {0.0f, 1};
    };


    // go through queue, updating production progress.  If a production item is
    // completed, create the produced object or take whatever other action is
    // appropriate, and record that queue item as complete, so it can be erased
    // from the queue
    std::vector<int> to_erase;
    for (unsigned int i = 0; i < m_production_queue.size(); ++i) {
        auto& elem = m_production_queue[i];

        if (elem.to_be_removed) {
            to_erase.push_back(i);
            DebugLogger() << "Marking flagged-to-be-removed item " << i << " to be removed from queue";
        }

        const int location_id = (elem.item.CostIsProductionLocationInvariant(universe) ? INVALID_OBJECT_ID : elem.location);
        auto [cost, turns] = get_cost_turns(queue_item_costs_and_times, elem, location_id);

        if (cost < 0.01f || turns < 1) {
            ErrorLogger() << "Empire::CheckProductionProgress got strang cost/time: " << cost << " / " << turns;
            break;
        }

        cost *= elem.blocksize;

        DebugLogger() << "elem: " << elem.Dump();
        DebugLogger() << "   allocated: " << elem.allocated_pp;
        DebugLogger() << "   initial progress: " << elem.progress;

        elem.progress += elem.allocated_pp / std::max(EPSILON, cost);  // add progress for allocated PP to queue item
        elem.progress_memory = elem.progress;
        elem.blocksize_memory = elem.blocksize;

        DebugLogger() << "   updated progress: " << elem.progress;
        DebugLogger() << " ";

        std::string build_description;
        switch (elem.item.build_type) {
            case BuildType::BT_BUILDING: {
                build_description = "BuildingType " + elem.item.name;
                break;
            }
            case BuildType::BT_SHIP: {
                build_description = "Ships(s) with design id " + std::to_string(elem.item.design_id);
                break;
            }
            case BuildType::BT_STOCKPILE: {
                build_description = "Stockpile PP transfer";
                break;
            }
            default:
                build_description = "unknown build type";
        }

        auto build_location = context.ContextObjects().get(elem.location);
        if (!build_location || (elem.item.build_type == BuildType::BT_BUILDING &&
                                build_location->ObjectType() != UniverseObjectType::OBJ_PLANET))
        {
            ErrorLogger() << "Couldn't get valid build location for completed " << build_description;
            continue;
        }
        auto system = context.ContextObjects().get<System>(build_location->SystemID());
        // TODO: account for shipyards and/or other ship production
        // sites that are in interstellar space, if needed
        if (!system) {
            ErrorLogger() << "Empire::CheckProductionProgress couldn't get system for producing new " << build_description;
            continue;
        }

        // check location condition before each item is created, so
        // that items being produced can prevent subsequent
        // completions on the same turn from going through
        if (!this->ProducibleItem(elem.item, elem.location, context)) {
            DebugLogger() << "Location test failed for " << build_description << " at location " << build_location->Name();
            continue;
        }


        // only if accumulated PP is sufficient, the item can be completed
        if (cost - EPSILON > elem.progress*cost)
            continue;


        // only if consumed resources are available, then item can be completd
        bool consumption_impossible = false;
        auto sc = elem.item.CompletionSpecialConsumption(elem.location, context);
        for (auto& [special_name, obj_consump] : sc) {
            for (auto& [spec_obj_id, consump] : obj_consump) {
                auto obj = context.ContextObjects().getRaw(spec_obj_id);
                float capacity = obj ? obj->SpecialCapacity(special_name) : 0.0f;
                if (capacity < consump * elem.blocksize) {
                    consumption_impossible = true;
                    break;
                }
            }
            if (consumption_impossible)
                break;
        }
        auto mc = elem.item.CompletionMeterConsumption(elem.location, context);
        for (auto& meter_type : mc) {
            if (consumption_impossible)
                break;
            for (auto& object_meter : meter_type.second) {
                auto obj = context.ContextObjects().getRaw(object_meter.first);
                const Meter* meter = obj ? obj->GetMeter(meter_type.first) : nullptr;
                if (!meter || meter->Current() < object_meter.second * elem.blocksize) {
                    consumption_impossible = true;
                    break;
                }
            }
        }
        if (consumption_impossible)
            continue;


        // deduct progress for complete item from accumulated progress, so that next
        // repetition can continue accumulating PP, but don't set progress to 0, as
        // this way overflow progress / PP allocated this turn can be used for the
        // next repetition of the item.
        elem.progress -= 1.0f;
        if (elem.progress < 0.0f) {
            if (elem.progress < -1e-3)
                ErrorLogger() << "Somehow got negative progress (" << elem.progress
                              << ") after deducting progress for completed item...";
            elem.progress = 0.0f;
        }

        elem.progress_memory = elem.progress;
        DebugLogger() << "Completed an item: " << elem.item.name;


        // consume the item's special and meter consumption
        for (auto& [special_name, consumption_map] : sc) {
            for (auto [obj_id, consumption] : consumption_map) {
                auto obj = context.ContextObjects().getRaw(obj_id);
                if (!obj)
                    continue;
                if (!obj->HasSpecial(special_name))
                    continue;
                float cur_capacity = obj->SpecialCapacity(special_name);
                float new_capacity = std::max(0.0f, cur_capacity - consumption * elem.blocksize);
                obj->SetSpecialCapacity(special_name, new_capacity, context.current_turn);
            }
        }
        for (const auto& [meter_type, consumption_map] : mc) {
            for (const auto [obj_id, consumption] : consumption_map) {
                auto* obj = context.ContextObjects().getRaw(obj_id);
                if (!obj)
                    continue;
                Meter* meter = obj->GetMeter(meter_type);
                if (!meter)
                    continue;
                const float cur_meter = meter->Current();
                const float new_meter = cur_meter - consumption * elem.blocksize;
                meter->SetCurrent(new_meter);
                meter->BackPropagate();
            }
        }


        // create actual thing(s) being produced
        switch (elem.item.build_type) {
        case BuildType::BT_BUILDING: {
            auto planet = context.ContextObjects().get<Planet>(elem.location);

            // create new building
            auto building = universe.InsertNew<Building>(m_id, elem.item.name,
                                                         m_id, context.current_turn);
            planet->AddBuilding(building->ID());
            building->SetPlanetID(planet->ID());
            system->Insert(building, System::NO_ORBIT, context.current_turn, context.ContextObjects());

            // record building production in empire stats
            m_building_types_produced[elem.item.name]++;

            AddSitRepEntry(CreateBuildingBuiltSitRep(building->ID(), planet->ID(),
                                                     context.current_turn));
            DebugLogger() << "New Building created on turn: " << context.current_turn;
            break;
        }

        case BuildType::BT_SHIP: {
            if (elem.blocksize < 1)
                break;   // nothing to do!

            // get species for this ship.  use popcenter species if build
            // location is a popcenter, or use ship species if build
            // location is a ship, or use empire capital species if there
            // is a valid capital, or otherwise ???
            // TODO: Add more fallbacks if necessary
            std::string species_name;
            if (auto location_planet = std::dynamic_pointer_cast<const Planet>(build_location))
                species_name = location_planet->SpeciesName();
            else if (auto location_ship = std::dynamic_pointer_cast<const Ship>(build_location))
                species_name = location_ship->SpeciesName();
            else if (auto capital_planet = context.ContextObjects().getRaw<Planet>(this->CapitalID()))
                species_name = capital_planet->SpeciesName();
            // else give up...

            if (species_name.empty()) {
                // only really a problem for colony ships, which need to have a species to function
                const auto* design = universe.GetShipDesign(elem.item.design_id);
                if (!design) {
                    ErrorLogger() << "Couldn't get ShipDesign with id: " << elem.item.design_id;
                    break;
                }
                if (design->CanColonize()) {
                    ErrorLogger() << "Couldn't get species in order to make colony ship!";
                    break;
                }
            }

            std::shared_ptr<Ship> ship;

            for (int count = 0; count < elem.blocksize; count++) {
                // create ship
                ship = universe.InsertNew<Ship>(
                    m_id, elem.item.design_id, species_name, universe,
                    context.species, m_id, context.current_turn);
                system->Insert(ship, System::NO_ORBIT, context.current_turn, context.ContextObjects());

                // record ship production in empire stats
                if (m_ship_designs_produced.contains(elem.item.design_id))
                    m_ship_designs_produced[elem.item.design_id]++;
                else
                    m_ship_designs_produced[elem.item.design_id] = 1;
                if (m_species_ships_produced.contains(species_name))
                    m_species_ships_produced[species_name]++;
                else
                    m_species_ships_produced[species_name] = 1;


                // set active meters that have associated max meters to an
                // initial very large value, so that when the active meters are
                // later clamped, they will equal the max meter after effects
                // have been applied, letting new ships start with maxed
                // everything that is traced with an associated max meter.
                ship->SetShipMetersToMax();
                // set ship speed so that it can be affected by non-zero speed checks
                if (auto* design = universe.GetShipDesign(elem.item.design_id))
                    ship->GetMeter(MeterType::METER_SPEED)->Set(design->Speed(), design->Speed());
                ship->BackPropagateMeters();

                ship->Rename(NewShipName());

                // store ships to put into fleets later
                const auto SHIP_ID = ship->ID();
                system_new_ships[system->ID()].push_back(ship.get());

                // store ship rally points
                if (elem.rally_point_id != INVALID_OBJECT_ID)
                    new_ship_rally_point_ids[SHIP_ID] = elem.rally_point_id;
            }

            // add sitrep
            if (elem.blocksize == 1) {
                AddSitRepEntry(CreateShipBuiltSitRep(ship->ID(), system->ID(),
                                                     ship->DesignID(), context.current_turn));
                DebugLogger() << "New Ship, id " << ship->ID() << ", created on turn: " << ship->CreationTurn();
            } else {
                AddSitRepEntry(CreateShipBlockBuiltSitRep(system->ID(), ship->DesignID(),
                                                          elem.blocksize, context.current_turn));
                DebugLogger() << "New block of "<< elem.blocksize << " ships created on turn: " << ship->CreationTurn();
            }
            break;
        }

        case BuildType::BT_STOCKPILE: {
            DebugLogger() << "Finished a transfer to stockpile";
            break;
        }

        default:
            ErrorLogger() << "Build item of unknown build type finished on production queue.";
            break;
        }

        if (--elem.remaining < 1) { // decrement number of remaining items to be produced in current queue element
            to_erase.push_back(i);  // remember completed element so that it can be removed from queue
            DebugLogger() << "Marking completed production queue item to be removed from queue";
        }
    }

    // create fleets for new ships and put ships into fleets
    for (auto& [system_id, new_ships] : system_new_ships) {
        auto system = context.ContextObjects().getRaw<System>(system_id);
        if (!system) {
            ErrorLogger() << "Couldn't get system with id " << system_id << " for creating new fleets for newly produced ships";
            continue;
        }
        if (new_ships.empty())
            continue;

        // group ships into fleets by rally point and design
        std::map<int, std::map<int, std::vector<Ship*>>>
            new_ships_by_rally_point_id_and_design_id;
        for (auto* ship : new_ships) {
            int rally_point_id = INVALID_OBJECT_ID;
            auto rally_it = new_ship_rally_point_ids.find(ship->ID());
            if (rally_it != new_ship_rally_point_ids.end())
                rally_point_id = rally_it->second;

            auto design_id = ship->DesignID();
            new_ships_by_rally_point_id_and_design_id[rally_point_id][design_id].push_back(ship);
        }


        // create fleets for ships with the same rally point, grouped by
        // ship design
        // Do not group unarmed ships with no troops (i.e. scouts and
        // colony ships).
        for (auto& rally_ships : new_ships_by_rally_point_id_and_design_id) {
            int rally_point_id = rally_ships.first;
            auto& new_ships_by_design = rally_ships.second;

            for (auto& ships_by_design : new_ships_by_design) {
                std::vector<int> ship_ids;

                auto& ships = ships_by_design.second;
                if (ships.empty())
                    continue;

                // create a single fleet for combat ships and individual
                // fleets for non-combat ships
                const auto* first_ship = ships.front();
                const bool individual_fleets = !(first_ship->IsArmed(context)
                                              || first_ship->HasFighters(universe)
                                              || first_ship->CanHaveTroops(universe)
                                              || first_ship->CanBombard(universe));

                std::vector<Fleet*> fleets;
                std::shared_ptr<Fleet> fleet;

                if (!individual_fleets) {
                    fleet = universe.InsertNew<Fleet>("", system->X(), system->Y(), m_id,
                                                      context.current_turn);

                    system->Insert(fleet, System::NO_ORBIT, context.current_turn, context.ContextObjects());
                    // set prev system to prevent conflicts with CalculateRouteTo used for
                    // rally points below, but leave next system as INVALID_OBJECT_ID so
                    // fleet won't necessarily be disqualified from making blockades if it
                    // is left stationary
                    fleet->SetNextAndPreviousSystems(INVALID_OBJECT_ID, system->ID());
                    // set invalid arrival starlane so that fleet won't necessarily be free from blockades
                    fleet->SetArrivalStarlane(INVALID_OBJECT_ID);

                    fleets.push_back(fleet.get());
                }

                for (auto* ship : ships) {
                    if (individual_fleets) {
                        fleet = universe.InsertNew<Fleet>("", system->X(), system->Y(),
                                                          m_id, context.current_turn);

                        system->Insert(fleet, System::NO_ORBIT, context.current_turn, context.ContextObjects());
                        // set prev system to prevent conflicts with CalculateRouteTo used for
                        // rally points below, but leave next system as INVALID_OBJECT_ID so
                        // fleet won't necessarily be disqualified from making blockades if it
                        // is left stationary
                        fleet->SetNextAndPreviousSystems(INVALID_OBJECT_ID, system->ID());
                        // set invalid arrival starlane so that fleet won't necessarily be free from blockades
                        fleet->SetArrivalStarlane(INVALID_OBJECT_ID);

                        fleets.push_back(fleet.get());
                    }
                    ship_ids.push_back(ship->ID());
                    fleet->AddShips({ship->ID()});
                    ship->SetFleetID(fleet->ID());
                }

                for (auto* next_fleet : fleets) {
                    // rename fleet, given its id and the ship that is in it
                    next_fleet->Rename(next_fleet->GenerateFleetName(context));
                    FleetAggression new_aggr = next_fleet->HasArmedShips(context) ?
                        FleetDefaults::FLEET_DEFAULT_ARMED : FleetDefaults::FLEET_DEFAULT_UNARMED;
                    next_fleet->SetAggression(new_aggr);

                    if (rally_point_id != INVALID_OBJECT_ID) {
                        if (context.ContextObjects().get<System>(rally_point_id)) {
                            next_fleet->CalculateRouteTo(rally_point_id, universe);
                        } else if (auto rally_obj = context.ContextObjects().get(rally_point_id)) {
                            if (context.ContextObjects().get<System>(rally_obj->SystemID()))
                                next_fleet->CalculateRouteTo(rally_obj->SystemID(), universe);
                        } else {
                            ErrorLogger() << "Unable to find system to route to with rally point id: " << rally_point_id;
                        }
                    }

                    DebugLogger() << "New Fleet \"" << next_fleet->Name()
                                  <<"\" created on turn: " << next_fleet->CreationTurn();
                }
            }
        }
    }

    // removed completed items from queue
    for (auto it = to_erase.rbegin(); it != to_erase.rend(); ++it)
        m_production_queue.erase(*it);

    // update stockpile
    SetResourceStockpile(ResourceType::RE_INDUSTRY, m_production_queue.ExpectedNewStockpileAmount());
}

void Empire::CheckInfluenceProgress() {
    DebugLogger() << "========Empire::CheckProductionProgress=======";
    // following commented line should be redundant, as previous call to
    // UpdateResourcePools should have generated necessary info
    // m_influence_queue.Update(context.ContextUniverse());

    auto spending = m_influence_queue.TotalIPsSpent();
    auto new_stockpile = m_influence_queue.ExpectedNewStockpileAmount();
    DebugLogger() << "Empire::CheckInfluenceProgress spending " << spending << " and setting stockpile to " << new_stockpile;

    m_influence_pool.SetStockpile(new_stockpile);
}

void Empire::InitResourcePools(const ObjectMap& objects, const SupplyManager& supply) {
    // get this empire's owned planets
    std::vector<int> planets;
    std::vector<int> planets_and_ships;
    planets.reserve(objects.allExisting<Planet>().size());
    planets_and_ships.reserve(objects.allExisting<Planet>().size() + objects.allExisting<Ship>().size());

    for (const auto& [res_id, res] : objects.allExisting<Planet>()) {
        if (res->OwnedBy(m_id))
            planets.push_back(res_id);
    }
    planets_and_ships.insert(planets_and_ships.end(), planets.begin(), planets.end());
    m_population_pool.SetPopCenters(std::move(planets));

    // add this empire's owned ships. planets and ships can produce resources
    for (const auto& [ship_id, ship] : objects.allExisting<Ship>()) {
        if (ship->OwnedBy(m_id))
            planets_and_ships.push_back(ship_id);
    }
    m_research_pool.SetObjects(planets_and_ships);
    m_industry_pool.SetObjects(planets_and_ships);
    m_influence_pool.SetObjects(std::move(planets_and_ships));


    // inform the blockadeable resource pools about systems that can share
    m_industry_pool.SetConnectedSupplyGroups(supply.ResourceSupplyGroups(m_id));

    // set non-blockadeable resource pools to share resources between all systems
    std::set<std::set<int>> sets_set;
    std::set<int> all_systems_set;
    for (const auto& entry : objects.allExisting<System>())
        all_systems_set.insert(entry.first);
    sets_set.insert(std::move(all_systems_set));
    m_research_pool.SetConnectedSupplyGroups(sets_set);
    m_influence_pool.SetConnectedSupplyGroups(sets_set);
}

void Empire::UpdateResourcePools(const ScriptingContext& context) {
    // updating queues, allocated_rp, distribution and growth each update their
    // respective pools, (as well as the ways in which the resources are used,
    // which needs to be done simultaneously to keep things consistent)
    UpdateResearchQueue(context);
    UpdateProductionQueue(context);
    UpdateInfluenceSpending(context);
    UpdatePopulationGrowth(context.ContextObjects());
}

void Empire::UpdateResearchQueue(const ScriptingContext& context) {
    m_research_pool.Update(context.ContextObjects());
    m_research_queue.Update(m_research_pool.TotalAvailable(),
                            m_research_progress, context);
    m_research_pool.ChangedSignal();
}

void Empire::UpdateProductionQueue(const ScriptingContext& context) {
    DebugLogger() << "========= Production Update for empire: " << EmpireID() << " ========";

    m_industry_pool.Update(context.ContextObjects());
    m_production_queue.Update(context);
    m_industry_pool.ChangedSignal();
}

void Empire::UpdateInfluenceSpending(const ScriptingContext& context) {
    m_influence_pool.Update(context.ContextObjects()); // recalculate total influence production
    m_influence_queue.Update(context);
    m_influence_pool.ChangedSignal();
}

void Empire::UpdatePopulationGrowth(const ObjectMap& objects)
{ m_population_pool.Update(objects); }

void Empire::ResetMeters() {
    for (auto& entry : m_meters)
        entry.second.ResetCurrent();
}

void Empire::UpdateOwnedObjectCounters(const Universe& universe) {
    const ObjectMap& objects{universe.Objects()};
    // ships of each species and design
    m_species_ships_owned.clear();
    m_ship_designs_owned.clear();
    for (const auto& entry : objects.allExisting<Ship>()) {
        if (!entry.second->OwnedBy(m_id))
            continue;
        const auto* ship = static_cast<const Ship*>(entry.second.get());
        if (!ship)
            continue;
        if (!ship->SpeciesName().empty())
            m_species_ships_owned[ship->SpeciesName()]++;
        m_ship_designs_owned[ship->DesignID()]++;
    }

    // ships in the queue for which production started
    m_ship_designs_in_production.clear();
    for (const auto& elem : m_production_queue) {
        const auto& item = elem.item;
        if ((item.build_type == BuildType::BT_SHIP) && (elem.progress > 0.0f))
            m_ship_designs_in_production[item.design_id] += elem.blocksize;
    }

    // update ship part counts
    m_ship_parts_owned.clear();
    m_ship_part_class_owned.clear();
    for (const auto [design_id, design_count] : m_ship_designs_owned) {
        const ShipDesign* design = universe.GetShipDesign(design_id);
        if (!design)
            continue;

        // update count of ShipParts
        for (const auto& [part_name, part_count] : design->ShipPartCount())
            m_ship_parts_owned[part_name] += part_count * design_count;

        // update count of ShipPartClasses
        for (const auto& part_class : design->PartClassCount())
            m_ship_part_class_owned[part_class.first] += part_class.second * design_count;
    }

    // colonies of each species, and unspecified outposts
    m_species_colonies_owned.clear();
    m_outposts_owned = 0;
    for (const auto& entry : objects.allExisting<Planet>()) {
        if (!entry.second->OwnedBy(this->EmpireID()))
            continue;
        const auto* planet = static_cast<const Planet*>(entry.second.get());
        if (!planet)
            continue;
        if (planet->SpeciesName().empty())
            m_outposts_owned++;
        else
            m_species_colonies_owned[planet->SpeciesName()]++;
    }

    // buildings of each type
    m_building_types_owned.clear();
    for (const auto& entry : objects.allExisting<Building>()) {
        if (!entry.second->OwnedBy(this->EmpireID()))
            continue;
        const auto building = static_cast<const Building*>(entry.second.get());
        if (!building)
            continue;
        m_building_types_owned[building->BuildingTypeName()]++;
    }
}


void Empire::CheckObsoleteGameContent() {
    // remove any unrecognized policies and uncategorized policies
    const auto policies_temp{m_adopted_policies};
    for (auto& [policy_name, adoption_info] : policies_temp) {
        if (!GetPolicy(policy_name)) {
            ErrorLogger() << "UpdatePolicies couldn't find policy with name: " << policy_name;
            m_adopted_policies.erase(policy_name);

        } else if (adoption_info.category.empty()) {
            ErrorLogger() << "UpdatePolicies found policy " << policy_name << " in empty category?";
            m_adopted_policies.erase(policy_name);
        }
    }
    const auto policies_temp2{m_available_policies};
    for (auto& policy_name : policies_temp2) {
        if (!GetPolicy(policy_name)) {
            ErrorLogger() << "UpdatePolicies couldn't find policy with name: " << policy_name;
            m_available_policies.erase(policy_name);
        }
    }
}


void Empire::SetAuthenticated(bool authenticated)
{ m_authenticated = authenticated; }

int Empire::TotalShipsOwned() const {
    // sum up counts for each ship design owned by this empire
    // (not using species ship counts, as an empire could potentially own a
    //  ship that has no species...)
    int counter = 0;
    for (const auto& entry : m_ship_designs_owned)
        counter += entry.second;
    return counter;
}

void Empire::RecordShipShotDown(const Ship& ship) {
    bool insert_succeeded = m_ships_destroyed.insert(ship.ID()).second;
    if (!insert_succeeded) {
        DebugLogger() << "Already recorded empire " << m_id << " destruction of ship " << ship.Name() << " (" << ship.ID() << ")";
        return; // already recorded this destruction
    }

    DebugLogger() << "Recording empire " << m_id << " destruction of ship " << ship.Name() << " (" << ship.ID() << ")";
    m_empire_ships_destroyed[ship.Owner()]++;
    m_ship_designs_destroyed[ship.DesignID()]++;
    m_species_ships_destroyed[ship.SpeciesName()]++;
}

void Empire::RecordShipLost(const Ship& ship) {
    m_species_ships_lost[ship.SpeciesName()]++;
    m_ship_designs_lost[ship.DesignID()]++;
}

void Empire::RecordShipScrapped(const Ship& ship) {
    m_ship_designs_scrapped[ship.DesignID()]++;
    m_species_ships_scrapped[ship.SpeciesName()]++;
}

void Empire::RecordBuildingScrapped(const Building& building)
{ m_building_types_scrapped[building.BuildingTypeName()]++; }

void Empire::RecordPlanetInvaded(const Planet& planet)
{ m_species_planets_invaded[planet.SpeciesName()]++; }

void Empire::RecordPlanetDepopulated(const Planet& planet)
{ m_species_planets_depoped[planet.SpeciesName()]++; }

int Empire::TotalShipPartsOwned() const {
    // sum counts of all ship parts owned by this empire
    int retval = 0;

    for (const auto& part_class : m_ship_part_class_owned)
        retval += part_class.second;

    return retval;
}

int Empire::TotalBuildingsOwned() const {
    // sum up counts for each building type owned by this empire
    int counter = 0;
    for (const auto& entry : m_building_types_owned)
        counter += entry.second;
    return counter;
}
