#include "Effect.h"

#include "../util/Logger.h"
#include "../util/Random.h"
#include "../util/Directories.h"
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
    SimpleMeterModification(MeterType meter, const ValueRef::ValueRefBase<double>* ref) {
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

        return fleet;
    }

    /** Creates a new fleet at \a system and inserts \a ship into it.  Used
     * when a ship has been moved by the MoveTo effect separately from the
     * fleet that previously held it.  Also used by CreateShip effect to give
     * the new ship a fleet.  All ships need to be within fleets. */
    TemporaryPtr<Fleet> CreateNewFleet(TemporaryPtr<System> system, TemporaryPtr<Ship> ship) {
        if (!system || !ship)
            return TemporaryPtr<Fleet>();

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
        if (Empire* empire = Empires().Lookup(target_object->Owner()))
            empire->AddExploredSystem(system_id);
    }

    /** Resets the previous and next systems of \a fleet and recalcultes /
     * resets the fleet's move route.  Used after a fleet has been moved with
     * the MoveTo effect, as its previous route was assigned based on its
     * previous location, and may not be valid for its new location. */
    void UpdateFleetRoute(TemporaryPtr<Fleet> fleet, int new_next_system, int new_previous_system) {
        if (!fleet) {
            Logger().errorStream() << "UpdateFleetRoute passed a null fleet pointer";
            return;
        }

        TemporaryPtr<const System> next_system = GetSystem(new_next_system);
        if (!next_system) {
            Logger().errorStream() << "UpdateFleetRoute couldn't get new next system with id: " << new_next_system;
            return;
        }

        if (new_previous_system != INVALID_OBJECT_ID && !GetSystem(new_previous_system)) {
            Logger().errorStream() << "UpdateFleetRoute couldn't get new previous system with id: " << new_previous_system;
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

    bool PartMatchesEffect(const PartType& part,
                           ShipPartClass part_class,
                           CombatFighterType fighter_type,
                           const std::string& part_name)
    {
        if (!part_name.empty())
            return part_name == part.Name();

        switch (part.Class()) {
        case PC_SHORT_RANGE:
        case PC_MISSILES:
        case PC_POINT_DEFENSE:
            return part.Class() == part_class;
        case PC_FIGHTERS:
            return boost::get<FighterStats>(part.Stats()).m_type == fighter_type;
        default:
            break;
        }

        return false;
    }

    void LoadSystemNames(std::list<std::string>& names) {
        boost::filesystem::ifstream ifs(GetResourceDir() / "starnames.txt");
        while (ifs) {
            std::string latest_name;
            std::getline(ifs, latest_name);
            if (!latest_name.empty())
                names.push_back(latest_name.substr(0, latest_name.find_last_not_of(" \t") + 1)); // strip off trailing whitespace
        }
    }

    std::string GenerateSystemName() {
        static std::list<std::string> star_names;
        if (star_names.empty())
            LoadSystemNames(star_names);

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
        Logger().errorStream() << "EffectsGroup::GetTargetSet passed invalid source object with id " << source_id;
        return;
    }
    if (!m_scope) {
        Logger().errorStream() << "EffectsGroup::GetTargetSet didn't find a valid scope condition to use...";
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

void EffectsGroup::Execute(int source_id, const TargetSet& targets) const {
    TemporaryPtr<const UniverseObject> source = GetUniverseObject(source_id);
    ScriptingContext source_context(source);
    //Logger().debugStream() << "effectsgroup source: " << source->Name() << " target " << (*it)->Name();
    for (std::vector<EffectBase*>::const_iterator effect_it = m_effects.begin();
         effect_it != m_effects.end(); ++effect_it)
    {
        (*effect_it)->Execute(source_context, targets);
    }
}

void EffectsGroup::Execute(int source_id, const TargetsAndCause& targets_and_cause, AccountingMap& accounting_map) const {
    TemporaryPtr<const UniverseObject> source = GetUniverseObject(source_id);
    const TargetSet& targets = targets_and_cause.target_set;
    ScriptingContext source_context(source);

    for (std::vector<EffectBase*>::const_iterator effect_it = m_effects.begin();
         effect_it != m_effects.end(); ++effect_it)
    {
        const EffectBase* effect = *effect_it;
        MeterType meter_type = INVALID_METER_TYPE;

        // for meter effects, need to separately call effect's Execute for each
        // target and do meter accounting before and after.
        const SetMeter* set_meter_effect = 0;
        const SetShipPartMeter* set_ship_part_meter_effect = 0;

        if (set_meter_effect = dynamic_cast<const SetMeter*>(effect)) {
            meter_type = set_meter_effect->GetMeterType();

        } else if (set_ship_part_meter_effect = dynamic_cast<const SetShipPartMeter*>(effect)) {
            meter_type = set_ship_part_meter_effect->GetMeterType();

        } else {
            // for non-meter effects, can do default batch execute
            effect->Execute(source_context, targets);
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
                TemporaryPtr<const Ship> ship = universe_object_ptr_cast<const Ship>(target);
                if (!ship)
                    continue;
                meter = ship->GetPartMeter(meter_type, set_ship_part_meter_effect->GetPartName());
            }

            if (!meter)
                continue;   // some objects might match target conditions, but not actually have the relevant meter


            // record pre-effect meter values...

            // accounting info for this effect on this meter of this target
            info.running_meter_total =  meter->Current();

            // actually execute effect to modify meter
            effect->Execute(ScriptingContext(source, target));

            // update for meter change and new total
            info.meter_change = meter->Current() - info.running_meter_total;
            info.running_meter_total = meter->Current();

            // add accounting for this effect to end of vector
            accounting_map[target->ID()][meter_type].push_back(info);
        }
    }
}

void EffectsGroup::ExecuteSetMeter(int source_id, const TargetSet& targets) const {
    TemporaryPtr<const UniverseObject> source = GetUniverseObject(source_id);

    // execute effects on targets
    for (TargetSet::const_iterator target_it = targets.begin(); target_it != targets.end(); ++target_it) {
        TemporaryPtr<UniverseObject> target = *target_it;

        //Logger().debugStream() << "effectsgroup source: " << source->Name() << " target " << (*it)->Name();
        for (std::vector<EffectBase*>::const_iterator effect_it = m_effects.begin();
             effect_it != m_effects.end(); ++effect_it)
        {
            const EffectBase* effect = *effect_it;
            MeterType meter_type = INVALID_METER_TYPE;
            const Meter* meter = 0;

            // only process SetMeter and SetShipPartMeter effects in this function

            if (const SetMeter* meter_effect = dynamic_cast<const SetMeter*>(effect)) {
                meter_type = meter_effect->GetMeterType();
                meter = target->GetMeter(meter_type);

            } else if (const SetShipPartMeter* meter_effect = dynamic_cast<const SetShipPartMeter*>(effect)) {
                meter_type = meter_effect->GetMeterType();
                if (target->ObjectType() != OBJ_SHIP)
                    continue;
                TemporaryPtr<const Ship> ship = universe_object_ptr_cast<const Ship>(target);
                if (!ship)
                    continue;
                meter = ship->GetPartMeter(meter_type, meter_effect->GetPartName());
            }

            if (!meter)
                continue;

            effect->Execute(ScriptingContext(source, target));
        }
    }
}

void EffectsGroup::ExecuteSetEmpireMeter(int source_id, const TargetSet& targets) const {
    TemporaryPtr<const UniverseObject> source = GetUniverseObject(source_id);

    // execute effects on targets
    for (TargetSet::const_iterator target_it = targets.begin(); target_it != targets.end(); ++target_it) {
        TemporaryPtr<UniverseObject> target = *target_it;

        // only process SetEmpireMeter effects in this function

        for (std::vector<EffectBase*>::const_iterator effect_it = m_effects.begin();
             effect_it != m_effects.end(); ++effect_it)
        {
            const EffectBase* effect = *effect_it;
            const SetEmpireMeter* meter_effect = dynamic_cast<const SetEmpireMeter*>(effect);
            if (!meter_effect)
                continue;
            effect->Execute(ScriptingContext(source, target));
        }
    }
}

void EffectsGroup::ExecuteSetMeter(int source_id, const TargetsAndCause& targets_and_cause,
                                   AccountingMap& accounting_map) const
{
    TemporaryPtr<const UniverseObject> source = GetUniverseObject(source_id);

    // execute effects on targets
    for (TargetSet::const_iterator target_it = targets_and_cause.target_set.begin();
         target_it != targets_and_cause.target_set.end(); ++target_it)
    {
        TemporaryPtr<UniverseObject> target = *target_it;

        for (std::vector<EffectBase*>::const_iterator effect_it = m_effects.begin();
             effect_it != m_effects.end(); ++effect_it)
        {
            const EffectBase* effect = *effect_it;
            MeterType meter_type = INVALID_METER_TYPE;
            const Meter* meter = 0;

            // only process SetMeter and SetShipPartMeter effects in this function

            if (const SetMeter* meter_effect = dynamic_cast<const SetMeter*>(effect)) {
                meter_type = meter_effect->GetMeterType();
                meter = target->GetMeter(meter_type);

            } else if (const SetShipPartMeter* meter_effect = dynamic_cast<const SetShipPartMeter*>(effect)) {
                meter_type = meter_effect->GetMeterType();
                if (target->ObjectType() != OBJ_SHIP)
                    continue;
                TemporaryPtr<const Ship> ship = universe_object_ptr_cast<const Ship>(target);
                if (!ship)
                    continue;

                const std::string& part_name = meter_effect->GetPartName();
                meter = ship->GetPartMeter(meter_type, part_name);
            }

            if (!meter)
                continue;

            // record pre-effect meter values in accounting info for this effect on this meter
            Effect::AccountingInfo info;
            info.cause_type =           targets_and_cause.effect_cause.cause_type;
            info.specific_cause =       targets_and_cause.effect_cause.specific_cause;
            info.custom_label =         targets_and_cause.effect_cause.custom_label;
            info.source_id =            source_id;
            info.running_meter_total =  meter->Current();

            // actually execute effect to modify meter
            effect->Execute(ScriptingContext(source, target));

            // update for meter change and new total
            info.meter_change = meter->Current() - info.running_meter_total;
            info.running_meter_total = meter->Current();

            // add accounting for this effect to end of vector
            accounting_map[target->ID()][meter_type].push_back(info);
        }
    }
}

void EffectsGroup::ExecuteAppearanceModifications(int source_id, const TargetSet& targets) const {
    TemporaryPtr<const UniverseObject> source = GetUniverseObject(source_id);

    // execute effects on targets
    for (TargetSet::const_iterator target_it = targets.begin(); target_it != targets.end(); ++target_it) {
        TemporaryPtr<UniverseObject> target = *target_it;

        for (std::vector<EffectBase*>::const_iterator effect_it = m_effects.begin();
             effect_it != m_effects.end(); ++effect_it)
        {
            const EffectBase* effect = *effect_it;
            if (!dynamic_cast<const SetTexture*>(effect) && !dynamic_cast<const SetOverlayTexture*>(effect))
                continue;
            effect->Execute(ScriptingContext(source, target));
        }
    }

}

EffectsGroup::Description EffectsGroup::GetDescription() const {
    Description retval;
    if (dynamic_cast<const Condition::Source*>(m_scope))
        retval.scope_description = UserString("DESC_EFFECTS_GROUP_SELF_SCOPE");
    else
        retval.scope_description = str(FlexibleFormat(UserString("DESC_EFFECTS_GROUP_SCOPE")) % m_scope->Description());

    if (!m_activation || dynamic_cast<const Condition::Source*>(m_activation) || dynamic_cast<const Condition::All*>(m_activation))
        retval.activation_description = UserString("DESC_EFFECTS_GROUP_ALWAYS_ACTIVE");
    else
        retval.activation_description = str(FlexibleFormat(UserString("DESC_EFFECTS_GROUP_ACTIVATION")) % m_activation->Description());

    for (unsigned int i = 0; i < m_effects.size(); ++i) {
        retval.effect_descriptions.push_back(m_effects[i]->Description());
    }
    return retval;
}

std::string EffectsGroup::DescriptionString() const {
    if (!m_explicit_description.empty()) {
        return UserString(m_explicit_description);
    } else {
        std::stringstream retval;
        Description description = GetDescription();
        retval << str(FlexibleFormat(UserString("DESC_EFFECTS_GROUP_SCOPE_DESC")) % description.scope_description);

        if (m_activation && !dynamic_cast<const Condition::Source*>(m_activation) && !dynamic_cast<const Condition::All*>(m_activation))
            retval << str(FlexibleFormat(UserString("DESC_EFFECTS_GROUP_ACTIVATION_DESC")) % description.activation_description);

        for (unsigned int i = 0; i < description.effect_descriptions.size(); ++i) {
            retval << str(FlexibleFormat(UserString("DESC_EFFECTS_GROUP_EFFECT_DESC")) % description.effect_descriptions[i]);
        }
        return retval.str();
    }
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


///////////////////////////////////////////////////////////
// EffectsDescription function                           //
///////////////////////////////////////////////////////////
std::string EffectsDescription(const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects_groups) {
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
SetMeter::SetMeter(MeterType meter, const ValueRef::ValueRefBase<double>* value) :
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
    //Logger().debugStream() << "SetMeter::Description " << simple << " / " << op << " / " << const_operand;
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
    case METER_BATTLE_SPEED:        retval += "BattleSpeed"; break;
    case METER_STARLANE_SPEED:      retval += "StarlaneSpeed"; break;

    default:                        retval += "?"; break;
    }
    retval += " value = " + m_value->Dump() + "\n";
    return retval;
}


///////////////////////////////////////////////////////////
// SetShipPartMeter                                      //
///////////////////////////////////////////////////////////
SetShipPartMeter::SetShipPartMeter(MeterType meter,
                                   ShipPartClass part_class,
                                   const ValueRef::ValueRefBase<double>* value) :
    m_part_class(part_class),
    m_fighter_type(INVALID_COMBAT_FIGHTER_TYPE),
    m_part_name(),
    m_meter(meter),
    m_value(value)
{
    if (m_part_class == PC_FIGHTERS)
        Logger().errorStream() << "SetShipPartMeter passed ShipPartClass of PC_FIGHTERS, which is invalid";
}

SetShipPartMeter::SetShipPartMeter(MeterType meter,
                                   CombatFighterType fighter_type,
                                   const ValueRef::ValueRefBase<double>* value) :
    m_part_class(INVALID_SHIP_PART_CLASS),
    m_fighter_type(fighter_type),
    m_part_name(),
    m_meter(meter),
    m_value(value)
{}

SetShipPartMeter::SetShipPartMeter(MeterType meter,
                                   const std::string& part_name,
                                   const ValueRef::ValueRefBase<double>* value) :
    m_part_class(INVALID_SHIP_PART_CLASS),
    m_fighter_type(INVALID_COMBAT_FIGHTER_TYPE),
    m_part_name(part_name),
    m_meter(meter),
    m_value(value)
{}

SetShipPartMeter::~SetShipPartMeter()
{ delete m_value; }

void SetShipPartMeter::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        Logger().debugStream() << "SetShipPartMeter::Execute passed null target pointer";
        return;
    }

    TemporaryPtr<Ship> ship = dynamic_ptr_cast<Ship>(context.effect_target);
    if (!ship) {
        Logger().errorStream() << "SetShipPartMeter::Execute acting on non-ship target:";
        context.effect_target->Dump();
        return;
    }

    if (m_part_class == PC_FIGHTERS && !m_part_name.empty()) {
        Logger().debugStream() << "SetShipPartMeter::Execute aborting due to part class being PC_FIGHTERS and part name being not empty";
        return;
    }

    // loop through all part meters in the ship design, applying effect to each if part is appropriate
    const std::vector<std::string>& design_parts = ship->Design()->Parts();
    std::set<std::string> unique_design_parts;
    std::copy(design_parts.begin(), design_parts.end(), std::inserter(unique_design_parts, unique_design_parts.begin()));

    for (std::set<std::string>::const_iterator it = unique_design_parts.begin();
         it != unique_design_parts.end(); ++it)
    {
        const std::string& target_part_name = *it;
        if (target_part_name.empty())
            continue;   // slots in a design may be empty... this isn't an error

        Meter* meter = ship->GetPartMeter(m_meter, target_part_name);
        if (!meter)
            continue;   // some parts may not have the requested meter.  this isn't an error

        const PartType* target_part = GetPartType(target_part_name);
        if (!target_part) {
            Logger().errorStream() << "SetShipPartMeter::Execute couldn't get part type: " << target_part_name;
            continue;
        }

        // verify that found part matches the target part type information for
        // this effect: same name, same class and slot type, or same fighter type
        if (PartMatchesEffect(*target_part, m_part_class, m_fighter_type, m_part_name)) {
            double val = m_value->Eval(ScriptingContext(context, meter->Current()));
            meter->SetCurrent(val);
        }
    }
}

