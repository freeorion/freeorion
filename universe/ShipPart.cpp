#include "ShipPart.h"

#include <boost/algorithm/string/case_conv.hpp>
#include "ConditionSource.h"
#include "Effects.h"
#include "ShipHull.h"
#include "ValueRefs.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Empire.h"
#include "../util/CheckSums.h"
#include "../util/GameRules.h"

#define CHECK_COND_VREF_MEMBER(m_ptr) { if (m_ptr == rhs.m_ptr) {            \
                                            /* check next member */          \
                                        } else if (!m_ptr || !rhs.m_ptr) {   \
                                            return false;                    \
                                        } else if (*m_ptr != *(rhs.m_ptr)) { \
                                            return false;                    \
                                        }   }

namespace {
    constexpr int ARBITRARY_LARGE_TURNS = 999999;
    constexpr float ARBITRARY_LARGE_COST = 999999.9f;

    // create effectsgroup that increases the value of \a meter_type
    // by the result of evalulating \a increase_vr
    auto IncreaseMeter(MeterType meter_type,
                       std::unique_ptr<ValueRef::ValueRef<double>>&& increase_vr)
    {
        auto scope = std::make_unique<Condition::Source>();
        auto activation = std::make_unique<Condition::Source>();

        auto vr = std::make_unique<ValueRef::Operation<double>>(
            ValueRef::OpType::PLUS,
            std::make_unique<ValueRef::Variable<double>>(ValueRef::ReferenceType::EFFECT_TARGET_VALUE_REFERENCE),
            std::move(increase_vr)
        );
        std::vector<std::unique_ptr<Effect::Effect>> effects;
        effects.push_back(std::make_unique<Effect::SetMeter>(meter_type, std::move(vr)));

        return Effect::EffectsGroup{std::move(scope), std::move(activation), std::move(effects)};
    }

    // create effectsgroup that increases the value of \a meter_type
    // by the specified amount \a fixed_increase
    auto IncreaseMeter(MeterType meter_type, float fixed_increase) {
        auto increase_vr = std::make_unique<ValueRef::Constant<double>>(fixed_increase);
        return IncreaseMeter(meter_type, std::move(increase_vr));
    }

    // create effectsgroup that increases the value of the part meter
    // of type \a meter_type for part name \a part_name
    // by the result of evalulating \a increase_vr
    auto IncreaseMeter(MeterType meter_type, const std::string& part_name,
                       std::unique_ptr<ValueRef::ValueRef<double>>&& increase_vr, bool allow_stacking = true)
    {
        auto scope = std::make_unique<Condition::Source>();
        auto activation = std::make_unique<Condition::Source>();

        auto value_vr = std::make_unique<ValueRef::Operation<double>>(
            ValueRef::OpType::PLUS,
            std::make_unique<ValueRef::Variable<double>>(ValueRef::ReferenceType::EFFECT_TARGET_VALUE_REFERENCE),
            std::move(increase_vr)
        );

        auto part_name_vr =
            std::make_unique<ValueRef::Constant<std::string>>(part_name);

        std::string stacking_group = allow_stacking ? "" :
            (std::string{part_name}.append("_").append(to_string(meter_type)).append("_PartMeter"));

        std::vector<std::unique_ptr<Effect::Effect>> effects;
        effects.emplace_back(std::make_unique<Effect::SetShipPartMeter>(
            meter_type, std::move(part_name_vr), std::move(value_vr)));

        return Effect::EffectsGroup{std::move(scope), std::move(activation), std::move(effects),
                                    part_name, stacking_group};
    }

