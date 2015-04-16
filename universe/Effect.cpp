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
#include "Universe.h"
#include "UniverseObject.h"
#include "Building.h"
#include "Planet.h"
#include "System.h"
#include "Field.h"
#include "Fleet.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "Tech.h"
#include "Species.h"

#include <boost/filesystem/fstream.hpp>

#include <cctype>

using namespace Effect;
using boost::io::str;
using boost::lexical_cast;

extern int g_indent;

namespace {
    boost::tuple<bool, ValueRef::OpType, double>
    SimpleMeterModification(MeterType meter, ValueRef::ValueRefBase<double>* ref) {
        boost::tuple<bool, ValueRef::OpType, double> retval(false, ValueRef::PLUS, 0.0);
        if (const ValueRef::Operation<double>* op = dynamic_cast<const ValueRef::Operation<double>*>(ref)) {
            if (!op->LHS() || !op->RHS())
                return retval;
            if (const ValueRef::Variable<double>* var = dynamic_cast<const ValueRef::Variable<double>*>(op->LHS())) {
                if (const ValueRef::Constant<double>* constant = dynamic_cast<const ValueRef::Constant<double>*>(op->RHS())) {
                    std::string meter_str = UserString(lexical_cast<std::string>(meter));
                    if (!meter_str.empty())
                        meter_str[0] = std::toupper(meter_str[0]);
                    retval.get<0>() = var->PropertyName().size() == 1 &&
                        ("Current" + meter_str) == var->PropertyName()[0];
                    retval.get<1>() = op->GetOpType();
                    retval.get<2>() = constant->Value();
                    return retval;
                }
            } else if (const ValueRef::Variable<double>* var = dynamic_cast<const ValueRef::Variable<double>*>(op->RHS())) {
                if (const ValueRef::Constant<double>* constant = dynamic_cast<const ValueRef::Constant<double>*>(op->LHS())) {
                    std::string meter_str = UserString(lexical_cast<std::string>(meter));
                    if (!meter_str.empty())
                        meter_str[0] = std::toupper(meter_str[0]);
                    retval.get<0>() = var->PropertyName().size() == 1 &&
                        ("Current" + meter_str) == var->PropertyName()[0];
                    retval.get<1>() = op->GetOpType();
                    retval.get<2>() = constant->Value();
                    return retval;
                }
            }
        }
        return retval;
    }

    /** creates a new fleet at a specified \a x and \a y location within the
     * Universe, and and inserts \a ship into it.  Used when a ship has been
     * moved by the MoveTo effect separately from the fleet that previously
     * held it.  All ships need to be within fleets. */
    TemporaryPtr<Fleet> CreateNewFleet(double x, double y, TemporaryPtr<Ship> ship) {
        Universe& universe = GetUniverse();
        if (!ship)
            return TemporaryPtr<Fleet>();

        TemporaryPtr<Fleet> fleet = universe.CreateFleet("", x, y, ship->Owner());

        std::vector<int> ship_ids;
        ship_ids.push_back(ship->ID());
        std::string fleet_name = Fleet::GenerateFleetName(ship_ids, fleet->ID());
        fleet->Rename(fleet_name);
        fleet->GetMeter(METER_STEALTH)->SetCurrent(Meter::LARGE_VALUE);

        fleet->AddShip(ship->ID());
        ship->SetFleetID(fleet->ID());
        fleet->SetAggressive(fleet->HasArmedShips());

        return fleet;
    }

    /** Creates a new fleet at \a system and inserts \a ship into it.  Used
     * when a ship has been moved by the MoveTo effect separately from the
     * fleet that previously held it.  Also used by CreateShip effect to give
     * the new ship a fleet.  All ships need to be within fleets. */
    TemporaryPtr<Fleet> CreateNewFleet(TemporaryPtr<System> system, TemporaryPtr<Ship> ship) {
        if (!system || !ship)
            return TemporaryPtr<Fleet>();

        // remove ship from old fleet / system, put into new system if necessary
        if (ship->SystemID() != system->ID()) {
            if (TemporaryPtr<System> old_system = GetSystem(ship->SystemID())) {
                old_system->Remove(ship->ID());
                ship->SetSystem(INVALID_OBJECT_ID);
            }
            system->Insert(ship);
        }

        if (ship->FleetID() != INVALID_OBJECT_ID) {
            if (TemporaryPtr<Fleet> old_fleet = GetFleet(ship->FleetID())) {
                old_fleet->RemoveShip(ship->ID());
            }
        }

        // create new fleet for ship, and put it in new system
        TemporaryPtr<Fleet> fleet = CreateNewFleet(system->X(), system->Y(), ship);
        system->Insert(fleet);

        return fleet;
    }

    /** Explores the system with the specified \a system_id for the owner of
      * the specified \a target_object.  Used when moving objects into a system
      * with the MoveTo effect, as otherwise the system wouldn't get explored,
      * and objects being moved into unexplored systems might disappear for
      * players or confuse the AI. */
    void ExploreSystem(int system_id, TemporaryPtr<const UniverseObject> target_object) {
        if (!target_object)
            return;
        if (Empire* empire = GetEmpire(target_object->Owner()))
            empire->AddExploredSystem(system_id);
    }

    /** Resets the previous and next systems of \a fleet and recalcultes /
     * resets the fleet's move route.  Used after a fleet has been moved with
     * the MoveTo effect, as its previous route was assigned based on its
     * previous location, and may not be valid for its new location. */
    void UpdateFleetRoute(TemporaryPtr<Fleet> fleet, int new_next_system, int new_previous_system) {
        if (!fleet) {
            ErrorLogger() << "UpdateFleetRoute passed a null fleet pointer";
            return;
        }

        TemporaryPtr<const System> next_system = GetSystem(new_next_system);
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

        std::pair<std::list<int>, double> route_pair = GetUniverse().ShortestPath(start_system, dest_system, fleet->Owner());

        // if shortest path is empty, the route may be impossible or trivial, so just set route to move fleet
        // to the next system that it was just set to move to anyway.
        if (route_pair.first.empty())
            route_pair.first.push_back(new_next_system);


        // set fleet with newly recalculated route
        fleet->SetRoute(route_pair.first);
    }

    std::string GenerateSystemName() {
        static std::list<std::string> star_names;
        if (star_names.empty())
            UserStringList("STAR_NAMES", star_names);

        const ObjectMap& objects = Objects();
        std::vector<TemporaryPtr<const System> > systems = objects.FindObjects<System>();

        // pick a name for the system
        for (std::list<std::string>::const_iterator it = star_names.begin(); it != star_names.end(); ++it) {
            // does an existing system have this name?
            bool dupe = false;
            for (std::vector<TemporaryPtr<const System> >::const_iterator sys_it = systems.begin();
                 sys_it != systems.end(); ++sys_it)
            {
                if ((*sys_it)->Name() == *it) {
                    dupe = true;
                    break;  // another systme has this name. skip to next potential name.
                }
            }
            if (!dupe)
                return *it; // no systems have this name yet. use it.
        }
        return "";  // fallback to empty name.
    }
}


///////////////////////////////////////////////////////////
// EffectsGroup                                          //
///////////////////////////////////////////////////////////
EffectsGroup::~EffectsGroup() {
    delete m_scope;
    delete m_activation;
    for (unsigned int i = 0; i < m_effects.size(); ++i) {
        delete m_effects[i];
    }
}

void EffectsGroup::GetTargetSet(int source_id, TargetSet& targets, const TargetSet& potential_targets) const {
    TargetSet copy_of_potential_targets(potential_targets);
    GetTargetSet(source_id, targets, copy_of_potential_targets);
}

void EffectsGroup::GetTargetSet(int source_id, TargetSet& targets, TargetSet& potential_targets) const {
    targets.clear();

    TemporaryPtr<UniverseObject> source = GetUniverseObject(source_id);
    if (!source && m_activation) {
        ErrorLogger() << "EffectsGroup::GetTargetSet passed invalid source object with id " << source_id;
        return;
    }
    if (!m_scope) {
        ErrorLogger() << "EffectsGroup::GetTargetSet didn't find a valid scope condition to use...";
    }

    // if there is an activation condition, evaluate it on the source object,
    // and abort with no targets if the source object doesn't match.
    // if there is no activation condition, continue as if the source object
    // had matched an activation condition.
    if (m_activation && !m_activation->Eval(ScriptingContext(source), source))
        return;

    BOOST_MPL_ASSERT((boost::is_same<TargetSet,             std::vector<TemporaryPtr<UniverseObject> > >));
    BOOST_MPL_ASSERT((boost::is_same<Condition::ObjectSet,  std::vector<TemporaryPtr<const UniverseObject> > >));

    // HACK! We're doing some dirt here for efficiency's sake.  Since we can't
    // const-cast std::set<TemporaryPtr<UniverseObject> > to std::set<const
    // TemporaryPtr<UniverseObject> >, we're telling the compiler that one type is actually
    // the other, rather than doing a copy.
    m_scope->Eval(ScriptingContext(source),
                  *static_cast<Condition::ObjectSet *>(static_cast<void *>(&targets)),
                  *static_cast<Condition::ObjectSet *>(static_cast<void *>(&potential_targets)));
}

void EffectsGroup::GetTargetSet(int source_id, TargetSet& targets) const {
    TemporaryPtr<UniverseObject> source = GetUniverseObject(source_id);
    //ObjectMap& objects = GetUniverse().Objects();
    TargetSet potential_targets;
    //potential_targets.reserve(objects.NumObjects());
    //for (ObjectMap::iterator<> it = objects.begin(); it != objects.end(); ++it)
    //    potential_targets.push_back(*it);
    m_scope->GetDefaultInitialCandidateObjects(ScriptingContext(source),
            *static_cast<Condition::ObjectSet *>(static_cast<void *>(&potential_targets)));
    GetTargetSet(source_id, targets, potential_targets);
}

void EffectsGroup::Execute(const Effect::TargetsCauses& targets_causes,
                           AccountingMap* accounting_map/* = 0*/,
                           bool only_meter_effects/* = false*/,
                           bool only_appearance_effects/* = false*/,
                           bool include_empire_meter_effects/* = false*/) const
{
    // execute each effect of the group one by one, unless filtered by flags
    for (std::vector<EffectBase*>::const_iterator effect_it = m_effects.begin();
         effect_it != m_effects.end(); ++effect_it)
    {
        (*effect_it)->Execute(targets_causes,
                              m_stacking_group.empty(), /* bool stacking */
                              accounting_map,
                              only_meter_effects,
                              only_appearance_effects,
                              include_empire_meter_effects);
    }
}

std::string EffectsGroup::DescriptionString() const {
    std::stringstream retval;

    if (dynamic_cast<const Condition::Source*>(m_scope))
        retval << UserString("DESC_EFFECTS_GROUP_SELF_SCOPE") + "\n";
    else
        retval << str(FlexibleFormat(UserString("DESC_EFFECTS_GROUP_SCOPE")) % m_scope->Description()) + "\n";

    if (m_activation && !dynamic_cast<const Condition::Source*>(m_activation) && !dynamic_cast<const Condition::All*>(m_activation))
    { retval << str(FlexibleFormat(UserString("DESC_EFFECTS_GROUP_ACTIVATION")) % m_activation->Description()) + "\n"; }

    for (unsigned int i = 0; i < m_effects.size(); ++i)
    { retval << m_effects[i]->Description() + "\n"; }

    return retval.str();
}

std::string EffectsGroup::Dump() const {
    std::string retval = DumpIndent() + "EffectsGroup\n";
    ++g_indent;
    retval += DumpIndent() + "scope =\n";
    ++g_indent;
    retval += m_scope->Dump();
    --g_indent;
    if (m_activation) {
        retval += DumpIndent() + "activation =\n";
        ++g_indent;
        retval += m_activation->Dump();
        --g_indent;
    }
    if (!m_stacking_group.empty())
        retval += DumpIndent() + "stackinggroup = \"" + m_stacking_group + "\"\n";
    if (m_effects.size() == 1) {
        retval += DumpIndent() + "effects =\n";
        ++g_indent;
        retval += m_effects[0]->Dump();
        --g_indent;
    } else {
        retval += DumpIndent() + "effects = [\n";
        ++g_indent;
        for (unsigned int i = 0; i < m_effects.size(); ++i) {
            retval += m_effects[i]->Dump();
        }
        --g_indent;
        retval += DumpIndent() + "]\n";
    }
    --g_indent;
    return retval;
}

void EffectsGroup::SetTopLevelContent(const std::string& content_name) {
    if (m_scope)
        m_scope->SetTopLevelContent(content_name);
    if (m_activation)
        m_activation->SetTopLevelContent(content_name);
    for (std::vector<EffectBase*>::iterator it = m_effects.begin(); it != m_effects.end(); ++it)
    { (*it)->SetTopLevelContent(content_name); }
}