std::string SetShipPartMeter::Description() const {
    std::string value_str = ValueRef::ConstantExpr(m_value) ?
                                lexical_cast<std::string>(m_value->Eval()) :
                                m_value->Description();
    std::string meter_str = UserString(lexical_cast<std::string>(m_meter));

    // TODO: indicate slot restrictions in SetShipPartMeter descriptions...

    if (m_part_class != INVALID_SHIP_PART_CLASS) {
        return str(FlexibleFormat(UserString("DESC_SET_SHIP_PART_METER"))
                   % meter_str
                   % UserString(lexical_cast<std::string>(m_part_class))
                   % value_str);
    } else if (m_fighter_type != INVALID_COMBAT_FIGHTER_TYPE) {
        return str(FlexibleFormat(UserString("DESC_SET_SHIP_PART_METER"))
                   % meter_str
                   % UserString(lexical_cast<std::string>(m_fighter_type))
                   % value_str);
    } else if (!m_part_name.empty()) {
        return str(FlexibleFormat(UserString("DESC_SET_SHIP_PART_METER"))
                   % meter_str
                   % UserString(m_part_name)
                   % value_str);
    } else {
        // something weird is going on... default cause shouldn't be needed
        return str(FlexibleFormat(UserString("DESC_SET_SHIP_PART_METER"))
                   % meter_str
                   % UserString("UIT_SHIP_PART")
                   % value_str);
    }
}

