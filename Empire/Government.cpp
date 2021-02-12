#include "Government.h"

#include "../universe/Effect.h"
#include "../universe/UniverseObject.h"
#include "../universe/ObjectMap.h"
#include "../universe/ValueRef.h"
#include "../util/OptionsDB.h"
#include "../util/Logger.h"
#include "../util/GameRules.h"
#include "../util/MultiplayerCommon.h"
#include "../util/GameRules.h"
#include "../util/CheckSums.h"
#include "../util/ScopedTimer.h"
#include "../Empire/EmpireManager.h"

#include <boost/filesystem/fstream.hpp>

namespace {
    #define UserStringNop(key) key

    void AddRules(GameRules& rules) {
        // makes all policies cost 1 influence to adopt
        rules.Add<bool>(UserStringNop("RULE_CHEAP_POLICIES"), UserStringNop("RULE_CHEAP_POLICIES_DESC"),
                        "", false, true);
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
               std::string graphic) :
    m_name(std::move(name)),
    m_description(std::move(description)),
    m_short_description(std::move(short_description)),
    m_category(std::move(category)),
    m_adoption_cost(std::move(adoption_cost)),
    m_prerequisites(std::move(prerequisites)),
    m_exclusions(std::move(exclusions)),
    m_graphic(std::move(graphic))
{
    for (auto&& effect : effects)
        m_effects.emplace_back(std::move(effect));

    if (m_adoption_cost)
        m_adoption_cost->SetTopLevelContent(m_name);

    for (auto& effect : m_effects)
        effect->SetTopLevelContent(m_name);
}

std::string Policy::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "Policy\n";
    retval += DumpIndent(ntabs+1) + "name = \"" + m_name + "\"\n";
    retval += DumpIndent(ntabs+1) + "description = \"" + m_description + "\"\n";
    retval += DumpIndent(ntabs+1) + "shortdescription = \"" + m_short_description + "\"\n";
    retval += DumpIndent(ntabs+1) + "category = \"" + m_category + "\"\n";
    retval += DumpIndent(ntabs+1) + "adoptioncost = " + m_adoption_cost->Dump(ntabs+1) + "\n";

    if (m_prerequisites.size() == 1) {
        retval += DumpIndent(ntabs+1) + "prerequisites = \"" + *m_prerequisites.begin() + "\"\n";
    } else if (m_prerequisites.size() > 1) {
        retval += DumpIndent(ntabs+1) + "prerequisites = [\n";
        for (const std::string& prerequisite : m_prerequisites)
            retval += DumpIndent(ntabs+2) + "\"" + prerequisite + "\"\n";
        retval += DumpIndent(ntabs+1) + "]\n";
    }

    if (m_exclusions.size() == 1) {
        retval += DumpIndent(ntabs+1) + "exclusions = \"" + *m_exclusions.begin() + "\"\n";
    } else if (m_exclusions.size() > 1) {
        retval += DumpIndent(ntabs+1) + "exclusions = [\n";
        for (const std::string& exclusion : m_exclusions)
            retval += DumpIndent(ntabs+2) + "\"" + exclusion + "\"\n";
        retval += DumpIndent(ntabs+1) + "]\n";
    }

    if (!m_effects.empty()) {
        if (m_effects.size() == 1) {
            retval += DumpIndent(ntabs+1) + "effectsgroups =\n";
            retval += m_effects[0]->Dump(ntabs+2);
        } else {
            retval += DumpIndent(ntabs+1) + "effectsgroups = [\n";
            for (auto& effect : m_effects)
                retval += effect->Dump(ntabs+2);
            retval += DumpIndent(ntabs+1) + "]\n";
        }
    }

    retval += DumpIndent(ntabs+1) + "graphic = \"" + m_graphic + "\"\n";
    return retval;
}

float Policy::AdoptionCost(int empire_id, const ObjectMap& objects) const {
    const auto arbitrary_large_number = 999999.9f;

    if (GetGameRules().Get<bool>("RULE_CHEAP_POLICIES") || !m_adoption_cost) {
        return 1.0;

    } else if (m_adoption_cost->ConstantExpr()) {
        return m_adoption_cost->Eval();

    } else if (m_adoption_cost->SourceInvariant()) {
        return m_adoption_cost->Eval();

    } else if (empire_id == ALL_EMPIRES) {
        return arbitrary_large_number;

    } else {
        auto source = Empires().GetSource(empire_id, objects);
        if (!source && !m_adoption_cost->SourceInvariant())
            return arbitrary_large_number;

        const ScriptingContext context(std::move(source));
        return m_adoption_cost->Eval(context);
    }
}

unsigned int Policy::GetCheckSum() const {
    unsigned int retval{0};

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
const Policy* PolicyManager::GetPolicy(const std::string& name) const {
    CheckPendingPolicies();
    auto it = m_policies.find(name);
    return it == m_policies.end() ? nullptr : it->second.get();
}

std::vector<std::string> PolicyManager::PolicyNames() const {
    CheckPendingPolicies();
    std::vector<std::string> retval;
    for (const auto& policy : m_policies)
        retval.emplace_back(policy.first);
    return retval;
}

std::vector<std::string> PolicyManager::PolicyNames(const std::string& name) const {
    CheckPendingPolicies();
    std::vector<std::string> retval;
    for (const auto& policy : m_policies)
        if (policy.second->Category() == name)
            retval.emplace_back(policy.first);
    return retval;
}

std::set<std::string> PolicyManager::PolicyCategories() const {
    CheckPendingPolicies();
    std::set<std::string> retval;
    for (const auto& policy : m_policies)
        retval.emplace(policy.second->Category());
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

PolicyManager::PolicyManager()
{}

PolicyManager::~PolicyManager()
{}

void PolicyManager::CheckPendingPolicies() const {
    if (!m_pending_types)
        return;

    Pending::SwapPending(m_pending_types, m_policies);
}

unsigned int PolicyManager::GetCheckSum() const {
    CheckPendingPolicies();
    unsigned int retval{0};
    for (auto const& policy : m_policies)
        CheckSums::CheckSumCombine(retval, policy);
    CheckSums::CheckSumCombine(retval, m_policies.size());

    DebugLogger() << "PolicyManager checksum: " << retval;
    return retval;
}

void PolicyManager::SetPolicies(Pending::Pending<PoliciesTypeMap>&& future)
{ m_pending_types = std::move(future); }

///////////////////////////////////////////////////////////
// Free Functions                                        //
///////////////////////////////////////////////////////////
PolicyManager& GetPolicyManager() {
    static PolicyManager manager;
    return manager;
}

const Policy* GetPolicy(const std::string& name)
{ return GetPolicyManager().GetPolicy(name); }