///////////////////////////////////////////////////////////
// EffectsDescription function                           //
///////////////////////////////////////////////////////////
std::string EffectsDescription(const std::vector<boost::shared_ptr<Effect::EffectsGroup> >& effects_groups) {
    std::stringstream retval;
    if (effects_groups.size() == 1) {
        retval << str(FlexibleFormat(UserString("DESC_EFFECTS_GROUP_EFFECTS_GROUP_DESC")) % effects_groups[0]->DescriptionString());
    } else {
        for (unsigned int i = 0; i < effects_groups.size(); ++i) {
            retval << str(FlexibleFormat(UserString("DESC_EFFECTS_GROUP_NUMBERED_EFFECTS_GROUP_DESC")) % (i + 1) % effects_groups[i]->DescriptionString());
        }
    }
    return retval.str();
}


///////////////////////////////////////////////////////////
// EffectBase                                            //
///////////////////////////////////////////////////////////
EffectBase::~EffectBase()
{}

void EffectBase::Execute(const Effect::TargetsCauses& targets_causes,
                         bool stacking,
                         AccountingMap* accounting_map/* = 0*/,
                         bool only_meter_effects/* = false*/,
                         bool only_appearance_effects/* = false*/,
                         bool include_empire_meter_effects/* = false*/) const
{
    bool log_verbose = GetOptionsDB().Get<bool>("verbose-logging");

    std::set<int> non_stacking_targets;
    MeterType meter_type = INVALID_METER_TYPE;

    // for meter effects, need to separately call effect's Execute for each
    // target and do meter accounting before and after.
    const SetMeter* set_meter_effect = 0;
    const SetShipPartMeter* set_ship_part_meter_effect = 0;

    if (set_meter_effect = dynamic_cast<const SetMeter*>(this)) {
        meter_type = set_meter_effect->GetMeterType();

    } else if (set_ship_part_meter_effect = dynamic_cast<const SetShipPartMeter*>(this)) {
        meter_type = set_ship_part_meter_effect->GetMeterType();
    }

    // filter executed effects according to flags
    if (only_appearance_effects) {
        if (!dynamic_cast<const SetTexture*>(this) && !dynamic_cast<const SetOverlayTexture*>(this))
            return;
    } else if (only_meter_effects) {
        if (!set_meter_effect && !set_ship_part_meter_effect) {
            if (!include_empire_meter_effects)
                return;
            if (!dynamic_cast<const SetEmpireMeter*>(this))
                return;
        }
    }

    // apply this effect to each source causing it
    for (Effect::TargetsCauses::const_iterator targets_it = targets_causes.begin();
        targets_it != targets_causes.end(); ++targets_it)
    {
        const Effect::SourcedEffectsGroup& sourced_effects_group = targets_it->first;
        int                                source_id             = sourced_effects_group.source_object_id;
        TemporaryPtr<const UniverseObject> source                = GetUniverseObject(source_id);
        ScriptingContext                   source_context(source);
        const Effect::TargetsAndCause&     targets_and_cause     = targets_it->second;
        Effect::TargetSet                  targets               = targets_and_cause.target_set;

        if (log_verbose) {
            DebugLogger() << "ExecuteEffects effectsgroup: \n" << Dump();
            DebugLogger() << "ExecuteEffects Targets before: ";
            for (Effect::TargetSet::const_iterator t_it = targets.begin(); t_it != targets.end(); ++t_it)
                DebugLogger() << " ... " << (*t_it)->Dump();
        }

        if (log_verbose) {
            DebugLogger() << "ExecuteEffects Targets after: ";
            for (Effect::TargetSet::const_iterator t_it = targets.begin(); t_it != targets.end(); ++t_it)
                DebugLogger() << " ... " << (*t_it)->Dump();
        }

        // for non-meter effects, can do default batch execute
        if (!accounting_map || (!set_meter_effect && !set_ship_part_meter_effect)) {
            Execute(source_context, targets);
            continue;
        }

        // accounting info for this effect on this meter, starting with
        // non-target-dependent info
        Effect::AccountingInfo info;
        info.cause_type =           targets_and_cause.effect_cause.cause_type;
        info.specific_cause =       targets_and_cause.effect_cause.specific_cause;
        info.custom_label =         targets_and_cause.effect_cause.custom_label;
        info.source_id =            source_id;

        // process each target separately to do effect accounting
        for (TargetSet::const_iterator target_it = targets.begin();
            target_it != targets.end(); ++target_it)
        {
            TemporaryPtr<UniverseObject> target = *target_it;

            // get Meter for this effect and target
            const Meter* meter = 0;

            if (set_meter_effect) {
                meter = target->GetMeter(meter_type);

            } else if (set_ship_part_meter_effect) {
                if (target->ObjectType() != OBJ_SHIP)
                    continue;   // only ships have ship part meters
                TemporaryPtr<const Ship> ship = boost::static_pointer_cast<const Ship>(target);

                const ValueRef::ValueRefBase<std::string>* part_name_value_ref = set_ship_part_meter_effect->GetPartName();
                std::string part_name = (part_name_value_ref ? part_name_value_ref->Eval(ScriptingContext(source, target)) : "");
                meter = ship->GetPartMeter(meter_type, part_name);
            }

            if (!meter)
                continue;   // some objects might match target conditions, but not actually have the relevant meter


            // record pre-effect meter values...

            // accounting info for this effect on this meter of this target
            info.running_meter_total =  meter->Current();

            // actually execute effect to modify meter
            Execute(ScriptingContext(source, target));

            // update for meter change and new total
            info.meter_change = meter->Current() - info.running_meter_total;
            info.running_meter_total = meter->Current();

            // add accounting for this effect to end of vector
            (*accounting_map)[target->ID()][meter_type].push_back(info);
        }
    }
}

void EffectBase::Execute(const ScriptingContext& context, const TargetSet& targets) const {
    if (targets.empty())
        return;
    // execute effects on targets
    ScriptingContext local_context = context;
    for (TargetSet::const_iterator target_it = targets.begin();
         target_it != targets.end(); ++target_it)
    {
        local_context.effect_target = *target_it;
        this->Execute(local_context);
    }
}


///////////////////////////////////////////////////////////
// SetMeter                                              //
///////////////////////////////////////////////////////////
SetMeter::SetMeter(MeterType meter, ValueRef::ValueRefBase<double>* value) :
    m_meter(meter),
    m_value(value)
{}

SetMeter::~SetMeter()
{ delete m_value; }

void SetMeter::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) return;
    Meter* m = context.effect_target->GetMeter(m_meter);
    if (!m) return;

    float val = m_value->Eval(ScriptingContext(context, m->Current()));
    m->SetCurrent(val);
}

void SetMeter::Execute(const ScriptingContext& context, const TargetSet& targets) const {
    if (targets.empty())
        return;
    // does meter value depend on target?
    if (m_value->TargetInvariant()) {
        float val = m_value->Eval(context);
        for (TargetSet::const_iterator it = targets.begin(); it != targets.end(); ++it) {
            Meter* m = (*it)->GetMeter(m_meter);
            if (!m) continue;
            m->SetCurrent(val);
        }
        return;
    }
    // meter value does depend on target, so handle with default case
    EffectBase::Execute(context, targets);
}

std::string SetMeter::Description() const {
    bool simple;
    ValueRef::OpType op;
    double const_operand;
    boost::tie(simple, op, const_operand) = SimpleMeterModification(m_meter, m_value);
    //DebugLogger() << "SetMeter::Description " << simple << " / " << op << " / " << const_operand;
    if (simple) {
        char op_char = '+';
        switch (op) {
        case ValueRef::PLUS:            op_char = '+'; break;
        case ValueRef::MINUS:           op_char = '-'; break;
        case ValueRef::TIMES:           op_char = '*'; break;
        case ValueRef::DIVIDE:          op_char = '/'; break;
        case ValueRef::EXPONENTIATE:    op_char = '^'; break;
        default: op_char = '?';
        }
        return str(FlexibleFormat(UserString("DESC_SIMPLE_SET_METER"))
                   % UserString(lexical_cast<std::string>(m_meter))
                   % op_char
                   % lexical_cast<std::string>(const_operand));
    } else {
        //std::string temp = m_value->Description();
        return str(FlexibleFormat(UserString("DESC_COMPLEX_SET_METER"))
                   % UserString(lexical_cast<std::string>(m_meter))
                   % m_value->Description());
    }
}

std::string SetMeter::Dump() const {
    std::string retval = DumpIndent() + "Set";
    switch (m_meter) {
    case METER_TARGET_POPULATION:   retval += "TargetPopulation"; break;
    case METER_TARGET_INDUSTRY:     retval += "TargetIndustry"; break;
    case METER_TARGET_RESEARCH:     retval += "TargetResearch"; break;
    case METER_TARGET_TRADE:        retval += "TargetTrade"; break;
    case METER_TARGET_CONSTRUCTION: retval += "TargetConstruction"; break;
    case METER_TARGET_HAPPINESS:    retval += "TargetHappiness"; break;

    case METER_MAX_FUEL:            retval += "MaxFuel"; break;
    case METER_MAX_SHIELD:          retval += "MaxShield"; break;
    case METER_MAX_STRUCTURE:       retval += "MaxStructure"; break;
    case METER_MAX_DEFENSE:         retval += "MaxDefense"; break;
    case METER_MAX_TROOPS:          retval += "MaxTroops"; break;

    case METER_POPULATION:          retval += "Population"; break;
    case METER_INDUSTRY:            retval += "Industry"; break;
    case METER_RESEARCH:            retval += "Research"; break;
    case METER_TRADE:               retval += "Trade"; break;
    case METER_CONSTRUCTION:        retval += "Construction"; break;
    case METER_HAPPINESS:           retval += "Happiness"; break;

    case METER_FUEL:                retval += "Fuel"; break;
    case METER_SHIELD:              retval += "Shield"; break;
    case METER_STRUCTURE:           retval += "Structure"; break;
    case METER_DEFENSE:             retval += "Defense"; break;
    case METER_TROOPS:              retval += "Troops"; break;

    case METER_REBEL_TROOPS:        retval += "RebelTroops"; break;
    case METER_SUPPLY:              retval += "Supply"; break;
    case METER_STEALTH:             retval += "Stealth"; break;
    case METER_DETECTION:           retval += "Detection"; break;
    case METER_SPEED:               retval += "Speed"; break;

    default:                        retval += "?"; break;
    }
    retval += " value = " + m_value->Dump() + "\n";
    return retval;
}

void SetMeter::SetTopLevelContent(const std::string& content_name) {
    if (m_value)
        m_value->SetTopLevelContent(content_name);
}


///////////////////////////////////////////////////////////
// SetShipPartMeter                                      //
///////////////////////////////////////////////////////////
SetShipPartMeter::SetShipPartMeter(MeterType meter,
                                   ValueRef::ValueRefBase<std::string>* part_name,
                                   ValueRef::ValueRefBase<double>* value) :
    m_part_name(part_name),
    m_meter(meter),
    m_value(value)
{}

SetShipPartMeter::~SetShipPartMeter() {
    delete m_value;
    delete m_part_name;
}

void SetShipPartMeter::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        DebugLogger() << "SetShipPartMeter::Execute passed null target pointer";
        return;
    }

    if (!m_part_name || !m_value) {
        ErrorLogger() << "SetShipPartMeter::Execute missing part name or value ValueRefs";
        return;
    }

    TemporaryPtr<Ship> ship = boost::dynamic_pointer_cast<Ship>(context.effect_target);
    if (!ship) {
        ErrorLogger() << "SetShipPartMeter::Execute acting on non-ship target:";
        context.effect_target->Dump();
        return;
    }

    std::string part_name = m_part_name->Eval(context);

    // get meter, evaluate new value, assign
    Meter* meter = ship->GetPartMeter(m_meter, part_name);
    if (!meter) {
        ErrorLogger() << "SetShipPartMeter::Execute couldn't find meter " << m_meter << " for ship part name: " << part_name;
        return;
    }

    double val = m_value->Eval(ScriptingContext(context, meter->Current()));
    meter->SetCurrent(val);
}

std::string SetShipPartMeter::Description() const {
    std::string value_str;
    if (m_value) {
        if (ValueRef::ConstantExpr(m_value))
            value_str = lexical_cast<std::string>(m_value->Eval());
        else
            value_str = m_value->Description();
    }

    std::string meter_str = UserString(lexical_cast<std::string>(m_meter));

    std::string part_str;
    if (m_part_name) {
        part_str = m_part_name->Description();
        if (ValueRef::ConstantExpr(m_part_name) && UserStringExists(part_str))
            part_str = UserString(part_str);
    }

    return str(FlexibleFormat(UserString("DESC_SET_SHIP_PART_METER"))
               % meter_str
               % part_str
               % value_str);
}