    // create effectsgroup that increases the value of \a meter_type
    // by the product of \a base_increase and the value of the game
    // rule of type double with the name \a scaling_factor_rule_name
    auto IncreaseMeterRuleScaled(MeterType meter_type, float base_increase,
                                 const std::string& scaling_factor_rule_name)
    {
        // if no rule specified, revert to fixed constant increase
        if (scaling_factor_rule_name.empty())
            return IncreaseMeter(meter_type, base_increase);

        auto increase_vr = std::make_unique<ValueRef::Operation<double>>(
            ValueRef::OpType::TIMES,
            std::make_unique<ValueRef::Constant<double>>(base_increase),
            std::make_unique<ValueRef::ComplexVariable<double>>(
                "GameRule", nullptr, nullptr, nullptr,
                std::make_unique<ValueRef::Constant<std::string>>(scaling_factor_rule_name)
            )
        );

        return IncreaseMeter(meter_type, std::move(increase_vr));
    }

    // create effectsgroup that increases the value of the part meter
    // of type \a meter_type for part name \a part_name by the fixed
    // amount \a fixed_increase
    auto IncreaseMeter(MeterType meter_type, const std::string& part_name,
                       float fixed_increase, bool allow_stacking = true)
    {
        auto increase_vr = std::make_unique<ValueRef::Constant<double>>(fixed_increase);
        return IncreaseMeter(meter_type, part_name, std::move(increase_vr), allow_stacking);
    }

    // create effectsgroup that increases the value of the part meter
    // of type \a meter_type for part name \a part_name by the fixed
    // amount \a base_increase and the value of the game
    // rule of type double with the name \a scaling_factor_rule_name
    auto IncreaseMeterRuleScaled(MeterType meter_type, const std::string& part_name,
                                 float base_increase, const std::string& scaling_factor_rule_name,
                                 bool allow_stacking = true)
    {
        // if no rule specified, revert to fixed constant increase
        if (scaling_factor_rule_name.empty())
            return IncreaseMeter(meter_type, part_name, base_increase, allow_stacking);

        auto increase_vr = std::make_unique<ValueRef::Operation<double>>(
            ValueRef::OpType::TIMES,
            std::make_unique<ValueRef::Constant<double>>(base_increase),
            std::make_unique<ValueRef::ComplexVariable<double>>(
                "GameRule", nullptr, nullptr, nullptr,
                std::make_unique<ValueRef::Constant<std::string>>(scaling_factor_rule_name)
            )
        );

        return IncreaseMeter(meter_type, part_name, std::move(increase_vr), allow_stacking);
    }

