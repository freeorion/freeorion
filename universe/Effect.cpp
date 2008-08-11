#include "Effect.h"

#include "../util/AppInterface.h"
#include "ValueRef.h"
#include "Condition.h"
#include "Universe.h"
#include "UniverseObject.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Empire.h"
#include "Building.h"
#include "Planet.h"
#include "System.h"
#include "Fleet.h"
#include "Ship.h"
#include "Tech.h"

#include <cctype>

using namespace Effect;
using namespace boost::io;
using boost::lexical_cast;

extern int g_indent;

namespace {
    boost::tuple<bool, ValueRef::OpType, double>
    SimpleMeterModification(MeterType meter, bool max, const ValueRef::ValueRefBase<double>* ref)
    {
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
                        ((max ? "Max" : "Current") + meter_str) == var->PropertyName()[0];
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
                        ((max ? "Max" : "Current") + meter_str) == var->PropertyName()[0];
                    retval.get<1>() = op->GetOpType();
                    retval.get<2>() = constant->Value();
                    return retval;
                }
            }
        }
        return retval;
    }

    /** creates a new fleet at \a system and inserts \a ship into it.  used when a ship has been moved by the MoveTo
        effect separately from the fleet that previously held it.  all ships need to be within fleets. */
    Fleet* CreateNewFleet(System* system, Ship* ship) {
        Universe& universe = GetUniverse();
        if (!system || !ship)
            return 0;

        int owner_empire_id = -1;
        const std::set<int>& owners = ship->Owners();
        if (!owners.empty())
            owner_empire_id = *(owners.begin());

        int new_fleet_id = GetNewObjectID();

        std::vector<int> ship_ids;
        ship_ids.push_back(ship->ID());
        std::string fleet_name = Fleet::GenerateFleetName(ship_ids, new_fleet_id);

        Fleet* fleet = new Fleet(fleet_name, system->X(), system->Y(), owner_empire_id);

        universe.InsertID(fleet, new_fleet_id);
        system->Insert(fleet);

        fleet->AddShip(ship->ID());

        return fleet;
    }

    /** creates a new fleet at a specified \a x and \a y location within the Universe, and and inserts \a ship into it.
        used when a ship has been moved by the MoveTo effect separately from the fleet that previously held it.  all
        ships need to be within fleets. */
    Fleet* CreateNewFleet(double x, double y, Ship* ship) {
        Universe& universe = GetUniverse();
        if (!ship)
            return 0;

        int owner_empire_id = -1;
        const std::set<int>& owners = ship->Owners();
        if (!owners.empty())
            owner_empire_id = *(owners.begin());

        int new_fleet_id = GetNewObjectID();

        std::vector<int> ship_ids;
        ship_ids.push_back(ship->ID());
        std::string fleet_name = Fleet::GenerateFleetName(ship_ids, new_fleet_id);

        Fleet* fleet = new Fleet(fleet_name, x, y, owner_empire_id);

        universe.InsertID(fleet, new_fleet_id);

        fleet->AddShip(ship->ID());

        return fleet;
    }

    /** Explores the system with the specified \a system_id for the owner of the specified \a target_object.  Used when
        moving objects into a system with the MoveTo effect, as otherwise the system wouldn't get explored, and objects
        being moved into unexplored systems might disappear for players or confuse the AI. */
    void ExploreSystem(int system_id, const UniverseObject* target_object) {
        if (!target_object) return;
        const std::set<int>& owners = target_object->Owners();
        if (!owners.empty())
            if (Empire* owner_empire = Empires().Lookup(*owners.begin()))
                owner_empire->AddExploredSystem(system_id);
    }

    /** Resets the previous and next systems of \a fleet and recalcultes / resets the fleet's move route.  Used after a fleet
        has been moved with the MoveTo effect, as its previous route was assigned based on its previous location, an may not
        be valid for its new location. */
    void UpdateFleetRoute(Fleet* fleet, int new_next_system, int new_previous_system) {
        if (!fleet) {
            Logger().errorStream() << "UpdateFleetRoute passed a null fleet pointer";
            return;
        }

        Universe& universe = GetUniverse();

        System* next_system = universe.Object<System>(new_next_system);
        if (!next_system) {
            Logger().errorStream() << "UpdateFleetRoute couldn't get new next system with id: " << new_next_system;
            return;
        }

        fleet->SetNextAndPreviousSystems(new_next_system, new_previous_system);

        int owner = -1;
        const std::set<int>& owners = fleet->Owners();
        if (!owners.empty())
            owner = *owners.begin();

        int start_system = fleet->SystemID();
        if (start_system == UniverseObject::INVALID_OBJECT_ID)
            start_system = new_next_system;

        int dest_system = fleet->FinalDestinationID();

        std::pair<std::list<System*>, double> route_pair = universe.ShortestPath(start_system, dest_system, owner);

        if (route_pair.first.empty()) {
            route_pair.first.push_back(next_system);

            double dist_x = next_system->X() - fleet->X();
            double dist_y = next_system->Y() - fleet->Y();
            route_pair.second = std::sqrt(dist_x * dist_x + dist_y * dist_y);
        }

        fleet->SetRoute(route_pair.first, route_pair.second);
    }

    /** returns true of the owners of the two passed objects are the same, and both are owned, false otherwise */
    bool SameOwners(const UniverseObject* obj1, const UniverseObject* obj2) {
        if (!obj1 || !obj2) return false;

        const std::set<int>& owners1 = obj1->Owners();
        const std::set<int>& owners2 = obj2->Owners();

        int owner1 = *owners1.begin();
        int owner2 = *owners2.begin();

        return owner1 == owner2;
    }
}

