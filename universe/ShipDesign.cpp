#include "ShipDesign.h"

#include "../util/OptionsDB.h"
#include "../util/Directories.h"
#include "../util/Logger.h"
#include "../util/AppInterface.h"
#include "../parse/Parse.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "Condition.h"
#include "Effect.h"
#include "Planet.h"
#include "Predicates.h"
#include "Species.h"
#include "Universe.h"
#include "ValueRef.h"

#include <cfloat>
#include <boost/filesystem/fstream.hpp>

using boost::io::str;

namespace {
    const bool CHEAP_AND_FAST_SHIP_PRODUCTION = false;    // makes all ships cost 1 PP and take 1 turn to build
}

namespace {
    boost::shared_ptr<Effect::EffectsGroup>
    IncreaseMeter(MeterType meter_type, float increase) {
        typedef boost::shared_ptr<Effect::EffectsGroup> EffectsGroupPtr;
        typedef std::vector<Effect::EffectBase*> Effects;
        Condition::Source* scope = new Condition::Source;
        Condition::Source* activation = new Condition::Source;
        ValueRef::ValueRefBase<double>* vr =
            new ValueRef::Operation<double>(
                ValueRef::PLUS,
                new ValueRef::Variable<double>(ValueRef::EFFECT_TARGET_VALUE_REFERENCE, std::vector<std::string>()),
                new ValueRef::Constant<double>(increase)
            );
        return EffectsGroupPtr(
            new Effect::EffectsGroup(
                scope, activation, Effects(1, new Effect::SetMeter(meter_type, vr))));
    }

    boost::shared_ptr<Effect::EffectsGroup>
    IncreaseMeter(MeterType meter_type, const std::string& part_name, float increase, bool allow_stacking = true) {
        typedef boost::shared_ptr<Effect::EffectsGroup> EffectsGroupPtr;
        typedef std::vector<Effect::EffectBase*> Effects;
        Condition::Source* scope = new Condition::Source;
        Condition::Source* activation = new Condition::Source;

        ValueRef::ValueRefBase<double>* value_vr =
            new ValueRef::Operation<double>(
                ValueRef::PLUS,
                new ValueRef::Variable<double>(ValueRef::EFFECT_TARGET_VALUE_REFERENCE, std::vector<std::string>()),
                new ValueRef::Constant<double>(increase)
            );

        ValueRef::ValueRefBase<std::string>* part_name_vr =
            new ValueRef::Constant<std::string>(part_name);

        std::string stacking_group = (allow_stacking ? "" :
            (part_name + "_" + boost::lexical_cast<std::string>(meter_type) + "_PartMeter"));

        return EffectsGroupPtr(
            new Effect::EffectsGroup(
                scope, activation,
                Effects(1, new Effect::SetShipPartMeter(meter_type, part_name_vr, value_vr)),
                part_name, stacking_group));
    }

    bool DesignsTheSame(const ShipDesign& one, const ShipDesign& two) {
        return (
            one.Name()              == two.Name() &&
            one.Description()       == two.Description() &&
            one.DesignedOnTurn()    == two.DesignedOnTurn() &&
            one.Hull()              == two.Hull() &&
            one.Parts()             == two.Parts() &&
            one.Icon()              == two.Icon() &&
            one.Model()             == two.Model()
        );
        // not checking that IDs are the same, since the purpose of this is to
        // check if a design that might be added to the universe (which doesn't
        // have an ID yet) is the same as one that has already been added
        // (which does have an ID)
    }
}

////////////////////////////////////////////////
// Free Functions                             //
////////////////////////////////////////////////
const PartTypeManager& GetPartTypeManager()
{ return PartTypeManager::GetPartTypeManager(); }

const PartType* GetPartType(const std::string& name)
{ return GetPartTypeManager().GetPartType(name); }

const HullTypeManager& GetHullTypeManager()
{ return HullTypeManager::GetHullTypeManager(); }

const HullType* GetHullType(const std::string& name)
{ return GetHullTypeManager().GetHullType(name); }

const ShipDesign* GetShipDesign(int ship_design_id)
{ return GetUniverse().GetShipDesign(ship_design_id); }


/////////////////////////////////////
// PartTypeManager                 //
/////////////////////////////////////
// static
PartTypeManager* PartTypeManager::s_instance = 0;

PartTypeManager::PartTypeManager() {
    if (s_instance)
        throw std::runtime_error("Attempted to create more than one PartTypeManager.");

    s_instance = this;

    parse::ship_parts(GetResourceDir() / "ship_parts.txt", m_parts);

    if (GetOptionsDB().Get<bool>("verbose-logging")) {
        DebugLogger() << "Part Types:";
        for (iterator it = begin(); it != end(); ++it) {
            const PartType* p = it->second;
            DebugLogger() << " ... " << p->Name() << " class: " << p->Class();
        }
    }
}

PartTypeManager::~PartTypeManager() {
    for (std::map<std::string, PartType*>::iterator it = m_parts.begin(); it != m_parts.end(); ++it) {
        delete it->second;
    }
}