    auto InitEffects(std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects,
                     const std::string& name, bool add_standard_capacity_effect,
                     float capacity, float secondary_stat, ShipPartClass part_class)
    {
        std::vector<Effect::EffectsGroup> retval;
        retval.reserve(effects.size() + 2);

        if ((capacity != 0 || secondary_stat != 0) && add_standard_capacity_effect) {
            switch (part_class) {
            case ShipPartClass::PC_COLONY:
            case ShipPartClass::PC_TROOPS:
                retval.push_back(IncreaseMeter(MeterType::METER_CAPACITY,                     name, capacity, false));
                break;
            case ShipPartClass::PC_FIGHTER_HANGAR: {   // capacity indicates how many fighters are stored in this type of part (combined for all copies of the part)
                retval.push_back(IncreaseMeter(MeterType::METER_MAX_CAPACITY,                 name, capacity, true));         // stacking capacities allowed for this part, so each part contributes to the total capacity
                retval.push_back(IncreaseMeterRuleScaled(MeterType::METER_MAX_SECONDARY_STAT, name, secondary_stat, "RULE_FIGHTER_DAMAGE_FACTOR",     false));  // stacking damage not allowed, as damage per shot should be the same regardless of number of shots
                break;
            }
            case ShipPartClass::PC_FIGHTER_BAY: {      // capacity indicates how many fighters each instance of the part can launch per combat bout...
                retval.push_back(IncreaseMeter(MeterType::METER_MAX_CAPACITY,                 name, capacity, false));
                retval.push_back(IncreaseMeter(MeterType::METER_MAX_SECONDARY_STAT,           name, secondary_stat, false));
                break;
            }
            case ShipPartClass::PC_DIRECT_WEAPON: {    // capacity indicates weapon damage per shot
                retval.push_back(IncreaseMeterRuleScaled(MeterType::METER_MAX_CAPACITY,       name, capacity,       "RULE_SHIP_WEAPON_DAMAGE_FACTOR", false));
                retval.push_back(IncreaseMeter(MeterType::METER_MAX_SECONDARY_STAT,           name, secondary_stat, false));
                break;
            }
            case ShipPartClass::PC_SHIELD: {
                retval.push_back(IncreaseMeterRuleScaled(MeterType::METER_MAX_SHIELD,    capacity,     "RULE_SHIP_WEAPON_DAMAGE_FACTOR"));
                break;
            }
            case ShipPartClass::PC_DETECTION: {
                retval.push_back(IncreaseMeter(MeterType::METER_DETECTION,               capacity));
                break;
            }
            case ShipPartClass::PC_STEALTH: {
                retval.push_back(IncreaseMeter(MeterType::METER_STEALTH,                 capacity));
                break;
            }
            case ShipPartClass::PC_FUEL: {
                retval.push_back(IncreaseMeter(MeterType::METER_MAX_FUEL,                capacity));
                break;
            }
            case ShipPartClass::PC_ARMOUR: {
                retval.push_back(IncreaseMeterRuleScaled(MeterType::METER_MAX_STRUCTURE, capacity,     "RULE_SHIP_STRUCTURE_FACTOR"));
                break;
            }
            case ShipPartClass::PC_SPEED: {
                retval.push_back(IncreaseMeterRuleScaled(MeterType::METER_SPEED,         capacity,     "RULE_SHIP_SPEED_FACTOR"));
                break;
            }
            case ShipPartClass::PC_RESEARCH: {
                retval.push_back(IncreaseMeter(MeterType::METER_TARGET_RESEARCH,         capacity));
                break;
            }
            case ShipPartClass::PC_INDUSTRY: {
                retval.push_back(IncreaseMeter(MeterType::METER_TARGET_INDUSTRY,         capacity));
                break;
            }
            case ShipPartClass::PC_INFLUENCE: {
                retval.push_back(IncreaseMeter(MeterType::METER_TARGET_INFLUENCE,        capacity));
                break;
            }
            default:
                break;
            }
        }

        for (auto& effect : effects) {
            effect->SetTopLevelContent(name);
            retval.push_back(std::move(*effect));
        }

        return retval;
    }
}


