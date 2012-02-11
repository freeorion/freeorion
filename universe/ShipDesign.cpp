#include "ShipDesign.h"

#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../util/Directories.h"
#include "../parse/Parse.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "Condition.h"
#include "Effect.h"
#include "ValueRef.h"
#include "Planet.h"
#include "Species.h"

#include <boost/filesystem/fstream.hpp>

std::string DumpIndent();

extern int g_indent;

namespace {
    const bool CHEAP_AND_FAST_SHIP_PRODUCTION = false;    // makes all ships cost 1 PP and take 1 turn to build
}

namespace {
    struct PartTypeStringVisitor :
        public boost::static_visitor<>
    {
        PartTypeStringVisitor(std::string& str) :
            m_str(str)
        {}
        void operator()(const double& d) const
        { m_str = "capacity stat"; }
        void operator()(const DirectFireStats& stats) const
        { m_str = "direct-fire weapon stats"; }
        void operator()(const LRStats& stats) const
        { m_str = "long-range weapon stats"; }
        void operator()(const FighterStats& stats) const
        { m_str = "fighter bay stats"; }
        std::string& m_str;
    };

    std::string PartTypeStatsString(const PartTypeStats& stats) {
        std::string retval;
        boost::apply_visitor(PartTypeStringVisitor(retval), stats);
        return retval;
    }

    boost::shared_ptr<const Effect::EffectsGroup>
    IncreaseMeter(MeterType meter_type, double increase) {
        typedef boost::shared_ptr<const Effect::EffectsGroup> EffectsGroupPtr;
        typedef std::vector<Effect::EffectBase*> Effects;
        Condition::Source* scope = new Condition::Source;
        Condition::Source* activation = new Condition::Source;
        ValueRef::ValueRefBase<double>* vr =
            new ValueRef::Operation<double>(
                ValueRef::PLUS,
                new ValueRef::Variable<double>(std::vector<adobe::name_t>(1, Value_name)),
                new ValueRef::Constant<double>(increase)
            );
        return EffectsGroupPtr(
            new Effect::EffectsGroup(
                scope, activation, Effects(1, new Effect::SetMeter(meter_type, vr))));
    }

    boost::shared_ptr<const Effect::EffectsGroup>
    IncreaseMeter(MeterType meter_type, const std::string& part_name, double increase) {
        typedef boost::shared_ptr<const Effect::EffectsGroup> EffectsGroupPtr;
        typedef std::vector<Effect::EffectBase*> Effects;
        Condition::Source* scope = new Condition::Source;
        Condition::Source* activation = new Condition::Source;
        ValueRef::ValueRefBase<double>* vr =
            new ValueRef::Operation<double>(
                ValueRef::PLUS,
                new ValueRef::Variable<double>(std::vector<adobe::name_t>(1, Value_name)),
                new ValueRef::Constant<double>(increase)
            );
        return EffectsGroupPtr(
            new Effect::EffectsGroup(
                scope, activation, Effects(1, new Effect::SetShipPartMeter(meter_type, part_name, vr)), part_name));
    }

    struct DescriptionVisitor : public boost::static_visitor<>
    {
        DescriptionVisitor(ShipPartClass part_class, std::string& description) :
            m_class(part_class),
            m_description(description)
        {}
        void operator()(const double& d) const {
            std::string desc_string =
                m_class == PC_FUEL || m_class == PC_COLONY ?
                "PART_DESC_CAPACITY" : "PART_DESC_STRENGTH";
            m_description +=
                str(FlexibleFormat(UserString(desc_string)) % d);
        }
        void operator()(const DirectFireStats& stats) const {
            m_description +=
                str(FlexibleFormat(UserString("PART_DESC_DIRECT_FIRE_STATS"))
                    % stats.m_damage
                    % stats.m_ROF
                    % stats.m_range);
        }
        void operator()(const LRStats& stats) const {
            m_description +=
                str(FlexibleFormat(UserString("PART_DESC_LR_STATS"))
                    % stats.m_damage
                    % stats.m_ROF
                    % stats.m_range
                    % stats.m_speed
                    % stats.m_structure
                    % stats.m_stealth
                    % stats.m_capacity);
        }
        void operator()(const FighterStats& stats) const {
            m_description +=
                str(FlexibleFormat(UserString("PART_DESC_FIGHTER_STATS"))
                    % UserString(stats.m_type == BOMBER ? "BOMBER" : "INTERCEPTOR")
                    % stats.m_anti_ship_damage
                    % stats.m_anti_fighter_damage
                    % stats.m_launch_rate
                    % stats.m_speed
                    % stats.m_stealth
                    % stats.m_structure
                    % stats.m_detection
                    % stats.m_capacity);
        }

        const ShipPartClass m_class;
        std::string& m_description;
    };

