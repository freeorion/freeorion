#include "Effects.h"

#include <cctype>
#include <iterator>
#include <boost/filesystem/fstream.hpp>
#include "BuildingType.h"
#include "Building.h"
#include "Condition.h"
#include "FieldType.h"
#include "Field.h"
#include "Fleet.h"
#include "Pathfinder.h"
#include "Planet.h"
#include "ShipDesign.h"
#include "Ship.h"
#include "Species.h"
#include "System.h"
#include "Tech.h"
#include "UniverseObject.h"
#include "Universe.h"
#include "ValueRefs.h"
#include "../Empire/Empire.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/Random.h"
#include "../util/SitRepEntry.h"
#include "../util/i18n.h"
#include "../util/ranges.h"

namespace {
    DeclareThreadSafeLogger(effects);
}

using boost::io::str;

#define CHECK_COND_VREF_MEMBER(m_ptr) { if (m_ptr == rhs_.m_ptr) {              \
                                            /* check next member */             \
                                        } else if (!m_ptr || !rhs_.m_ptr) {     \
                                            return false;                       \
                                        } else {                                \
                                            if (*m_ptr != *(rhs_.m_ptr))        \
                                                return false;                   \
                                        }   }


namespace {
    CONSTEXPR_VEC_AND_STRING const std::string EMPTY_STRING;
    CONSTEXPR_VEC_AND_STRING const ScriptingContext::CurrentValueVariant
        EMPTY_STRING_CURRENT_VALUE{EMPTY_STRING};
    CONSTEXPR_VEC_AND_STRING const ScriptingContext::CurrentValueVariant&
        ZERO_INT_CURRENT_VALUE{ScriptingContext::DEFAULT_CURRENT_VALUE};

    /** creates a new fleet at a specified \a x and \a y location within the
     * Universe, and and inserts \a ship into it.  Used when a ship has been
     * moved by the MoveTo effect separately from the fleet that previously
     * held it.  All ships need to be within fleets. */
    std::shared_ptr<Fleet> CreateNewFleet(
        double x, double y, Ship* ship, ScriptingContext& context,
        FleetAggression aggression = FleetAggression::INVALID_FLEET_AGGRESSION)
    {
        if (!ship)
            return nullptr;

        Universe& universe = context.ContextUniverse();
        auto fleet = universe.InsertNew<Fleet>("", x, y, ship->Owner(), context.current_turn);

        fleet->Rename(fleet->GenerateFleetName(context));
        fleet->GetMeter(MeterType::METER_STEALTH)->SetCurrent(Meter::LARGE_VALUE);

        fleet->AddShips({ship->ID()});
        ship->SetFleetID(fleet->ID());

        // if aggression specified, use that, otherwise get from whether ship is armed
        FleetAggression new_aggr = aggression == FleetAggression::INVALID_FLEET_AGGRESSION ?
            (ship->IsArmed(context) ? FleetDefaults::FLEET_DEFAULT_ARMED : FleetDefaults::FLEET_DEFAULT_UNARMED) :
            (aggression);
        fleet->SetAggression(new_aggr);

        return fleet;
    }

    /** Creates a new fleet at \a system and inserts \a ship into it.  Used
     * when a ship has been moved by the MoveTo effect separately from the
     * fleet that previously held it.  Also used by CreateShip effect to give
     * the new ship a fleet.  All ships need to be within fleets. */
    std::shared_ptr<Fleet> CreateNewFleet(
        System* system, Ship* ship, ScriptingContext& context,
        FleetAggression aggression = FleetAggression::INVALID_FLEET_AGGRESSION)
    {
        if (!system || !ship)
            return nullptr;

        // remove ship from old fleet / system, put into new system if necessary
        if (ship->SystemID() != system->ID()) {
            if (auto old_system = context.ContextObjects().getRaw<System>(ship->SystemID())) {
                old_system->Remove(ship->ID());
                ship->SetSystem(INVALID_OBJECT_ID);
            }
            system->Insert(ship, System::NO_ORBIT, context.current_turn, context.ContextObjects());
        }

        if (ship->FleetID() != INVALID_OBJECT_ID) {
            if (auto old_fleet = context.ContextObjects().getRaw<Fleet>(ship->FleetID()))
                old_fleet->RemoveShips({ship->ID()});
        }

        // create new fleet for ship, and put it in new system
        auto fleet = CreateNewFleet(system->X(), system->Y(), std::move(ship), context, aggression);
        system->Insert(fleet, System::NO_ORBIT, context.current_turn, context.ContextObjects());

        return fleet;
    }

    /** Explores the system with the specified \a system_id for the owner of
      * the specified \a target_object.  Used when moving objects into a system
      * with the MoveTo effect, as otherwise the system wouldn't get explored,
      * and objects being moved into unexplored systems might disappear for
      * players or confuse the AI. */
    void ExploreSystem(int system_id, int empire_id, ScriptingContext& context) {
        if (empire_id == ALL_EMPIRES || system_id == INVALID_OBJECT_ID)
            return;
        if (auto empire = context.GetEmpire(empire_id))
            empire->AddExploredSystem(system_id, context.current_turn, context.ContextObjects());
    }

    /** Resets the previous and next systems of \a fleet and recalcultes /
     * resets the fleet's move route.  Used after a fleet has been moved with
     * the MoveTo effect, as its previous route was assigned based on its
     * previous location, and may not be valid for its new location. */
    void UpdateFleetRoute(Fleet* fleet, int new_next_system,
                          int new_previous_system, const ScriptingContext& context)
    {
        if (!fleet) {
            ErrorLogger(effects) << "UpdateFleetRoute passed a null fleet pointer";
            return;
        }

        const ObjectMap& objects = context.ContextObjects();

        const auto next_system = objects.getRaw<System>(new_next_system);
        if (!next_system) {
            ErrorLogger(effects) << "UpdateFleetRoute couldn't get new next system with id: " << new_next_system;
            return;
        }

        if (new_previous_system != INVALID_OBJECT_ID && !objects.getRaw<System>(new_previous_system))
            ErrorLogger(effects) << "UpdateFleetRoute couldn't get new previous system with id: " << new_previous_system;

        fleet->SetNextAndPreviousSystems(new_next_system, new_previous_system);


        // recalculate route from the shortest path between first system on path and final destination
        int start_system = fleet->SystemID();
        if (start_system == INVALID_OBJECT_ID)
            start_system = new_next_system;

        const int dest_system = fleet->FinalDestinationID();

        auto route = context.ContextUniverse().GetPathfinder().ShortestPath(
            start_system, dest_system, fleet->Owner(), objects).first;

        // if shortest path is empty, the route may be impossible or trivial, so just set route to move fleet
        // to the next system that it was just set to move to anyway.
        if (route.empty())
            route.push_back(new_next_system);


        // set fleet with newly recalculated route
        try {
            fleet->SetRoute(std::move(route), objects);
        } catch (const std::exception& e) {
            ErrorLogger(effects) << "Caught exception updating fleet route in effect code: " << e.what();
        }
    }

    std::string GenerateSystemName(const ObjectMap& objects) {
        static const std::vector<std::string> star_names = UserStringList("STAR_NAMES");

        // pick a name for the system
        for (const std::string& star_name : star_names) {
            // does an existing system have this name?
            bool dupe = false;
            for (auto* system : objects.allRaw<System>()) {
                if (system->Name() == star_name) {
                    dupe = true;
                    break;  // another systme has this name. skip to next potential name.
                }
            }
            if (!dupe)
                return star_name; // no systems have this name yet. use it.
        }
        // generate hopefully unique name?
        return UserString("SYSTEM") + " " + std::to_string(RandInt(objects.size<System>(), objects.size<System>() + 10000));
    }
}

namespace Effect {
///////////////////////////////////////////////////////////
// EffectsGroup                                          //
///////////////////////////////////////////////////////////
EffectsGroup::EffectsGroup(std::unique_ptr<Condition::Condition>&& scope,
                           std::unique_ptr<Condition::Condition>&& activation,
                           std::vector<std::unique_ptr<Effect>>&& effects,
                           std::string accounting_label,
                           std::string stacking_group, int priority,
                           std::string description,
                           std::string content_name):
    m_scope(std::move(scope)),
    m_activation(std::move(activation)),
    m_stacking_group(std::move(stacking_group)),
    m_effects(std::move(effects)),
    m_accounting_label(std::move(accounting_label)),
    m_priority(priority),
    m_description(std::move(description)),
    m_content_name(std::move(content_name)),
    m_has_meter_effects([this]() {
        return std::any_of(m_effects.begin(), m_effects.end(),
                           [](const auto& e) noexcept { return e->IsMeterEffect(); });
    }()),
    m_has_appearance_effects([this]() {
        return std::any_of(m_effects.begin(), m_effects.end(),
                           [](const auto& e) noexcept { return e->IsAppearanceEffect(); });
    }()),
    m_has_sitrep_effects([this]() {
        return std::any_of(m_effects.begin(), m_effects.end(),
                           [](const auto& e) noexcept { return e->IsSitrepEffect(); });
    }())
{}

EffectsGroup::~EffectsGroup() = default;

bool EffectsGroup::operator==(const EffectsGroup& rhs) const {
    if (&rhs == this)
        return true;

    if (m_stacking_group != rhs.m_stacking_group ||
        m_description != rhs.m_description ||
        m_accounting_label != rhs.m_accounting_label ||
        m_description != rhs.m_description ||
        m_content_name != rhs.m_content_name ||
        m_priority != rhs.m_priority)
    { return false; }

    if (m_scope == rhs.m_scope) { // could be nullptr
        // check next member
    } else if (!m_scope || !rhs.m_scope) {
        return false;
    } else {
        if (*m_scope != *(rhs.m_scope))
            return false;
    }

    if (m_activation == rhs.m_activation) { // could be nullptr
        // check next member
    } else if (!m_activation || !rhs.m_activation) {
        return false;
    } else {
        if (*m_activation != *(rhs.m_activation))
            return false;
    }

    if (m_effects.size() != rhs.m_effects.size())
        return false;
    try {
        for (std::size_t idx = 0; idx < m_effects.size(); ++idx) {
            const auto& my_op = m_effects.at(idx);
            const auto& rhs_op = rhs.m_effects.at(idx);

            if (my_op == rhs_op)
                continue;
            if (!my_op || !rhs_op || *my_op != *rhs_op)
                return false;
        }
    } catch (...) {
        return false;
    }

    return true;
}

void EffectsGroup::Execute(ScriptingContext& context,
                           const TargetsAndCause& targets_cause,
                           AccountingMap* accounting_map,
                           bool only_meter_effects, bool only_appearance_effects,
                           bool include_empire_meter_effects,
                           bool only_generate_sitrep_effects) const
{
    if (!context.source)    // todo: move into loop and check only when an effect is source-variant
        WarnLogger(effects) << "EffectsGroup being executed without a defined source object";

    // execute each effect of the group one by one, unless filtered by flags
    for (auto& effect : m_effects) {
        // skip excluded effect types
        if (   (only_appearance_effects       && !effect->IsAppearanceEffect())
            || (only_meter_effects            && !effect->IsMeterEffect())
            || (!include_empire_meter_effects &&  effect->IsEmpireMeterEffect())
            || (only_generate_sitrep_effects  && !effect->IsSitrepEffect()))
        { continue; }

        effect->Execute(context, targets_cause.target_set, accounting_map,
                        targets_cause.effect_cause,
                        only_meter_effects, only_appearance_effects,
                        include_empire_meter_effects, only_generate_sitrep_effects);
    }
}

std::string EffectsGroup::Dump(uint8_t ntabs) const {
    std::string retval = DumpIndent(ntabs) + "EffectsGroup";
    if (!m_content_name.empty())
        retval += " // from " + m_content_name;
    retval += "\n";
    if (!m_description.empty())
        retval += DumpIndent(ntabs+1) + "description = \"" + m_description + "\"\n";
    retval += DumpIndent(ntabs+1) + "scope =\n";
    retval += m_scope->Dump(ntabs+2);
    if (m_activation) {
        retval += DumpIndent(ntabs+1) + "activation =\n";
        retval += m_activation->Dump(ntabs+2);
    }
    if (!m_accounting_label.empty())
        retval += DumpIndent(ntabs+1) + "accountinglabel = \"" + m_accounting_label + "\"\n";
    if (!m_stacking_group.empty())
        retval += DumpIndent(ntabs+1) + "stackinggroup = \"" + m_stacking_group + "\"\n";
    retval += DumpIndent(ntabs+1) + "priority = " + std::to_string(m_priority) + "\n";
    if (m_effects.size() == 1) {
        retval += DumpIndent(ntabs+1) + "effects =\n";
        retval += m_effects[0]->Dump(ntabs+2);
    } else {
        retval += DumpIndent(ntabs+1) + "effects = [\n";
        for (auto& effect : m_effects) {
            retval += effect->Dump(ntabs+2);
        }
        retval += DumpIndent(ntabs+1) + "]\n";
    }
    return retval;
}

void EffectsGroup::SetTopLevelContent(std::string content_name) {
    if (m_scope)
        m_scope->SetTopLevelContent(content_name);
    if (m_activation)
        m_activation->SetTopLevelContent(content_name);
    for (auto& effect : m_effects)
        effect->SetTopLevelContent(content_name);
    m_content_name = std::move(content_name);
}

uint32_t EffectsGroup::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "EffectsGroup");
    CheckSums::CheckSumCombine(retval, m_scope);
    CheckSums::CheckSumCombine(retval, m_activation);
    CheckSums::CheckSumCombine(retval, m_stacking_group);
    CheckSums::CheckSumCombine(retval, m_effects);
    CheckSums::CheckSumCombine(retval, m_accounting_label);
    CheckSums::CheckSumCombine(retval, m_priority);
    CheckSums::CheckSumCombine(retval, m_description);

    TraceLogger(effects) << "GetCheckSum(EffectsGroup): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// Dump function                                         //
///////////////////////////////////////////////////////////
std::string Dump(const std::vector<EffectsGroup>& effects_groups) {
    std::stringstream retval;

    for (auto& effects_group : effects_groups)
        retval << "\n" << effects_group.Dump();

    return retval.str();
}



///////////////////////////////////////////////////////////
// Effect                                                //
///////////////////////////////////////////////////////////
bool Effect::operator==(const Effect& rhs) const {
    if (this == &rhs)
        return true;

    if (typeid(*this) != typeid(rhs))
        return false;

    return true;
}

void Effect::Execute(ScriptingContext& context,
                     const TargetSet& targets,
                     AccountingMap* accounting_map,
                     const EffectCause& effect_cause,
                     bool only_meter_effects,
                     bool only_appearance_effects,
                     bool include_empire_meter_effects,
                     bool only_generate_sitrep_effects) const
{
    if (   (only_appearance_effects       && !this->IsAppearanceEffect())
        || (only_meter_effects            && !this->IsMeterEffect())
        || (!include_empire_meter_effects &&  this->IsEmpireMeterEffect())
        || (only_generate_sitrep_effects  && !this->IsSitrepEffect()))
    { return; }
    // generic / most effects don't do anything special for accounting, so just
    // use standard Execute. overrides may implement something else.
    Execute(context, targets);
}

void Effect::Execute(ScriptingContext& context, const TargetSet& targets) const {
    if (targets.empty())
        return;

    // execute effects on targets
    ScriptingContext local_context{context};
    for (auto* target : targets) {
        local_context.effect_target = target;
        Execute(local_context);
    }
}

uint32_t Effect::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "Effect");

    TraceLogger(effects) << "GetCheckSum(EffectsGroup): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// NoOp                                                  //
///////////////////////////////////////////////////////////
void NoOp::Execute(ScriptingContext& context) const
{ DebugLogger(effects) << "Effect::NoOp::Execute: src: " << context.source << "  tgt: " << context.effect_target; }

std::string NoOp::Dump(uint8_t ntabs) const
{ return DumpIndent(ntabs) + "NoOp\n"; }

uint32_t NoOp::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "NoOp");

    TraceLogger(effects) << "GetCheckSum(NoOp): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> NoOp::Clone() const
{ return std::make_unique<NoOp>(); }


///////////////////////////////////////////////////////////
// SetMeter                                              //
///////////////////////////////////////////////////////////
SetMeter::SetMeter(MeterType meter,
                   std::unique_ptr<ValueRef::ValueRef<double>>&& value,
                   boost::optional<std::string> accounting_label) :
    m_meter(meter),
    m_value(std::move(value))
{
    if (accounting_label)
        m_accounting_label = std::move(*accounting_label);
}

bool SetMeter::operator==(const Effect& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const SetMeter& rhs_ = static_cast<const SetMeter&>(rhs);

    if (m_meter != rhs_.m_meter ||
        m_accounting_label != rhs_.m_accounting_label)
    { return false; }

    CHECK_COND_VREF_MEMBER(m_value)

    return true;
}

namespace {
    std::string TargetsDump(const TargetSet& targets) {
        std::string retval;
        retval.reserve(1500*targets.size()); // rough guesstimate
        for (auto* target : targets)
            retval.append("\n").append(target->Dump());
        return retval;
    }

    constexpr double OperateData(::ValueRef::OpType op_type, double lhs, double rhs)
    { return ::ValueRef::OperateData(op_type, lhs, rhs, ::ValueRef::vr_rand_int, ::ValueRef::vr_rand_double); }
}

void SetMeter::Execute(ScriptingContext& context) const {
    if (!context.effect_target || !m_value)
        return;
    if (Meter* meter = context.effect_target->GetMeter(m_meter)) {
        if (m_value->TargetInvariant()) {
            meter->SetCurrent(static_cast<float>(m_value->Eval(context)));
        } else {
            const ScriptingContext::CurrentValueVariant cvv{static_cast<double>(meter->Current())};
            const ScriptingContext target_meter_context(context, ScriptingContext::Target{}, context.effect_target, cvv);
            meter->SetCurrent(static_cast<float>(m_value->Eval(target_meter_context)));
        }
    }
}

void SetMeter::Execute(ScriptingContext& context,
                       const TargetSet& targets,
                       AccountingMap* accounting_map,
                       const EffectCause& effect_cause,
                       bool only_meter_effects,
                       bool only_appearance_effects,
                       bool include_empire_meter_effects,
                       bool only_generate_sitrep_effects) const
{
    if (only_appearance_effects || only_generate_sitrep_effects)
        return;
    if (targets.empty())
        return;

    TraceLogger(effects) << "\n\nExecute SetMeter effect: \n" << Dump();
    TraceLogger(effects) << "SetMeter execute " << targets.size() << " before:" << TargetsDump(targets);

    const int source_id{context.source ? context.source->ID() : INVALID_OBJECT_ID};
    const auto& accounting_label{m_accounting_label.empty() ? effect_cause.custom_label : m_accounting_label};
    static std::decay_t<decltype(*accounting_map)> EMPTY_ACCOUNTING = {}; // dummy thing to bind lambda capture reference to
    auto& accounting = accounting_map ? *accounting_map : EMPTY_ACCOUNTING;

    auto update_meter =
        [source_id, &accounting_label, &effect_cause, meter_type{m_meter},
         have_accounting{accounting_map != nullptr}, &accounting]
        (float new_meter_value, int target_id, Meter* meter) -> void
    {
        auto old_value = meter->Current();
        meter->SetCurrent(new_meter_value);

        if (have_accounting) {
            auto diff = new_meter_value - old_value;
            accounting[target_id][meter_type].emplace_back(
                source_id, effect_cause.cause_type,
                diff, new_meter_value,
                effect_cause.specific_cause, accounting_label);
        }
    };


    if (m_value->TargetInvariant()) {
        // meter value does not depend on target, so handle with single ValueRef evaluation
        const auto new_val = static_cast<float>(m_value->Eval(context));
        for (auto* target : targets) {
            if (target) {
                if (Meter* meter = target->GetMeter(m_meter))
                    update_meter(new_val, target->ID(), meter);
            }
        }

    } else if (m_value->SimpleIncrement()) {
        const auto op_ref = static_cast<ValueRef::Operation<double>*>(m_value.get());
        const auto op_type = op_ref->GetOpType();
        const auto rhs = op_ref->RHS()->Eval(context);
        assert(op_ref->LHS() && op_ref->LHS()->IsEffectModifiedValueReference());

        for (auto* target : targets) {
            if (target) {
                if (Meter* meter = target->GetMeter(m_meter)) {
                    auto lhs = static_cast<double>(meter->Current());
                    auto new_val = static_cast<float>(OperateData(op_type, lhs, rhs));
                    update_meter(new_val, target->ID(), meter);
                }
            }
        }

    } else if (targets.size() == 1) {
        if (auto* target = targets.front()) {
            if (Meter* meter = target->GetMeter(m_meter)) {
                const ScriptingContext::CurrentValueVariant cvv{static_cast<double>(meter->Current())};
                const ScriptingContext target_meter_context(context, ScriptingContext::Target{}, target, cvv);
                auto new_val = static_cast<float>(m_value->Eval(target_meter_context));
                update_meter(new_val, target->ID(), meter);
            }
        }

    } else {
        // calculate new meter values before modifying anything...
        std::vector<std::tuple<decltype(Meter().Current()), int, Meter*>> target_new_meter_vals;
        target_new_meter_vals.reserve(targets.size());
        for (auto* target : targets) {
            if (!target)
                continue;
            if (Meter* meter = target->GetMeter(m_meter)) {
                const ScriptingContext::CurrentValueVariant cvv{static_cast<double>(meter->Current())};
                const ScriptingContext target_meter_context(context, ScriptingContext::Target{}, target, cvv);
                target_new_meter_vals.emplace_back(m_value->Eval(target_meter_context), target->ID(), meter);
            }
        }

        // set new meter values and update accounting
        for (const auto& [new_val, target_id, meter] : target_new_meter_vals)
            update_meter(new_val, target_id, meter);
    }

    TraceLogger(effects) << "SetMeter execute " << targets.size() << " after:" << TargetsDump(targets);
}