ShipPart::ShipPart(ShipPartClass part_class, double capacity, double stat2,
                   CommonParams&& common_params, std::string&& name,
                   std::string&& description, std::set<std::string>&& exclusions,
                   std::vector<ShipSlotType> mountable_slot_types,
                   std::string&& icon, bool add_standard_capacity_effect,
                   std::unique_ptr<Condition::Condition>&& combat_targets,
                   std::unique_ptr<ValueRef::ValueRef<double>>&& total_fighter_damage,
                   std::unique_ptr<ValueRef::ValueRef<double>>&& total_ship_damage) :
    m_name(name), // no move so available below
    m_description(std::move(description)),
    m_class(part_class),
    m_capacity(capacity),
    m_secondary_stat(stat2),
    m_production_cost([](auto&& pc, const std::string& name) {
        if (pc)
            pc->SetTopLevelContent(name);
        return std::move(pc);
    }(std::move(common_params.production_cost), name)),
    m_production_time([](auto&& pt, const std::string& name) {
        if (pt)
            pt->SetTopLevelContent(name);
        return std::move(pt);
    }(std::move(common_params.production_time), name)),
    m_mountable_slot_types(std::move(mountable_slot_types)),
    m_tags_concatenated([&common_params]() {
        // ensure tags are all upper-case
        std::for_each(common_params.tags.begin(), common_params.tags.end(),
                      [](auto& t) { boost::to_upper<std::string>(t); });

        // allocate storage for concatenated tags
        std::size_t params_sz = std::transform_reduce(common_params.tags.begin(), common_params.tags.end(), 0u, std::plus{},
                                                      [](const auto& tag) { return tag.size(); });
        std::string retval;
        retval.reserve(params_sz);

        // concatenate tags
        std::for_each(common_params.tags.begin(), common_params.tags.end(),
                      [&retval](const auto& t) { retval.append(t); });
        return retval;
    }()),
    m_tags([&common_params, this]() {
        std::vector<std::string_view> retval;
        std::size_t next_idx = 0;
        retval.reserve(common_params.tags.size());
        std::string_view sv{m_tags_concatenated};

        // store views into concatenated tags string
        std::for_each(common_params.tags.begin(), common_params.tags.end(),
                      [&next_idx, &retval, sv](const auto& t)
        {
            auto tag = sv.substr(next_idx, t.size());
            retval.push_back(tag);
            next_idx += t.size();
        });
        return retval;
    }()),
    m_pedia_tags([&common_params, this]() {
        std::vector<std::string_view> retval;
        std::size_t next_idx = 0;
        retval.reserve(common_params.tags.size());
        std::string_view sv{m_tags_concatenated};

        // store views into concatenated tags string
        std::for_each(common_params.tags.begin(), common_params.tags.end(),
                      [&next_idx, &retval, sv](const auto& t)
        {
            auto tag = sv.substr(next_idx, t.size());
            static constexpr auto len{TAG_PEDIA_PREFIX.length()};
            if (tag.substr(0, len) == TAG_PEDIA_PREFIX)
                retval.push_back(tag);
            next_idx += t.size();
        });
        return retval;
    }()),
    m_production_meter_consumption(std::move(common_params.production_meter_consumption)),
    m_production_special_consumption(std::move(common_params.production_special_consumption)),
    m_location([](auto&& l, const std::string& name) {
        if (l)
            l->SetTopLevelContent(name);
        return std::move(l);
    }(std::move(common_params.location), name)),
    m_exclusions(exclusions.begin(), exclusions.end()),
    m_effects(InitEffects(std::move(common_params.effects), name,
                          add_standard_capacity_effect,
                          capacity, stat2, part_class)),
    m_icon(std::move(icon)),
    m_combat_targets([](auto&& c, const std::string& name) {
        if (c)
            c->SetTopLevelContent(name);
        return std::move(c);
    }(std::move(combat_targets), name)),
    m_total_fighter_damage(std::move(total_fighter_damage)),
    m_total_ship_damage(std::move(total_ship_damage)),
    m_add_standard_capacity_effect(add_standard_capacity_effect),
    m_producible(common_params.producible)
{}

ShipPart::~ShipPart() = default;

