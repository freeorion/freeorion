#include "FieldType.h"

#include <boost/algorithm/string/case_conv.hpp>
#include "ConditionSource.h"
#include "Effects.h"
#include "Enums.h"
#include "UniverseObject.h"
#include "ValueRefs.h"
#include "../util/CheckSums.h"


namespace {
    auto IncreaseMeter(MeterType meter_type, double increase) {
        auto scope = std::make_unique<Condition::Source>();

        auto vr = std::make_unique<ValueRef::Operation<double>>(
            ValueRef::OpType::PLUS,
            std::make_unique<ValueRef::Variable<double>>(ValueRef::ReferenceType::EFFECT_TARGET_VALUE_REFERENCE),
            std::make_unique<ValueRef::Constant<double>>(increase)
        );

        std::vector<std::unique_ptr<Effect::Effect>> effects;
        effects.push_back(std::make_unique<Effect::SetMeter>(meter_type, std::move(vr)));

        return Effect::EffectsGroup{std::move(scope), nullptr, std::move(effects)};
    }
}


FieldType::FieldType(std::string&& name, std::string&& description,
                     float stealth, const std::set<std::string>& tags,
                     std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects,
                     std::string&& graphic) :
    m_name(name), // not a move so available later in member initializer list
    m_description(std::move(description)),
    m_stealth(stealth),
    m_tags_concatenated([&tags]() {
        // allocate storage for concatenated tags
        std::size_t params_sz = std::transform_reduce(tags.begin(), tags.end(), 0u, std::plus{},
                                                      [](const auto& tag) { return tag.size(); });
        std::string retval;
        retval.reserve(params_sz);

        // concatenate tags
        std::for_each(tags.begin(), tags.end(), [&retval](const auto& t)
        { retval.append(boost::to_upper_copy<std::string>(t)); });
        return retval;
    }()),
    m_tags([&tags, this]() {
        std::vector<std::string_view> retval;
        std::size_t next_idx = 0;
        retval.reserve(tags.size());
        std::string_view sv{m_tags_concatenated};

        // store views into concatenated tags string
        std::for_each(tags.begin(), tags.end(), [&next_idx, &retval, sv](const auto& t) {
            std::string upper_t = boost::to_upper_copy<std::string>(t);
            retval.push_back(sv.substr(next_idx, upper_t.size()));
            next_idx += upper_t.size();
        });
        return retval;
    }()),
    m_effects([](auto& effects, const auto& name) {
        std::vector<Effect::EffectsGroup> retval;
        retval.reserve(effects.size());
        for (auto& e : effects) {
            e->SetTopLevelContent(name);
            retval.push_back(std::move(*e));
        }
        return retval;
    }(effects, name)),
    m_graphic(std::move(graphic))
{
    if (m_stealth != 0.0f)
        m_effects.push_back(IncreaseMeter(MeterType::METER_STEALTH, m_stealth));
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

    return m_effects == rhs.m_effects;
}

std::string FieldType::Dump(uint8_t ntabs) const {
    std::string retval = DumpIndent(ntabs) + "FieldType\n";
    retval += DumpIndent(ntabs+1) + "name = \"" + m_name + "\"\n";
    retval += DumpIndent(ntabs+1) + "description = \"" + m_description + "\"\n";
    retval += DumpIndent(ntabs+1) + "location = \n";
    if (m_effects.size() == 1) {
        retval += DumpIndent(ntabs+1) + "effectsgroups =\n";
        retval += m_effects.front().Dump(ntabs+2);
    } else {
        retval += DumpIndent(ntabs+1) + "effectsgroups = [\n";
        for (auto& effect : m_effects) {
            retval += effect.Dump(ntabs+2);
        }
        retval += DumpIndent(ntabs+1) + "]\n";
    }
    retval += DumpIndent(ntabs+1) + "graphic = \"" + m_graphic + "\"\n";
    return retval;
}

uint32_t FieldType::GetCheckSum() const {
    uint32_t retval{0};

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

const FieldType* FieldTypeManager::GetFieldType(std::string_view name) const {
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

std::size_t FieldTypeManager::size() const {
    CheckPendingFieldTypes();
    return m_field_types.size();
}

FieldTypeManager& FieldTypeManager::GetFieldTypeManager() {
    static FieldTypeManager manager;
    return manager;
}

uint32_t FieldTypeManager::GetCheckSum() const {
    CheckPendingFieldTypes();
    uint32_t retval{0};
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

const FieldType* GetFieldType(std::string_view name)
{ return FieldTypeManager::GetFieldTypeManager().GetFieldType(name); }