void SetMeter::Execute(ScriptingContext& context, const TargetSet& targets) const {
#if defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 12) || (__GNUC__ == 12 && __GNUC_MINOR__ >= 2))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934))) && ((!defined(__clang_major__) || (__clang_major__ >= 17)))
    static constexpr const EffectCause default_effect_cause;
#else
    static const EffectCause default_effect_cause;
#endif
    Execute(context, targets, nullptr, default_effect_cause);
}

std::string SetMeter::Dump(uint8_t ntabs) const {
    std::string retval = DumpIndent(ntabs) + "Set";
    switch (m_meter) {
    case MeterType::METER_TARGET_POPULATION:   retval += "TargetPopulation"; break;
    case MeterType::METER_TARGET_INDUSTRY:     retval += "TargetIndustry"; break;
    case MeterType::METER_TARGET_RESEARCH:     retval += "TargetResearch"; break;
    case MeterType::METER_TARGET_INFLUENCE:    retval += "TargetInfluence"; break;
    case MeterType::METER_TARGET_CONSTRUCTION: retval += "TargetConstruction"; break;
    case MeterType::METER_TARGET_HAPPINESS:    retval += "TargetHappiness"; break;

    case MeterType::METER_MAX_CAPACITY:        retval += "MaxCapacity"; break;

    case MeterType::METER_MAX_FUEL:            retval += "MaxFuel"; break;
    case MeterType::METER_MAX_SHIELD:          retval += "MaxShield"; break;
    case MeterType::METER_MAX_STRUCTURE:       retval += "MaxStructure"; break;
    case MeterType::METER_MAX_DEFENSE:         retval += "MaxDefense"; break;
    case MeterType::METER_MAX_SUPPLY:          retval += "MaxSupply"; break;
    case MeterType::METER_MAX_STOCKPILE:       retval += "MaxStockpile"; break;
    case MeterType::METER_MAX_TROOPS:          retval += "MaxTroops"; break;

    case MeterType::METER_POPULATION:          retval += "Population"; break;
    case MeterType::METER_INDUSTRY:            retval += "Industry"; break;
    case MeterType::METER_RESEARCH:            retval += "Research"; break;
    case MeterType::METER_INFLUENCE:           retval += "Influence"; break;
    case MeterType::METER_CONSTRUCTION:        retval += "Construction"; break;
    case MeterType::METER_HAPPINESS:           retval += "Happiness"; break;

    case MeterType::METER_CAPACITY:            retval += "Capacity"; break;

    case MeterType::METER_FUEL:                retval += "Fuel"; break;
    case MeterType::METER_SHIELD:              retval += "Shield"; break;
    case MeterType::METER_STRUCTURE:           retval += "Structure"; break;
    case MeterType::METER_DEFENSE:             retval += "Defense"; break;
    case MeterType::METER_SUPPLY:              retval += "Supply"; break;
    case MeterType::METER_STOCKPILE:           retval += "Stockpile"; break;
    case MeterType::METER_TROOPS:              retval += "Troops"; break;

    case MeterType::METER_REBEL_TROOPS:        retval += "RebelTroops"; break;
    case MeterType::METER_SIZE:                retval += "Size"; break;
    case MeterType::METER_STEALTH:             retval += "Stealth"; break;
    case MeterType::METER_DETECTION:           retval += "Detection"; break;
    case MeterType::METER_SPEED:               retval += "Speed"; break;

    default:                                   retval += "?"; break;
    }
    retval += " value = " + m_value->Dump(ntabs) + "\n";
    return retval;
}

void SetMeter::SetTopLevelContent(const std::string& content_name) {
    if (m_value)
        m_value->SetTopLevelContent(content_name);
}

uint32_t SetMeter::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "SetMeter");
    CheckSums::CheckSumCombine(retval, m_meter);
    CheckSums::CheckSumCombine(retval, m_value);
    CheckSums::CheckSumCombine(retval, m_accounting_label);

    TraceLogger(effects) << "GetCheckSum(SetMeter): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> SetMeter::Clone() const
{ return std::make_unique<SetMeter>(m_meter, ValueRef::CloneUnique(m_value), m_accounting_label); }


///////////////////////////////////////////////////////////
// SetShipPartMeter                                      //
///////////////////////////////////////////////////////////
SetShipPartMeter::SetShipPartMeter(MeterType meter,
                                   std::unique_ptr<ValueRef::ValueRef<std::string>>&& part_name,
                                   std::unique_ptr<ValueRef::ValueRef<double>>&& value) :
    m_part_name(std::move(part_name)),
    m_meter(meter),
    m_value(std::move(value))
{}

bool SetShipPartMeter::operator==(const Effect& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const SetShipPartMeter& rhs_ = static_cast<const SetShipPartMeter&>(rhs);

    if (m_meter != rhs_.m_meter)
        return false;

    CHECK_COND_VREF_MEMBER(m_part_name)
    CHECK_COND_VREF_MEMBER(m_value)

    return true;
}

void SetShipPartMeter::Execute(ScriptingContext& context) const {
    if (!context.effect_target) return;

    if (!m_part_name || !m_value) {
        ErrorLogger(effects) << "SetShipPartMeter::Execute missing part name or value ValueRefs";
        return;
    }

    if (context.effect_target->ObjectType() != UniverseObjectType::OBJ_SHIP) {
        ErrorLogger(effects) << "SetShipPartMeter::Execute acting on non-ship target:";
        //context.effect_target->Dump();
        return;
    }

    const auto part_name = m_part_name->TargetInvariant() ?
        m_part_name->Eval(context) :
        m_part_name->Eval(ScriptingContext{context, ScriptingContext::Target{}, context.effect_target, EMPTY_STRING_CURRENT_VALUE});

    Meter* meter = static_cast<Ship*>(context.effect_target)->GetPartMeter(m_meter, part_name);
    if (!meter)
        return;

    if (m_value->TargetInvariant()) {
        meter->SetCurrent(static_cast<float>(m_value->Eval(context)));
    } else {
        const ScriptingContext::CurrentValueVariant cvv{static_cast<double>(meter->Current())};
        const ScriptingContext target_meter_context(context, ScriptingContext::Target{}, context.effect_target, cvv);
        meter->SetCurrent(static_cast<float>(m_value->Eval(target_meter_context)));
    }
}

void SetShipPartMeter::Execute(ScriptingContext& context,
                               const TargetSet& targets,
                               AccountingMap* accounting_map,
                               const EffectCause& effect_cause,
                               bool only_meter_effects,
                               bool only_appearance_effects,
                               bool include_empire_meter_effects,
                               bool only_generate_sitrep_effects) const
{
    if (only_appearance_effects || only_generate_sitrep_effects)
        return;

    auto dump_targets = [&targets]() {
        std::string retval;
        retval.reserve(targets.size() * 2000); // guesstimate
        for (auto* target : targets)
            retval.append("\n ... ").append(target->Dump(1));
        return retval;
    };

    TraceLogger(effects) << "\n\nExecute SetShipPartMeter effect: \n" << Dump();
    TraceLogger(effects) << "SetShipPartMeter execute targets before: " << dump_targets();

    Execute(context, targets);

    TraceLogger(effects) << "SetShipPartMeter execute targets after: " << dump_targets();
}

void SetShipPartMeter::Execute(ScriptingContext& context, const TargetSet& targets) const {
    if (targets.empty())
        return;
    if (!m_part_name || !m_value) {
        ErrorLogger(effects) << "SetShipPartMeter::Execute missing part name or value ValueRefs";
        return;
    }


    if (!m_part_name->TargetInvariant()) {
        // part name can be different for each target object, so need to re-evaluate for each

        if (m_value->TargetInvariant()) {
            // new meter value does not depend on target or value modified by this effect,
            // so do a single ValueRef evaluation
            const float new_val = static_cast<float>(m_value->Eval(context));

            for (auto* target : targets) {
                if (!target || target->ObjectType() != UniverseObjectType::OBJ_SHIP)
                    continue;
                const auto part_name = m_part_name->Eval(ScriptingContext{context, ScriptingContext::Target{}, target, EMPTY_STRING_CURRENT_VALUE});
                if (Meter* meter = static_cast<Ship*>(target)->GetPartMeter(m_meter, part_name))
                    meter->SetCurrent(new_val);
            }

        } else if (m_value->SimpleIncrement()) {
            // new meter value is like "Value + Increment", where + may be any operation and
            // Increment is a single target-invariant value (effectively constant for this Execute call)
            const auto op_ref = static_cast<ValueRef::Operation<double>*>(m_value.get());
            const auto op_type = op_ref->GetOpType();
            const auto rhs = op_ref->RHS()->Eval(context);
            assert(op_ref->LHS() && op_ref->LHS()->IsEffectModifiedValueReference());

            for (auto* target : targets) {
                if (!target || target->ObjectType() != UniverseObjectType::OBJ_SHIP)
                    continue;
                const auto part_name = m_part_name->Eval(ScriptingContext{context, ScriptingContext::Target{}, target, EMPTY_STRING_CURRENT_VALUE});
                if (Meter* meter = static_cast<Ship*>(target)->GetPartMeter(m_meter, part_name)) {
                    auto new_val = OperateData(op_type, static_cast<double>(meter->Current()), rhs);
                    meter->SetCurrent(static_cast<float>(new_val));
                }
            }

        } else if (targets.size() == 1) {
            auto* target = targets.front();
            if (!target || target->ObjectType() != UniverseObjectType::OBJ_SHIP)
                return;
            const auto part_name = m_part_name->Eval(ScriptingContext{context, ScriptingContext::Target{}, target, EMPTY_STRING_CURRENT_VALUE});
            if (Meter* meter = static_cast<Ship*>(target)->GetPartMeter(m_meter, part_name)) {
                const ScriptingContext::CurrentValueVariant cvv{static_cast<double>(meter->Current())};
                const ScriptingContext target_meter_context(context, ScriptingContext::Target{}, target, cvv);
                meter->SetCurrent(static_cast<float>(m_value->Eval(target_meter_context)));
            }

        } else {
            // each new meter value could be anything, including depending on other meter values
            // that are being modified in this effect. So,  calculate new meter values before modifying anything
            std::vector<std::pair<float, Meter*>> target_new_meter_vals;
            target_new_meter_vals.reserve(targets.size());

            for (auto* target : targets) {
                if (!target || target->ObjectType() != UniverseObjectType::OBJ_SHIP)
                    continue;
                const ScriptingContext target_context{context, ScriptingContext::Target{}, target, EMPTY_STRING_CURRENT_VALUE};
                const auto part_name = m_part_name->Eval(target_context);
                if (Meter* meter = static_cast<Ship*>(target)->GetPartMeter(m_meter, part_name)) {
                    const ScriptingContext::CurrentValueVariant cvv{static_cast<double>(meter->Current())};
                    const ScriptingContext target_meter_context(context, ScriptingContext::Target{}, target, cvv);
                    target_new_meter_vals.emplace_back(static_cast<float>(m_value->Eval(target_meter_context)), meter);
                }
            }

            // set new meter values and update accounting
            for (auto& [new_val, meter] : target_new_meter_vals)
                meter->SetCurrent(new_val);
        }

    } else {
        // part name doesn't depend on target, so handle with single ValueRef evaluation
        const auto part_name = m_part_name->Eval(context);

        if (m_value->TargetInvariant()) {
            // meter value does not depend on target, so handle with single ValueRef evaluation
            auto new_val = m_value->Eval(context);
            for (auto* target : targets) {
                if (!target || target->ObjectType() != UniverseObjectType::OBJ_SHIP)
                    continue;
                if (Meter* meter = static_cast<Ship*>(target)->GetPartMeter(m_meter, part_name))
                    meter->SetCurrent(new_val);
            }

        } else if (m_value->SimpleIncrement()) {
            const auto op_ref = static_cast<ValueRef::Operation<double>*>(m_value.get());
            const auto op_type = op_ref->GetOpType();
            const auto rhs = op_ref->RHS()->Eval(context);
            assert(op_ref->LHS() && op_ref->LHS()->IsEffectModifiedValueReference());

            for (auto* target : targets) {
                if (!target || target->ObjectType() != UniverseObjectType::OBJ_SHIP)
                    continue;
                if (Meter* meter = static_cast<Ship*>(target)->GetPartMeter(m_meter, part_name))
                    meter->SetCurrent(OperateData(op_type, static_cast<double>(meter->Current()), rhs));
            }

        } else if (targets.size() == 1) {
            auto* target = targets.front();
            if (!target || target->ObjectType() != UniverseObjectType::OBJ_SHIP)
                return;
            if (Meter* meter = static_cast<Ship*>(target)->GetPartMeter(m_meter, part_name)) {
                const ScriptingContext::CurrentValueVariant cvv{static_cast<double>(meter->Current())};
                const ScriptingContext target_meter_context(context, ScriptingContext::Target{}, target, cvv);
                meter->SetCurrent(static_cast<float>(m_value->Eval(target_meter_context)));
            }

        } else {
            // calculate new meter values before modifying anything...
            std::vector<std::pair<double, Meter*>> target_new_meter_vals;
            target_new_meter_vals.reserve(targets.size());
            for (auto* target : targets) {
                if (!target || target->ObjectType() != UniverseObjectType::OBJ_SHIP)
                    continue;
                if (Meter* meter = static_cast<Ship*>(target)->GetPartMeter(m_meter, part_name)) {
                    const ScriptingContext::CurrentValueVariant cvv{static_cast<double>(meter->Current())};
                    const ScriptingContext target_meter_context(context, ScriptingContext::Target{}, target, cvv);
                    meter->SetCurrent(static_cast<float>(m_value->Eval(target_meter_context)));
                }
            }

            // set new meter values and update accounting
            for (auto [new_val, meter] : target_new_meter_vals)
                meter->SetCurrent(new_val);
        }
    }
}

std::string SetShipPartMeter::Dump(uint8_t ntabs) const {
    std::string retval = DumpIndent(ntabs);
    switch (m_meter) {
        case MeterType::METER_CAPACITY:            retval += "SetCapacity";        break;
        case MeterType::METER_MAX_CAPACITY:        retval += "SetMaxCapacity";     break;
        case MeterType::METER_SECONDARY_STAT:      retval += "SetSecondaryStat";   break;
        case MeterType::METER_MAX_SECONDARY_STAT:  retval += "SetMaxSecondaryStat";break;
        default:                                   retval += "Set???";         break;
    }

    if (m_part_name)
        retval += " partname = " + m_part_name->Dump(ntabs);

    retval += " value = " + m_value->Dump(ntabs);

    return retval;
}

void SetShipPartMeter::SetTopLevelContent(const std::string& content_name) {
    if (m_value)
        m_value->SetTopLevelContent(content_name);
    if (m_part_name)
        m_part_name->SetTopLevelContent(content_name);
}

uint32_t SetShipPartMeter::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "SetShipPartMeter");
    CheckSums::CheckSumCombine(retval, m_part_name);
    CheckSums::CheckSumCombine(retval, m_meter);
    CheckSums::CheckSumCombine(retval, m_value);

    TraceLogger(effects) << "GetCheckSum(SetShipPartMeter): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> SetShipPartMeter::Clone() const {
    return std::make_unique<SetShipPartMeter>(m_meter,
                                              ValueRef::CloneUnique(m_part_name),
                                              ValueRef::CloneUnique(m_value));
}


///////////////////////////////////////////////////////////
// SetEmpireMeter                                        //
///////////////////////////////////////////////////////////
SetEmpireMeter::SetEmpireMeter(std::string& meter, std::unique_ptr<ValueRef::ValueRef<double>>&& value) :
    m_empire_id(std::make_unique<ValueRef::Variable<int>>(
        ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE, "Owner")),
    m_meter(std::move(meter)),
    m_value(std::move(value))
{}

SetEmpireMeter::SetEmpireMeter(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id, std::string& meter,
                               std::unique_ptr<ValueRef::ValueRef<double>>&& value) :
    m_empire_id(std::move(empire_id)),
    m_meter(std::move(meter)),
    m_value(std::move(value))
{}

bool SetEmpireMeter::operator==(const Effect& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const SetEmpireMeter& rhs_ = static_cast<const SetEmpireMeter&>(rhs);

    if (m_meter != rhs_.m_meter)
        return false;

    CHECK_COND_VREF_MEMBER(m_empire_id)
    CHECK_COND_VREF_MEMBER(m_value)

    return true;
}

namespace {
    Meter* GetEmpireMeter(ScriptingContext& context, int empire_id, const std::string& meter) {
        const auto empire = context.GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger(effects) << "SetEmpireMeter::Execute unable to find empire with id " << empire_id;
            return nullptr;
        } else if (Meter* m = empire->GetMeter(meter)) {
            return m;
        } else {
            ErrorLogger(effects) << "SetEmpireMeter::Execute empire " << empire->Name()
                                 << " doesn't have a meter named " << meter;
            return nullptr;
        }
    }
}

void SetEmpireMeter::Execute(ScriptingContext& context) const {
    if (!context.effect_target) return;

    if (!m_empire_id || !m_value || m_meter.empty()) {
        ErrorLogger(effects) << "SetEmpireMeter::Execute missing empire id or value ValueRefs, or given empty meter name";
        return;
    }
    const auto empire_id = m_empire_id->Eval(context);
    auto* meter = GetEmpireMeter(context, empire_id, m_meter);
    if (!meter) {
        ErrorLogger(effects) << "SetEmpireMeter::Execute found no empire " << empire_id << " meter named " << m_meter;
        return;
    }
    if (m_value->TargetInvariant()) {
        meter->SetCurrent(static_cast<float>(m_value->Eval(context)));
    } else {
        const ScriptingContext::CurrentValueVariant cvv{static_cast<double>(meter->Current())};
        const ScriptingContext target_meter_context{context, cvv};
        meter->SetCurrent(static_cast<float>(m_value->Eval(target_meter_context)));
    }
}

void SetEmpireMeter::Execute(ScriptingContext& context,
                             const TargetSet& targets,
                             AccountingMap* accounting_map,
                             const EffectCause& effect_cause,
                             bool only_meter_effects,
                             bool only_appearance_effects,
                             bool include_empire_meter_effects,
                             bool only_generate_sitrep_effects) const
{
    if (!include_empire_meter_effects ||
        only_appearance_effects ||
        only_generate_sitrep_effects)
    { return; }
    // presently no accounting done for empire meters.
    // TODO: maybe implement empire meter effect accounting?
    Execute(context, targets);
}

void SetEmpireMeter::Execute(ScriptingContext& context, const TargetSet& targets) const {
    if (targets.empty())
        return;
    if (!m_empire_id || !m_value || m_meter.empty()) {
        ErrorLogger(effects) << "SetEmpireMeter::Execute missing empire id or value ValueRefs or meter name";
        return;
    }

    if (!m_empire_id->TargetInvariant()) {
        // need to re-evaluate empire id for each target object
        if (m_value->TargetInvariant()) {
            // value to set does not depend on target object or current value of meter being set,
            // so handle with a single ValueRef evaluation
            const float new_val = static_cast<float>(m_value->Eval(context));

            // empire id, and thus which meter is being modified, can be different per-target,
            // so re-get for each target
            for (auto* target : targets) {
                if (target) {
                    ScriptingContext target_context{context, ScriptingContext::Target{}, target, ZERO_INT_CURRENT_VALUE};
                    const auto empire_id = m_empire_id->Eval(target_context);
                    if (auto* meter = GetEmpireMeter(context, empire_id, m_meter))
                        meter->SetCurrent(new_val);
                }
            }

        } else if (m_value->SimpleIncrement()) {
            // new meter value is like "Value + Increment", where + may be any operation and
            // Increment is a single target-invariant value (effectively constant for this Execute call)
            const auto op_ref = static_cast<ValueRef::Operation<double>*>(m_value.get());
            assert(op_ref->RHS()->TargetInvariant());
            const auto op_type = op_ref->GetOpType();
            const auto rhs = op_ref->RHS()->Eval(context);
            assert(op_ref->LHS() && op_ref->LHS()->IsEffectModifiedValueReference());

            for (auto* target : targets) {
                if (target) {
                    ScriptingContext target_context{context, ScriptingContext::Target{}, target, ZERO_INT_CURRENT_VALUE};
                    const auto empire_id = m_empire_id->Eval(target_context);
                    if (auto* meter = GetEmpireMeter(context, empire_id, m_meter)) {
                        const auto new_val = OperateData(op_type, static_cast<double>(meter->Current()), rhs);
                        meter->SetCurrent(static_cast<float>(new_val));
                    }
                }
            }

        } else {
            // for empire meters, unlike object meters, pre-calculating new meter values
            // won't work because multiple target objects could be associated with the same
            // empire meter, which could have its current value modified repeatedly.
            // thus, all new meter values need to be calculated separately and in order
            for (auto* target : targets) {
                if (target) {
                    ScriptingContext target_context{context, ScriptingContext::Target{}, target, ZERO_INT_CURRENT_VALUE};
                    const auto empire_id = m_empire_id->Eval(target_context);
                    if (auto* meter = GetEmpireMeter(context, empire_id, m_meter)) {
                        const ScriptingContext::CurrentValueVariant cvv{meter->Current()};
                        const ScriptingContext target_meter_context{context, ScriptingContext::Target{}, target, cvv};
                        meter->SetCurrent(static_cast<float>(m_value->Eval(target_meter_context)));
                    }
                }
            }
        }

        return;
    }



    // empire id doesn't depend on target, so determine it with a single ValueRef evaluation
    const auto empire_id = m_empire_id->Eval(context);
    auto* meter = GetEmpireMeter(context, empire_id, m_meter);
    if (!meter) {
        ErrorLogger() << "SetEmpireMeter couldn't get empire id " << empire_id << " meter " << m_meter;
        return;
    }

    if (m_value->TargetInvariant()) {
        // meter value does not depend on target, so handle with single ValueRef evaluation and just set once
        // if there are multiple targets, that shouldn't matter, since the same empire id will be used for all
        meter->SetCurrent(static_cast<float>(m_value->Eval(context)));

    } else if (m_value->SimpleIncrement()) {
        // meter value is modified by a fixed other value once per target
        const auto op_ref = static_cast<ValueRef::Operation<double>*>(m_value.get());
        assert(op_ref->RHS()->TargetInvariant());
        const auto op_type = op_ref->GetOpType();
        const auto rhs = op_ref->RHS()->Eval(context);
        assert(op_ref->LHS() && op_ref->LHS()->IsEffectModifiedValueReference());

        double lhs = static_cast<double>(meter->Current());
        for (const auto* _ : targets) {
            (void)_;
            lhs = OperateData(op_type, lhs, rhs);
        }
        meter->SetCurrent(static_cast<float>(lhs));

    } else {
        // for empire meters, unlike object meters, pre-calculating new meter values
        // won't work because multiple target objects are associated with the same
        // empire meter (all the same empire_id here), and that meter could have its
        // current value modified repeatedly to different results.
        // thus, all the new meter values need to be calculated separately and in order
        for (auto* target : targets) {
            if (target) {
                const ScriptingContext::CurrentValueVariant cvv{meter->Current()};
                const ScriptingContext target_meter_context{context, ScriptingContext::Target{}, target, cvv};
                meter->SetCurrent(static_cast<float>(m_value->Eval(target_meter_context)));
            }
        }
    }
}