bool ShipPart::operator==(const ShipPart& rhs) const {
    if (&rhs == this)
        return true;

    if (m_name != rhs.m_name ||
        m_description != rhs.m_description ||
        m_class != rhs.m_class ||
        m_capacity != rhs.m_capacity ||
        m_secondary_stat != rhs.m_secondary_stat ||
        m_producible != rhs.m_producible ||
        m_mountable_slot_types != rhs.m_mountable_slot_types ||
        m_tags != rhs.m_tags ||
        m_exclusions != rhs.m_exclusions ||
        m_icon != rhs.m_icon ||
        m_add_standard_capacity_effect != rhs.m_add_standard_capacity_effect)
    { return false; }

    CHECK_COND_VREF_MEMBER(m_production_cost)
    CHECK_COND_VREF_MEMBER(m_production_time)
    CHECK_COND_VREF_MEMBER(m_location)
    CHECK_COND_VREF_MEMBER(m_total_fighter_damage)
    CHECK_COND_VREF_MEMBER(m_total_ship_damage)
    CHECK_COND_VREF_MEMBER(m_combat_targets)

    if (m_effects != rhs.m_effects)
        return false;

    if (m_production_meter_consumption.size() != rhs.m_production_meter_consumption.size())
        return false;
    try {
        for (auto& [meter_type, my_refs_cond_pair] : m_production_meter_consumption) {
            auto& [my_ref, my_cond] = my_refs_cond_pair;
            const auto& rhs_refs_cond_pair{rhs.m_production_meter_consumption.at(meter_type)};
            auto& [rhs_ref, rhs_cond] = rhs_refs_cond_pair;

            if (!my_ref && !rhs_ref && !my_cond && !rhs_cond)
                continue;
            if ((my_ref && !rhs_ref) || (!my_ref && rhs_ref))
                return false;
            if (*my_ref != *rhs_ref)
                return false;
            if ((my_cond && !rhs_cond) || (!my_cond && rhs_cond))
                return false;
            if (*my_cond != *rhs_cond)
                return false;
        }
    } catch (...) {
        return false;
    }

    if (m_production_meter_consumption.size() != rhs.m_production_meter_consumption.size())
        return false;
    try {
        for (auto& [meter_type, my_refs_cond_pair] : m_production_special_consumption) {
            auto& [my_ref, my_cond] = my_refs_cond_pair;
            const auto& rhs_refs_cond_pair{rhs.m_production_special_consumption.at(meter_type)};
            auto& [rhs_ref, rhs_cond] = rhs_refs_cond_pair;

            if (!my_ref && !rhs_ref && !my_cond && !rhs_cond)
                continue;
            if ((my_ref && !rhs_ref) || (!my_ref && rhs_ref))
                return false;
            if (*my_ref != *rhs_ref)
                return false;
            if ((my_cond && !rhs_cond) || (!my_cond && rhs_cond))
                return false;
            if (*my_cond != *rhs_cond)
                return false;
        }
    } catch (...) {
        return false;
    }

    return true;
}

float ShipPart::Capacity() const {
    switch (m_class) {
    case ShipPartClass::PC_ARMOUR:
        return m_capacity * (m_add_standard_capacity_effect ? GetGameRules().Get<double>("RULE_SHIP_STRUCTURE_FACTOR") : 1.0f);
        break;
    case ShipPartClass::PC_DIRECT_WEAPON:
    case ShipPartClass::PC_SHIELD:
        return m_capacity * (m_add_standard_capacity_effect ? GetGameRules().Get<double>("RULE_SHIP_WEAPON_DAMAGE_FACTOR") : 1.0f);
        break;
    case ShipPartClass::PC_SPEED:
        return m_capacity * (m_add_standard_capacity_effect ? GetGameRules().Get<double>("RULE_SHIP_SPEED_FACTOR") : 1.0f);
        break;
    default:
        return m_capacity;
    }
}

float ShipPart::SecondaryStat() const {
    switch (m_class) {
    case ShipPartClass::PC_FIGHTER_HANGAR:
        return m_secondary_stat * (m_add_standard_capacity_effect ? GetGameRules().Get<double>("RULE_FIGHTER_DAMAGE_FACTOR") : 1.0f);
        break;
    default:
        return m_secondary_stat;
    }
}

std::string ShipPart::CapacityDescription() const {
    std::string desc_string;
    float main_stat = Capacity();
    float sdry_stat = SecondaryStat();

    switch (m_class) {
    case ShipPartClass::PC_FUEL:
    case ShipPartClass::PC_TROOPS:
    case ShipPartClass::PC_COLONY:
    case ShipPartClass::PC_FIGHTER_BAY:
        desc_string += str(FlexibleFormat(UserString("PART_DESC_CAPACITY")) % main_stat);
        break;
    case ShipPartClass::PC_DIRECT_WEAPON:
        desc_string += str(FlexibleFormat(UserString("PART_DESC_DIRECT_FIRE_STATS")) % main_stat % sdry_stat);
        break;
    case ShipPartClass::PC_FIGHTER_HANGAR:
        desc_string += str(FlexibleFormat(UserString("PART_DESC_HANGAR_STATS")) % main_stat % sdry_stat);
        break;
    case ShipPartClass::PC_SHIELD:
        desc_string = str(FlexibleFormat(UserString("PART_DESC_SHIELD_STRENGTH")) % main_stat);
        break;
    case ShipPartClass::PC_DETECTION:
        desc_string = str(FlexibleFormat(UserString("PART_DESC_DETECTION")) % main_stat);
        break;
    default:
        desc_string = str(FlexibleFormat(UserString("PART_DESC_STRENGTH")) % main_stat);
        break;
    }
    return desc_string;
}