const PartType* PartTypeManager::GetPartType(const std::string& name) const {
    std::map<std::string, PartType*>::const_iterator it = m_parts.find(name);
    return it != m_parts.end() ? it->second : 0;
}

const PartTypeManager& PartTypeManager::GetPartTypeManager() {
    static PartTypeManager manager;
    return manager;
}

PartTypeManager::iterator PartTypeManager::begin() const
{ return m_parts.begin(); }

PartTypeManager::iterator PartTypeManager::end() const
{ return m_parts.end(); }


namespace {
    // Looks like there are at least 3 SourceForEmpire functions lying around - one in ShipDesign, one in Tech, and one in Building
    // TODO: Eliminate duplication
    TemporaryPtr<const UniverseObject> SourceForEmpire(int empire_id) {
        const Empire* empire = GetEmpire(empire_id);
        if (!empire) {
            DebugLogger() << "SourceForEmpire: Unable to get empire with ID: " << empire_id;
            return TemporaryPtr<const UniverseObject>();
        }
        // get a source object, which is owned by the empire with the passed-in
        // empire id.  this is used in conditions to reference which empire is
        // doing the building.  Ideally this will be the capital, but any object
        // owned by the empire will work.
        TemporaryPtr<const UniverseObject> source = GetUniverseObject(empire->CapitalID());
        // no capital?  scan through all objects to find one owned by this empire
        if (!source) {
            for (ObjectMap::const_iterator<> obj_it = Objects().const_begin(); obj_it != Objects().const_end(); ++obj_it) {
                if (obj_it->OwnedBy(empire_id)) {
                    source = *obj_it;
                    break;
                }
            }
        }
        return source;
    }
}

////////////////////////////////////////////////
// PartType
////////////////////////////////////////////////
void PartType::Init(const std::vector<boost::shared_ptr<Effect::EffectsGroup> >& effects) {
    if (m_capacity != 0) {
        switch (m_class) {
        case PC_SHORT_RANGE:
        case PC_POINT_DEFENSE:
        case PC_MISSILES:
        case PC_FIGHTERS:
        case PC_COLONY:
        case PC_TROOPS:
            m_effects.push_back(IncreaseMeter(METER_CAPACITY,       m_name, m_capacity, false));
            break;
        case PC_SHIELD:
            m_effects.push_back(IncreaseMeter(METER_MAX_SHIELD,     m_capacity));
            break;
        case PC_DETECTION:
            m_effects.push_back(IncreaseMeter(METER_DETECTION,      m_capacity));
            break;
        case PC_STEALTH:
            m_effects.push_back(IncreaseMeter(METER_STEALTH,        m_capacity));
            break;
        case PC_FUEL:
            m_effects.push_back(IncreaseMeter(METER_MAX_FUEL,       m_capacity));
            break;
        case PC_ARMOUR:
            m_effects.push_back(IncreaseMeter(METER_MAX_STRUCTURE,  m_capacity));
            break;
        case PC_SPEED:
            m_effects.push_back(IncreaseMeter(METER_SPEED,          m_capacity));
            break;
        case PC_RESEARCH:
            m_effects.push_back(IncreaseMeter(METER_TARGET_RESEARCH,m_capacity));
            break;
        case PC_INDUSTRY:
            m_effects.push_back(IncreaseMeter(METER_TARGET_INDUSTRY,m_capacity));
            break;
        case PC_TRADE:
            m_effects.push_back(IncreaseMeter(METER_TARGET_TRADE,   m_capacity));
            break;
        default:
            break;
        }
    }

    for (std::vector<boost::shared_ptr<Effect::EffectsGroup> >::const_iterator
         it = effects.begin(); it != effects.end(); ++it)
    {
        (*it)->SetTopLevelContent(m_name);
        m_effects.push_back(*it);
    }
}

PartType::~PartType()
{ delete m_location; }

float PartType::Capacity() const
{ return m_capacity; }

const std::string PartType::CapacityDescription() const {
    std::string desc_string;
    float d = Capacity();

    switch (m_class) {
    case PC_FUEL:
    case PC_TROOPS:
    case PC_COLONY:
        desc_string += UserString("PART_DESC_CAPACITY");
        break;
    case PC_SHIELD:
        desc_string = UserString("PART_DESC_SHIELD_STRENGTH");
        break;
    case PC_DETECTION:
        desc_string = UserString("PART_DESC_DETECTION");
        break;
    default:
        desc_string = UserString("PART_DESC_STRENGTH");
        break;
    }
    return str(FlexibleFormat(desc_string) % d);
}

bool PartType::CanMountInSlotType(ShipSlotType slot_type) const {
    if (INVALID_SHIP_SLOT_TYPE == slot_type)
        return false;
    for (std::vector<ShipSlotType>::const_iterator it = m_mountable_slot_types.begin(); it != m_mountable_slot_types.end(); ++it)
        if (*it == slot_type)
            return true;
    return false;
}