std::string SetShipPartMeter::Dump() const {
    std::string retval = DumpIndent();
    switch (m_meter) {
        case METER_DAMAGE:              retval += "SetDamage";              break;
        case METER_ROF:                 retval += "SetROF";                 break;
        case METER_RANGE:               retval += "SetRange";               break;
        case METER_SPEED:               retval += "SetSpeed";               break;
        case METER_CAPACITY:            retval += "SetCapacity";            break;
        case METER_ANTI_SHIP_DAMAGE:    retval += "SetAntiShipDamage";      break;
        case METER_ANTI_FIGHTER_DAMAGE: retval += "SetAntiFighterDamage";   break;
        case METER_LAUNCH_RATE:         retval += "SetLaunchRate";          break;
        case METER_FIGHTER_WEAPON_RANGE:retval += "SetFighterWeaponRange";  break;
        case METER_STEALTH:             retval += "SetStealth";             break;
        case METER_STRUCTURE:           retval += "SetStructure";           break;
        case METER_DETECTION:           retval += "SetDetection";           break;
        default:                        retval += "Set????";                break;
    }

    if (m_part_class != INVALID_SHIP_PART_CLASS)
        retval += " partclass = " + lexical_cast<std::string>(m_part_class);
    else if (m_fighter_type != INVALID_COMBAT_FIGHTER_TYPE)
        retval += " fightertype = " + lexical_cast<std::string>(m_fighter_type);
    else if (!m_part_name.empty())
        retval += " partname = " + UserString(m_part_name);
    else
        retval += " ???";

    retval += " value = " + m_value->Dump();

    return retval;
}


///////////////////////////////////////////////////////////
// SetEmpireMeter                                        //
///////////////////////////////////////////////////////////
SetEmpireMeter::SetEmpireMeter(const std::string& meter, const ValueRef::ValueRefBase<double>* value) :
    m_empire_id(new ValueRef::Variable<int>(ValueRef::EFFECT_TARGET_REFERENCE, std::vector<std::string>(1, "Owner"))),
    m_meter(meter),
    m_value(value)
{}

SetEmpireMeter::SetEmpireMeter(const ValueRef::ValueRefBase<int>* empire_id, const std::string& meter,
                               const ValueRef::ValueRefBase<double>* value) :
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

    Empire* empire = Empires().Lookup(empire_id);
    if (!empire) {
        Logger().debugStream() << "SetEmpireMeter::Execute unable to find empire with id " << empire_id;
        return;
    }

    Meter* meter = empire->GetMeter(m_meter);
    if (!meter) {
        Logger().debugStream() << "SetEmpireMeter::Execute empire " << empire->Name() << " doesn't have a meter named " << m_meter;
        return;
    }

    double value = m_value->Eval(ScriptingContext(context, meter->Current()));

    meter->SetCurrent(value);
}