std::string SetEmpireMeter::Dump(uint8_t ntabs) const
{ return DumpIndent(ntabs) + "SetEmpireMeter meter = " + m_meter + " empire = " + m_empire_id->Dump(ntabs) + " value = " + m_value->Dump(ntabs); }

void SetEmpireMeter::SetTopLevelContent(const std::string& content_name) {
    if (m_empire_id)
        m_empire_id->SetTopLevelContent(content_name);
    if (m_value)
        m_value->SetTopLevelContent(content_name);
}

uint32_t SetEmpireMeter::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "SetEmpireMeter");
    CheckSums::CheckSumCombine(retval, m_empire_id);
    CheckSums::CheckSumCombine(retval, m_meter);
    CheckSums::CheckSumCombine(retval, m_value);

    TraceLogger(effects) << "GetCheckSum(SetEmpireMeter): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> SetEmpireMeter::Clone() const {
    auto meter{m_meter};
    return std::make_unique<SetEmpireMeter>(ValueRef::CloneUnique(m_empire_id),
                                            meter,
                                            ValueRef::CloneUnique(m_value));
}


///////////////////////////////////////////////////////////
// SetEmpireStockpile                                    //
///////////////////////////////////////////////////////////
SetEmpireStockpile::SetEmpireStockpile(ResourceType stockpile,
                                       std::unique_ptr<ValueRef::ValueRef<double>>&& value) :
    m_empire_id(std::make_unique<ValueRef::Variable<int>>(
        ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE, "Owner")),
    m_stockpile(stockpile),
    m_value(std::move(value))
{}

SetEmpireStockpile::SetEmpireStockpile(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id,
                                       ResourceType stockpile,
                                       std::unique_ptr<ValueRef::ValueRef<double>>&& value) :
    m_empire_id(std::move(empire_id)),
    m_stockpile(stockpile),
    m_value(std::move(value))
{}

bool SetEmpireStockpile::operator==(const Effect& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const SetEmpireStockpile& rhs_ = static_cast<const SetEmpireStockpile&>(rhs);

    if (m_stockpile != rhs_.m_stockpile)
        return false;

    CHECK_COND_VREF_MEMBER(m_empire_id)
    CHECK_COND_VREF_MEMBER(m_value)

    return true;
}

void SetEmpireStockpile::Execute(ScriptingContext& context) const {
    int empire_id = m_empire_id->Eval(context);

    auto empire = context.GetEmpire(empire_id);
    if (!empire) {
        DebugLogger(effects) << "SetEmpireStockpile::Execute couldn't find an empire with id " << empire_id;
        return;
    }

    const ScriptingContext::CurrentValueVariant cvv{empire->ResourceStockpile(m_stockpile)};
    const ScriptingContext stockpile_context{context, cvv};
    empire->SetResourceStockpile(m_stockpile, m_value->Eval(stockpile_context));
}

std::string SetEmpireStockpile::Dump(uint8_t ntabs) const {
    std::string retval = DumpIndent(ntabs);
    switch (m_stockpile) {
    // TODO: Support for other resource stockpiles?
    case ResourceType::RE_INDUSTRY:  retval += "SetEmpireStockpile"; break;
    case ResourceType::RE_INFLUENCE: retval += "SetEmpireStockpile"; break;
    case ResourceType::RE_RESEARCH:  retval += "SetEmpireStockpile"; break;
    default:                         retval += "?"; break;
    }
    retval += " empire = " + m_empire_id->Dump(ntabs) + " value = " + m_value->Dump(ntabs) + "\n";
    return retval;
}

void SetEmpireStockpile::SetTopLevelContent(const std::string& content_name) {
    if (m_empire_id)
        m_empire_id->SetTopLevelContent(content_name);
    if (m_value)
        m_value->SetTopLevelContent(content_name);
}

uint32_t SetEmpireStockpile::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "SetEmpireStockpile");
    CheckSums::CheckSumCombine(retval, m_empire_id);
    CheckSums::CheckSumCombine(retval, m_stockpile);
    CheckSums::CheckSumCombine(retval, m_value);

    TraceLogger(effects) << "GetCheckSum(SetEmpireStockpile): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> SetEmpireStockpile::Clone() const {
    return std::make_unique<SetEmpireStockpile>(ValueRef::CloneUnique(m_empire_id),
                                                m_stockpile,
                                                ValueRef::CloneUnique(m_value));
}


///////////////////////////////////////////////////////////
// SetEmpireCapital                                      //
///////////////////////////////////////////////////////////
SetEmpireCapital::SetEmpireCapital() :
    m_empire_id(std::make_unique<ValueRef::Variable<int>>(
        ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE, "Owner"))
{}

SetEmpireCapital::SetEmpireCapital(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id) :
    m_empire_id(std::move(empire_id))
{}

bool SetEmpireCapital::operator==(const Effect& rhs) const {
    if (this == &rhs)
        return true;
    if (typeid(*this) != typeid(rhs))
        return false;

    const SetEmpireCapital& rhs_ = static_cast<const SetEmpireCapital&>(rhs);

    CHECK_COND_VREF_MEMBER(m_empire_id)

    return true;
}

void SetEmpireCapital::Execute(ScriptingContext& context) const {
    if (!context.effect_target || context.effect_target->ObjectType() != UniverseObjectType::OBJ_PLANET)
        return;

    int empire_id = m_empire_id->Eval(context);
    if (auto empire = context.GetEmpire(empire_id))
        empire->SetCapitalID(context.effect_target->ID(), context.ContextObjects());
    context.Empires().RefreshCapitalIDs();
}

std::string SetEmpireCapital::Dump(uint8_t ntabs) const
{ return DumpIndent(ntabs) + "SetEmpireCapital empire = " + m_empire_id->Dump(ntabs) + "\n"; }

void SetEmpireCapital::SetTopLevelContent(const std::string& content_name) {
    if (m_empire_id)
        m_empire_id->SetTopLevelContent(content_name);
}

uint32_t SetEmpireCapital::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "SetEmpireCapital");
    CheckSums::CheckSumCombine(retval, m_empire_id);

    TraceLogger(effects) << "GetCheckSum(SetEmpireCapital): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> SetEmpireCapital::Clone() const
{ return std::make_unique<SetEmpireCapital>(ValueRef::CloneUnique(m_empire_id)); }


///////////////////////////////////////////////////////////
// SetPlanetType                                         //
///////////////////////////////////////////////////////////
SetPlanetType::SetPlanetType(std::unique_ptr<ValueRef::ValueRef<PlanetType>>&& type) :
    m_type(std::move(type))
{}

void SetPlanetType::Execute(ScriptingContext& context) const {
    if (!context.effect_target || context.effect_target->ObjectType() != UniverseObjectType::OBJ_PLANET)
        return;
    auto p = static_cast<Planet*>(context.effect_target);

    const ScriptingContext::CurrentValueVariant cvv{p->Type()};
    const ScriptingContext type_context{context, cvv};
    const PlanetType type = m_type->Eval(type_context);
    p->SetType(type);

    if (type == PlanetType::PT_ASTEROIDS)
        p->SetSize(PlanetSize::SZ_ASTEROIDS);
    else if (type == PlanetType::PT_GASGIANT)
        p->SetSize(PlanetSize::SZ_GASGIANT);
    else if (p->Size() == PlanetSize::SZ_ASTEROIDS)
        p->SetSize(PlanetSize::SZ_TINY);
    else if (p->Size() == PlanetSize::SZ_GASGIANT)
        p->SetSize(PlanetSize::SZ_HUGE);
}

std::string SetPlanetType::Dump(uint8_t ntabs) const
{ return DumpIndent(ntabs) + "SetPlanetType type = " + m_type->Dump(ntabs) + "\n"; }

void SetPlanetType::SetTopLevelContent(const std::string& content_name) {
    if (m_type)
        m_type->SetTopLevelContent(content_name);
}

uint32_t SetPlanetType::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "SetPlanetType");
    CheckSums::CheckSumCombine(retval, m_type);

    TraceLogger(effects) << "GetCheckSum(SetPlanetType): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> SetPlanetType::Clone() const
{ return std::make_unique<SetPlanetType>(ValueRef::CloneUnique(m_type)); }


///////////////////////////////////////////////////////////
// SetOriginalType                                         //
///////////////////////////////////////////////////////////
SetOriginalType::SetOriginalType(std::unique_ptr<ValueRef::ValueRef<PlanetType>>&& type) :
    m_type(std::move(type))
{}

void SetOriginalType::Execute(ScriptingContext& context) const {
    if (!context.effect_target || context.effect_target->ObjectType() != UniverseObjectType::OBJ_PLANET)
        return;
    auto p = static_cast<Planet*>(context.effect_target);

    const ScriptingContext::CurrentValueVariant cvv{p->OriginalType()};
    const ScriptingContext type_context{context, cvv};
    PlanetType type = m_type->Eval(type_context);
    p->SetOriginalType(type);

    if (type == PlanetType::PT_ASTEROIDS)
        p->SetSize(PlanetSize::SZ_ASTEROIDS);
    else if (type == PlanetType::PT_GASGIANT)
        p->SetSize(PlanetSize::SZ_GASGIANT);
    else if (p->Size() == PlanetSize::SZ_ASTEROIDS)
        p->SetSize(PlanetSize::SZ_TINY);
    else if (p->Size() == PlanetSize::SZ_GASGIANT)
        p->SetSize(PlanetSize::SZ_HUGE);
}

std::string SetOriginalType::Dump(uint8_t ntabs) const
{ return DumpIndent(ntabs) + "SetOriginalType type = " + m_type->Dump(ntabs) + "\n"; }

void SetOriginalType::SetTopLevelContent(const std::string& content_name) {
    if (m_type)
        m_type->SetTopLevelContent(content_name);
}

uint32_t SetOriginalType::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "SetOriginalType");
    CheckSums::CheckSumCombine(retval, m_type);

    TraceLogger(effects) << "GetCheckSum(SetOriginalType): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> SetOriginalType::Clone() const
{ return std::make_unique<SetOriginalType>(ValueRef::CloneUnique(m_type)); }


///////////////////////////////////////////////////////////
// SetPlanetSize                                         //
///////////////////////////////////////////////////////////
SetPlanetSize::SetPlanetSize(std::unique_ptr<ValueRef::ValueRef<PlanetSize>>&& size) :
    m_size(std::move(size))
{}

void SetPlanetSize::Execute(ScriptingContext& context) const {
    if (!context.effect_target || context.effect_target->ObjectType() != UniverseObjectType::OBJ_PLANET)
        return;
    auto p = static_cast<Planet*>(context.effect_target);

    const ScriptingContext::CurrentValueVariant cvv{p->Size()};
    const ScriptingContext size_context{context, cvv};
    PlanetSize size = m_size->Eval(size_context);
    p->SetSize(size);

    if (size == PlanetSize::SZ_ASTEROIDS)
        p->SetType(PlanetType::PT_ASTEROIDS);
    else if (size == PlanetSize::SZ_GASGIANT)
        p->SetType(PlanetType::PT_GASGIANT);
    else if (p->Type() == PlanetType::PT_ASTEROIDS || p->Type() == PlanetType::PT_GASGIANT)
        p->SetType(PlanetType::PT_BARREN);
}

std::string SetPlanetSize::Dump(uint8_t ntabs) const
{ return DumpIndent(ntabs) + "SetPlanetSize size = " + m_size->Dump(ntabs) + "\n"; }

void SetPlanetSize::SetTopLevelContent(const std::string& content_name) {
    if (m_size)
        m_size->SetTopLevelContent(content_name);
}

uint32_t SetPlanetSize::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "SetPlanetSize");
    CheckSums::CheckSumCombine(retval, m_size);

    TraceLogger(effects) << "GetCheckSum(SetPlanetSize): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> SetPlanetSize::Clone() const
{ return std::make_unique<SetPlanetSize>(ValueRef::CloneUnique(m_size)); }


///////////////////////////////////////////////////////////
// SetFocus                                            //
///////////////////////////////////////////////////////////
SetFocus::SetFocus(std::unique_ptr<ValueRef::ValueRef<std::string>>&& focus) :
    m_focus_name(std::move(focus))
{}

void SetFocus::Execute(ScriptingContext& context) const {
    if (!context.effect_target || !m_focus_name ||
        context.effect_target->ObjectType() != UniverseObjectType::OBJ_PLANET)
    { return; }

    Planet* const planet = static_cast<Planet*>(context.effect_target);

    const ScriptingContext::CurrentValueVariant cvv{planet->Focus()};
    const ScriptingContext name_context{context, cvv};
    auto new_focus = m_focus_name->Eval(name_context);
    if (new_focus.empty() || new_focus == planet->Focus())
        return;

    // only change focus if new focus is available
    if (planet->FocusAvailable(new_focus, context))
        planet->SetFocus(std::move(new_focus), context);
}

std::string SetFocus::Dump(uint8_t ntabs) const
{ return DumpIndent(ntabs) + "SetFocus name = " + m_focus_name->Dump(ntabs) + "\n"; }

void SetFocus::SetTopLevelContent(const std::string& content_name) {
    if (m_focus_name)
        m_focus_name->SetTopLevelContent(content_name);
}

uint32_t SetFocus::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "SetFocus");
    CheckSums::CheckSumCombine(retval, m_focus_name);

    TraceLogger(effects) << "GetCheckSum(SetFocus): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> SetFocus::Clone() const
{ return std::make_unique<SetFocus>(ValueRef::CloneUnique(m_focus_name)); }

///////////////////////////////////////////////////////////
// SetSpecies                                            //
///////////////////////////////////////////////////////////
SetSpecies::SetSpecies(std::unique_ptr<ValueRef::ValueRef<std::string>>&& species) :
    m_species_name(std::move(species))
{}

void SetSpecies::Execute(ScriptingContext& context) const {
    if (!context.effect_target)
        return;

    if (context.effect_target->ObjectType() == UniverseObjectType::OBJ_SHIP) {
        auto ship = static_cast<Ship*>(context.effect_target);
        const ScriptingContext::CurrentValueVariant cvv{ship->SpeciesName()};
        const ScriptingContext name_context{context, cvv};
        ship->SetSpecies(m_species_name->Eval(name_context), context.species);
        return;

    } else if (context.effect_target->ObjectType() == UniverseObjectType::OBJ_PLANET) {
        auto planet = static_cast<Planet*>(context.effect_target);

        const ScriptingContext::CurrentValueVariant cvv{planet->SpeciesName()};
        const ScriptingContext name_context{context, cvv};
        planet->SetSpecies(m_species_name->Eval(name_context), context.current_turn, context.species);
        if (!planet->SpeciesName().empty())
            planet->SetLastColonizedByEmpire(planet->Owner());

        // ensure non-empty and permissible focus setting for new species
        auto& initial_focus = planet->Focus();
        auto available_foci = planet->AvailableFoci(context);

        // leave current focus unchanged if available.
        if (std::any_of(available_foci.begin(), available_foci.end(),
                        [&initial_focus](const auto& af) { return initial_focus == af; }))
        { return; }

        const Species* species = context.species.GetSpecies(planet->SpeciesName());
        const auto default_focus = species ? std::string_view{species->DefaultFocus()} : "";

        // chose default focus if available. otherwise use any available focus
        bool default_available = false;
        for (const auto& available_focus : available_foci) {
            if (available_focus == default_focus) {
                default_available = true;
                break;
            }
        }

        if (default_available)
            planet->SetFocus(std::string{default_focus}, context);
        else if (!available_foci.empty())
            planet->SetFocus(std::string{available_foci.front()}, context);
    }
}

std::string SetSpecies::Dump(uint8_t ntabs) const
{ return DumpIndent(ntabs) + "SetSpecies name = " + m_species_name->Dump(ntabs) + "\n"; }

void SetSpecies::SetTopLevelContent(const std::string& content_name) {
    if (m_species_name)
        m_species_name->SetTopLevelContent(content_name);
}

uint32_t SetSpecies::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "SetSpecies");
    CheckSums::CheckSumCombine(retval, m_species_name);

    TraceLogger(effects) << "GetCheckSum(SetSpecies): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> SetSpecies::Clone() const
{ return std::make_unique<SetSpecies>(ValueRef::CloneUnique(m_species_name)); }


///////////////////////////////////////////////////////////
// SetOwner                                              //
///////////////////////////////////////////////////////////
SetOwner::SetOwner(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id) :
    m_empire_id(std::move(empire_id))
{}

void SetOwner::Execute(ScriptingContext& context) const {
    if (!context.effect_target)
        return;
    int initial_owner = context.effect_target->Owner();

    const ScriptingContext owner_context{context, ScriptingContext::CurrentValueVariant{initial_owner}};
    int empire_id = m_empire_id->Eval(owner_context);
    if (initial_owner == empire_id)
        return;

    Universe& universe = context.ContextUniverse();
    ObjectMap& objects = context.ContextObjects();
    context.effect_target->SetOwner(empire_id);

    // TODO: handle planets? buildings?
    if (context.effect_target->ObjectType() == UniverseObjectType::OBJ_SHIP) {
        auto ship = static_cast<Ship*>(context.effect_target);

        // assigning ownership of a ship requires updating the containing
        // fleet, or splitting ship off into a new fleet at the same location
        auto old_fleet = objects.getRaw<Fleet>(ship->FleetID());
        if (!old_fleet)
            return;
        if (old_fleet->Owner() == empire_id)
            return;

        // if ship is armed use old fleet's aggression. otherwise use auto-determined aggression
        auto aggr = ship->IsArmed(context) ? old_fleet->Aggression() : FleetAggression::INVALID_FLEET_AGGRESSION;

        // move ship into new fleet
        std::shared_ptr<Fleet> new_fleet;
        if (auto system = objects.getRaw<System>(ship->SystemID())) {
            new_fleet = CreateNewFleet(system, ship, context, aggr);
        } else {
            auto x = ship->X();
            auto y = ship->Y();
            new_fleet = CreateNewFleet(x, y, std::move(ship), context, aggr);
        }

        if (new_fleet)
            new_fleet->SetNextAndPreviousSystems(old_fleet->NextSystemID(), old_fleet->PreviousSystemID());

        // if old fleet is empty, destroy it.  Don't reassign ownership of fleet
        // in case that would reval something to the recipient that shouldn't be...
        if (old_fleet->Empty())
            universe.EffectDestroy(old_fleet->ID(), INVALID_OBJECT_ID); // no particular source destroyed the fleet in this case
    }
}

std::string SetOwner::Dump(uint8_t ntabs) const
{ return DumpIndent(ntabs) + "SetOwner empire = " + m_empire_id->Dump(ntabs) + "\n"; }

void SetOwner::SetTopLevelContent(const std::string& content_name) {
    if (m_empire_id)
        m_empire_id->SetTopLevelContent(content_name);
}

uint32_t SetOwner::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "SetOwner");
    CheckSums::CheckSumCombine(retval, m_empire_id);

    TraceLogger(effects) << "GetCheckSum(SetOwner): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> SetOwner::Clone() const
{ return std::make_unique<SetOwner>(ValueRef::CloneUnique(m_empire_id)); }


///////////////////////////////////////////////////////////
// SetSpeciesEmpireOpinion                               //
///////////////////////////////////////////////////////////
SetSpeciesEmpireOpinion::SetSpeciesEmpireOpinion(
    std::unique_ptr<ValueRef::ValueRef<std::string>>&& species_name,
    std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id,
    std::unique_ptr<ValueRef::ValueRef<double>>&& opinion,
    bool target_opinion) :
    m_species_name(std::move(species_name)),
    m_empire_id(std::move(empire_id)),
    m_opinion(std::move(opinion)),
    m_target(target_opinion)
{}

