#include "Effect.h"

#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/Random.h"
#include "../util/Directories.h"
#include "../util/i18n.h"
#include "../util/SitRepEntry.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Empire.h"
#include "ValueRef.h"
#include "Condition.h"
#include "Pathfinder.h"
#include "Universe.h"
#include "UniverseObject.h"
#include "Building.h"
#include "Planet.h"
#include "System.h"
#include "Field.h"
#include "Fleet.h"
#include "Ship.h"
#include "Tech.h"
#include "Species.h"
#include "Enums.h"

#include <boost/filesystem/fstream.hpp>
//TODO: replace with std::make_unique when transitioning to C++14
#include <boost/smart_ptr/make_unique.hpp>

#include <cctype>
#include <iterator>

namespace {
    DeclareThreadSafeLogger(effects);
}

using boost::io::str;

FO_COMMON_API extern const int INVALID_DESIGN_ID;

namespace {
    /** creates a new fleet at a specified \a x and \a y location within the
     * Universe, and and inserts \a ship into it.  Used when a ship has been
     * moved by the MoveTo effect separately from the fleet that previously
     * held it.  All ships need to be within fleets. */
    std::shared_ptr<Fleet> CreateNewFleet(double x, double y, std::shared_ptr<Ship> ship) {
        Universe& universe = GetUniverse();
        if (!ship)
            return nullptr;

        auto fleet = universe.InsertNew<Fleet>("", x, y, ship->Owner());

        fleet->Rename(fleet->GenerateFleetName());
        fleet->GetMeter(METER_STEALTH)->SetCurrent(Meter::LARGE_VALUE);

        fleet->AddShips({ship->ID()});
        ship->SetFleetID(fleet->ID());
        fleet->SetAggressive(fleet->HasArmedShips() || fleet->HasFighterShips());

        return fleet;
    }

    /** Creates a new fleet at \a system and inserts \a ship into it.  Used
     * when a ship has been moved by the MoveTo effect separately from the
     * fleet that previously held it.  Also used by CreateShip effect to give
     * the new ship a fleet.  All ships need to be within fleets. */
    std::shared_ptr<Fleet> CreateNewFleet(std::shared_ptr<System> system, std::shared_ptr<Ship> ship) {
        if (!system || !ship)
            return nullptr;

        // remove ship from old fleet / system, put into new system if necessary
        if (ship->SystemID() != system->ID()) {
            if (auto old_system = GetSystem(ship->SystemID())) {
                old_system->Remove(ship->ID());
                ship->SetSystem(INVALID_OBJECT_ID);
            }
            system->Insert(ship);
        }

        if (ship->FleetID() != INVALID_OBJECT_ID) {
            if (auto old_fleet = GetFleet(ship->FleetID())) {
                old_fleet->RemoveShips({ship->ID()});
            }
        }

        // create new fleet for ship, and put it in new system
        auto fleet = CreateNewFleet(system->X(), system->Y(), ship);
        system->Insert(fleet);

        return fleet;
    }

    /** Explores the system with the specified \a system_id for the owner of
      * the specified \a target_object.  Used when moving objects into a system
      * with the MoveTo effect, as otherwise the system wouldn't get explored,
      * and objects being moved into unexplored systems might disappear for
      * players or confuse the AI. */
    void ExploreSystem(int system_id, std::shared_ptr<const UniverseObject> target_object) {
        if (!target_object)
            return;
        if (Empire* empire = GetEmpire(target_object->Owner()))
            empire->AddExploredSystem(system_id);
    }

    /** Resets the previous and next systems of \a fleet and recalcultes /
     * resets the fleet's move route.  Used after a fleet has been moved with
     * the MoveTo effect, as its previous route was assigned based on its
     * previous location, and may not be valid for its new location. */
    void UpdateFleetRoute(std::shared_ptr<Fleet> fleet, int new_next_system, int new_previous_system) {
        if (!fleet) {
            ErrorLogger() << "UpdateFleetRoute passed a null fleet pointer";
            return;
        }

        auto next_system = GetSystem(new_next_system);
        if (!next_system) {
            ErrorLogger() << "UpdateFleetRoute couldn't get new next system with id: " << new_next_system;
            return;
        }

        if (new_previous_system != INVALID_OBJECT_ID && !GetSystem(new_previous_system)) {
            ErrorLogger() << "UpdateFleetRoute couldn't get new previous system with id: " << new_previous_system;
        }

        fleet->SetNextAndPreviousSystems(new_next_system, new_previous_system);


        // recalculate route from the shortest path between first system on path and final destination
        int start_system = fleet->SystemID();
        if (start_system == INVALID_OBJECT_ID)
            start_system = new_next_system;

        int dest_system = fleet->FinalDestinationID();

        std::pair<std::list<int>, double> route_pair = GetPathfinder()->ShortestPath(start_system, dest_system, fleet->Owner());

        // if shortest path is empty, the route may be impossible or trivial, so just set route to move fleet
        // to the next system that it was just set to move to anyway.
        if (route_pair.first.empty())
            route_pair.first.push_back(new_next_system);


        // set fleet with newly recalculated route
        fleet->SetRoute(route_pair.first);
    }

    std::string GenerateSystemName() {
        static std::vector<std::string> star_names = UserStringList("STAR_NAMES");

        // pick a name for the system
        for (const std::string& star_name : star_names) {
            // does an existing system have this name?
            bool dupe = false;
            for (auto& system : Objects().FindObjects<System>()) {
                if (system->Name() == star_name) {
                    dupe = true;
                    break;  // another systme has this name. skip to next potential name.
                }
            }
            if (!dupe)
                return star_name; // no systems have this name yet. use it.
        }
        return "";  // fallback to empty name.
    }
}

namespace Effect {
///////////////////////////////////////////////////////////
// EffectsGroup                                          //
///////////////////////////////////////////////////////////
EffectsGroup::EffectsGroup(std::unique_ptr<Condition::ConditionBase>&& scope,
                           std::unique_ptr<Condition::ConditionBase>&& activation,
                           std::vector<std::unique_ptr<EffectBase>>&& effects,
                           const std::string& accounting_label,
                           const std::string& stacking_group, int priority,
                           const std::string& description,
                           const std::string& content_name):
    m_scope(std::move(scope)),
    m_activation(std::move(activation)),
    m_stacking_group(stacking_group),
    m_effects(std::move(effects)),
    m_accounting_label(accounting_label),
    m_priority(priority),
    m_description(description),
    m_content_name(content_name)
{}

EffectsGroup::~EffectsGroup()
{}

void EffectsGroup::Execute(const TargetsCauses& targets_causes, AccountingMap* accounting_map,
                           bool only_meter_effects, bool only_appearance_effects,
                           bool include_empire_meter_effects,
                           bool only_generate_sitrep_effects) const
{
    // execute each effect of the group one by one, unless filtered by flags
    for (auto& effect : m_effects) {
        effect->Execute(targets_causes, accounting_map,
                        only_meter_effects, only_appearance_effects,
                        include_empire_meter_effects,
                        only_generate_sitrep_effects);
    }
}

const std::vector<EffectBase*>  EffectsGroup::EffectsList() const {
    std::vector<EffectBase*> retval(m_effects.size());
    std::transform(m_effects.begin(), m_effects.end(), retval.begin(),
                   [](const std::unique_ptr<EffectBase>& xx) {return xx.get();});
    return retval;
}

const std::string& EffectsGroup::GetDescription() const
{ return m_description; }

std::string EffectsGroup::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "EffectsGroup";
    if (!m_content_name.empty())
        retval += " // from " + m_content_name;
    retval += "\n";
    retval += DumpIndent(ntabs+1) + "scope =\n";
    retval += m_scope->Dump(ntabs+2);
    if (m_activation) {
        retval += DumpIndent(ntabs+1) + "activation =\n";
        retval += m_activation->Dump(ntabs+2);
    }
    if (!m_stacking_group.empty())
        retval += DumpIndent(ntabs+1) + "stackinggroup = \"" + m_stacking_group + "\"\n";
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

bool EffectsGroup::HasMeterEffects() const {
    for (auto& effect : m_effects) {
        if (effect->IsMeterEffect())
            return true;
    }
    return false;
}

bool EffectsGroup::HasAppearanceEffects() const {
    for (auto& effect : m_effects) {
        if (effect->IsAppearanceEffect())
            return true;
    }
    return false;
}

bool EffectsGroup::HasSitrepEffects() const {
    for (auto& effect : m_effects) {
        if (effect->IsSitrepEffect())
            return true;
    }
    return false;
}

void EffectsGroup::SetTopLevelContent(const std::string& content_name) {
    m_content_name = content_name;
    if (m_scope)
        m_scope->SetTopLevelContent(content_name);
    if (m_activation)
        m_activation->SetTopLevelContent(content_name);
    for (auto& effect : m_effects) {
        effect->SetTopLevelContent(content_name);
    }
}

unsigned int EffectsGroup::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "EffectsGroup");
    CheckSums::CheckSumCombine(retval, m_scope);
    CheckSums::CheckSumCombine(retval, m_activation);
    CheckSums::CheckSumCombine(retval, m_stacking_group);
    CheckSums::CheckSumCombine(retval, m_effects);
    CheckSums::CheckSumCombine(retval, m_accounting_label);
    CheckSums::CheckSumCombine(retval, m_priority);
    CheckSums::CheckSumCombine(retval, m_description);

    TraceLogger() << "GetCheckSum(EffectsGroup): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// Dump function                                         //
///////////////////////////////////////////////////////////
std::string Dump(const std::vector<std::shared_ptr<EffectsGroup>>& effects_groups) {
    std::stringstream retval;

    for (auto& effects_group : effects_groups) {
        retval << "\n" << effects_group->Dump();
    }

    return retval.str();
}


///////////////////////////////////////////////////////////
// EffectBase                                            //
///////////////////////////////////////////////////////////
EffectBase::~EffectBase()
{}

void EffectBase::Execute(const TargetsCauses& targets_causes,
                         AccountingMap* accounting_map,
                         bool only_meter_effects, bool only_appearance_effects,
                         bool include_empire_meter_effects,
                         bool only_generate_sitrep_effects) const
{
    if (   (only_appearance_effects      && !this->IsAppearanceEffect())
        || (only_meter_effects           && !this->IsMeterEffect())
        || (!include_empire_meter_effects && this->IsEmpireMeterEffect())
        || (only_generate_sitrep_effects && !this->IsSitrepEffect()))
    { return; }
    // apply this effect for each source causing it
    for (const auto& targets_entry : targets_causes) {
        ScriptingContext source_context(GetUniverseObject(targets_entry.first.source_object_id));
        Execute(source_context, targets_entry.second.target_set,
                accounting_map, targets_entry.second.effect_cause,
                only_meter_effects, only_appearance_effects,
                include_empire_meter_effects, only_generate_sitrep_effects);
    }
}

void EffectBase::Execute(const ScriptingContext& context,
                         const TargetSet& targets,
                         AccountingMap* accounting_map,
                         const EffectCause& effect_cause,
                         bool only_meter_effects, bool only_appearance_effects,
                         bool include_empire_meter_effects,
                         bool only_generate_sitrep_effects) const
{
    if (   (only_appearance_effects      && !this->IsAppearanceEffect())
        || (only_meter_effects           && !this->IsMeterEffect())
        || (!include_empire_meter_effects && this->IsEmpireMeterEffect())
        || (only_generate_sitrep_effects && !this->IsSitrepEffect()))
    { return; }
    // generic / most effects don't do anything special for accounting, so just
    // use standard Execute. overrides may implement something else.
    Execute(context, targets);
}

void EffectBase::Execute(const ScriptingContext& context, const TargetSet& targets) const {
    if (targets.empty())
        return;

    // execute effects on targets
    ScriptingContext local_context = context;
    for (const auto& target : targets) {
        local_context.effect_target = target;
        Execute(local_context);
    }
}

unsigned int EffectBase::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "EffectBase");

    TraceLogger() << "GetCheckSum(EffectsGroup): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// NoOp                                                  //
///////////////////////////////////////////////////////////
NoOp::NoOp()
{}

void NoOp::Execute(const ScriptingContext& context) const
{}

std::string NoOp::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "NoOp\n"; }

unsigned int NoOp::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "NoOp");

    TraceLogger() << "GetCheckSum(NoOp): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// SetMeter                                              //
///////////////////////////////////////////////////////////
SetMeter::SetMeter(MeterType meter,
                   std::unique_ptr<ValueRef::ValueRefBase<double>>&& value,
                   const boost::optional<std::string>& accounting_label) :
    m_meter(meter),
    m_value(std::move(value)),
    m_accounting_label(accounting_label ? *accounting_label : std::string())
{}

void SetMeter::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) return;
    Meter* m = context.effect_target->GetMeter(m_meter);
    if (!m) return;

    float val = m_value->Eval(ScriptingContext(context, m->Current()));
    m->SetCurrent(val);
}

void SetMeter::Execute(const ScriptingContext& context,
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

    TraceLogger(effects) << "\n\nExecute SetMeter effect: \n" << Dump();
    TraceLogger(effects) << "SetMeter execute targets before: ";
    for (const auto& target : targets)
        TraceLogger(effects) << " ... " << target->Dump(1);

    if (!accounting_map) {
        // without accounting, can do default batch execute
        Execute(context, targets);

    } else {
        // accounting info for this effect on this meter, starting with non-target-dependent info
        AccountingInfo info;
        info.cause_type =     effect_cause.cause_type;
        info.specific_cause = effect_cause.specific_cause;
        info.custom_label =   (m_accounting_label.empty() ? effect_cause.custom_label : m_accounting_label);
        info.source_id =      context.source->ID();

        // process each target separately in order to do effect accounting for each
        for (auto& target : targets) {
            // get Meter for this effect and target
            const Meter* meter = target->GetMeter(m_meter);
            if (!meter)
                continue;   // some objects might match target conditions, but not actually have the relevant meter. In that case, don't need to do accounting.

            // record pre-effect meter values...

            // accounting info for this effect on this meter of this target
            info.running_meter_total =  meter->Current();

            // actually execute effect to modify meter
            Execute(ScriptingContext(context.source, target));

            // update for meter change and new total
            info.meter_change = meter->Current() - info.running_meter_total;
            info.running_meter_total = meter->Current();

            // add accounting for this effect to end of vector
            (*accounting_map)[target->ID()][m_meter].push_back(info);
        }
    }

    TraceLogger(effects) << "SetMeter execute targets after: ";
    for (auto& target : targets)
        TraceLogger(effects) << " ... " << target->Dump();
}