std::string SetEmpireMeter::Description() const {
    std::string empire_str;
    if (m_empire_id) {
        if (ValueRef::ConstantExpr(m_empire_id)) {
            if (const Empire* empire = Empires().Lookup(m_empire_id->Eval()))
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


///////////////////////////////////////////////////////////
// SetEmpireStockpile                                    //
///////////////////////////////////////////////////////////
SetEmpireStockpile::SetEmpireStockpile(ResourceType stockpile,
                                       const ValueRef::ValueRefBase<double>* value) :
    m_empire_id(new ValueRef::Variable<int>(ValueRef::EFFECT_TARGET_REFERENCE, std::vector<std::string>(1, "Owner"))),
    m_stockpile(stockpile),
    m_value(value)
{}

SetEmpireStockpile::SetEmpireStockpile(const ValueRef::ValueRefBase<int>* empire_id,
                                       ResourceType stockpile,
                                       const ValueRef::ValueRefBase<double>* value) :
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

    Empire* empire = Empires().Lookup(empire_id);
    if (!empire) {
        Logger().debugStream() << "SetEmpireStockpile::Execute couldn't find an empire with id " << empire_id;
        return;
    }

    double value = m_value->Eval(ScriptingContext(context, empire->ResourceStockpile(m_stockpile)));
    empire->SetResourceStockpile(m_stockpile, value);
}

std::string SetEmpireStockpile::Description() const {
    std::string empire_str;
    if (m_empire_id) {
        if (ValueRef::ConstantExpr(m_empire_id)) {
            if (const Empire* empire = Empires().Lookup(m_empire_id->Eval()))
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


///////////////////////////////////////////////////////////
// SetEmpireCapital                                      //
///////////////////////////////////////////////////////////
SetEmpireCapital::SetEmpireCapital() :
    m_empire_id(new ValueRef::Variable<int>(ValueRef::EFFECT_TARGET_REFERENCE, std::vector<std::string>(1, "Owner")))
{}

SetEmpireCapital::SetEmpireCapital(const ValueRef::ValueRefBase<int>* empire_id) :
    m_empire_id(empire_id)
{}

SetEmpireCapital::~SetEmpireCapital()
{ delete m_empire_id; }

void SetEmpireCapital::Execute(const ScriptingContext& context) const {
    int empire_id = m_empire_id->Eval(context);

    Empire* empire = Empires().Lookup(empire_id);
    if (!empire)
        return;

    TemporaryPtr<const Planet> planet = dynamic_ptr_cast<const Planet>(context.effect_target);
    if (!planet)
        return;

    empire->SetCapitalID(planet->ID());
}

std::string SetEmpireCapital::Description() const {
    std::string empire_str;
    if (m_empire_id) {
        if (ValueRef::ConstantExpr(m_empire_id)) {
            if (const Empire* empire = Empires().Lookup(m_empire_id->Eval()))
                empire_str = empire->Name();
        } else {
            empire_str = m_empire_id->Description();
        }
    }
    return str(FlexibleFormat(UserString("DESC_SET_EMPIRE_CAPITAL")) % empire_str);
}

std::string SetEmpireCapital::Dump() const
{ return DumpIndent() + "SetEmpireCapital empire = " + m_empire_id->Dump() + "\n"; }


///////////////////////////////////////////////////////////
// SetPlanetType                                         //
///////////////////////////////////////////////////////////
SetPlanetType::SetPlanetType(const ValueRef::ValueRefBase<PlanetType>* type) :
    m_type(type)
{}

SetPlanetType::~SetPlanetType()
{ delete m_type; }

void SetPlanetType::Execute(const ScriptingContext& context) const {
    if (TemporaryPtr<Planet> p = dynamic_ptr_cast<Planet>(context.effect_target)) {
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


///////////////////////////////////////////////////////////
// SetPlanetSize                                         //
///////////////////////////////////////////////////////////
SetPlanetSize::SetPlanetSize(const ValueRef::ValueRefBase<PlanetSize>* size) :
    m_size(size)
{}

SetPlanetSize::~SetPlanetSize()
{ delete m_size; }

void SetPlanetSize::Execute(const ScriptingContext& context) const {
    if (TemporaryPtr<Planet> p = dynamic_ptr_cast<Planet>(context.effect_target)) {
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


///////////////////////////////////////////////////////////
// SetSpecies                                            //
///////////////////////////////////////////////////////////
SetSpecies::SetSpecies(const ValueRef::ValueRefBase<std::string>* species) :
    m_species_name(species)
{}

SetSpecies::~SetSpecies()
{ delete m_species_name; }

void SetSpecies::Execute(const ScriptingContext& context) const {
    if (TemporaryPtr<Planet> p = dynamic_ptr_cast<Planet>(context.effect_target)) {
        std::string species_name = m_species_name->Eval(ScriptingContext(context, p->SpeciesName()));
        p->SetSpecies(species_name);
    } else if (TemporaryPtr<Ship> s = dynamic_ptr_cast<Ship>(context.effect_target)) {
        std::string species_name = m_species_name->Eval(ScriptingContext(context, s->SpeciesName()));
        s->SetSpecies(species_name);
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


///////////////////////////////////////////////////////////
// SetOwner                                              //
///////////////////////////////////////////////////////////
SetOwner::SetOwner(const ValueRef::ValueRefBase<int>* empire_id) :
    m_empire_id(empire_id)
{}

SetOwner::~SetOwner()
{ delete m_empire_id; }

void SetOwner::Execute(const ScriptingContext& context) const {
    int empire_id = m_empire_id->Eval(context);
    if (!context.effect_target)
        return;
    int initial_owner = context.effect_target->Owner();
    if (initial_owner == empire_id)
        return;

    context.effect_target->SetOwner(empire_id);

    if (TemporaryPtr<Ship> ship = dynamic_ptr_cast<Ship>(context.effect_target)) {
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
        if (new_fleet)
            new_fleet->SetNextAndPreviousSystems(fleet->NextSystemID(), fleet->PreviousSystemID());

        // if old fleet is empty, destroy it.  Don't reassign ownership of fleet
        // in case that would reval something to the recipient that shouldn't be...
        if (fleet->Empty())
            GetUniverse().EffectDestroy(fleet->ID());
    }
}

std::string SetOwner::Description() const {
    std::string empire_str;
    if (m_empire_id) {
        if (ValueRef::ConstantExpr(m_empire_id)) {
            if (const Empire* empire = Empires().Lookup(m_empire_id->Eval()))
                empire_str = empire->Name();
        } else {
            empire_str = m_empire_id->Description();
        }
    }
    return str(FlexibleFormat(UserString("DESC_SET_OWNER")) % empire_str);
}

std::string SetOwner::Dump() const
{ return DumpIndent() + "SetOwner empire = " + m_empire_id->Dump() + "\n"; }


///////////////////////////////////////////////////////////
// CreatePlanet                                          //
///////////////////////////////////////////////////////////
CreatePlanet::CreatePlanet(const ValueRef::ValueRefBase<PlanetType>* type,
                           const ValueRef::ValueRefBase<PlanetSize>* size) :
    m_type(type),
    m_size(size)
{
    Logger().debugStream() << "CreatePlanet::CreatePlanet";
    Logger().debugStream() << "    type: " << (m_type ? m_type->Dump() : "no type");
    Logger().debugStream() << "    size: " << (m_size ? m_size->Dump() : "no size");
    Logger().debugStream() << Dump();
}

CreatePlanet::~CreatePlanet() {
    Logger().debugStream() << "CreatePlanet::~CreatePlanet";
    delete m_type;
    delete m_size;
}

void CreatePlanet::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        Logger().errorStream() << "CreatePlanet::Execute passed no target object";
        return;
    }
    TemporaryPtr<System> location = GetSystem(context.effect_target->SystemID());
    if (!location) {
        Logger().errorStream() << "CreatePlanet::Execute couldn't get a System object at which to create the planet";
        return;
    }

    PlanetSize target_size = INVALID_PLANET_SIZE;
    PlanetType target_type = INVALID_PLANET_TYPE;
    if (TemporaryPtr<const Planet> location_planet = dynamic_ptr_cast<const Planet>(context.effect_target)) {
        target_size = location_planet->Size();
        target_type = location_planet->Type();
    }

    PlanetSize size = m_size->Eval(ScriptingContext(context, target_size));
    PlanetType type = m_type->Eval(ScriptingContext(context, target_type));
    if (size == INVALID_PLANET_SIZE || type == INVALID_PLANET_TYPE) {
        Logger().errorStream() << "CreatePlanet::Execute got invalid size or type of planet to create...";
        return;
    }

    //  determine if and which orbits are available
    std::set<int> free_orbits = location->FreeOrbits();
    if (free_orbits.empty()) {
        Logger().errorStream() << "CreatePlanet::Execute couldn't find any free orbits in system where planet was to be created";
        return;
    }

    TemporaryPtr<Planet> planet = GetUniverse().CreatePlanet(type, size);
    if (!planet) {
        Logger().errorStream() << "CreatePlanet::Execute unable to create new Planet object";
        return;
    }

    int orbit = *(free_orbits.begin());
    location->Insert(TemporaryPtr<UniverseObject>(planet), orbit);
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

std::string CreatePlanet::Dump() const
{
    Logger().debugStream() << "CreatePlanet::Dump()";
    return DumpIndent() + "CreatePlanet size = " + m_size->Dump() + " type = " + m_type->Dump() + "\n";
}


///////////////////////////////////////////////////////////
// CreateBuilding                                        //
///////////////////////////////////////////////////////////
CreateBuilding::CreateBuilding(const ValueRef::ValueRefBase<std::string>* building_type_name) :
    m_building_type_name(building_type_name)
{}

CreateBuilding::~CreateBuilding()
{ delete m_building_type_name; }

void CreateBuilding::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        Logger().errorStream() << "CreateBuilding::Execute passed no target object";
        return;
    }
    TemporaryPtr<Planet> location = dynamic_ptr_cast<Planet>(context.effect_target);
    if (!location)
        if (TemporaryPtr<Building> location_building = dynamic_ptr_cast<Building>(context.effect_target))
            location = GetPlanet(location_building->PlanetID());
    if (!location) {
        Logger().errorStream() << "CreateBuilding::Execute couldn't get a Planet object at which to create the building";
        return;
    }

    std::string building_type_name = m_building_type_name->Eval(context);
    const BuildingType* building_type = GetBuildingType(building_type_name);
    if (!building_type) {
        Logger().errorStream() << "CreateBuilding::Execute couldn't get building type: " << building_type_name;
        return;
    }

    TemporaryPtr<Building> building = GetUniverse().CreateBuilding(ALL_EMPIRES, building_type_name, ALL_EMPIRES);
    if (!building) {
        Logger().errorStream() << "CreateBuilding::Execute couldn't create building!";
        return;
    }

    location->AddBuilding(building->ID());

    building->SetOwner(location->Owner());
}

std::string CreateBuilding::Description() const {
    std::string type_str = ValueRef::ConstantExpr(m_building_type_name) ?
                                UserString(lexical_cast<std::string>(m_building_type_name->Eval())) :
                                m_building_type_name->Description();
    return str(FlexibleFormat(UserString("DESC_CREATE_BUILDING"))
               % type_str);
}

std::string CreateBuilding::Dump() const
{ return DumpIndent() + "CreateBuilding type = " + m_building_type_name->Dump() + "\n"; }


///////////////////////////////////////////////////////////
// CreateShip                                            //
///////////////////////////////////////////////////////////
CreateShip::CreateShip(const std::string& predefined_ship_design_name,
                       const ValueRef::ValueRefBase<int>* empire_id,
                       const ValueRef::ValueRefBase<std::string>* species_name) :
    m_design_name(predefined_ship_design_name),
    m_design_id(0), // this specifies a null pointer to a ValueRef, not the constant 0
    m_empire_id(empire_id),
    m_species_name(species_name)
{}

CreateShip::CreateShip(const ValueRef::ValueRefBase<int>* ship_design_id,
                       const ValueRef::ValueRefBase<int>* empire_id,
                       const ValueRef::ValueRefBase<std::string>* species_name) :
    m_design_name(),
    m_design_id(ship_design_id),
    m_empire_id(empire_id),
    m_species_name(species_name)
{}

CreateShip::CreateShip(const std::string& predefined_ship_design_name,
                       const ValueRef::ValueRefBase<int>* empire_id) :
    m_design_name(predefined_ship_design_name),
    m_design_id(0),     // this specifies a null pointer to a ValueRef, not the constant 0
    m_empire_id(empire_id),
    m_species_name(0)   // ...
{}

CreateShip::CreateShip(const std::string& predefined_ship_design_name) :
    m_design_name(predefined_ship_design_name),
    m_design_id(0),     // this specifies a null pointer to a ValueRef, not the constant 0
    m_empire_id(0),     // ...
    m_species_name(0)   // ...
{}

CreateShip::~CreateShip() {
    delete m_design_id;
    delete m_empire_id;
    delete m_species_name;
}

void CreateShip::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        Logger().errorStream() << "CreateShip::Execute passed null target";
        return;
    }

    TemporaryPtr<System> system = GetSystem(context.effect_target->SystemID());
    if (!system) {
        Logger().errorStream() << "CreateShip::Execute passed a target not in a system";
        return;
    }

    int design_id = ShipDesign::INVALID_DESIGN_ID;
    if (m_design_id) {
        design_id = m_design_id->Eval(context);
        if (!GetShipDesign(design_id)) {
            Logger().errorStream() << "CreateShip::Execute couldn't get ship design with id: " << design_id;
            return;
        }
    } else {
        const ShipDesign* ship_design = GetPredefinedShipDesign(m_design_name);
        if (!ship_design) {
            Logger().errorStream() << "CreateShip::Execute couldn't get predefined ship design with name " << m_design_name;
            return;
        }
        design_id = ship_design->ID();
    }
    if (design_id == ShipDesign::INVALID_DESIGN_ID) {
        Logger().errorStream() << "CreateShip::Execute got invalid ship design id: -1";
        return;
    }

    int empire_id = ALL_EMPIRES;
    Empire* empire(0);  // not const Empire* so that empire::NewShipName can be called
    if (m_empire_id) {
        empire_id = m_empire_id->Eval(context);
        if (empire_id != ALL_EMPIRES) {
            empire = Empires().Lookup(empire_id);
            if (!empire) {
                Logger().errorStream() << "CreateShip::Execute couldn't get empire with id " << empire_id;
                return;
            }
        }
    }

    std::string species_name;
    if (m_species_name) {
        species_name = m_species_name->Eval(context);
        if (!species_name.empty() && !GetSpecies(species_name)) {
            Logger().errorStream() << "CreateShip::Execute couldn't get species with which to create a ship";
            return;
        }
    }

    //// possible future modification: try to put new ship into existing fleet if
    //// ownership with target object's fleet works out (if target is a ship)
    //// attempt to find a
    //TemporaryPtr<Fleet> fleet = dynamic_ptr_cast<Fleet>(target);
    //if (!fleet)
    //    if (TemporaryPtr<const Ship> ship = dynamic_ptr_cast<const Ship>(target))
    //        fleet = ship->FleetID();
    //// etc.

    TemporaryPtr<Ship> ship = GetUniverse().CreateShip(empire_id, design_id, species_name, ALL_EMPIRES);

    ship->Rename(empire ? empire->NewShipName() : ship->Design()->Name());

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
            if (const Empire* empire = Empires().Lookup(m_empire_id->Eval()))
                empire_str = empire->Name();
        } else {
            empire_str = m_empire_id->Description();
        }
    }

    std::string design_str = UserString("ERROR");
    if (m_design_id) {
        if (ValueRef::ConstantExpr(m_design_id)) {
            if (const ShipDesign* design = GetShipDesign(m_design_id->Eval()))
                design_str = design->Name();
        } else {
            design_str = m_design_id->Description();
        }
    } else {
        design_str = UserString(m_design_name);
    }

    std::string species_str;
    if (m_species_name)
        species_str = ValueRef::ConstantExpr(m_species_name) ?
                      UserString(m_species_name->Eval()) :
                      m_species_name->Description();

    if (!empire_str.empty() && !species_str.empty())
        return str(FlexibleFormat(UserString("DESC_CREATE_SHIP"))
                   % design_str
                   % empire_str
                   % species_str);
    else
        return str(FlexibleFormat(UserString("DESC_CREATE_SHIP_SIMPLE"))
                   % design_str);
}

std::string CreateShip::Dump() const {
    std::string retval;
    if (m_design_id)
        retval = DumpIndent() + "CreateShip design_id = " + m_design_id->Dump();
    else
        retval = DumpIndent() + "CreateShip predefined_ship_design_name = \"" + m_design_name + "\"";
    if (m_empire_id)
        retval += " empire = " + m_empire_id->Dump();
    if (m_species_name)
        retval += " species_name = " + m_species_name->Dump();
    retval += "\n";
    return retval;
}


///////////////////////////////////////////////////////////
// CreateField                                           //
///////////////////////////////////////////////////////////
CreateField::CreateField(const std::string& field_type_name,
                         const ValueRef::ValueRefBase<double>* size/* = 0*/) :
    m_field_type_name(field_type_name),
    m_x(0),
    m_y(0),
    m_size(size)
{}

CreateField::CreateField(const std::string& field_type_name,
                         const ValueRef::ValueRefBase<double>* x,
                         const ValueRef::ValueRefBase<double>* y,
                         const ValueRef::ValueRefBase<double>* size/* = 0*/) :
    m_field_type_name(field_type_name),
    m_x(x),
    m_y(y),
    m_size(size)
{}

CreateField::~CreateField()
{ delete m_size; }

void CreateField::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        Logger().errorStream() << "CreateField::Execute passed null target";
        return;
    }
    TemporaryPtr<UniverseObject> target = context.effect_target;

    const FieldType* field_type = GetFieldType(m_field_type_name);
    if (!field_type) {
        Logger().errorStream() << "CreateField::Execute couldn't get field type with name: " << m_field_type_name;
        return;
    }

    double size = 10.0;
    if (m_size)
        size = m_size->Eval(context);
    if (size < 1.0) {
        Logger().errorStream() << "CreateField::Execute given very small / negative size: " << size << "  ... so resetting to 1.0";
        size = 1.0;
    }
    if (size > 10000) {
        Logger().errorStream() << "CreateField::Execute given very large size: " << size << "  ... so resetting to 10000";
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

    TemporaryPtr<Field> field = GetUniverse().CreateField(m_field_type_name, x, y, size);
    if (!field) {
        Logger().errorStream() << "CreateField::Execute couldn't create field!";
        return;
    }

    // if target is a system, and location matches system location, can put
    // field into system
    TemporaryPtr<System> system = dynamic_ptr_cast<System>(target);
    if (!system)
        return;
    if ((!m_y || y == system->Y()) && (!m_x || x == system->X()))
        system->Insert(field);
}

std::string CreateField::Description() const {
    if (m_size) {
        std::string size_str;
        if (ValueRef::ConstantExpr(m_size)) {
            size_str = boost::lexical_cast<std::string>(m_size->Eval());
        } else {
            size_str = m_size->Description();
        }
        return str(FlexibleFormat(UserString("DESC_CREATE_FIELD_SIZE"))
                   % UserString(m_field_type_name)
                   % size_str);
    } else {
        return str(FlexibleFormat(UserString("DESC_CREATE_FIELD"))
                   % UserString(m_field_type_name));
    }
}

std::string CreateField::Dump() const {
    std::string retval = DumpIndent() + "CreateField type = " + m_field_type_name;
    if (m_x)
        retval += " x = " + m_x->Dump();
    if (m_y)
        retval += " y = " + m_y->Dump();
    if (m_size)
        retval += " size = " + m_size->Dump();
    retval += "\n";
    return retval;
}


///////////////////////////////////////////////////////////
// CreateSystem                                          //
///////////////////////////////////////////////////////////
CreateSystem::CreateSystem(const ValueRef::ValueRefBase< ::StarType>* type,
                           const ValueRef::ValueRefBase<double>* x,
                           const ValueRef::ValueRefBase<double>* y) :
    m_type(type),
    m_x(x),
    m_y(y)
{}

CreateSystem::CreateSystem(const ValueRef::ValueRefBase<double>* x,
                           const ValueRef::ValueRefBase<double>* y) :
    m_type(0),
    m_x(x),
    m_y(y)
{}

CreateSystem::~CreateSystem() {
    delete m_type;
    delete m_x;
    delete m_y;
}

void CreateSystem::Execute(const ScriptingContext& context) const {
    //if (!context.effect_target) {
    //    Logger().errorStream() << "CreateSystem::Execute passed null target";
    //    return;
    //}
    //TemporaryPtr<const UniverseObject> target = context.effect_target;

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

    const int MAX_SYSTEM_ORBITS = 9;    // hard coded value in UniverseServer.cpp

    TemporaryPtr<System> system = GetUniverse().CreateSystem(star_type, MAX_SYSTEM_ORBITS, GenerateSystemName(), x, y);
    if (!system) {
        Logger().errorStream() << "CreateSystem::Execute couldn't create system!";
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
    retval += "\n";
    return retval;
}


///////////////////////////////////////////////////////////
// Destroy                                               //
///////////////////////////////////////////////////////////
Destroy::Destroy()
{}

void Destroy::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        Logger().errorStream() << "Destroy::Execute passed no target object";
        return;
    }
    GetUniverse().EffectDestroy(context.effect_target->ID());
}

std::string Destroy::Description() const
{ return UserString("DESC_DESTROY"); }

std::string Destroy::Dump() const
{ return DumpIndent() + "Destroy\n"; }


///////////////////////////////////////////////////////////
// AddSpecial                                            //
///////////////////////////////////////////////////////////
AddSpecial::AddSpecial(const std::string& name) :
    m_name(name)
{}

void AddSpecial::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        Logger().errorStream() << "AddSpecial::Execute passed no target object";
        return;
    }
    context.effect_target->AddSpecial(m_name);
}