bool PartType::ProductionCostTimeLocationInvariant() const {
    if (CHEAP_AND_FAST_SHIP_PRODUCTION)
        return true;
    if (m_production_cost && !m_production_cost->TargetInvariant())
        return false;
    if (m_production_time && !m_production_time->TargetInvariant())
        return false;
    return true;
}

float PartType::ProductionCost(int empire_id, int location_id) const {
    if (CHEAP_AND_FAST_SHIP_PRODUCTION || !m_production_cost) {
        return 1.0f;
    } else {
        if (ValueRef::ConstantExpr(m_production_cost))
            return static_cast<float>(m_production_cost->Eval());

        TemporaryPtr<UniverseObject> location = GetUniverseObject(location_id);
        if (!location)
            return 999999.9f;    // arbitrary large number

        TemporaryPtr<const UniverseObject> source = SourceForEmpire(empire_id);
        if (!source && !m_production_cost->SourceInvariant())
            return 999999.9f;

        ScriptingContext context(source, location);

        return static_cast<float>(m_production_cost->Eval(context));
    }
}

int PartType::ProductionTime(int empire_id, int location_id) const {
    if (CHEAP_AND_FAST_SHIP_PRODUCTION || !m_production_time) {
        return 1;
    } else {
        if (ValueRef::ConstantExpr(m_production_time))
            return m_production_time->Eval();

        TemporaryPtr<UniverseObject> location = GetUniverseObject(location_id);
        if (!location)
            return 9999;    // arbitrary large number

        TemporaryPtr<const UniverseObject> source = SourceForEmpire(empire_id);
        if (!source && !m_production_time->SourceInvariant())
            return 9999;

        ScriptingContext context(source, location);

        return m_production_time->Eval(context);
    }
}


////////////////////////////////////////////////
// HullType
////////////////////////////////////////////////
void HullType::Init(const std::vector<boost::shared_ptr<Effect::EffectsGroup> >& effects) {
    if (m_fuel != 0)
        m_effects.push_back(IncreaseMeter(METER_MAX_FUEL,       m_fuel));
    if (m_stealth != 0)
        m_effects.push_back(IncreaseMeter(METER_STEALTH,        m_stealth));
    if (m_structure != 0)
        m_effects.push_back(IncreaseMeter(METER_MAX_STRUCTURE,  m_structure));
    if (m_speed != 0)
        m_effects.push_back(IncreaseMeter(METER_SPEED,          m_speed));

    for (std::vector<boost::shared_ptr<Effect::EffectsGroup> >::const_iterator it = effects.begin();
         it != effects.end(); ++it)
    {
        (*it)->SetTopLevelContent(m_name);
        m_effects.push_back(*it);
    }
}

HullType::~HullType()
{ delete m_location; }

unsigned int HullType::NumSlots(ShipSlotType slot_type) const {
    unsigned int count = 0;
    for (std::vector<Slot>::const_iterator it = m_slots.begin(); it != m_slots.end(); ++it)
        if (it->type == slot_type)
            ++count;
    return count;
}


// HullType:: and PartType::ProductionCost and ProductionTime are almost identical.
// Chances are, the same is true of buildings and techs as well.
// TODO: Eliminate duplication
bool HullType::ProductionCostTimeLocationInvariant() const {
    if (CHEAP_AND_FAST_SHIP_PRODUCTION)
        return true;
    if (m_production_cost && !m_production_cost->LocalCandidateInvariant())
        return false;
    if (m_production_time && !m_production_time->LocalCandidateInvariant())
        return false;
    return true;
}

float HullType::ProductionCost(int empire_id, int location_id) const {
    if (CHEAP_AND_FAST_SHIP_PRODUCTION || !m_production_cost) {
        return 1.0f;
    } else {
        if (ValueRef::ConstantExpr(m_production_cost))
            return static_cast<float>(m_production_cost->Eval());

        TemporaryPtr<UniverseObject> location = GetUniverseObject(location_id);
        if (!location)
            return 999999.9f;    // arbitrary large number

        TemporaryPtr<const UniverseObject> source = SourceForEmpire(empire_id);
        if (!source && !m_production_cost->SourceInvariant())
            return 999999.9f;

        ScriptingContext context(source, location);

        return static_cast<float>(m_production_cost->Eval(context));
    }
}

int HullType::ProductionTime(int empire_id, int location_id) const {
    if (CHEAP_AND_FAST_SHIP_PRODUCTION || !m_production_time) {
        return 1;
    } else {
        if (ValueRef::ConstantExpr(m_production_time))
            return m_production_time->Eval();

        TemporaryPtr<UniverseObject> location = GetUniverseObject(location_id);
        if (!location)
            return 9999;    // arbitrary large number

        TemporaryPtr<const UniverseObject> source = SourceForEmpire(empire_id);
        if (!source && !m_production_time->SourceInvariant())
            return 999999;

        ScriptingContext context(source, location);

        return m_production_time->Eval(context);
    }
}


/////////////////////////////////////
// HullTypeManager                 //
/////////////////////////////////////
// static
HullTypeManager* HullTypeManager::s_instance = 0;