///////////////////////////////////////////////////////////
// EffectsGroup                                          //
///////////////////////////////////////////////////////////
EffectsGroup::EffectsGroup(const Condition::ConditionBase* scope, const Condition::ConditionBase* activation,
                           const std::vector<EffectBase*>& effects, const std::string& stacking_group/* = ""*/) :
    m_scope(scope),
    m_activation(activation),
    m_stacking_group(stacking_group),
    m_effects(effects)
{}

EffectsGroup::~EffectsGroup()
{
    delete m_scope;
    delete m_activation;
    for (unsigned int i = 0; i < m_effects.size(); ++i) {
        delete m_effects[i];
    }
}

void EffectsGroup::GetTargetSet(int source_id, TargetSet& targets, const TargetSet& potential_targets) const
{
    Universe& universe = GetUniverse();
    UniverseObject* source = universe.Object(source_id);
    assert(source);

    targets.clear();

    // evaluate the activation condition only on the source object
    Condition::ObjectSet non_targets;
    non_targets.insert(source);
    m_activation->Eval(source, targets, non_targets);

    // if the activation condition did not evaluate to true for the source object, do nothing
    if (targets.empty())
        return;

    // evaluate the scope condition
    targets.clear();
    non_targets = potential_targets;
    m_scope->Eval(source, targets, non_targets);
}

void EffectsGroup::GetTargetSet(int source_id, TargetSet& targets) const
{
    Universe& universe = GetUniverse();
    Condition::ObjectSet potential_targets;
    for (Universe::const_iterator it = universe.begin(); it != universe.end(); ++it)
        potential_targets.insert(it->second);
    GetTargetSet(source_id, targets, potential_targets);
}

void EffectsGroup::Execute(int source_id, const TargetSet& targets) const
{
    UniverseObject* source = GetUniverse().Object(source_id);
    assert(source);

    // execute effects on targets
    for (Condition::ObjectSet::const_iterator it = targets.begin(); it != targets.end(); ++it) {
        //Logger().debugStream() << "effectsgroup source: " << source->Name() << " target " << (*it)->Name();
        for (unsigned int i = 0; i < m_effects.size(); ++i) {
            m_effects[i]->Execute(source, *it);
        }
    }
}

void EffectsGroup::Execute(int source_id, const TargetSet& targets, int effect_index) const
{
    UniverseObject* source = GetUniverse().Object(source_id);
    assert(source);

    assert(0 <= effect_index && effect_index < static_cast<int>(m_effects.size()));

    // execute effect on targets
    for (Condition::ObjectSet::const_iterator it = targets.begin(); it != targets.end(); ++it) {
        m_effects[effect_index]->Execute(source, *it);
    }
}

const std::string& EffectsGroup::StackingGroup() const
{
    return m_stacking_group;
}

const std::vector<EffectBase*>& EffectsGroup::EffectsList() const
{
    return m_effects;
}