std::string SetShipPartMeter::Dump() const {
    std::string retval = DumpIndent();
    switch (m_meter) {
        case METER_DAMAGE:              retval += "SetDamage";              break;
        case METER_SPEED:               retval += "SetSpeed";               break;
        case METER_CAPACITY:            retval += "SetCapacity";            break;
        case METER_STEALTH:             retval += "SetStealth";             break;
        case METER_STRUCTURE:           retval += "SetStructure";           break;
        case METER_DETECTION:           retval += "SetDetection";           break;
        default:                        retval += "Set????";                break;
    }

    if (m_part_name)
        retval += " partname = " + m_part_name->Dump();

    retval += " value = " + m_value->Dump();

    return retval;
}

void SetShipPartMeter::SetTopLevelContent(const std::string& content_name) {
    if (m_value)
        m_value->SetTopLevelContent(content_name);
    if (m_part_name)
        m_part_name->SetTopLevelContent(content_name);
}


///////////////////////////////////////////////////////////
// SetEmpireMeter                                        //
///////////////////////////////////////////////////////////
SetEmpireMeter::SetEmpireMeter(const std::string& meter, ValueRef::ValueRefBase<double>* value) :
    m_empire_id(new ValueRef::Variable<int>(ValueRef::EFFECT_TARGET_REFERENCE, std::vector<std::string>(1, "Owner"))),
    m_meter(meter),
    m_value(value)
{}

SetEmpireMeter::SetEmpireMeter(ValueRef::ValueRefBase<int>* empire_id, const std::string& meter,
                               ValueRef::ValueRefBase<double>* value) :
    m_empire_id(empire_id),
    m_meter(meter),
    m_value(value)
{}

SetEmpireMeter::~SetEmpireMeter() {
    delete m_empire_id;
    delete m_value;
}

void SetEmpireMeter::Execute(const ScriptingContext& context) const {
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

    double value = m_value->Eval(ScriptingContext(context, meter->Current()));

    meter->SetCurrent(value);
}

std::string SetEmpireMeter::Description() const {
    std::string empire_str;
    if (m_empire_id) {
        if (ValueRef::ConstantExpr(m_empire_id)) {
            if (const Empire* empire = GetEmpire(m_empire_id->Eval()))
                empire_str = empire->Name();
        } else {
            empire_str = m_empire_id->Description();
        }
    }
    std::string value_str =     ValueRef::ConstantExpr(m_value) ?
                                    lexical_cast<std::string>(m_value->Eval()) :
                                    m_value->Description();

    return str(FlexibleFormat(UserString("DESC_SET_EMPIRE_METER"))
               % empire_str
               % UserString(m_meter)
               % value_str);
}

std::string SetEmpireMeter::Dump() const
{ return DumpIndent() + "SetEmpireMeter meter = " + m_meter + " empire = " + m_empire_id->Dump() + " value = " + m_value->Dump(); }

void SetEmpireMeter::SetTopLevelContent(const std::string& content_name) {
    if (m_empire_id)
        m_empire_id->SetTopLevelContent(content_name);
    if (m_value)
        m_value->SetTopLevelContent(content_name);
}


///////////////////////////////////////////////////////////
// SetEmpireStockpile                                    //
///////////////////////////////////////////////////////////
SetEmpireStockpile::SetEmpireStockpile(ResourceType stockpile,
                                       ValueRef::ValueRefBase<double>* value) :
    m_empire_id(new ValueRef::Variable<int>(ValueRef::EFFECT_TARGET_REFERENCE, std::vector<std::string>(1, "Owner"))),
    m_stockpile(stockpile),
    m_value(value)
{}

SetEmpireStockpile::SetEmpireStockpile(ValueRef::ValueRefBase<int>* empire_id,
                                       ResourceType stockpile,
                                       ValueRef::ValueRefBase<double>* value) :
    m_empire_id(empire_id),
    m_stockpile(stockpile),
    m_value(value)
{}

SetEmpireStockpile::~SetEmpireStockpile() {
    delete m_empire_id;
    delete m_value;
}

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

std::string SetEmpireStockpile::Description() const {
    std::string empire_str;
    if (m_empire_id) {
        if (ValueRef::ConstantExpr(m_empire_id)) {
            if (const Empire* empire = GetEmpire(m_empire_id->Eval()))
                empire_str = empire->Name();
        } else {
            empire_str = m_empire_id->Description();
        }
    }
    std::string value_str = ValueRef::ConstantExpr(m_value) ?
                                lexical_cast<std::string>(m_value->Eval()) :
                                m_value->Description();

    return str(FlexibleFormat(UserString("DESC_SET_EMPIRE_STOCKPILE"))
               % UserString(lexical_cast<std::string>(m_stockpile))
               % value_str
               % empire_str);
}

std::string SetEmpireStockpile::Dump() const {
    std::string retval = DumpIndent();
    switch (m_stockpile) {
    case RE_TRADE:      retval += "SetEmpireTradeStockpile"; break;
    default:            retval += "?"; break;
    }
    retval += " empire = " + m_empire_id->Dump() + " value = " + m_value->Dump() + "\n";
    return retval;
}

void SetEmpireStockpile::SetTopLevelContent(const std::string& content_name) {
    if (m_empire_id)
        m_empire_id->SetTopLevelContent(content_name);
    if (m_value)
        m_value->SetTopLevelContent(content_name);
}


///////////////////////////////////////////////////////////
// SetEmpireCapital                                      //
///////////////////////////////////////////////////////////
SetEmpireCapital::SetEmpireCapital() :
    m_empire_id(new ValueRef::Variable<int>(ValueRef::EFFECT_TARGET_REFERENCE, std::vector<std::string>(1, "Owner")))
{}

SetEmpireCapital::SetEmpireCapital(ValueRef::ValueRefBase<int>* empire_id) :
    m_empire_id(empire_id)
{}

SetEmpireCapital::~SetEmpireCapital()
{ delete m_empire_id; }

void SetEmpireCapital::Execute(const ScriptingContext& context) const {
    int empire_id = m_empire_id->Eval(context);

    Empire* empire = GetEmpire(empire_id);
    if (!empire)
        return;

    TemporaryPtr<const Planet> planet = boost::dynamic_pointer_cast<const Planet>(context.effect_target);
    if (!planet)
        return;

    empire->SetCapitalID(planet->ID());
}

std::string SetEmpireCapital::Description() const {
    std::string empire_str;
    if (m_empire_id) {
        if (ValueRef::ConstantExpr(m_empire_id)) {
            if (const Empire* empire = GetEmpire(m_empire_id->Eval()))
                empire_str = empire->Name();
        } else {
            empire_str = m_empire_id->Description();
        }
    }
    return str(FlexibleFormat(UserString("DESC_SET_EMPIRE_CAPITAL")) % empire_str);
}

std::string SetEmpireCapital::Dump() const
{ return DumpIndent() + "SetEmpireCapital empire = " + m_empire_id->Dump() + "\n"; }

void SetEmpireCapital::SetTopLevelContent(const std::string& content_name) {
    if (m_empire_id)
        m_empire_id->SetTopLevelContent(content_name);
}


///////////////////////////////////////////////////////////
// SetPlanetType                                         //
///////////////////////////////////////////////////////////
SetPlanetType::SetPlanetType(ValueRef::ValueRefBase<PlanetType>* type) :
    m_type(type)
{}

SetPlanetType::~SetPlanetType()
{ delete m_type; }

