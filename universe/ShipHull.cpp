#include "ShipHull.h"


#include "ConditionSource.h"
#include "Effects.h"
#include "Enums.h"
#include "ValueRefs.h"
#include "../Empire/EmpireManager.h"
#include "../util/GameRules.h"


namespace {
    void AddRules(GameRules& rules) {
        rules.Add<double>("RULE_SHIP_SPEED_FACTOR", "RULE_SHIP_SPEED_FACTOR_DESC",
                          "BALANCE", 1.0, true, RangedValidator<double>(0.1, 10.0));
        rules.Add<double>("RULE_SHIP_STRUCTURE_FACTOR", "RULE_SHIP_STRUCTURE_FACTOR_DESC",
                          "BALANCE", 1.0, true, RangedValidator<double>(0.1, 80.0));
        rules.Add<double>("RULE_SHIP_WEAPON_DAMAGE_FACTOR", "RULE_SHIP_WEAPON_DAMAGE_FACTOR_DESC",
                          "BALANCE", 1.0, true, RangedValidator<double>(0.1, 60.0));
        rules.Add<double>("RULE_FIGHTER_DAMAGE_FACTOR", "RULE_FIGHTER_DAMAGE_FACTOR_DESC",
                          "BALANCE", 1.0, true, RangedValidator<double>(0.1, 60.0));
    }
    bool temp_bool = RegisterGameRules(&AddRules);

    const float ARBITRARY_LARGE_COST = 999999.9f;
    const int ARBITRARY_LARGE_TURNS = 999999;

    // create effectsgroup that increases the value of \a meter_type
    // by the result of evalulating \a increase_vr
    std::shared_ptr<Effect::EffectsGroup>
    IncreaseMeter(MeterType meter_type,
                  std::unique_ptr<ValueRef::ValueRef<double>>&& increase_vr)
    {
        typedef std::vector<std::unique_ptr<Effect::Effect>> Effects;
        auto scope = std::make_unique<Condition::Source>();
        auto activation = std::make_unique<Condition::Source>();

        auto vr =
            std::make_unique<ValueRef::Operation<double>>(
                ValueRef::PLUS,
                std::make_unique<ValueRef::Variable<double>>(
                    ValueRef::EFFECT_TARGET_VALUE_REFERENCE, std::vector<std::string>()),
                std::move(increase_vr)
            );
        auto effects = Effects();
        effects.push_back(std::make_unique<Effect::SetMeter>(meter_type, std::move(vr)));
        return std::make_shared<Effect::EffectsGroup>(std::move(scope), std::move(activation), std::move(effects));
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
            ValueRef::TIMES,
            std::make_unique<ValueRef::Constant<double>>(base_increase),
            std::make_unique<ValueRef::ComplexVariable<double>>(
                "GameRule", nullptr, nullptr, nullptr,
                std::make_unique<ValueRef::Constant<std::string>>(scaling_factor_rule_name)
            )
        );

        return IncreaseMeter(meter_type, std::move(increase_vr));
    }

}


ShipHull::ShipHull()
{}

ShipHull::ShipHull(const ShipHullStats& stats,
                   CommonParams&& common_params,
                   const MoreCommonParams& more_common_params,
                   const std::vector<Slot>& slots,
                   const std::string& icon, const std::string& graphic) :
    m_name(more_common_params.name),
    m_description(more_common_params.description),
    m_speed(stats.speed),
    m_fuel(stats.fuel),
    m_stealth(stats.stealth),
    m_structure(stats.structure),
    m_production_cost(std::move(common_params.production_cost)),
    m_production_time(std::move(common_params.production_time)),
    m_producible(common_params.producible),
    m_slots(slots),
    m_production_meter_consumption(std::move(common_params.production_meter_consumption)),
    m_production_special_consumption(std::move(common_params.production_special_consumption)),
    m_location(std::move(common_params.location)),
    m_exclusions(more_common_params.exclusions),
    m_graphic(graphic),
    m_icon(icon)
{
    TraceLogger() << "hull type: " << m_name << " producible: " << m_producible << std::endl;
    Init(std::move(common_params.effects), stats);

    for (const std::string& tag : common_params.tags)
        m_tags.insert(boost::to_upper_copy<std::string>(tag));
}

ShipHull::Slot::Slot() :
    type(INVALID_SHIP_SLOT_TYPE)
{}

ShipHull::~ShipHull() {}