EffectsGroup::Description EffectsGroup::GetDescription() const
{
    Description retval;
    if (dynamic_cast<const Condition::Self*>(m_scope))
        retval.scope_description = UserString("DESC_EFFECTS_GROUP_SELF_SCOPE");
    else
        retval.scope_description = str(FlexibleFormat(UserString("DESC_EFFECTS_GROUP_SCOPE")) % m_scope->Description());
    if (dynamic_cast<const Condition::Self*>(m_activation) || dynamic_cast<const Condition::All*>(m_activation))
        retval.activation_description = UserString("DESC_EFFECTS_GROUP_ALWAYS_ACTIVE");
    else
        retval.activation_description = str(FlexibleFormat(UserString("DESC_EFFECTS_GROUP_ACTIVATION")) % m_activation->Description());
    for (unsigned int i = 0; i < m_effects.size(); ++i) {
        retval.effect_descriptions.push_back(m_effects[i]->Description());
    }
    return retval;
}

std::string EffectsGroup::DescriptionString() const
{
    if (!m_explicit_description.empty()) {
        return UserString(m_explicit_description);
    } else {
        std::stringstream retval;
        Description description = GetDescription();
        retval << str(FlexibleFormat(UserString("DESC_EFFECTS_GROUP_SCOPE_DESC")) % description.scope_description);
        if (!dynamic_cast<const Condition::Self*>(m_activation) && !dynamic_cast<const Condition::All*>(m_activation))
            retval << str(FlexibleFormat(UserString("DESC_EFFECTS_GROUP_ACTIVATION_DESC")) % description.activation_description);
        for (unsigned int i = 0; i < description.effect_descriptions.size(); ++i) {
            retval << str(FlexibleFormat(UserString("DESC_EFFECTS_GROUP_EFFECT_DESC")) % description.effect_descriptions[i]);
        }
        return retval.str();
    }
}