HullTypeManager::HullTypeManager() {
    if (s_instance)
        throw std::runtime_error("Attempted to create more than one HullTypeManager.");

    s_instance = this;

    parse::ship_hulls(GetResourceDir() / "ship_hulls.txt", m_hulls);

    if (GetOptionsDB().Get<bool>("verbose-logging")) {
        DebugLogger() << "Hull Types:";
        for (iterator it = begin(); it != end(); ++it) {
            const HullType* h = it->second;
            DebugLogger() << " ... " << h->Name();
        }
    }
}

HullTypeManager::~HullTypeManager() {
    for (std::map<std::string, HullType*>::iterator it = m_hulls.begin(); it != m_hulls.end(); ++it) {
        delete it->second;
    }
}

const HullType* HullTypeManager::GetHullType(const std::string& name) const {
    std::map<std::string, HullType*>::const_iterator it = m_hulls.find(name);
    return it != m_hulls.end() ? it->second : 0;
}

const HullTypeManager& HullTypeManager::GetHullTypeManager() {
    static HullTypeManager manager;
    return manager;
}

HullTypeManager::iterator HullTypeManager::begin() const
{ return m_hulls.begin(); }

HullTypeManager::iterator HullTypeManager::end() const
{ return m_hulls.end(); }


////////////////////////////////////////////////
// ShipDesign
////////////////////////////////////////////////
// static(s)
const int       ShipDesign::INVALID_DESIGN_ID = -1;
const int       ShipDesign::MAX_ID            = 2000000000;

ShipDesign::ShipDesign() :
    m_id(INVALID_OBJECT_ID),
    m_name(),
    m_designed_on_turn(UniverseObject::INVALID_OBJECT_AGE),
    m_hull(),
    m_parts(),
    m_is_monster(false),
    m_icon(),
    m_3D_model(),
    m_name_desc_in_stringtable(false),
    m_is_armed(false),
    m_can_bombard(false),
    m_detection(0.0),
    m_colony_capacity(0.0),
    m_troop_capacity(0.0),
    m_stealth(0.0),
    m_fuel(0.0),
    m_shields(0.0),
    m_structure(0.0),
    m_speed(0.0),
    m_research_generation(0.0),
    m_industry_generation(0.0),
    m_trade_generation(0.0),
    m_is_production_location(false)
{}

ShipDesign::ShipDesign(const std::string& name, const std::string& description,
                       int designed_on_turn, int designed_by_empire, const std::string& hull,
                       const std::vector<std::string>& parts,
                       const std::string& icon, const std::string& model,
                       bool name_desc_in_stringtable, bool monster) :
    m_id(INVALID_OBJECT_ID),
    m_name(name),
    m_description(description),
    m_designed_on_turn(designed_on_turn),
    m_designed_by_empire(designed_by_empire),
    m_hull(hull),
    m_parts(parts),
    m_is_monster(monster),
    m_icon(icon),
    m_3D_model(model),
    m_name_desc_in_stringtable(name_desc_in_stringtable),
    m_is_armed(false),
    m_can_bombard(false),
    m_detection(0.0),
    m_colony_capacity(0.0),
    m_troop_capacity(0.0),
    m_stealth(0.0),
    m_fuel(0.0),
    m_shields(0.0),
    m_structure(0.0),
    m_speed(0.0),
    m_research_generation(0.0),
    m_industry_generation(0.0),
    m_trade_generation(0.0),
    m_is_production_location(false)
{
    // expand parts list to have empty values if fewer parts are given than hull has slots
    if (const HullType* hull_type = GetHullType(m_hull)) {
        if (m_parts.size() < hull_type->NumSlots())
            m_parts.resize(hull_type->NumSlots(), "");
    }

    if (!ValidDesign(m_hull, m_parts)) {
        ErrorLogger() << "constructing an invalid ShipDesign!";
        ErrorLogger() << Dump();
    }
    BuildStatCaches();
}

const std::string& ShipDesign::Name(bool stringtable_lookup /* = true */) const {
    if (m_name_desc_in_stringtable && stringtable_lookup)
        return UserString(m_name);
    else
        return m_name;
}

void ShipDesign::SetName(const std::string& name) {
    if (m_name != "") {
        m_name = name;
    }
}

const std::string& ShipDesign::Description(bool stringtable_lookup /* = true */) const {
    if (m_name_desc_in_stringtable && stringtable_lookup)
        return UserString(m_description);
    else
        return m_description;
}

void ShipDesign::SetDescription(const std::string& description) {
    if (m_description != "") {
        m_description = description;
    }
}

bool ShipDesign::ProductionCostTimeLocationInvariant() const {
    if (CHEAP_AND_FAST_SHIP_PRODUCTION)
        return true;
    // as seen in ShipDesign::ProductionCost, the location is passed as the
    // local candidate in the ScriptingContext

    // check hull and all parts
    if (const HullType* hull = GetHullType(m_hull))
        if (!hull->ProductionCostTimeLocationInvariant())
            return false;
    for (std::vector<std::string>::const_iterator it = m_parts.begin(); it != m_parts.end(); ++it)
        if (const PartType* part = GetPartType(*it))
            if (!part->ProductionCostTimeLocationInvariant())
                return false;

    // if hull and all parts are invariant, so is whole design
    return true;
}

