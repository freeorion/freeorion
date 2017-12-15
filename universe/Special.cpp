#include "Special.h"

#include "Condition.h"
#include "Effect.h"
#include "UniverseObject.h"
#include "ValueRef.h"
#include "../util/OptionsDB.h"
#include "../util/Logger.h"
#include "../util/AppInterface.h"
#include "../util/CheckSums.h"
#include "../util/ScopedTimer.h"

#include <boost/filesystem/fstream.hpp>

SpecialsManager::SpecialsManager()
{}

SpecialsManager::~SpecialsManager()
{}

std::vector<std::string> SpecialsManager::SpecialNames() const {
    CheckPendingSpecialsTypes();
    std::vector<std::string> retval;
    for (const auto& entry : m_specials) {
        retval.push_back(entry.first);
    }
    return retval;
}

const Special* SpecialsManager::GetSpecial(const std::string& name) const {
    CheckPendingSpecialsTypes();
    auto it = m_specials.find(name);
    return it != m_specials.end() ? it->second.get() : nullptr;
}

unsigned int SpecialsManager::GetCheckSum() const {
    CheckPendingSpecialsTypes();
    unsigned int retval{0};
    for (auto const& name_type_pair : m_specials)
        CheckSums::CheckSumCombine(retval, name_type_pair);
    CheckSums::CheckSumCombine(retval, m_specials.size());
    DebugLogger() << "SpecialsManager checksum: " << retval;
    return retval;
}

void SpecialsManager::SetSpecialsTypes(Pending::Pending<SpecialsTypeMap>&& future)
{ m_pending_types = std::move(future); }

void SpecialsManager::CheckPendingSpecialsTypes() const {
    if (!m_pending_types)
        return;

    Pending::SwapPending(m_pending_types, m_specials);
}

SpecialsManager& GetSpecialsManager() {
    static SpecialsManager special_manager;
    return special_manager;
}

/////////////////////////////////////////////////
// Special                                     //
/////////////////////////////////////////////////
Special::Special(const std::string& name, const std::string& description,
                 std::unique_ptr<ValueRef::ValueRefBase<double>>&& stealth,
                 std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects,
                 double spawn_rate /*= 1.0*/, int spawn_limit /*= 99999*/,
                 std::unique_ptr<ValueRef::ValueRefBase<double>>&& initial_capaicty /*= nullptr*/,
                 std::unique_ptr<Condition::ConditionBase>&& location /*= nullptr*/,
                 const std::string& graphic /*= ""*/) :
    m_name(name),
    m_description(description),
    m_stealth(std::move(stealth)),
    m_effects(),
    m_spawn_rate(spawn_rate),
    m_spawn_limit(spawn_limit),
    m_initial_capacity(std::move(initial_capaicty)),
    m_location(std::move(location)),
    m_graphic(graphic)
{
    for (auto&& effect : effects)
        m_effects.emplace_back(std::move(effect));

    Init();
}

Special::~Special()
{}

std::string Special::Description() const {
    std::stringstream result;

    result << UserString(m_description) << "\n";

    for (auto& effect : m_effects) {
        const std::string& description = effect->GetDescription();

        if (!description.empty()) {
            result << "\n" << UserString(description) << "\n";
        }
    }

    return result.str();
}

void Special::Init() {
    if (m_stealth)
        m_stealth->SetTopLevelContent(m_name);
    for (auto& effect : m_effects) {
        effect->SetTopLevelContent(m_name);
    }
    if (m_initial_capacity)
        m_initial_capacity->SetTopLevelContent(m_name);
    if (m_location)
        m_location->SetTopLevelContent(m_name);
}

std::string Special::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "Special\n";
    retval += DumpIndent(ntabs+1) + "name = \"" + m_name + "\"\n";
    retval += DumpIndent(ntabs+1) + "description = \"" + m_description + "\"\n";

    if (m_stealth)
        retval += DumpIndent(ntabs+1) + "stealth = " + m_stealth->Dump(ntabs+1) + "\n";

    retval += DumpIndent(ntabs+1) + "spawnrate = " + std::to_string(m_spawn_rate) + "\n"
           +  DumpIndent(ntabs+1) + "spawnlimit = " + std::to_string(m_spawn_limit) + "\n";

    if (m_initial_capacity) {
        retval += DumpIndent(ntabs+1) + "initialcapacity = ";
        retval += m_initial_capacity->Dump(ntabs+2);
    }

    if (m_location) {
        retval += DumpIndent(ntabs+1) + "location =\n";
        retval += m_location->Dump(ntabs+2);
    }

    if (m_effects.size() == 1) {
        retval += DumpIndent(ntabs+1) + "effectsgroups =\n";
        retval += m_effects[0]->Dump(ntabs+2);
    } else {
        retval += DumpIndent(ntabs+1) + "effectsgroups = [\n";
        for (auto& effect : m_effects)
            retval += effect->Dump(ntabs+2);
        retval += DumpIndent(ntabs+1) + "]\n";
    }
    retval += DumpIndent(ntabs+1) + "graphic = \"" + m_graphic + "\"\n";
    return retval;
}

float Special::InitialCapacity(int object_id) const {
    if (!m_initial_capacity)
        return 0.0f;

    auto obj = GetUniverseObject(object_id);
    if (!obj)
        return 0.0f;

    ScriptingContext context(obj);

    return m_initial_capacity->Eval(context);
}

unsigned int Special::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, m_name);
    CheckSums::CheckSumCombine(retval, m_description);
    CheckSums::CheckSumCombine(retval, m_stealth);
    CheckSums::CheckSumCombine(retval, m_effects);
    CheckSums::CheckSumCombine(retval, m_spawn_rate);
    CheckSums::CheckSumCombine(retval, m_spawn_limit);
    CheckSums::CheckSumCombine(retval, m_initial_capacity);
    CheckSums::CheckSumCombine(retval, m_location);
    CheckSums::CheckSumCombine(retval, m_graphic);

    return retval;
}

const Special* GetSpecial(const std::string& name)
{ return GetSpecialsManager().GetSpecial(name); }

std::vector<std::string> SpecialNames()
{ return GetSpecialsManager().SpecialNames(); }