void SetPlanetType::Execute(const ScriptingContext& context) const {
    if (TemporaryPtr<Planet> p = boost::dynamic_pointer_cast<Planet>(context.effect_target)) {
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

std::string SetPlanetType::Description() const {
    std::string value_str = ValueRef::ConstantExpr(m_type) ?
                                UserString(lexical_cast<std::string>(m_type->Eval())) :
                                m_type->Description();
    return str(FlexibleFormat(UserString("DESC_SET_PLANET_TYPE")) % value_str);
}

std::string SetPlanetType::Dump() const
{ return DumpIndent() + "SetPlanetType type = " + m_type->Dump() + "\n"; }

void SetPlanetType::SetTopLevelContent(const std::string& content_name) {
    if (m_type)
        m_type->SetTopLevelContent(content_name);
}


///////////////////////////////////////////////////////////
// SetPlanetSize                                         //
///////////////////////////////////////////////////////////
SetPlanetSize::SetPlanetSize(ValueRef::ValueRefBase<PlanetSize>* size) :
    m_size(size)
{}

SetPlanetSize::~SetPlanetSize()
{ delete m_size; }

void SetPlanetSize::Execute(const ScriptingContext& context) const {
    if (TemporaryPtr<Planet> p = boost::dynamic_pointer_cast<Planet>(context.effect_target)) {
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

std::string SetPlanetSize::Description() const {
    std::string value_str = ValueRef::ConstantExpr(m_size) ?
                                UserString(lexical_cast<std::string>(m_size->Eval())) :
                                m_size->Description();
    return str(FlexibleFormat(UserString("DESC_SET_PLANET_SIZE")) % value_str);
}

std::string SetPlanetSize::Dump() const
{ return DumpIndent() + "SetPlanetSize size = " + m_size->Dump() + "\n"; }

void SetPlanetSize::SetTopLevelContent(const std::string& content_name) {
    if (m_size)
        m_size->SetTopLevelContent(content_name);
}


///////////////////////////////////////////////////////////
// SetSpecies                                            //
///////////////////////////////////////////////////////////
SetSpecies::SetSpecies(ValueRef::ValueRefBase<std::string>* species) :
    m_species_name(species)
{}

SetSpecies::~SetSpecies()
{ delete m_species_name; }

void SetSpecies::Execute(const ScriptingContext& context) const {
    if (TemporaryPtr<Planet> planet = boost::dynamic_pointer_cast<Planet>(context.effect_target)) {
        std::string species_name = m_species_name->Eval(ScriptingContext(context, planet->SpeciesName()));
        planet->SetSpecies(species_name);

        // ensure non-empty and permissible focus setting for new species
        std::string initial_focus = planet->Focus();
        std::vector<std::string> available_foci = planet->AvailableFoci();

        // leave current focus unchanged if available.
        for (std::vector<std::string>::const_iterator it = available_foci.begin();
             it != available_foci.end(); ++it)
        {
            if (*it == initial_focus) {
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
        for (std::vector<std::string>::const_iterator it = available_foci.begin();
                it != available_foci.end(); ++it)
        {
            if (*it == preferred_focus) {
                preferred_available = true;
                break;
            }
        }

        if (preferred_available) {
            new_focus = preferred_focus;
        } else if (!available_foci.empty()) {
            new_focus = *available_foci.begin();
        }

        planet->SetFocus(new_focus);

    } else if (TemporaryPtr<Ship> ship = boost::dynamic_pointer_cast<Ship>(context.effect_target)) {
        std::string species_name = m_species_name->Eval(ScriptingContext(context, ship->SpeciesName()));
        ship->SetSpecies(species_name);
    }
}

std::string SetSpecies::Description() const {
    std::string value_str = ValueRef::ConstantExpr(m_species_name) ?
                                UserString(m_species_name->Eval()) :
                                m_species_name->Description();
    return str(FlexibleFormat(UserString("DESC_SET_SPECIES")) % value_str);
}

std::string SetSpecies::Dump() const
{ return DumpIndent() + "SetSpecies name = " + m_species_name->Dump() + "\n"; }

void SetSpecies::SetTopLevelContent(const std::string& content_name) {
    if (m_species_name)
        m_species_name->SetTopLevelContent(content_name);
}


///////////////////////////////////////////////////////////
// SetOwner                                              //
///////////////////////////////////////////////////////////
SetOwner::SetOwner(ValueRef::ValueRefBase<int>* empire_id) :
    m_empire_id(empire_id)
{}

SetOwner::~SetOwner()
{ delete m_empire_id; }

void SetOwner::Execute(const ScriptingContext& context) const {
    if (!context.effect_target)
        return;
    int initial_owner = context.effect_target->Owner();

    int empire_id = m_empire_id->Eval(ScriptingContext(context, initial_owner));
    if (initial_owner == empire_id)
        return;

    context.effect_target->SetOwner(empire_id);

    if (TemporaryPtr<Ship> ship = boost::dynamic_pointer_cast<Ship>(context.effect_target)) {
        // assigning ownership of a ship requires updating the containing
        // fleet, or splitting ship off into a new fleet at the same location
        TemporaryPtr<Fleet> fleet = GetFleet(ship->FleetID());
        if (!fleet)
            return;
        if (fleet->Owner() == empire_id)
            return;

        // move ship into new fleet
        TemporaryPtr<Fleet> new_fleet;
        if (TemporaryPtr<System> system = GetSystem(ship->SystemID()))
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

std::string SetOwner::Description() const {
    std::string empire_str;
    if (m_empire_id) {
        if (ValueRef::ConstantExpr(m_empire_id)) {
            if (const Empire* empire = GetEmpire(m_empire_id->Eval()))
                empire_str = empire->Name();
        } else {
            empire_str = m_empire_id->Description();
        }
    }
    return str(FlexibleFormat(UserString("DESC_SET_OWNER")) % empire_str);
}

std::string SetOwner::Dump() const
{ return DumpIndent() + "SetOwner empire = " + m_empire_id->Dump() + "\n"; }

void SetOwner::SetTopLevelContent(const std::string& content_name) {
    if (m_empire_id)
        m_empire_id->SetTopLevelContent(content_name);
}


///////////////////////////////////////////////////////////
// SetSpeciesEmpireOpinion                               //
///////////////////////////////////////////////////////////
SetSpeciesEmpireOpinion::SetSpeciesEmpireOpinion(ValueRef::ValueRefBase<std::string>* species_name,
                                                 ValueRef::ValueRefBase<int>* empire_id,
                                                 ValueRef::ValueRefBase<double>* opinion) :
    m_species_name(species_name),
    m_empire_id(empire_id),
    m_opinion(opinion)
{}

SetSpeciesEmpireOpinion::~SetSpeciesEmpireOpinion() {
    delete m_species_name;
    delete m_empire_id;
    delete m_opinion;
}

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

std::string SetSpeciesEmpireOpinion::Description() const {
    std::string empire_str;
    if (m_empire_id) {
        if (ValueRef::ConstantExpr(m_empire_id)) {
            if (const Empire* empire = GetEmpire(m_empire_id->Eval()))
                empire_str = empire->Name();
        } else {
            empire_str = m_empire_id->Description();
        }
    }
    return str(FlexibleFormat(UserString("DESC_SET_OWNER")) % empire_str);
    // todo: fix
}

std::string SetSpeciesEmpireOpinion::Dump() const
{ return DumpIndent() + "SetSpeciesEmpireOpinion empire = " + m_empire_id->Dump() + "\n"; }

void SetSpeciesEmpireOpinion::SetTopLevelContent(const std::string& content_name) {
    if (m_empire_id)
        m_empire_id->SetTopLevelContent(content_name);
    if (m_species_name)
        m_species_name->SetTopLevelContent(content_name);
    if (m_opinion)
        m_opinion->SetTopLevelContent(content_name);
}


///////////////////////////////////////////////////////////
// SetSpeciesSpeciesOpinion                              //
///////////////////////////////////////////////////////////
SetSpeciesSpeciesOpinion::SetSpeciesSpeciesOpinion(ValueRef::ValueRefBase<std::string>* opinionated_species_name,
                                                   ValueRef::ValueRefBase<std::string>* rated_species_name,
                                                   ValueRef::ValueRefBase<double>* opinion) :
    m_opinionated_species_name(opinionated_species_name),
    m_rated_species_name(rated_species_name),
    m_opinion(opinion)
{}

SetSpeciesSpeciesOpinion::~SetSpeciesSpeciesOpinion() {
    delete m_opinionated_species_name;
    delete m_rated_species_name;
    delete m_opinion;
}

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

    double initial_opinion = GetSpeciesManager().SpeciesSpeciesOpinion(opinionated_species_name, rated_species_name);
    double opinion = m_opinion->Eval(ScriptingContext(context, initial_opinion));

    GetSpeciesManager().SetSpeciesSpeciesOpinion(opinionated_species_name, rated_species_name, opinion);
}

std::string SetSpeciesSpeciesOpinion::Description() const {
    std::string empire_str;
    //if (m_empire_id) {
    //    if (ValueRef::ConstantExpr(m_empire_id)) {
    //        if (const Empire* empire = GetEmpire(m_empire_id->Eval()))
    //            empire_str = empire->Name();
    //    } else {
    //        empire_str = m_empire_id->Description();
    //    }
    //}
    return str(FlexibleFormat(UserString("DESC_SET_OWNER")) % empire_str);
    // todo: fix
}

std::string SetSpeciesSpeciesOpinion::Dump() const
{ return DumpIndent() + "SetSpeciesSpeciesOpinion" + "\n"; }

void SetSpeciesSpeciesOpinion::SetTopLevelContent(const std::string& content_name) {
    if (m_opinionated_species_name)
        m_opinionated_species_name->SetTopLevelContent(content_name);
    if (m_rated_species_name)
        m_rated_species_name->SetTopLevelContent(content_name);
    if (m_opinion)
        m_opinion->SetTopLevelContent(content_name);
}


///////////////////////////////////////////////////////////
// CreatePlanet                                          //
///////////////////////////////////////////////////////////
CreatePlanet::CreatePlanet(ValueRef::ValueRefBase<PlanetType>* type,
                           ValueRef::ValueRefBase<PlanetSize>* size,
                           ValueRef::ValueRefBase<std::string>* name) :
    m_type(type),
    m_size(size),
    m_name(name)
{}

CreatePlanet::~CreatePlanet() {
    delete m_type;
    delete m_size;
    delete m_name;
}

void CreatePlanet::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger() << "CreatePlanet::Execute passed no target object";
        return;
    }
    TemporaryPtr<System> location = GetSystem(context.effect_target->SystemID());
    if (!location) {
        ErrorLogger() << "CreatePlanet::Execute couldn't get a System object at which to create the planet";
        return;
    }

    PlanetSize target_size = INVALID_PLANET_SIZE;
    PlanetType target_type = INVALID_PLANET_TYPE;
    if (TemporaryPtr<const Planet> location_planet = boost::dynamic_pointer_cast<const Planet>(context.effect_target)) {
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
    std::set<int> free_orbits = location->FreeOrbits();
    if (free_orbits.empty()) {
        ErrorLogger() << "CreatePlanet::Execute couldn't find any free orbits in system where planet was to be created";
        return;
    }

    TemporaryPtr<Planet> planet = GetUniverse().CreatePlanet(type, size);
    if (!planet) {
        ErrorLogger() << "CreatePlanet::Execute unable to create new Planet object";
        return;
    }

    location->Insert(planet);   // let system chose an orbit for planet

    std::string name;
    if (m_name) {
        name = m_name->Eval(context);
        if (ValueRef::ConstantExpr(m_name) && UserStringExists(name))
            name = UserString(name);
    } else {
        name = str(FlexibleFormat(UserString("NEW_PLANET_NAME")) % location->Name());
    }
    planet->Rename(name);
}

std::string CreatePlanet::Description() const {
    std::string type_str = ValueRef::ConstantExpr(m_type) ?
                                UserString(lexical_cast<std::string>(m_type->Eval())) :
                                m_type->Description();
    std::string size_str = ValueRef::ConstantExpr(m_size) ?
                                UserString(lexical_cast<std::string>(m_size->Eval())) :
                                m_size->Description();

    return str(FlexibleFormat(UserString("DESC_CREATE_PLANET"))
                % type_str
                % size_str);
}

std::string CreatePlanet::Dump() const {
    std::string retval = DumpIndent() + "CreatePlanet";
    if (m_size)
        retval += " size = " + m_size->Dump();
    if (m_type)
        retval += " type = " + m_type->Dump();
    if (m_name)
        retval += " name = " + m_name->Dump();
    return retval + "\n";
}

void CreatePlanet::SetTopLevelContent(const std::string& content_name) {
    if (m_type)
        m_type->SetTopLevelContent(content_name);
    if (m_size)
        m_size->SetTopLevelContent(content_name);
    if (m_name)
        m_name->SetTopLevelContent(content_name);
}


///////////////////////////////////////////////////////////
// CreateBuilding                                        //
///////////////////////////////////////////////////////////
CreateBuilding::CreateBuilding(ValueRef::ValueRefBase<std::string>* building_type_name,
                               ValueRef::ValueRefBase<std::string>* name) :
    m_building_type_name(building_type_name),
    m_name(name)
{}

CreateBuilding::~CreateBuilding() {
    delete m_building_type_name;
    delete m_name;
}

void CreateBuilding::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger() << "CreateBuilding::Execute passed no target object";
        return;
    }
    TemporaryPtr<Planet> location = boost::dynamic_pointer_cast<Planet>(context.effect_target);
    if (!location)
        if (TemporaryPtr<Building> location_building = boost::dynamic_pointer_cast<Building>(context.effect_target))
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

    TemporaryPtr<Building> building = GetUniverse().CreateBuilding(ALL_EMPIRES, building_type_name, ALL_EMPIRES);
    if (!building) {
        ErrorLogger() << "CreateBuilding::Execute couldn't create building!";
        return;
    }

    location->AddBuilding(building->ID());
    building->SetPlanetID(location->ID());

    building->SetOwner(location->Owner());

    TemporaryPtr<System> system = GetSystem(location->SystemID());
    if (system)
        system->Insert(building);

    if (m_name) {
        std::string name = m_name->Eval(context);
        if (ValueRef::ConstantExpr(m_name) && UserStringExists(name))
            name = UserString(name);
        building->Rename(name);
    }
}

std::string CreateBuilding::Description() const {
    std::string type_str = ValueRef::ConstantExpr(m_building_type_name) ?
                                UserString(lexical_cast<std::string>(m_building_type_name->Eval())) :
                                m_building_type_name->Description();

    return str(FlexibleFormat(UserString("DESC_CREATE_BUILDING"))
                % type_str);
}

std::string CreateBuilding::Dump() const {
    std::string retval = DumpIndent() + "CreateBuilding";
    if (m_building_type_name)
        retval += " type = " + m_building_type_name->Dump();
    if (m_name)
        retval += " name = " + m_name->Dump();
    return retval + "\n";
}

void CreateBuilding::SetTopLevelContent(const std::string& content_name) {
    if (m_building_type_name)
        m_building_type_name->SetTopLevelContent(content_name);
    if (m_name)
        m_name->SetTopLevelContent(content_name);
}


///////////////////////////////////////////////////////////
// CreateShip                                            //
///////////////////////////////////////////////////////////
CreateShip::CreateShip(ValueRef::ValueRefBase<std::string>* predefined_ship_design_name,
                       ValueRef::ValueRefBase<int>* empire_id,
                       ValueRef::ValueRefBase<std::string>* species_name,
                       ValueRef::ValueRefBase<std::string>* ship_name) :
    m_design_name(predefined_ship_design_name),
    m_design_id(0),
    m_empire_id(empire_id),
    m_species_name(species_name),
    m_name(ship_name)
{}

CreateShip::CreateShip(ValueRef::ValueRefBase<int>* ship_design_id,
                       ValueRef::ValueRefBase<int>* empire_id,
                       ValueRef::ValueRefBase<std::string>* species_name,
                       ValueRef::ValueRefBase<std::string>* ship_name) :
    m_design_name(0),
    m_design_id(ship_design_id),
    m_empire_id(empire_id),
    m_species_name(species_name),
    m_name(ship_name)
{}

CreateShip::~CreateShip() {
    delete m_design_name;
    delete m_design_id;
    delete m_empire_id;
    delete m_species_name;
    delete m_name;
}

void CreateShip::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger() << "CreateShip::Execute passed null target";
        return;
    }

    TemporaryPtr<System> system = GetSystem(context.effect_target->SystemID());
    if (!system) {
        ErrorLogger() << "CreateShip::Execute passed a target not in a system";
        return;
    }

    int design_id = ShipDesign::INVALID_DESIGN_ID;
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
    if (design_id == ShipDesign::INVALID_DESIGN_ID) {
        ErrorLogger() << "CreateShip::Execute got invalid ship design id: -1";
        return;
    }

    int empire_id = ALL_EMPIRES;
    Empire* empire(0);  // not const Empire* so that empire::NewShipName can be called
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
    //TemporaryPtr<Fleet> fleet = boost::dynamic_pointer_cast<Fleet>(target);
    //if (!fleet)
    //    if (TemporaryPtr<const Ship> ship = boost::dynamic_pointer_cast<const Ship>(target))
    //        fleet = ship->FleetID();
    //// etc.

    TemporaryPtr<Ship> ship = GetUniverse().CreateShip(empire_id, design_id, species_name, ALL_EMPIRES);
    system->Insert(ship);

    if (m_name) {
        std::string name = m_name->Eval(context);
        if (ValueRef::ConstantExpr(m_name) && UserStringExists(name))
            name = UserString(name);
        ship->Rename(name);
    } else if (ship->IsMonster()) {
        ship->Rename(NewMonsterName());
    } else if (empire) {
        ship->Rename(empire->NewShipName());
    } else {
        ship->Rename(ship->Design()->Name());
    }

    ship->ResetTargetMaxUnpairedMeters();
    ship->ResetPairedActiveMeters();

    ship->GetMeter(METER_MAX_FUEL)->SetCurrent(Meter::LARGE_VALUE);
    ship->GetMeter(METER_MAX_SHIELD)->SetCurrent(Meter::LARGE_VALUE);
    ship->GetMeter(METER_MAX_STRUCTURE)->SetCurrent(Meter::LARGE_VALUE);
    ship->GetMeter(METER_FUEL)->SetCurrent(Meter::LARGE_VALUE);
    ship->GetMeter(METER_SHIELD)->SetCurrent(Meter::LARGE_VALUE);
    ship->GetMeter(METER_STRUCTURE)->SetCurrent(Meter::LARGE_VALUE);

    ship->BackPropegateMeters();

    GetUniverse().SetEmpireKnowledgeOfShipDesign(design_id, empire_id);

    CreateNewFleet(system, ship);
}

std::string CreateShip::Description() const {
    std::string empire_str;
    if (m_empire_id) {
        if (ValueRef::ConstantExpr(m_empire_id)) {
            if (const Empire* empire = GetEmpire(m_empire_id->Eval()))
                empire_str = empire->Name();
        } else {
            empire_str = m_empire_id->Description();
        }
    }

    std::string design_str;
    if (m_design_id) {
        if (ValueRef::ConstantExpr(m_design_id)) {
            if (const ShipDesign* design = GetShipDesign(m_design_id->Eval()))
                design_str = design->Name();
        } else {
            design_str = m_design_id->Description();
        }
    } else if (m_design_name) {
        design_str = m_design_name->Description();
        if (ValueRef::ConstantExpr(m_design_name) && UserStringExists(design_str))
            design_str = UserString(design_str);
    }

    std::string species_str;
    if (m_species_name)
        species_str = ValueRef::ConstantExpr(m_species_name) ?
                      UserString(m_species_name->Eval()) :
                      m_species_name->Description();

    if (!empire_str.empty() && !species_str.empty()) {
        return str(FlexibleFormat(UserString("DESC_CREATE_SHIP"))
                   % design_str
                   % empire_str
                   % species_str);
    } else {
        return str(FlexibleFormat(UserString("DESC_CREATE_SHIP_SIMPLE"))
                   % design_str);
    }
}

std::string CreateShip::Dump() const {
    std::string retval = DumpIndent() + "CreateShip";
    if (m_design_id)
        retval += " designid = " + m_design_id->Dump();
    if (m_design_name)
        retval += " designname = " + m_design_name->Dump();
    if (m_empire_id)
        retval += " empire = " + m_empire_id->Dump();
    if (m_species_name)
        retval += " species = " + m_species_name->Dump();
    if (m_name)
        retval += " name = " + m_species_name->Dump();

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
}


///////////////////////////////////////////////////////////
// CreateField                                           //
///////////////////////////////////////////////////////////
CreateField::CreateField(ValueRef::ValueRefBase<std::string>* field_type_name,
                         ValueRef::ValueRefBase<double>* size,
                         ValueRef::ValueRefBase<std::string>* name) :
    m_field_type_name(field_type_name),
    m_x(0),
    m_y(0),
    m_size(size),
    m_name(name)
{}

CreateField::CreateField(ValueRef::ValueRefBase<std::string>* field_type_name,
                         ValueRef::ValueRefBase<double>* x,
                         ValueRef::ValueRefBase<double>* y,
                         ValueRef::ValueRefBase<double>* size,
                         ValueRef::ValueRefBase<std::string>* name) :
    m_field_type_name(field_type_name),
    m_x(x),
    m_y(y),
    m_size(size),
    m_name(name)
{}

CreateField::~CreateField() {
    delete m_field_type_name;
    delete m_x;
    delete m_y;
    delete m_size;
    delete m_name;
}

void CreateField::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger() << "CreateField::Execute passed null target";
        return;
    }
    TemporaryPtr<UniverseObject> target = context.effect_target;

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

    TemporaryPtr<Field> field = GetUniverse().CreateField(field_type->Name(), x, y, size);
    if (!field) {
        ErrorLogger() << "CreateField::Execute couldn't create field!";
        return;
    }

    // if target is a system, and location matches system location, can put
    // field into system
    TemporaryPtr<System> system = boost::dynamic_pointer_cast<System>(target);
    if (!system)
        return;
    if ((!m_y || y == system->Y()) && (!m_x || x == system->X()))
        system->Insert(field);

    if (m_name) {
        std::string name = m_name->Eval(context);
        if (ValueRef::ConstantExpr(m_name) && UserStringExists(name))
            name = UserString(name);
        field->Rename(name);
    }
}

