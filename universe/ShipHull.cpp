#include "ShipHull.h"

#include "ConditionSource.h"
#include "Effects.h"
#include "Enums.h"
#include "ValueRefs.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../util/GameRules.h"

#define CHECK_COND_VREF_MEMBER(m_ptr) { if (m_ptr == rhs.m_ptr) {           \
                                            /* check next member */         \
                                        } else if (!m_ptr || !rhs.m_ptr) {  \
                                            return false;                   \
                                        } else {                            \
                                            if (*m_ptr != *(rhs.m_ptr))     \
                                                return false;               \
                                        }   }

namespace {
    void AddRules(GameRules& rules) {
        rules.Add<double>(UserStringNop("RULE_SHIP_SPEED_FACTOR"),
                          UserStringNop("RULE_SHIP_SPEED_FACTOR_DESC"),
                          UserStringNop("BALANCE"), 1.0, true, RangedValidator<double>(0.1, 10.0));
        rules.Add<double>(UserStringNop("RULE_SHIP_STRUCTURE_FACTOR"),
                          UserStringNop("RULE_SHIP_STRUCTURE_FACTOR_DESC"),
                          UserStringNop("BALANCE"), 8.0, true, RangedValidator<double>(0.1, 80.0));
        rules.Add<double>(UserStringNop("RULE_SHIP_WEAPON_DAMAGE_FACTOR"),
                          UserStringNop("RULE_SHIP_WEAPON_DAMAGE_FACTOR_DESC"),
                          UserStringNop("BALANCE"), 6.0, true, RangedValidator<double>(0.1, 60.0));
        rules.Add<double>(UserStringNop("RULE_FIGHTER_DAMAGE_FACTOR"),
                          UserStringNop("RULE_FIGHTER_DAMAGE_FACTOR_DESC"),
                          UserStringNop("BALANCE"), 6.0, true, RangedValidator<double>(0.1, 60.0));
    }
    bool temp_bool = RegisterGameRules(&AddRules);

    constexpr float ARBITRARY_LARGE_COST = 999999.9f;
    constexpr int ARBITRARY_LARGE_TURNS = 999999;

    // create effectsgroup that increases the value of \a meter_type
    // by the result of evalulating \a increase_vr
    std::shared_ptr<Effect::EffectsGroup>
    IncreaseMeter(MeterType meter_type,
                  std::unique_ptr<ValueRef::ValueRef<double>>&& increase_vr)
    {
        auto scope = std::make_unique<Condition::Source>();
        auto activation = std::make_unique<Condition::Source>();

        auto vr =
            std::make_unique<ValueRef::Operation<double>>(
                ValueRef::OpType::PLUS,
                std::make_unique<ValueRef::Variable<double>>(ValueRef::ReferenceType::EFFECT_TARGET_VALUE_REFERENCE),
                std::move(increase_vr)
            );
        std::vector<std::unique_ptr<Effect::Effect>> effects;
        effects.push_back(std::make_unique<Effect::SetMeter>(meter_type, std::move(vr)));

        return std::make_shared<Effect::EffectsGroup>(std::move(scope), std::move(activation),
                                                      std::move(effects));
    }

    // create effectsgroup that increases the value of \a meter_type
    // by the specified amount \a fixed_increase
    std::shared_ptr<Effect::EffectsGroup>
    IncreaseMeter(MeterType meter_type, float fixed_increase) {
        auto increase_vr = std::make_unique<ValueRef::Constant<double>>(fixed_increase);
        return IncreaseMeter(meter_type, std::move(increase_vr));
    }