bool ShipPart::CanMountInSlotType(ShipSlotType slot_type) const {
    if (ShipSlotType::INVALID_SHIP_SLOT_TYPE == slot_type)
        return false;
    for (ShipSlotType mountable_slot_type : m_mountable_slot_types)
        if (mountable_slot_type == slot_type)
            return true;
    return false;
}

bool ShipPart::ProductionCostTimeLocationInvariant() const {
    if (GetGameRules().Get<bool>("RULE_CHEAP_AND_FAST_SHIP_PRODUCTION"))
        return true;
    if (m_production_cost && !m_production_cost->TargetInvariant())
        return false;
    if (m_production_time && !m_production_time->TargetInvariant())
        return false;
    return true;
}

float ShipPart::ProductionCost(int empire_id, int location_id, const ScriptingContext& context,
                               int in_design_id) const
{
    if (GetGameRules().Get<bool>("RULE_CHEAP_AND_FAST_SHIP_PRODUCTION") || !m_production_cost)
        return 1.0f;

    static constexpr int PRODUCTION_BLOCK_SIZE = 1;

    if (m_production_cost->ConstantExpr()) {
        return static_cast<float>(m_production_cost->Eval());
    } else if (m_production_cost->SourceInvariant() && m_production_cost->TargetInvariant()) {
        const ScriptingContext design_id_context{
            context, ScriptingContext::Source{}, nullptr, ScriptingContext::Target{}, nullptr,
            in_design_id, PRODUCTION_BLOCK_SIZE};
        return static_cast<float>(m_production_cost->Eval(design_id_context));
    }


    const ObjectMap& objects{context.ContextObjects()};
    auto location = objects.getRaw(location_id);
    if (!location && !m_production_cost->TargetInvariant())
        return ARBITRARY_LARGE_COST;

    auto empire = context.GetEmpire(empire_id);
    auto source = empire ? empire->Source(context.ContextObjects()) : nullptr;
    if (!source && !m_production_cost->SourceInvariant())
        return ARBITRARY_LARGE_COST;


    const ScriptingContext design_id_context{
        context, ScriptingContext::Source{}, source.get(),
        ScriptingContext::Target{}, const_cast<UniverseObject*>(location), // won't be modified when evaluating a ValueRef, but needs to be a pointer to mutable to be passed as the target object
        in_design_id, PRODUCTION_BLOCK_SIZE};

    return static_cast<float>(m_production_cost->Eval(design_id_context));
}

int ShipPart::ProductionTime(int empire_id, int location_id, const ScriptingContext& context,
                             int in_design_id) const
{
    if (GetGameRules().Get<bool>("RULE_CHEAP_AND_FAST_SHIP_PRODUCTION") || !m_production_time)
        return 1;

    static constexpr int PRODUCTION_BLOCK_SIZE = 1;

    if (m_production_time->ConstantExpr()) {
        return m_production_time->Eval();
    } else if (m_production_time->SourceInvariant() && m_production_time->TargetInvariant()) {
        const ScriptingContext design_id_context{
            context, ScriptingContext::Source{}, nullptr, ScriptingContext::Target{}, nullptr,
            in_design_id, PRODUCTION_BLOCK_SIZE};
        return m_production_time->Eval(design_id_context);
    }

    const ObjectMap& objects{context.ContextObjects()};
    auto location = objects.getRaw(location_id);
    if (!location && !m_production_time->TargetInvariant())
        return ARBITRARY_LARGE_TURNS;

    std::shared_ptr<const UniverseObject> source;
    if (auto empire = context.GetEmpire(empire_id))
        source = empire->Source(context.ContextObjects());
    if (!source && !m_production_time->SourceInvariant())
        return ARBITRARY_LARGE_TURNS;

    const ScriptingContext design_id_context{
        context, ScriptingContext::Source{}, source.get(),
        ScriptingContext::Target{}, const_cast<UniverseObject*>(location), // won't be modified when evaluating a ValueRef, but needs to be a pointer to mutable to be passed as the target object
        in_design_id, PRODUCTION_BLOCK_SIZE};
    return m_production_time->Eval(design_id_context);
}