void SetMeter::Execute(const ScriptingContext& context, const TargetSet& targets) const {
    if (targets.empty())
        return;

    if (m_value->TargetInvariant()) {
        // meter value does not depend on target, so handle with single ValueRef evaluation
        float val = m_value->Eval(context);
        for (auto& target : targets) {
            Meter* m = target->GetMeter(m_meter);
            if (!m) continue;
            m->SetCurrent(val);
        }
        return;

    } else if (m_value->SimpleIncrement()) {
        // meter value is a consistent constant increment for each target, so handle with
        // deep inspection single ValueRef evaluation
        auto op = dynamic_cast<ValueRef::Operation<double>*>(m_value.get());
        if (!op) {
            ErrorLogger() << "SetMeter::Execute couldn't cast simple increment ValueRef to an Operation. Reverting to standard execute.";
            EffectBase::Execute(context, targets);
            return;
        }
        // RHS should be a ConstantExpr
        float increment = op->RHS()->Eval();
        if (op->GetOpType() == ValueRef::PLUS) {
            // do nothing to modify increment
        } else if (op->GetOpType() == ValueRef::MINUS) {
            increment = -increment;
        } else {
            ErrorLogger() << "SetMeter::Execute got invalid increment optype (not PLUS or MINUS). Reverting to standard execute.";
            EffectBase::Execute(context, targets);
            return;
        }
        //DebugLogger() << "simple increment: " << increment;
        // increment all target meters...
        for (auto& target : targets) {
            Meter* m = target->GetMeter(m_meter);
            if (!m) continue;
            m->AddToCurrent(increment);
        }
        return;
    }

    // meter value depends on target non-trivially, so handle with default case of per-target ValueRef evaluation
    EffectBase::Execute(context, targets);
}

std::string SetMeter::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "Set";
    switch (m_meter) {
    case METER_TARGET_POPULATION:   retval += "TargetPopulation"; break;
    case METER_TARGET_INDUSTRY:     retval += "TargetIndustry"; break;
    case METER_TARGET_RESEARCH:     retval += "TargetResearch"; break;
    case METER_TARGET_TRADE:        retval += "TargetTrade"; break;
    case METER_TARGET_CONSTRUCTION: retval += "TargetConstruction"; break;
    case METER_TARGET_HAPPINESS:    retval += "TargetHappiness"; break;

    case METER_MAX_CAPACITY:        retval += "MaxCapacity"; break;

    case METER_MAX_FUEL:            retval += "MaxFuel"; break;
    case METER_MAX_SHIELD:          retval += "MaxShield"; break;
    case METER_MAX_STRUCTURE:       retval += "MaxStructure"; break;
    case METER_MAX_DEFENSE:         retval += "MaxDefense"; break;
    case METER_MAX_SUPPLY:          retval += "MaxSupply"; break;
    case METER_MAX_STOCKPILE:       retval += "MaxStockpile"; break;
    case METER_MAX_TROOPS:          retval += "MaxTroops"; break;

    case METER_POPULATION:          retval += "Population"; break;
    case METER_INDUSTRY:            retval += "Industry"; break;
    case METER_RESEARCH:            retval += "Research"; break;
    case METER_TRADE:               retval += "Trade"; break;
    case METER_CONSTRUCTION:        retval += "Construction"; break;
    case METER_HAPPINESS:           retval += "Happiness"; break;

    case METER_CAPACITY:            retval += "Capacity"; break;

    case METER_FUEL:                retval += "Fuel"; break;
    case METER_SHIELD:              retval += "Shield"; break;
    case METER_STRUCTURE:           retval += "Structure"; break;
    case METER_DEFENSE:             retval += "Defense"; break;
    case METER_SUPPLY:              retval += "Supply"; break;
    case METER_STOCKPILE:           retval += "Stockpile"; break;
    case METER_TROOPS:              retval += "Troops"; break;

    case METER_REBEL_TROOPS:        retval += "RebelTroops"; break;
    case METER_SIZE:                retval += "Size"; break;
    case METER_STEALTH:             retval += "Stealth"; break;
    case METER_DETECTION:           retval += "Detection"; break;
    case METER_SPEED:               retval += "Speed"; break;

    default:                        retval += "?"; break;
    }
    retval += " value = " + m_value->Dump(ntabs) + "\n";
    return retval;
}

void SetMeter::SetTopLevelContent(const std::string& content_name) {
    if (m_value)
        m_value->SetTopLevelContent(content_name);
}

unsigned int SetMeter::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "SetMeter");
    CheckSums::CheckSumCombine(retval, m_meter);
    CheckSums::CheckSumCombine(retval, m_value);
    CheckSums::CheckSumCombine(retval, m_accounting_label);

    TraceLogger() << "GetCheckSum(SetMeter): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// SetShipPartMeter                                      //
///////////////////////////////////////////////////////////
SetShipPartMeter::SetShipPartMeter(MeterType meter,
                                   std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& part_name,
                                   std::unique_ptr<ValueRef::ValueRefBase<double>>&& value) :
    m_part_name(std::move(part_name)),
    m_meter(meter),
    m_value(std::move(value))
{}

void SetShipPartMeter::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        DebugLogger() << "SetShipPartMeter::Execute passed null target pointer";
        return;
    }

    if (!m_part_name || !m_value) {
        ErrorLogger() << "SetShipPartMeter::Execute missing part name or value ValueRefs";
        return;
    }

    auto ship = std::dynamic_pointer_cast<Ship>(context.effect_target);
    if (!ship) {
        ErrorLogger() << "SetShipPartMeter::Execute acting on non-ship target:";
        //context.effect_target->Dump();
        return;
    }

    std::string part_name = m_part_name->Eval(context);

    // get meter, evaluate new value, assign
    Meter* meter = ship->GetPartMeter(m_meter, part_name);
    if (!meter)
        return;

    double val = m_value->Eval(ScriptingContext(context, meter->Current()));
    meter->SetCurrent(val);
}

void SetShipPartMeter::Execute(const ScriptingContext& context,
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

    TraceLogger(effects) << "\n\nExecute SetShipPartMeter effect: \n" << Dump();
    TraceLogger(effects) << "SetShipPartMeter execute targets before: ";
    for (auto& target : targets)
        TraceLogger(effects) << " ... " << target->Dump(1);

    Execute(context, targets);

    TraceLogger(effects) << "SetShipPartMeter execute targets after: ";
    for (auto& target : targets)
        TraceLogger(effects) << " ... " << target->Dump(1);
}

void SetShipPartMeter::Execute(const ScriptingContext& context, const TargetSet& targets) const {
    if (targets.empty())
        return;
    if (!m_part_name || !m_value) {
        ErrorLogger() << "SetShipPartMeter::Execute missing part name or value ValueRefs";
        return;
    }

    // TODO: Handle efficiently the case where the part name varies from target
    // to target, but the value is target-invariant
    if (!m_part_name->TargetInvariant()) {
        DebugLogger() << "SetShipPartMeter::Execute has target-variant part name, which it is not (yet) coded to handle efficiently!";
        EffectBase::Execute(context, targets);
        return;
    }

    // part name doesn't depend on target, so handle with single ValueRef evaluation
    std::string part_name = m_part_name->Eval(context);

    if (m_value->TargetInvariant()) {
        // meter value does not depend on target, so handle with single ValueRef evaluation
        float val = m_value->Eval(context);
        for (auto& target : targets) {
            if (target->ObjectType() != OBJ_SHIP)
                continue;
            auto ship = std::dynamic_pointer_cast<Ship>(target);
            if (!ship)
                continue;
            Meter* m = ship->GetPartMeter(m_meter, part_name);
            if (m)
                m->SetCurrent(val);
        }
        return;

    } else if (m_value->SimpleIncrement()) {
        // meter value is a consistent constant increment for each target, so handle with
        // deep inspection single ValueRef evaluation
        auto op = dynamic_cast<ValueRef::Operation<double>*>(m_value.get());
        if (!op) {
            ErrorLogger() << "SetShipPartMeter::Execute couldn't cast simple increment ValueRef to an Operation. Reverting to standard execute.";
            EffectBase::Execute(context, targets);
            return;
        }
        // RHS should be a ConstantExpr
        float increment = op->RHS()->Eval();
        if (op->GetOpType() == ValueRef::PLUS) {
            // do nothing to modify increment
        } else if (op->GetOpType() == ValueRef::MINUS) {
            increment = -increment;
        } else {
            ErrorLogger() << "SetShipPartMeter::Execute got invalid increment optype (not PLUS or MINUS). Reverting to standard execute.";
            EffectBase::Execute(context, targets);
            return;
        }
        //DebugLogger() << "simple increment: " << increment;
        // increment all target meters...
        for (auto& target : targets) {
            if (target->ObjectType() != OBJ_SHIP)
                continue;
            auto ship = std::dynamic_pointer_cast<Ship>(target);
            if (!ship)
                continue;
            Meter* m = ship->GetPartMeter(m_meter, part_name);
            if (m)
                m->AddToCurrent(increment);
        }
        return;

    } else {
        //DebugLogger() << "complicated meter adjustment...";
        // meter value depends on target non-trivially, so handle with default case of per-target ValueRef evaluation
        EffectBase::Execute(context, targets);
    }
}

std::string SetShipPartMeter::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs);
    switch (m_meter) {
        case METER_CAPACITY:            retval += "SetCapacity";        break;
        case METER_MAX_CAPACITY:        retval += "SetMaxCapacity";     break;
        case METER_SECONDARY_STAT:      retval += "SetSecondaryStat";   break;
        case METER_MAX_SECONDARY_STAT:  retval += "SetMaxSecondaryStat";break;
        default:                retval += "Set???";         break;
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

unsigned int SetShipPartMeter::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "SetShipPartMeter");
    CheckSums::CheckSumCombine(retval, m_part_name);
    CheckSums::CheckSumCombine(retval, m_meter);
    CheckSums::CheckSumCombine(retval, m_value);

    TraceLogger() << "GetCheckSum(SetShipPartMeter): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// SetEmpireMeter                                        //
///////////////////////////////////////////////////////////
SetEmpireMeter::SetEmpireMeter(const std::string& meter, std::unique_ptr<ValueRef::ValueRefBase<double>>&& value) :
    m_empire_id(boost::make_unique<ValueRef::Variable<int>>(ValueRef::EFFECT_TARGET_REFERENCE, std::vector<std::string>(1, "Owner"))),
    m_meter(meter),
    m_value(std::move(value))
{}

SetEmpireMeter::SetEmpireMeter(std::unique_ptr<ValueRef::ValueRefBase<int>>&& empire_id, const std::string& meter,
                               std::unique_ptr<ValueRef::ValueRefBase<double>>&& value) :
    m_empire_id(std::move(empire_id)),
    m_meter(meter),
    m_value(std::move(value))
{}

void SetEmpireMeter::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        DebugLogger() << "SetEmpireMeter::Execute passed null target pointer";
        return;
    }
    if (!m_empire_id || !m_value || m_meter.empty()) {
        ErrorLogger() << "SetEmpireMeter::Execute missing empire id or value ValueRefs, or given empty meter name";
        return;
    }

    int empire_id = m_empire_id->Eval(context);
    Empire* empire = GetEmpire(empire_id);
    if (!empire) {
        DebugLogger() << "SetEmpireMeter::Execute unable to find empire with id " << empire_id;
        return;
    }

    Meter* meter = empire->GetMeter(m_meter);
    if (!meter) {
        DebugLogger() << "SetEmpireMeter::Execute empire " << empire->Name() << " doesn't have a meter named " << m_meter;
        return;
    }

    double&& value = m_value->Eval(ScriptingContext(context, meter->Current()));

    meter->SetCurrent(value);
}

void SetEmpireMeter::Execute(const ScriptingContext& context,
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

void SetEmpireMeter::Execute(const ScriptingContext& context, const TargetSet& targets) const {
    if (targets.empty())
        return;
    if (!m_empire_id || m_meter.empty() || !m_value) {
        ErrorLogger() << "SetEmpireMeter::Execute missing empire id or value ValueRefs or meter name";
        return;
    }

    // TODO: efficiently handle target invariant empire id and value
    EffectBase::Execute(context, targets);
    //return;

    //if (m_empire_id->TargetInvariant() && m_value->TargetInvariant()) {
    //}

    ////DebugLogger() << "complicated meter adjustment...";
    //// meter value depends on target non-trivially, so handle with default case of per-target ValueRef evaluation
    //EffectBase::Execute(context, targets);
}

std::string SetEmpireMeter::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "SetEmpireMeter meter = " + m_meter + " empire = " + m_empire_id->Dump(ntabs) + " value = " + m_value->Dump(ntabs); }

void SetEmpireMeter::SetTopLevelContent(const std::string& content_name) {
    if (m_empire_id)
        m_empire_id->SetTopLevelContent(content_name);
    if (m_value)
        m_value->SetTopLevelContent(content_name);
}