std::string AddSpecial::Description() const
{ return str(FlexibleFormat(UserString("DESC_ADD_SPECIAL")) % UserString(m_name)); }

std::string AddSpecial::Dump() const
{ return DumpIndent() + "AddSpecial name = \"" + m_name + "\"\n"; }


///////////////////////////////////////////////////////////
// RemoveSpecial                                         //
///////////////////////////////////////////////////////////
RemoveSpecial::RemoveSpecial(const std::string& name) :
    m_name(name)
{}

void RemoveSpecial::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        Logger().errorStream() << "RemoveSpecial::Execute pass no target object.";
        return;
    }
    context.effect_target->RemoveSpecial(m_name);
}

std::string RemoveSpecial::Description() const
{ return str(FlexibleFormat(UserString("DESC_REMOVE_SPECIAL")) % UserString(m_name)); }

std::string RemoveSpecial::Dump() const
{ return DumpIndent() + "RemoveSpecial name = \"" + m_name + "\"\n"; }


///////////////////////////////////////////////////////////
// AddStarlanes                                          //
///////////////////////////////////////////////////////////
AddStarlanes::AddStarlanes(const Condition::ConditionBase* other_lane_endpoint_condition) :
    m_other_lane_endpoint_condition(other_lane_endpoint_condition)
{}