    // create effectsgroup that increases the value of \a meter_type
    // by the product of \a base_increase and the value of the game
    // rule of type double with the name \a scaling_factor_rule_name
    std::shared_ptr<Effect::EffectsGroup>
    IncreaseMeter(MeterType meter_type, float base_increase,
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

}


ShipHull::ShipHull(float fuel, float speed, float stealth, float structure,
                   bool default_fuel_effects, bool default_speed_effects,
                   bool default_stealth_effects, bool default_structure_effects,
                   CommonParams&& common_params,
                   std::string&& name, std::string&& description,
                   std::set<std::string>&& exclusions, std::vector<Slot>&& slots,
                   std::string&& icon, std::string&& graphic) :
    m_name(std::move(name)),
    m_description(std::move(description)),
    m_speed(speed),
    m_fuel(fuel),
    m_stealth(stealth),
    m_structure(structure),
    m_production_cost(std::move(common_params.production_cost)),
    m_production_time(std::move(common_params.production_time)),
    m_producible(common_params.producible),
    m_slots(std::move(slots)),
    m_tags_concatenated([&common_params]() {
        // ensure tags are all upper-case
        std::for_each(common_params.tags.begin(), common_params.tags.end(),
                      [](auto& t) { boost::to_upper<std::string>(t); });

        // allocate storage for concatenated tags
        std::string retval;
        // TODO: transform_reduce when available on all platforms...
        std::size_t params_sz = 0;
        for (const auto& t : common_params.tags)
            params_sz += t.size();
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
            retval.push_back(sv.substr(next_idx, t.size()));
            next_idx += t.size();
        });
        return retval;
    }()),
    m_production_meter_consumption(std::move(common_params.production_meter_consumption)),
    m_production_special_consumption(std::move(common_params.production_special_consumption)),
    m_location(std::move(common_params.location)),
    m_exclusions(std::move(exclusions)),
    m_graphic(std::move(graphic)),
    m_icon(std::move(icon))
{
    TraceLogger() << "hull type: " << m_name << " producible: " << m_producible << "\n";
    Init(std::move(common_params.effects),
         default_fuel_effects,
         default_speed_effects,
         default_stealth_effects,
         default_structure_effects);
}

ShipHull::~ShipHull() = default;