std::string EffectsGroup::Dump() const
{
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
std::string EffectsDescription(const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects_groups)
{
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


///////////////////////////////////////////////////////////
// SetMeter                                              //
///////////////////////////////////////////////////////////
SetMeter::SetMeter(MeterType meter, const ValueRef::ValueRefBase<double>* value, bool max) :
    m_meter(meter),
    m_value(value),
    m_max(max)
{}

SetMeter::~SetMeter()
{
    delete m_value;
}

void SetMeter::Execute(const UniverseObject* source, UniverseObject* target) const
{
    if (Meter* m = target->GetMeter(m_meter)) {
        double val = m_value->Eval(source, target);
        //Logger().debugStream() << "Setting " << (m_max ? "max " : "current ") << boost::lexical_cast<std::string>(m_meter) << " meter from "
        //                       << (m_max ? m->Max() : m->Current()) << " to " << val;
        m_max ? m->SetMax(val) : m->SetCurrent(val);
    }
}

std::string SetMeter::Description() const
{
    bool simple;
    ValueRef::OpType op;
    double const_operand;
    boost::tie(simple, op, const_operand) = SimpleMeterModification(m_meter, m_max, m_value);
    if (simple) {
        char op_char = '+';
        switch (op) {
        case ValueRef::PLUS:    op_char = '+'; break;
        case ValueRef::MINUS:   op_char = '-'; break;
        case ValueRef::TIMES:   op_char = '*'; break;
        case ValueRef::DIVIDES: op_char = '/'; break;
        default: op_char = '?';
        }
        return str(FlexibleFormat(UserString(m_max ? "DESC_SIMPLE_SET_METER_MAX" : "DESC_SIMPLE_SET_METER_CURRENT"))
                   % UserString(lexical_cast<std::string>(m_meter))
                   % op_char
                   % lexical_cast<std::string>(const_operand));
    } else {
        return str(FlexibleFormat(UserString(m_max ? "DESC_COMPLEX_SET_METER_MAX" : "DESC_COMPLEX_SET_METER_CURRENT"))
                   % UserString(lexical_cast<std::string>(m_meter))
                   % m_value->Description());
    }
}

std::string SetMeter::Dump() const
{
    std::string retval = DumpIndent() + (m_max ? "SetMax" : "SetCurrent");
    switch (m_meter) {
    case METER_POPULATION:   retval += "Population"; break;
    case METER_FARMING:      retval += "Farming"; break;
    case METER_INDUSTRY:     retval += "Industry"; break;
    case METER_RESEARCH:     retval += "Research"; break;
    case METER_TRADE:        retval += "Trade"; break;
    case METER_MINING:       retval += "Mining"; break;
    case METER_CONSTRUCTION: retval += "Construction"; break;
    case METER_HEALTH:       retval += "Health"; break;
    default: retval += "?"; break;
    }
    retval += " value = " + m_value->Dump() + "\n";
    return retval;
}


///////////////////////////////////////////////////////////
// SetEmpireStockpile                                    //
///////////////////////////////////////////////////////////
SetEmpireStockpile::SetEmpireStockpile(ResourceType stockpile, const ValueRef::ValueRefBase<double>* value) :
    m_stockpile(stockpile),
    m_value(value)
{}

SetEmpireStockpile::~SetEmpireStockpile()
{
    delete m_value;
}

void SetEmpireStockpile::Execute(const UniverseObject* source, UniverseObject* target) const
{
    if (source->Owners().size() != 1)
        return;

    double value = m_value->Eval(source, target);
    Empire* empire = Empires().Lookup(*source->Owners().begin());
    empire->SetResourceStockpile(m_stockpile, value);
}

std::string SetEmpireStockpile::Description() const
{
    std::string value_str = ValueRef::ConstantExpr(m_value) ? lexical_cast<std::string>(m_value->Eval(0, 0)) : m_value->Description();
    return str(FlexibleFormat(UserString("DESC_SET_EMPIRE_STOCKPILE")) % UserString(lexical_cast<std::string>(m_stockpile)) % value_str);
}

std::string SetEmpireStockpile::Dump() const
{
    std::string retval = DumpIndent();
    switch (m_stockpile) {
    case RE_FOOD:       retval += "SetOwnerFoodStockpile"; break;
    case RE_MINERALS:   retval += "SetOwnerMineralStockpile"; break;
    case RE_TRADE:      retval += "SetOwnerTradeStockpile"; break;
    default:            retval += "?"; break;
    }
    retval += " value = " + m_value->Dump() + "\n";
    return retval;
}


///////////////////////////////////////////////////////////
// SetEmpireCapitol                                      //
///////////////////////////////////////////////////////////
SetEmpireCapitol::SetEmpireCapitol()
{}

void SetEmpireCapitol::Execute(const UniverseObject* source, UniverseObject* target) const
{
    if (const Planet* planet = universe_object_cast<const Planet*>(target)) {   // verify that target object is a planet
        const std::set<int>& owners = planet->Owners();                         // get owner(s)
        if (owners.size() == 1)                                                 // verify that there is only a single owner
            if (Empire* empire = Empires().Lookup(*owners.begin()))             // get that owner empire object
                empire->SetCapitolID(planet->ID());                             // make target planet the capitol of its owner empire
    }
}

std::string SetEmpireCapitol::Description() const
{
    return UserString("DESC_SET_EMPIRE_CAPITOL");
}

std::string SetEmpireCapitol::Dump() const
{
    return DumpIndent() + "SetEmpireCapitol\n";
}


///////////////////////////////////////////////////////////
// SetPlanetType                                         //
///////////////////////////////////////////////////////////
SetPlanetType::SetPlanetType(const ValueRef::ValueRefBase<PlanetType>* type) :
    m_type(type)
{}

SetPlanetType::~SetPlanetType()
{
    delete m_type;
}

void SetPlanetType::Execute(const UniverseObject* source, UniverseObject* target) const
{
    if (Planet* p = universe_object_cast<Planet*>(target)) {
        PlanetType type = m_type->Eval(source, target);
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

std::string SetPlanetType::Description() const
{
    std::string value_str = ValueRef::ConstantExpr(m_type) ? UserString(lexical_cast<std::string>(m_type->Eval(0, 0))) : m_type->Description();
    return str(FlexibleFormat(UserString("DESC_SET_PLANET_TYPE")) % value_str);
}

std::string SetPlanetType::Dump() const
{
    return DumpIndent() + "SetPlanetType type = " + m_type->Dump() + "\n";
}


///////////////////////////////////////////////////////////
// SetPlanetSize                                         //
///////////////////////////////////////////////////////////
SetPlanetSize::SetPlanetSize(const ValueRef::ValueRefBase<PlanetSize>* size) :
    m_size(size)
{}

SetPlanetSize::~SetPlanetSize()
{
    delete m_size;
}

void SetPlanetSize::Execute(const UniverseObject* source, UniverseObject* target) const
{
    if (Planet* p = universe_object_cast<Planet*>(target)) {
        PlanetSize size = m_size->Eval(source, target);
        p->SetSize(size);
        if (size == SZ_ASTEROIDS)
            p->SetType(PT_ASTEROIDS);
        else if (size == SZ_GASGIANT)
            p->SetType(PT_GASGIANT);
        else if (p->Type() == PT_ASTEROIDS || p->Type() == PT_GASGIANT)
            p->SetType(PT_BARREN);
    }
}

std::string SetPlanetSize::Description() const
{
    std::string value_str = ValueRef::ConstantExpr(m_size) ? UserString(lexical_cast<std::string>(m_size->Eval(0, 0))) : m_size->Description();
    return str(FlexibleFormat(UserString("DESC_SET_PLANET_SIZE")) % value_str);
}

std::string SetPlanetSize::Dump() const
{
    return DumpIndent() + "SetPlanetSize size = " + m_size->Dump() + "\n";
}


///////////////////////////////////////////////////////////
// AddOwner                                              //
///////////////////////////////////////////////////////////
AddOwner::AddOwner(const ValueRef::ValueRefBase<int>* empire_id) :
    m_empire_id(empire_id)
{}

AddOwner::~AddOwner()
{
    delete m_empire_id;
}

void AddOwner::Execute(const UniverseObject* source, UniverseObject* target) const
{
    int empire_id = m_empire_id->Eval(source, target);
    assert(Empires().Lookup(empire_id));
    target->AddOwner(empire_id);
}

std::string AddOwner::Description() const
{
    std::string value_str = ValueRef::ConstantExpr(m_empire_id) ? Empires().Lookup(m_empire_id->Eval(0, 0))->Name() : m_empire_id->Description();
    return str(FlexibleFormat(UserString("DESC_ADD_OWNER")) % value_str);
}

std::string AddOwner::Dump() const
{
    return DumpIndent() + "AddOwner empire = " + m_empire_id->Dump() + "\n";
}


///////////////////////////////////////////////////////////
// RemoveOwner                                           //
///////////////////////////////////////////////////////////
RemoveOwner::RemoveOwner(const ValueRef::ValueRefBase<int>* empire_id) :
    m_empire_id(empire_id)
{}

RemoveOwner::~RemoveOwner()
{
    delete m_empire_id;
}

void RemoveOwner::Execute(const UniverseObject* source, UniverseObject* target) const
{
    int empire_id = m_empire_id->Eval(source, target);
    assert(Empires().Lookup(empire_id));
    target->RemoveOwner(empire_id);
}

std::string RemoveOwner::Description() const
{
    std::string value_str = ValueRef::ConstantExpr(m_empire_id) ? Empires().Lookup(m_empire_id->Eval(0, 0))->Name() : m_empire_id->Description();
    return str(FlexibleFormat(UserString("DESC_REMOVE_OWNER")) % value_str);
}

std::string RemoveOwner::Dump() const
{
    return DumpIndent() + "RemoveOwner empire = " + m_empire_id->Dump() + "\n";
}


///////////////////////////////////////////////////////////
// CreatePlanet                                          //
///////////////////////////////////////////////////////////
CreatePlanet::CreatePlanet(const ValueRef::ValueRefBase<PlanetType>* type, const ValueRef::ValueRefBase<PlanetSize>* size) :
    m_type(type),
    m_size(size)
{}

CreatePlanet::~CreatePlanet()
{
    delete m_type;
    delete m_size;
}

void CreatePlanet::Execute(const UniverseObject* source, UniverseObject* target) const
{
    System* location = target->GetSystem();
    if (!location) {
        Logger().errorStream() << "CreatePlanet::Execute couldn't get a System object at which to create the planet";
        return;
    }

    PlanetSize size = m_size->Eval(source, target);
    PlanetType type = m_type->Eval(source, target);
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

    Planet* planet = new Planet(type, size);
    int new_planet_id = GetNewObjectID();
    GetUniverse().InsertID(planet, new_planet_id);

    int orbit = *(free_orbits.begin());
    location->Insert(planet, orbit);
}

std::string CreatePlanet::Description() const
{
    std::string type_str = ValueRef::ConstantExpr(m_type) ? UserString(lexical_cast<std::string>(m_type->Eval(0, 0))) : m_type->Description();
    std::string size_str = ValueRef::ConstantExpr(m_size) ? UserString(lexical_cast<std::string>(m_size->Eval(0, 0))) : m_size->Description();

    return str(FlexibleFormat(UserString("DESC_CREATE_PLANET"))
               % type_str
               % size_str);
}

std::string CreatePlanet::Dump() const
{
    return DumpIndent() + "CreatePlanet size = " + m_size->Dump() + " type = " + m_type->Dump() + "\n";
}


///////////////////////////////////////////////////////////
// CreateBuilding                                        //
///////////////////////////////////////////////////////////
CreateBuilding::CreateBuilding(const std::string& building_type) :
    m_type(building_type)
{}

void CreateBuilding::Execute(const UniverseObject* source, UniverseObject* target) const
{
}

std::string CreateBuilding::Description() const
{
    return str(FlexibleFormat(UserString("DESC_CREATE_BUILDING"))
               % UserString(m_type));
}

std::string CreateBuilding::Dump() const
{
    return DumpIndent() + "CreateBuilding type = " + m_type + "\n";
}


///////////////////////////////////////////////////////////
// Destroy                                               //
///////////////////////////////////////////////////////////
Destroy::Destroy()
{}

void Destroy::Execute(const UniverseObject* source, UniverseObject* target) const
{
    GetUniverse().EffectDestroy(target->ID());
}

std::string Destroy::Description() const
{
    return UserString("DESC_DESTROY");
}

std::string Destroy::Dump() const
{
    return DumpIndent() + "Destroy\n";
}


///////////////////////////////////////////////////////////
// AddSpecial                                            //
///////////////////////////////////////////////////////////
AddSpecial::AddSpecial(const std::string& name) :
    m_name(name)
{}

void AddSpecial::Execute(const UniverseObject* source, UniverseObject* target) const
{
    target->AddSpecial(m_name);
}

std::string AddSpecial::Description() const
{
    return str(FlexibleFormat(UserString("DESC_ADD_SPECIAL")) % UserString(m_name));
}

std::string AddSpecial::Dump() const
{
    return DumpIndent() + "AddSpecial name = \"" + m_name + "\"\n";
}


///////////////////////////////////////////////////////////
// RemoveSpecial                                         //
///////////////////////////////////////////////////////////
RemoveSpecial::RemoveSpecial(const std::string& name) :
    m_name(name)
{}

void RemoveSpecial::Execute(const UniverseObject* source, UniverseObject* target) const
{
    target->RemoveSpecial(m_name);
}

std::string RemoveSpecial::Description() const
{
    return str(FlexibleFormat(UserString("DESC_REMOVE_SPECIAL")) % UserString(m_name));
}

std::string RemoveSpecial::Dump() const
{
    return DumpIndent() + "RemoveSpecial name = \"" + m_name + "\"\n";
}


///////////////////////////////////////////////////////////
// SetStarType                                           //
///////////////////////////////////////////////////////////
SetStarType::SetStarType(const ValueRef::ValueRefBase<StarType>* type) :
    m_type(type)
{}

SetStarType::~SetStarType()
{
    delete m_type;
}

void SetStarType::Execute(const UniverseObject* source, UniverseObject* target) const
{
    if (System* s = universe_object_cast<System*>(target)) {
        s->SetStarType(m_type->Eval(source, target));
    }
}

std::string SetStarType::Description() const
{
    std::string value_str = ValueRef::ConstantExpr(m_type) ? UserString(lexical_cast<std::string>(m_type->Eval(0, 0))) : m_type->Description();
    return str(FlexibleFormat(UserString("DESC_SET_STAR_TYPE")) % value_str);
}

std::string SetStarType::Dump() const
{
    return DumpIndent() + "SetStarType type = " + m_type->Dump() + "\n";
}


///////////////////////////////////////////////////////////
// MoveTo                                                //
///////////////////////////////////////////////////////////
MoveTo::MoveTo(const Condition::ConditionBase* location_condition) :
    m_location_condition(location_condition)
{}

MoveTo::~MoveTo()
{
    delete m_location_condition;
}

void MoveTo::Execute(const UniverseObject* source, UniverseObject* target) const
{
    Universe& universe = GetUniverse();

    Condition::ObjectSet potential_locations;
    for (Universe::const_iterator it = universe.begin(); it != universe.end(); ++it)
        potential_locations.insert(it->second);

    Condition::ObjectSet valid_locations;

    m_location_condition->Eval(source, valid_locations, potential_locations);

    if (valid_locations.empty())
        return;

    UniverseObject* destination = *valid_locations.begin();

    if (Fleet* fleet = universe_object_cast<Fleet*>(target)) {
        // fleets can be inserted into the system that contains the destination object (or the 
        // destination object istelf if it is a system
        if (System* dest_system = destination->GetSystem()) {
            if (fleet->SystemID() != dest_system->ID()) {
                dest_system->Insert(target);
                ExploreSystem(dest_system->ID(), target);
                UpdateFleetRoute(fleet, UniverseObject::INVALID_OBJECT_ID, UniverseObject::INVALID_OBJECT_ID);  // inserted into dest_system, so next and previous systems are invalid objects
            }
        } else {
            fleet->UniverseObject::MoveTo(destination);

            // fleet has been moved to a location that is not a system.  Presumably this will be located on a starlane between two
            // other systems, which may or may not have been explored.  Regardless, the fleet needs to be given a new next and
            // previous system so it can move into a system, or can be ordered to a new location, and so that it won't try to move
            // off of starlanes towards some other system from its current location (if it was heading to another system) and so it
            // won't be stuck in the middle of a starlane, unable to move (if it wasn't previously moving)

            // if destination object is a fleet or is part of a fleet, can use that fleet's previous and next systems to get
            // valid next and previous systems for the target fleet.
            const Fleet* dest_fleet = 0;

            dest_fleet = universe_object_cast<const Fleet*>(destination);
            if (!dest_fleet)
                if (const Ship* dest_ship = universe_object_cast<const Ship*>(destination))
                    dest_fleet = universe_object_cast<const Fleet*>(dest_ship->GetFleet());

            if (dest_fleet) {
                UpdateFleetRoute(fleet, dest_fleet->NextSystemID(), dest_fleet->PreviousSystemID());
            } else {
                // need to do something more fancy, although as of this writing, there are no other types of UniverseObject subclass
                // that can be located between systems other than fleets and ships, so this shouldn't matter for now...
                Logger().errorStream() << "Effect::MoveTo::Execute couldn't find a way to set the previous and next systems for the target fleet!";
            }
        }

    } else if (Ship* ship = universe_object_cast<Ship*>(target)) {
        // TODO: make sure colonization doesn't interfere with this effect, and vice versa

        Fleet* old_fleet = ship->GetFleet();
        Fleet* dest_fleet = universe_object_cast<Fleet*>(destination);  // may be 0 if destination is not a fleet
        bool same_owners = SameOwners(ship, destination);
        int dest_sys_id = destination->SystemID();
        int ship_sys_id = ship->SystemID();

        if (dest_fleet && same_owners) {
            // ship is moving to a different fleet owned by the same empire, so can be inserted into it
            dest_fleet->AddShip(ship->ID());    // does nothing if fleet already contains the ship

        } else if (dest_sys_id == ship_sys_id && dest_sys_id != UniverseObject::INVALID_OBJECT_ID) {
            // ship is moving to the system it is already in, but isn't being or can't be moved into a specific fleet, so the ship
            // can be left in its current fleet and at its current location

        } else if (destination->X() == ship->X() && destination->Y() == ship->Y()) {
            // ship is moving to the same location it's already at, but isn't being or can't be moved to a specific fleet, so the ship
            // can be left in its current fleet and at its current location

        } else {
            // need to create a new fleet for ship
            Fleet* new_fleet = 0;
            if (System* dest_system = destination->GetSystem()) {
                new_fleet = CreateNewFleet(dest_system, ship);                          // creates new fleet, inserts fleet into system and ship into fleet
                ExploreSystem(dest_system->ID(), target);

            } else {
                new_fleet = CreateNewFleet(destination->X(), destination->Y(), ship);   // creates new fleet and inserts ship into fleet
            }
        }

        if (old_fleet && old_fleet->NumShips() < 1)
            universe.EffectDestroy(old_fleet->ID());

    } else if (Planet* planet = universe_object_cast<Planet*>(target)) {
        // planets need to be located in systems, so get system that contains destination object
        if (System* dest_system = destination->GetSystem()) {
            // check if planet is already in this system.  if so, don't need to do anything
            if (planet->SystemID() == UniverseObject::INVALID_OBJECT_ID || planet->SystemID() != dest_system->ID()) {
                //  determine if and which orbits are available
                std::set<int> free_orbits = dest_system->FreeOrbits();
                if (!free_orbits.empty()) {
                    int orbit = *(free_orbits.begin());
                    dest_system->Insert(target, orbit);
                    ExploreSystem(dest_system->ID(), target);
                }
            }
        }
        // don't move planets to a location outside a system

    } else if (Building* building = universe_object_cast<Building*>(target)) {
        // buildings need to be located on planets, so if destination is a planet, insert building into it,
        // or attempt to get the planet on which the destination object is located and insert target building into that
        if (Planet* dest_planet = universe_object_cast<Planet*>(destination)) {
            dest_planet->AddBuilding(building->ID());
            if (const System* dest_system = dest_planet->GetSystem())
                ExploreSystem(dest_system->ID(), target);


        } else if (Building* dest_building = universe_object_cast<Building*>(destination)) {
            if (Planet* dest_planet = dest_building->GetPlanet()) {
                dest_planet->AddBuilding(building->ID());
                if (const System* dest_system = dest_planet->GetSystem())
                    ExploreSystem(dest_system->ID(), target);
            }
        }
        // else if destination is something else that can be on a planet...
    }
}

std::string MoveTo::Description() const
{
    std::string value_str = m_location_condition->Description();
    return str(FlexibleFormat(UserString("DESC_MOVE_TO")) % value_str);
}

std::string MoveTo::Dump() const
{
    return DumpIndent() + "MoveTo destination = " + m_location_condition->Dump() + "\n";
}


///////////////////////////////////////////////////////////
// SetTechAvailability                                   //
///////////////////////////////////////////////////////////
SetTechAvailability::SetTechAvailability(const std::string& tech_name, const ValueRef::ValueRefBase<int>* empire_id, bool available, bool include_tech) :
    m_tech_name(tech_name),
    m_empire_id(empire_id),
    m_available(available),
    m_include_tech(include_tech)
{}

SetTechAvailability::~SetTechAvailability()
{
    delete m_empire_id;
}

void SetTechAvailability::Execute(const UniverseObject* source, UniverseObject* target) const
{
    Empire* empire = Empires().Lookup(m_empire_id->Eval(source, target));
    const Tech* tech = GetTech(m_tech_name);
    assert(empire && tech);

    const std::vector<ItemSpec>& items = tech->UnlockedItems();
    for (unsigned int i = 0; i < items.size(); ++i) {
        if (m_available)
            empire->UnlockItem(items[i]);
        else
            empire->LockItem(items[i]);
    }

    if (m_include_tech) {
        if (m_available)
            empire->AddTech(m_tech_name);
        else
            empire->RemoveTech(m_tech_name);
    }
}

std::string SetTechAvailability::Description() const
{
    std::string affected = str(FlexibleFormat(UserString(m_include_tech ? "DESC_TECH_AND_ITEMS_AFFECTED" : "DESC_ITEMS_ONLY_AFFECTED")) % m_tech_name);
    std::string empire_str = ValueRef::ConstantExpr(m_empire_id) ? Empires().Lookup(m_empire_id->Eval(0, 0))->Name() : m_empire_id->Description();
    return str(FlexibleFormat(UserString(m_available ? "DESC_SET_TECH_AVAIL" : "DESC_SET_TECH_UNAVAIL"))
               % affected
               % empire_str);
}

std::string SetTechAvailability::Dump() const
{
    std::string retval = DumpIndent();
    if (m_available && m_include_tech)
        retval += "GiveTechToOwner";
    if (!m_available && m_include_tech)
        retval += "RevokeTechFromOwner";
    if (m_available && !m_include_tech)
        retval += "UnlockTechItemsForOwner";
    if (!m_available && !m_include_tech)
        retval += "LockTechItemsForOwner";
    retval += " name = \"" + m_tech_name + "\"\n";
    return retval;
}


///////////////////////////////////////////////////////////
// SetEffectTarget                                       //
///////////////////////////////////////////////////////////
SetEffectTarget::SetEffectTarget(const ValueRef::ValueRefBase<int>* effect_target_id) :
    m_effect_target_id(effect_target_id)
{}

SetEffectTarget::~SetEffectTarget()
{
    delete m_effect_target_id;
}

void SetEffectTarget::Execute(const UniverseObject* source, UniverseObject* target) const
{
    // TODO: implement after Effect targets are implemented
}

std::string SetEffectTarget::Description() const
{
    // TODO: implement after Effect targets are implemented
    return "ERROR: SetEffectTarget is currently unimplemented.";
}

std::string SetEffectTarget::Dump() const
{
    return "";
}
