#include "Special.h"

#include <boost/filesystem/fstream.hpp>
#include "Condition.h"
#include "Effect.h"
#include "UniverseObject.h"
#include "ValueRef.h"
#include "../util/AppInterface.h"
#include "../util/CheckSums.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/ScopedTimer.h"
#include "../util/i18n.h"


#define CHECK_COND_VREF_MEMBER(m_ptr) { if (m_ptr == rhs.m_ptr) {            \
                                            /* check next member */          \
                                        } else if (!m_ptr || !rhs.m_ptr) {   \
                                            return false;                    \
                                        } else if (*m_ptr != *(rhs.m_ptr)) { \
                                            return false;                    \
                                        }   }

std::vector<std::string_view> SpecialsManager::SpecialNames() const {
    CheckPendingSpecialsTypes();
    return m_special_names;
}

const Special* SpecialsManager::GetSpecial(std::string_view name) const {
    CheckPendingSpecialsTypes();
    auto name_it = std::find(m_special_names.begin(), m_special_names.end(), name);
    if (name_it == m_special_names.end())
        return nullptr;
    auto offset = std::distance(m_special_names.begin(), name_it);
    return &m_specials[offset];
}

uint32_t SpecialsManager::GetCheckSum() const {
    CheckPendingSpecialsTypes();
    uint32_t retval{0};
    for (auto const& special : m_specials)
        CheckSums::CheckSumCombine(retval, special);
    CheckSums::CheckSumCombine(retval, m_specials.size());
    DebugLogger() << "SpecialsManager checksum: " << retval;
    return retval;
}

void SpecialsManager::SetSpecialsTypes(Pending::Pending<SpecialsTypeMap>&& future)
{ m_pending_types = std::move(future); }

void SpecialsManager::CheckPendingSpecialsTypes() const {
    if (!m_pending_types)
        return;

    std::scoped_lock lock(m_pending_types->m_mutex);
    if (!m_pending_types)
        return; // another thread in the meantime reset m_pending_types after transferring pending to stored

    if (auto tt = Pending::WaitForPendingUnlocked(std::move(*m_pending_types))) { // moving from contained object should / does not reset the optional
        // extract from optional
        SpecialsTypeMap temp;
        std::swap(*tt, temp);
        // TODO: validate all passed in pointers before using

        // copy to internal storage and make views
        std::size_t special_names_sz = 0;
        std::for_each(temp.begin(), temp.end(),
                      [&special_names_sz](const auto& s) { special_names_sz += s.first.size(); });
        m_concatenated_special_names.reserve(special_names_sz);
        m_special_names.reserve(temp.size());
        m_specials.reserve(temp.size());
        std::for_each(temp.begin(), temp.end(), [this](SpecialsTypeMap::value_type& s) {
            auto next_idx = m_concatenated_special_names.length();
            m_concatenated_special_names.append(s.first);
            m_special_names.push_back(
                std::string_view{m_concatenated_special_names}.substr(next_idx, s.first.size()));
            m_specials.push_back(std::move(*(s.second.release())));
        });
    }

    m_pending_types.reset(); // after processing, set pending to empty so future calls to this function will early exit and any waiting on the mutex will exit when it is available to them
}

SpecialsManager& GetSpecialsManager() {
    static SpecialsManager special_manager;
    return special_manager;
}

/////////////////////////////////////////////////
// Special                                     //
/////////////////////////////////////////////////
Special::Special(std::string&& name, std::string&& description,
                 std::unique_ptr<ValueRef::ValueRef<double>>&& stealth,
                 std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects,
                 double spawn_rate, int spawn_limit,
                 std::unique_ptr<ValueRef::ValueRef<double>>&& initial_capaicty,
                 std::unique_ptr<Condition::Condition>&& location,
                 const std::string& graphic) :
    m_name(name), // not moving so usable below
    m_description(std::move(description)),
    m_stealth(std::move(stealth)),
    m_effects([](auto& effects, const auto& name) {
        std::vector<Effect::EffectsGroup> retval;
        retval.reserve(effects.size());
        for (auto& e : effects) {
            e->SetTopLevelContent(name);
            retval.push_back(std::move(*e));
        }
        return retval;
    }(effects, name)),
    m_spawn_rate(spawn_rate),
    m_spawn_limit(spawn_limit),
    m_initial_capacity(std::move(initial_capaicty)),
    m_location(std::move(location)),
    m_graphic(graphic)
{
    Init();
}

Special::~Special() = default;

bool Special::operator==(const Special& rhs) const {
    if (&rhs == this)
        return true;

    if (m_name != rhs.m_name ||
        m_description != rhs.m_description ||
        m_spawn_rate != rhs.m_spawn_rate ||
        m_spawn_limit != rhs.m_spawn_limit ||
        m_graphic != rhs.m_graphic)
    { return false; }

    CHECK_COND_VREF_MEMBER(m_stealth)
    CHECK_COND_VREF_MEMBER(m_initial_capacity)
    CHECK_COND_VREF_MEMBER(m_location)

    return m_effects == rhs.m_effects;
}

std::string Special::Description() const {
    std::stringstream result;

    result << UserString(m_description) << "\n";

    for (auto& effect : m_effects) {
        const auto& description = effect.GetDescription();
        if (!description.empty())
            result << "\n" << UserString(description) << "\n";
    }

    return result.str();
}

void Special::Init() {
    if (m_stealth)
        m_stealth->SetTopLevelContent(m_name);
    if (m_initial_capacity)
        m_initial_capacity->SetTopLevelContent(m_name);
    if (m_location)
        m_location->SetTopLevelContent(m_name);
}

std::string Special::Dump(uint8_t ntabs) const {
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
        retval += m_effects.front().Dump(ntabs+2);
    } else {
        retval += DumpIndent(ntabs+1) + "effectsgroups = [\n";
        for (auto& effect : m_effects)
            retval += effect.Dump(ntabs+2);
        retval += DumpIndent(ntabs+1) + "]\n";
    }
    retval += DumpIndent(ntabs+1) + "graphic = \"" + m_graphic + "\"\n";
    return retval;
}

float Special::InitialCapacity(int object_id, const ScriptingContext& context) const {
    if (!m_initial_capacity)
        return 0.0f;

    auto obj = context.ContextObjects().getRaw(object_id);
    if (!obj)
        return 0.0f;

    const ScriptingContext local_context{context, ScriptingContext::Source{}, obj};
    return m_initial_capacity->Eval(local_context);
}

uint32_t Special::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, m_name);
    CheckSums::CheckSumCombine(retval, m_description);
    CheckSums::CheckSumCombine(retval, m_stealth);
    CheckSums::CheckSumCombine(retval, m_effects);
    CheckSums::CheckSumCombine(retval, m_initial_capacity);
    CheckSums::CheckSumCombine(retval, m_location);
    CheckSums::CheckSumCombine(retval, m_graphic);

    return retval;
}

const Special* GetSpecial(std::string_view name)
{ return GetSpecialsManager().GetSpecial(std::string{name}); }

std::vector<std::string_view> SpecialNames()
{ return GetSpecialsManager().SpecialNames(); }