bool ShipHull::operator==(const ShipHull& rhs) const {
    if (&rhs == this)
        return true;

    if (m_name != rhs.m_name ||
        m_description != rhs.m_description ||
        m_speed != rhs.m_speed ||
        m_fuel != rhs.m_fuel ||
        m_stealth != rhs.m_stealth ||
        m_structure != rhs.m_structure ||
        m_producible != rhs.m_producible ||
        m_slots != rhs.m_slots ||
        m_tags != rhs.m_tags ||
        m_exclusions != rhs.m_exclusions ||
        m_graphic != rhs.m_graphic ||
        m_icon != rhs.m_icon)
    { return false; }

    CHECK_COND_VREF_MEMBER(m_production_cost)
    CHECK_COND_VREF_MEMBER(m_production_time)
    CHECK_COND_VREF_MEMBER(m_location)

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

void ShipHull::Init(std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects,
                    bool default_fuel_effects,
                    bool default_speed_effects,
                    bool default_stealth_effects,
                    bool default_structure_effects)
{
    if (default_fuel_effects && m_fuel != 0)
        m_effects.push_back(IncreaseMeter(MeterType::METER_MAX_FUEL,      m_fuel));
    if (default_stealth_effects && m_stealth != 0)
        m_effects.push_back(IncreaseMeter(MeterType::METER_STEALTH,       m_stealth));
    if (default_structure_effects && m_structure != 0)
        m_effects.push_back(IncreaseMeter(MeterType::METER_MAX_STRUCTURE, m_structure, "RULE_SHIP_STRUCTURE_FACTOR"));
    if (default_speed_effects && m_speed != 0)
        m_effects.push_back(IncreaseMeter(MeterType::METER_SPEED,         m_speed,     "RULE_SHIP_SPEED_FACTOR"));

    if (m_production_cost)
        m_production_cost->SetTopLevelContent(m_name);
    if (m_production_time)
        m_production_time->SetTopLevelContent(m_name);
    if (m_location)
        m_location->SetTopLevelContent(m_name);
    for (auto&& effect : effects) {
        effect->SetTopLevelContent(m_name);
        m_effects.push_back(std::move(effect));
    }
}

float ShipHull::Speed() const
{ return m_speed * GetGameRules().Get<double>("RULE_SHIP_SPEED_FACTOR"); }

float ShipHull::Structure() const
{ return m_structure * GetGameRules().Get<double>("RULE_SHIP_STRUCTURE_FACTOR"); }

uint32_t ShipHull::NumSlots(ShipSlotType slot_type) const noexcept {
    uint32_t count = 0;
    for (const Slot& slot : m_slots)
        if (slot.type == slot_type)
            ++count;
    return count;
}

// ShipHull:: and ShipPart::ProductionCost and ProductionTime are almost identical.
// Chances are, the same is true of buildings and techs as well.
// TODO: Eliminate duplication
bool ShipHull::ProductionCostTimeLocationInvariant() const {
    if (GetGameRules().Get<bool>("RULE_CHEAP_AND_FAST_SHIP_PRODUCTION"))
        return true;
    if (m_production_cost && !m_production_cost->LocalCandidateInvariant())
        return false;
    if (m_production_time && !m_production_time->LocalCandidateInvariant())
        return false;
    return true;
}

float ShipHull::ProductionCost(int empire_id, int location_id,
                               const ScriptingContext& parent_context, int in_design_id) const
{
    if (GetGameRules().Get<bool>("RULE_CHEAP_AND_FAST_SHIP_PRODUCTION") || !m_production_cost)
        return 1.0f;

    if (m_production_cost->ConstantExpr())
        return static_cast<float>(m_production_cost->Eval());

    static constexpr int PRODUCTION_BLOCK_SIZE = 1;

    if (m_production_cost->SourceInvariant() && m_production_cost->TargetInvariant()) {
        const ScriptingContext design_id_context{
            parent_context, nullptr, nullptr, in_design_id, PRODUCTION_BLOCK_SIZE};
        return static_cast<float>(m_production_cost->Eval(design_id_context));
    }

    auto location = parent_context.ContextObjects().getRaw(location_id);
    if (!location && !m_production_cost->TargetInvariant())
        return ARBITRARY_LARGE_COST;

    auto empire = parent_context.GetEmpire(empire_id);
    auto source = empire ? empire->Source(parent_context.ContextObjects()) : nullptr;
    if (!source && !m_production_cost->SourceInvariant())
        return ARBITRARY_LARGE_COST;

    const ScriptingContext design_id_context{
        parent_context, source.get(),
        const_cast<UniverseObject*>(location), // won't be modified when evaluating a ValueRef, but needs to be a pointer to mutable to be passed as the target object
        in_design_id, PRODUCTION_BLOCK_SIZE};
    return static_cast<float>(m_production_cost->Eval(design_id_context));
}

int ShipHull::ProductionTime(int empire_id, int location_id,
                             const ScriptingContext& parent_context, int in_design_id) const
{
    if (GetGameRules().Get<bool>("RULE_CHEAP_AND_FAST_SHIP_PRODUCTION") || !m_production_time)
        return 1;

    if (m_production_time->ConstantExpr())
        return m_production_time->Eval();

    static constexpr int PRODUCTION_BLOCK_SIZE = 1;

    if (m_production_time->SourceInvariant() && m_production_time->TargetInvariant()) {
        const ScriptingContext design_id_context{
            parent_context, nullptr, nullptr, in_design_id, PRODUCTION_BLOCK_SIZE};
        return m_production_time->Eval(design_id_context);
    }

    auto location = parent_context.ContextObjects().getRaw(location_id);
    if (!location && !m_production_time->TargetInvariant())
        return ARBITRARY_LARGE_TURNS;

    auto empire = parent_context.GetEmpire(empire_id);
    auto source = empire ? empire->Source(parent_context.ContextObjects()) : nullptr;
    if (!source && !m_production_time->SourceInvariant())
        return ARBITRARY_LARGE_TURNS;

    const ScriptingContext design_id_context{
        parent_context, source.get(),
        const_cast<UniverseObject*>(location), // won't be modified when evaluating a ValueRef, but needs to be a pointer to mutable to be passed as the target object
        in_design_id, PRODUCTION_BLOCK_SIZE};
    return m_production_time->Eval(design_id_context);
}

uint32_t ShipHull::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, m_name);
    CheckSums::CheckSumCombine(retval, m_description);
    CheckSums::CheckSumCombine(retval, m_speed);
    CheckSums::CheckSumCombine(retval, m_fuel);
    CheckSums::CheckSumCombine(retval, m_stealth);
    CheckSums::CheckSumCombine(retval, m_structure);
    CheckSums::CheckSumCombine(retval, m_production_cost);
    CheckSums::CheckSumCombine(retval, m_production_time);
    CheckSums::CheckSumCombine(retval, m_producible);
    CheckSums::CheckSumCombine(retval, m_slots);
    CheckSums::CheckSumCombine(retval, m_tags);
    CheckSums::CheckSumCombine(retval, m_production_meter_consumption);
    CheckSums::CheckSumCombine(retval, m_production_special_consumption);
    CheckSums::CheckSumCombine(retval, m_location);
    CheckSums::CheckSumCombine(retval, m_exclusions);
    CheckSums::CheckSumCombine(retval, m_effects);
    CheckSums::CheckSumCombine(retval, m_graphic);
    CheckSums::CheckSumCombine(retval, m_icon);

    return retval;
}