unsigned int SetEmpireMeter::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "SetEmpireMeter");
    CheckSums::CheckSumCombine(retval, m_empire_id);
    CheckSums::CheckSumCombine(retval, m_meter);
    CheckSums::CheckSumCombine(retval, m_value);

    TraceLogger() << "GetCheckSum(SetEmpireMeter): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// SetEmpireStockpile                                    //
///////////////////////////////////////////////////////////
SetEmpireStockpile::SetEmpireStockpile(ResourceType stockpile,
                                       std::unique_ptr<ValueRef::ValueRefBase<double>>&& value) :
    m_empire_id(boost::make_unique<ValueRef::Variable<int>>(ValueRef::EFFECT_TARGET_REFERENCE, std::vector<std::string>(1, "Owner"))),
    m_stockpile(stockpile),
    m_value(std::move(value))
{}

SetEmpireStockpile::SetEmpireStockpile(std::unique_ptr<ValueRef::ValueRefBase<int>>&& empire_id,
                                       ResourceType stockpile,
                                       std::unique_ptr<ValueRef::ValueRefBase<double>>&& value) :
    m_empire_id(std::move(empire_id)),
    m_stockpile(stockpile),
    m_value(std::move(value))
{}

void SetEmpireStockpile::Execute(const ScriptingContext& context) const {
    int empire_id = m_empire_id->Eval(context);

    Empire* empire = GetEmpire(empire_id);
    if (!empire) {
        DebugLogger() << "SetEmpireStockpile::Execute couldn't find an empire with id " << empire_id;
        return;
    }

    double value = m_value->Eval(ScriptingContext(context, empire->ResourceStockpile(m_stockpile)));
    empire->SetResourceStockpile(m_stockpile, value);
}

std::string SetEmpireStockpile::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs);
    switch (m_stockpile) {
        // TODO: Support for other resource stockpiles?
    case RE_INDUSTRY:   retval += "SetEmpireStockpile"; break;
    default:            retval += "?"; break;
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

unsigned int SetEmpireStockpile::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "SetEmpireStockpile");
    CheckSums::CheckSumCombine(retval, m_empire_id);
    CheckSums::CheckSumCombine(retval, m_stockpile);
    CheckSums::CheckSumCombine(retval, m_value);

    TraceLogger() << "GetCheckSum(SetEmpireStockpile): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// SetEmpireCapital                                      //
///////////////////////////////////////////////////////////
SetEmpireCapital::SetEmpireCapital() :
    m_empire_id(boost::make_unique<ValueRef::Variable<int>>(ValueRef::EFFECT_TARGET_REFERENCE, std::vector<std::string>(1, "Owner")))
{}

SetEmpireCapital::SetEmpireCapital(std::unique_ptr<ValueRef::ValueRefBase<int>>&& empire_id) :
    m_empire_id(std::move(empire_id))
{}

void SetEmpireCapital::Execute(const ScriptingContext& context) const {
    int empire_id = m_empire_id->Eval(context);

    Empire* empire = GetEmpire(empire_id);
    if (!empire)
        return;

    auto planet = std::dynamic_pointer_cast<const Planet>(context.effect_target);
    if (!planet)
        return;

    empire->SetCapitalID(planet->ID());
}

std::string SetEmpireCapital::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "SetEmpireCapital empire = " + m_empire_id->Dump(ntabs) + "\n"; }

void SetEmpireCapital::SetTopLevelContent(const std::string& content_name) {
    if (m_empire_id)
        m_empire_id->SetTopLevelContent(content_name);
}

unsigned int SetEmpireCapital::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "SetEmpireCapital");
    CheckSums::CheckSumCombine(retval, m_empire_id);

    TraceLogger() << "GetCheckSum(SetEmpireCapital): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// SetPlanetType                                         //
///////////////////////////////////////////////////////////
SetPlanetType::SetPlanetType(std::unique_ptr<ValueRef::ValueRefBase<PlanetType>>&& type) :
    m_type(std::move(type))
{}

void SetPlanetType::Execute(const ScriptingContext& context) const {
    if (auto p = std::dynamic_pointer_cast<Planet>(context.effect_target)) {
        PlanetType type = m_type->Eval(ScriptingContext(context, p->Type()));
        p->SetType(type);
        if (type == PT_ASTEROIDS)
            p->SetSize(SZ_ASTEROIDS);
        else if (type == PT_GASGIANT)
            p->SetSize(SZ_GASGIANT);
        else if (p->Size() == SZ_ASTEROIDS)
            p->SetSize(SZ_TINY);
        else if (p->Size() == SZ_GASGIANT)
            p->SetSize(SZ_HUGE);
    }
}

std::string SetPlanetType::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "SetPlanetType type = " + m_type->Dump(ntabs) + "\n"; }

void SetPlanetType::SetTopLevelContent(const std::string& content_name) {
    if (m_type)
        m_type->SetTopLevelContent(content_name);
}

unsigned int SetPlanetType::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "SetPlanetType");
    CheckSums::CheckSumCombine(retval, m_type);

    TraceLogger() << "GetCheckSum(SetPlanetType): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// SetPlanetSize                                         //
///////////////////////////////////////////////////////////
SetPlanetSize::SetPlanetSize(std::unique_ptr<ValueRef::ValueRefBase<PlanetSize>>&& size) :
    m_size(std::move(size))
{}

void SetPlanetSize::Execute(const ScriptingContext& context) const {
    if (auto p = std::dynamic_pointer_cast<Planet>(context.effect_target)) {
        PlanetSize size = m_size->Eval(ScriptingContext(context, p->Size()));
        p->SetSize(size);
        if (size == SZ_ASTEROIDS)
            p->SetType(PT_ASTEROIDS);
        else if (size == SZ_GASGIANT)
            p->SetType(PT_GASGIANT);
        else if (p->Type() == PT_ASTEROIDS || p->Type() == PT_GASGIANT)
            p->SetType(PT_BARREN);
    }
}

std::string SetPlanetSize::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "SetPlanetSize size = " + m_size->Dump(ntabs) + "\n"; }

void SetPlanetSize::SetTopLevelContent(const std::string& content_name) {
    if (m_size)
        m_size->SetTopLevelContent(content_name);
}

unsigned int SetPlanetSize::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "SetPlanetSize");
    CheckSums::CheckSumCombine(retval, m_size);

    TraceLogger() << "GetCheckSum(SetPlanetSize): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// SetSpecies                                            //
///////////////////////////////////////////////////////////
SetSpecies::SetSpecies(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& species) :
    m_species_name(std::move(species))
{}

void SetSpecies::Execute(const ScriptingContext& context) const {
    if (auto planet = std::dynamic_pointer_cast<Planet>(context.effect_target)) {
        std::string species_name = m_species_name->Eval(ScriptingContext(context, planet->SpeciesName()));
        planet->SetSpecies(species_name);

        // ensure non-empty and permissible focus setting for new species
        std::string initial_focus = planet->Focus();
        std::vector<std::string> available_foci = planet->AvailableFoci();

        // leave current focus unchanged if available.
        for (const std::string& available_focus : available_foci) {
            if (available_focus == initial_focus) {
                return;
            }
        }

        // need to set new focus
        std::string new_focus;

        const Species* species = GetSpecies(species_name);
        std::string preferred_focus;
        if (species)
            preferred_focus = species->PreferredFocus();

        // chose preferred focus if available. otherwise use any available focus
        bool preferred_available = false;
        for (const std::string& available_focus : available_foci) {
            if (available_focus == preferred_focus) {
                preferred_available = true;
                break;
            }
        }

        if (preferred_available) {
            new_focus = std::move(preferred_focus);
        } else if (!available_foci.empty()) {
            new_focus = *available_foci.begin();
        }

        planet->SetFocus(new_focus);

    } else if (auto ship = std::dynamic_pointer_cast<Ship>(context.effect_target)) {
        std::string species_name = m_species_name->Eval(ScriptingContext(context, ship->SpeciesName()));
        ship->SetSpecies(species_name);
    }
}

std::string SetSpecies::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "SetSpecies name = " + m_species_name->Dump(ntabs) + "\n"; }

void SetSpecies::SetTopLevelContent(const std::string& content_name) {
    if (m_species_name)
        m_species_name->SetTopLevelContent(content_name);
}

unsigned int SetSpecies::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "SetSpecies");
    CheckSums::CheckSumCombine(retval, m_species_name);

    TraceLogger() << "GetCheckSum(SetSpecies): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// SetOwner                                              //
///////////////////////////////////////////////////////////
SetOwner::SetOwner(std::unique_ptr<ValueRef::ValueRefBase<int>>&& empire_id) :
    m_empire_id(std::move(empire_id))
{}

void SetOwner::Execute(const ScriptingContext& context) const {
    if (!context.effect_target)
        return;
    int initial_owner = context.effect_target->Owner();

    int empire_id = m_empire_id->Eval(ScriptingContext(context, initial_owner));
    if (initial_owner == empire_id)
        return;

    context.effect_target->SetOwner(empire_id);

    if (auto ship = std::dynamic_pointer_cast<Ship>(context.effect_target)) {
        // assigning ownership of a ship requires updating the containing
        // fleet, or splitting ship off into a new fleet at the same location
        auto fleet = GetFleet(ship->FleetID());
        if (!fleet)
            return;
        if (fleet->Owner() == empire_id)
            return;

        // move ship into new fleet
        std::shared_ptr<Fleet> new_fleet;
        if (auto system = GetSystem(ship->SystemID()))
            new_fleet = CreateNewFleet(system, ship);
        else
            new_fleet = CreateNewFleet(ship->X(), ship->Y(), ship);
        if (new_fleet) {
            new_fleet->SetNextAndPreviousSystems(fleet->NextSystemID(), fleet->PreviousSystemID());
        }

        // if old fleet is empty, destroy it.  Don't reassign ownership of fleet
        // in case that would reval something to the recipient that shouldn't be...
        if (fleet->Empty())
            GetUniverse().EffectDestroy(fleet->ID(), INVALID_OBJECT_ID);    // no particular source destroyed the fleet in this case
    }
}

std::string SetOwner::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "SetOwner empire = " + m_empire_id->Dump(ntabs) + "\n"; }

void SetOwner::SetTopLevelContent(const std::string& content_name) {
    if (m_empire_id)
        m_empire_id->SetTopLevelContent(content_name);
}

unsigned int SetOwner::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "SetOwner");
    CheckSums::CheckSumCombine(retval, m_empire_id);

    TraceLogger() << "GetCheckSum(SetOwner): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// SetSpeciesEmpireOpinion                               //
///////////////////////////////////////////////////////////
SetSpeciesEmpireOpinion::SetSpeciesEmpireOpinion(
    std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& species_name,
    std::unique_ptr<ValueRef::ValueRefBase<int>>&& empire_id,
    std::unique_ptr<ValueRef::ValueRefBase<double>>&& opinion
) :
    m_species_name(std::move(species_name)),
    m_empire_id(std::move(empire_id)),
    m_opinion(std::move(opinion))
{}

void SetSpeciesEmpireOpinion::Execute(const ScriptingContext& context) const {
    if (!context.effect_target)
        return;
    if (!m_species_name || !m_opinion || !m_empire_id)
        return;

    int empire_id = m_empire_id->Eval(context);
    if (empire_id == ALL_EMPIRES)
        return;

    std::string species_name = m_species_name->Eval(context);
    if (species_name.empty())
        return;

    double initial_opinion = GetSpeciesManager().SpeciesEmpireOpinion(species_name, empire_id);
    double opinion = m_opinion->Eval(ScriptingContext(context, initial_opinion));

    GetSpeciesManager().SetSpeciesEmpireOpinion(species_name, empire_id, opinion);
}

std::string SetSpeciesEmpireOpinion::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "SetSpeciesEmpireOpinion empire = " + m_empire_id->Dump(ntabs) + "\n"; }

void SetSpeciesEmpireOpinion::SetTopLevelContent(const std::string& content_name) {
    if (m_empire_id)
        m_empire_id->SetTopLevelContent(content_name);
    if (m_species_name)
        m_species_name->SetTopLevelContent(content_name);
    if (m_opinion)
        m_opinion->SetTopLevelContent(content_name);
}

unsigned int SetSpeciesEmpireOpinion::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "SetSpeciesEmpireOpinion");
    CheckSums::CheckSumCombine(retval, m_species_name);
    CheckSums::CheckSumCombine(retval, m_empire_id);
    CheckSums::CheckSumCombine(retval, m_opinion);

    TraceLogger() << "GetCheckSum(SetSpeciesEmpireOpinion): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// SetSpeciesSpeciesOpinion                              //
///////////////////////////////////////////////////////////
SetSpeciesSpeciesOpinion::SetSpeciesSpeciesOpinion(
    std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& opinionated_species_name,
    std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& rated_species_name,
    std::unique_ptr<ValueRef::ValueRefBase<double>>&& opinion
) :
    m_opinionated_species_name(std::move(opinionated_species_name)),
    m_rated_species_name(std::move(rated_species_name)),
    m_opinion(std::move(opinion))
{}

void SetSpeciesSpeciesOpinion::Execute(const ScriptingContext& context) const {
    if (!context.effect_target)
        return;
    if (!m_opinionated_species_name || !m_opinion || !m_rated_species_name)
        return;

    std::string opinionated_species_name = m_opinionated_species_name->Eval(context);
    if (opinionated_species_name.empty())
        return;

    std::string rated_species_name = m_rated_species_name->Eval(context);
    if (rated_species_name.empty())
        return;

    float initial_opinion = GetSpeciesManager().SpeciesSpeciesOpinion(opinionated_species_name, rated_species_name);
    float opinion = m_opinion->Eval(ScriptingContext(context, initial_opinion));

    GetSpeciesManager().SetSpeciesSpeciesOpinion(opinionated_species_name, rated_species_name, opinion);
}

std::string SetSpeciesSpeciesOpinion::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "SetSpeciesSpeciesOpinion" + "\n"; }