float ShipDesign::ProductionCost(int empire_id, int location_id) const {
    if (CHEAP_AND_FAST_SHIP_PRODUCTION) {
        return 1.0f;
    } else {
        float cost_accumulator = 0.0f;
        if (const HullType* hull = GetHullType(m_hull))
            cost_accumulator += hull->ProductionCost(empire_id, location_id);
        for (std::vector<std::string>::const_iterator it = m_parts.begin(); it != m_parts.end(); ++it)
            if (const PartType* part = GetPartType(*it))
                cost_accumulator += part->ProductionCost(empire_id, location_id);
        return std::max(0.0f, cost_accumulator);
    }
}

float ShipDesign::PerTurnCost(int empire_id, int location_id) const
{ return ProductionCost(empire_id, location_id) / std::max(1, ProductionTime(empire_id, location_id)); }

int ShipDesign::ProductionTime(int empire_id, int location_id) const {
    if (CHEAP_AND_FAST_SHIP_PRODUCTION) {
        return 1;
    } else {
        int time_accumulator = 1;
        if (const HullType* hull = GetHullType(m_hull))
            time_accumulator = std::max(time_accumulator, hull->ProductionTime(empire_id, location_id));
        for (std::vector<std::string>::const_iterator it = m_parts.begin(); it != m_parts.end(); ++it)
            if (const PartType* part = GetPartType(*it))
                time_accumulator = std::max(time_accumulator, part->ProductionTime(empire_id, location_id));
        return std::max(1, time_accumulator);
    }
}

bool ShipDesign::CanColonize() const {
    for (std::vector<std::string>::const_iterator it = m_parts.begin(); it != m_parts.end(); ++it) {
        const std::string& part_name = *it;
        if (part_name.empty())
            continue;
        if (const PartType* part = GetPartType(part_name))
            if (part->Class() == PC_COLONY)
                return true;
    }
    return false;
}

//// TEMPORARY
float ShipDesign::Defense() const {
    // accumulate defense from defensive parts in design.
    float total_defense = 0.0f;
    const PartTypeManager& part_manager = GetPartTypeManager();
    std::vector<std::string> all_parts = Parts();
    for (std::vector<std::string>::const_iterator it = all_parts.begin(); it != all_parts.end(); ++it) {
        const PartType* part = part_manager.GetPartType(*it);
        if (part && (part->Class() == PC_SHIELD || part->Class() == PC_ARMOUR))
            total_defense += part->Capacity();
    }
    return total_defense;
}

float ShipDesign::Attack() const {
    // total damage against a target with the no shield.
    return AdjustedAttack(0.0f);
}

float ShipDesign::AdjustedAttack(float shield) const {
    // total damage against a target with the given shield.
    // accumulate attack stat from all weapon parts in design
    const PartTypeManager& manager = GetPartTypeManager();

    float total_attack = 0.0f;
    std::vector<std::string> all_parts = Parts();
    for (std::vector<std::string>::const_iterator it = all_parts.begin(); it != all_parts.end(); ++it) {
        const PartType* part = manager.GetPartType(*it);
        if (part && (part->Class() == PC_SHORT_RANGE || part->Class() == PC_POINT_DEFENSE ||
                     part->Class() == PC_MISSILES    || part->Class() == PC_FIGHTERS))
        { total_attack += std::max(0.0f, static_cast<float>(part->Capacity()) - shield); }
    }
    return total_attack;
}
//// END TEMPORARY

std::vector<std::string> ShipDesign::Parts(ShipSlotType slot_type) const {
    std::vector<std::string> retval;

    const HullType* hull = GetHull();
    assert(hull);
    const std::vector<HullType::Slot>& slots = hull->Slots();

    unsigned int size = m_parts.size();
    assert(size == hull->NumSlots());

    // add to output vector each part that is in a slot of the indicated ShipSlotType 
    for (unsigned int i = 0; i < size; ++i)
        if (slots[i].type == slot_type)
            retval.push_back(m_parts[i]);

    return retval;
}

std::vector<std::string> ShipDesign::Weapons() const {
    std::vector<std::string> retval;
    retval.reserve(m_parts.size());
    for (unsigned int i = 0; i < m_parts.size(); ++i) {
        const std::string& part_name = m_parts[i];
        const PartType* part = GetPartType(part_name);
        if (!part)
            continue;
        ShipPartClass part_class = part->Class();
        if (part_class == PC_SHORT_RANGE    || part_class == PC_MISSILES ||
            part_class == PC_FIGHTERS       || part_class == PC_POINT_DEFENSE)
        { retval.push_back(part_name); }
    }
    return retval;
}