void SetSpeciesEmpireOpinion::Execute(ScriptingContext& context) const {
    if (!context.effect_target)
        return;
    if (!m_species_name || !m_opinion || !m_empire_id)
        return;

    const int empire_id = m_empire_id->Eval(context);
    if (empire_id == ALL_EMPIRES)
        return;

    std::string species_name = m_species_name->Eval(context);
    if (species_name.empty())
        return;

    double previous_value_opinion = context.species.SpeciesEmpireOpinion(species_name, empire_id, m_target, true);
    const ScriptingContext::CurrentValueVariant cvv{previous_value_opinion};
    const ScriptingContext opinion_context{context, cvv};
    float new_value_opinion = static_cast<float>(m_opinion->Eval(opinion_context));

    TraceLogger(effects) << "SetSpeciesEmpire" << (m_target ? "Target" : "") << "Opinion "
                         << " initially: " << previous_value_opinion << " new: " << new_value_opinion;

    context.species.SetSpeciesEmpireOpinion(species_name, empire_id, new_value_opinion, m_target);
}

std::string SetSpeciesEmpireOpinion::Dump(uint8_t ntabs) const
{ return DumpIndent(ntabs) + "SetSpeciesEmpireOpinion empire = " + m_empire_id->Dump(ntabs) + "\n"; } // TODO: complete this

void SetSpeciesEmpireOpinion::SetTopLevelContent(const std::string& content_name) {
    if (m_empire_id)
        m_empire_id->SetTopLevelContent(content_name);
    if (m_species_name)
        m_species_name->SetTopLevelContent(content_name);
    if (m_opinion)
        m_opinion->SetTopLevelContent(content_name);
}

uint32_t SetSpeciesEmpireOpinion::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "SetSpeciesEmpireOpinion");
    CheckSums::CheckSumCombine(retval, m_species_name);
    CheckSums::CheckSumCombine(retval, m_empire_id);
    CheckSums::CheckSumCombine(retval, m_opinion);
    CheckSums::CheckSumCombine(retval, m_target);

    TraceLogger(effects) << "GetCheckSum(SetSpeciesEmpireOpinion): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> SetSpeciesEmpireOpinion::Clone() const {
    return std::make_unique<SetSpeciesEmpireOpinion>(ValueRef::CloneUnique(m_species_name),
                                                     ValueRef::CloneUnique(m_empire_id),
                                                     ValueRef::CloneUnique(m_opinion),
                                                     m_target);
}


///////////////////////////////////////////////////////////
// SetSpeciesSpeciesOpinion                              //
///////////////////////////////////////////////////////////
SetSpeciesSpeciesOpinion::SetSpeciesSpeciesOpinion(
    std::unique_ptr<ValueRef::ValueRef<std::string>>&& opinionated_species_name,
    std::unique_ptr<ValueRef::ValueRef<std::string>>&& rated_species_name,
    std::unique_ptr<ValueRef::ValueRef<double>>&& opinion,
    bool target_opinion) :
    m_opinionated_species_name(std::move(opinionated_species_name)),
    m_rated_species_name(std::move(rated_species_name)),
    m_opinion(std::move(opinion)),
    m_target(target_opinion)
{}

void SetSpeciesSpeciesOpinion::Execute(ScriptingContext& context) const {
    if (!context.effect_target)
        return;
    if (!m_opinionated_species_name || !m_opinion || !m_rated_species_name)
        return;

    const std::string opinionated_species_name = m_opinionated_species_name->Eval(context);
    if (opinionated_species_name.empty())
        return;

    const std::string rated_species_name = m_rated_species_name->Eval(context);
    if (rated_species_name.empty())
        return;

    const float previous_value_opinion = context.species.SpeciesSpeciesOpinion(opinionated_species_name,
                                                                               rated_species_name, m_target, true);
    const ScriptingContext::CurrentValueVariant cvv{previous_value_opinion};
    const ScriptingContext opinion_context{context, cvv};
    const float new_value_opinion = static_cast<float>(m_opinion->Eval(opinion_context));

    context.species.SetSpeciesSpeciesOpinion(opinionated_species_name, rated_species_name, new_value_opinion, m_target);
}

std::string SetSpeciesSpeciesOpinion::Dump(uint8_t ntabs) const
{ return DumpIndent(ntabs) + "SetSpeciesSpeciesOpinion" + "\n"; }

void SetSpeciesSpeciesOpinion::SetTopLevelContent(const std::string& content_name) {
    if (m_opinionated_species_name)
        m_opinionated_species_name->SetTopLevelContent(content_name);
    if (m_rated_species_name)
        m_rated_species_name->SetTopLevelContent(content_name);
    if (m_opinion)
        m_opinion->SetTopLevelContent(content_name);
}

uint32_t SetSpeciesSpeciesOpinion::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "SetSpeciesSpeciesOpinion");
    CheckSums::CheckSumCombine(retval, m_opinionated_species_name);
    CheckSums::CheckSumCombine(retval, m_rated_species_name);
    CheckSums::CheckSumCombine(retval, m_opinion);
    CheckSums::CheckSumCombine(retval, m_target);

    TraceLogger(effects) << "GetCheckSum(SetSpeciesSpeciesOpinion): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> SetSpeciesSpeciesOpinion::Clone() const {
    return std::make_unique<SetSpeciesSpeciesOpinion>(ValueRef::CloneUnique(m_opinionated_species_name),
                                                      ValueRef::CloneUnique(m_rated_species_name),
                                                      ValueRef::CloneUnique(m_opinion),
                                                      m_target);
}


///////////////////////////////////////////////////////////
// CreatePlanet                                          //
///////////////////////////////////////////////////////////
CreatePlanet::CreatePlanet(std::unique_ptr<ValueRef::ValueRef<PlanetType>>&& type,
                           std::unique_ptr<ValueRef::ValueRef<PlanetSize>>&& size,
                           std::unique_ptr<ValueRef::ValueRef<std::string>>&& name,
                           std::vector<std::unique_ptr<Effect>>&& effects_to_apply_after) :
    m_type(std::move(type)),
    m_size(std::move(size)),
    m_name(std::move(name)),
    m_effects_to_apply_after(std::move(effects_to_apply_after))
{}

void CreatePlanet::Execute(ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger(effects) << "CreatePlanet::Execute passed no target object";
        return;
    }
    auto system = context.ContextObjects().getRaw<System>(context.effect_target->SystemID());
    if (!system) {
        ErrorLogger(effects) << "CreatePlanet::Execute couldn't get a System object at which to create the planet";
        return;
    }

    PlanetSize target_size = PlanetSize::INVALID_PLANET_SIZE;
    PlanetType target_type = PlanetType::INVALID_PLANET_TYPE;
    if (context.effect_target->ObjectType() == UniverseObjectType::OBJ_PLANET) {
        auto location_planet = static_cast<const Planet*>(context.effect_target);
        target_size = location_planet->Size();
        target_type = location_planet->Type();
    }

    const ScriptingContext::CurrentValueVariant size_cvv{target_size};
    const ScriptingContext size_context{context, size_cvv};
    PlanetSize size = m_size->Eval(size_context);
    const ScriptingContext::CurrentValueVariant type_cvv{target_type};
    const ScriptingContext type_context{context, type_cvv};
    PlanetType type = m_type->Eval(type_context);
    if (size == PlanetSize::INVALID_PLANET_SIZE || type == PlanetType::INVALID_PLANET_TYPE) {
        ErrorLogger(effects) << "CreatePlanet::Execute got invalid size or type of planet to create...";
        return;
    }

    // determine if and which orbits are available
    if (system->FreeOrbits().empty()) {
        ErrorLogger(effects) << "CreatePlanet::Execute couldn't find any free orbits in system where planet was to be created";
        return;
    }

    auto& universe = context.ContextUniverse();
    auto planet = universe.InsertNew<Planet>(type, size, context.current_turn);
    if (!planet) {
        ErrorLogger(effects) << "CreatePlanet::Execute unable to create new Planet object";
        return;
    }

    system->Insert(planet, System::NO_ORBIT, context.current_turn, context.ContextObjects()); // let system chose an orbit for planet

    std::string name_str;
    if (m_name) {
        name_str = m_name->Eval(context);
        if (m_name->ConstantExpr() && UserStringExists(name_str))
            name_str = UserString(name_str);
    } else {
        name_str = str(FlexibleFormat(UserString("NEW_PLANET_NAME"))
                       % system->Name()
                       % planet->CardinalSuffix(context.ContextObjects()));
    }
    planet->Rename(std::move(name_str));

    // apply after-creation effects
    ScriptingContext local_context{context, ScriptingContext::Target{}, planet.get(), ScriptingContext::DEFAULT_CURRENT_VALUE};
    for (auto& effect : m_effects_to_apply_after) {
        if (effect)
            effect->Execute(local_context);
    }
}

std::string CreatePlanet::Dump(uint8_t ntabs) const {
    std::string retval = DumpIndent(ntabs) + "CreatePlanet";
    if (m_size)
        retval += " size = " + m_size->Dump(ntabs);
    if (m_type)
        retval += " type = " + m_type->Dump(ntabs);
    if (m_name)
        retval += " name = " + m_name->Dump(ntabs);
    return retval + "\n";
}

void CreatePlanet::SetTopLevelContent(const std::string& content_name) {
    if (m_type)
        m_type->SetTopLevelContent(content_name);
    if (m_size)
        m_size->SetTopLevelContent(content_name);
    if (m_name)
        m_name->SetTopLevelContent(content_name);
    for (auto& effect : m_effects_to_apply_after) {
        if (!effect)
            continue;
        effect->SetTopLevelContent(content_name);
    }
}

uint32_t CreatePlanet::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "CreatePlanet");
    CheckSums::CheckSumCombine(retval, m_type);
    CheckSums::CheckSumCombine(retval, m_size);
    CheckSums::CheckSumCombine(retval, m_name);
    CheckSums::CheckSumCombine(retval, m_effects_to_apply_after);

    TraceLogger(effects) << "GetCheckSum(CreatePlanet): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> CreatePlanet::Clone() const {
    return std::make_unique<CreatePlanet>(ValueRef::CloneUnique(m_type),
                                          ValueRef::CloneUnique(m_size),
                                          ValueRef::CloneUnique(m_name),
                                          ValueRef::CloneUnique(m_effects_to_apply_after));
}

///////////////////////////////////////////////////////////
// CreateBuilding                                        //
///////////////////////////////////////////////////////////
CreateBuilding::CreateBuilding(std::unique_ptr<ValueRef::ValueRef<std::string>>&& building_type_name,
                               std::unique_ptr<ValueRef::ValueRef<std::string>>&& name,
                               std::vector<std::unique_ptr<Effect>>&& effects_to_apply_after) :
    m_building_type_name(std::move(building_type_name)),
    m_name(std::move(name)),
    m_effects_to_apply_after(std::move(effects_to_apply_after))
{}

void CreateBuilding::Execute(ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger(effects) << "CreateBuilding::Execute passed no target object";
        return;
    }

    Planet* location = nullptr;
    if (context.effect_target->ObjectType() == UniverseObjectType::OBJ_PLANET) {
        location = static_cast<Planet*>(context.effect_target);

    } else if (context.effect_target->ObjectType() == UniverseObjectType::OBJ_BUILDING) {
        auto* location_building = static_cast<Building*>(context.effect_target);
        location = context.ContextObjects().getRaw<Planet>(location_building->PlanetID());
    }
    if (!location) {
        ErrorLogger(effects) << "CreateBuilding::Execute couldn't get a Planet object at which to create the building";
        return;
    }

    if (!m_building_type_name) {
        ErrorLogger(effects) << "CreateBuilding::Execute has no building type specified!";
        return;
    }

    std::string building_type_name = m_building_type_name->Eval(context);
    const BuildingType* building_type = GetBuildingType(building_type_name);
    if (!building_type) {
        ErrorLogger(effects) << "CreateBuilding::Execute couldn't get building type: " << building_type_name;
        return;
    }

    auto& universe = context.ContextUniverse();
    auto building = universe.InsertNew<Building>(ALL_EMPIRES, std::move(building_type_name),
                                                 ALL_EMPIRES, context.current_turn);
    if (!building) {
        ErrorLogger(effects) << "CreateBuilding::Execute couldn't create building!";
        return;
    }

    location->AddBuilding(building->ID());
    building->SetPlanetID(location->ID());

    building->SetOwner(location->Owner());

    auto system = context.ContextObjects().getRaw<System>(location->SystemID());
    if (system)
        system->Insert(building, System::NO_ORBIT, context.current_turn, context.ContextObjects());

    if (m_name) {
        std::string name_str = m_name->Eval(context);
        if (m_name->ConstantExpr() && UserStringExists(name_str))
            name_str = UserString(name_str);
        building->Rename(std::move(name_str));
    }

    // apply after-creation effects
    ScriptingContext local_context{context, ScriptingContext::Target{}, building.get(), ScriptingContext::DEFAULT_CURRENT_VALUE};
    for (auto& effect : m_effects_to_apply_after) {
        if (effect)
            effect->Execute(local_context);
    }
}

std::string CreateBuilding::Dump(uint8_t ntabs) const {
    std::string retval = DumpIndent(ntabs) + "CreateBuilding";
    if (m_building_type_name)
        retval += " type = " + m_building_type_name->Dump(ntabs);
    if (m_name)
        retval += " name = " + m_name->Dump(ntabs);
    return retval + "\n";
}

void CreateBuilding::SetTopLevelContent(const std::string& content_name) {
    if (m_building_type_name)
        m_building_type_name->SetTopLevelContent(content_name);
    if (m_name)
        m_name->SetTopLevelContent(content_name);
    for (auto& effect : m_effects_to_apply_after) {
        if (!effect)
            continue;
        effect->SetTopLevelContent(content_name);
    }
}

uint32_t CreateBuilding::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "CreateBuilding");
    CheckSums::CheckSumCombine(retval, m_building_type_name);
    CheckSums::CheckSumCombine(retval, m_name);
    CheckSums::CheckSumCombine(retval, m_effects_to_apply_after);

    TraceLogger(effects) << "GetCheckSum(CreateBuilding): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> CreateBuilding::Clone() const {
    return std::make_unique<CreateBuilding>(ValueRef::CloneUnique(m_building_type_name),
                                            ValueRef::CloneUnique(m_name),
                                            ValueRef::CloneUnique(m_effects_to_apply_after));
}


///////////////////////////////////////////////////////////
// CreateShip                                            //
///////////////////////////////////////////////////////////
CreateShip::CreateShip(std::unique_ptr<ValueRef::ValueRef<std::string>>&& predefined_ship_design_name,
                       std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id,
                       std::unique_ptr<ValueRef::ValueRef<std::string>>&& species_name,
                       std::unique_ptr<ValueRef::ValueRef<std::string>>&& ship_name,
                       std::vector<std::unique_ptr<Effect>>&& effects_to_apply_after) :
    m_design_name(std::move(predefined_ship_design_name)),
    m_empire_id(std::move(empire_id)),
    m_species_name(std::move(species_name)),
    m_name(std::move(ship_name)),
    m_effects_to_apply_after(std::move(effects_to_apply_after))
{}

CreateShip::CreateShip(std::unique_ptr<ValueRef::ValueRef<int>>&& ship_design_id,
                       std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id,
                       std::unique_ptr<ValueRef::ValueRef<std::string>>&& species_name,
                       std::unique_ptr<ValueRef::ValueRef<std::string>>&& ship_name,
                       std::vector<std::unique_ptr<Effect>>&& effects_to_apply_after) :
    m_design_id(std::move(ship_design_id)),
    m_empire_id(std::move(empire_id)),
    m_species_name(std::move(species_name)),
    m_name(std::move(ship_name)),
    m_effects_to_apply_after(std::move(effects_to_apply_after))
{}

void CreateShip::Execute(ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger(effects) << "CreateShip::Execute passed null target";
        return;
    }

    auto system = context.ContextObjects().getRaw<System>(context.effect_target->SystemID());
    if (!system) {
        ErrorLogger(effects) << "CreateShip::Execute passed a target not in a system";
        return;
    }

    int design_id = INVALID_DESIGN_ID;
    const ShipDesign* ship_design = nullptr;
    if (m_design_id) {
        design_id = m_design_id->Eval(context);
        if (!context.ContextUniverse().GetShipDesign(design_id)) {
            ErrorLogger(effects) << "CreateShip::Execute couldn't get ship design with id: " << design_id;
            return;
        }
    } else if (m_design_name) {
        std::string design_name = m_design_name->Eval(context);
        ship_design = context.ContextUniverse().GetGenericShipDesign(design_name);
        if (!ship_design) {
            ErrorLogger(effects) << "CreateShip::Execute couldn't get predefined ship design with name " << m_design_name->Dump();
            return;
        }
        design_id = ship_design->ID();
    }
    if (design_id == INVALID_DESIGN_ID) {
        ErrorLogger(effects) << "CreateShip::Execute got invalid ship design id: -1";
        return;
    }

    int empire_id = ALL_EMPIRES;
    std::shared_ptr<Empire> empire;
    if (m_empire_id) {
        empire_id = m_empire_id->Eval(context);
        if (empire_id != ALL_EMPIRES) {
            empire = context.GetEmpire(empire_id);
            if (!empire) {
                ErrorLogger(effects) << "CreateShip::Execute couldn't get empire with id " << empire_id;
                return;
            }
        }
    }

    std::string species_name;
    if (m_species_name) {
        species_name = m_species_name->Eval(context);
        if (!species_name.empty() && !context.species.GetSpecies(species_name)) {
            ErrorLogger(effects) << "CreateShip::Execute couldn't get species with which to create a ship";
            return;
        }
    }

    //// possible future modification: try to put new ship into existing fleet if
    //// ownership with target object's fleet works out (if target is a ship)
    //// attempt to find a
    //auto fleet = std::dynamic_pointer_cast<Fleet>(target);
    //if (!fleet)
    //    if (auto ship = std::dynamic_pointer_cast<const Ship>(target))
    //        fleet = ship->FleetID();
    //// etc.

    auto ship = context.ContextUniverse().InsertNew<Ship>(
        empire_id, design_id, std::move(species_name), context.ContextUniverse(),
        context.species, ALL_EMPIRES, context.current_turn);
    system->Insert(ship, System::NO_ORBIT, context.current_turn, context.ContextObjects());

    if (m_name) {
        std::string name_str = m_name->Eval(context);
        if (m_name->ConstantExpr() && UserStringExists(name_str))
            name_str = UserString(name_str);
        ship->Rename(std::move(name_str));
    } else if (ship->IsMonster(context.ContextUniverse())) {
        ship->Rename(NewMonsterName());
    } else if (empire) {
        ship->Rename(empire->NewShipName());
    } else {
        if (!ship_design)
            ship_design = context.ContextUniverse().GetShipDesign(design_id);
        if (ship_design)
            ship->Rename(ship_design->Name());
    }

    ship->ResetTargetMaxUnpairedMeters();
    ship->ResetPairedActiveMeters();
    ship->SetShipMetersToMax();

    ship->BackPropagateMeters();

    context.ContextUniverse().SetEmpireKnowledgeOfShipDesign(design_id, empire_id);

    CreateNewFleet(system, ship.get(), context);

    // apply after-creation effects
    ScriptingContext local_context{context, ScriptingContext::Target{}, ship.get(), ScriptingContext::DEFAULT_CURRENT_VALUE};
    for (auto& effect : m_effects_to_apply_after) {
        if (effect)
            effect->Execute(local_context);
    }
}

std::string CreateShip::Dump(uint8_t ntabs) const {
    std::string retval = DumpIndent(ntabs) + "CreateShip";
    if (m_design_id)
        retval += " designid = " + m_design_id->Dump(ntabs);
    if (m_design_name)
        retval += " designname = " + m_design_name->Dump(ntabs);
    if (m_empire_id)
        retval += " empire = " + m_empire_id->Dump(ntabs);
    if (m_species_name)
        retval += " species = " + m_species_name->Dump(ntabs);
    if (m_name)
        retval += " name = " + m_name->Dump(ntabs);

    retval += "\n";
    return retval;
}

void CreateShip::SetTopLevelContent(const std::string& content_name) {
    if (m_design_name)
        m_design_name->SetTopLevelContent(content_name);
    if (m_design_id)
        m_design_id->SetTopLevelContent(content_name);
    if (m_empire_id)
        m_empire_id->SetTopLevelContent(content_name);
    if (m_species_name)
        m_species_name->SetTopLevelContent(content_name);
    if (m_name)
        m_name->SetTopLevelContent(content_name);
    for (auto& effect : m_effects_to_apply_after) {
        if (!effect)
            continue;
        effect->SetTopLevelContent(content_name);
    }
}

uint32_t CreateShip::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "CreateShip");
    CheckSums::CheckSumCombine(retval, m_design_name);
    CheckSums::CheckSumCombine(retval, m_design_id);
    CheckSums::CheckSumCombine(retval, m_empire_id);
    CheckSums::CheckSumCombine(retval, m_species_name);
    CheckSums::CheckSumCombine(retval, m_name);
    CheckSums::CheckSumCombine(retval, m_effects_to_apply_after);

    TraceLogger(effects) << "GetCheckSum(CreateShip): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> CreateShip::Clone() const {
    auto retval = std::make_unique<CreateShip>(ValueRef::CloneUnique(m_design_name),
                                               ValueRef::CloneUnique(m_empire_id),
                                               ValueRef::CloneUnique(m_species_name),
                                               ValueRef::CloneUnique(m_name),
                                               ValueRef::CloneUnique(m_effects_to_apply_after));
    retval->m_design_id = ValueRef::CloneUnique(m_design_id);
    return retval;
}