AddStarlanes::~AddStarlanes()
{ delete m_other_lane_endpoint_condition; }

void AddStarlanes::Execute(const ScriptingContext& context) const {
    // get target system
    if (!context.effect_target) {
        Logger().errorStream() << "AddStarlanes::Execute passed no target object";
        return;
    }
    TemporaryPtr<System> target_system = dynamic_ptr_cast<System>(context.effect_target);
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
        TemporaryPtr<const System> endpoint_system = dynamic_ptr_cast<const System>(endpoint_object);
        if (!endpoint_system)
            endpoint_system = GetSystem(endpoint_object->SystemID());
        if (!endpoint_system)
            continue;
        endpoint_systems.insert(const_ptr_cast<System>(endpoint_system));
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


///////////////////////////////////////////////////////////
// RemoveStarlanes                                       //
///////////////////////////////////////////////////////////
RemoveStarlanes::RemoveStarlanes(const Condition::ConditionBase* other_lane_endpoint_condition) :
    m_other_lane_endpoint_condition(other_lane_endpoint_condition)
{}

RemoveStarlanes::~RemoveStarlanes()
{ delete m_other_lane_endpoint_condition; }

void RemoveStarlanes::Execute(const ScriptingContext& context) const {
    // get target system
    if (!context.effect_target) {
        Logger().errorStream() << "AddStarlanes::Execute passed no target object";
        return;
    }
    TemporaryPtr<System> target_system = dynamic_ptr_cast<System>(context.effect_target);
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
        TemporaryPtr<const System> endpoint_system = dynamic_ptr_cast<const System>(endpoint_object);
        if (!endpoint_system)
            endpoint_system = GetSystem(endpoint_object->SystemID());
        if (!endpoint_system)
            continue;
        endpoint_systems.insert(const_ptr_cast<System>(endpoint_system));
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


///////////////////////////////////////////////////////////
// SetStarType                                           //
///////////////////////////////////////////////////////////
SetStarType::SetStarType(const ValueRef::ValueRefBase<StarType>* type) :
    m_type(type)
{}

SetStarType::~SetStarType()
{ delete m_type; }

void SetStarType::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        Logger().errorStream() << "SetStarType::Execute given no target object";
        return;
    }
    if (TemporaryPtr<System> s = dynamic_ptr_cast<System>(context.effect_target))
        s->SetStarType(m_type->Eval(ScriptingContext(context, s->GetStarType())));
    else
        Logger().errorStream() << "SetStarType::Execute given a non-system target";
}

std::string SetStarType::Description() const {
    std::string value_str = ValueRef::ConstantExpr(m_type) ?
                                UserString(lexical_cast<std::string>(m_type->Eval())) :
                                m_type->Description();
    return str(FlexibleFormat(UserString("DESC_SET_STAR_TYPE")) % value_str);
}

std::string SetStarType::Dump() const
{ return DumpIndent() + "SetStarType type = " + m_type->Dump() + "\n"; }


///////////////////////////////////////////////////////////
// MoveTo                                                //
///////////////////////////////////////////////////////////
MoveTo::MoveTo(const Condition::ConditionBase* location_condition) :
    m_location_condition(location_condition)
{}

MoveTo::~MoveTo()
{ delete m_location_condition; }