void SetSpeciesSpeciesOpinion::SetTopLevelContent(const std::string& content_name) {
    if (m_opinionated_species_name)
        m_opinionated_species_name->SetTopLevelContent(content_name);
    if (m_rated_species_name)
        m_rated_species_name->SetTopLevelContent(content_name);
    if (m_opinion)
        m_opinion->SetTopLevelContent(content_name);
}

unsigned int SetSpeciesSpeciesOpinion::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "SetSpeciesSpeciesOpinion");
    CheckSums::CheckSumCombine(retval, m_opinionated_species_name);
    CheckSums::CheckSumCombine(retval, m_rated_species_name);
    CheckSums::CheckSumCombine(retval, m_opinion);

    TraceLogger() << "GetCheckSum(SetSpeciesSpeciesOpinion): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// CreatePlanet                                          //
///////////////////////////////////////////////////////////
CreatePlanet::CreatePlanet(std::unique_ptr<ValueRef::ValueRefBase<PlanetType>>&& type,
                           std::unique_ptr<ValueRef::ValueRefBase<PlanetSize>>&& size,
                           std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& name,
                           std::vector<std::unique_ptr<EffectBase>>&& effects_to_apply_after) :
    m_type(std::move(type)),
    m_size(std::move(size)),
    m_name(std::move(name)),
    m_effects_to_apply_after(std::move(effects_to_apply_after))
{}

void CreatePlanet::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger() << "CreatePlanet::Execute passed no target object";
        return;
    }
    auto system = GetSystem(context.effect_target->SystemID());
    if (!system) {
        ErrorLogger() << "CreatePlanet::Execute couldn't get a System object at which to create the planet";
        return;
    }

    PlanetSize target_size = INVALID_PLANET_SIZE;
    PlanetType target_type = INVALID_PLANET_TYPE;
    if (auto location_planet = std::dynamic_pointer_cast<const Planet>(context.effect_target)) {
        target_size = location_planet->Size();
        target_type = location_planet->Type();
    }

    PlanetSize size = m_size->Eval(ScriptingContext(context, target_size));
    PlanetType type = m_type->Eval(ScriptingContext(context, target_type));
    if (size == INVALID_PLANET_SIZE || type == INVALID_PLANET_TYPE) {
        ErrorLogger() << "CreatePlanet::Execute got invalid size or type of planet to create...";
        return;
    }

    // determine if and which orbits are available
    std::set<int> free_orbits = system->FreeOrbits();
    if (free_orbits.empty()) {
        ErrorLogger() << "CreatePlanet::Execute couldn't find any free orbits in system where planet was to be created";
        return;
    }

    auto planet = GetUniverse().InsertNew<Planet>(type, size);
    if (!planet) {
        ErrorLogger() << "CreatePlanet::Execute unable to create new Planet object";
        return;
    }

    system->Insert(planet);   // let system chose an orbit for planet

    std::string name_str;
    if (m_name) {
        name_str = m_name->Eval(context);
        if (m_name->ConstantExpr() && UserStringExists(name_str))
            name_str = UserString(name_str);
    } else {
        name_str = str(FlexibleFormat(UserString("NEW_PLANET_NAME")) % system->Name() % planet->CardinalSuffix());
    }
    planet->Rename(name_str);

    // apply after-creation effects
    ScriptingContext local_context = context;
    local_context.effect_target = planet;
    for (auto& effect : m_effects_to_apply_after) {
        if (!effect)
            continue;
        effect->Execute(local_context);
    }
}

std::string CreatePlanet::Dump(unsigned short ntabs) const {
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

unsigned int CreatePlanet::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "CreatePlanet");
    CheckSums::CheckSumCombine(retval, m_type);
    CheckSums::CheckSumCombine(retval, m_size);
    CheckSums::CheckSumCombine(retval, m_name);
    CheckSums::CheckSumCombine(retval, m_effects_to_apply_after);

    TraceLogger() << "GetCheckSum(CreatePlanet): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// CreateBuilding                                        //
///////////////////////////////////////////////////////////
CreateBuilding::CreateBuilding(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& building_type_name,
                               std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& name,
                               std::vector<std::unique_ptr<EffectBase>>&& effects_to_apply_after) :
    m_building_type_name(std::move(building_type_name)),
    m_name(std::move(name)),
    m_effects_to_apply_after(std::move(effects_to_apply_after))
{}

void CreateBuilding::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger() << "CreateBuilding::Execute passed no target object";
        return;
    }
    auto location = std::dynamic_pointer_cast<Planet>(context.effect_target);
    if (!location)
        if (auto location_building = std::dynamic_pointer_cast<Building>(context.effect_target))
            location = GetPlanet(location_building->PlanetID());
    if (!location) {
        ErrorLogger() << "CreateBuilding::Execute couldn't get a Planet object at which to create the building";
        return;
    }

    if (!m_building_type_name) {
        ErrorLogger() << "CreateBuilding::Execute has no building type specified!";
        return;
    }

    std::string building_type_name = m_building_type_name->Eval(context);
    const BuildingType* building_type = GetBuildingType(building_type_name);
    if (!building_type) {
        ErrorLogger() << "CreateBuilding::Execute couldn't get building type: " << building_type_name;
        return;
    }

    auto building = GetUniverse().InsertNew<Building>(ALL_EMPIRES, building_type_name, ALL_EMPIRES);
    if (!building) {
        ErrorLogger() << "CreateBuilding::Execute couldn't create building!";
        return;
    }

    location->AddBuilding(building->ID());
    building->SetPlanetID(location->ID());

    building->SetOwner(location->Owner());

    auto system = GetSystem(location->SystemID());
    if (system)
        system->Insert(building);

    if (m_name) {
        std::string name_str = m_name->Eval(context);
        if (m_name->ConstantExpr() && UserStringExists(name_str))
            name_str = UserString(name_str);
        building->Rename(name_str);
    }

    // apply after-creation effects
    ScriptingContext local_context = context;
    local_context.effect_target = building;
    for (auto& effect : m_effects_to_apply_after) {
        if (!effect)
            continue;
        effect->Execute(local_context);
    }
}

std::string CreateBuilding::Dump(unsigned short ntabs) const {
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

unsigned int CreateBuilding::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "CreateBuilding");
    CheckSums::CheckSumCombine(retval, m_building_type_name);
    CheckSums::CheckSumCombine(retval, m_name);
    CheckSums::CheckSumCombine(retval, m_effects_to_apply_after);

    TraceLogger() << "GetCheckSum(CreateBuilding): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// CreateShip                                            //
///////////////////////////////////////////////////////////
CreateShip::CreateShip(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& predefined_ship_design_name,
                       std::unique_ptr<ValueRef::ValueRefBase<int>>&& empire_id,
                       std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& species_name,
                       std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& ship_name,
                       std::vector<std::unique_ptr<EffectBase>>&& effects_to_apply_after) :
    m_design_name(std::move(predefined_ship_design_name)),
    m_design_id(nullptr),
    m_empire_id(std::move(empire_id)),
    m_species_name(std::move(species_name)),
    m_name(std::move(ship_name)),
    m_effects_to_apply_after(std::move(effects_to_apply_after))
{}

CreateShip::CreateShip(std::unique_ptr<ValueRef::ValueRefBase<int>>&& ship_design_id,
                       std::unique_ptr<ValueRef::ValueRefBase<int>>&& empire_id,
                       std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& species_name,
                       std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& ship_name,
                       std::vector<std::unique_ptr<EffectBase>>&& effects_to_apply_after) :
    m_design_name(nullptr),
    m_design_id(std::move(ship_design_id)),
    m_empire_id(std::move(empire_id)),
    m_species_name(std::move(species_name)),
    m_name(std::move(ship_name)),
    m_effects_to_apply_after(std::move(effects_to_apply_after))
{}

void CreateShip::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger() << "CreateShip::Execute passed null target";
        return;
    }

    auto system = GetSystem(context.effect_target->SystemID());
    if (!system) {
        ErrorLogger() << "CreateShip::Execute passed a target not in a system";
        return;
    }

    int design_id = INVALID_DESIGN_ID;
    if (m_design_id) {
        design_id = m_design_id->Eval(context);
        if (!GetShipDesign(design_id)) {
            ErrorLogger() << "CreateShip::Execute couldn't get ship design with id: " << design_id;
            return;
        }
    } else if (m_design_name) {
        std::string design_name = m_design_name->Eval(context);
        const ShipDesign* ship_design = GetPredefinedShipDesign(design_name);
        if (!ship_design) {
            ErrorLogger() << "CreateShip::Execute couldn't get predefined ship design with name " << m_design_name->Dump();
            return;
        }
        design_id = ship_design->ID();
    }
    if (design_id == INVALID_DESIGN_ID) {
        ErrorLogger() << "CreateShip::Execute got invalid ship design id: -1";
        return;
    }

    int empire_id = ALL_EMPIRES;
    Empire* empire = nullptr;  // not const Empire* so that empire::NewShipName can be called
    if (m_empire_id) {
        empire_id = m_empire_id->Eval(context);
        if (empire_id != ALL_EMPIRES) {
            empire = GetEmpire(empire_id);
            if (!empire) {
                ErrorLogger() << "CreateShip::Execute couldn't get empire with id " << empire_id;
                return;
            }
        }
    }

    std::string species_name;
    if (m_species_name) {
        species_name = m_species_name->Eval(context);
        if (!species_name.empty() && !GetSpecies(species_name)) {
            ErrorLogger() << "CreateShip::Execute couldn't get species with which to create a ship";
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

    auto ship = GetUniverse().InsertNew<Ship>(empire_id, design_id, species_name, ALL_EMPIRES);
    system->Insert(ship);

    if (m_name) {
        std::string name_str = m_name->Eval(context);
        if (m_name->ConstantExpr() && UserStringExists(name_str))
            name_str = UserString(name_str);
        ship->Rename(name_str);
    } else if (ship->IsMonster()) {
        ship->Rename(NewMonsterName());
    } else if (empire) {
        ship->Rename(empire->NewShipName());
    } else {
        ship->Rename(ship->Design()->Name());
    }

    ship->ResetTargetMaxUnpairedMeters();
    ship->ResetPairedActiveMeters();
    ship->SetShipMetersToMax();

    ship->BackPropagateMeters();

    GetUniverse().SetEmpireKnowledgeOfShipDesign(design_id, empire_id);

    CreateNewFleet(system, ship);

    // apply after-creation effects
    ScriptingContext local_context = context;
    local_context.effect_target = ship;
    for (auto& effect : m_effects_to_apply_after) {
        if (!effect)
            continue;
        effect->Execute(local_context);
    }
}

std::string CreateShip::Dump(unsigned short ntabs) const {
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

unsigned int CreateShip::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "CreateShip");
    CheckSums::CheckSumCombine(retval, m_design_name);
    CheckSums::CheckSumCombine(retval, m_design_id);
    CheckSums::CheckSumCombine(retval, m_empire_id);
    CheckSums::CheckSumCombine(retval, m_species_name);
    CheckSums::CheckSumCombine(retval, m_name);
    CheckSums::CheckSumCombine(retval, m_effects_to_apply_after);

    TraceLogger() << "GetCheckSum(CreateShip): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// CreateField                                           //
///////////////////////////////////////////////////////////
CreateField::CreateField(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& field_type_name,
                         std::unique_ptr<ValueRef::ValueRefBase<double>>&& size,
                         std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& name,
                         std::vector<std::unique_ptr<EffectBase>>&& effects_to_apply_after) :
    m_field_type_name(std::move(field_type_name)),
    m_x(nullptr),
    m_y(nullptr),
    m_size(std::move(size)),
    m_name(std::move(name)),
    m_effects_to_apply_after(std::move(effects_to_apply_after))
{}

CreateField::CreateField(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& field_type_name,
                         std::unique_ptr<ValueRef::ValueRefBase<double>>&& x,
                         std::unique_ptr<ValueRef::ValueRefBase<double>>&& y,
                         std::unique_ptr<ValueRef::ValueRefBase<double>>&& size,
                         std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& name,
                         std::vector<std::unique_ptr<EffectBase>>&& effects_to_apply_after) :
    m_field_type_name(std::move(field_type_name)),
    m_x(std::move(x)),
    m_y(std::move(y)),
    m_size(std::move(size)),
    m_name(std::move(name)),
    m_effects_to_apply_after(std::move(effects_to_apply_after))
{}

void CreateField::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger() << "CreateField::Execute passed null target";
        return;
    }
    auto target = context.effect_target;

    if (!m_field_type_name)
        return;

    const FieldType* field_type = GetFieldType(m_field_type_name->Eval(context));
    if (!field_type) {
        ErrorLogger() << "CreateField::Execute couldn't get field type with name: " << m_field_type_name->Dump();
        return;
    }

    double size = 10.0;
    if (m_size)
        size = m_size->Eval(context);
    if (size < 1.0) {
        ErrorLogger() << "CreateField::Execute given very small / negative size: " << size << "  ... so resetting to 1.0";
        size = 1.0;
    }
    if (size > 10000) {
        ErrorLogger() << "CreateField::Execute given very large size: " << size << "  ... so resetting to 10000";
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

    auto field = GetUniverse().InsertNew<Field>(field_type->Name(), x, y, size);
    if (!field) {
        ErrorLogger() << "CreateField::Execute couldn't create field!";
        return;
    }

    // if target is a system, and location matches system location, can put
    // field into system
    auto system = std::dynamic_pointer_cast<System>(target);
    if (system && (!m_y || y == system->Y()) && (!m_x || x == system->X()))
        system->Insert(field);

    std::string name_str;
    if (m_name) {
        name_str = m_name->Eval(context);
        if (m_name->ConstantExpr() && UserStringExists(name_str))
            name_str = UserString(name_str);
    } else {
        name_str = UserString(field_type->Name());
    }
    field->Rename(name_str);

    // apply after-creation effects
    ScriptingContext local_context = context;
    local_context.effect_target = field;
    for (auto& effect : m_effects_to_apply_after) {
        if (!effect)
            continue;
        effect->Execute(local_context);
    }
}