///////////////////////////////////////////////////////////
// CreateField                                           //
///////////////////////////////////////////////////////////
CreateField::CreateField(std::unique_ptr<ValueRef::ValueRef<std::string>>&& field_type_name,
                         std::unique_ptr<ValueRef::ValueRef<double>>&& size,
                         std::unique_ptr<ValueRef::ValueRef<std::string>>&& name,
                         std::vector<std::unique_ptr<Effect>>&& effects_to_apply_after) :
    m_field_type_name(std::move(field_type_name)),
    m_size(std::move(size)),
    m_name(std::move(name)),
    m_effects_to_apply_after(std::move(effects_to_apply_after))
{}

CreateField::CreateField(std::unique_ptr<ValueRef::ValueRef<std::string>>&& field_type_name,
                         std::unique_ptr<ValueRef::ValueRef<double>>&& x,
                         std::unique_ptr<ValueRef::ValueRef<double>>&& y,
                         std::unique_ptr<ValueRef::ValueRef<double>>&& size,
                         std::unique_ptr<ValueRef::ValueRef<std::string>>&& name,
                         std::vector<std::unique_ptr<Effect>>&& effects_to_apply_after) :
    m_field_type_name(std::move(field_type_name)),
    m_x(std::move(x)),
    m_y(std::move(y)),
    m_size(std::move(size)),
    m_name(std::move(name)),
    m_effects_to_apply_after(std::move(effects_to_apply_after))
{}

void CreateField::Execute(ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger(effects) << "CreateField::Execute passed null target";
        return;
    }
    auto target = context.effect_target;

    if (!m_field_type_name)
        return;

    const FieldType* field_type = GetFieldType(m_field_type_name->Eval(context));
    if (!field_type) {
        ErrorLogger(effects) << "CreateField::Execute couldn't get field type with name: " << m_field_type_name->Dump();
        return;
    }

    double size = 10.0;
    if (m_size)
        size = m_size->Eval(context);
    if (size < 1.0) {
        ErrorLogger(effects) << "CreateField::Execute given very small / negative size: " << size << "  ... so resetting to 1.0";
        size = 1.0;
    }
    if (size > 10000) {
        ErrorLogger(effects) << "CreateField::Execute given very large size: " << size << "  ... so resetting to 10000";
        size = 10000;
    }

    double x = 0.0;
    double y = 0.0;
    if (m_x)
        x = m_x->Eval(context);
    else
        x = target->X();
    if (m_y)
        y = m_y->Eval(context);
    else
        y = target->Y();

    auto& universe = context.ContextUniverse();
    auto field = universe.InsertNew<Field>(field_type->Name(), x, y, size, context.current_turn);
    if (!field) {
        ErrorLogger(effects) << "CreateField::Execute couldn't create field!";
        return;
    }

    // if target is a system, and location matches system location, can put
    // field into system
    if (target->ObjectType() == UniverseObjectType::OBJ_SYSTEM) {
        auto system = static_cast<System*>(target);
        if ((!m_y || y == system->Y()) && (!m_x || x == system->X()))
            system->Insert(field, System::NO_ORBIT, context.current_turn, context.ContextObjects());
    }

    if (m_name) {
        auto name_str = m_name->Eval(context);
        if (m_name->ConstantExpr() && UserStringExists(name_str))
            field->Rename(UserString(name_str));
        else
            field->Rename(std::move(name_str));
    } else {
        field->Rename(UserString(field_type->Name()));
    }

    // apply after-creation effects
    ScriptingContext new_field_target_context{context, ScriptingContext::Target{}, field.get(), ScriptingContext::DEFAULT_CURRENT_VALUE};
    for (auto& effect : m_effects_to_apply_after) {
        if (effect)
            effect->Execute(new_field_target_context);
    }
}

std::string CreateField::Dump(uint8_t ntabs) const {
    std::string retval = DumpIndent(ntabs) + "CreateField";
    if (m_field_type_name)
        retval += " type = " + m_field_type_name->Dump(ntabs);
    if (m_x)
        retval += " x = " + m_x->Dump(ntabs);
    if (m_y)
        retval += " y = " + m_y->Dump(ntabs);
    if (m_size)
        retval += " size = " + m_size->Dump(ntabs);
    if (m_name)
        retval += " name = " + m_name->Dump(ntabs);
    retval += "\n";
    return retval;
}

void CreateField::SetTopLevelContent(const std::string& content_name) {
    if (m_field_type_name)
        m_field_type_name->SetTopLevelContent(content_name);
    if (m_x)
        m_x->SetTopLevelContent(content_name);
    if (m_y)
        m_y->SetTopLevelContent(content_name);
    if (m_size)
        m_size->SetTopLevelContent(content_name);
    if (m_name)
        m_name->SetTopLevelContent(content_name);
    for (auto& effect : m_effects_to_apply_after) {
        if (!effect)
            continue;
        effect->SetTopLevelContent(content_name);
    }
}

uint32_t CreateField::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "CreateField");
    CheckSums::CheckSumCombine(retval, m_field_type_name);
    CheckSums::CheckSumCombine(retval, m_x);
    CheckSums::CheckSumCombine(retval, m_y);
    CheckSums::CheckSumCombine(retval, m_size);
    CheckSums::CheckSumCombine(retval, m_name);
    CheckSums::CheckSumCombine(retval, m_effects_to_apply_after);

    TraceLogger(effects) << "GetCheckSum(CreateField): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> CreateField::Clone() const {
    return std::make_unique<CreateField>(ValueRef::CloneUnique(m_field_type_name),
                                         ValueRef::CloneUnique(m_x),
                                         ValueRef::CloneUnique(m_y),
                                         ValueRef::CloneUnique(m_size),
                                         ValueRef::CloneUnique(m_name),
                                         ValueRef::CloneUnique(m_effects_to_apply_after));
}

///////////////////////////////////////////////////////////
// CreateSystem                                          //
///////////////////////////////////////////////////////////
CreateSystem::CreateSystem(std::unique_ptr<ValueRef::ValueRef< ::StarType>>&& type,
                           std::unique_ptr<ValueRef::ValueRef<double>>&& x,
                           std::unique_ptr<ValueRef::ValueRef<double>>&& y,
                           std::unique_ptr<ValueRef::ValueRef<std::string>>&& name,
                           std::vector<std::unique_ptr<Effect>>&& effects_to_apply_after) :
    m_type(std::move(type)),
    m_x(std::move(x)),
    m_y(std::move(y)),
    m_name(std::move(name)),
    m_effects_to_apply_after(std::move(effects_to_apply_after))
{
    DebugLogger(effects) << "Effect System created 1";
}

CreateSystem::CreateSystem(std::unique_ptr<ValueRef::ValueRef<double>>&& x,
                           std::unique_ptr<ValueRef::ValueRef<double>>&& y,
                           std::unique_ptr<ValueRef::ValueRef<std::string>>&& name,
                           std::vector<std::unique_ptr<Effect>>&& effects_to_apply_after) :
    m_x(std::move(x)),
    m_y(std::move(y)),
    m_name(std::move(name)),
    m_effects_to_apply_after(std::move(effects_to_apply_after))
{
    DebugLogger(effects) << "Effect System created 2";
}

void CreateSystem::Execute(ScriptingContext& context) const {
    // pick a star type
    StarType star_type = StarType::STAR_NONE;
    if (m_type) {
        star_type = m_type->Eval(context);
    } else {
        int max_type_idx = int(StarType::NUM_STAR_TYPES) - 1;
        int type_idx = RandInt(0, max_type_idx);
        star_type = StarType(type_idx);
    }

    // pick location
    double x = 0.0;
    double y = 0.0;
    if (m_x)
        x = m_x->Eval(context);
    if (m_y)
        y = m_y->Eval(context);

    std::string name_str;
    if (m_name) {
        name_str = m_name->Eval(context);
        if (m_name->ConstantExpr() && UserStringExists(name_str))
            name_str = UserString(name_str);
    } else {
        name_str = GenerateSystemName(context.ContextObjects());
    }

    auto& universe = context.ContextUniverse();
    auto system = universe.InsertNew<System>(star_type, name_str, x, y, context.current_turn);
    if (!system) {
        ErrorLogger(effects) << "CreateSystem::Execute couldn't create system!";
        return;
    }

    // apply after-creation effects
    ScriptingContext system_target_context{context, ScriptingContext::Target{}, system.get(), ScriptingContext::DEFAULT_CURRENT_VALUE};
    for (auto& effect : m_effects_to_apply_after) {
        if (effect)
            effect->Execute(system_target_context);
    }
}

std::string CreateSystem::Dump(uint8_t ntabs) const {
    std::string retval = DumpIndent(ntabs) + "CreateSystem";
    if (m_type)
        retval += " type = " + m_type->Dump(ntabs);
    if (m_x)
        retval += " x = " + m_x->Dump(ntabs);
    if (m_y)
        retval += " y = " + m_y->Dump(ntabs);
    if (m_name)
        retval += " name = " + m_name->Dump(ntabs);
    retval += "\n";
    return retval;
}

void CreateSystem::SetTopLevelContent(const std::string& content_name) {
    if (m_x)
        m_x->SetTopLevelContent(content_name);
    if (m_y)
        m_y->SetTopLevelContent(content_name);
    if (m_type)
        m_type->SetTopLevelContent(content_name);
    if (m_name)
        m_name->SetTopLevelContent(content_name);
    for (auto& effect : m_effects_to_apply_after) {
        if (!effect)
            continue;
        effect->SetTopLevelContent(content_name);
    }
}

uint32_t CreateSystem::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "CreateSystem");
    CheckSums::CheckSumCombine(retval, m_type);
    CheckSums::CheckSumCombine(retval, m_x);
    CheckSums::CheckSumCombine(retval, m_y);
    CheckSums::CheckSumCombine(retval, m_name);
    CheckSums::CheckSumCombine(retval, m_effects_to_apply_after);

    TraceLogger(effects) << "GetCheckSum(CreateSystem): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> CreateSystem::Clone() const {
    return std::make_unique<CreateSystem>(ValueRef::CloneUnique(m_type),
                                          ValueRef::CloneUnique(m_x),
                                          ValueRef::CloneUnique(m_y),
                                          ValueRef::CloneUnique(m_name),
                                          ValueRef::CloneUnique(m_effects_to_apply_after));
}


///////////////////////////////////////////////////////////
// Destroy                                               //
///////////////////////////////////////////////////////////
void Destroy::Execute(ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger(effects) << "Destroy::Execute passed no target object";
        return;
    }

    int source_id = INVALID_OBJECT_ID;
    if (context.source)
        source_id = context.source->ID();

    context.ContextUniverse().EffectDestroy(context.effect_target->ID(), source_id);
}

std::string Destroy::Dump(uint8_t ntabs) const
{ return DumpIndent(ntabs) + "Destroy\n"; }

uint32_t Destroy::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "Destroy");

    TraceLogger(effects) << "GetCheckSum(Destroy): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> Destroy::Clone() const
{ return std::make_unique<Destroy>(); }


///////////////////////////////////////////////////////////
// AddSpecial                                            //
///////////////////////////////////////////////////////////
AddSpecial::AddSpecial(std::string& name, float capacity) :
    m_name(std::make_unique<ValueRef::Constant<std::string>>(std::move(name))),
    m_capacity(std::make_unique<ValueRef::Constant<double>>(capacity))
{}

AddSpecial::AddSpecial(std::unique_ptr<ValueRef::ValueRef<std::string>>&& name,
                       std::unique_ptr<ValueRef::ValueRef<double>>&& capacity) :
    m_name(std::move(name)),
    m_capacity(std::move(capacity))
{}

void AddSpecial::Execute(ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger(effects) << "AddSpecial::Execute passed no target object";
        return;
    }

    std::string name = (m_name ? m_name->Eval(context) : "");

    float initial_capacity = context.effect_target->SpecialCapacity(name);  // returns 0.0f if no such special yet present
    float capacity = initial_capacity;
    if (m_capacity) {
        const ScriptingContext::CurrentValueVariant cvv{capacity};
        const ScriptingContext capacity_context{context, cvv};
        capacity = static_cast<float>(m_capacity->Eval(capacity_context));
    }

    context.effect_target->SetSpecialCapacity(std::move(name), capacity, context.current_turn);
}

std::string AddSpecial::Dump(uint8_t ntabs) const {
    return DumpIndent(ntabs) + "AddSpecial name = " +  (m_name ? m_name->Dump(ntabs) : "") +
        " capacity = " + (m_capacity ? m_capacity->Dump(ntabs) : "0.0") +  "\n";
}

void AddSpecial::SetTopLevelContent(const std::string& content_name) {
    if (m_name)
        m_name->SetTopLevelContent(content_name);
    if (m_capacity)
        m_capacity->SetTopLevelContent(content_name);
}

uint32_t AddSpecial::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "AddSpecial");
    CheckSums::CheckSumCombine(retval, m_name);
    CheckSums::CheckSumCombine(retval, m_capacity);

    TraceLogger(effects) << "GetCheckSum(AddSpecial): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> AddSpecial::Clone() const {
    return std::make_unique<AddSpecial>(ValueRef::CloneUnique(m_name),
                                        ValueRef::CloneUnique(m_capacity));
}


///////////////////////////////////////////////////////////
// RemoveSpecial                                         //
///////////////////////////////////////////////////////////
RemoveSpecial::RemoveSpecial(std::string& name) :
    m_name(std::make_unique<ValueRef::Constant<std::string>>(std::move(name)))
{}

RemoveSpecial::RemoveSpecial(std::unique_ptr<ValueRef::ValueRef<std::string>>&& name) :
    m_name(std::move(name))
{}

void RemoveSpecial::Execute(ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger(effects) << "RemoveSpecial::Execute passed no target object";
        return;
    }

    std::string name = (m_name ? m_name->Eval(context) : "");
    context.effect_target->RemoveSpecial(name);
}

std::string RemoveSpecial::Dump(uint8_t ntabs) const {
    return DumpIndent(ntabs) + "RemoveSpecial name = " +  (m_name ? m_name->Dump(ntabs) : "") + "\n";
}

void RemoveSpecial::SetTopLevelContent(const std::string& content_name) {
    if (m_name)
        m_name->SetTopLevelContent(content_name);
}

uint32_t RemoveSpecial::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "RemoveSpecial");
    CheckSums::CheckSumCombine(retval, m_name);

    TraceLogger(effects) << "GetCheckSum(RemoveSpecial): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> RemoveSpecial::Clone() const
{ return std::make_unique<RemoveSpecial>(ValueRef::CloneUnique(m_name)); }


///////////////////////////////////////////////////////////
// AddStarlanes                                          //
///////////////////////////////////////////////////////////
AddStarlanes::AddStarlanes(std::unique_ptr<Condition::Condition>&& other_lane_endpoint_condition) :
    m_other_lane_endpoint_condition(std::move(other_lane_endpoint_condition))
{}

namespace {
    constexpr auto not_null = [](const auto* o) -> bool { return o; };
}

void AddStarlanes::Execute(ScriptingContext& context) const {
    const auto to_system = [&context](UniverseObject* o) -> System* {
        if (o->ObjectType() == UniverseObjectType::OBJ_SYSTEM)
            return static_cast<System*>(o);
        return context.ContextObjects().getRaw<System>(o->SystemID()); // may be nullptr
    };

    // get target system
    if (!context.effect_target) {
        ErrorLogger(effects) << "AddStarlanes::Execute passed no target object";
        return;
    }
    auto* target_system = to_system(context.effect_target);
    if (!target_system)
        return; // nothing to do!

    // from endpoints condition, get objects whose systems should be
    // connected to the source system
    TargetSet endpoint_objects = m_other_lane_endpoint_condition->Eval(context);

    // early exit if there are no valid locations
    if (endpoint_objects.empty())
        return; // nothing to do!

    // get systems containing at least one endpoint object
    std::vector<System*> endpoint_systems;
    endpoint_systems.reserve(endpoint_objects.size());
    auto end_sys_rng = endpoint_objects | range_filter(not_null)
        | range_transform(to_system) | range_filter(not_null);
    range_copy(end_sys_rng, std::back_inserter(endpoint_systems));

    // ensure uniqueness of results
    std::sort(endpoint_systems.begin(), endpoint_systems.end());
    const auto it = std::unique(endpoint_systems.begin(), endpoint_systems.end());
    endpoint_systems.resize(std::distance(endpoint_systems.begin(), it));

    // add starlanes from target to endpoint systems
    for (auto endpoint_system : endpoint_systems) {
        target_system->AddStarlane(endpoint_system->ID());
        endpoint_system->AddStarlane(target_system->ID());
    }
}

std::string AddStarlanes::Dump(uint8_t ntabs) const
{ return DumpIndent(ntabs) + "AddStarlanes endpoints = " + m_other_lane_endpoint_condition->Dump(ntabs) + "\n"; }

void AddStarlanes::SetTopLevelContent(const std::string& content_name) {
    if (m_other_lane_endpoint_condition)
        m_other_lane_endpoint_condition->SetTopLevelContent(content_name);
}

uint32_t AddStarlanes::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "AddStarlanes");
    CheckSums::CheckSumCombine(retval, m_other_lane_endpoint_condition);

    TraceLogger(effects) << "GetCheckSum(AddStarlanes): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> AddStarlanes::Clone() const
{ return std::make_unique<AddStarlanes>(ValueRef::CloneUnique(m_other_lane_endpoint_condition)); }


///////////////////////////////////////////////////////////
// RemoveStarlanes                                       //
///////////////////////////////////////////////////////////
RemoveStarlanes::RemoveStarlanes(std::unique_ptr<Condition::Condition>&& other_lane_endpoint_condition) :
    m_other_lane_endpoint_condition(std::move(other_lane_endpoint_condition))
{}

void RemoveStarlanes::Execute(ScriptingContext& context) const {
    // get target system
    if (!context.effect_target) {
        ErrorLogger(effects) << "AddStarlanes::Execute passed no target object";
        return;
    }
    auto target_system = dynamic_cast<System*>(context.effect_target);
    if (!target_system)
        target_system = context.ContextObjects().getRaw<System>(context.effect_target->SystemID());
    if (!target_system)
        return; // nothing to do!
    const int target_system_id = target_system->ID();

    // get other endpoint systems...

    // apply endpoints condition to determine objects whose systems should be
    // connected to the source system
    const auto endpoint_objects = m_other_lane_endpoint_condition->Eval(std::as_const(context));

    // get system IDs of those objects
    std::vector<int> endpoint_system_ids;
    endpoint_system_ids.reserve(endpoint_objects.size());
    std::transform(endpoint_objects.begin(), endpoint_objects.end(), std::back_inserter(endpoint_system_ids),
                   [](const UniverseObject* obj) { return obj ? obj->SystemID() : INVALID_OBJECT_ID; });
    // uniquify
    std::sort(endpoint_system_ids.begin(), endpoint_system_ids.end());
    auto unique_it = std::unique(endpoint_system_ids.begin(), endpoint_system_ids.end());
    endpoint_system_ids.erase(unique_it, endpoint_system_ids.end());
    // get all those systems
    auto endpoint_systems = context.ContextObjects().findRaw<System>(endpoint_system_ids);

    // remove starlanes from target to endpoint systems
    for (auto* endpoint_system : endpoint_systems) {
        target_system->RemoveStarlane(endpoint_system->ID());
        endpoint_system->RemoveStarlane(target_system_id);
    }
}

std::string RemoveStarlanes::Dump(uint8_t ntabs) const
{ return DumpIndent(ntabs) + "RemoveStarlanes endpoints = " + m_other_lane_endpoint_condition->Dump(ntabs) + "\n"; }

void RemoveStarlanes::SetTopLevelContent(const std::string& content_name) {
    if (m_other_lane_endpoint_condition)
        m_other_lane_endpoint_condition->SetTopLevelContent(content_name);
}

uint32_t RemoveStarlanes::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "RemoveStarlanes");
    CheckSums::CheckSumCombine(retval, m_other_lane_endpoint_condition);

    TraceLogger(effects) << "GetCheckSum(RemoveStarlanes): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> RemoveStarlanes::Clone() const
{ return std::make_unique<RemoveStarlanes>(ValueRef::CloneUnique(m_other_lane_endpoint_condition)); }


///////////////////////////////////////////////////////////
// SetStarType                                           //
///////////////////////////////////////////////////////////
SetStarType::SetStarType(std::unique_ptr<ValueRef::ValueRef<StarType>>&& type) :
    m_type(std::move(type))
{}

void SetStarType::Execute(ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger(effects) << "SetStarType::Execute given no target object";
        return;
    }
    if (context.effect_target->ObjectType() == UniverseObjectType::OBJ_SYSTEM) {
        auto s = static_cast<System*>(context.effect_target);
        const ScriptingContext::CurrentValueVariant cvv{s->GetStarType()};
        const ScriptingContext type_context{context, cvv};
        s->SetStarType(m_type->Eval(type_context));
    } else {
        ErrorLogger(effects) << "SetStarType::Execute given a non-system target";
    }
}

std::string SetStarType::Dump(uint8_t ntabs) const
{ return DumpIndent(ntabs) + "SetStarType type = " + m_type->Dump(ntabs) + "\n"; }

void SetStarType::SetTopLevelContent(const std::string& content_name) {
    if (m_type)
        m_type->SetTopLevelContent(content_name);
}

uint32_t SetStarType::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "SetStarType");
    CheckSums::CheckSumCombine(retval, m_type);

    TraceLogger(effects) << "GetCheckSum(SetStarType): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> SetStarType::Clone() const
{ return std::make_unique<SetStarType>(ValueRef::CloneUnique(m_type)); }


///////////////////////////////////////////////////////////
// MoveTo                                                //
///////////////////////////////////////////////////////////
MoveTo::MoveTo(std::unique_ptr<Condition::Condition>&& location_condition) :
    m_location_condition(std::move(location_condition))
{}

