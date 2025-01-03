#include "Government.h"

#include "../universe/Effect.h"
#include "../universe/UniverseObject.h"
#include "../universe/ObjectMap.h"
#include "../universe/ValueRef.h"
#include "../universe/UnlockableItem.h"
#include "../util/OptionsDB.h"
#include "../util/Logger.h"
#include "../util/GameRules.h"
#include "../util/MultiplayerCommon.h"
#include "../util/GameRuleRanks.h"
#include "../util/CheckSums.h"
#include "../util/ScopedTimer.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"

#include <boost/filesystem/fstream.hpp>

namespace {
    #define UserStringNop(key) key

    void AddRules(GameRules& rules) {
        // makes all policies cost 1 influence to adopt
        rules.Add<bool>(UserStringNop("RULE_CHEAP_POLICIES"), UserStringNop("RULE_CHEAP_POLICIES_DESC"),
                        GameRuleCategories::GameRuleCategory::TEST, false, true,
                        GameRuleRanks::RULE_CHEAP_POLICIES_RANK);
    }
    bool temp_bool = RegisterGameRules(&AddRules);
}

///////////////////////////////////////////////////////////
// Policy                                                //
///////////////////////////////////////////////////////////
Policy::Policy(std::string name, std::string description,
               std::string short_description, std::string category,
               std::unique_ptr<ValueRef::ValueRef<double>>&& adoption_cost,
               std::set<std::string>&& prerequisites,
               std::set<std::string>&& exclusions,
               std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects,
               std::vector<UnlockableItem>&& unlocked_items,
               std::string graphic) :
    m_name(name),
    m_description(std::move(description)),
    m_short_description(std::move(short_description)),
    m_category(std::move(category)),
    m_adoption_cost([](auto&& c, const std::string& name) {
        if (c)
            c->SetTopLevelContent(name);
        return std::move(c);
    }(std::move(adoption_cost), name)),
    m_prerequisites(prerequisites.begin(), prerequisites.end()),
    m_exclusions(exclusions.begin(), exclusions.end()),
    m_effects([](auto&& effects, const auto& name) {
        std::vector<Effect::EffectsGroup> retval;
        retval.reserve(effects.size());
        for (auto& e : effects) {
            e->SetTopLevelContent(name);
            retval.push_back(std::move(*e));
        }
        return retval;
    }(effects, name)),
    m_unlocked_items(std::move(unlocked_items)),
    m_graphic(std::move(graphic))
{}

std::string Policy::Dump(uint8_t ntabs) const {
    std::string retval = DumpIndent(ntabs) + "Policy\n";
    retval += DumpIndent(ntabs+1) + "name = \"" + m_name + "\"\n";
    retval += DumpIndent(ntabs+1) + "description = \"" + m_description + "\"\n";
    retval += DumpIndent(ntabs+1) + "shortdescription = \"" + m_short_description + "\"\n";
    retval += DumpIndent(ntabs+1) + "category = \"" + m_category + "\"\n";
    retval += DumpIndent(ntabs+1) + "adoptioncost = " + m_adoption_cost->Dump(ntabs+1) + "\n";

    if (m_prerequisites.size() == 1) {
        retval += DumpIndent(ntabs+1) + "prerequisites = \"" + m_prerequisites.front() + "\"\n";
    } else if (m_prerequisites.size() > 1) {
        retval += DumpIndent(ntabs+1) + "prerequisites = [\n";
        for (const std::string& prerequisite : m_prerequisites)
            retval += DumpIndent(ntabs+2) + "\"" + prerequisite + "\"\n";
        retval += DumpIndent(ntabs+1) + "]\n";
    }

    if (m_exclusions.size() == 1) {
        retval += DumpIndent(ntabs+1) + "exclusions = \"" + m_exclusions.front() + "\"\n";
    } else if (m_exclusions.size() > 1) {
        retval += DumpIndent(ntabs+1) + "exclusions = [\n";
        for (const std::string& exclusion : m_exclusions)
            retval += DumpIndent(ntabs+2) + "\"" + exclusion + "\"\n";
        retval += DumpIndent(ntabs+1) + "]\n";
    }

    retval += DumpIndent(ntabs+1) + "unlock = ";
    if (m_unlocked_items.empty()) {
        retval += "[]\n";
    } else if (m_unlocked_items.size() == 1) {
        retval += m_unlocked_items.front().Dump();
    } else {
        retval += "[\n";
        for (const UnlockableItem& unlocked_item : m_unlocked_items)
            retval += DumpIndent(ntabs+2) + unlocked_item.Dump();
        retval += DumpIndent(ntabs+1) + "]\n";
    }

    if (!m_effects.empty()) {
        if (m_effects.size() == 1) {
            retval += DumpIndent(ntabs+1) + "effectsgroups =\n";
            retval += m_effects.front().Dump(ntabs+2);
        } else {
            retval += DumpIndent(ntabs+1) + "effectsgroups = [\n";
            for (auto& effect : m_effects)
                retval += effect.Dump(ntabs+2);
            retval += DumpIndent(ntabs+1) + "]\n";
        }
    }

    retval += DumpIndent(ntabs+1) + "graphic = \"" + m_graphic + "\"\n";
    return retval;
}