std::string CreateField::Dump(unsigned short ntabs) const {
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

unsigned int CreateField::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "CreateField");
    CheckSums::CheckSumCombine(retval, m_field_type_name);
    CheckSums::CheckSumCombine(retval, m_x);
    CheckSums::CheckSumCombine(retval, m_y);
    CheckSums::CheckSumCombine(retval, m_size);
    CheckSums::CheckSumCombine(retval, m_name);
    CheckSums::CheckSumCombine(retval, m_effects_to_apply_after);

    TraceLogger() << "GetCheckSum(CreateField): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// CreateSystem                                          //
///////////////////////////////////////////////////////////
CreateSystem::CreateSystem(std::unique_ptr<ValueRef::ValueRefBase< ::StarType>>&& type,
                           std::unique_ptr<ValueRef::ValueRefBase<double>>&& x,
                           std::unique_ptr<ValueRef::ValueRefBase<double>>&& y,
                           std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& name,
                           std::vector<std::unique_ptr<EffectBase>>&& effects_to_apply_after) :
    m_type(std::move(type)),
    m_x(std::move(x)),
    m_y(std::move(y)),
    m_name(std::move(name)),
    m_effects_to_apply_after(std::move(effects_to_apply_after))
{
    DebugLogger() << "Effect System created 1";
}

CreateSystem::CreateSystem(std::unique_ptr<ValueRef::ValueRefBase<double>>&& x,
                           std::unique_ptr<ValueRef::ValueRefBase<double>>&& y,
                           std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& name,
                           std::vector<std::unique_ptr<EffectBase>>&& effects_to_apply_after) :
    m_type(nullptr),
    m_x(std::move(x)),
    m_y(std::move(y)),
    m_name(std::move(name)),
    m_effects_to_apply_after(std::move(effects_to_apply_after))
{
    DebugLogger() << "Effect System created 2";
}

void CreateSystem::Execute(const ScriptingContext& context) const {
    // pick a star type
    StarType star_type = STAR_NONE;
    if (m_type) {
        star_type = m_type->Eval(context);
    } else {
        int max_type_idx = int(NUM_STAR_TYPES) - 1;
        int type_idx = RandSmallInt(0, max_type_idx);
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
        name_str = GenerateSystemName();
    }

    auto system = GetUniverse().InsertNew<System>(star_type, name_str, x, y);
    if (!system) {
        ErrorLogger() << "CreateSystem::Execute couldn't create system!";
        return;
    }

    // apply after-creation effects
    ScriptingContext local_context = context;
    local_context.effect_target = system;
    for (auto& effect : m_effects_to_apply_after) {
        if (!effect)
            continue;
        effect->Execute(local_context);
    }
}

std::string CreateSystem::Dump(unsigned short ntabs) const {
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

unsigned int CreateSystem::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "CreateSystem");
    CheckSums::CheckSumCombine(retval, m_type);
    CheckSums::CheckSumCombine(retval, m_x);
    CheckSums::CheckSumCombine(retval, m_y);
    CheckSums::CheckSumCombine(retval, m_name);
    CheckSums::CheckSumCombine(retval, m_effects_to_apply_after);

    TraceLogger() << "GetCheckSum(CreateSystem): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// Destroy                                               //
///////////////////////////////////////////////////////////
Destroy::Destroy()
{}

void Destroy::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger() << "Destroy::Execute passed no target object";
        return;
    }

    int source_id = INVALID_OBJECT_ID;
    if (context.source)
        source_id = context.source->ID();

    GetUniverse().EffectDestroy(context.effect_target->ID(), source_id);
}

std::string Destroy::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "Destroy\n"; }

unsigned int Destroy::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Destroy");

    TraceLogger() << "GetCheckSum(Destroy): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// AddSpecial                                            //
///////////////////////////////////////////////////////////
AddSpecial::AddSpecial(const std::string& name, float capacity) :
    m_name(boost::make_unique<ValueRef::Constant<std::string>>(name)),
    m_capacity(boost::make_unique<ValueRef::Constant<double>>(capacity))
{}

AddSpecial::AddSpecial(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& name,
                       std::unique_ptr<ValueRef::ValueRefBase<double>>&& capacity) :
    m_name(std::move(name)),
    m_capacity(std::move(capacity))
{}

void AddSpecial::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger() << "AddSpecial::Execute passed no target object";
        return;
    }

    std::string name = (m_name ? m_name->Eval(context) : "");

    float initial_capacity = context.effect_target->SpecialCapacity(name);  // returns 0.0f if no such special yet present
    float capacity = (m_capacity ? m_capacity->Eval(ScriptingContext(context, initial_capacity)) : initial_capacity);

    context.effect_target->SetSpecialCapacity(name, capacity);
}

std::string AddSpecial::Dump(unsigned short ntabs) const {
    return DumpIndent(ntabs) + "AddSpecial name = " +  (m_name ? m_name->Dump(ntabs) : "") +
        " capacity = " + (m_capacity ? m_capacity->Dump(ntabs) : "0.0") +  "\n";
}

void AddSpecial::SetTopLevelContent(const std::string& content_name) {
    if (m_name)
        m_name->SetTopLevelContent(content_name);
    if (m_capacity)
        m_capacity->SetTopLevelContent(content_name);
}

unsigned int AddSpecial::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "AddSpecial");
    CheckSums::CheckSumCombine(retval, m_name);
    CheckSums::CheckSumCombine(retval, m_capacity);

    TraceLogger() << "GetCheckSum(AddSpecial): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// RemoveSpecial                                         //
///////////////////////////////////////////////////////////
RemoveSpecial::RemoveSpecial(const std::string& name) :
    m_name(boost::make_unique<ValueRef::Constant<std::string>>(name))
{}

RemoveSpecial::RemoveSpecial(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& name) :
    m_name(std::move(name))
{}

void RemoveSpecial::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger() << "RemoveSpecial::Execute passed no target object";
        return;
    }

    std::string name = (m_name ? m_name->Eval(context) : "");
    context.effect_target->RemoveSpecial(name);
}

std::string RemoveSpecial::Dump(unsigned short ntabs) const {
    return DumpIndent(ntabs) + "RemoveSpecial name = " +  (m_name ? m_name->Dump(ntabs) : "") + "\n";
}

void RemoveSpecial::SetTopLevelContent(const std::string& content_name) {
    if (m_name)
        m_name->SetTopLevelContent(content_name);
}

unsigned int RemoveSpecial::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "RemoveSpecial");
    CheckSums::CheckSumCombine(retval, m_name);

    TraceLogger() << "GetCheckSum(RemoveSpecial): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// AddStarlanes                                          //
///////////////////////////////////////////////////////////
AddStarlanes::AddStarlanes(std::unique_ptr<Condition::ConditionBase>&& other_lane_endpoint_condition) :
    m_other_lane_endpoint_condition(std::move(other_lane_endpoint_condition))
{}

void AddStarlanes::Execute(const ScriptingContext& context) const {
    // get target system
    if (!context.effect_target) {
        ErrorLogger() << "AddStarlanes::Execute passed no target object";
        return;
    }
    auto target_system = std::dynamic_pointer_cast<System>(context.effect_target);
    if (!target_system)
        target_system = GetSystem(context.effect_target->SystemID());
    if (!target_system)
        return; // nothing to do!

    // get other endpoint systems...
    Condition::ObjectSet endpoint_objects;
    // apply endpoints condition to determine objects whose systems should be
    // connected to the source system
    m_other_lane_endpoint_condition->Eval(context, endpoint_objects);

    // early exit if there are no valid locations
    if (endpoint_objects.empty())
        return; // nothing to do!

    // get systems containing at least one endpoint object
    std::set<std::shared_ptr<System>> endpoint_systems;
    for (auto& endpoint_object : endpoint_objects) {
        auto endpoint_system = std::dynamic_pointer_cast<const System>(endpoint_object);
        if (!endpoint_system)
            endpoint_system = GetSystem(endpoint_object->SystemID());
        if (!endpoint_system)
            continue;
        endpoint_systems.insert(std::const_pointer_cast<System>(endpoint_system));
    }

    // add starlanes from target to endpoint systems
    for (auto& endpoint_system : endpoint_systems) {
        target_system->AddStarlane(endpoint_system->ID());
        endpoint_system->AddStarlane(target_system->ID());
    }
}

std::string AddStarlanes::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "AddStarlanes endpoints = " + m_other_lane_endpoint_condition->Dump(ntabs) + "\n"; }

void AddStarlanes::SetTopLevelContent(const std::string& content_name) {
    if (m_other_lane_endpoint_condition)
        m_other_lane_endpoint_condition->SetTopLevelContent(content_name);
}

unsigned int AddStarlanes::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "AddStarlanes");
    CheckSums::CheckSumCombine(retval, m_other_lane_endpoint_condition);

    TraceLogger() << "GetCheckSum(AddStarlanes): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// RemoveStarlanes                                       //
///////////////////////////////////////////////////////////
RemoveStarlanes::RemoveStarlanes(std::unique_ptr<Condition::ConditionBase>&& other_lane_endpoint_condition) :
    m_other_lane_endpoint_condition(std::move(other_lane_endpoint_condition))
{}

void RemoveStarlanes::Execute(const ScriptingContext& context) const {
    // get target system
    if (!context.effect_target) {
        ErrorLogger() << "AddStarlanes::Execute passed no target object";
        return;
    }
    auto target_system = std::dynamic_pointer_cast<System>(context.effect_target);
    if (!target_system)
        target_system = GetSystem(context.effect_target->SystemID());
    if (!target_system)
        return; // nothing to do!

    // get other endpoint systems...

    Condition::ObjectSet endpoint_objects;
    // apply endpoints condition to determine objects whose systems should be
    // connected to the source system
    m_other_lane_endpoint_condition->Eval(context, endpoint_objects);

    // early exit if there are no valid locations - can't move anything if there's nowhere to move to
    if (endpoint_objects.empty())
        return; // nothing to do!

    // get systems containing at least one endpoint object
    std::set<std::shared_ptr<System>> endpoint_systems;
    for (auto& endpoint_object : endpoint_objects) {
        auto endpoint_system = std::dynamic_pointer_cast<const System>(endpoint_object);
        if (!endpoint_system)
            endpoint_system = GetSystem(endpoint_object->SystemID());
        if (!endpoint_system)
            continue;
        endpoint_systems.insert(std::const_pointer_cast<System>(endpoint_system));
    }

    // remove starlanes from target to endpoint systems
    int target_system_id = target_system->ID();
    for (auto& endpoint_system : endpoint_systems) {
        target_system->RemoveStarlane(endpoint_system->ID());
        endpoint_system->RemoveStarlane(target_system_id);
    }
}

std::string RemoveStarlanes::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "RemoveStarlanes endpoints = " + m_other_lane_endpoint_condition->Dump(ntabs) + "\n"; }

void RemoveStarlanes::SetTopLevelContent(const std::string& content_name) {
    if (m_other_lane_endpoint_condition)
        m_other_lane_endpoint_condition->SetTopLevelContent(content_name);
}

unsigned int RemoveStarlanes::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "RemoveStarlanes");
    CheckSums::CheckSumCombine(retval, m_other_lane_endpoint_condition);

    TraceLogger() << "GetCheckSum(RemoveStarlanes): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// SetStarType                                           //
///////////////////////////////////////////////////////////
SetStarType::SetStarType(std::unique_ptr<ValueRef::ValueRefBase<StarType>>&& type) :
    m_type(std::move(type))
{}

void SetStarType::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger() << "SetStarType::Execute given no target object";
        return;
    }
    if (auto s = std::dynamic_pointer_cast<System>(context.effect_target))
        s->SetStarType(m_type->Eval(ScriptingContext(context, s->GetStarType())));
    else
        ErrorLogger() << "SetStarType::Execute given a non-system target";
}

std::string SetStarType::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "SetStarType type = " + m_type->Dump(ntabs) + "\n"; }

void SetStarType::SetTopLevelContent(const std::string& content_name) {
    if (m_type)
        m_type->SetTopLevelContent(content_name);
}

unsigned int SetStarType::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "SetStarType");
    CheckSums::CheckSumCombine(retval, m_type);

    TraceLogger() << "GetCheckSum(SetStarType): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// MoveTo                                                //
///////////////////////////////////////////////////////////
MoveTo::MoveTo(std::unique_ptr<Condition::ConditionBase>&& location_condition) :
    m_location_condition(std::move(location_condition))
{}