void MoveTo::Execute(ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger(effects) << "MoveTo::Execute given no target object";
        return;
    }

    Universe& universe = context.ContextUniverse();
    ObjectMap& objects = context.ContextObjects();

    // apply location condition to determine valid location to move target to
    TargetSet valid_locations = m_location_condition->Eval(context);

    // early exit if there are no valid locations - can't move anything if there's nowhere to move to
    if (valid_locations.empty())
        return;

    // "randomly" pick a destination
    auto destination = valid_locations.front();
    if (!destination) {
        ErrorLogger(effects) << "MoveTo::Execute got null destination!";
        return;
    }

    // get previous system from which to remove object if necessary
    auto old_sys = objects.getRaw<System>(context.effect_target->SystemID());

    auto get_fleet = [](auto* destination, ObjectMap& objs) -> Fleet* {
        const auto obj_type = destination->ObjectType();
        if (obj_type == UniverseObjectType::OBJ_FLEET)
            return static_cast<Fleet*>(destination);
        if (obj_type == UniverseObjectType::OBJ_SHIP) {
            auto ship = static_cast<Ship*>(destination);
            return objs.getRaw<Fleet>(ship->FleetID());
        }
        return nullptr;
    };

    auto get_planet = [](auto* destination, ObjectMap& objs) -> Planet* {
        const auto obj_type = destination->ObjectType();
        if (obj_type == UniverseObjectType::OBJ_PLANET)
            return static_cast<Planet*>(destination);
        if (obj_type == UniverseObjectType::OBJ_BUILDING) {
            auto building = static_cast<Building*>(destination);
            return objs.getRaw<Planet>(building->PlanetID());
        }
        return nullptr;
    };

    // do the moving...
    if (context.effect_target->ObjectType() == UniverseObjectType::OBJ_FLEET) {
        auto fleet = static_cast<Fleet*>(context.effect_target);
        // fleets can be inserted into the system that contains the destination
        // object (or the destination object itself if it is a system)
        if (auto dest_system = objects.getRaw<System>(destination->SystemID())) {
            if (fleet->SystemID() != dest_system->ID()) {
                // remove fleet from old system, put into new system
                if (old_sys)
                    old_sys->Remove(fleet->ID());
                dest_system->Insert(fleet, System::NO_ORBIT, context.current_turn, objects);

                // also move ships of fleet
                for (auto* ship : objects.findRaw<Ship>(fleet->ShipIDs())) {
                    if (old_sys)
                        old_sys->Remove(ship->ID());
                    dest_system->Insert(ship, System::NO_ORBIT, context.current_turn, objects);
                }

                ExploreSystem(dest_system->ID(), fleet->Owner(), context);
                UpdateFleetRoute(fleet, INVALID_OBJECT_ID, INVALID_OBJECT_ID, context);  // inserted into dest_system, so next and previous systems are invalid objects
            }

            // if old and new systems are the same, and destination is that
            // system, don't need to do anything

        } else {
            // move fleet to new location
            if (old_sys)
                old_sys->Remove(fleet->ID());
            fleet->SetSystem(INVALID_OBJECT_ID);
            fleet->MoveTo(destination);

            // also move ships of fleet
            for (auto* ship : objects.findRaw<Ship>(fleet->ShipIDs())) {
                if (old_sys)
                    old_sys->Remove(ship->ID());
                ship->SetSystem(INVALID_OBJECT_ID);
                ship->MoveTo(destination);
            }


            // fleet has been moved to a location that is not a system.
            // Presumably this will be located on a starlane between two other
            // systems, which may or may not have been explored.  Regardless,
            // the fleet needs to be given a new next and previous system so it
            // can move into a system, or can be ordered to a new location, and
            // so that it won't try to move off of starlanes towards some other
            // system from its current location (if it was heading to another
            // system) and so it won't be stuck in the middle of a starlane,
            // unable to move (if it wasn't previously moving)

            // if destination object is a fleet or is part of a fleet, can use
            // that fleet's previous and next systems to get valid next and
            // previous systems for the target fleet.
            if (auto dest_fleet = get_fleet(destination, objects)) {
                UpdateFleetRoute(fleet, dest_fleet->NextSystemID(), dest_fleet->PreviousSystemID(), context);

            } else {
                // TODO: need to do something else to get updated previous/next
                // systems if the destination is a field.
                ErrorLogger(effects) << "MoveTo::Execute couldn't find a way to set the previous and next systems for the target fleet!";
            }
        }

    } else if (context.effect_target->ObjectType() == UniverseObjectType::OBJ_SHIP) {
        auto ship = static_cast<Ship*>(context.effect_target);
        // TODO: make sure colonization doesn't interfere with this effect, and vice versa

        // is destination a ship/fleet ?
        auto dest_fleet = get_fleet(destination, objects);
        if (dest_fleet)
            if (dest_fleet->ID() == ship->FleetID())
                return; // already in destination fleet. nothing to do.


        bool same_owners = ship->Owner() == destination->Owner();
        int dest_sys_id = destination->SystemID();
        int ship_sys_id = ship->SystemID();


        if (ship_sys_id != dest_sys_id) {
            // ship is moving to a different system.

            // remove ship from old system
            if (old_sys) {
                old_sys->Remove(ship->ID());
                ship->SetSystem(INVALID_OBJECT_ID);
            }

            if (auto new_sys = objects.getRaw<System>(dest_sys_id)) {
                // ship is moving to a new system. insert it.
                new_sys->Insert(ship, System::NO_ORBIT, context.current_turn, context.ContextObjects());
            } else {
                // ship is moving to a non-system location. move it there.
                ship->MoveTo(dest_fleet);
            }

            // may create a fleet for ship below...
        }

        auto old_fleet = objects.getRaw<Fleet>(ship->FleetID());

        if (dest_fleet && same_owners) {
            // ship is moving to a different fleet owned by the same empire, so
            // can be inserted into it.
            if (old_fleet)
                old_fleet->RemoveShips({ship->ID()});
            dest_fleet->AddShips({ship->ID()});
            ship->SetFleetID(dest_fleet->ID());

        } else if (dest_sys_id == ship_sys_id && dest_sys_id != INVALID_OBJECT_ID) {
            // ship is moving to the system it is already in, but isn't being or
            // can't be moved into a specific fleet, so the ship can be left in
            // its current fleet and at its current location

        } else if (destination->X() == ship->X() && destination->Y() == ship->Y()) {
            // ship is moving to the same location it's already at, but isn't
            // being or can't be moved to a specific fleet, so the ship can be
            // left in its current fleet and at its current location

        } else {
            // need to create a new fleet for ship

            // if ship is armed use old fleet's aggression. otherwise use auto-determined aggression
            auto aggr = old_fleet && ship->IsArmed(context) ? old_fleet->Aggression() : FleetAggression::INVALID_FLEET_AGGRESSION;

            if (auto dest_system = objects.getRaw<System>(dest_sys_id)) {
                // creates new fleet, inserts fleet into system and ship into fleet
                CreateNewFleet(dest_system, ship, context, aggr);
                ExploreSystem(dest_sys_id, ship->Owner(), context);

            } else {
                // creates new fleet and inserts ship into fleet
                CreateNewFleet(destination->X(), destination->Y(), ship, context, aggr);
            }
        }

        if (old_fleet && old_fleet->Empty()) {
            old_sys->Remove(old_fleet->ID());
            universe.EffectDestroy(old_fleet->ID(), INVALID_OBJECT_ID); // no particular object destroyed this fleet
        }

    } else if (context.effect_target->ObjectType() == UniverseObjectType::OBJ_PLANET) {
        auto planet = static_cast<Planet*>(context.effect_target);
        // planets need to be located in systems, so get system that contains destination object

        auto dest_system = objects.getRaw<System>(destination->SystemID());
        if (!dest_system)
            return; // can't move a planet to a non-system

        if (planet->SystemID() == dest_system->ID())
            return; // planet already at destination

        if (dest_system->FreeOrbits().empty())
            return; // no room for planet at destination

        if (old_sys)
            old_sys->Remove(planet->ID());
        dest_system->Insert(planet, System::NO_ORBIT, context.current_turn, objects); // let system pick an orbit

        // also insert buildings of planet into system.
        for (auto* building : objects.findRaw<Building>(planet->BuildingIDs())) {
            if (old_sys)
                old_sys->Remove(building->ID());
            dest_system->Insert(building, System::NO_ORBIT, context.current_turn, objects);
        }

        // buildings planet should be unchanged by move, as should planet's
        // records of its buildings

        ExploreSystem(dest_system->ID(), planet->Owner(), context);


    } else if (context.effect_target->ObjectType() == UniverseObjectType::OBJ_BUILDING) {
        auto building = static_cast<Building*>(context.effect_target);
        // buildings need to be located on planets, so if destination is a
        // planet, insert building into it, or attempt to get the planet on
        // which the destination object is located and insert target building
        // into that
        auto* dest_planet = get_planet(destination, objects);
        if (!dest_planet)
            return;

        if (dest_planet->ID() == building->PlanetID())
            return; // nothing to do

        auto dest_system = objects.getRaw<System>(destination->SystemID());
        if (!dest_system)
            return;

        // remove building from old planet / system, add to new planet / system
        if (old_sys)
            old_sys->Remove(building->ID());
        building->SetSystem(INVALID_OBJECT_ID);

        if (auto old_planet = objects.getRaw<Planet>(building->PlanetID()))
            old_planet->RemoveBuilding(building->ID());

        dest_planet->AddBuilding(building->ID());
        building->SetPlanetID(dest_planet->ID());

        dest_system->Insert(building, System::NO_ORBIT, context.current_turn, objects);
        ExploreSystem(dest_system->ID(), building->Owner(), context);


    } else if (context.effect_target->ObjectType() == UniverseObjectType::OBJ_SYSTEM) {
        auto system = static_cast<System*>(context.effect_target);
        if (destination->SystemID() != INVALID_OBJECT_ID) {
            // TODO: merge systems ... ?
            return;
        }

        // move target system to new destination, and insert destination object
        // and related objects into system
        system->MoveTo(destination);

        if (destination->ObjectType() == UniverseObjectType::OBJ_FIELD)
            system->Insert(destination, System::NO_ORBIT, context.current_turn, objects);

        // find fleets / ships at destination location and insert into system
        for (auto* obj : objects.allRaw<Fleet>()) {
            if (obj->X() == system->X() && obj->Y() == system->Y())
                system->Insert(obj, System::NO_ORBIT, context.current_turn, objects);
        }

        for (auto* obj : objects.allRaw<Ship>()) {
            if (obj->X() == system->X() && obj->Y() == system->Y())
                system->Insert(obj, System::NO_ORBIT, context.current_turn, objects);
        }


    } else if (context.effect_target->ObjectType() == UniverseObjectType::OBJ_FIELD) {
        auto field = static_cast<Field*>(context.effect_target);
        if (old_sys)
            old_sys->Remove(field->ID());
        field->SetSystem(INVALID_OBJECT_ID);
        field->MoveTo(destination);
        if (destination->ObjectType() == UniverseObjectType::OBJ_SYSTEM) {
            auto dest_system = static_cast<System*>(destination);
            dest_system->Insert(field, System::NO_ORBIT, context.current_turn, objects);
        }
    }
}

std::string MoveTo::Dump(uint8_t ntabs) const
{ return DumpIndent(ntabs) + "MoveTo destination = " + m_location_condition->Dump(ntabs) + "\n"; }

void MoveTo::SetTopLevelContent(const std::string& content_name) {
    if (m_location_condition)
        m_location_condition->SetTopLevelContent(content_name);
}

uint32_t MoveTo::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "MoveTo");
    CheckSums::CheckSumCombine(retval, m_location_condition);

    TraceLogger(effects) << "GetCheckSum(MoveTo): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> MoveTo::Clone() const
{ return std::make_unique<MoveTo>(ValueRef::CloneUnique(m_location_condition)); }


///////////////////////////////////////////////////////////
// MoveInOrbit                                           //
///////////////////////////////////////////////////////////
MoveInOrbit::MoveInOrbit(std::unique_ptr<ValueRef::ValueRef<double>>&& speed,
                         std::unique_ptr<Condition::Condition>&& focal_point_condition) :
    m_speed(std::move(speed)),
    m_focal_point_condition(std::move(focal_point_condition))
{}

MoveInOrbit::MoveInOrbit(std::unique_ptr<ValueRef::ValueRef<double>>&& speed,
                         std::unique_ptr<ValueRef::ValueRef<double>>&& focus_x,
                         std::unique_ptr<ValueRef::ValueRef<double>>&& focus_y) :
    m_speed(std::move(speed)),
    m_focus_x(std::move(focus_x)),
    m_focus_y(std::move(focus_y))
{}

void MoveInOrbit::Execute(ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger(effects) << "MoveInOrbit::Execute given no target object";
        return;
    }
    auto* target = context.effect_target;

    double focus_x = 0.0, focus_y = 0.0, speed = 1.0;
    if (m_focus_x) {
        const ScriptingContext::CurrentValueVariant cvv{target->X()};
        const ScriptingContext x_context{context, cvv};
        focus_x = m_focus_x->Eval(x_context);
    }
    if (m_focus_y) {
        const ScriptingContext::CurrentValueVariant cvv{target->Y()};
        const ScriptingContext y_context{context, cvv};
        focus_y = m_focus_y->Eval(y_context);
    }
    if (m_speed)
        speed = m_speed->Eval(context);
    if (speed == 0.0)
        return;
    if (m_focal_point_condition) {
        Condition::ObjectSet matches = m_focal_point_condition->Eval(std::as_const(context));
        if (matches.empty())
            return;
        const auto* focus_object = matches.front();
        focus_x = focus_object->X();
        focus_y = focus_object->Y();
    }

    double focus_to_target_x = target->X() - focus_x;
    double focus_to_target_y = target->Y() - focus_y;
    double focus_to_target_radius = std::sqrt(focus_to_target_x * focus_to_target_x +
                                              focus_to_target_y * focus_to_target_y);
    if (focus_to_target_radius < 1.0)
        return;    // don't move objects that are too close to focus

    double angle_radians = atan2(focus_to_target_y, focus_to_target_x);
    double angle_increment_radians = speed / focus_to_target_radius;
    double new_angle_radians = angle_radians + angle_increment_radians;

    double new_x = focus_x + focus_to_target_radius * cos(new_angle_radians);
    double new_y = focus_y + focus_to_target_radius * sin(new_angle_radians);

    if (target->X() == new_x && target->Y() == new_y)
        return;

    auto old_sys = context.ContextObjects().getRaw<System>(target->SystemID());

    if (auto system = dynamic_cast<System*>(target)) {
        system->MoveTo(new_x, new_y);
        return;

    } else if (auto fleet = dynamic_cast<Fleet*>(target)) {
        if (old_sys)
            old_sys->Remove(fleet->ID());
        fleet->SetSystem(INVALID_OBJECT_ID);
        fleet->MoveTo(new_x, new_y);
        UpdateFleetRoute(fleet, INVALID_OBJECT_ID, INVALID_OBJECT_ID, context);

        for (auto* ship : context.ContextObjects().findRaw<Ship>(fleet->ShipIDs())) {
            if (old_sys)
                old_sys->Remove(ship->ID());
            ship->SetSystem(INVALID_OBJECT_ID);
            ship->MoveTo(new_x, new_y);
        }
        return;

    } else if (auto ship = dynamic_cast<Ship*>(target)) {
        if (old_sys)
            old_sys->Remove(ship->ID());
        ship->SetSystem(INVALID_OBJECT_ID);

        auto old_fleet = context.ContextObjects().getRaw<Fleet>(ship->FleetID());
        if (old_fleet) {
            old_fleet->RemoveShips({ship->ID()});
            if (old_fleet->Empty()) {
                old_sys->Remove(old_fleet->ID());
                context.ContextUniverse().EffectDestroy(old_fleet->ID(), INVALID_OBJECT_ID);    // no object in particular destroyed this fleet
            }
        }

        ship->SetFleetID(INVALID_OBJECT_ID);
        ship->MoveTo(new_x, new_y);

        // creates new fleet and inserts ship into fleet
        CreateNewFleet(new_x, new_y, ship, context);
        return;

    } else if (auto field = dynamic_cast<Field*>(target)) {
        if (old_sys)
            old_sys->Remove(field->ID());
        field->SetSystem(INVALID_OBJECT_ID);
        field->MoveTo(new_x, new_y);
    }
    // don't move planets or buildings, as these can't exist outside of systems
}

std::string MoveInOrbit::Dump(uint8_t ntabs) const {
    if (m_focal_point_condition)
        return DumpIndent(ntabs) + "MoveInOrbit around = " + m_focal_point_condition->Dump(ntabs) + "\n";
    else if (m_focus_x && m_focus_y)
        return DumpIndent(ntabs) + "MoveInOrbit x = " + m_focus_x->Dump(ntabs) + " y = " + m_focus_y->Dump(ntabs) + "\n";
    else
        return DumpIndent(ntabs) + "MoveInOrbit";
}

void MoveInOrbit::SetTopLevelContent(const std::string& content_name) {
    if (m_speed)
        m_speed->SetTopLevelContent(content_name);
    if (m_focal_point_condition)
        m_focal_point_condition->SetTopLevelContent(content_name);
    if (m_focus_x)
        m_focus_x->SetTopLevelContent(content_name);
    if (m_focus_y)
        m_focus_y->SetTopLevelContent(content_name);
}

uint32_t MoveInOrbit::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "MoveInOrbit");
    CheckSums::CheckSumCombine(retval, m_speed);
    CheckSums::CheckSumCombine(retval, m_focal_point_condition);
    CheckSums::CheckSumCombine(retval, m_focus_x);
    CheckSums::CheckSumCombine(retval, m_focus_y);

    TraceLogger(effects) << "GetCheckSum(MoveInOrbit): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> MoveInOrbit::Clone() const {
    auto retval = std::make_unique<MoveInOrbit>(ValueRef::CloneUnique(m_speed),
                                                ValueRef::CloneUnique(m_focus_x),
                                                ValueRef::CloneUnique(m_focus_y));
    retval->m_focal_point_condition = ValueRef::CloneUnique(m_focal_point_condition);
    return retval;
}


///////////////////////////////////////////////////////////
// MoveTowards                                           //
///////////////////////////////////////////////////////////
MoveTowards::MoveTowards(std::unique_ptr<ValueRef::ValueRef<double>>&& speed,
                         std::unique_ptr<Condition::Condition>&& dest_condition) :
    m_speed(std::move(speed)),
    m_dest_condition(std::move(dest_condition))
{}

MoveTowards::MoveTowards(std::unique_ptr<ValueRef::ValueRef<double>>&& speed,
                         std::unique_ptr<ValueRef::ValueRef<double>>&& dest_x,
                         std::unique_ptr<ValueRef::ValueRef<double>>&& dest_y) :
    m_speed(std::move(speed)),
    m_dest_x(std::move(dest_x)),
    m_dest_y(std::move(dest_y))
{}

void MoveTowards::Execute(ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger(effects) << "MoveTowards::Execute given no target object";
        return;
    }
    auto target = context.effect_target;

    double dest_x = 0.0, dest_y = 0.0, speed = 1.0;
    if (m_dest_x) {
        const ScriptingContext::CurrentValueVariant cvv{target->X()};
        const ScriptingContext x_context{context, cvv};
        dest_x = m_dest_x->Eval(x_context);
    }
    if (m_dest_y) {
        const ScriptingContext::CurrentValueVariant cvv{target->Y()};
        const ScriptingContext y_context{context, cvv};
        dest_y = m_dest_y->Eval(y_context);
    }
    if (m_speed)
        speed = m_speed->Eval(context);
    if (speed == 0.0)
        return;
    if (m_dest_condition) {
        Condition::ObjectSet matches = m_dest_condition->Eval(std::as_const(context));
        if (matches.empty())
            return;
        auto focus_object = matches.front();
        dest_x = focus_object->X();
        dest_y = focus_object->Y();
    }

    double dest_to_target_x = dest_x - target->X();
    double dest_to_target_y = dest_y - target->Y();
    double dest_to_target_dist = std::sqrt(dest_to_target_x * dest_to_target_x +
                                           dest_to_target_y * dest_to_target_y);
    double new_x, new_y;

    if (dest_to_target_dist < speed) {
        new_x = dest_x;
        new_y = dest_y;
    } else {
        // ensure no divide by zero issues
        if (dest_to_target_dist < 1.0)
            dest_to_target_dist = 1.0;
        // avoid stalling when right on top of object and attempting to move away from it
        if (dest_to_target_x == 0.0 && dest_to_target_y == 0.0)
            dest_to_target_x = 1.0;
        // move in direction of target
        new_x = target->X() + dest_to_target_x / dest_to_target_dist * speed;
        new_y = target->Y() + dest_to_target_y / dest_to_target_dist * speed;
    }
    if (target->X() == new_x && target->Y() == new_y)
        return; // nothing to do

    if (auto* system = dynamic_cast<System*>(target)) {
        system->MoveTo(new_x, new_y);
        for (auto* obj : context.ContextObjects().findRaw<UniverseObject>(system->ObjectIDs()))
            obj->MoveTo(new_x, new_y);

        // don't need to remove objects from system or insert into it, as all
        // contained objects in system are moved with it, maintaining their
        // containment situation

    } else if (auto* fleet = dynamic_cast<Fleet*>(target)) {
        auto old_sys = context.ContextObjects().getRaw<System>(fleet->SystemID());
        if (old_sys)
            old_sys->Remove(fleet->ID());
        fleet->SetSystem(INVALID_OBJECT_ID);
        fleet->MoveTo(new_x, new_y);
        for (auto* ship : context.ContextObjects().findRaw<Ship>(fleet->ShipIDs())) {
            if (old_sys)
                old_sys->Remove(ship->ID());
            ship->SetSystem(INVALID_OBJECT_ID);
            ship->MoveTo(new_x, new_y);
        }

        // todo: is fleet now close enough to fall into a system?
        UpdateFleetRoute(fleet, INVALID_OBJECT_ID, INVALID_OBJECT_ID, context);

    } else if (auto* ship = dynamic_cast<Ship*>(target)) {
        auto old_sys = context.ContextObjects().getRaw<System>(ship->SystemID());
        if (old_sys)
            old_sys->Remove(ship->ID());
        ship->SetSystem(INVALID_OBJECT_ID);

        auto old_fleet = context.ContextObjects().getRaw<Fleet>(ship->FleetID());
        FleetAggression old_fleet_aggr = FleetAggression::INVALID_FLEET_AGGRESSION;
        if (old_fleet) {
            old_fleet_aggr = old_fleet->Aggression();
            old_fleet->RemoveShips({ship->ID()});
        }
        ship->SetFleetID(INVALID_OBJECT_ID);

        // if ship is armed use old fleet's aggression. otherwise use auto-determined aggression
        auto aggr = ship->IsArmed(context) ? old_fleet_aggr : FleetAggression::INVALID_FLEET_AGGRESSION;

        CreateNewFleet(new_x, new_y, ship, context, aggr); // creates new fleet and inserts ship into fleet
        if (old_fleet && old_fleet->Empty()) {
            if (old_sys)
                old_sys->Remove(old_fleet->ID());
            context.ContextUniverse().EffectDestroy(old_fleet->ID(), INVALID_OBJECT_ID);    // no object in particular destroyed this fleet
        }

    } else if (auto* field = dynamic_cast<Field*>(target)) {
        auto old_sys = context.ContextObjects().getRaw<System>(field->SystemID());
        if (old_sys)
            old_sys->Remove(field->ID());
        field->SetSystem(INVALID_OBJECT_ID);
        field->MoveTo(new_x, new_y);

    }
    // don't move planets or buildings, as these can't exist outside of systems
}