bool ShipDesign::ProductionLocation(int empire_id, int location_id) const {
    TemporaryPtr<const UniverseObject> location = GetUniverseObject(location_id);
    if (!location)
        return false;

    // currently ships can only be built at planets, and by species that are
    // not planetbound
    TemporaryPtr<const Planet> planet = boost::dynamic_pointer_cast<const Planet>(location);
    if (!planet)
        return false;
    const std::string& species_name = planet->SpeciesName();
    if (species_name.empty())
        return false;
    const Species* species = GetSpecies(species_name);
    if (!species)
        return false;
    if (!species->CanProduceShips())
        return false;
    // also, species that can't colonize can't produce colony ships
    if (this->CanColonize() && !species->CanColonize())
        return false;

    Empire* empire = GetEmpire(empire_id);
    if (!empire) {
        DebugLogger() << "ShipDesign::ProductionLocation: Unable to get pointer to empire " << empire_id;
        return false;
    }

    // get a source object, which is owned by the empire with the passed-in
    // empire id.  this is used in conditions to reference which empire is
    // doing the producing.  Ideally this will be the capital, but any object
    // owned by the empire will work.
    TemporaryPtr<const UniverseObject> source = SourceForEmpire(empire_id);
    // if this empire doesn't own ANYTHING, then how is it producing anyway?
    if (!source)
        return false;

    // apply hull location conditions to potential location
    const HullType* hull = GetHull();
    if (!hull) {
        ErrorLogger() << "ShipDesign::ProductionLocation  ShipDesign couldn't get its own hull with name " << m_hull;
        return false;
    }
    if (!hull->Location()->Eval(ScriptingContext(source), location))
        return false;

    // apply external and internal parts' location conditions to potential location
    for (std::vector<std::string>::const_iterator part_it = m_parts.begin(); part_it != m_parts.end(); ++part_it) {
        std::string part_name = *part_it;
        if (part_name.empty())
            continue;       // empty slots don't limit build location

        const PartType* part = GetPartType(part_name);
        if (!part) {
            ErrorLogger() << "ShipDesign::ProductionLocation  ShipDesign couldn't get part with name " << part_name;
            return false;
        }
        if (!part->Location()->Eval(ScriptingContext(source), location))
            return false;
    }
    // location matched all hull and part conditions, so is a valid build location
    return true;
}

void ShipDesign::SetID(int id)
{ m_id = id; }

bool ShipDesign::ValidDesign(const std::string& hull, const std::vector<std::string>& parts) {
    // ensure hull type exists and has at least enough slots for passed parts
    const HullType* hull_type = GetHullTypeManager().GetHullType(hull);
    if (!hull_type) {
        DebugLogger() << "ShipDesign::ValidDesign: hull not found: " << hull;
        return false;
    }

    unsigned int size = parts.size();
    if (size > hull_type->NumSlots()) {
        DebugLogger() << "ShipDesign::ValidDesign: given " << size << " parts for hull with " << hull_type->NumSlots() << " slots";
        return false;
    }

    const std::vector<HullType::Slot>& slots = hull_type->Slots();

    // ensure all passed parts can be mounted in slots of type they were passed for
    const PartTypeManager& part_manager = GetPartTypeManager();
    for (unsigned int i = 0; i < size; ++i) {
        const std::string& part_name = parts[i];
        if (part_name.empty())
            continue;   // if part slot is empty, ignore - doesn't invalidate design

        const PartType* part = part_manager.GetPartType(part_name);
        if (!part) {
            DebugLogger() << "ShipDesign::ValidDesign: part not found: " << part_name;
            return false;
        }

        // verify part can mount in indicated slot
        ShipSlotType slot_type = slots[i].type;
        if (!(part->CanMountInSlotType(slot_type))) {
            DebugLogger() << "ShipDesign::ValidDesign: part " << part_name << " can't be mounted in " << boost::lexical_cast<std::string>(slot_type) << " slot";
            return false;
        }
    }

    return true;
}

void ShipDesign::BuildStatCaches() {
    const HullType* hull = GetHullType(m_hull);
    if (!hull) {
        ErrorLogger() << "ShipDesign::BuildStatCaches couldn't get hull with name " << m_hull;
        return;
    }

    m_producible =      hull->Producible();
    m_detection =       hull->Detection();
    m_colony_capacity = hull->ColonyCapacity();
    m_troop_capacity =  hull->TroopCapacity();
    m_stealth =         hull->Stealth();
    m_fuel =            hull->Fuel();
    m_shields =         hull->Shields();
    m_structure =       hull->Structure();
    m_speed =           hull->Speed();

    for (std::vector<std::string>::const_iterator it = m_parts.begin(); it != m_parts.end(); ++it) {
        if (it->empty())
            continue;

        const PartType* part = GetPartType(*it);
        if (!part) {
            ErrorLogger() << "ShipDesign::BuildStatCaches couldn't get part with name " << *it;
            continue;
        }

        if (!part->Producible())
            m_producible = false;

        switch (part->Class()) {
        case PC_SHORT_RANGE:
        case PC_MISSILES:
        case PC_FIGHTERS:
        case PC_POINT_DEFENSE: {
            m_is_armed = true;
            break;
        }
        case PC_COLONY:
            m_colony_capacity += part->Capacity();
            break;
        case PC_TROOPS:
            m_troop_capacity += part->Capacity();
            break;
        case PC_STEALTH:
            m_stealth += part->Capacity();
            break;
        case PC_SPEED:
            m_speed += part->Capacity();
            break;
        case PC_SHIELD:
            m_shields += part->Capacity();
            break;
        case PC_FUEL:
            m_fuel += part->Capacity();
            break;
        case PC_ARMOUR:
            m_structure += part->Capacity();
            break;
        case PC_DETECTION:
            m_detection += part->Capacity();
            break;
        case PC_BOMBARD:
            m_can_bombard = true;
            break;
        case PC_RESEARCH:
            m_research_generation += part->Capacity();
            break;
        case PC_INDUSTRY:
            m_industry_generation += part->Capacity();
            break;
        case PC_TRADE:
            m_trade_generation += part->Capacity();
            break;
        case PC_PRODICTION_LOCATION:
            m_is_production_location = true;
            break;
        default:
            break;
        }
    }
}