ShipHullManager* ShipHullManager::s_instance = nullptr;

ShipHullManager::ShipHullManager() {
    if (s_instance)
        throw std::runtime_error("Attempted to create more than one ShipHullManager.");

    // Only update the global pointer on sucessful construction.
    s_instance = this;
}

const ShipHull* ShipHullManager::GetShipHull(std::string_view name) const {
    CheckPendingShipHulls();
    auto it = m_hulls.find(name);
    return it != m_hulls.end() ? it->second.get() : nullptr;
}

ShipHullManager& ShipHullManager::GetShipHullManager() {
    static ShipHullManager manager;
    return manager;
}

ShipHullManager::iterator ShipHullManager::begin() const {
    CheckPendingShipHulls();
    return m_hulls.begin();
}

ShipHullManager::iterator ShipHullManager::end() const {
    CheckPendingShipHulls();
    return m_hulls.end();
}

std::size_t ShipHullManager::size() const {
    CheckPendingShipHulls();
    return m_hulls.size();
}

uint32_t ShipHullManager::GetCheckSum() const {
    CheckPendingShipHulls();
    uint32_t retval{0};
    for (auto const& name_hull_pair : m_hulls)
        CheckSums::CheckSumCombine(retval, name_hull_pair);
    CheckSums::CheckSumCombine(retval, m_hulls.size());

    DebugLogger() << "ShipHullManager checksum: " << retval;
    return retval;
}

void ShipHullManager::SetShipHulls(Pending::Pending<container_type>&& pending_ship_hulls)
{ m_pending_ship_hulls = std::move(pending_ship_hulls); }

void ShipHullManager::CheckPendingShipHulls() const {
    if (!m_pending_ship_hulls)
        return;

    Pending::SwapPending(m_pending_ship_hulls, m_hulls);

    TraceLogger() << [this]() {
        std::string retval("Hull Types:");
        for (const auto& entry : m_hulls)
            retval.append("\n\t" + entry.second->Name());
        return retval;
    }();

    if (m_hulls.empty())
        ErrorLogger() << "ShipHullManager expects at least one hull type.  All ship design construction will fail.";
}


namespace CheckSums {
    void CheckSumCombine(uint32_t& sum, const ShipHull::Slot& slot) {
        TraceLogger() << "CheckSumCombine(Slot): " << typeid(slot).name();
        CheckSumCombine(sum, slot.x);
        CheckSumCombine(sum, slot.y);
        CheckSumCombine(sum, slot.type);
    }
}


ShipHullManager& GetShipHullManager()
{ return ShipHullManager::GetShipHullManager(); }

const ShipHull* GetShipHull(std::string_view name)
{ return GetShipHullManager().GetShipHull(std::string{name}); }