void MoveTo::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        Logger().errorStream() << "MoveTo::Execute given no target object";
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
    TemporaryPtr<UniverseObject> destination = const_ptr_cast<UniverseObject>(*valid_locations.begin());


    // do the moving...

    if (TemporaryPtr<Fleet> fleet = dynamic_ptr_cast<Fleet>(context.effect_target)) {
        // fleets can be inserted into the system that contains the destination object (or the 
        // destination object istelf if it is a system
        if (TemporaryPtr<System> dest_system = GetSystem(destination->SystemID())) {
            if (fleet->SystemID() != dest_system->ID()) {
                dest_system->Insert(fleet);
                ExploreSystem(dest_system->ID(), fleet);
                UpdateFleetRoute(fleet, INVALID_OBJECT_ID, INVALID_OBJECT_ID);  // inserted into dest_system, so next and previous systems are invalid objects
            }
        } else {
            fleet->UniverseObject::MoveTo(destination); // UniverseObject:: scope specification seemingly needed due to mixture of overridded and non-virtual base class MoveTo functions

            // fleet has been moved to a location that is not a system.  Presumably this will be located on a starlane between two
            // other systems, which may or may not have been explored.  Regardless, the fleet needs to be given a new next and
            // previous system so it can move into a system, or can be ordered to a new location, and so that it won't try to move
            // off of starlanes towards some other system from its current location (if it was heading to another system) and so it
            // won't be stuck in the middle of a starlane, unable to move (if it wasn't previously moving)

            // if destination object is a fleet or is part of a fleet, can use that fleet's previous and next systems to get
            // valid next and previous systems for the target fleet.
            TemporaryPtr<const Fleet> dest_fleet;

            dest_fleet = dynamic_ptr_cast<const Fleet>(destination);
            if (!dest_fleet)
                if (TemporaryPtr<const Ship> dest_ship = dynamic_ptr_cast<const Ship>(destination))
                    dest_fleet = GetFleet(dest_ship->FleetID());

            if (dest_fleet) {
                UpdateFleetRoute(fleet, dest_fleet->NextSystemID(), dest_fleet->PreviousSystemID());
            } else {
                // need to do something more fancy, although as of this writing, there are no other types of UniverseObject subclass
                // that can be located between systems other than fleets and ships, so this shouldn't matter for now...
                Logger().errorStream() << "Effect::MoveTo::Execute couldn't find a way to set the previous and next systems for the target fleet!";
            }
        }

    } else if (TemporaryPtr<Ship> ship = dynamic_ptr_cast<Ship>(context.effect_target)) {
        // TODO: make sure colonization doesn't interfere with this effect, and vice versa

        TemporaryPtr<Fleet> old_fleet = GetFleet(ship->FleetID());
        TemporaryPtr<Fleet> dest_fleet = dynamic_ptr_cast<Fleet>(destination);  // may be 0 if destination is not a fleet
        bool same_owners = ship->Owner() == destination->Owner();
        int dest_sys_id = destination->SystemID();
        int ship_sys_id = ship->SystemID();

        if (dest_fleet && same_owners) {
            // ship is moving to a different fleet owned by the same empire, so can be inserted into it
            dest_fleet->AddShip(ship->ID());    // does nothing if fleet already contains the ship

        } else if (dest_sys_id == ship_sys_id && dest_sys_id != INVALID_OBJECT_ID) {
            // ship is moving to the system it is already in, but isn't being or can't be moved into a specific fleet, so the ship
            // can be left in its current fleet and at its current location

        } else if (destination->X() == ship->X() && destination->Y() == ship->Y()) {
            // ship is moving to the same location it's already at, but isn't being or can't be moved to a specific fleet, so the ship
            // can be left in its current fleet and at its current location

        } else {
            // need to create a new fleet for ship
            if (TemporaryPtr<System> dest_system = GetSystem(destination->SystemID())) {
                CreateNewFleet(dest_system, ship);                          // creates new fleet, inserts fleet into system and ship into fleet
                ExploreSystem(dest_system->ID(), ship);

            } else {
                CreateNewFleet(destination->X(), destination->Y(), ship);   // creates new fleet and inserts ship into fleet
            }
        }

        if (old_fleet && old_fleet->NumShips() < 1)
            universe.EffectDestroy(old_fleet->ID());

    } else if (TemporaryPtr<Planet> planet = dynamic_ptr_cast<Planet>(context.effect_target)) {
        // planets need to be located in systems, so get system that contains destination object
        if (TemporaryPtr<System> dest_system = GetSystem(destination->SystemID())) {
            // check if planet is already in this system.  if so, don't need to do anything
            if (planet->SystemID() == INVALID_OBJECT_ID || planet->SystemID() != dest_system->ID()) {
                //  determine if and which orbits are available
                std::set<int> free_orbits = dest_system->FreeOrbits();
                if (!free_orbits.empty()) {
                    int orbit = *(free_orbits.begin());
                    dest_system->Insert(TemporaryPtr<UniverseObject>(planet), orbit);
                    ExploreSystem(dest_system->ID(), planet);
                }
            }
        }
        // don't move planets to a location outside a system

    } else if (TemporaryPtr<Building> building = dynamic_ptr_cast<Building>(context.effect_target)) {
        // buildings need to be located on planets, so if destination is a planet, insert building into it,
        // or attempt to get the planet on which the destination object is located and insert target building into that
        if (TemporaryPtr<Planet> dest_planet = dynamic_ptr_cast<Planet>(destination)) {
            dest_planet->AddBuilding(building->ID());
            if (TemporaryPtr<const System> dest_system = GetSystem(dest_planet->SystemID()))
                ExploreSystem(dest_system->ID(), building);


        } else if (TemporaryPtr<Building> dest_building = dynamic_ptr_cast<Building>(destination)) {
            if (TemporaryPtr<Planet> dest_planet = GetPlanet(dest_building->PlanetID())) {
                dest_planet->AddBuilding(building->ID());
                if (TemporaryPtr<const System> dest_system = GetSystem(dest_planet->SystemID()))
                    ExploreSystem(dest_system->ID(), building);
            }
        }
        // else if destination is something else that can be on a planet...

    } else if (TemporaryPtr<System> system = dynamic_ptr_cast<System>(context.effect_target)) {
        if (destination->SystemID() == INVALID_OBJECT_ID) {
            // can simply move system to new location, and insert destination object into system
            system->UniverseObject::MoveTo(destination);
            // find fleets at this location and insert into system
            std::vector<TemporaryPtr<Fleet> > fleets = GetUniverse().Objects().FindObjects<Fleet>();
            for (std::vector<TemporaryPtr<Fleet> >::iterator it = fleets.begin(); it != fleets.end(); ++it)
                if (TemporaryPtr<Fleet> fleet = *it)
                    if (fleet->X() == system->X() && fleet->Y() == system->Y())
                        system->Insert(fleet);
        } else {
            //TemporaryPtr<System> destination_system = GetSystem(destination->SystemID());
            // TODO: merge systems
        }
    } else if (TemporaryPtr<Field> field = dynamic_ptr_cast<Field>(context.effect_target)) {
        field->UniverseObject::MoveTo(destination);
    }
}

std::string MoveTo::Description() const {
    std::string value_str = m_location_condition->Description();
    return str(FlexibleFormat(UserString("DESC_MOVE_TO")) % value_str);
}

std::string MoveTo::Dump() const
{ return DumpIndent() + "MoveTo destination = " + m_location_condition->Dump() + "\n"; }


///////////////////////////////////////////////////////////
// MoveInOrbit                                           //
///////////////////////////////////////////////////////////
MoveInOrbit::MoveInOrbit(const ValueRef::ValueRefBase<double>* speed,
                         const Condition::ConditionBase* focal_point_condition) :
    m_speed(speed),
    m_focal_point_condition(focal_point_condition),
    m_focus_x(0),
    m_focus_y(0)
{}