float Policy::AdoptionCost(int empire_id, const ScriptingContext& context) const {
    static constexpr auto arbitrary_large_number = 999999.9f;

    if (GetGameRules().Get<bool>("RULE_CHEAP_POLICIES") || !m_adoption_cost) {
        return 1.0f;

    } else if (m_adoption_cost->ConstantExpr()) {
        return static_cast<float>(m_adoption_cost->Eval());

    } else if (m_adoption_cost->SourceInvariant()) {
        return static_cast<float>(m_adoption_cost->Eval());

    } else if (empire_id == ALL_EMPIRES) {
        return arbitrary_large_number;

    } else {
        if (context.source)
            return static_cast<float>(m_adoption_cost->Eval(context));

        // get a source to reference in evaulation of cost value-ref
        auto empire = context.GetEmpire(empire_id);
        if (!empire)
            return arbitrary_large_number;
        auto source = empire->Source(context.ContextObjects());
        if (!source)
            return arbitrary_large_number;

        // construct new context with source specified
        const ScriptingContext source_context{context, ScriptingContext::Source{}, source.get()};
        return static_cast<float>(m_adoption_cost->Eval(source_context));
    }
}

uint32_t Policy::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, m_name);
    CheckSums::CheckSumCombine(retval, m_description);
    CheckSums::CheckSumCombine(retval, m_short_description);
    CheckSums::CheckSumCombine(retval, m_category);
    CheckSums::CheckSumCombine(retval, m_adoption_cost);
    CheckSums::CheckSumCombine(retval, m_effects);
    CheckSums::CheckSumCombine(retval, m_graphic);

    return retval;
}

///////////////////////////////////////////////////////////
// PolicyManager                                         //
///////////////////////////////////////////////////////////
const Policy* PolicyManager::GetPolicy(std::string_view name) const {
    CheckPendingPolicies();
    auto it = m_policies.find(name);
    return it == m_policies.end() ? nullptr : &it->second;
}

std::vector<std::string_view> PolicyManager::PolicyNames() const {
    CheckPendingPolicies();
    std::vector<std::string_view> retval;
    retval.reserve(m_policies.size());
    std::transform(m_policies.begin(), m_policies.end(), std::back_inserter(retval),
                   [](const auto& name_policy) -> std::string_view { return name_policy.first; });
    return retval;
}

std::vector<std::string_view> PolicyManager::PolicyNames(const std::string& category_name) const {
    CheckPendingPolicies();
    std::vector<std::string_view> retval;
    retval.reserve(m_policies.size());
    const auto in_category = [&category_name](const auto& p) { return p.second.Category() == category_name; };
    range_copy(m_policies | range_filter(in_category) | range_keys, std::back_inserter(retval));
    return retval;
}

std::vector<std::string_view> PolicyManager::PolicyCategories() const {
    CheckPendingPolicies();
    std::vector<std::string_view> retval;
    retval.reserve(12); // guesstimate
    std::transform(m_policies.begin(), m_policies.end(), std::back_inserter(retval),
                   [](const auto& name_policy) -> std::string_view { return name_policy.second.Category(); });
    std::sort(retval.begin(), retval.end());
    auto unique_it = std::unique(retval.begin(), retval.end());
    retval.erase(unique_it, retval.end());
    return retval;
}

PolicyManager::iterator PolicyManager::begin() const {
    CheckPendingPolicies();
    return m_policies.begin();
}

PolicyManager::iterator PolicyManager::end() const {
    CheckPendingPolicies();
    return m_policies.end();
}

void PolicyManager::CheckPendingPolicies() const {
    if (!m_pending_types)
        return;

    auto parsed = WaitForPending(m_pending_types);
    if (!parsed)
        return;
    auto parsed_vec{std::move(*parsed)};
    if (parsed_vec.empty())
        return;

    std::sort(parsed_vec.begin(), parsed_vec.end(),
              [](const auto& lhs, const auto& rhs) { return lhs.Name() < rhs.Name(); });

    m_policies.reserve(parsed_vec.size());

    for (auto& policy : parsed_vec) {
        auto name{policy.Name()};
        m_policies.try_emplace(m_policies.end(), std::move(name), std::move(policy));
    }
}

uint32_t PolicyManager::GetCheckSum() const {
    CheckPendingPolicies();
    uint32_t retval{0};
    for (auto const& policy : m_policies)
        CheckSums::CheckSumCombine(retval, policy);
    CheckSums::CheckSumCombine(retval, m_policies.size());
    return retval;
}

void PolicyManager::SetPolicies(Pending::Pending<std::vector<Policy>>&& future)
{ m_pending_types = std::move(future); }

///////////////////////////////////////////////////////////
// Free Functions                                        //
///////////////////////////////////////////////////////////
[[nodiscard]] PolicyManager& GetPolicyManager() {
    static PolicyManager manager;
    return manager;
}

[[nodiscard]] const Policy* GetPolicy(std::string_view name)
{ return GetPolicyManager().GetPolicy(name); }