std::string MoveTowards::Dump(uint8_t ntabs) const {
    if (m_dest_condition)
        return DumpIndent(ntabs) + "MoveTowards destination = " + m_dest_condition->Dump(ntabs) + "\n";
    else if (m_dest_x && m_dest_y)
        return DumpIndent(ntabs) + "MoveTowards x = " + m_dest_x->Dump(ntabs) + " y = " + m_dest_y->Dump(ntabs) + "\n";
    else
        return DumpIndent(ntabs) + "MoveTowards";
}

void MoveTowards::SetTopLevelContent(const std::string& content_name) {
    if (m_speed)
        m_speed->SetTopLevelContent(content_name);
    if (m_dest_condition)
        m_dest_condition->SetTopLevelContent(content_name);
    if (m_dest_x)
        m_dest_x->SetTopLevelContent(content_name);
    if (m_dest_y)
        m_dest_y->SetTopLevelContent(content_name);
}

uint32_t MoveTowards::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "MoveTowards");
    CheckSums::CheckSumCombine(retval, m_speed);
    CheckSums::CheckSumCombine(retval, m_dest_condition);
    CheckSums::CheckSumCombine(retval, m_dest_x);
    CheckSums::CheckSumCombine(retval, m_dest_y);

    TraceLogger(effects) << "GetCheckSum(MoveTowards): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> MoveTowards::Clone() const {
    auto retval = std::make_unique<MoveTowards>(ValueRef::CloneUnique(m_speed),
                                                ValueRef::CloneUnique(m_dest_x),
                                                ValueRef::CloneUnique(m_dest_y));
    retval->m_dest_condition = ValueRef::CloneUnique(m_dest_condition);
    return retval;
}


///////////////////////////////////////////////////////////
// SetDestination                                        //
///////////////////////////////////////////////////////////
SetDestination::SetDestination(std::unique_ptr<Condition::Condition>&& location_condition) :
    m_location_condition(std::move(location_condition))
{}

void SetDestination::Execute(ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger(effects) << "SetDestination::Execute given no target object";
        return;
    }

    auto target_fleet = dynamic_cast<Fleet*>(context.effect_target);
    if (!target_fleet) {
        ErrorLogger(effects) << "SetDestination::Execute acting on non-fleet target:" << context.effect_target->Dump();
        return;
    }

    // apply location condition to determine valid location to move target to
    TargetSet valid_locations = m_location_condition->Eval(context);

    // early exit if there are no valid locations - can't move anything if there's nowhere to move to
    if (valid_locations.empty())
        return;

    // "randomly" pick a destination
    int destination_idx = RandInt(0, valid_locations.size() - 1);
    auto destination = valid_locations[destination_idx];
    int destination_system_id = destination->SystemID();

    // early exit if destination is not / in a system
    if (destination_system_id == INVALID_OBJECT_ID)
        return;

    int start_system_id = target_fleet->SystemID();
    if (start_system_id == INVALID_OBJECT_ID)
        start_system_id = target_fleet->NextSystemID();
    // abort if no valid starting system
    if (start_system_id == INVALID_OBJECT_ID)
        return;

    // find shortest path for fleet's owner
    auto route_list = context.ContextUniverse().GetPathfinder().ShortestPath(
        start_system_id, destination_system_id, target_fleet->Owner(), context.ContextObjects()).first;

    // reject empty move paths (no path exists).
    if (route_list.empty())
        return;

    // check destination validity: disallow movement that's out of range
    const auto eta_final = target_fleet->ETA(target_fleet->MovePath(route_list, false, context)).first;
    if (eta_final == Fleet::ETA_NEVER || eta_final == Fleet::ETA_OUT_OF_RANGE)
        return;

    try {
        target_fleet->SetRoute(std::move(route_list), context.ContextObjects());
    } catch (const std::exception& e) {
        ErrorLogger(effects) << "Caught exception in Effect::SetDestination setting fleet route: " << e.what();
    }
}

std::string SetDestination::Dump(uint8_t ntabs) const
{ return DumpIndent(ntabs) + "SetDestination destination = " + m_location_condition->Dump(ntabs) + "\n"; }

void SetDestination::SetTopLevelContent(const std::string& content_name) {
    if (m_location_condition)
        m_location_condition->SetTopLevelContent(content_name);
}

uint32_t SetDestination::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "SetDestination");
    CheckSums::CheckSumCombine(retval, m_location_condition);

    TraceLogger(effects) << "GetCheckSum(SetDestination): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> SetDestination::Clone() const
{ return std::make_unique<SetDestination>(ValueRef::CloneUnique(m_location_condition)); }


///////////////////////////////////////////////////////////
// SetAggression                                         //
///////////////////////////////////////////////////////////
SetAggression::SetAggression(FleetAggression aggression) :
    m_aggression(aggression)
{}

void SetAggression::Execute(ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger(effects) << "SetAggression::Execute given no target object";
        return;
    }
    if (context.effect_target->ObjectType() != UniverseObjectType::OBJ_FLEET) {
        ErrorLogger(effects) << "SetAggression::Execute acting on non-fleet target:" << context.effect_target->Dump();
        return;
    }

    auto target_fleet = static_cast<Fleet*>(context.effect_target);
    target_fleet->SetAggression(m_aggression);
}

std::string SetAggression::Dump(uint8_t ntabs) const {
    return DumpIndent(ntabs) + [aggr{this->m_aggression}]() {
        switch(aggr) {
        case FleetAggression::FLEET_AGGRESSIVE:  return "SetAggressive";  break;
        case FleetAggression::FLEET_OBSTRUCTIVE: return "SetObstructive"; break;
        case FleetAggression::FLEET_DEFENSIVE:   return "SetDefensive";   break;
        case FleetAggression::FLEET_PASSIVE:     return "SetPassive";     break;
        default:                                 return "Set???";         break;
        }
    }();
}

uint32_t SetAggression::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "SetAggression");
    CheckSums::CheckSumCombine(retval, m_aggression);

    TraceLogger(effects) << "GetCheckSum(SetAggression): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> SetAggression::Clone() const
{ return std::make_unique<SetAggression>(m_aggression); }


///////////////////////////////////////////////////////////
// Victory                                               //
///////////////////////////////////////////////////////////
Victory::Victory(std::string reason_string) :
    m_reason_string(std::move(reason_string))
{}

void Victory::Execute(ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger(effects) << "Victory::Execute given no target object";
        return;
    }
    if (auto empire = context.GetEmpire(context.effect_target->Owner()))
        empire->Win(m_reason_string, context.Empires().GetEmpires(), context.current_turn);
    else
        ErrorLogger(effects) << "Trying to grant victory to a missing empire!";
}

std::string Victory::Dump(uint8_t ntabs) const
{ return DumpIndent(ntabs) + "Victory reason = \"" + m_reason_string + "\"\n"; }

uint32_t Victory::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "Victory");
    CheckSums::CheckSumCombine(retval, m_reason_string);

    TraceLogger(effects) << "GetCheckSum(Victory): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> Victory::Clone() const
{ return std::make_unique<Victory>(m_reason_string); }


///////////////////////////////////////////////////////////
// SetEmpireTechProgress                                 //
///////////////////////////////////////////////////////////
SetEmpireTechProgress::SetEmpireTechProgress(std::unique_ptr<ValueRef::ValueRef<std::string>>&& tech_name,
                                             std::unique_ptr<ValueRef::ValueRef<double>>&& research_progress,
                                             std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id) :
    m_tech_name(std::move(tech_name)),
    m_research_progress(std::move(research_progress)),
    m_empire_id(
        empire_id
        ? std::move(empire_id)
        : std::make_unique<ValueRef::Variable<int>>(ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE, "Owner"))
{}

void SetEmpireTechProgress::Execute(ScriptingContext& context) const {
    if (!m_empire_id) return;
    auto empire = context.GetEmpire(m_empire_id->Eval(context));
    if (!empire) return;

    if (!m_tech_name) {
        ErrorLogger(effects) << "SetEmpireTechProgress::Execute has not tech name to evaluate";
        return;
    }
    std::string tech_name = m_tech_name->Eval(context);
    if (tech_name.empty())
        return;

    const Tech* tech = GetTech(tech_name);
    if (!tech) {
        ErrorLogger(effects) << "SetEmpireTechProgress::Execute couldn't get tech with name " << tech_name;
        return;
    }

    const ScriptingContext::CurrentValueVariant cvv{empire->ResearchProgress(tech_name, context)};
    const ScriptingContext progress_context{context, cvv};
    empire->SetTechResearchProgress(tech_name, m_research_progress->Eval(progress_context), context);
}

std::string SetEmpireTechProgress::Dump(uint8_t ntabs) const {
    std::string retval = "SetEmpireTechProgress name = ";
    if (m_tech_name)
        retval += m_tech_name->Dump(ntabs);
    if (m_research_progress)
        retval += " progress = " + m_research_progress->Dump(ntabs);
    if (m_empire_id)
        retval += " empire = " + m_empire_id->Dump(ntabs) + "\n";
    return retval;
}

void SetEmpireTechProgress::SetTopLevelContent(const std::string& content_name) {
    if (m_tech_name)
        m_tech_name->SetTopLevelContent(content_name);
    if (m_research_progress)
        m_research_progress->SetTopLevelContent(content_name);
    if (m_empire_id)
        m_empire_id->SetTopLevelContent(content_name);
}

uint32_t SetEmpireTechProgress::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "SetEmpireTechProgress");
    CheckSums::CheckSumCombine(retval, m_tech_name);
    CheckSums::CheckSumCombine(retval, m_research_progress);
    CheckSums::CheckSumCombine(retval, m_empire_id);

    TraceLogger(effects) << "GetCheckSum(SetEmpireTechProgress): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> SetEmpireTechProgress::Clone() const {
    return std::make_unique<SetEmpireTechProgress>(ValueRef::CloneUnique(m_tech_name),
                                                   ValueRef::CloneUnique(m_research_progress),
                                                   ValueRef::CloneUnique(m_empire_id));
}


///////////////////////////////////////////////////////////
// GiveEmpireContent                                        //
///////////////////////////////////////////////////////////
GiveEmpireContent::GiveEmpireContent(std::unique_ptr<ValueRef::ValueRef<std::string>>&& content_name,
                                     UnlockableItemType unlock_type,
                                     std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id) :
    m_content_name(std::move(content_name)),
    m_unlock_type(unlock_type),
    m_empire_id(empire_id ?
                    std::move(empire_id) :
                    std::make_unique<ValueRef::Variable<int>>(ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE, "Owner")
               )
{}

void GiveEmpireContent::Execute(ScriptingContext& context) const {
    if (!m_empire_id || !m_content_name) return;
    auto empire = context.GetEmpire(m_empire_id->Eval(context));
    if (!empire) return;

    switch (m_unlock_type) {
    case UnlockableItemType::UIT_BUILDING:  empire->AddBuildingType(m_content_name->Eval(context), context.current_turn); break;
    case UnlockableItemType::UIT_SHIP_PART: empire->AddShipPart(m_content_name->Eval(context), context.current_turn);     break;
    case UnlockableItemType::UIT_SHIP_HULL: empire->AddShipHull(m_content_name->Eval(context), context.current_turn);     break;
    case UnlockableItemType::UIT_POLICY:    empire->AddPolicy(m_content_name->Eval(context), context.current_turn);       break;
    case UnlockableItemType::UIT_TECH: {
        auto content_name = m_content_name->Eval(context);
        if (!GetTech(content_name)) {
            ErrorLogger(effects) << "GiveEmpireContent::Execute couldn't get tech with name: " << content_name;
            return;
        }
        empire->AddNewlyResearchedTechToGrantAtStartOfNextTurn(std::move(content_name));
        break;
    }
    default: {
        ErrorLogger(effects) << "GiveEmpireContent::Execute given invalid unlockable item type: " << to_string(m_unlock_type);
    }
    }
}

std::string GiveEmpireContent::Dump(uint8_t ntabs) const {
    std::string retval = DumpIndent(ntabs) + "GiveEmpire";

    switch(m_unlock_type) {
    case UnlockableItemType::UIT_TECH:      retval += "Tech";       break;
    case UnlockableItemType::UIT_BUILDING:  retval += "Building";   break;
    case UnlockableItemType::UIT_SHIP_HULL: retval += "Hull";       break;
    case UnlockableItemType::UIT_SHIP_PART: retval += "Part";       break;
    case UnlockableItemType::UIT_POLICY:    retval += "Policy";     break;
    default:                                retval += "???";
    }

    if (m_content_name)
        retval += " name = " + m_content_name->Dump(ntabs);

    if (m_empire_id)
        retval += " empire = " + m_empire_id->Dump(ntabs);

    retval += "\n";
    return retval;
}

void GiveEmpireContent::SetTopLevelContent(const std::string& content_name) {
    if (m_empire_id)
        m_empire_id->SetTopLevelContent(content_name);
    if (m_content_name)
        m_content_name->SetTopLevelContent(content_name);
}

uint32_t GiveEmpireContent::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "GiveEmpireContent");
    CheckSums::CheckSumCombine(retval, m_content_name);
    CheckSums::CheckSumCombine(retval, m_unlock_type);
    CheckSums::CheckSumCombine(retval, m_empire_id);

    TraceLogger(effects) << "GetCheckSum(GiveEmpireContent): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> GiveEmpireContent::Clone() const {
    return std::make_unique<GiveEmpireContent>(ValueRef::CloneUnique(m_content_name),
                                               m_unlock_type,
                                               ValueRef::CloneUnique(m_empire_id));
}


///////////////////////////////////////////////////////////
// GenerateSitRepMessage                                 //
///////////////////////////////////////////////////////////
GenerateSitRepMessage::GenerateSitRepMessage(std::string message_string,
                                             std::string icon,
                                             MessageParams&& message_parameters,
                                             std::unique_ptr<ValueRef::ValueRef<int>>&& recipient_empire_id,
                                             EmpireAffiliationType affiliation,
                                             std::string label,
                                             bool stringtable_lookup) :
    m_message_string(std::move(message_string)),
    m_icon(std::move(icon)),
    m_message_parameters(std::move(message_parameters)),
    m_recipient_empire_id(std::move(recipient_empire_id)),
    m_affiliation(affiliation),
    m_label(std::move(label)),
    m_stringtable_lookup(stringtable_lookup)
{}

GenerateSitRepMessage::GenerateSitRepMessage(std::string message_string,
                                             std::string icon,
                                             MessageParams&& message_parameters,
                                             EmpireAffiliationType affiliation,
                                             std::unique_ptr<Condition::Condition>&& condition,
                                             std::string label,
                                             bool stringtable_lookup) :
    m_message_string(std::move(message_string)),
    m_icon(std::move(icon)),
    m_message_parameters(std::move(message_parameters)),
    m_condition(std::move(condition)),
    m_affiliation(affiliation),
    m_label(std::move(label)),
    m_stringtable_lookup(stringtable_lookup)
{}

GenerateSitRepMessage::GenerateSitRepMessage(std::string message_string,
                                             std::string icon,
                                             MessageParams&& message_parameters,
                                             EmpireAffiliationType affiliation,
                                             std::string label,
                                             bool stringtable_lookup) :
    m_message_string(std::move(message_string)),
    m_icon(std::move(icon)),
    m_message_parameters(std::move(message_parameters)),
    m_affiliation(affiliation),
    m_label(std::move(label)),
    m_stringtable_lookup(stringtable_lookup)
{}

void GenerateSitRepMessage::Execute(ScriptingContext& context) const {
    const int recipient_id = m_recipient_empire_id ? m_recipient_empire_id->Eval(context) : ALL_EMPIRES;

    // track any ship designs used in message, which any recipients must be
    // made aware of so sitrep won't have errors
    std::set<int> ship_design_ids_to_inform_receipits_of;

    // TODO: should any referenced object IDs being made known at basic visibility?


    // evaluate all parameter valuerefs so they can be substituted into sitrep template
    std::vector<std::pair<std::string, std::string>> parameter_tag_values;
    parameter_tag_values.reserve(m_message_parameters.size());
    for (auto& [param_tag, param_ref] : m_message_parameters) {
        if (!param_ref)
            ErrorLogger(effects) << "GenerateSitRepMessage::Execute got null parameter reference for tag: " << param_tag;

        std::string param_val{param_ref ? param_ref->Eval(context) : std::string()};
        // special case for ship designs: make sure sitrep recipient knows about the design
        // so the sitrep won't have errors about unknown designs being referenced
        if (param_tag == VarText::PREDEFINED_DESIGN_TAG) {
            if (const ShipDesign* design = context.ContextUniverse().GetGenericShipDesign(param_val))
                ship_design_ids_to_inform_receipits_of.insert(design->ID());
        }

        parameter_tag_values.emplace_back(param_tag, std::move(param_val));
    }

    const auto not_recipient = [recipient_id](const auto empire_id) { return recipient_id != empire_id; };
    const auto to_id_status = [&context, recipient_id](const auto empire_id)
    { return std::pair(empire_id, context.ContextDiploStatus(recipient_id, empire_id)); };

    // whom to send to?
    std::set<int> recipient_empire_ids;
    switch (m_affiliation) {
    case EmpireAffiliationType::AFFIL_SELF: {
        // add just specified empire
        if (recipient_id != ALL_EMPIRES)
            recipient_empire_ids.insert(recipient_id);
        break;
    }

    case EmpireAffiliationType::AFFIL_ALLY: {
        static constexpr auto are_allied = [](const auto& id_status)
        { return id_status.second >= DiplomaticStatus::DIPLO_ALLIED; };

        // add allies of specified empire
        if (recipient_id != ALL_EMPIRES) {
            auto allies_rng = context.EmpireIDs() | range_filter(not_recipient)
                | range_transform(to_id_status) | range_filter(are_allied) | range_keys;
            recipient_empire_ids.insert(allies_rng.begin(), allies_rng.end());
        }
        break;
    }

    case EmpireAffiliationType::AFFIL_PEACE: {
        static constexpr auto are_peacy = [](const auto& id_status)
        { return id_status.second >= DiplomaticStatus::DIPLO_PEACE; };

        // add empires at peace with the specified empire
        if (recipient_id != ALL_EMPIRES) {
            auto peacey_rng = context.EmpireIDs() | range_filter(not_recipient)
                | range_transform(to_id_status) | range_filter(are_peacy) | range_keys;
            recipient_empire_ids.insert(peacey_rng.begin(), peacey_rng.end());
        }
        break;
    }

    case EmpireAffiliationType::AFFIL_ENEMY: {
        static constexpr auto are_warring = [](const auto& id_status)
        { return id_status.second == DiplomaticStatus::DIPLO_WAR; };

        // add enemies of specified empire
        if (recipient_id != ALL_EMPIRES) {
            auto warring_rng = context.EmpireIDs() | range_filter(not_recipient)
                | range_transform(to_id_status) | range_filter(are_warring) | range_keys;
            recipient_empire_ids.insert(warring_rng.begin(), warring_rng.end());
        }
        break;
    }

    case EmpireAffiliationType::AFFIL_CAN_SEE: {
        if (!m_condition)
            break;
        const auto objs = m_condition->Eval(std::as_const(context)); // TODO: use lazy condition evaluation
        const auto obj_ids_rng = objs | range_transform([](const auto* obj) { return obj->ID(); });
        const auto& eov = context.empire_object_vis;

        // can an empire see an object that matches the condition?
        const auto can_see_any_obj_id = [obj_ids_rng, &eov](const auto empire_id) {
            const auto eov_it = eov.find(empire_id);
            if (eov_it == eov.end())
                return false;
            const auto can_see_obj_id = [&ov{eov_it->second}](const auto obj_id) {
                const auto ov_it = ov.find(obj_id);
                return ov_it != ov.end() && ov_it->second >= Visibility::VIS_BASIC_VISIBILITY;
            };
            return range_any_of(obj_ids_rng, can_see_obj_id);
        };

        // add empires that can see any condition-matching object
        auto empires_that_can_see_something = context.EmpireIDs() | range_filter(can_see_any_obj_id);
        recipient_empire_ids.insert(empires_that_can_see_something.begin(), empires_that_can_see_something.end());
        break;
    }

    case EmpireAffiliationType::AFFIL_NONE:
        // add no empires
        break;

    case EmpireAffiliationType::AFFIL_HUMAN:
        // todo: implement this separately, though not high priority since it
        // probably doesn't matter if AIs get an extra sitrep message meant for
        // human eyes
    case EmpireAffiliationType::AFFIL_ANY:
    default: {
        // add all empires
        const auto& all_empires_rng = context.EmpireIDs();
        recipient_empire_ids.insert(all_empires_rng.begin(), all_empires_rng.end());
        break;
    }
    }

    const int sitrep_turn = context.current_turn + 1;

    // send to recipient empires
    for (int empire_id : recipient_empire_ids) {
        auto empire = context.GetEmpire(empire_id);
        if (!empire)
            continue;
        empire->AddSitRepEntry(CreateSitRep(m_message_string, sitrep_turn, m_icon,
                                            parameter_tag_values,   // copy tag values for each
                                            m_label, m_stringtable_lookup));

        // also inform of any ship designs recipients should know about
        for (int design_id : ship_design_ids_to_inform_receipits_of)
            context.ContextUniverse().SetEmpireKnowledgeOfShipDesign(design_id, empire_id);
    }
}