void ShipHull::Init(std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects,
                    const ShipHullStats& stats)
{
    if (stats.default_fuel_effects && m_fuel != 0)
        m_effects.push_back(IncreaseMeter(METER_MAX_FUEL,       m_fuel));
    if (stats.default_stealth_effects && m_stealth != 0)
        m_effects.push_back(IncreaseMeter(METER_STEALTH,        m_stealth));
    if (stats.default_structure_effects && m_structure != 0)
        m_effects.push_back(IncreaseMeter(METER_MAX_STRUCTURE,  m_structure,    "RULE_SHIP_STRUCTURE_FACTOR"));
    if (stats.default_speed_effects && m_speed != 0)
        m_effects.push_back(IncreaseMeter(METER_SPEED,          m_speed,        "RULE_SHIP_SPEED_FACTOR"));

    if (m_production_cost)
        m_production_cost->SetTopLevelContent(m_name);
    if (m_production_time)
        m_production_time->SetTopLevelContent(m_name);
    if (m_location)
        m_location->SetTopLevelContent(m_name);
    for (auto&& effect : effects) {
        effect->SetTopLevelContent(m_name);
        m_effects.emplace_back(std::move(effect));
    }
}

float ShipHull::Speed() const
{ return m_speed * GetGameRules().Get<double>("RULE_SHIP_SPEED_FACTOR"); }

float ShipHull::Structure() const
{ return m_structure * GetGameRules().Get<double>("RULE_SHIP_STRUCTURE_FACTOR"); }

unsigned int ShipHull::NumSlots(ShipSlotType slot_type) const {
    unsigned int count = 0;
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

float ShipHull::ProductionCost(int empire_id, int location_id, int in_design_id) const {
    if (GetGameRules().Get<bool>("RULE_CHEAP_AND_FAST_SHIP_PRODUCTION") || !m_production_cost)
        return 1.0f;

    if (m_production_cost->ConstantExpr()) {
        return static_cast<float>(m_production_cost->Eval());
    } else if (m_production_cost->SourceInvariant() && m_production_cost->TargetInvariant()) {
        ScriptingContext context(nullptr, nullptr, in_design_id);
        return static_cast<float>(m_production_cost->Eval(context));
    }

    auto location = Objects().get(location_id);
    if (!location && !m_production_cost->TargetInvariant())
        return ARBITRARY_LARGE_COST;

    auto source = Empires().GetSource(empire_id);
    if (!source && !m_production_cost->SourceInvariant())
        return ARBITRARY_LARGE_COST;

    ScriptingContext context(source, location, in_design_id);
    return static_cast<float>(m_production_cost->Eval(context));
}

int ShipHull::ProductionTime(int empire_id, int location_id, int in_design_id) const {
    if (GetGameRules().Get<bool>("RULE_CHEAP_AND_FAST_SHIP_PRODUCTION") || !m_production_time)
        return 1;

    if (m_production_time->ConstantExpr()) {
        return m_production_time->Eval();
    } else if (m_production_time->SourceInvariant() && m_production_time->TargetInvariant()) {
        ScriptingContext context(nullptr, nullptr, in_design_id);
        return m_production_time->Eval(context);
    }

    auto location = Objects().get(location_id);
    if (!location && !m_production_time->TargetInvariant())
        return ARBITRARY_LARGE_TURNS;

    auto source = Empires().GetSource(empire_id);
    if (!source && !m_production_time->SourceInvariant())
        return ARBITRARY_LARGE_TURNS;

    ScriptingContext context(source, location, in_design_id);
    return m_production_time->Eval(context);
}

unsigned int ShipHull::GetCheckSum() const {
    unsigned int retval{0};

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

const ShipHull* ShipHullManager::GetShipHull(const std::string& name) const {
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

unsigned int ShipHullManager::GetCheckSum() const {
    CheckPendingShipHulls();
    unsigned int retval{0};
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
        for (const auto& entry : m_hulls) {
            retval.append("\n\t" + entry.second->Name());
        }
        return retval;
    }();

    if (m_hulls.empty())
        ErrorLogger() << "ShipHullManager expects at least one hull type.  All ship design construction will fail.";
}


namespace CheckSums {
    void CheckSumCombine(unsigned int& sum, const ShipHull::Slot& slot) {
        TraceLogger() << "CheckSumCombine(Slot): " << typeid(slot).name();
        CheckSumCombine(sum, slot.x);
        CheckSumCombine(sum, slot.y);
        CheckSumCombine(sum, slot.type);
    }
}


ShipHullManager& GetShipHullManager()
{ return ShipHullManager::GetShipHullManager(); }

const ShipHull* GetShipHull(const std::string& name)
{ return GetShipHullManager().GetShipHull(name); }