std::string ShipDesign::Dump() const {
    std::string retval = DumpIndent() + "ShipDesign\n";
    ++g_indent;
    retval += DumpIndent() + "name = \"" + m_name + "\"\n";
    retval += DumpIndent() + "description = \"" + m_description + "\"\n";
    std::cout << "ShipDesign::Dump: m_name_desc_in_stringtable: " << m_name_desc_in_stringtable << std::endl;
    if (!m_name_desc_in_stringtable)
        retval += DumpIndent() + "NoStringtableLookup\n";
    retval += DumpIndent() + "hull = \"" + m_hull + "\"\n";
    retval += DumpIndent() + "parts = ";
    if (m_parts.empty()) {
        retval += "[]\n";
    } else if (m_parts.size() == 1) {
        retval += "\"" + *m_parts.begin() + "\"\n";
    } else {
        retval += "[\n";
        ++g_indent;
        for (std::vector<std::string>::const_iterator it = m_parts.begin(); it != m_parts.end(); ++it) {
            retval += DumpIndent() + "\"" + *it + "\"\n";
        }
        --g_indent;
        retval += DumpIndent() + "]\n";
    }
    if (!m_icon.empty())
        retval += DumpIndent() + "icon = \"" + m_icon + "\"\n";
    retval += DumpIndent() + "model = \"" + m_3D_model + "\"\n";
    --g_indent;
    return retval; 
}

bool operator ==(const ShipDesign& first, const ShipDesign& second) {
    if (first.Hull() != second.Hull())
        return false;

    std::map<std::string, int> first_parts;
    std::map<std::string, int> second_parts;

    for (std::vector<std::string>::const_iterator it = first.Parts().begin();
         it != first.Parts().end(); ++it)
    { ++first_parts[*it]; }

    for (std::vector<std::string>::const_iterator it = second.Parts().begin();
         it != second.Parts().end(); ++it)
    { ++second_parts[*it]; }

    return first_parts == second_parts;
}


/////////////////////////////////////
// PredefinedShipDesignManager     //
/////////////////////////////////////
// static(s)
PredefinedShipDesignManager* PredefinedShipDesignManager::s_instance = 0;

PredefinedShipDesignManager::PredefinedShipDesignManager() {
    if (s_instance)
        throw std::runtime_error("Attempted to create more than one PredefinedShipDesignManager.");

    s_instance = this;

    DebugLogger() << "Initializing PredefinedShipDesignManager";

    parse::ship_designs(GetResourceDir() / "premade_ship_designs.txt", m_ship_designs);

    parse::ship_designs(GetResourceDir() / "space_monsters.txt", m_monster_designs);

    if (GetOptionsDB().Get<bool>("verbose-logging")) {
        DebugLogger() << "Predefined Ship Designs:";
        for (iterator it = begin(); it != end(); ++it) {
            const ShipDesign* d = it->second;
            DebugLogger() << " ... " << d->Name();
        }
        DebugLogger() << "Monster Ship Designs:";
        for (iterator it = begin_monsters(); it != end_monsters(); ++it) {
            const ShipDesign* d = it->second;
            DebugLogger() << " ... " << d->Name();
        }
    }
}

PredefinedShipDesignManager::~PredefinedShipDesignManager() {
    for (std::map<std::string, ShipDesign*>::iterator it = m_ship_designs.begin(); it != m_ship_designs.end(); ++it)
        delete it->second;
}

