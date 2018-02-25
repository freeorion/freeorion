#include "Government.h"

#include "../universe/Effect.h"
#include "../universe/UniverseObject.h"
#include "../universe/ObjectMap.h"
#include "../util/OptionsDB.h"
#include "../util/Logger.h"
#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"
#include "../util/CheckSums.h"
#include "../util/ScopedTimer.h"
#include "../universe/ValueRef.h"
#include "../universe/Enums.h"
#include "../Empire/EmpireManager.h"

#include <boost/filesystem/fstream.hpp>

namespace {
    void AddRules(GameRules& rules) {
        // makes all policies cost 1 influence to adopt
        rules.Add<bool>("RULE_CHEAP_POLICIES",
                        "RULE_CHEAP_POLICIES_DESC",
                        "", false, true);
    }
    bool temp_bool = RegisterGameRules(&AddRules);
}

///////////////////////////////////////////////////////////
// Policy                                                //
///////////////////////////////////////////////////////////
Policy::Policy(const std::string& name, const std::string& description,
               const std::string& short_description, const std::string& category,
               std::unique_ptr<ValueRef::ValueRefBase<double>>&& adoption_cost,
               std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects,
               const std::string& graphic) :
    m_name(name),
    m_description(description),
    m_short_description(short_description),
    m_category(category),
    m_adoption_cost(std::move(adoption_cost)),
    m_effects(),
    m_graphic(graphic)
{
    for (auto&& effect : effects)
        m_effects.emplace_back(std::move(effect));

    Init();
}

Policy::~Policy()
{}

void Policy::Init() {
    if (m_adoption_cost)
        m_adoption_cost->SetTopLevelContent(m_name);

    for (auto& effect : m_effects)
    { effect->SetTopLevelContent(m_name); }
}

std::string Policy::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "Policy\n";
    retval += DumpIndent(ntabs+1) + "name = \"" + m_name + "\"\n";
    retval += DumpIndent(ntabs+1) + "description = \"" + m_description + "\"\n";
    retval += DumpIndent(ntabs+1) + "shortdescription = \"" + m_short_description + "\"\n";
    retval += DumpIndent(ntabs+1) + "category = \"" + m_category + "\"\n";
    retval += DumpIndent(ntabs+1) + "adoptioncost = " + m_adoption_cost->Dump(ntabs+1) + "\n";
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

float Policy::AdoptionCost(int empire_id) const {
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
        auto source = Empires().GetSource(empire_id);
        if (!source && !m_adoption_cost->SourceInvariant())
            return arbitrary_large_number;

        ScriptingContext context(source);
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
        retval.push_back(policy.first);
    return retval;
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