std::string CreateField::Description() const {
    std::string size_str;
    if (m_size) {
        if (ValueRef::ConstantExpr(m_size)) {
            size_str = boost::lexical_cast<std::string>(m_size->Eval());
        } else {
            size_str = m_size->Description();
        }
    }
    std::string type_str;
    if (m_field_type_name) {
        type_str = m_field_type_name->Description();
        if (ValueRef::ConstantExpr(m_field_type_name) && UserStringExists(type_str))
            type_str = UserString(type_str);
    }

    if (!size_str.empty()) {
        return str(FlexibleFormat(UserString("DESC_CREATE_FIELD_SIZE"))
                   % type_str
                   % size_str);
    } else {
        return str(FlexibleFormat(UserString("DESC_CREATE_FIELD"))
                   % type_str);
    }
}

std::string CreateField::Dump() const {
    std::string retval = DumpIndent() + "CreateField";
    if (m_field_type_name)
        retval += " type = " + m_field_type_name->Dump();
    if (m_x)
        retval += " x = " + m_x->Dump();
    if (m_y)
        retval += " y = " + m_y->Dump();
    if (m_size)
        retval += " size = " + m_size->Dump();
    if (m_name)
        retval += " name = " + m_name->Dump();
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
}


///////////////////////////////////////////////////////////
// CreateSystem                                          //
///////////////////////////////////////////////////////////
CreateSystem::CreateSystem(ValueRef::ValueRefBase< ::StarType>* type,
                           ValueRef::ValueRefBase<double>* x,
                           ValueRef::ValueRefBase<double>* y,
                           ValueRef::ValueRefBase<std::string>* name) :
    m_type(type),
    m_x(x),
    m_y(y),
    m_name(name)
{}

CreateSystem::CreateSystem(ValueRef::ValueRefBase<double>* x,
                           ValueRef::ValueRefBase<double>* y,
                           ValueRef::ValueRefBase<std::string>* name) :
    m_type(0),
    m_x(x),
    m_y(y),
    m_name(name)
{}

CreateSystem::~CreateSystem() {
    delete m_type;
    delete m_x;
    delete m_y;
    delete m_name;
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

    std::string name;
    if (m_name) {
        name = m_name->Eval(context);
        if (ValueRef::ConstantExpr(m_name) && UserStringExists(name))
            name = UserString(name);
    } else {
        name = GenerateSystemName();
    }

    TemporaryPtr<System> system = GetUniverse().CreateSystem(star_type, name, x, y);
    if (!system) {
        ErrorLogger() << "CreateSystem::Execute couldn't create system!";
        return;
    }
}

std::string CreateSystem::Description() const {
    if (m_type) {
        std::string type_str;
        if (ValueRef::ConstantExpr(m_type)) {
            type_str = boost::lexical_cast<std::string>(m_type->Eval());
        } else {
            type_str = m_type->Description();
        }
        return str(FlexibleFormat(UserString("DESC_CREATE_SYSTEM_TYPE"))
                   % UserString(type_str));
    } else {
        return UserString("DESC_CREATE_SYSTEM");
    }
}

std::string CreateSystem::Dump() const {
    std::string retval = DumpIndent() + "CreateSystem";
    if (m_type)
        retval += " type = " + m_type->Dump();
    if (m_x)
        retval += " x = " + m_x->Dump();
    if (m_y)
        retval += " y = " + m_y->Dump();
    if (m_name)
        retval += " name = " + m_name->Dump();
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

std::string Destroy::Description() const
{ return UserString("DESC_DESTROY"); }

std::string Destroy::Dump() const
{ return DumpIndent() + "Destroy\n"; }


///////////////////////////////////////////////////////////
// AddSpecial                                            //
///////////////////////////////////////////////////////////
AddSpecial::AddSpecial(const std::string& name, float capacity) :
    m_name(new ValueRef::Constant<std::string>(name)),
    m_capacity(new ValueRef::Constant<double>(capacity))
{}

AddSpecial::AddSpecial(ValueRef::ValueRefBase<std::string>* name,
                       ValueRef::ValueRefBase<double>* capacity) :
    m_name(name),
    m_capacity(capacity)
{}

AddSpecial::~AddSpecial()
{ delete m_name; }

void AddSpecial::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger() << "AddSpecial::Execute passed no target object";
        return;
    }

    std::string name = (m_name ? m_name->Eval(context) : "");
    float capacity = (m_capacity ? m_capacity->Eval(context) : 0.0f);

    context.effect_target->AddSpecial(name, capacity);
}

std::string AddSpecial::Description() const {
    std::string name = (m_name ? m_name->Description() : "");
    std::string capacity = (m_capacity ? m_capacity->Description() : "1.0");

    return str(FlexibleFormat(UserString("DESC_ADD_SPECIAL")) % UserString(name) % capacity);
}

std::string AddSpecial::Dump() const {
    return DumpIndent() + "AddSpecial name = " +  (m_name ? m_name->Dump() : "") +
        " capacity = " + (m_capacity ? m_capacity->Dump() : "0.0") +  "\n";
}

void AddSpecial::SetTopLevelContent(const std::string& content_name) {
    if (m_name)
        m_name->SetTopLevelContent(content_name);
    if (m_capacity)
        m_capacity->SetTopLevelContent(content_name);
}


///////////////////////////////////////////////////////////
// RemoveSpecial                                         //
///////////////////////////////////////////////////////////
RemoveSpecial::RemoveSpecial(const std::string& name) :
    m_name(new ValueRef::Constant<std::string>(name))
{}

RemoveSpecial::RemoveSpecial(ValueRef::ValueRefBase<std::string>* name) :
    m_name(name)
{}

RemoveSpecial::~RemoveSpecial()
{ delete m_name; }

void RemoveSpecial::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger() << "RemoveSpecial::Execute passed no target object";
        return;
    }

    std::string name = (m_name ? m_name->Eval(context) : "");
    context.effect_target->RemoveSpecial(name);
}

std::string RemoveSpecial::Description() const {
    std::string name = (m_name ? m_name->Description() : "");
    return str(FlexibleFormat(UserString("DESC_REMOVE_SPECIAL")) % UserString(name));
}

std::string RemoveSpecial::Dump() const {
    return DumpIndent() + "RemoveSpecial name = " +  (m_name ? m_name->Dump() : "") + "\n";
}

void RemoveSpecial::SetTopLevelContent(const std::string& content_name) {
    if (m_name)
        m_name->SetTopLevelContent(content_name);
}


///////////////////////////////////////////////////////////
// AddStarlanes                                          //
///////////////////////////////////////////////////////////
AddStarlanes::AddStarlanes(Condition::ConditionBase* other_lane_endpoint_condition) :
    m_other_lane_endpoint_condition(other_lane_endpoint_condition)
{}

AddStarlanes::~AddStarlanes()
{ delete m_other_lane_endpoint_condition; }

void AddStarlanes::Execute(const ScriptingContext& context) const {
    // get target system
    if (!context.effect_target) {
        ErrorLogger() << "AddStarlanes::Execute passed no target object";
        return;
    }
    TemporaryPtr<System> target_system = boost::dynamic_pointer_cast<System>(context.effect_target);
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
    std::set<TemporaryPtr<System> > endpoint_systems;
    for (Condition::ObjectSet::const_iterator it = endpoint_objects.begin(); it != endpoint_objects.end(); ++it) {
        TemporaryPtr<const UniverseObject> endpoint_object = *it;
        TemporaryPtr<const System> endpoint_system = boost::dynamic_pointer_cast<const System>(endpoint_object);
        if (!endpoint_system)
            endpoint_system = GetSystem(endpoint_object->SystemID());
        if (!endpoint_system)
            continue;
        endpoint_systems.insert(boost::const_pointer_cast<System>(endpoint_system));
    }

    // add starlanes from target to endpoint systems
    int target_system_id = target_system->ID();
    for (std::set<TemporaryPtr<System> >::iterator it = endpoint_systems.begin(); it != endpoint_systems.end(); ++it) {
        TemporaryPtr<System> endpoint_system = *it;
        target_system->AddStarlane(endpoint_system->ID());
        endpoint_system->AddStarlane(target_system_id);
    }
}

std::string AddStarlanes::Description() const {
    std::string value_str = m_other_lane_endpoint_condition->Description();
    return str(FlexibleFormat(UserString("DESC_ADD_STARLANES")) % value_str);
}

std::string AddStarlanes::Dump() const
{ return DumpIndent() + "AddStarlanes endpoints = " + m_other_lane_endpoint_condition->Dump() + "\n"; }

void AddStarlanes::SetTopLevelContent(const std::string& content_name) {
    if (m_other_lane_endpoint_condition)
        m_other_lane_endpoint_condition->SetTopLevelContent(content_name);
}


///////////////////////////////////////////////////////////
// RemoveStarlanes                                       //
///////////////////////////////////////////////////////////
RemoveStarlanes::RemoveStarlanes(Condition::ConditionBase* other_lane_endpoint_condition) :
    m_other_lane_endpoint_condition(other_lane_endpoint_condition)
{}

