#include "FieldType.h"

#include <boost/algorithm/string/case_conv.hpp>
#include "ConditionSource.h"
#include "Effects.h"
#include "Enums.h"
#include "UniverseObject.h"
#include "ValueRefs.h"
#include "../util/CheckSums.h"


namespace {
    std::shared_ptr<Effect::EffectsGroup>
    IncreaseMeter(MeterType meter_type, double increase) {
        auto scope = std::make_unique<Condition::Source>();

        auto vr =
            std::make_unique<ValueRef::Operation<double>>(
                ValueRef::OpType::PLUS,
                std::make_unique<ValueRef::Variable<double>>(ValueRef::ReferenceType::EFFECT_TARGET_VALUE_REFERENCE),
                std::make_unique<ValueRef::Constant<double>>(increase)
            );
        std::vector<std::unique_ptr<Effect::Effect>> effects;
        effects.emplace_back(std::make_unique<Effect::SetMeter>(meter_type, std::move(vr)));

        return std::make_shared<Effect::EffectsGroup>(std::move(scope), nullptr, std::move(effects));
    }
}


FieldType::FieldType(std::string&& name, std::string&& description,
                     float stealth, const std::set<std::string>& tags,
                     std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects,
                     std::string&& graphic) :
    m_name(std::move(name)),
    m_description(std::move(description)),
    m_stealth(stealth),
    m_graphic(std::move(graphic))
{
    for (const std::string& tag : tags)
        m_tags.emplace(boost::to_upper_copy<std::string>(tag));

    for (auto&& effect : effects)
        m_effects.emplace_back(std::move(effect));

    if (m_stealth != 0.0f)
        m_effects.emplace_back(IncreaseMeter(MeterType::METER_STEALTH, m_stealth));

    for (auto& effect : m_effects)
        effect->SetTopLevelContent(m_name);
}

bool FieldType::operator==(const FieldType& rhs) const {
    if (&rhs == this)
        return true;

    if (m_name != rhs.m_name ||
        m_description != rhs.m_description ||
        m_stealth != rhs.m_stealth ||
        m_tags != rhs.m_tags ||
        m_graphic != rhs.m_graphic)
    { return false; }

    if (m_effects.size() != rhs.m_effects.size())
        return false;
    try {
        for (std::size_t idx = 0; idx < m_effects.size(); ++idx) {
            const auto& my_op = m_effects.at(idx);
            const auto& rhs_op = rhs.m_effects.at(idx);

            if (my_op == rhs_op) // could both be nullptr
                continue;
            if (!my_op || !rhs_op)
                return false;
            if (*my_op != *rhs_op)
                return false;
        }
    } catch (...) {
        return false;
    }

    return true;
}

std::string FieldType::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "FieldType\n";
    retval += DumpIndent(ntabs+1) + "name = \"" + m_name + "\"\n";
    retval += DumpIndent(ntabs+1) + "description = \"" + m_description + "\"\n";
    retval += DumpIndent(ntabs+1) + "location = \n";
    if (m_effects.size() == 1) {
        retval += DumpIndent(ntabs+1) + "effectsgroups =\n";
        retval += m_effects[0]->Dump(ntabs+2);
    } else {
        retval += DumpIndent(ntabs+1) + "effectsgroups = [\n";
        for (auto& effect : m_effects) {
            retval += effect->Dump(ntabs+2);
        }
        retval += DumpIndent(ntabs+1) + "]\n";
    }
    retval += DumpIndent(ntabs+1) + "graphic = \"" + m_graphic + "\"\n";
    return retval;
}

unsigned int FieldType::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, m_name);
    CheckSums::CheckSumCombine(retval, m_description);
    CheckSums::CheckSumCombine(retval, m_stealth);
    CheckSums::CheckSumCombine(retval, m_tags);
    CheckSums::CheckSumCombine(retval, m_effects);
    CheckSums::CheckSumCombine(retval, m_graphic);

    TraceLogger() << "FieldType checksum: " << retval;
    return retval;
}


FieldTypeManager* FieldTypeManager::s_instance = nullptr;

FieldTypeManager::FieldTypeManager() {
    if (s_instance)
        throw std::runtime_error("Attempted to create more than one FieldTypeManager.");

    // Only update the global pointer on sucessful construction.
    s_instance = this;
}

const FieldType* FieldTypeManager::GetFieldType(const std::string& name) const {
    CheckPendingFieldTypes();
    auto it = m_field_types.find(name);
    return it != m_field_types.end() ? it->second.get() : nullptr;
}

FieldTypeManager::iterator FieldTypeManager::begin() const {
    CheckPendingFieldTypes();
    return m_field_types.begin();
}

FieldTypeManager::iterator FieldTypeManager::end() const {
    CheckPendingFieldTypes();
    return m_field_types.end();
}

FieldTypeManager& FieldTypeManager::GetFieldTypeManager() {
    static FieldTypeManager manager;
    return manager;
}

unsigned int FieldTypeManager::GetCheckSum() const {
    CheckPendingFieldTypes();
    unsigned int retval{0};
    for (auto const& name_type_pair : m_field_types)
        CheckSums::CheckSumCombine(retval, name_type_pair);
    CheckSums::CheckSumCombine(retval, m_field_types.size());

    DebugLogger() << "FieldTypeManager checksum: " << retval;
    return retval;
}

void FieldTypeManager::SetFieldTypes(Pending::Pending<container_type>&& future)
{ m_pending_types = std::move(future); }

void FieldTypeManager::CheckPendingFieldTypes() const
{ Pending::SwapPending(m_pending_types, m_field_types); }


FieldTypeManager& GetFieldTypeManager()
{ return FieldTypeManager::GetFieldTypeManager(); }

const FieldType* GetFieldType(const std::string& name)
{ return FieldTypeManager::GetFieldTypeManager().GetFieldType(name); }