void MoveTo::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger() << "MoveTo::Execute given no target object";
        return;
    }

    Universe& universe = GetUniverse();

    Condition::ObjectSet valid_locations;
    // apply location condition to determine valid location to move target to
    m_location_condition->Eval(context, valid_locations);

    // early exit if there are no valid locations - can't move anything if there's nowhere to move to
    if (valid_locations.empty())
        return;

    // "randomly" pick a destination
    auto destination = std::const_pointer_cast<UniverseObject>(*valid_locations.begin());

    // get previous system from which to remove object if necessary
    auto old_sys = GetSystem(context.effect_target->SystemID());

    // do the moving...
    if (auto fleet = std::dynamic_pointer_cast<Fleet>(context.effect_target)) {
        // fleets can be inserted into the system that contains the destination
        // object (or the destination object itself if it is a system)
        if (auto dest_system = GetSystem(destination->SystemID())) {
            if (fleet->SystemID() != dest_system->ID()) {
                // remove fleet from old system, put into new system
                if (old_sys)
                    old_sys->Remove(fleet->ID());
                dest_system->Insert(fleet);

                // also move ships of fleet
                for (auto& ship : Objects().FindObjects<Ship>(fleet->ShipIDs())) {
                    if (old_sys)
                        old_sys->Remove(ship->ID());
                    dest_system->Insert(ship);
                }

                ExploreSystem(dest_system->ID(), fleet);
                UpdateFleetRoute(fleet, INVALID_OBJECT_ID, INVALID_OBJECT_ID);  // inserted into dest_system, so next and previous systems are invalid objects
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
            for (auto& ship : Objects().FindObjects<Ship>(fleet->ShipIDs())) {
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
            auto dest_fleet = std::dynamic_pointer_cast<const Fleet>(destination);
            if (!dest_fleet)
                if (auto dest_ship = std::dynamic_pointer_cast<const Ship>(destination))
                    dest_fleet = GetFleet(dest_ship->FleetID());
            if (dest_fleet) {
                UpdateFleetRoute(fleet, dest_fleet->NextSystemID(), dest_fleet->PreviousSystemID());

            } else {
                // TODO: need to do something else to get updated previous/next
                // systems if the destination is a field.
                ErrorLogger() << "MoveTo::Execute couldn't find a way to set the previous and next systems for the target fleet!";
            }
        }

    } else if (auto ship = std::dynamic_pointer_cast<Ship>(context.effect_target)) {
        // TODO: make sure colonization doesn't interfere with this effect, and vice versa

        // is destination a ship/fleet ?
        auto dest_fleet = std::dynamic_pointer_cast<Fleet>(destination);
        if (!dest_fleet) {
            auto dest_ship = std::dynamic_pointer_cast<Ship>(destination);
            if (dest_ship)
                dest_fleet = GetFleet(dest_ship->FleetID());
        }
        if (dest_fleet && dest_fleet->ID() == ship->FleetID())
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

            if (auto new_sys = GetSystem(dest_sys_id)) {
                // ship is moving to a new system. insert it.
                new_sys->Insert(ship);
            } else {
                // ship is moving to a non-system location. move it there.
                ship->MoveTo(std::dynamic_pointer_cast<UniverseObject>(dest_fleet));
            }

            // may create a fleet for ship below...
        }

        auto old_fleet = GetFleet(ship->FleetID());

        if (dest_fleet && same_owners) {
            // ship is moving to a different fleet owned by the same empire, so
            // can be inserted into it.
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
            if (auto dest_system = GetSystem(dest_sys_id)) {
                CreateNewFleet(dest_system, ship);                          // creates new fleet, inserts fleet into system and ship into fleet
                ExploreSystem(dest_system->ID(), ship);

            } else {
                CreateNewFleet(destination->X(), destination->Y(), ship);   // creates new fleet and inserts ship into fleet
            }
        }

        if (old_fleet && old_fleet->Empty()) {
            old_sys->Remove(old_fleet->ID());
            universe.EffectDestroy(old_fleet->ID(), INVALID_OBJECT_ID); // no particular object destroyed this fleet
        }

    } else if (auto planet = std::dynamic_pointer_cast<Planet>(context.effect_target)) {
        // planets need to be located in systems, so get system that contains destination object

        auto dest_system = GetSystem(destination->SystemID());
        if (!dest_system)
            return; // can't move a planet to a non-system

        if (planet->SystemID() == dest_system->ID())
            return; // planet already at destination

        if (dest_system->FreeOrbits().empty())
            return; // no room for planet at destination

        if (old_sys)
            old_sys->Remove(planet->ID());
        dest_system->Insert(planet);  // let system pick an orbit

        // also insert buildings of planet into system.
        for (auto& building : Objects().FindObjects<Building>(planet->BuildingIDs())) {
            if (old_sys)
                old_sys->Remove(building->ID());
            dest_system->Insert(building);
        }

        // buildings planet should be unchanged by move, as should planet's
        // records of its buildings

        ExploreSystem(dest_system->ID(), planet);


    } else if (auto building = std::dynamic_pointer_cast<Building>(context.effect_target)) {
        // buildings need to be located on planets, so if destination is a
        // planet, insert building into it, or attempt to get the planet on
        // which the destination object is located and insert target building
        // into that
        auto dest_planet = std::dynamic_pointer_cast<Planet>(destination);
        if (!dest_planet) {
            auto dest_building = std::dynamic_pointer_cast<Building>(destination);
            if (dest_building) {
                dest_planet = GetPlanet(dest_building->PlanetID());
            }
        }
        if (!dest_planet)
            return;

        if (dest_planet->ID() == building->PlanetID())
            return; // nothing to do

        auto dest_system = GetSystem(destination->SystemID());
        if (!dest_system)
            return;

        // remove building from old planet / system, add to new planet / system
        if (old_sys)
            old_sys->Remove(building->ID());
        building->SetSystem(INVALID_OBJECT_ID);

        if (auto old_planet = GetPlanet(building->PlanetID()))
            old_planet->RemoveBuilding(building->ID());

        dest_planet->AddBuilding(building->ID());
        building->SetPlanetID(dest_planet->ID());

        dest_system->Insert(building);
        ExploreSystem(dest_system->ID(), building);


    } else if (auto system = std::dynamic_pointer_cast<System>(context.effect_target)) {
        if (destination->SystemID() != INVALID_OBJECT_ID) {
            // TODO: merge systems
            return;
        }

        // move target system to new destination, and insert destination object
        // and related objects into system
        system->MoveTo(destination);

        if (destination->ObjectType() == OBJ_FIELD)
            system->Insert(destination);

        // find fleets / ships at destination location and insert into system
        for (auto& obj : Objects().FindObjects<Fleet>()) {
            if (obj->X() == system->X() && obj->Y() == system->Y())
                system->Insert(obj);
        }

        for (auto& obj : Objects().FindObjects<Ship>()) {
            if (obj->X() == system->X() && obj->Y() == system->Y())
                system->Insert(obj);
        }


    } else if (auto field = std::dynamic_pointer_cast<Field>(context.effect_target)) {
        if (old_sys)
            old_sys->Remove(field->ID());
        field->SetSystem(INVALID_OBJECT_ID);
        field->MoveTo(destination);
        if (auto dest_system = std::dynamic_pointer_cast<System>(destination))
            dest_system->Insert(field);
    }
}

std::string MoveTo::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "MoveTo destination = " + m_location_condition->Dump(ntabs) + "\n"; }

void MoveTo::SetTopLevelContent(const std::string& content_name) {
    if (m_location_condition)
        m_location_condition->SetTopLevelContent(content_name);
}

unsigned int MoveTo::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "MoveTo");
    CheckSums::CheckSumCombine(retval, m_location_condition);

    TraceLogger() << "GetCheckSum(MoveTo): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// MoveInOrbit                                           //
///////////////////////////////////////////////////////////
MoveInOrbit::MoveInOrbit(std::unique_ptr<ValueRef::ValueRefBase<double>>&& speed,
                         std::unique_ptr<Condition::ConditionBase>&& focal_point_condition) :
    m_speed(std::move(speed)),
    m_focal_point_condition(std::move(focal_point_condition)),
    m_focus_x(nullptr),
    m_focus_y(nullptr)
{}

MoveInOrbit::MoveInOrbit(std::unique_ptr<ValueRef::ValueRefBase<double>>&& speed,
                         std::unique_ptr<ValueRef::ValueRefBase<double>>&& focus_x/* = 0*/,
                         std::unique_ptr<ValueRef::ValueRefBase<double>>&& focus_y/* = 0*/) :
    m_speed(std::move(speed)),
    m_focal_point_condition(nullptr),
    m_focus_x(std::move(focus_x)),
    m_focus_y(std::move(focus_y))
{}

void MoveInOrbit::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger() << "MoveInOrbit::Execute given no target object";
        return;
    }
    auto target = context.effect_target;

    double focus_x = 0.0, focus_y = 0.0, speed = 1.0;
    if (m_focus_x)
        focus_x = m_focus_x->Eval(ScriptingContext(context, target->X()));
    if (m_focus_y)
        focus_y = m_focus_y->Eval(ScriptingContext(context, target->Y()));
    if (m_speed)
        speed = m_speed->Eval(context);
    if (speed == 0.0)
        return;
    if (m_focal_point_condition) {
        Condition::ObjectSet matches;
        m_focal_point_condition->Eval(context, matches);
        if (matches.empty())
            return;
        std::shared_ptr<const UniverseObject> focus_object = *matches.begin();
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

    auto old_sys = GetSystem(target->SystemID());

    if (auto system = std::dynamic_pointer_cast<System>(target)) {
        system->MoveTo(new_x, new_y);
        return;

    } else if (auto fleet = std::dynamic_pointer_cast<Fleet>(target)) {
        if (old_sys)
            old_sys->Remove(fleet->ID());
        fleet->SetSystem(INVALID_OBJECT_ID);
        fleet->MoveTo(new_x, new_y);
        UpdateFleetRoute(fleet, INVALID_OBJECT_ID, INVALID_OBJECT_ID);

        for (auto& ship : Objects().FindObjects<Ship>(fleet->ShipIDs())) {
            if (old_sys)
                old_sys->Remove(ship->ID());
            ship->SetSystem(INVALID_OBJECT_ID);
            ship->MoveTo(new_x, new_y);
        }
        return;

    } else if (auto ship = std::dynamic_pointer_cast<Ship>(target)) {
        if (old_sys)
            old_sys->Remove(ship->ID());
        ship->SetSystem(INVALID_OBJECT_ID);

        auto old_fleet = GetFleet(ship->FleetID());
        if (old_fleet) {
            old_fleet->RemoveShips({ship->ID()});
            if (old_fleet->Empty()) {
                old_sys->Remove(old_fleet->ID());
                GetUniverse().EffectDestroy(old_fleet->ID(), INVALID_OBJECT_ID);    // no object in particular destroyed this fleet
            }
        }

        ship->SetFleetID(INVALID_OBJECT_ID);
        ship->MoveTo(new_x, new_y);

        CreateNewFleet(new_x, new_y, ship); // creates new fleet and inserts ship into fleet
        return;

    } else if (auto field = std::dynamic_pointer_cast<Field>(target)) {
        if (old_sys)
            old_sys->Remove(field->ID());
        field->SetSystem(INVALID_OBJECT_ID);
        field->MoveTo(new_x, new_y);
    }
    // don't move planets or buildings, as these can't exist outside of systems
}

std::string MoveInOrbit::Dump(unsigned short ntabs) const {
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

unsigned int MoveInOrbit::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "MoveInOrbit");
    CheckSums::CheckSumCombine(retval, m_speed);
    CheckSums::CheckSumCombine(retval, m_focal_point_condition);
    CheckSums::CheckSumCombine(retval, m_focus_x);
    CheckSums::CheckSumCombine(retval, m_focus_y);

    TraceLogger() << "GetCheckSum(MoveInOrbit): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// MoveTowards                                           //
///////////////////////////////////////////////////////////
MoveTowards::MoveTowards(std::unique_ptr<ValueRef::ValueRefBase<double>>&& speed,
                         std::unique_ptr<Condition::ConditionBase>&& dest_condition) :
    m_speed(std::move(speed)),
    m_dest_condition(std::move(dest_condition)),
    m_dest_x(nullptr),
    m_dest_y(nullptr)
{}

MoveTowards::MoveTowards(std::unique_ptr<ValueRef::ValueRefBase<double>>&& speed,
                         std::unique_ptr<ValueRef::ValueRefBase<double>>&& dest_x/* = 0*/,
                         std::unique_ptr<ValueRef::ValueRefBase<double>>&& dest_y/* = 0*/) :
    m_speed(std::move(speed)),
    m_dest_condition(nullptr),
    m_dest_x(std::move(dest_x)),
    m_dest_y(std::move(dest_y))
{}

void MoveTowards::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger() << "MoveTowards::Execute given no target object";
        return;
    }
    auto target = context.effect_target;

    double dest_x = 0.0, dest_y = 0.0, speed = 1.0;
    if (m_dest_x)
        dest_x = m_dest_x->Eval(ScriptingContext(context, target->X()));
    if (m_dest_y)
        dest_y = m_dest_y->Eval(ScriptingContext(context, target->Y()));
    if (m_speed)
        speed = m_speed->Eval(context);
    if (speed == 0.0)
        return;
    if (m_dest_condition) {
        Condition::ObjectSet matches;
        m_dest_condition->Eval(context, matches);
        if (matches.empty())
            return;
        std::shared_ptr<const UniverseObject> focus_object = *matches.begin();
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

    if (auto system = std::dynamic_pointer_cast<System>(target)) {
        system->MoveTo(new_x, new_y);
        for (auto& obj : Objects().FindObjects<UniverseObject>(system->ObjectIDs())) {
            obj->MoveTo(new_x, new_y);
        }
        // don't need to remove objects from system or insert into it, as all
        // contained objects in system are moved with it, maintaining their
        // containment situation

    } else if (auto fleet = std::dynamic_pointer_cast<Fleet>(target)) {
        auto old_sys = GetSystem(fleet->SystemID());
        if (old_sys)
            old_sys->Remove(fleet->ID());
        fleet->SetSystem(INVALID_OBJECT_ID);
        fleet->MoveTo(new_x, new_y);
        for (auto& ship : Objects().FindObjects<Ship>(fleet->ShipIDs())) {
            if (old_sys)
                old_sys->Remove(ship->ID());
            ship->SetSystem(INVALID_OBJECT_ID);
            ship->MoveTo(new_x, new_y);
        }

        // todo: is fleet now close enough to fall into a system?
        UpdateFleetRoute(fleet, INVALID_OBJECT_ID, INVALID_OBJECT_ID);

    } else if (auto ship = std::dynamic_pointer_cast<Ship>(target)) {
        auto old_sys = GetSystem(ship->SystemID());
        if (old_sys)
            old_sys->Remove(ship->ID());
        ship->SetSystem(INVALID_OBJECT_ID);

        auto old_fleet = GetFleet(ship->FleetID());
        if (old_fleet)
            old_fleet->RemoveShips({ship->ID()});
        ship->SetFleetID(INVALID_OBJECT_ID);

        CreateNewFleet(new_x, new_y, ship); // creates new fleet and inserts ship into fleet
        if (old_fleet && old_fleet->Empty()) {
            if (old_sys)
                old_sys->Remove(old_fleet->ID());
            GetUniverse().EffectDestroy(old_fleet->ID(), INVALID_OBJECT_ID);    // no object in particular destroyed this fleet
        }

    } else if (auto field = std::dynamic_pointer_cast<Field>(target)) {
        auto old_sys = GetSystem(field->SystemID());
        if (old_sys)
            old_sys->Remove(field->ID());
        field->SetSystem(INVALID_OBJECT_ID);
        field->MoveTo(new_x, new_y);

    }
    // don't move planets or buildings, as these can't exist outside of systems
}