    bool DesignsTheSame(const ShipDesign& one, const ShipDesign& two) {
        return (
            one.Name()              == two.Name() &&
            one.Description()       == two.Description() &&
            one.DesignedByEmpire()  == two.DesignedByEmpire() &&
            one.DesignedOnTurn()    == two.DesignedOnTurn() &&
            one.Hull()              == two.Hull() &&
            one.Parts()             == two.Parts() &&
            one.Graphic()           == two.Graphic() &&
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
        Logger().debugStream() << "Part Types:";
        for (iterator it = begin(); it != end(); ++it) {
            const PartType* p = it->second;
            Logger().debugStream() << " ... " << p->Name() << " class: " << p->Class() << " stats: " << p->StatDescription();
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


////////////////////////////////////////////////
// PartType stat variant types                //
////////////////////////////////////////////////
const double DirectFireStats::PD_SELF_DEFENSE_FACTOR = 2.0 / 3.0;

DirectFireStats::DirectFireStats() :
    m_damage(),
    m_ROF(),
    m_range()
{}

DirectFireStats::DirectFireStats(double damage,
                                 double ROF,
                                 double range) :
    m_damage(damage),
    m_ROF(ROF),
    m_range(range)
{}

LRStats::LRStats() :
    m_damage(),
    m_ROF(),
    m_range(),
    m_speed(),
    m_stealth(),
    m_structure()
{}

LRStats::LRStats(double damage,
                 double ROF,
                 double range,
                 double speed,
                 double stealth,
                 double structure,
                 int capacity) :
    m_damage(damage),
    m_ROF(ROF),
    m_range(range),
    m_speed(speed),
    m_stealth(stealth),
    m_structure(structure),
    m_capacity(capacity)
{
    if (m_capacity < 0)
        throw std::runtime_error("Attempted to create a LRStats with a "
                                 "nonpositive capacity.");
}

FighterStats::FighterStats() :
    m_type(),
    m_anti_ship_damage(),
    m_anti_fighter_damage(),
    m_launch_rate(),
    m_fighter_weapon_range(),
    m_speed(),
    m_stealth(),
    m_structure(),
    m_detection(),
    m_capacity()
{}

FighterStats::FighterStats(CombatFighterType type,
                           double anti_ship_damage,
                           double anti_fighter_damage,
                           double launch_rate,
                           double fighter_weapon_range,
                           double speed,
                           double stealth,
                           double structure,
                           double detection,
                           int capacity) :
    m_type(type),
    m_anti_ship_damage(anti_ship_damage),
    m_anti_fighter_damage(anti_fighter_damage),
    m_launch_rate(launch_rate),
    m_fighter_weapon_range(fighter_weapon_range),
    m_speed(speed),
    m_stealth(stealth),
    m_structure(structure),
    m_detection(detection),
    m_capacity(capacity)
{
    if (type == INTERCEPTOR && m_anti_fighter_damage < m_anti_ship_damage)
        Logger().debugStream() << "Creating an INTERCEPTOR FighterStats with weaker anti-fighter stat than anti-ship stat.";
    if (type == BOMBER && m_anti_ship_damage < m_anti_fighter_damage)
        Logger().debugStream() << "Creating a BOMBER FighterStats with weaker anti-ship stat than anti-fighter stat.";
    if (m_capacity < 0) {
        m_capacity = 0;
        Logger().debugStream() << "Creating a FighterStats with a nonpositive capacity.";
    }
}


////////////////////////////////////////////////
// PartType
////////////////////////////////////////////////
void PartType::Init(const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects) {
    switch (m_class) {
    case PC_SHORT_RANGE:
    case PC_POINT_DEFENSE:
        if (!boost::get<DirectFireStats>(&m_stats)) {
            std::string type_name = m_class == PC_SHORT_RANGE?
                "PC_SHORT_RANGE" : "PC_POINT_DEFENSE";
            throw std::runtime_error("PartType::PartType() : Wrong kind of stats specified "
                                     "for " + type_name + " part \"" + m_name + "\" -- "
                                     "was " + PartTypeStatsString(m_stats) + "; should have "
                                     "been " + PartTypeStatsString(DirectFireStats()));
        }
        break;
    case PC_MISSILES:
        if (!boost::get<LRStats>(&m_stats)) {
            throw std::runtime_error("PartType::PartType() : Wrong kind of stats specified "
                                     "for PC_MISSILES part \"" + m_name + "\" -- "
                                     "was " + PartTypeStatsString(m_stats) + "; should have "
                                     "been " + PartTypeStatsString(LRStats()));
        }
        break;
    case PC_FIGHTERS:
        if (!boost::get<FighterStats>(&m_stats)) {
            throw std::runtime_error("PartType::PartType() : Wrong kind of stats specified "
                                     "for PC_FIGHTERS part \"" + m_name + "\" -- "
                                     "was " + PartTypeStatsString(m_stats) + "; should have "
                                     "been " + PartTypeStatsString(FighterStats()));
        }
        break;
    default:
        if (!boost::get<double>(&m_stats)) {
            throw std::runtime_error("PartType::PartType() : Wrong kind of stats specified "
                                     "for generic part \"" + m_name + "\" -- "
                                     "was " + PartTypeStatsString(m_stats) + "; should have "
                                     "been " + PartTypeStatsString(double()));
        }
        break;
    }

    switch (m_class) {
    case PC_SHORT_RANGE:
    case PC_POINT_DEFENSE: {
        const DirectFireStats& stats = boost::get<DirectFireStats>(m_stats);
        m_effects.push_back(IncreaseMeter(METER_DAMAGE,                 m_name, stats.m_damage));
        m_effects.push_back(IncreaseMeter(METER_ROF,                    m_name, stats.m_ROF));
        m_effects.push_back(IncreaseMeter(METER_RANGE,                  m_name, stats.m_range));
        break;
    }
    case PC_MISSILES: {
        const LRStats& stats = boost::get<LRStats>(m_stats);
        m_effects.push_back(IncreaseMeter(METER_DAMAGE,                 m_name, stats.m_damage));
        m_effects.push_back(IncreaseMeter(METER_ROF,                    m_name, stats.m_ROF));
        m_effects.push_back(IncreaseMeter(METER_RANGE,                  m_name, stats.m_range));
        m_effects.push_back(IncreaseMeter(METER_SPEED,                  m_name, stats.m_speed));
        m_effects.push_back(IncreaseMeter(METER_STEALTH,                m_name, stats.m_stealth));
        m_effects.push_back(IncreaseMeter(METER_STRUCTURE,              m_name, stats.m_structure));
        m_effects.push_back(IncreaseMeter(METER_CAPACITY,               m_name, stats.m_capacity));
        break;
    }
    case PC_FIGHTERS: {
        const FighterStats& stats = boost::get<FighterStats>(m_stats);
        m_effects.push_back(IncreaseMeter(METER_ANTI_SHIP_DAMAGE,       m_name, stats.m_anti_ship_damage));
        m_effects.push_back(IncreaseMeter(METER_ANTI_FIGHTER_DAMAGE,    m_name, stats.m_anti_fighter_damage));
        m_effects.push_back(IncreaseMeter(METER_LAUNCH_RATE,            m_name, stats.m_launch_rate));
        m_effects.push_back(IncreaseMeter(METER_FIGHTER_WEAPON_RANGE,   m_name, stats.m_fighter_weapon_range));
        m_effects.push_back(IncreaseMeter(METER_SPEED,                  m_name, stats.m_speed));
        m_effects.push_back(IncreaseMeter(METER_STEALTH,                m_name, stats.m_stealth));
        m_effects.push_back(IncreaseMeter(METER_STRUCTURE,              m_name, stats.m_structure));    // not METER_MAX_STRUCTURE because this stat sets the initial/max structure of battle-spawned missiles, but doesn't need to track a separate max and actual value itself
        m_effects.push_back(IncreaseMeter(METER_DETECTION,              m_name, stats.m_detection));
        m_effects.push_back(IncreaseMeter(METER_CAPACITY,               m_name, stats.m_capacity));
        break;
    }
    case PC_SHIELD:
        if (boost::get<double>(m_stats) != 0)
            m_effects.push_back(IncreaseMeter(METER_MAX_SHIELD,     boost::get<double>(m_stats)));
        break;
    case PC_DETECTION:
        if (boost::get<double>(m_stats) != 0)
            m_effects.push_back(IncreaseMeter(METER_DETECTION,      boost::get<double>(m_stats)));
        break;
    case PC_STEALTH:
        if (boost::get<double>(m_stats) != 0)
            m_effects.push_back(IncreaseMeter(METER_STEALTH,        boost::get<double>(m_stats)));
        break;
    case PC_FUEL:
        if (boost::get<double>(m_stats) != 0)
            m_effects.push_back(IncreaseMeter(METER_MAX_FUEL,       boost::get<double>(m_stats)));
        break;
    case PC_ARMOUR:
        if (boost::get<double>(m_stats) != 0)
            m_effects.push_back(IncreaseMeter(METER_MAX_STRUCTURE,  boost::get<double>(m_stats)));
        break;
    case PC_BATTLE_SPEED:
        if (boost::get<double>(m_stats) != 0)
            m_effects.push_back(IncreaseMeter(METER_BATTLE_SPEED,   boost::get<double>(m_stats)));
        break;
    case PC_STARLANE_SPEED:
        if (boost::get<double>(m_stats) != 0)
            m_effects.push_back(IncreaseMeter(METER_STARLANE_SPEED, boost::get<double>(m_stats)));
    default:
        break;
    }

    for (std::vector<boost::shared_ptr<const Effect::EffectsGroup> >::const_iterator it = effects.begin(); it != effects.end(); ++it)
        m_effects.push_back(*it);
}

PartType::~PartType()
{ delete m_location; }

std::string PartType::StatDescription() const {
    std::string retval;
    boost::apply_visitor(DescriptionVisitor(m_class, retval), m_stats);
    return retval;
}

bool PartType::CanMountInSlotType(ShipSlotType slot_type) const {
    if (INVALID_SHIP_SLOT_TYPE == slot_type)
        return false;
    for (std::vector<ShipSlotType>::const_iterator it = m_mountable_slot_types.begin(); it != m_mountable_slot_types.end(); ++it)
        if (*it == slot_type)
            return true;
    return false;
}


////////////////////////////////////////////////
// HullType stats                             //
////////////////////////////////////////////////
HullTypeStats::HullTypeStats() :
    m_fuel(0.0),
    m_battle_speed(0.0),
    m_starlane_speed(0.0),
    m_stealth(0.0),
    m_structure(0.0)
{}

HullTypeStats::HullTypeStats(double fuel, double battle_speed, double starlane_speed,
                             double stealth, double structure) :
    m_fuel(fuel),
    m_battle_speed(battle_speed),
    m_starlane_speed(starlane_speed),
    m_stealth(stealth),
    m_structure(structure)
{}


////////////////////////////////////////////////
// HullType
////////////////////////////////////////////////
HullType::HullType(const std::string& name, const std::string& description,
                   double fuel, double battle_speed, double starlane_speed,
                   double stealth, double structure,
                   double production_cost, int production_time, bool producible,
                   const std::vector<Slot>& slots, const Condition::ConditionBase* location,
                   const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects,
                   const std::string& graphic) :
    m_name(name),
    m_description(description),
    m_battle_speed(battle_speed),
    m_starlane_speed(starlane_speed),
    m_fuel(fuel),
    m_stealth(stealth),
    m_structure(structure),
    m_production_cost(production_cost),
    m_production_time(production_time),
    m_producible(producible),
    m_slots(slots),
    m_location(location),
    m_effects(),
    m_graphic(graphic)
{
    if (m_fuel != 0)
        m_effects.push_back(IncreaseMeter(METER_MAX_FUEL,       m_fuel));
    if (m_stealth != 0)
        m_effects.push_back(IncreaseMeter(METER_STEALTH,        m_stealth));
    if (m_structure != 0)
        m_effects.push_back(IncreaseMeter(METER_MAX_STRUCTURE,  m_structure));
    if (m_battle_speed != 0)
        m_effects.push_back(IncreaseMeter(METER_BATTLE_SPEED,   m_battle_speed));
    if (m_starlane_speed != 0)
        m_effects.push_back(IncreaseMeter(METER_STARLANE_SPEED, m_starlane_speed));

    for (std::vector<boost::shared_ptr<const Effect::EffectsGroup> >::const_iterator it = effects.begin(); it != effects.end(); ++it)
        m_effects.push_back(*it);
}

HullType::HullType(const std::string& name, const std::string& description,
                   const HullTypeStats& stats,
                   double production_cost, int production_time, bool producible,
                   const std::vector<Slot>& slots, const Condition::ConditionBase* location,
                   const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects,
                   const std::string& graphic) :
    m_name(name),
    m_description(description),
    m_battle_speed(stats.m_battle_speed),
    m_starlane_speed(stats.m_starlane_speed),
    m_fuel(stats.m_fuel),
    m_stealth(stats.m_stealth),
    m_structure(stats.m_structure),
    m_production_cost(production_cost),
    m_production_time(production_time),
    m_producible(producible),
    m_slots(slots),
    m_location(location),
    m_effects(),
    m_graphic(graphic)
{
    if (m_fuel != 0)
        m_effects.push_back(IncreaseMeter(METER_MAX_FUEL,       m_fuel));
    if (m_stealth != 0)
        m_effects.push_back(IncreaseMeter(METER_STEALTH,        m_stealth));
    if (m_structure != 0)
        m_effects.push_back(IncreaseMeter(METER_MAX_STRUCTURE,  m_structure));
    if (m_battle_speed != 0)
        m_effects.push_back(IncreaseMeter(METER_BATTLE_SPEED,   m_battle_speed));
    if (m_starlane_speed != 0)
        m_effects.push_back(IncreaseMeter(METER_STARLANE_SPEED, m_starlane_speed));

    for (std::vector<boost::shared_ptr<const Effect::EffectsGroup> >::const_iterator it = effects.begin(); it != effects.end(); ++it)
        m_effects.push_back(*it);
}

HullType::~HullType()
{ delete m_location; }

std::string HullType::StatDescription() const {
    std::string retval = 
        str(FlexibleFormat(UserString("HULL_DESC"))
            % m_starlane_speed
            % m_fuel
            % m_battle_speed
            % m_structure);
    return retval;
}

unsigned int HullType::NumSlots(ShipSlotType slot_type) const {
    unsigned int count = 0;
    for (std::vector<Slot>::const_iterator it = m_slots.begin(); it != m_slots.end(); ++it)
        if (it->type == slot_type)
            ++count;
    return count;
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
        Logger().debugStream() << "Hull Types:";
        for (iterator it = begin(); it != end(); ++it) {
            const HullType* h = it->second;
            Logger().debugStream() << " ... " << h->Name() << " stats: " << h->StatDescription();
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
    m_name(""),
    m_designed_by_empire_id(ALL_EMPIRES),
    m_designed_on_turn(UniverseObject::INVALID_OBJECT_AGE),
    m_hull(""),
    m_parts(),
    m_is_monster(false),
    m_graphic(""),
    m_3D_model(""),
    m_name_desc_in_stringtable(false),
    m_is_armed(false),
    m_detection(0.0),
    m_colony_capacity(0.0),
    m_troop_capacity(0.0),
    m_stealth(0.0),
    m_fuel(0.0),
    m_shields(0.0),
    m_structure(0.0),
    m_battle_speed(0.0),
    m_starlane_speed(0.0),
    m_production_cost(0.0),
    m_production_time(0),
    m_min_SR_range(DBL_MAX),
    m_max_SR_range(0.0),
    m_min_LR_range(DBL_MAX),
    m_max_LR_range(0.0),
    m_min_PD_range(DBL_MAX),
    m_max_PD_range(0.0),
    m_min_weapon_range(DBL_MAX),
    m_max_weapon_range(0.0),
    m_min_non_PD_weapon_range(DBL_MAX),
    m_max_non_PD_weapon_range(0.0)
{}

ShipDesign::ShipDesign(const std::string& name, const std::string& description, int designed_by_empire_id,
                       int designed_on_turn, const std::string& hull, const std::vector<std::string>& parts,
                       const std::string& graphic, const std::string& model,
                       bool name_desc_in_stringtable, bool monster) :
    m_id(INVALID_OBJECT_ID),
    m_name(name),
    m_description(description),
    m_designed_by_empire_id(designed_by_empire_id),
    m_designed_on_turn(designed_on_turn),
    m_hull(hull),
    m_parts(parts),
    m_is_monster(monster),
    m_graphic(graphic),
    m_3D_model(model),
    m_name_desc_in_stringtable(name_desc_in_stringtable),
    m_is_armed(false),
    m_detection(0.0),
    m_colony_capacity(0.0),
    m_troop_capacity(0.0),
    m_stealth(0.0),
    m_fuel(0.0),
    m_shields(0.0),
    m_structure(0.0),
    m_battle_speed(0.0),
    m_starlane_speed(0.0),
    m_production_cost(0.0),
    m_production_time(0),
    m_min_SR_range(DBL_MAX),
    m_max_SR_range(0.0),
    m_min_LR_range(DBL_MAX),
    m_max_LR_range(0.0),
    m_min_PD_range(DBL_MAX),
    m_max_PD_range(0.0),
    m_min_weapon_range(DBL_MAX),
    m_max_weapon_range(0.0),
    m_min_non_PD_weapon_range(DBL_MAX),
    m_max_non_PD_weapon_range(0.0)
{
    // expand parts list to have empty values if fewer parts are given than hull has slots
    if (const HullType* hull_type = GetHullType(m_hull)) {
        if (m_parts.size() < hull_type->NumSlots())
            m_parts.resize(hull_type->NumSlots(), "");
    }

    if (!ValidDesign(m_hull, m_parts)) {
        Logger().errorStream() << "constructing an invalid ShipDesign!";
        Logger().errorStream() << Dump();
    }
    BuildStatCaches();
}

const std::string& ShipDesign::Name(bool stringtable_lookup /* = true */) const {
    if (m_name_desc_in_stringtable && stringtable_lookup)
        return UserString(m_name);
    else
        return m_name;
}

const std::string& ShipDesign::Description(bool stringtable_lookup /* = true */) const {
    if (m_name_desc_in_stringtable && stringtable_lookup)
        return UserString(m_description);
    else
        return m_description;
}

double ShipDesign::ProductionCost() const {
    if (!CHEAP_AND_FAST_SHIP_PRODUCTION)
        return m_production_cost;
    else
        return 1.0;
}

double ShipDesign::PerTurnCost() const
{ return ProductionCost() / std::max(1, ProductionTime()); }

int ShipDesign::ProductionTime() const {
    if (!CHEAP_AND_FAST_SHIP_PRODUCTION)
        return m_production_time;
    else
        return 1;
}


//// TEMPORARY
double ShipDesign::Defense() const {
    // accumulate defense from defensive parts in design.
    double total_defense = 0.0;
    const PartTypeManager& part_manager = GetPartTypeManager();
    std::vector<std::string> all_parts = Parts();
    for (std::vector<std::string>::const_iterator it = all_parts.begin(); it != all_parts.end(); ++it) {
        const PartType* part = part_manager.GetPartType(*it);
        if (part && (part->Class() == PC_SHIELD || part->Class() == PC_ARMOUR))
            total_defense += boost::get<double>(part->Stats());
    }
    return total_defense;
}

double ShipDesign::Attack() const {
    // accumulate attack stat from all weapon parts in design
    const PartTypeManager& manager = GetPartTypeManager();

    double total_attack = 0.0;
    std::vector<std::string> all_parts = Parts();
    for (std::vector<std::string>::const_iterator it = all_parts.begin(); it != all_parts.end(); ++it) {
        const PartType* part = manager.GetPartType(*it);
        if (part) {
            if (part->Class() == PC_SHORT_RANGE || part->Class() == PC_POINT_DEFENSE)
                total_attack += boost::get<DirectFireStats>(part->Stats()).m_damage;
            else if (part->Class() == PC_MISSILES)
                total_attack += boost::get<LRStats>(part->Stats()).m_damage;
            else if (part->Class() == PC_FIGHTERS)
                total_attack += boost::get<FighterStats>(part->Stats()).m_anti_ship_damage;
        }
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

bool ShipDesign::ProductionLocation(int empire_id, int location_id) const {
    const UniverseObject* location = GetUniverseObject(location_id);
    if (!location)
        return false;

    // currently ships can only be built at planets, and by species that are
    // not planetbound
    const Planet* planet = universe_object_cast<const Planet*>(location);
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

    Empire* empire = Empires().Lookup(empire_id);
    if (!empire) {
        Logger().debugStream() << "ShipDesign::ProductionLocation: Unable to get pointer to empire " << empire_id;
        return false;
    }

    // get a source object, which is owned by the empire with the passed-in
    // empire id.  this is used in conditions to reference which empire is
    // doing the producing.  Ideally this will be the capital, but any object
    // owned by the empire will work.
    const UniverseObject* source = GetUniverseObject(empire->CapitalID());
    if (!source && location->OwnedBy(empire_id))
        source = location;
    // still no valid source?!  scan through all objects to find one owned by this empire
    if (!source) {
        for (ObjectMap::const_iterator obj_it = Objects().const_begin(); obj_it != Objects().const_end(); ++obj_it) {
            if (obj_it->second->OwnedBy(empire_id)) {
                source = obj_it->second;
                break;
            }
        }
    }
    // if this empire doesn't own ANYTHING, then how is it producing anyway?
    if (!source)
        return false;

    // apply hull location conditions to potential location
    const HullType* hull = GetHull();
    if (!hull) {
        Logger().errorStream() << "ShipDesign::ProductionLocation  ShipDesign couldn't get its own hull with name " << m_hull;
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
            Logger().errorStream() << "ShipDesign::ProductionLocation  ShipDesign couldn't get part with name " << part_name;
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
    // ensure hull type exists and has exactly enough slots for passed parts
    const HullType* hull_type = GetHullTypeManager().GetHullType(hull);
    if (!hull_type) {
        Logger().debugStream() << "ShipDesign::ValidDesign: hull not found: " << hull;
        return false;
    }

    unsigned int size = parts.size();
    if (size > hull_type->NumSlots()) {
        Logger().debugStream() << "ShipDesign::ValidDesign: given " << size << " parts for hull with " << hull_type->NumSlots() << " slots";
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
            Logger().debugStream() << "ShipDesign::ValidDesign: part not found: " << part_name;
            return false;
        }

        // verify part can mount in indicated slot
        ShipSlotType slot_type = slots[i].type;
        if (!(part->CanMountInSlotType(slot_type))) {
            Logger().debugStream() << "ShipDesign::ValidDesign: part " << part_name << " can't be mounted in " << boost::lexical_cast<std::string>(slot_type) << " slot";
            return false;
        }
    }

    return true;
}

void ShipDesign::BuildStatCaches() {
    const HullType* hull = GetHullType(m_hull);
    if (!hull) {
        Logger().errorStream() << "ShipDesign::BuildStatCaches couldn't get hull with name " << m_hull;
        return;
    }

    m_production_time = hull->ProductionTime();
    m_production_cost = hull->ProductionCost();
    m_producible =      hull->Producible();
    m_detection =       hull->Detection();
    m_colony_capacity = hull->ColonyCapacity();
    m_troop_capacity =  hull->TroopCapacity();
    m_stealth =         hull->Stealth();
    m_fuel =            hull->Fuel();
    m_shields =         hull->Shields();
    m_structure =       hull->Structure();
    m_battle_speed =    hull->BattleSpeed();
    m_starlane_speed =  hull->StarlaneSpeed();

    for (std::vector<std::string>::const_iterator it = m_parts.begin(); it != m_parts.end(); ++it) {
        if (it->empty())
            continue;

        const PartType* part = GetPartType(*it);
        if (!part) {
            Logger().errorStream() << "ShipDesign::BuildStatCaches couldn't get part with name " << *it;
            continue;
        }

        m_production_time = std::max(m_production_time, part->ProductionTime()); // assume hull and parts are built in parallel
        m_production_cost += part->ProductionCost();                             // add up costs of all parts
        if (!part->Producible())
            m_producible = false;

        switch (part->Class()) {
        case PC_SHORT_RANGE: {
            const DirectFireStats& stats = boost::get<DirectFireStats>(part->Stats());
            m_SR_weapons.insert(std::make_pair(stats.m_range, part));
            m_is_armed = true;
            m_min_SR_range = std::min(m_min_SR_range, stats.m_range);
            m_max_SR_range = std::max(m_max_SR_range, stats.m_range);
            m_min_weapon_range = std::min(m_min_weapon_range, stats.m_range);
            m_max_weapon_range = std::max(m_max_weapon_range, stats.m_range);
            m_min_non_PD_weapon_range = std::min(m_min_non_PD_weapon_range, stats.m_range);
            m_max_non_PD_weapon_range = std::max(m_max_non_PD_weapon_range, stats.m_range);
            break;
        }
        case PC_MISSILES: {
            const LRStats& stats = boost::get<LRStats>(part->Stats());
            m_LR_weapons.insert(std::make_pair(stats.m_range, part));
            m_is_armed = true;
            m_min_LR_range = std::min(m_min_LR_range, stats.m_range);
            m_max_LR_range = std::max(m_max_LR_range, stats.m_range);
            m_min_weapon_range = std::min(m_min_weapon_range, stats.m_range);
            m_max_weapon_range = std::max(m_max_weapon_range, stats.m_range);
            m_min_non_PD_weapon_range = std::min(m_min_non_PD_weapon_range, stats.m_range);
            m_max_non_PD_weapon_range = std::max(m_max_non_PD_weapon_range, stats.m_range);
            break;
        }
        case PC_FIGHTERS:
            m_F_weapons.push_back(part);
            m_is_armed = true;
            break;
        case PC_POINT_DEFENSE: {
            const DirectFireStats& stats = boost::get<DirectFireStats>(part->Stats());
            m_PD_weapons.insert(std::make_pair(stats.m_range, part));
            m_is_armed = true;
            m_min_PD_range = std::min(m_min_PD_range, stats.m_range);
            m_max_PD_range = std::max(m_max_PD_range, stats.m_range);
            m_min_weapon_range = std::min(m_min_weapon_range, stats.m_range);
            m_max_weapon_range = std::max(m_max_weapon_range, stats.m_range);
            break;
        }
        case PC_COLONY:
            m_colony_capacity += boost::get<double>(part->Stats());
            break;
        case PC_TROOPS:
            m_troop_capacity += boost::get<double>(part->Stats());
            break;
        case PC_STEALTH:
            m_stealth += boost::get<double>(part->Stats());
            break;
        case PC_BATTLE_SPEED:
            m_battle_speed += boost::get<double>(part->Stats());
            break;
        case PC_STARLANE_SPEED:
            m_starlane_speed += boost::get<double>(part->Stats());
            break;
        case PC_SHIELD:
            m_shields += boost::get<double>(part->Stats());
            break;
        case PC_FUEL:
            m_fuel += boost::get<double>(part->Stats());
            break;
        case PC_ARMOUR:
            m_structure += boost::get<double>(part->Stats());
            break;
        case PC_DETECTION:
            m_detection += boost::get<double>(part->Stats());
            break;
        default:
            break;
        }
    }

    if (m_SR_weapons.empty())
        m_min_SR_range = 0.0;
    if (m_LR_weapons.empty())
        m_min_LR_range = 0.0;
    if (m_PD_weapons.empty())
        m_min_PD_range = 0.0;
    if (!m_min_SR_range && !m_min_LR_range && !m_min_PD_range)
        m_min_weapon_range = 0.0;
    if (!m_min_LR_range && !m_min_PD_range)
        m_min_non_PD_weapon_range = 0.0;
}

std::string ShipDesign::Dump() const {
    std::string retval = DumpIndent() + "ShipDesign\n";
    ++g_indent;
    retval += DumpIndent() + "name = \"" + m_name + "\"\n";
    retval += DumpIndent() + "description = \"" + m_description + "\"\n";
    retval += DumpIndent() + "lookup_strings = " + (m_name_desc_in_stringtable ? "true" : "false") + "\n";
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
    retval += DumpIndent() + "graphic = \"" + m_graphic + "\"\n";
    retval += DumpIndent() + "model = \"" + m_3D_model + "\"\n";
    --g_indent;
    return retval; 
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

    Logger().debugStream() << "Initializing PredefinedShipDesignManager";

    parse::ship_designs(GetResourceDir() / "premade_ship_designs.txt", m_ship_designs);

    parse::ship_designs(GetResourceDir() / "space_monsters.txt", m_monster_designs);

    if (GetOptionsDB().Get<bool>("verbose-logging")) {
        Logger().debugStream() << "Predefined Ship Designs:";
        for (iterator it = begin(); it != end(); ++it) {
            const ShipDesign* d = it->second;
            Logger().debugStream() << " ... " << d->Name();
        }
        Logger().debugStream() << "Monster Ship Designs:";
        for (iterator it = begin_monsters(); it != end_monsters(); ++it) {
            const ShipDesign* d = it->second;
            Logger().debugStream() << " ... " << d->Name();
        }
    }
}

PredefinedShipDesignManager::~PredefinedShipDesignManager() {
    for (std::map<std::string, ShipDesign*>::iterator it = m_ship_designs.begin(); it != m_ship_designs.end(); ++it)
        delete it->second;
}

std::map<std::string, int> PredefinedShipDesignManager::AddShipDesignsToEmpire(Empire* empire, const std::vector<std::string>& design_names) const {
    std::map<std::string, int> retval;
    if (!empire || design_names.empty())
        return retval;
    int empire_id = empire->EmpireID();
    Universe& universe = GetUniverse();

    for (std::vector<std::string>::const_iterator it = design_names.begin(); it != design_names.end(); ++it) {
        std::map<std::string, ShipDesign*>::const_iterator design_it = m_ship_designs.find(*it);
        if (design_it == m_ship_designs.end())
            continue;

        ShipDesign* d = design_it->second;
        if (!d->Producible())
            continue;
        if (design_it->first != d->Name(false))
            Logger().errorStream() << "Predefined ship design name in map (" << design_it->first << ") doesn't match name in ShipDesign::m_name (" << d->Name(false) << ")";

        // make copy of design to add to universe, after modifying created by empire ID
        ShipDesign* copy = new ShipDesign(d->Name(false), d->Description(false), empire_id,
                                          d->DesignedOnTurn(), d->Hull(), d->Parts(),
                                          d->Graphic(), d->Model(), true);

        int design_id = empire->AddShipDesign(copy);    // also inserts design into Universe

        if (design_id == ShipDesign::INVALID_DESIGN_ID) {
            delete copy;
            Logger().errorStream() << "PredefinedShipDesignManager::AddShipDesignsToEmpire couldn't add a design to an empire";
        } else {
            retval[design_it->first] = design_id;

            universe.SetEmpireKnowledgeOfShipDesign(design_id, empire_id);
        }
    }

    return retval;
}

namespace {
    void AddDesignToUniverse(std::map<std::string, int>& design_generic_ids, ShipDesign* design, bool monster) {
        if (!design)
            return;

        ShipDesign* copy = new ShipDesign(design->Name(false), design->Description(false), ALL_EMPIRES,
                                          design->DesignedOnTurn(), design->Hull(), design->Parts(),
                                          design->Graphic(), design->Model(), true, monster);

        if (!copy) {
            Logger().errorStream() << "PredefinedShipDesignManager::AddShipDesignsToUniverse() couldn't duplicate the design with name " << design->Name();
            return;
        }

        const ShipDesign& design_ref = *copy;

        bool already_added = false;
        Universe& universe = GetUniverse();
        /* check if there already exists this same design in the universe. */
        for (Universe::ship_design_iterator it = universe.beginShipDesigns(); it != universe.endShipDesigns(); ++it) {
            const ShipDesign* existing_design = it->second;
            if (!existing_design) {
                Logger().errorStream() << "PredefinedShipDesignManager::AddShipDesignsToUniverse found an invalid design in the Universe";
                continue;
            }

            const ShipDesign& existing_design_ref = *existing_design;

            if (DesignsTheSame(existing_design_ref, design_ref)) {
                Logger().debugStream() << "PredefinedShipDesignManager::AddShipDesignsToUniverse found there already is an exact duplicate of a design to be added, so is not re-adding it";
                design_generic_ids[design_ref.Name(false)] = existing_design_ref.ID();
                already_added = true;
                break;
            }
        }

        if (already_added) {
            delete copy;
            return;
        }

        // design is apparently new, so add it to the universe
        int new_design_id = GetNewDesignID();   // generate new design id

        if (new_design_id == ShipDesign::INVALID_DESIGN_ID) {
            Logger().errorStream() << "PredefinedShipDesignManager::AddShipDesignsToUniverse Unable to get new design id";
            delete copy;
            return;
        }

        bool success = universe.InsertShipDesignID(copy, new_design_id);

        if (!success) {
            Logger().errorStream() << "Empire::AddShipDesign Unable to add new design to universe";
            delete copy;
            return;
        }

        design_generic_ids[design_ref.Name(false)] = new_design_id;
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

int PredefinedShipDesignManager::GenericDesignID(const std::string& name) const {
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