std::string GenerateSitRepMessage::Dump(uint8_t ntabs) const {
    std::string retval = DumpIndent(ntabs);
    retval += "GenerateSitRepMessage\n";
    retval += DumpIndent(ntabs+1) + "message = \"" + m_message_string + "\"" + " icon = " + m_icon + "\n";

    if (m_message_parameters.size() == 1) {
        retval += DumpIndent(ntabs+1) + "parameters = tag = " + m_message_parameters[0].first;
        retval += " data = " + m_message_parameters[0].second->Dump(ntabs+1) + "\n";
    } else if (!m_message_parameters.empty()) {
        retval += DumpIndent(ntabs+1) + "parameters = [ ";
        for (const auto& entry : m_message_parameters) {
            retval += " tag = " + entry.first
                   + " data = " + entry.second->Dump(ntabs+1)
                   + " ";
        }
        retval += "]\n";
    }

    retval += DumpIndent(ntabs+1) + "affiliation = ";
    switch (m_affiliation) {
    case EmpireAffiliationType::AFFIL_SELF:    retval += "TheEmpire";  break;
    case EmpireAffiliationType::AFFIL_ENEMY:   retval += "EnemyOf";    break;
    case EmpireAffiliationType::AFFIL_PEACE:   retval += "PeaceWith";  break;
    case EmpireAffiliationType::AFFIL_ALLY:    retval += "AllyOf";     break;
    case EmpireAffiliationType::AFFIL_ANY:     retval += "AnyEmpire";  break;
    case EmpireAffiliationType::AFFIL_CAN_SEE: retval += "CanSee";     break;
    case EmpireAffiliationType::AFFIL_HUMAN:   retval += "Human";      break;
    default:                                   retval += "?";          break;
    }

    if (m_recipient_empire_id)
        retval += "\n" + DumpIndent(ntabs+1) + "empire = " + m_recipient_empire_id->Dump(ntabs+1) + "\n";
    if (m_condition)
        retval += "\n" + DumpIndent(ntabs+1) + "condition = " + m_condition->Dump(ntabs+1) + "\n";

    return retval;
}

void GenerateSitRepMessage::SetTopLevelContent(const std::string& content_name) {
    for (auto& entry : m_message_parameters)
        entry.second->SetTopLevelContent(content_name);
    if (m_recipient_empire_id)
        m_recipient_empire_id->SetTopLevelContent(content_name);
    if (m_condition)
        m_condition->SetTopLevelContent(content_name);
}

uint32_t GenerateSitRepMessage::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "GenerateSitRepMessage");
    CheckSums::CheckSumCombine(retval, m_message_string);
    CheckSums::CheckSumCombine(retval, m_icon);
    CheckSums::CheckSumCombine(retval, m_message_parameters);
    CheckSums::CheckSumCombine(retval, m_recipient_empire_id);
    CheckSums::CheckSumCombine(retval, m_condition);
    CheckSums::CheckSumCombine(retval, m_affiliation);
    CheckSums::CheckSumCombine(retval, m_label);
    CheckSums::CheckSumCombine(retval, m_stringtable_lookup);

    TraceLogger(effects) << "GetCheckSum(GenerateSitRepMessage): retval: " << retval;
    return retval;
}

std::vector<std::pair<std::string, const ValueRef::ValueRef<std::string>*>>
GenerateSitRepMessage::MessageParameters() const {
    std::vector<std::pair<std::string, const ValueRef::ValueRef<std::string>*>> retval;
    retval.reserve(m_message_parameters.size());
    std::transform(m_message_parameters.begin(), m_message_parameters.end(), std::back_inserter(retval),
                   [](const auto& xx) { return std::pair(xx.first, xx.second.get()); });
    return retval;
}

std::unique_ptr<Effect> GenerateSitRepMessage::Clone() const {
    auto retval = std::make_unique<GenerateSitRepMessage>(
        m_message_string, m_icon, ValueRef::CloneUnique(m_message_parameters),
        ValueRef::CloneUnique(m_recipient_empire_id), m_affiliation,
        m_label, m_stringtable_lookup);
    retval->m_condition = ValueRef::CloneUnique(m_condition);
    return retval;
}

///////////////////////////////////////////////////////////
// SetOverlayTexture                                     //
///////////////////////////////////////////////////////////
SetOverlayTexture::SetOverlayTexture(std::string& texture,
                                     std::unique_ptr<ValueRef::ValueRef<double>>&& size) :
    m_texture(std::move(texture)),
    m_size(std::move(size))
{}

SetOverlayTexture::SetOverlayTexture(std::string& texture, ValueRef::ValueRef<double>* size) :
    m_texture(std::move(texture)),
    m_size(size)
{}

void SetOverlayTexture::Execute(ScriptingContext& context) const {
    if (!context.effect_target || context.effect_target->ObjectType() != UniverseObjectType::OBJ_SYSTEM)
        return;
    double size = m_size ? m_size->Eval(context) : 1.0;
    auto system = static_cast<System*>(context.effect_target);
    system->SetOverlayTexture(m_texture, size);
}

std::string SetOverlayTexture::Dump(uint8_t ntabs) const {
    std::string retval = DumpIndent(ntabs) + "SetOverlayTexture texture = " + m_texture;
    if (m_size)
        retval += " size = " + m_size->Dump(ntabs);
    retval += "\n";
    return retval;
}

void SetOverlayTexture::SetTopLevelContent(const std::string& content_name) {
    if (m_size)
        m_size->SetTopLevelContent(content_name);
}

uint32_t SetOverlayTexture::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "SetOverlayTexture");
    CheckSums::CheckSumCombine(retval, m_texture);
    CheckSums::CheckSumCombine(retval, m_size);

    TraceLogger(effects) << "GetCheckSum(SetOverlayTexture): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> SetOverlayTexture::Clone() const {
    auto texture{m_texture};
    return std::make_unique<SetOverlayTexture>(texture, ValueRef::CloneUnique(m_size));
}


///////////////////////////////////////////////////////////
// SetTexture                                            //
///////////////////////////////////////////////////////////
void SetTexture::Execute(ScriptingContext& context) const {
    if (!context.effect_target || context.effect_target->ObjectType() != UniverseObjectType::OBJ_PLANET)
        return;
    auto planet = static_cast<Planet*>(context.effect_target);
    planet->SetSurfaceTexture(m_texture);
}

std::string SetTexture::Dump(uint8_t ntabs) const
{ return DumpIndent(ntabs) + "SetTexture texture = " + m_texture + "\n"; }

uint32_t SetTexture::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "SetTexture");
    CheckSums::CheckSumCombine(retval, m_texture);

    TraceLogger(effects) << "GetCheckSum(SetTexture): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> SetTexture::Clone() const {
    auto texture{m_texture};
    return std::make_unique<SetTexture>(texture);
}


///////////////////////////////////////////////////////////
// SetVisibility                                         //
///////////////////////////////////////////////////////////
SetVisibility::SetVisibility(std::unique_ptr<ValueRef::ValueRef<Visibility>> vis,
                             EmpireAffiliationType affiliation,
                             std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id,
                             std::unique_ptr<Condition::Condition>&& of_objects) :
    m_vis(std::move(vis)),
    m_empire_id(std::move(empire_id)),
    m_affiliation(affiliation),
    m_condition(std::move(of_objects))
{}

void SetVisibility::Execute(ScriptingContext& context) const {
    if (!context.effect_target)
        return;

    // Note: TODO: currently ignoring upgrade-only flag

    if (!m_vis)
        return; // nothing to evaluate!

    const int main_empire_id = m_empire_id ? m_empire_id->Eval(context) : ALL_EMPIRES;
    const auto not_main_empire = [main_empire_id](const auto other_id) { return main_empire_id != other_id; };
    const auto to_id_status = [&context, main_empire_id](const auto other_empire_id)
    { return std::pair(other_empire_id, context.ContextDiploStatus(main_empire_id, other_empire_id)); };


    // whom to set visbility for?
    const auto all_empire_ids{context.EmpireIDs()};
    std::vector<int> empire_ids;

    switch (m_affiliation) {
    case EmpireAffiliationType::AFFIL_SELF: {
        // add just specified empire
        if (main_empire_id != ALL_EMPIRES)
            empire_ids.push_back(main_empire_id);
        break;
    }

    case EmpireAffiliationType::AFFIL_ALLY: {
        static constexpr auto are_allied = [](const auto& id_status)
        { return id_status.second >= DiplomaticStatus::DIPLO_ALLIED; };

        // add allies of specified empire
        if (main_empire_id != ALL_EMPIRES) {
            auto allied_rng = all_empire_ids | range_filter(not_main_empire)
                | range_transform(to_id_status) | range_filter(are_allied) | range_keys;
            empire_ids.insert(empire_ids.end(), allied_rng.begin(), allied_rng.end());
        }
        break;
    }

    case EmpireAffiliationType::AFFIL_PEACE: {
        static constexpr auto are_peacey = [](const auto& id_status)
        { return id_status.second >= DiplomaticStatus::DIPLO_PEACE; };

        // add empires at peace with specified empire
        if (main_empire_id != ALL_EMPIRES) {
            auto peacey_rng = all_empire_ids | range_filter(not_main_empire)
                | range_transform(to_id_status) | range_filter(are_peacey) | range_keys;
            empire_ids.insert(empire_ids.end(), peacey_rng.begin(), peacey_rng.end());
        }
        break;
    }

    case EmpireAffiliationType::AFFIL_ENEMY: {
        static constexpr auto are_warring = [](const auto& id_status)
        { return id_status.second == DiplomaticStatus::DIPLO_WAR; };

        // add enemies of specified empire
        if (main_empire_id != ALL_EMPIRES) {
            auto warring_rng = all_empire_ids | range_filter(not_main_empire)
                | range_transform(to_id_status) | range_filter(are_warring) | range_keys;
            empire_ids.insert(empire_ids.end(), warring_rng.begin(), warring_rng.end());
        }
        break;
    }

    case EmpireAffiliationType::AFFIL_CAN_SEE:
        // unsupported so far...
    case EmpireAffiliationType::AFFIL_HUMAN:
        // unsupported so far...
    case EmpireAffiliationType::AFFIL_NONE:
        // add no empires
        break;

    case EmpireAffiliationType::AFFIL_ANY:
    default: 
        empire_ids.insert(empire_ids.end(), all_empire_ids.begin(), all_empire_ids.end());
        break;
    }

    // what to set visibility of?
    std::vector<int> object_ids;
    if (!m_condition) {
        object_ids.push_back(context.effect_target->ID());
    } else {
        // get target object IDs
        Condition::ObjectSet condition_matches = m_condition->Eval(std::as_const(context));
        object_ids.reserve(condition_matches.size());
        std::transform(condition_matches.begin(), condition_matches.end(),
                       std::back_inserter(object_ids),
                       [](const auto* o) { return o->ID(); });
        // ensure uniqueness
        std::sort(object_ids.begin(), object_ids.end());
        auto unique_it = std::unique(object_ids.begin(), object_ids.end());
        object_ids.resize(std::distance(object_ids.begin(), unique_it));
    }

    const int source_id = context.source ? context.source->ID() : INVALID_OBJECT_ID;

    for (const int emp_id : empire_ids) {
        for (const int obj_id : object_ids) {
            // store source object id and ValueRef to evaluate to determine
            // what visibility level to set at time of application
            context.ContextUniverse().SetEffectDerivedVisibility(emp_id, obj_id, source_id, m_vis.get());
        }
    }
}

std::string SetVisibility::Dump(uint8_t ntabs) const {
    std::string retval = DumpIndent(ntabs);

    retval += DumpIndent(ntabs) + "SetVisibility affiliation = ";
    switch (m_affiliation) {
    case EmpireAffiliationType::AFFIL_SELF:    retval += "TheEmpire";  break;
    case EmpireAffiliationType::AFFIL_ENEMY:   retval += "EnemyOf";    break;
    case EmpireAffiliationType::AFFIL_PEACE:   retval += "PeaceWith";  break;
    case EmpireAffiliationType::AFFIL_ALLY:    retval += "AllyOf";     break;
    case EmpireAffiliationType::AFFIL_ANY:     retval += "AnyEmpire";  break;
    case EmpireAffiliationType::AFFIL_CAN_SEE: retval += "CanSee";     break;
    case EmpireAffiliationType::AFFIL_HUMAN:   retval += "Human";      break;
    default:                                   retval += "?";          break;
    }

    if (m_empire_id)
        retval += " empire = " + m_empire_id->Dump(ntabs);

    if (m_vis)
        retval += " visibility = " + m_vis->Dump(ntabs);

    if (m_condition)
        retval += " condition = " + m_condition->Dump(ntabs);

    retval += "\n";
    return retval;
}

void SetVisibility::SetTopLevelContent(const std::string& content_name) {
    if (m_vis)
        m_vis->SetTopLevelContent(content_name);
    if (m_empire_id)
        m_empire_id->SetTopLevelContent(content_name);
    if (m_condition)
        m_condition->SetTopLevelContent(content_name);
}

uint32_t SetVisibility::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "SetVisibility");
    CheckSums::CheckSumCombine(retval, m_vis);
    CheckSums::CheckSumCombine(retval, m_empire_id);
    CheckSums::CheckSumCombine(retval, m_affiliation);
    CheckSums::CheckSumCombine(retval, m_condition);

    TraceLogger(effects) << "GetCheckSum(SetVisibility): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> SetVisibility::Clone() const {
    return std::make_unique<SetVisibility>(ValueRef::CloneUnique(m_vis),
                                           m_affiliation,
                                           ValueRef::CloneUnique(m_empire_id),
                                           ValueRef::CloneUnique(m_condition));
}


///////////////////////////////////////////////////////////
// Conditional                                           //
///////////////////////////////////////////////////////////
Conditional::Conditional(std::unique_ptr<Condition::Condition>&& target_condition,
                         std::vector<std::unique_ptr<Effect>>&& true_effects,
                         std::vector<std::unique_ptr<Effect>>&& false_effects) :
    m_target_condition(std::move(target_condition)),
    m_true_effects(std::move(true_effects)),
    m_false_effects(std::move(false_effects))
{
    if (m_target_condition && !m_target_condition->TargetInvariant()) {
        ErrorLogger(effects) << "Conditional effect has a target condition that depends on the target object. The condition is evaluated once to pick the targets, so when evaluating it, there is no defined target object.";
        DebugLogger(effects) << "Condition effect is: " << Dump();
    }
}

void Conditional::Execute(ScriptingContext& context) const {
    if (!context.effect_target)
        return;

    if (!m_target_condition || m_target_condition->EvalOne(context, context.effect_target)) {
        for (auto& effect : m_true_effects) {
            if (effect)
                effect->Execute(context);
        }
    } else {
        for (auto& effect : m_false_effects) {
            if (effect)
                effect->Execute(context);
        }
    }
}

void Conditional::Execute(ScriptingContext& context, const TargetSet& targets) const {
    if (targets.empty())
        return;

    // apply sub-condition to target set to pick which to act on with which of sub-effects
    TargetSet matches{targets.begin(), targets.end()};
    TargetSet non_matches;
    non_matches.reserve(matches.size());
    if (m_target_condition)
        m_target_condition->Eval(context, matches, non_matches, Condition::SearchDomain::MATCHES);

    if (!matches.empty() && !m_true_effects.empty()) {
        for (auto& effect : m_true_effects) {
            if (effect)
                effect->Execute(context, matches);
        }
    }
    if (!non_matches.empty() && !m_false_effects.empty()) {
        for (auto& effect : m_false_effects) {
            if (effect)
                effect->Execute(context, non_matches);
        }
    }
}

void Conditional::Execute(ScriptingContext& context,
                          const TargetSet& targets,
                          AccountingMap* accounting_map,
                          const EffectCause& effect_cause,
                          bool only_meter_effects,
                          bool only_appearance_effects,
                          bool include_empire_meter_effects,
                          bool only_generate_sitrep_effects) const
{
    TraceLogger(effects) << "\n\nExecute Conditional effect: \n" << Dump();

    // apply sub-condition to target set to pick which to act on with which of sub-effects
    TargetSet matches{targets.begin(), targets.end()};
    TargetSet non_matches;
    non_matches.reserve(matches.size());

    if (m_target_condition)
        m_target_condition->Eval(context, matches, non_matches, Condition::SearchDomain::MATCHES);


    // execute true and false effects to target matches and non-matches respectively
    if (!matches.empty() && !m_true_effects.empty()) {
        for (const auto& effect : m_true_effects) {
            effect->Execute(context, matches, accounting_map,
                            effect_cause,
                            only_meter_effects, only_appearance_effects,
                            include_empire_meter_effects,
                            only_generate_sitrep_effects);
        }
    }
    if (!non_matches.empty() && !m_false_effects.empty()) {
        for (const auto& effect : m_false_effects) {
            effect->Execute(context, non_matches, accounting_map,
                            effect_cause,
                            only_meter_effects, only_appearance_effects,
                            include_empire_meter_effects,
                            only_generate_sitrep_effects);
        }
    }
}

std::string Conditional::Dump(uint8_t ntabs) const {
    std::string retval = DumpIndent(ntabs) + "If\n";
    if (m_target_condition) {
        retval += DumpIndent(ntabs+1) + "condition =\n";
        retval += m_target_condition->Dump(ntabs+2);
    }

    if (m_true_effects.size() == 1) {
        retval += DumpIndent(ntabs+1) + "effects =\n";
        retval += m_true_effects[0]->Dump(ntabs+2);
    } else {
        retval += DumpIndent(ntabs+1) + "effects = [\n";
        for (auto& effect : m_true_effects) {
            retval += effect->Dump(ntabs+2);
        }
        retval += DumpIndent(ntabs+1) + "]\n";
    }

    if (m_false_effects.empty()) {
        // output nothing
    } else if (m_false_effects.size() == 1) {
        retval += DumpIndent(ntabs+1) + "else =\n";
        retval += m_false_effects[0]->Dump(ntabs+2);
    } else {
        retval += DumpIndent(ntabs+1) + "else = [\n";
        for (auto& effect : m_false_effects) {
            retval += effect->Dump(ntabs+2);
        }
        retval += DumpIndent(ntabs+1) + "]\n";
    }

    return retval;
}

bool Conditional::IsMeterEffect() const noexcept {
    for (auto& effect : m_true_effects) {
        if (effect->IsMeterEffect())
            return true;
    }
    for (auto& effect : m_false_effects) {
        if (effect->IsMeterEffect())
            return true;
    }
    return false;
}

bool Conditional::IsAppearanceEffect() const noexcept {
    for (auto& effect : m_true_effects) {
        if (effect->IsAppearanceEffect())
            return true;
    }
    for (auto& effect : m_false_effects) {
        if (effect->IsAppearanceEffect())
            return true;
    }
    return false;
}

bool Conditional::IsSitrepEffect() const noexcept {
    for (auto& effect : m_true_effects) {
        if (effect->IsSitrepEffect())
            return true;
    }
    for (auto& effect : m_false_effects) {
        if (effect->IsSitrepEffect())
            return true;
    }
    return false;
}

void Conditional::SetTopLevelContent(const std::string& content_name) {
    if (m_target_condition)
        m_target_condition->SetTopLevelContent(content_name);
    for (auto& effect : m_true_effects)
        if (effect)
            (effect)->SetTopLevelContent(content_name);
    for (auto& effect : m_false_effects)
        if (effect)
            (effect)->SetTopLevelContent(content_name);
}

uint32_t Conditional::GetCheckSum() const {
    uint32_t retval{0};

    CheckSums::CheckSumCombine(retval, "Conditional");
    CheckSums::CheckSumCombine(retval, m_target_condition);
    CheckSums::CheckSumCombine(retval, m_true_effects);
    CheckSums::CheckSumCombine(retval, m_false_effects);

    TraceLogger(effects) << "GetCheckSum(Conditional): retval: " << retval;
    return retval;
}

std::unique_ptr<Effect> Conditional::Clone() const {
    return std::make_unique<Conditional>(ValueRef::CloneUnique(m_target_condition),
                                         ValueRef::CloneUnique(m_true_effects),
                                         ValueRef::CloneUnique(m_false_effects));
}

}