std::string MoveTowards::Dump(unsigned short ntabs) const {
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

unsigned int MoveTowards::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "MoveTowards");
    CheckSums::CheckSumCombine(retval, m_speed);
    CheckSums::CheckSumCombine(retval, m_dest_condition);
    CheckSums::CheckSumCombine(retval, m_dest_x);
    CheckSums::CheckSumCombine(retval, m_dest_y);

    TraceLogger() << "GetCheckSum(MoveTowards): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// SetDestination                                        //
///////////////////////////////////////////////////////////
SetDestination::SetDestination(std::unique_ptr<Condition::ConditionBase>&& location_condition) :
    m_location_condition(std::move(location_condition))
{}

void SetDestination::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger() << "SetDestination::Execute given no target object";
        return;
    }

    auto target_fleet = std::dynamic_pointer_cast<Fleet>(context.effect_target);
    if (!target_fleet) {
        ErrorLogger() << "SetDestination::Execute acting on non-fleet target:";
        context.effect_target->Dump();
        return;
    }

    Universe& universe = GetUniverse();

    Condition::ObjectSet valid_locations;
    // apply location condition to determine valid location to move target to
    m_location_condition->Eval(context, valid_locations);

    // early exit if there are no valid locations - can't move anything if there's nowhere to move to
    if (valid_locations.empty())
        return;

    // "randomly" pick a destination
    int destination_idx = RandSmallInt(0, valid_locations.size() - 1);
    auto destination = std::const_pointer_cast<UniverseObject>(
        *std::next(valid_locations.begin(), destination_idx));
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
    std::pair<std::list<int>, double> short_path = universe.GetPathfinder()->ShortestPath(start_system_id, destination_system_id, target_fleet->Owner());
    const std::list<int>& route_list = short_path.first;

    // reject empty move paths (no path exists).
    if (route_list.empty())
        return;

    // check destination validity: disallow movement that's out of range
    std::pair<int, int> eta = target_fleet->ETA(target_fleet->MovePath(route_list));
    if (eta.first == Fleet::ETA_NEVER || eta.first == Fleet::ETA_OUT_OF_RANGE)
        return;

    target_fleet->SetRoute(route_list);
}

std::string SetDestination::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "SetDestination destination = " + m_location_condition->Dump(ntabs) + "\n"; }

void SetDestination::SetTopLevelContent(const std::string& content_name) {
    if (m_location_condition)
        m_location_condition->SetTopLevelContent(content_name);
}

unsigned int SetDestination::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "SetDestination");
    CheckSums::CheckSumCombine(retval, m_location_condition);

    TraceLogger() << "GetCheckSum(SetDestination): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// SetAggression                                         //
///////////////////////////////////////////////////////////
SetAggression::SetAggression(bool aggressive) :
    m_aggressive(aggressive)
{}

void SetAggression::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger() << "SetAggression::Execute given no target object";
        return;
    }

    auto target_fleet = std::dynamic_pointer_cast<Fleet>(context.effect_target);
    if (!target_fleet) {
        ErrorLogger() << "SetAggression::Execute acting on non-fleet target:";
        context.effect_target->Dump();
        return;
    }

    target_fleet->SetAggressive(m_aggressive);
}

std::string SetAggression::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + (m_aggressive ? "SetAggressive" : "SetPassive") + "\n"; }

unsigned int SetAggression::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "SetAggression");
    CheckSums::CheckSumCombine(retval, m_aggressive);

    TraceLogger() << "GetCheckSum(SetAggression): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// Victory                                               //
///////////////////////////////////////////////////////////
Victory::Victory(const std::string& reason_string) :
    m_reason_string(reason_string)
{}

void Victory::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger() << "Victory::Execute given no target object";
        return;
    }
    if (Empire* empire = GetEmpire(context.effect_target->Owner()))
        empire->Win(m_reason_string);
    else
        ErrorLogger() << "Trying to grant victory to a missing empire!";
}

std::string Victory::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "Victory reason = \"" + m_reason_string + "\"\n"; }

unsigned int Victory::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Victory");
    CheckSums::CheckSumCombine(retval, m_reason_string);

    TraceLogger() << "GetCheckSum(Victory): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// SetEmpireTechProgress                                 //
///////////////////////////////////////////////////////////
SetEmpireTechProgress::SetEmpireTechProgress(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& tech_name,
                                             std::unique_ptr<ValueRef::ValueRefBase<double>>&& research_progress,
                                             std::unique_ptr<ValueRef::ValueRefBase<int>>&& empire_id /*= nullptr*/) :
    m_tech_name(std::move(tech_name)),
    m_research_progress(std::move(research_progress)),
    m_empire_id(
        empire_id
        ? std::move(empire_id)
        : boost::make_unique<ValueRef::Variable<int>>(ValueRef::EFFECT_TARGET_REFERENCE, std::vector<std::string>(1, "Owner")))
{}

void SetEmpireTechProgress::Execute(const ScriptingContext& context) const {
    if (!m_empire_id) return;
    Empire* empire = GetEmpire(m_empire_id->Eval(context));
    if (!empire) return;

    if (!m_tech_name) {
        ErrorLogger() << "SetEmpireTechProgress::Execute has not tech name to evaluate";
        return;
    }
    std::string tech_name = m_tech_name->Eval(context);
    if (tech_name.empty())
        return;

    const Tech* tech = GetTech(tech_name);
    if (!tech) {
        ErrorLogger() << "SetEmpireTechProgress::Execute couldn't get tech with name " << tech_name;
        return;
    }

    float initial_progress = empire->ResearchProgress(tech_name);
    double value = m_research_progress->Eval(ScriptingContext(context, initial_progress));
    empire->SetTechResearchProgress(tech_name, value);
}

std::string SetEmpireTechProgress::Dump(unsigned short ntabs) const {
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

unsigned int SetEmpireTechProgress::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "SetEmpireTechProgress");
    CheckSums::CheckSumCombine(retval, m_tech_name);
    CheckSums::CheckSumCombine(retval, m_research_progress);
    CheckSums::CheckSumCombine(retval, m_empire_id);

    TraceLogger() << "GetCheckSum(SetEmpireTechProgress): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// GiveEmpireTech                                        //
///////////////////////////////////////////////////////////
GiveEmpireTech::GiveEmpireTech(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& tech_name,
                               std::unique_ptr<ValueRef::ValueRefBase<int>>&& empire_id) :
    m_tech_name(std::move(tech_name)),
    m_empire_id(std::move(empire_id))
{
    if (!m_empire_id)
        m_empire_id.reset(new ValueRef::Variable<int>(ValueRef::EFFECT_TARGET_REFERENCE, std::vector<std::string>(1, "Owner")));
}

void GiveEmpireTech::Execute(const ScriptingContext& context) const {
    if (!m_empire_id) return;
    Empire* empire = GetEmpire(m_empire_id->Eval(context));
    if (!empire) return;

    if (!m_tech_name)
        return;

    std::string tech_name = m_tech_name->Eval(context);

    const Tech* tech = GetTech(tech_name);
    if (!tech) {
        ErrorLogger() << "GiveEmpireTech::Execute couldn't get tech with name: " << tech_name;
        return;
    }

    empire->AddTech(tech_name);
}

std::string GiveEmpireTech::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "GiveEmpireTech";

    if (m_tech_name)
        retval += " name = " + m_tech_name->Dump(ntabs);

    if (m_empire_id)
        retval += " empire = " + m_empire_id->Dump(ntabs);

    retval += "\n";
    return retval;
}

void GiveEmpireTech::SetTopLevelContent(const std::string& content_name) {
    if (m_empire_id)
        m_empire_id->SetTopLevelContent(content_name);
    if (m_tech_name)
        m_tech_name->SetTopLevelContent(content_name);
}

unsigned int GiveEmpireTech::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "GiveEmpireTech");
    CheckSums::CheckSumCombine(retval, m_tech_name);
    CheckSums::CheckSumCombine(retval, m_empire_id);

    TraceLogger() << "GetCheckSum(GiveEmpireTech): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// GenerateSitRepMessage                                 //
///////////////////////////////////////////////////////////
GenerateSitRepMessage::GenerateSitRepMessage(const std::string& message_string,
                                             const std::string& icon,
                                             MessageParams&& message_parameters,
                                             std::unique_ptr<ValueRef::ValueRefBase<int>>&& recipient_empire_id,
                                             EmpireAffiliationType affiliation,
                                             const std::string label,
                                             bool stringtable_lookup) :
    m_message_string(message_string),
    m_icon(icon),
    m_message_parameters(std::move(message_parameters)),
    m_recipient_empire_id(std::move(recipient_empire_id)),
    m_condition(nullptr),
    m_affiliation(affiliation),
    m_label(label),
    m_stringtable_lookup(stringtable_lookup)
{}

GenerateSitRepMessage::GenerateSitRepMessage(const std::string& message_string,
                                             const std::string& icon,
                                             MessageParams&& message_parameters,
                                             EmpireAffiliationType affiliation,
                                             std::unique_ptr<Condition::ConditionBase>&& condition,
                                             const std::string label,
                                             bool stringtable_lookup) :
    m_message_string(message_string),
    m_icon(icon),
    m_message_parameters(std::move(message_parameters)),
    m_recipient_empire_id(nullptr),
    m_condition(std::move(condition)),
    m_affiliation(affiliation),
    m_label(label),
    m_stringtable_lookup(stringtable_lookup)
{}

GenerateSitRepMessage::GenerateSitRepMessage(const std::string& message_string, const std::string& icon,
                                             MessageParams&& message_parameters,
                                             EmpireAffiliationType affiliation,
                                             const std::string& label,
                                             bool stringtable_lookup):
    m_message_string(message_string),
    m_icon(icon),
    m_message_parameters(std::move(message_parameters)),
    m_recipient_empire_id(nullptr),
    m_condition(),
    m_affiliation(affiliation),
    m_label(label),
    m_stringtable_lookup(stringtable_lookup)
{}

void GenerateSitRepMessage::Execute(const ScriptingContext& context) const {
    int recipient_id = ALL_EMPIRES;
    if (m_recipient_empire_id)
        recipient_id = m_recipient_empire_id->Eval(context);

    // track any ship designs used in message, which any recipients must be
    // made aware of so sitrep won't have errors
    std::set<int> ship_design_ids_to_inform_receipits_of;

    // TODO: should any referenced object IDs being made known at basic visibility?


    // evaluate all parameter valuerefs so they can be substituted into sitrep template
    std::vector<std::pair<std::string, std::string>> parameter_tag_values;
    for (const auto& entry : m_message_parameters) {
        parameter_tag_values.push_back({entry.first, entry.second->Eval(context)});

        // special case for ship designs: make sure sitrep recipient knows about the design
        // so the sitrep won't have errors about unknown designs being referenced
        if (entry.first == VarText::PREDEFINED_DESIGN_TAG) {
            if (const ShipDesign* design = GetPredefinedShipDesign(entry.second->Eval(context))) {
                ship_design_ids_to_inform_receipits_of.insert(design->ID());
            }
        }
    }

    // whom to send to?
    std::set<int> recipient_empire_ids;
    switch (m_affiliation) {
    case AFFIL_SELF: {
        // add just specified empire
        if (recipient_id != ALL_EMPIRES)
            recipient_empire_ids.insert(recipient_id);
        break;
    }

    case AFFIL_ALLY: {
        // add allies of specified empire
        for (auto& empire_id : Empires()) {
            if (empire_id.first == recipient_id || recipient_id == ALL_EMPIRES)
                continue;

            DiplomaticStatus status = Empires().GetDiplomaticStatus(recipient_id, empire_id.first);
            if (status == DIPLO_PEACE)
                recipient_empire_ids.insert(empire_id.first);
        }
        break;
    }

    case AFFIL_ENEMY: {
        // add enemies of specified empire
        for (auto& empire_id : Empires()) {
            if (empire_id.first == recipient_id || recipient_id == ALL_EMPIRES)
                continue;

            DiplomaticStatus status = Empires().GetDiplomaticStatus(recipient_id, empire_id.first);
            if (status == DIPLO_WAR)
                recipient_empire_ids.insert(empire_id.first);
        }
        break;
    }

    case AFFIL_CAN_SEE: {
        // evaluate condition
        Condition::ObjectSet condition_matches;
        if (m_condition)
            m_condition->Eval(context, condition_matches);

        // add empires that can see any condition-matching object
        for (auto& empire_entry : Empires()) {
            int empire_id = empire_entry.first;
            for (auto& object : condition_matches) {
                if (object->GetVisibility(empire_id) >= VIS_BASIC_VISIBILITY) {
                    recipient_empire_ids.insert(empire_id);
                    break;
                }
            }
        }
        break;
    }

    case AFFIL_NONE:
        // add no empires
        break;

    case AFFIL_HUMAN:
        // todo: implement this separately, though not high priority since it
        // probably doesn't matter if AIs get an extra sitrep message meant for
        // human eyes
    case AFFIL_ANY:
    default: {
        // add all empires
        for (auto& empire_entry : Empires())
            recipient_empire_ids.insert(empire_entry.first);
        break;
    }
    }

    int sitrep_turn = CurrentTurn() + 1;

    // send to recipient empires
    for (int empire_id : recipient_empire_ids) {
        Empire* empire = GetEmpire(empire_id);
        if (!empire)
            continue;
        empire->AddSitRepEntry(CreateSitRep(m_message_string, sitrep_turn, m_icon,
                                            parameter_tag_values, m_label, m_stringtable_lookup));

        // also inform of any ship designs recipients should know about
        for (int design_id : ship_design_ids_to_inform_receipits_of) {
            GetUniverse().SetEmpireKnowledgeOfShipDesign(design_id, empire_id);
        }
    }
}