uint32_t ShipPart::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, m_name);
    CheckSums::CheckSumCombine(retval, m_description);
    CheckSums::CheckSumCombine(retval, m_class);
    CheckSums::CheckSumCombine(retval, m_capacity);
    CheckSums::CheckSumCombine(retval, m_secondary_stat);
    CheckSums::CheckSumCombine(retval, m_production_cost);
    CheckSums::CheckSumCombine(retval, m_production_time);
    CheckSums::CheckSumCombine(retval, m_producible);
    CheckSums::CheckSumCombine(retval, m_mountable_slot_types);
    CheckSums::CheckSumCombine(retval, m_tags);
    CheckSums::CheckSumCombine(retval, m_production_meter_consumption);
    CheckSums::CheckSumCombine(retval, m_production_special_consumption);
    CheckSums::CheckSumCombine(retval, m_location);
    CheckSums::CheckSumCombine(retval, m_exclusions);
    CheckSums::CheckSumCombine(retval, m_effects);
    CheckSums::CheckSumCombine(retval, m_icon);
    CheckSums::CheckSumCombine(retval, m_add_standard_capacity_effect),
    CheckSums::CheckSumCombine(retval, m_combat_targets),
    CheckSums::CheckSumCombine(retval, m_total_fighter_damage);
    CheckSums::CheckSumCombine(retval, m_total_ship_damage);

    return retval;
}


ShipPartManager* ShipPartManager::s_instance = nullptr;

ShipPartManager::ShipPartManager() {
    if (s_instance)
        throw std::runtime_error("Attempted to create more than one ShipPartManager.");

    // Only update the global pointer on sucessful construction.
    s_instance = this;
}

const ShipPart* ShipPartManager::GetShipPart(std::string_view name) const {
    CheckPendingShipParts();
    auto it = m_parts.find(name);
    return it != m_parts.end() ? it->second.get() : nullptr;
}

ShipPartManager& ShipPartManager::GetShipPartManager() {
    static ShipPartManager manager;
    return manager;
}

ShipPartManager::iterator ShipPartManager::begin() const {
    CheckPendingShipParts();
    return m_parts.begin();
}

ShipPartManager::iterator ShipPartManager::end() const {
    CheckPendingShipParts();
    return m_parts.end();
}

std::size_t ShipPartManager::size() const {
    CheckPendingShipParts();
    return m_parts.size();
}

uint32_t ShipPartManager::GetCheckSum() const {
    CheckPendingShipParts();
    uint32_t retval{0};
    for (auto const& name_part_pair : m_parts)
        CheckSums::CheckSumCombine(retval, name_part_pair);
    CheckSums::CheckSumCombine(retval, m_parts.size());


    DebugLogger() << "ShipPartManager checksum: " << retval;
    return retval;
}

void ShipPartManager::SetShipParts(Pending::Pending<ShipPartMap>&& pending_ship_parts)
{ m_pending_ship_parts = std::move(pending_ship_parts); }

void ShipPartManager::CheckPendingShipParts() const {
    if (!m_pending_ship_parts)
        return;

    Pending::SwapPending(m_pending_ship_parts, m_parts);

    TraceLogger() << [this]() {
        std::string retval("Part Types:");
        for (const auto& [part_name, part] : m_parts)
            retval.append("\n\t").append(part_name).append(" class: ").append(to_string(part->Class()));
        return retval;
    }();
}

ShipPartManager& GetShipPartManager()
{ return ShipPartManager::GetShipPartManager(); }

const ShipPart* GetShipPart(std::string_view name)
{ return GetShipPartManager().GetShipPart(name); }