RemoveStarlanes::~RemoveStarlanes()
{ delete m_other_lane_endpoint_condition; }

void RemoveStarlanes::Execute(const ScriptingContext& context) const {
    // get target system
    if (!context.effect_target) {
        ErrorLogger() << "AddStarlanes::Execute passed no target object";
        return;
    }
    TemporaryPtr<System> target_system = boost::dynamic_pointer_cast<System>(context.effect_target);
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
    std::set<TemporaryPtr<System> > endpoint_systems;
    for (Condition::ObjectSet::const_iterator it = endpoint_objects.begin(); it != endpoint_objects.end(); ++it) {
        TemporaryPtr<const UniverseObject> endpoint_object = *it;
        TemporaryPtr<const System> endpoint_system = boost::dynamic_pointer_cast<const System>(endpoint_object);
        if (!endpoint_system)
            endpoint_system = GetSystem(endpoint_object->SystemID());
        if (!endpoint_system)
            continue;
        endpoint_systems.insert(boost::const_pointer_cast<System>(endpoint_system));
    }

    // remove starlanes from target to endpoint systems
    int target_system_id = target_system->ID();
    for (std::set<TemporaryPtr<System> >::iterator it = endpoint_systems.begin(); it != endpoint_systems.end(); ++it) {
        TemporaryPtr<System> endpoint_system = *it;
        target_system->RemoveStarlane(endpoint_system->ID());
        endpoint_system->RemoveStarlane(target_system_id);
    }
}

std::string RemoveStarlanes::Description() const {
    std::string value_str = m_other_lane_endpoint_condition->Description();
    return str(FlexibleFormat(UserString("DESC_REMOVE_STARLANES")) % value_str);
}

std::string RemoveStarlanes::Dump() const
{ return DumpIndent() + "RemoveStarlanes endpoints = " + m_other_lane_endpoint_condition->Dump() + "\n"; }

void RemoveStarlanes::SetTopLevelContent(const std::string& content_name) {
    if (m_other_lane_endpoint_condition)
        m_other_lane_endpoint_condition->SetTopLevelContent(content_name);
}


///////////////////////////////////////////////////////////
// SetStarType                                           //
///////////////////////////////////////////////////////////
SetStarType::SetStarType(ValueRef::ValueRefBase<StarType>* type) :
    m_type(type)
{}

SetStarType::~SetStarType()
{ delete m_type; }

void SetStarType::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger() << "SetStarType::Execute given no target object";
        return;
    }
    if (TemporaryPtr<System> s = boost::dynamic_pointer_cast<System>(context.effect_target))
        s->SetStarType(m_type->Eval(ScriptingContext(context, s->GetStarType())));
    else
        ErrorLogger() << "SetStarType::Execute given a non-system target";
}

std::string SetStarType::Description() const {
    std::string value_str = ValueRef::ConstantExpr(m_type) ?
                                UserString(lexical_cast<std::string>(m_type->Eval())) :
                                m_type->Description();
    return str(FlexibleFormat(UserString("DESC_SET_STAR_TYPE")) % value_str);
}

std::string SetStarType::Dump() const
{ return DumpIndent() + "SetStarType type = " + m_type->Dump() + "\n"; }

void SetStarType::SetTopLevelContent(const std::string& content_name) {
    if (m_type)
        m_type->SetTopLevelContent(content_name);
}


///////////////////////////////////////////////////////////
// MoveTo                                                //
///////////////////////////////////////////////////////////
MoveTo::MoveTo(Condition::ConditionBase* location_condition) :
    m_location_condition(location_condition)
{}