std::string GenerateSitRepMessage::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs);
    retval += "GenerateSitRepMessage\n";
    retval += DumpIndent(ntabs+1) + "message = \"" + m_message_string + "\"" + " icon = " + m_icon + "\n";

    if (m_message_parameters.size() == 1) {
        retval += DumpIndent(ntabs+1) + "parameters = tag = " + m_message_parameters[0].first + " data = " + m_message_parameters[0].second->Dump(ntabs+1) + "\n";
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
    case AFFIL_SELF:    retval += "TheEmpire";  break;
    case AFFIL_ENEMY:   retval += "EnemyOf";    break;
    case AFFIL_ALLY:    retval += "AllyOf";     break;
    case AFFIL_ANY:     retval += "AnyEmpire";  break;
    case AFFIL_CAN_SEE: retval += "CanSee";     break;
    case AFFIL_HUMAN:   retval += "Human";      break;
    default:            retval += "?";          break;
    }

    if (m_recipient_empire_id)
        retval += "\n" + DumpIndent(ntabs+1) + "empire = " + m_recipient_empire_id->Dump(ntabs+1) + "\n";
    if (m_condition)
        retval += "\n" + DumpIndent(ntabs+1) + "condition = " + m_condition->Dump(ntabs+1) + "\n";

    return retval;
}

void GenerateSitRepMessage::SetTopLevelContent(const std::string& content_name) {
    for (auto& entry : m_message_parameters) {
        entry.second->SetTopLevelContent(content_name);
    }
    if (m_recipient_empire_id)
        m_recipient_empire_id->SetTopLevelContent(content_name);
    if (m_condition)
        m_condition->SetTopLevelContent(content_name);
}

unsigned int GenerateSitRepMessage::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "GenerateSitRepMessage");
    CheckSums::CheckSumCombine(retval, m_message_string);
    CheckSums::CheckSumCombine(retval, m_icon);
    CheckSums::CheckSumCombine(retval, m_message_parameters);
    CheckSums::CheckSumCombine(retval, m_recipient_empire_id);
    CheckSums::CheckSumCombine(retval, m_condition);
    CheckSums::CheckSumCombine(retval, m_affiliation);
    CheckSums::CheckSumCombine(retval, m_label);
    CheckSums::CheckSumCombine(retval, m_stringtable_lookup);

    TraceLogger() << "GetCheckSum(GenerateSitRepMessage): retval: " << retval;
    return retval;
}

std::vector<std::pair<std::string, ValueRef::ValueRefBase<std::string>*>>
GenerateSitRepMessage::MessageParameters() const {
    std::vector<std::pair<std::string, ValueRef::ValueRefBase<std::string>*>> retval(m_message_parameters.size());
    std::transform(m_message_parameters.begin(), m_message_parameters.end(), retval.begin(),
                   [](const std::pair<std::string, std::unique_ptr<ValueRef::ValueRefBase<std::string>>>& xx) {
                       return std::make_pair(xx.first, xx.second.get());
                   });
    return retval;
}

///////////////////////////////////////////////////////////
// SetOverlayTexture                                     //
///////////////////////////////////////////////////////////
SetOverlayTexture::SetOverlayTexture(const std::string& texture,
                                     std::unique_ptr<ValueRef::ValueRefBase<double>>&& size) :
    m_texture(texture),
    m_size(std::move(size))
{}

SetOverlayTexture::SetOverlayTexture(const std::string& texture, ValueRef::ValueRefBase<double>* size) :
    m_texture(texture),
    m_size(size)
{}

void SetOverlayTexture::Execute(const ScriptingContext& context) const {
    if (!context.effect_target)
        return;
    double size = 1.0;
    if (m_size)
        size = m_size->Eval(context);

    if (auto system = std::dynamic_pointer_cast<System>(context.effect_target))
        system->SetOverlayTexture(m_texture, size);
}

std::string SetOverlayTexture::Dump(unsigned short ntabs) const {
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

unsigned int SetOverlayTexture::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "SetOverlayTexture");
    CheckSums::CheckSumCombine(retval, m_texture);
    CheckSums::CheckSumCombine(retval, m_size);

    TraceLogger() << "GetCheckSum(SetOverlayTexture): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// SetTexture                                            //
///////////////////////////////////////////////////////////
SetTexture::SetTexture(const std::string& texture) :
    m_texture(texture)
{}

void SetTexture::Execute(const ScriptingContext& context) const {
    if (!context.effect_target)
        return;
    if (auto planet = std::dynamic_pointer_cast<Planet>(context.effect_target))
        planet->SetSurfaceTexture(m_texture);
}

std::string SetTexture::Dump(unsigned short ntabs) const
{ return DumpIndent(ntabs) + "SetTexture texture = " + m_texture + "\n"; }

unsigned int SetTexture::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "SetTexture");
    CheckSums::CheckSumCombine(retval, m_texture);

    TraceLogger() << "GetCheckSum(SetTexture): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// SetVisibility                                         //
///////////////////////////////////////////////////////////
SetVisibility::SetVisibility(std::unique_ptr<ValueRef::ValueRefBase<Visibility>> vis,
                             EmpireAffiliationType affiliation,
                             std::unique_ptr<ValueRef::ValueRefBase<int>>&& empire_id,
                             std::unique_ptr<Condition::ConditionBase>&& of_objects) :
    m_vis(std::move(vis)),
    m_empire_id(std::move(empire_id)),
    m_affiliation(affiliation),
    m_condition(std::move(of_objects))
{}

void SetVisibility::Execute(const ScriptingContext& context) const {
    if (!context.effect_target)
        return;

    // Note: currently ignoring upgrade-only flag

    if (!m_vis)
        return; // nothing to evaluate!

    int empire_id = ALL_EMPIRES;
    if (m_empire_id)
        empire_id = m_empire_id->Eval(context);

    // whom to set visbility for?
    std::set<int> empire_ids;
    switch (m_affiliation) {
    case AFFIL_SELF: {
        // add just specified empire
        if (empire_id != ALL_EMPIRES)
            empire_ids.insert(empire_id);
        break;
    }

    case AFFIL_ALLY: {
        // add allies of specified empire
        for (const auto& empire_entry : Empires()) {
            if (empire_entry.first == empire_id || empire_id == ALL_EMPIRES)
                continue;

            DiplomaticStatus status = Empires().GetDiplomaticStatus(empire_id, empire_entry.first);
            if (status == DIPLO_PEACE)
                empire_ids.insert(empire_entry.first);
        }
        break;
    }

    case AFFIL_ENEMY: {
        // add enemies of specified empire
        for (const auto& empire_entry : Empires()) {
            if (empire_entry.first == empire_id || empire_id == ALL_EMPIRES)
                continue;

            DiplomaticStatus status = Empires().GetDiplomaticStatus(empire_id, empire_entry.first);
            if (status == DIPLO_WAR)
                empire_ids.insert(empire_entry.first);
        }
        break;
    }

    case AFFIL_CAN_SEE:
        // unsupported so far...
    case AFFIL_HUMAN:
        // unsupported so far...
    case AFFIL_NONE:
        // add no empires
        break;

    case AFFIL_ANY:
    default: {
        // add all empires
        for (const auto& empire_entry : Empires())
            empire_ids.insert(empire_entry.first);
        break;
    }
    }

    // what to set visibility of?
    std::set<int> object_ids;
    if (!m_condition) {
        object_ids.insert(context.effect_target->ID());
    } else {
        Condition::ObjectSet condition_matches;
        m_condition->Eval(context, condition_matches);
        for (auto& object : condition_matches) {
            object_ids.insert(object->ID());
        }
    }

    int source_id = INVALID_OBJECT_ID;
    if (context.source)
        source_id = context.source->ID();

    for (int emp_id : empire_ids) {
        if (!GetEmpire(emp_id))
            continue;
        for (int obj_id : object_ids) {
            // store source object id and ValueRef to evaluate to determine
            // what visibility level to set at time of application
            GetUniverse().SetEffectDerivedVisibility(emp_id, obj_id, source_id, m_vis.get());
        }
    }
}

std::string SetVisibility::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs);

    retval += DumpIndent(ntabs) + "SetVisibility affiliation = ";
    switch (m_affiliation) {
    case AFFIL_SELF:    retval += "TheEmpire";  break;
    case AFFIL_ENEMY:   retval += "EnemyOf";    break;
    case AFFIL_ALLY:    retval += "AllyOf";     break;
    case AFFIL_ANY:     retval += "AnyEmpire";  break;
    case AFFIL_CAN_SEE: retval += "CanSee";     break;
    case AFFIL_HUMAN:   retval += "Human";      break;
    default:            retval += "?";          break;
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

unsigned int SetVisibility::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "SetVisibility");
    CheckSums::CheckSumCombine(retval, m_vis.get());
    CheckSums::CheckSumCombine(retval, m_empire_id);
    CheckSums::CheckSumCombine(retval, m_affiliation);
    CheckSums::CheckSumCombine(retval, m_condition);

    TraceLogger() << "GetCheckSum(SetVisibility): retval: " << retval;
    return retval;
}


///////////////////////////////////////////////////////////
// Conditional                                           //
///////////////////////////////////////////////////////////
Conditional::Conditional(std::unique_ptr<Condition::ConditionBase>&& target_condition,
                         std::vector<std::unique_ptr<EffectBase>>&& true_effects,
                         std::vector<std::unique_ptr<EffectBase>>&& false_effects) :
    m_target_condition(std::move(target_condition)),
    m_true_effects(std::move(true_effects)),
    m_false_effects(std::move(false_effects))
{}

void Conditional::Execute(const ScriptingContext& context) const {
    if (!context.effect_target)
        return;

    if (!m_target_condition || m_target_condition->Eval(context, context.effect_target)) {
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

void Conditional::Execute(const ScriptingContext& context, const TargetSet& targets) const {
    if (targets.empty())
        return;

    // apply sub-condition to target set to pick which to act on with which of sub-effects
    const Condition::ObjectSet& potential_target_objects =
        *reinterpret_cast<const Condition::ObjectSet*>(&targets);

    Condition::ObjectSet matches = potential_target_objects;
    Condition::ObjectSet non_matches;
    if (m_target_condition)
        m_target_condition->Eval(context, matches, non_matches, Condition::MATCHES);

    if (!matches.empty() && !m_true_effects.empty()) {
        Effect::TargetSet& match_targets = *reinterpret_cast<Effect::TargetSet*>(&matches);
        for (auto& effect : m_true_effects) {
            if (effect)
                effect->Execute(context, match_targets);
        }
    }
    if (!non_matches.empty() && !m_false_effects.empty()) {
        Effect::TargetSet& non_match_targets = *reinterpret_cast<Effect::TargetSet*>(&non_matches);
        for (auto& effect : m_false_effects) {
            if (effect)
                effect->Execute(context, non_match_targets);
        }
    }
}

void Conditional::Execute(const ScriptingContext& context,
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
    const Condition::ObjectSet& potential_target_objects =
        *reinterpret_cast<const Condition::ObjectSet*>(&targets);
    Condition::ObjectSet matches = potential_target_objects;
    Condition::ObjectSet non_matches;
    if (m_target_condition)
        m_target_condition->Eval(context, matches, non_matches,
                                 Condition::MATCHES);

    // execute true and false effects to target matches and non-matches respectively
    if (!matches.empty() && !m_true_effects.empty()) {
        Effect::TargetSet& match_targets =
            *reinterpret_cast<Effect::TargetSet*>(&matches);
        for (const auto& effect : m_true_effects) {
            effect->Execute(context, match_targets, accounting_map,
                            effect_cause,
                            only_meter_effects, only_appearance_effects,
                            include_empire_meter_effects,
                            only_generate_sitrep_effects);
        }
    }
    if (!non_matches.empty() && !m_false_effects.empty()) {
        Effect::TargetSet& non_match_targets =
            *reinterpret_cast<Effect::TargetSet*>(&non_matches);
        for (const auto& effect : m_false_effects) {
            effect->Execute(context, non_match_targets, accounting_map,
                            effect_cause,
                            only_meter_effects, only_appearance_effects,
                            include_empire_meter_effects,
                            only_generate_sitrep_effects);
        }
    }
}

std::string Conditional::Dump(unsigned short ntabs) const {
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

bool Conditional::IsMeterEffect() const {
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

bool Conditional::IsAppearanceEffect() const {
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

bool Conditional::IsSitrepEffect() const {
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

unsigned int Conditional::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, "Conditional");
    CheckSums::CheckSumCombine(retval, m_target_condition);
    CheckSums::CheckSumCombine(retval, m_true_effects);
    CheckSums::CheckSumCombine(retval, m_false_effects);

    TraceLogger() << "GetCheckSum(Conditional): retval: " << retval;
    return retval;
}

} // namespace Effect