void PredefinedShipDesignManager::AddShipDesignsToEmpire(Empire* empire,
                                                         const std::vector<std::string>& design_names) const
{
    if (!empire || design_names.empty())
        return;
    int empire_id = empire->EmpireID();
    Universe& universe = GetUniverse();

    for (std::vector<std::string>::const_iterator it = design_names.begin(); it != design_names.end(); ++it) {
        const std::string& design_name = *it;
        std::map<std::string, ShipDesign*>::const_iterator design_it = m_ship_designs.find(design_name);
        if (design_it == m_ship_designs.end()) {
            ErrorLogger() << "Couldn't find predefined ship design with name " << design_name << " to add to empire";
            continue;
        }

        // only add producible designs to empires
        const ShipDesign* d = design_it->second;
        if (!d->Producible())
            continue;

        // safety / santiy check
        if (design_it->first != d->Name(false))
            ErrorLogger() << "Predefined ship design name in map (" << design_it->first << ") doesn't match name in ShipDesign::m_name (" << d->Name(false) << ")";

        int design_id = this->GetDesignID(design_name);

        if (design_id == ShipDesign::INVALID_DESIGN_ID) {
            ErrorLogger() << "PredefinedShipDesignManager::AddShipDesignsToEmpire couldn't add a design to an empire";
            continue;
        } else {
            universe.SetEmpireKnowledgeOfShipDesign(design_id, empire_id);
            empire->AddShipDesign(design_id);
        }
    }
}

namespace {
    void AddDesignToUniverse(std::map<std::string, int>& design_generic_ids,
                             ShipDesign* design, bool monster)
    {
        if (!design)
            return;

        Universe& universe = GetUniverse();
        /* check if there already exists this same design in the universe. */
        for (Universe::ship_design_iterator it = universe.beginShipDesigns();
             it != universe.endShipDesigns(); ++it)
        {
            const ShipDesign* existing_design = it->second;
            if (!existing_design) {
                ErrorLogger() << "PredefinedShipDesignManager::AddShipDesignsToUniverse found an invalid design in the Universe";
                continue;
            }

            if (DesignsTheSame(*existing_design, *design)) {
                DebugLogger() << "PredefinedShipDesignManager::AddShipDesignsToUniverse found there already is an exact duplicate of a design to be added, so is not re-adding it";
                design_generic_ids[design->Name(false)] = existing_design->ID();
                return; // design already added; don't need to do so again
            }
        }


        // generate id for new design
        int new_design_id = GetNewDesignID();
        if (new_design_id == ShipDesign::INVALID_DESIGN_ID) {
            ErrorLogger() << "PredefinedShipDesignManager::AddShipDesignsToUniverse Unable to get new design id";
            return;
        }


        // duplicate design to add to Universe
        ShipDesign* copy = new ShipDesign(design->Name(false), design->Description(false),
                                          design->DesignedOnTurn(), design->DesignedByEmpire(),
                                          design->Hull(), design->Parts(), design->Icon(),
                                          design->Model(), design->LookupInStringtable(), monster);
        if (!copy) {
            ErrorLogger() << "PredefinedShipDesignManager::AddShipDesignsToUniverse() couldn't duplicate the design with name " << design->Name();
            return;
        }

        bool success = universe.InsertShipDesignID(copy, new_design_id);
        if (!success) {
            ErrorLogger() << "Empire::AddShipDesign Unable to add new design to universe";
            delete copy;
            return;
        }

        design_generic_ids[design->Name(false)] = new_design_id;
    };
}

const std::map<std::string, int>& PredefinedShipDesignManager::AddShipDesignsToUniverse() const {
    m_design_generic_ids.clear();   // std::map<std::string, int>

    for (iterator it = begin(); it != end(); ++it) {
        ShipDesign* d = it->second;
        AddDesignToUniverse(m_design_generic_ids, d, false);
    }

    for (iterator it = begin_monsters(); it != end_monsters(); ++it) {
        ShipDesign* d = it->second;
        AddDesignToUniverse(m_design_generic_ids, d, true);
    }

    return m_design_generic_ids;
}

PredefinedShipDesignManager& PredefinedShipDesignManager::GetPredefinedShipDesignManager() {
    static PredefinedShipDesignManager manager;
    return manager;
}

PredefinedShipDesignManager::iterator PredefinedShipDesignManager::begin() const
{ return m_ship_designs.begin(); }

PredefinedShipDesignManager::iterator PredefinedShipDesignManager::end() const
{ return m_ship_designs.end(); }

PredefinedShipDesignManager::iterator PredefinedShipDesignManager::begin_monsters() const
{ return m_monster_designs.begin(); }

PredefinedShipDesignManager::iterator PredefinedShipDesignManager::end_monsters() const
{ return m_monster_designs.end(); }

PredefinedShipDesignManager::generic_iterator PredefinedShipDesignManager::begin_generic() const
{ return m_design_generic_ids.begin(); }

PredefinedShipDesignManager::generic_iterator PredefinedShipDesignManager::end_generic() const
{ return m_design_generic_ids.end(); }

int PredefinedShipDesignManager::GetDesignID(const std::string& name) const {
    std::map<std::string, int>::const_iterator it = m_design_generic_ids.find(name);
    if (it == m_design_generic_ids.end())
        return ShipDesign::INVALID_DESIGN_ID;
    return it->second;
}

///////////////////////////////////////////////////////////
// Free Functions                                        //
///////////////////////////////////////////////////////////
const PredefinedShipDesignManager& GetPredefinedShipDesignManager()
{ return PredefinedShipDesignManager::GetPredefinedShipDesignManager(); }

const ShipDesign* GetPredefinedShipDesign(const std::string& name)
{ return GetUniverse().GetGenericShipDesign(name); }