MoveInOrbit::MoveInOrbit(const ValueRef::ValueRefBase<double>* speed,
                         const ValueRef::ValueRefBase<double>* focus_x/* = 0*/,
                         const ValueRef::ValueRefBase<double>* focus_y/* = 0*/) :
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
        Logger().errorStream() << "MoveInOrbit::Execute given no target object";
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

    if (TemporaryPtr<System> system = dynamic_ptr_cast<System>(target)) {
        system->MoveTo(new_x, new_y);
        return;
    } else if (TemporaryPtr<Fleet> fleet = dynamic_ptr_cast<Fleet>(target)) {
        fleet->MoveTo(new_x, new_y);
        // todo: is fleet now close enough to fall into a system?
        UpdateFleetRoute(fleet, INVALID_OBJECT_ID, INVALID_OBJECT_ID);
        return;
    } else if (TemporaryPtr<Ship> ship = dynamic_ptr_cast<Ship>(target)) {
        TemporaryPtr<Fleet> old_fleet = GetFleet(ship->FleetID());
        CreateNewFleet(new_x, new_y, ship); // creates new fleet and inserts ship into fleet
        if (old_fleet && old_fleet->NumShips() < 1)
            GetUniverse().EffectDestroy(old_fleet->ID());
    } else if (TemporaryPtr<Field> field = dynamic_ptr_cast<Field>(target)) {
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


///////////////////////////////////////////////////////////
// MoveTowards                                           //
///////////////////////////////////////////////////////////
MoveTowards::MoveTowards(const ValueRef::ValueRefBase<double>* speed,
                         const Condition::ConditionBase* dest_condition) :
    m_speed(speed),
    m_dest_condition(dest_condition),
    m_dest_x(0),
    m_dest_y(0)
{}

MoveTowards::MoveTowards(const ValueRef::ValueRefBase<double>* speed,
                         const ValueRef::ValueRefBase<double>* dest_x/* = 0*/,
                         const ValueRef::ValueRefBase<double>* dest_y/* = 0*/) :
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
        Logger().errorStream() << "MoveTowards::Execute given no target object";
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
        return;

    if (TemporaryPtr<System> system = dynamic_ptr_cast<System>(target)) {
        system->MoveTo(new_x, new_y);
        return;
    } else if (TemporaryPtr<Fleet> fleet = dynamic_ptr_cast<Fleet>(target)) {
        fleet->MoveTo(new_x, new_y);
        // todo: is fleet now close enough to fall into a system?
        UpdateFleetRoute(fleet, INVALID_OBJECT_ID, INVALID_OBJECT_ID);
        return;
    } else if (TemporaryPtr<Ship> ship = dynamic_ptr_cast<Ship>(target)) {
        TemporaryPtr<Fleet> old_fleet = GetFleet(ship->FleetID());
        CreateNewFleet(new_x, new_y, ship); // creates new fleet and inserts ship into fleet
        if (old_fleet && old_fleet->NumShips() < 1)
            GetUniverse().EffectDestroy(old_fleet->ID());
    } else if (TemporaryPtr<Field> field = dynamic_ptr_cast<Field>(target)) {
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


///////////////////////////////////////////////////////////
// SetDestination                                        //
///////////////////////////////////////////////////////////
SetDestination::SetDestination(const Condition::ConditionBase* location_condition) :
    m_location_condition(location_condition)
{}

SetDestination::~SetDestination()
{ delete m_location_condition; }

void SetDestination::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        Logger().errorStream() << "SetDestination::Execute given no target object";
        return;
    }

    TemporaryPtr<Fleet> target_fleet = dynamic_ptr_cast<Fleet>(context.effect_target);
    if (!target_fleet) {
        Logger().errorStream() << "SetDestination::Execute acting on non-fleet target:";
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
    TemporaryPtr<UniverseObject> destination = const_ptr_cast<UniverseObject>(*obj_it);
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

///////////////////////////////////////////////////////////
// SetAggression                                         //
///////////////////////////////////////////////////////////
SetAggression::SetAggression(bool aggressive) :
    m_aggressive(aggressive)
{}

void SetAggression::Execute(const ScriptingContext& context) const {
    if (!context.effect_target) {
        Logger().errorStream() << "SetAggression::Execute given no target object";
        return;
    }

    TemporaryPtr<Fleet> target_fleet = dynamic_ptr_cast<Fleet>(context.effect_target);
    if (!target_fleet) {
        Logger().errorStream() << "SetAggression::Execute acting on non-fleet target:";
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
        Logger().errorStream() << "Victory::Execute given no target object";
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
                                             const ValueRef::ValueRefBase<int>* empire_id) :
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
    Empire* empire = Empires().Lookup(m_empire_id->Eval(context));
    if (!empire) return;

    if (!m_tech_name) {
        Logger().errorStream() << "SetEmpireTechProgress::Execute has not tech name to evaluate";
        return;
    }
    std::string tech_name = m_tech_name->Eval(context);
    if (tech_name.empty())
        return;

    const Tech* tech = GetTech(tech_name);
    if (!tech) {
        Logger().errorStream() << "SetEmpireTechProgress::Execute couldn't get tech with name " << tech_name;
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
            if (const Empire* empire = Empires().Lookup(m_empire_id->Eval()))
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


///////////////////////////////////////////////////////////
// GiveEmpireTech                                        //
///////////////////////////////////////////////////////////
GiveEmpireTech::GiveEmpireTech(const std::string& tech_name) :
    m_tech_name(tech_name),
    m_empire_id(new ValueRef::Variable<int>(ValueRef::EFFECT_TARGET_REFERENCE, std::vector<std::string>(1, "Owner")))
{}

GiveEmpireTech::GiveEmpireTech(const std::string& tech_name,
                               const ValueRef::ValueRefBase<int>* empire_id) :
    m_tech_name(tech_name),
    m_empire_id(empire_id)
{}

GiveEmpireTech::~GiveEmpireTech()
{ delete m_empire_id; }

void GiveEmpireTech::Execute(const ScriptingContext& context) const {
    if (!m_empire_id) return;
    Empire* empire = Empires().Lookup(m_empire_id->Eval(context));
    if (!empire) return;

    const Tech* tech = GetTech(m_tech_name);
    if (!tech) {
        Logger().errorStream() << "GiveEmpireTech::Execute couldn't get tech with name " << m_tech_name;
        return;
    }

    empire->AddTech(m_tech_name);
}

std::string GiveEmpireTech::Description() const {
    std::string empire_str;
    if (m_empire_id) {
        if (ValueRef::ConstantExpr(m_empire_id)) {
            if (const Empire* empire = Empires().Lookup(m_empire_id->Eval()))
                empire_str = empire->Name();
        } else {
            empire_str = m_empire_id->Description();
        }
    }
    return str(FlexibleFormat(UserString("DESC_GIVE_EMPIRE_TECH"))
                % UserString(m_tech_name)
                % empire_str);
}

std::string GiveEmpireTech::Dump() const {
    std::string retval = "GiveEmpireTech name = \"" + m_tech_name + "\"";
    if (m_empire_id)
        retval += " empire = " + m_empire_id->Dump();
    retval += "\n";
    return retval;
}


///////////////////////////////////////////////////////////
// GenerateSitRepMessage                                 //
///////////////////////////////////////////////////////////
GenerateSitRepMessage::GenerateSitRepMessage(const std::string& message_string,
                                             const std::string& icon,
                                             const std::vector<std::pair<std::string, const ValueRef::ValueRefBase<std::string>*> >& message_parameters,
                                             const ValueRef::ValueRefBase<int>* recipient_empire_id,
                                             EmpireAffiliationType affiliation) :
    m_message_string(message_string),
    m_icon(icon),
    m_message_parameters(message_parameters),
    m_recipient_empire_id(recipient_empire_id),
    m_affiliation(affiliation)
{}

GenerateSitRepMessage::GenerateSitRepMessage(const std::string& message_string,
                                             const std::string& icon,
                                             const std::vector<std::pair<std::string, const ValueRef::ValueRefBase<std::string>*> >& message_parameters,
                                             EmpireAffiliationType affiliation) :
    m_message_string(message_string),
    m_icon(icon),
    m_message_parameters(message_parameters),
    m_recipient_empire_id(0),
    m_affiliation(affiliation)
{}

GenerateSitRepMessage::~GenerateSitRepMessage() {
    for (std::vector<std::pair<std::string, const ValueRef::ValueRefBase<std::string>*> >::iterator it =
         m_message_parameters.begin(); it != m_message_parameters.end(); ++it)
    {
        delete it->second;
    }
    delete m_recipient_empire_id;
}

void GenerateSitRepMessage::Execute(const ScriptingContext& context) const {
    Empire* empire = 0;
    if (m_recipient_empire_id)
        empire = Empires().Lookup(m_recipient_empire_id->Eval(context));
    if (!empire && m_affiliation != AFFIL_ANY) return;

    std::vector<std::pair<std::string, std::string> > parameter_tag_values;
    for (std::vector<std::pair<std::string, const ValueRef::ValueRefBase<std::string>*> >::const_iterator it =
         m_message_parameters.begin(); it != m_message_parameters.end(); ++it)
    {
        parameter_tag_values.push_back(std::make_pair(it->first, it->second->Eval(context)));
        if (it->first == VarText::PREDEFINED_DESIGN_TAG) {
            if (const ShipDesign* design = GetPredefinedShipDesign(it->second->Eval(context))) {
                int design_id = design->ID();
                if (!empire) {
                    // ensure all empires aware of ship design
                    for (EmpireManager::const_iterator empire_it = Empires().begin(); empire_it != Empires().end(); ++empire_it)
                        GetUniverse().SetEmpireKnowledgeOfShipDesign(design_id, empire_it->first);
                } else if (m_affiliation == AFFIL_SELF) {
                    // ensure empire has knowledge of the design
                    GetUniverse().SetEmpireKnowledgeOfShipDesign(design_id, empire->EmpireID());
                }
            }
        }
    }

    if (!empire) {
        // send to all empires
        for (EmpireManager::const_iterator empire_it = Empires().begin(); empire_it != Empires().end(); ++empire_it)
            empire_it->second->AddSitRepEntry(CreateSitRep(m_message_string, m_icon, parameter_tag_values));
    } else if (m_affiliation == AFFIL_SELF) {
        // send to just single empire
        empire->AddSitRepEntry(CreateSitRep(m_message_string, m_icon, parameter_tag_values));
    } else if (m_affiliation == AFFIL_ENEMY) {
        // send to enemies of single empire
    } else if (m_affiliation == AFFIL_ALLY) {
        // send to allies
    }
}

std::string GenerateSitRepMessage::Description() const {
    std::string empire_str;
    if (m_recipient_empire_id) {
        int empire_id = ALL_EMPIRES;
        if (ValueRef::ConstantExpr(m_recipient_empire_id))
            empire_id = m_recipient_empire_id->Eval();
        if (const Empire* empire = Empires().Lookup(empire_id))
            empire_str = empire->Name();
        else
            empire_str = m_recipient_empire_id->Description();
    }
    return str(FlexibleFormat(UserString("DESC_GENERATE_SITREP")) % empire_str);
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
    default:            retval += "?";          break;
    }

    if (m_recipient_empire_id)
        retval += "\n" + DumpIndent() + "empire = " + m_recipient_empire_id->Dump() + "\n";
    --g_indent;

    return retval;
}


///////////////////////////////////////////////////////////
// SetOverlayTexture                                     //
///////////////////////////////////////////////////////////
SetOverlayTexture::SetOverlayTexture(const std::string& texture, const ValueRef::ValueRefBase<double>* size) :
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

    if (TemporaryPtr<System> system = dynamic_ptr_cast<System>(context.effect_target))
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


///////////////////////////////////////////////////////////
// SetTexture                                 //
///////////////////////////////////////////////////////////
SetTexture::SetTexture(const std::string& texture) :
    m_texture(texture)
{}

void SetTexture::Execute(const ScriptingContext& context) const {
    if (!context.effect_target)
        return;
    if (TemporaryPtr<Planet> planet = dynamic_ptr_cast<Planet>(context.effect_target))
        planet->SetSurfaceTexture(m_texture);
}

std::string SetTexture::Description() const
{ return UserString("DESC_SET_TEXTURE"); }

std::string SetTexture::Dump() const
{ return DumpIndent() + "SetTexture texture = " + m_texture + "\n"; }