MoveTo::~MoveTo()
{ delete m_location_condition; }

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
    TemporaryPtr<UniverseObject> destination = boost::const_pointer_cast<UniverseObject>(*valid_locations.begin());

    // get previous system from which to remove object if necessary
    TemporaryPtr<System> old_sys = GetSystem(context.effect_target->SystemID());

    // do the moving...
    if (TemporaryPtr<Fleet> fleet = boost::dynamic_pointer_cast<Fleet>(context.effect_target)) {
        // fleets can be inserted into the system that contains the destination
        // object (or the destination object itself if it is a system)
        if (TemporaryPtr<System> dest_system = GetSystem(destination->SystemID())) {
            if (fleet->SystemID() != dest_system->ID()) {
                // remove fleet from old system, put into new system
                if (old_sys)
                    old_sys->Remove(fleet->ID());
                dest_system->Insert(fleet);

                // also move ships of fleet
                std::vector<TemporaryPtr<Ship> > ships = Objects().FindObjects<Ship>(fleet->ShipIDs());
                for (std::vector<TemporaryPtr<Ship> >::iterator ship_it = ships.begin();
                     ship_it != ships.end(); ++ship_it)
                {
                    TemporaryPtr<Ship> ship = *ship_it;
                    if (old_sys)
                        old_sys->Remove(ship->ID());
                    dest_system->Insert(*ship_it);
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
            std::vector<TemporaryPtr<Ship> > ships = Objects().FindObjects<Ship>(fleet->ShipIDs());
            for (std::vector<TemporaryPtr<Ship> >::iterator ship_it = ships.begin();
                    ship_it != ships.end(); ++ship_it)
            {
                TemporaryPtr<Ship> ship = *ship_it;
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
            TemporaryPtr<const Fleet> dest_fleet = boost::dynamic_pointer_cast<const Fleet>(destination);
            if (!dest_fleet)
                if (TemporaryPtr<const Ship> dest_ship = boost::dynamic_pointer_cast<const Ship>(destination))
                    dest_fleet = GetFleet(dest_ship->FleetID());
            if (dest_fleet) {
                UpdateFleetRoute(fleet, dest_fleet->NextSystemID(), dest_fleet->PreviousSystemID());

            } else {
                // TODO: need to do something else to get updated previous/next
                // systems if the destination is a field.
                ErrorLogger() << "Effect::MoveTo::Execute couldn't find a way to set the previous and next systems for the target fleet!";
            }
        }

    } else if (TemporaryPtr<Ship> ship = boost::dynamic_pointer_cast<Ship>(context.effect_target)) {
        // TODO: make sure colonization doesn't interfere with this effect, and vice versa

        // is destination a ship/fleet ?
        TemporaryPtr<Fleet> dest_fleet = boost::dynamic_pointer_cast<Fleet>(destination);   // may be 0 if destination is not a fleet
        if (!dest_fleet) {
            TemporaryPtr<Ship> dest_ship = boost::dynamic_pointer_cast<Ship>(destination);  // may still be 0
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

            if (TemporaryPtr<System> new_sys = GetSystem(dest_sys_id)) {
                // ship is moving to a new system. insert it.
                new_sys->Insert(ship);
            } else {
                // ship is moving to a non-system location. move it there.
                ship->MoveTo(boost::dynamic_pointer_cast<UniverseObject>(dest_fleet));
            }

            // may create a fleet for ship below...
        }

        TemporaryPtr<Fleet> old_fleet = GetFleet(ship->FleetID());

        if (dest_fleet && same_owners) {
            // ship is moving to a different fleet owned by the same empire, so
            // can be inserted into it.
            old_fleet->RemoveShip(ship->ID());
            dest_fleet->AddShip(ship->ID());
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
            if (TemporaryPtr<System> dest_system = GetSystem(dest_sys_id)) {
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

    } else if (TemporaryPtr<Planet> planet = boost::dynamic_pointer_cast<Planet>(context.effect_target)) {
        // planets need to be located in systems, so get system that contains destination object

        TemporaryPtr<System> dest_system = GetSystem(destination->SystemID());
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
        std::vector<TemporaryPtr<Building> > buildings = Objects().FindObjects<Building>(planet->BuildingIDs());
        for (std::vector<TemporaryPtr<Building> >::iterator building_it = buildings.begin();
             building_it != buildings.end(); ++building_it)
        {
            TemporaryPtr<Building> building = *building_it;
            if (old_sys)
                old_sys->Remove(building->ID());
            dest_system->Insert(building);
        }

        // buildings planet should be unchanged by move, as should planet's
        // records of its buildings

        ExploreSystem(dest_system->ID(), planet);


    } else if (TemporaryPtr<Building> building = boost::dynamic_pointer_cast<Building>(context.effect_target)) {
        // buildings need to be located on planets, so if destination is a
        // planet, insert building into it, or attempt to get the planet on
        // which the destination object is located and insert target building
        // into that
        TemporaryPtr<Planet> dest_planet = boost::dynamic_pointer_cast<Planet>(destination);
        if (!dest_planet) {
            TemporaryPtr<Building> dest_building = boost::dynamic_pointer_cast<Building>(destination);
            if (dest_building) {
                dest_planet = GetPlanet(dest_building->PlanetID());
            }
        }
        if (!dest_planet)
            return;

        if (dest_planet->ID() == building->PlanetID())
            return; // nothing to do

        TemporaryPtr<System> dest_system = GetSystem(destination->SystemID());
        if (!dest_system)
            return;

        // remove building from old planet / system, add to new planet / system
        if (old_sys)
            old_sys->Remove(building->ID());
        building->SetSystem(INVALID_OBJECT_ID);

        if (TemporaryPtr<Planet> old_planet = GetPlanet(building->PlanetID()))
            old_planet->RemoveBuilding(building->ID());

        dest_planet->AddBuilding(building->ID());
        building->SetPlanetID(dest_planet->ID());

        dest_system->Insert(building);
        ExploreSystem(dest_system->ID(), building);


    } else if (TemporaryPtr<System> system = boost::dynamic_pointer_cast<System>(context.effect_target)) {
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
        for (ObjectMap::iterator<Fleet> it = Objects().begin<Fleet>(); it != Objects().end<Fleet>(); ++it) {
            TemporaryPtr<Fleet> obj = *it;
            if (obj->X() == system->X() && obj->Y() == system->Y())
                system->Insert(obj);
        }

        for (ObjectMap::iterator<Ship> it = Objects().begin<Ship>(); it != Objects().end<Ship>(); ++it) {
            TemporaryPtr<Ship> obj = *it;
            if (obj->X() == system->X() && obj->Y() == system->Y())
                system->Insert(obj);
        }


    } else if (TemporaryPtr<Field> field = boost::dynamic_pointer_cast<Field>(context.effect_target)) {
        if (old_sys)
            old_sys->Remove(field->ID());
        field->SetSystem(INVALID_OBJECT_ID);
        field->MoveTo(destination);
        if (TemporaryPtr<System> dest_system = boost::dynamic_pointer_cast<System>(destination))
            dest_system->Insert(field);
    }
}

std::string MoveTo::Description() const {
    std::string value_str = m_location_condition->Description();
    return str(FlexibleFormat(UserString("DESC_MOVE_TO")) % value_str);
}

std::string MoveTo::Dump() const
{ return DumpIndent() + "MoveTo destination = " + m_location_condition->Dump() + "\n"; }

void MoveTo::SetTopLevelContent(const std::string& content_name) {
    if (m_location_condition)
        m_location_condition->SetTopLevelContent(content_name);
}


///////////////////////////////////////////////////////////
// MoveInOrbit                                           //
///////////////////////////////////////////////////////////
MoveInOrbit::MoveInOrbit(ValueRef::ValueRefBase<double>* speed,
                         Condition::ConditionBase* focal_point_condition) :
    m_speed(speed),
    m_focal_point_condition(focal_point_condition),
    m_focus_x(0),
    m_focus_y(0)
{}

MoveInOrbit::MoveInOrbit(ValueRef::ValueRefBase<double>* speed,
                         ValueRef::ValueRefBase<double>* focus_x/* = 0*/,
                         ValueRef::ValueRefBase<double>* focus_y/* = 0*/) :
    m_speed(speed),
    m_focal_point_condition(0),
    m_focus_x(focus_x),
    m_focus_y(focus_y)
{}

MoveInOrbit::~MoveInOrbit() {
    delete m_speed;
    delete m_focal_point_condition;
    delete m_focus_x;
    delete m_focus_y;
}

void MoveInOrbit::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger() << "MoveInOrbit::Execute given no target object";
        return;
    }
    TemporaryPtr<UniverseObject> target = context.effect_target;

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
        TemporaryPtr<const UniverseObject> focus_object = *matches.begin();
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

    TemporaryPtr<System> old_sys = GetSystem(target->SystemID());

    if (TemporaryPtr<System> system = boost::dynamic_pointer_cast<System>(target)) {
        system->MoveTo(new_x, new_y);
        return;

    } else if (TemporaryPtr<Fleet> fleet = boost::dynamic_pointer_cast<Fleet>(target)) {
        if (old_sys)
            old_sys->Remove(fleet->ID());
        fleet->SetSystem(INVALID_OBJECT_ID);
        fleet->MoveTo(new_x, new_y);
        UpdateFleetRoute(fleet, INVALID_OBJECT_ID, INVALID_OBJECT_ID);

        std::vector<TemporaryPtr<Ship> > ships = Objects().FindObjects<Ship>(fleet->ShipIDs());
        for (std::vector<TemporaryPtr<Ship> >::iterator ship_it = ships.begin();
             ship_it != ships.end(); ++ship_it)
        {
            TemporaryPtr<Ship> ship = *ship_it;
            if (old_sys)
                old_sys->Remove(ship->ID());
            ship->SetSystem(INVALID_OBJECT_ID);
            ship->MoveTo(new_x, new_y);
        }
        return;

    } else if (TemporaryPtr<Ship> ship = boost::dynamic_pointer_cast<Ship>(target)) {
        if (old_sys)
            old_sys->Remove(ship->ID());
        ship->SetSystem(INVALID_OBJECT_ID);

        TemporaryPtr<Fleet> old_fleet = GetFleet(ship->FleetID());
        if (old_fleet) {
            old_fleet->RemoveShip(ship->ID());
            if (old_fleet->Empty()) {
                old_sys->Remove(old_fleet->ID());
                GetUniverse().EffectDestroy(old_fleet->ID(), INVALID_OBJECT_ID);    // no object in particular destroyed this fleet
            }
        }

        ship->SetFleetID(INVALID_OBJECT_ID);
        ship->MoveTo(new_x, new_y);

        CreateNewFleet(new_x, new_y, ship); // creates new fleet and inserts ship into fleet
        return;

    } else if (TemporaryPtr<Field> field = boost::dynamic_pointer_cast<Field>(target)) {
        if (old_sys)
            old_sys->Remove(field->ID());
        field->SetSystem(INVALID_OBJECT_ID);
        field->MoveTo(new_x, new_y);
    }
    // don't move planets or buildings, as these can't exist outside of systems
}

std::string MoveInOrbit::Description() const {
    std::string focus_str;
    if (m_focal_point_condition)
        focus_str = m_focal_point_condition->Description();

    std::string speed_str;
    if (m_speed)
        speed_str = m_speed->Description();

    if (!focus_str.empty())
        return str(FlexibleFormat(UserString("DESC_MOVE_IN_ORBIT_OF_OBJECT"))
                   % focus_str
                   % speed_str);

    std::string x_str = "0.0";
    if (m_focus_x)
        x_str = m_focus_x->Description();

    std::string y_str = "0.0";
    if (m_focus_y)
        y_str = m_focus_y->Description();

    return str(FlexibleFormat(UserString("DESC_MOVE_IN_ORBIT_OF_XY"))
               % x_str
               % y_str
               % speed_str);
}

std::string MoveInOrbit::Dump() const {
    if (m_focal_point_condition)
        return DumpIndent() + "MoveInOrbit around = " + m_focal_point_condition->Dump() + "\n";
    else if (m_focus_x && m_focus_y)
        return DumpIndent() + "MoveInOrbit x = " + m_focus_x->Dump() + " y = " + m_focus_y->Dump() + "\n";
    else
        return DumpIndent() + "MoveInOrbit";
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


///////////////////////////////////////////////////////////
// MoveTowards                                           //
///////////////////////////////////////////////////////////
MoveTowards::MoveTowards(ValueRef::ValueRefBase<double>* speed,
                         Condition::ConditionBase* dest_condition) :
    m_speed(speed),
    m_dest_condition(dest_condition),
    m_dest_x(0),
    m_dest_y(0)
{}

MoveTowards::MoveTowards(ValueRef::ValueRefBase<double>* speed,
                         ValueRef::ValueRefBase<double>* dest_x/* = 0*/,
                         ValueRef::ValueRefBase<double>* dest_y/* = 0*/) :
    m_speed(speed),
    m_dest_condition(0),
    m_dest_x(dest_x),
    m_dest_y(dest_y)
{}

MoveTowards::~MoveTowards() {
    delete m_speed;
    delete m_dest_condition;
    delete m_dest_x;
    delete m_dest_y;
}

void MoveTowards::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger() << "MoveTowards::Execute given no target object";
        return;
    }
    TemporaryPtr<UniverseObject> target = context.effect_target;

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
        TemporaryPtr<const UniverseObject> focus_object = *matches.begin();
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

    if (TemporaryPtr<System> system = boost::dynamic_pointer_cast<System>(target)) {
        system->MoveTo(new_x, new_y);
        std::vector<TemporaryPtr<UniverseObject> > contained_objects =
            Objects().FindObjects<UniverseObject>(system->ObjectIDs());
        for (std::vector<TemporaryPtr<UniverseObject> >::iterator it = contained_objects.begin();
             it != contained_objects.end(); ++it)
        {
            TemporaryPtr<UniverseObject> obj = *it;
            obj->MoveTo(new_x, new_y);
        }
        // don't need to remove objects from system or insert into it, as all
        // contained objects in system are moved with it, maintaining their
        // containment situation

    } else if (TemporaryPtr<Fleet> fleet = boost::dynamic_pointer_cast<Fleet>(target)) {
        TemporaryPtr<System> old_sys = GetSystem(fleet->SystemID());
        if (old_sys)
            old_sys->Remove(fleet->ID());
        fleet->SetSystem(INVALID_OBJECT_ID);
        fleet->MoveTo(new_x, new_y);
        std::vector<TemporaryPtr<Ship> > ships = Objects().FindObjects<Ship>(fleet->ShipIDs());
        for (std::vector<TemporaryPtr<Ship> >::iterator it = ships.begin();
             it != ships.end(); ++it)
        {
            TemporaryPtr<Ship> ship = *it;
            if (old_sys)
                old_sys->Remove(ship->ID());
            ship->SetSystem(INVALID_OBJECT_ID);
            ship->MoveTo(new_x, new_y);
        }

        // todo: is fleet now close enough to fall into a system?
        UpdateFleetRoute(fleet, INVALID_OBJECT_ID, INVALID_OBJECT_ID);

    } else if (TemporaryPtr<Ship> ship = boost::dynamic_pointer_cast<Ship>(target)) {
        TemporaryPtr<System> old_sys = GetSystem(ship->SystemID());
        if (old_sys)
            old_sys->Remove(ship->ID());
        ship->SetSystem(INVALID_OBJECT_ID);

        TemporaryPtr<Fleet> old_fleet = GetFleet(ship->FleetID());
        if (old_fleet)
            old_fleet->RemoveShip(ship->ID());
        ship->SetFleetID(INVALID_OBJECT_ID);

        CreateNewFleet(new_x, new_y, ship); // creates new fleet and inserts ship into fleet
        if (old_fleet && old_fleet->Empty()) {
            if (old_sys)
                old_sys->Remove(old_fleet->ID());
            GetUniverse().EffectDestroy(old_fleet->ID(), INVALID_OBJECT_ID);    // no object in particular destroyed this fleet
        }

    } else if (TemporaryPtr<Field> field = boost::dynamic_pointer_cast<Field>(target)) {
        TemporaryPtr<System> old_sys = GetSystem(field->SystemID());
        if (old_sys)
            old_sys->Remove(field->ID());
        field->SetSystem(INVALID_OBJECT_ID);
        field->MoveTo(new_x, new_y);

    }
    // don't move planets or buildings, as these can't exist outside of systems
}

std::string MoveTowards::Description() const {
    std::string dest_str;
    if (m_dest_condition)
        dest_str = m_dest_condition->Description();

    std::string speed_str;
    if (m_speed)
        speed_str = m_speed->Description();

    if (!dest_str.empty())
        return str(FlexibleFormat(UserString("DESC_MOVE_TOWARDS_OBJECT"))
                   % dest_str
                   % speed_str);

    std::string x_str = "0.0";
    if (m_dest_x)
        x_str = m_dest_x->Description();

    std::string y_str = "0.0";
    if (m_dest_y)
        y_str = m_dest_y->Description();

    return str(FlexibleFormat(UserString("DESC_MOVE_TOWARDS_XY"))
               % x_str
               % y_str
               % speed_str);
}

std::string MoveTowards::Dump() const {
    if (m_dest_condition)
        return DumpIndent() + "MoveTowards destination = " + m_dest_condition->Dump() + "\n";
    else if (m_dest_x && m_dest_y)
        return DumpIndent() + "MoveTowards x = " + m_dest_x->Dump() + " y = " + m_dest_y->Dump() + "\n";
    else
        return DumpIndent() + "MoveTowards";
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


///////////////////////////////////////////////////////////
// SetDestination                                        //
///////////////////////////////////////////////////////////
SetDestination::SetDestination(Condition::ConditionBase* location_condition) :
    m_location_condition(location_condition)
{}

SetDestination::~SetDestination()
{ delete m_location_condition; }

void SetDestination::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        ErrorLogger() << "SetDestination::Execute given no target object";
        return;
    }

    TemporaryPtr<Fleet> target_fleet = boost::dynamic_pointer_cast<Fleet>(context.effect_target);
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
    Condition::ObjectSet::iterator obj_it = valid_locations.begin();
    std::advance(obj_it, destination_idx);
    TemporaryPtr<UniverseObject> destination = boost::const_pointer_cast<UniverseObject>(*obj_it);
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
    std::pair<std::list<int>, double> short_path = universe.ShortestPath(start_system_id, destination_system_id, target_fleet->Owner());
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

std::string SetDestination::Description() const {
    std::string value_str = m_location_condition->Description();
    return str(FlexibleFormat(UserString("DESC_SET_DESTINATION")) % value_str);
}

std::string SetDestination::Dump() const
{ return DumpIndent() + "SetDestination destination = " + m_location_condition->Dump() + "\n"; }

void SetDestination::SetTopLevelContent(const std::string& content_name) {
    if (m_location_condition)
        m_location_condition->SetTopLevelContent(content_name);
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

    TemporaryPtr<Fleet> target_fleet = boost::dynamic_pointer_cast<Fleet>(context.effect_target);
    if (!target_fleet) {
        ErrorLogger() << "SetAggression::Execute acting on non-fleet target:";
        context.effect_target->Dump();
        return;
    }

    target_fleet->SetAggressive(m_aggressive);
}

std::string SetAggression::Description() const
{ return (m_aggressive ? UserString("DESC_SET_AGGRESSIVE") : UserString("DESC_SET_PASSIVE")); }

std::string SetAggression::Dump() const
{ return DumpIndent() + (m_aggressive ? "SetAggressive" : "SetPassive") + "\n"; }


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
    GetUniverse().EffectVictory(context.effect_target->ID(), m_reason_string);
}

std::string Victory::Description() const
{ return UserString("DESC_VICTORY"); }

std::string Victory::Dump() const
{ return DumpIndent() + "Victory reason = \"" + m_reason_string + "\"\n"; }


///////////////////////////////////////////////////////////
// SetEmpireTechProgress                                 //
///////////////////////////////////////////////////////////
SetEmpireTechProgress::SetEmpireTechProgress(ValueRef::ValueRefBase<std::string>* tech_name,
                                             ValueRef::ValueRefBase<double>* research_progress) :
    m_tech_name(tech_name),
    m_research_progress(research_progress),
    m_empire_id(new ValueRef::Variable<int>(ValueRef::EFFECT_TARGET_REFERENCE, std::vector<std::string>(1, "Owner")))
{}

SetEmpireTechProgress::SetEmpireTechProgress(ValueRef::ValueRefBase<std::string>* tech_name,
                                             ValueRef::ValueRefBase<double>* research_progress,
                                             ValueRef::ValueRefBase<int>* empire_id) :
    m_tech_name(tech_name),
    m_research_progress(research_progress),
    m_empire_id(empire_id)
{}

SetEmpireTechProgress::~SetEmpireTechProgress() {
    delete m_tech_name;
    delete m_research_progress;
    delete m_empire_id;
}

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

std::string SetEmpireTechProgress::Description() const {
    std::string progress_str = ValueRef::ConstantExpr(m_research_progress) ?
                               lexical_cast<std::string>(m_research_progress->Eval()) :
                               m_research_progress->Description();
    std::string empire_str;
    if (m_empire_id) {
        if (ValueRef::ConstantExpr(m_empire_id)) {
            if (const Empire* empire = GetEmpire(m_empire_id->Eval()))
                empire_str = empire->Name();
        } else {
            empire_str = m_empire_id->Description();
        }
    }
    std::string tech_name;
    if (m_tech_name) {
        if (ValueRef::ConstantExpr(m_tech_name)) {
            tech_name = m_tech_name->Eval();
        } else {
            tech_name = m_tech_name->Description();
        }
        if (GetTech(tech_name)) {
            std::string name_temp = tech_name;
            tech_name = UserString(name_temp);
        }
    }

    return str(FlexibleFormat(UserString("DESC_SET_EMPIRE_TECH_PROGRESS"))
               % tech_name
               % progress_str
               % empire_str);
}

std::string SetEmpireTechProgress::Dump() const {
    std::string retval = "SetEmpireTechProgress name = ";
    if (m_tech_name)
        retval += m_tech_name->Dump();
    if (m_research_progress)
        retval += " progress = " + m_research_progress->Dump();
    if (m_empire_id)
        retval += " empire = " + m_empire_id->Dump() + "\n";
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


///////////////////////////////////////////////////////////
// GiveEmpireTech                                        //
///////////////////////////////////////////////////////////
GiveEmpireTech::GiveEmpireTech(ValueRef::ValueRefBase<std::string>* tech_name,
                               ValueRef::ValueRefBase<int>* empire_id) :
    m_tech_name(tech_name),
    m_empire_id(empire_id)
{
    if (!m_empire_id)
        m_empire_id = new ValueRef::Variable<int>(ValueRef::EFFECT_TARGET_REFERENCE, std::vector<std::string>(1, "Owner"));
}

GiveEmpireTech::~GiveEmpireTech() {
    delete m_empire_id;
    delete m_tech_name;
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

std::string GiveEmpireTech::Description() const {
    std::string empire_str;
    if (m_empire_id) {
        if (ValueRef::ConstantExpr(m_empire_id)) {
            if (const Empire* empire = GetEmpire(m_empire_id->Eval()))
                empire_str = empire->Name();
        } else {
            empire_str = m_empire_id->Description();
        }
    }

    std::string tech_str;
    if (m_tech_name) {
        tech_str = m_tech_name->Description();
        if (ValueRef::ConstantExpr(m_tech_name) && UserStringExists(tech_str))
            tech_str = UserString(tech_str);
    }

    return str(FlexibleFormat(UserString("DESC_GIVE_EMPIRE_TECH"))
                % tech_str
                % empire_str);
}

std::string GiveEmpireTech::Dump() const {
    std::string retval = DumpIndent() + "GiveEmpireTech";

    if (m_tech_name)
        retval += " name = " + m_tech_name->Dump();

    if (m_empire_id)
        retval += " empire = " + m_empire_id->Dump();

    retval += "\n";
    return retval;
}

void GiveEmpireTech::SetTopLevelContent(const std::string& content_name) {
    if (m_empire_id)
        m_empire_id->SetTopLevelContent(content_name);
    if (m_tech_name)
        m_tech_name->SetTopLevelContent(content_name);
}


///////////////////////////////////////////////////////////
// GenerateSitRepMessage                                 //
///////////////////////////////////////////////////////////
GenerateSitRepMessage::GenerateSitRepMessage(const std::string& message_string,
                                             const std::string& icon,
                                             const std::vector<std::pair<std::string, ValueRef::ValueRefBase<std::string>*> >& message_parameters,
                                             ValueRef::ValueRefBase<int>* recipient_empire_id,
                                             EmpireAffiliationType affiliation) :
    m_message_string(message_string),
    m_icon(icon),
    m_message_parameters(message_parameters),
    m_recipient_empire_id(recipient_empire_id),
    m_condition(0),
    m_affiliation(affiliation)
{}

GenerateSitRepMessage::GenerateSitRepMessage(const std::string& message_string,
                                             const std::string& icon,
                                             const std::vector<std::pair<std::string, ValueRef::ValueRefBase<std::string>*> >& message_parameters,
                                             EmpireAffiliationType affiliation,
                                             Condition::ConditionBase* condition) :
    m_message_string(message_string),
    m_icon(icon),
    m_message_parameters(message_parameters),
    m_recipient_empire_id(0),
    m_condition(condition),
    m_affiliation(affiliation)
{}

GenerateSitRepMessage::~GenerateSitRepMessage() {
    for (std::vector<std::pair<std::string, ValueRef::ValueRefBase<std::string>*> >::iterator it =
         m_message_parameters.begin(); it != m_message_parameters.end(); ++it)
    {
        delete it->second;
    }
    delete m_recipient_empire_id;
}

void GenerateSitRepMessage::Execute(const ScriptingContext& context) const {
    int recipient_id = ALL_EMPIRES;
    if (m_recipient_empire_id)
        recipient_id = m_recipient_empire_id->Eval(context);

    // track any ship designs used in message, which any recipients must be
    // made aware of so sitrep won't have errors
    std::set<int> ship_design_ids_to_inform_receipits_of;

    // TODO: should any referenced object IDs being made known at basic visibility?


    // evaluate all parameter valuerefs so they can be substituted into sitrep template
    std::vector<std::pair<std::string, std::string> > parameter_tag_values;
    for (std::vector<std::pair<std::string, ValueRef::ValueRefBase<std::string>*> >::const_iterator it =
         m_message_parameters.begin(); it != m_message_parameters.end(); ++it)
    {
        parameter_tag_values.push_back(std::make_pair(it->first, it->second->Eval(context)));

        // special case for ship designs: make sure sitrep recipient knows about the design
        // so the sitrep won't have errors about unknown designs being referenced
        if (it->first == VarText::PREDEFINED_DESIGN_TAG) {
            if (const ShipDesign* design = GetPredefinedShipDesign(it->second->Eval(context))) {
                int design_id = design->ID();
                ship_design_ids_to_inform_receipits_of.insert(design_id);
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
        for (EmpireManager::const_iterator emp_it = Empires().begin();
             emp_it != Empires().end(); ++emp_it)
        {
            if (emp_it->first == recipient_id || recipient_id == ALL_EMPIRES)
                continue;

            DiplomaticStatus status = Empires().GetDiplomaticStatus(recipient_id, emp_it->first);
            if (status == DIPLO_PEACE)
                recipient_empire_ids.insert(emp_it->first);
        }
        break;
    }

    case AFFIL_ENEMY: {
        // add enemies of specified empire
        for (EmpireManager::const_iterator emp_it = Empires().begin();
             emp_it != Empires().end(); ++emp_it)
        {
            if (emp_it->first == recipient_id || recipient_id == ALL_EMPIRES)
                continue;

            DiplomaticStatus status = Empires().GetDiplomaticStatus(recipient_id, emp_it->first);
            if (status == DIPLO_WAR)
                recipient_empire_ids.insert(emp_it->first);
        }
        break;
    }

    case AFFIL_CAN_SEE: {
        // evaluate condition
        Condition::ObjectSet condition_matches;
        if (m_condition)
            m_condition->Eval(context, condition_matches);

        // add empires that can see any condition-matching object
        for (EmpireManager::iterator empire_it = Empires().begin();
             empire_it != Empires().end(); ++empire_it)
        {
            int empire_id = empire_it->first;
            for (Condition::ObjectSet::iterator obj_it = condition_matches.begin();
                 obj_it != condition_matches.end(); ++obj_it)
            {
                if ((*obj_it)->GetVisibility(empire_id) >= VIS_BASIC_VISIBILITY) {
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

    case AFFIL_ANY:
    default: {
        // add all empires
        for (EmpireManager::const_iterator empire_it = Empires().begin(); empire_it != Empires().end(); ++empire_it)
            recipient_empire_ids.insert(empire_it->first);
        break;
    }
    }


    // send to recipient empires
    for (std::set<int>::const_iterator emp_it = recipient_empire_ids.begin();
         emp_it != recipient_empire_ids.end(); ++emp_it)
    {
        Empire* empire = GetEmpire(*emp_it);
        if (!empire)
            continue;

        empire->AddSitRepEntry(CreateSitRep(m_message_string, m_icon, parameter_tag_values));

        // also inform of any ship designs recipients should know about
        for (std::set<int>::const_iterator design_it = ship_design_ids_to_inform_receipits_of.begin();
             design_it != ship_design_ids_to_inform_receipits_of.end(); ++design_it)
        { GetUniverse().SetEmpireKnowledgeOfShipDesign(*design_it, *emp_it); }
    }
}

std::string GenerateSitRepMessage::Description() const {
    std::string empire_str;
    if (m_recipient_empire_id) {
        int empire_id = ALL_EMPIRES;
        if (ValueRef::ConstantExpr(m_recipient_empire_id))
            empire_id = m_recipient_empire_id->Eval();
        if (const Empire* empire = GetEmpire(empire_id))
            empire_str = empire->Name();
        else
            empire_str = m_recipient_empire_id->Description();
    }

    std::string condition_str;
    if (m_condition)
        condition_str = m_condition->Description();

    // pick appropriate sitrep text...
    std::string desc_template;
    switch (m_affiliation) {
    case AFFIL_ALLY:    desc_template = UserString("DESC_GENERATE_SITREP_ALLIES");  break;
    case AFFIL_ENEMY:   desc_template = UserString("DESC_GENERATE_SITREP_ENEMIES"); break;
    case AFFIL_CAN_SEE: desc_template = UserString("DESC_GENERATE_SITREP_CAN_SEE"); break;
    case AFFIL_NONE:    desc_template = UserString("DESC_GENERATE_SITREP_NONE");    break;
    case AFFIL_ANY:     desc_template = UserString("DESC_GENERATE_SITREP_ALL");     break;
    case AFFIL_SELF:
    default:
        desc_template = UserString("DESC_GENERATE_SITREP");
    }

    return str(FlexibleFormat(desc_template) % empire_str % condition_str);
}

std::string GenerateSitRepMessage::Dump() const {
    std::string retval = DumpIndent();
    retval += "GenerateSitRepMessage\n";
    ++g_indent;
    retval += DumpIndent() + "message = \"" + m_message_string + "\"" + " icon = " + m_icon + "\n";

    if (m_message_parameters.size() == 1) {
        retval += DumpIndent() + "parameters = tag = " + m_message_parameters[0].first + " data = " + m_message_parameters[0].second->Dump() + "\n";
    } else if (!m_message_parameters.empty()) {
        retval += DumpIndent() + "parameters = [ ";
        for (unsigned int i = 0; i < m_message_parameters.size(); ++i) {
            retval += " tag = " + m_message_parameters[i].first
                   + " data = " + m_message_parameters[i].second->Dump()
                   + " ";
        }
        retval += "]\n";
    }

    retval += DumpIndent() + "affiliation = ";
    switch (m_affiliation) {
    case AFFIL_SELF:    retval += "TheEmpire";  break;
    case AFFIL_ENEMY:   retval += "EnemyOf";    break;
    case AFFIL_ALLY:    retval += "AllyOf";     break;
    case AFFIL_ANY:     retval += "AnyEmpire";  break;
    case AFFIL_CAN_SEE: retval += "CanSee";     break;
    default:            retval += "?";          break;
    }

    if (m_recipient_empire_id)
        retval += "\n" + DumpIndent() + "empire = " + m_recipient_empire_id->Dump() + "\n";
    if (m_condition)
        retval += "\n" + DumpIndent() + "condition = " + m_condition->Dump() + "\n";
    --g_indent;

    return retval;
}

void GenerateSitRepMessage::SetTopLevelContent(const std::string& content_name) {
    for (std::vector<std::pair<std::string, ValueRef::ValueRefBase<std::string>*> >::iterator it = m_message_parameters.begin();
         it != m_message_parameters.end(); ++it)
    { it->second->SetTopLevelContent(content_name); }
    if (m_recipient_empire_id)
        m_recipient_empire_id->SetTopLevelContent(content_name);
    if (m_condition)
        m_condition->SetTopLevelContent(content_name);
}


///////////////////////////////////////////////////////////
// SetOverlayTexture                                     //
///////////////////////////////////////////////////////////
SetOverlayTexture::SetOverlayTexture(const std::string& texture, ValueRef::ValueRefBase<double>* size) :
    m_texture(texture),
    m_size(size)
{}

SetOverlayTexture::~SetOverlayTexture()
{ delete m_size; }

void SetOverlayTexture::Execute(const ScriptingContext& context) const {
    if (!context.effect_target)
        return;
    double size = 1.0;
    if (m_size)
        size = m_size->Eval(context);

    if (TemporaryPtr<System> system = boost::dynamic_pointer_cast<System>(context.effect_target))
        system->SetOverlayTexture(m_texture, size);
}

std::string SetOverlayTexture::Description() const
{ return UserString("DESC_SET_OVERLAY_TEXTURE"); }

std::string SetOverlayTexture::Dump() const {
    std::string retval = DumpIndent() + "SetOverlayTexture texture = " + m_texture;
    if (m_size)
        retval += " size = " + m_size->Dump();
    retval += "\n";
    return retval;
}

void SetOverlayTexture::SetTopLevelContent(const std::string& content_name) {
    if (m_size)
        m_size->SetTopLevelContent(content_name);
}


///////////////////////////////////////////////////////////
// SetTexture                                 //
///////////////////////////////////////////////////////////
SetTexture::SetTexture(const std::string& texture) :
    m_texture(texture)
{}

void SetTexture::Execute(const ScriptingContext& context) const {
    if (!context.effect_target)
        return;
    if (TemporaryPtr<Planet> planet = boost::dynamic_pointer_cast<Planet>(context.effect_target))
        planet->SetSurfaceTexture(m_texture);
}

std::string SetTexture::Description() const
{ return UserString("DESC_SET_TEXTURE"); }

std::string SetTexture::Dump() const
{ return DumpIndent() + "SetTexture texture = " + m_texture + "\n"; }
