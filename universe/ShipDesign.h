// -*- C++ -*-
#ifndef _ShipDesign_h_
#define _ShipDesign_h_

#include <string>
#include <vector>
#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/variant.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include "Enums.h"

namespace Condition {
    struct ConditionBase;
}
namespace Effect {
    class EffectsGroup;
}
class Empire;

/** Part stats for the PC_SHORT_RANGE and PC_POINT_DEFENSE part classes. */
struct DirectFireStats {
    DirectFireStats();
    DirectFireStats(double damage,
                    double ROF,
                    double range);

    double m_damage;
    double m_ROF;
    double m_range;

    /** The factor by which PD damage should be multiplied when used in
        defense of the ship firing it. */
    static const double PD_SELF_DEFENSE_FACTOR;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar  & BOOST_SERIALIZATION_NVP(m_damage)
            & BOOST_SERIALIZATION_NVP(m_ROF)
            & BOOST_SERIALIZATION_NVP(m_range);
    }
};

/** Part stats for the PC_MISSILES part class. */
struct LRStats {
    LRStats();
    LRStats(double damage,
            double ROF,
            double range,
            double speed,
            double stealth,
            double structure,
            int capacity);

    double m_damage;
    double m_ROF;
    double m_range;
    double m_speed;
    double m_stealth;
    double m_structure;
    int m_capacity;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar  & BOOST_SERIALIZATION_NVP(m_damage)
            & BOOST_SERIALIZATION_NVP(m_ROF)
            & BOOST_SERIALIZATION_NVP(m_range)
            & BOOST_SERIALIZATION_NVP(m_speed)
            & BOOST_SERIALIZATION_NVP(m_stealth)
            & BOOST_SERIALIZATION_NVP(m_structure)
            & BOOST_SERIALIZATION_NVP(m_capacity);
    }
};

/** Part stats for the PC_FIGHTERS part class. */
struct FighterStats {
    FighterStats();
    FighterStats(CombatFighterType type,
                 double anti_ship_damage,
                 double anti_fighter_damage,
                 double launch_rate,
                 double fighter_weapon_range,
                 double speed,
                 double stealth,
                 double structure,
                 double detection,
                 int capacity);

    CombatFighterType   m_type;
    double              m_anti_ship_damage;
    double              m_anti_fighter_damage;
    double              m_launch_rate;
    double              m_fighter_weapon_range;
    double              m_speed;
    double              m_stealth;
    double              m_structure;
    double              m_detection;
    int                 m_capacity;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar  & BOOST_SERIALIZATION_NVP(m_type)
            & BOOST_SERIALIZATION_NVP(m_anti_ship_damage)
            & BOOST_SERIALIZATION_NVP(m_anti_fighter_damage)
            & BOOST_SERIALIZATION_NVP(m_launch_rate)
            & BOOST_SERIALIZATION_NVP(m_fighter_weapon_range)
            & BOOST_SERIALIZATION_NVP(m_speed)
            & BOOST_SERIALIZATION_NVP(m_stealth)
            & BOOST_SERIALIZATION_NVP(m_structure)
            & BOOST_SERIALIZATION_NVP(m_detection)
            & BOOST_SERIALIZATION_NVP(m_capacity);
    }
};

/** A variant type containing all ShipPartClass-specific stats for a PartType.
  * Note that most parts need only a single value to represent their
  * capabilities.  This is represented by the double variant. */
typedef boost::variant<double, DirectFireStats, LRStats, FighterStats> PartTypeStats;

/** Common parameters for PartType and HullType constructors.  Used as temporary
  * storage for parsing to reduce number of sub-items parsed per item. */
struct PartHullCommonParams {
    PartHullCommonParams() :
        production_cost(0.0),
        production_time(1),
        producible(false),
        tags(),
        location(0),
        effects(),
        icon()
    {}
    PartHullCommonParams(double production_cost_,
                         int production_time_,
                         bool producible_,
                         const std::vector<std::string>& tags_,
                         const Condition::ConditionBase* location_,
                         const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects_,
                         const std::string& icon_) :
        production_cost(production_cost_),
        production_time(production_time_),
        producible(producible_),
        tags(tags_),
        location(location_),
        effects(effects_),
        icon(icon_)
    {}
    double                          production_cost;  // in PP
    int                             production_time;  // in turns
    bool                            producible;
    std::vector<std::string>        tags;
    const Condition::ConditionBase* location;
    std::vector<boost::shared_ptr<const Effect::EffectsGroup> >
                                    effects;
    std::string                     icon;
};

/** A type of ship part */
class PartType {
public:
    /** \name Structors */ //@{
    PartType() :
        m_name("invalid part type"),
        m_description("indescribable"),
        m_class(INVALID_SHIP_PART_CLASS),
        m_stats(1.0),
        m_production_cost(1.0),
        m_production_time(1),
        m_mountable_slot_types(),
        m_tags(),
        m_location(0),
        m_effects(),
        m_icon()
    {}
    PartType(const std::string& name, const std::string& description,
             ShipPartClass part_class, const PartTypeStats& stats,
             const PartHullCommonParams& common_params,
             std::vector<ShipSlotType> mountable_slot_types) :
        m_name(name),
        m_description(description),
        m_class(part_class),
        m_stats(stats),
        m_production_cost(common_params.production_cost),
        m_production_time(common_params.production_time),
        m_producible(common_params.producible),
        m_mountable_slot_types(mountable_slot_types),
        m_tags(common_params.tags),
        m_location(common_params.location),
        m_effects(),
        m_icon(common_params.icon)
    { Init(common_params.effects); }

    ~PartType();
    //@}

    /** \name Accessors */ //@{
    const std::string&      Name() const            { return m_name; };         ///< returns name of part
    const std::string&      Description() const     { return m_description; }   ///< returns description, including a description of the stats and effects of this part
    std::string             StatDescription() const;                            ///< returns autogenerated list of various stats associated with part, such as capacity or power, or more type specific stats like range or firing rate
    ShipPartClass           Class() const           { return m_class; }         ///< returns that class of part that this is.
    const PartTypeStats&    Stats() const           { return m_stats; }         ///< returns how good the part is at its function.  might be weapon or shield strength, or cargo hold capacity
    bool                    CanMountInSlotType(ShipSlotType slot_type) const;   ///< returns true if this part can be placed in a slot of the indicated type
    double                  ProductionCost() const  { return m_production_cost;}///< returns total cost of part
    int                     ProductionTime() const  { return m_production_time;}///< returns turns taken to build this part (the minimum time to build a ship design containing this part)
    bool                    Producible() const      { return m_producible; }    ///< returns whether this part type is producible by players and appears on the design screen
    const std::vector<std::string>& Tags() const    { return m_tags; }
    const Condition::ConditionBase* Location() const{ return m_location; }      ///< returns the condition that determines the locations where ShipDesign containing part can be produced
    const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& Effects() const
    { return m_effects; }                                                       ///< returns the EffectsGroups that encapsulate the effects this part has
    const std::string&      Icon() const            { return m_icon; }          ///< returns icon graphic that represents part in UI
    //@}

private:
    void                    Init(const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects);

    std::string         m_name;
    std::string         m_description;
    ShipPartClass       m_class;
    PartTypeStats       m_stats;
    double              m_production_cost;  // in PP
    int                 m_production_time;  // in turns
    bool                m_producible;
    std::vector<ShipSlotType>       m_mountable_slot_types;
    std::vector<std::string>        m_tags;
    const Condition::ConditionBase* m_location;

    std::vector<boost::shared_ptr<const Effect::EffectsGroup> >
                        m_effects;

    std::string         m_icon;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Holds FreeOrion ship part types */
class PartTypeManager {
public:
    typedef std::map<std::string, PartType*>::const_iterator iterator;

    /** \name Accessors */ //@{
    /** returns the part type with the name \a name; you should use the free function GetPartType() instead */
    const PartType* GetPartType(const std::string& name) const;

    /** iterator to the first part type */
    iterator begin() const;

    /** iterator to the last + 1th part type */
    iterator end() const;

    /** returns the instance of this singleton class; you should use the free function GetPartTypeManager() instead */
    static const PartTypeManager& GetPartTypeManager();
    //@}

private:
    PartTypeManager();
    ~PartTypeManager();

    std::map<std::string, PartType*>    m_parts;
    static PartTypeManager*             s_instance;
};


/** returns the singleton part type manager */
const PartTypeManager& GetPartTypeManager();

/** Returns the ship PartType specification object with name \a name.  If no
  * such PartType exists, 0 is returned instead. */
const PartType* GetPartType(const std::string& name);

/** Hull stats.  Used by parser due to limits on number of sub-items per
  * parsed main item. */
struct HullTypeStats {
    HullTypeStats() :
        fuel(0.0),
        battle_speed(0.0),
        starlane_speed(0.0),
        stealth(0.0),
        structure(0.0)
    {}

    HullTypeStats(double fuel_,
                  double battle_speed_,
                  double starlane_speed_,
                  double stealth_,
                  double structure_) :
        fuel(fuel_),
        battle_speed(battle_speed_),
        starlane_speed(starlane_speed_),
        stealth(stealth_),
        structure(structure_)
    {}

    double  fuel;
    double  battle_speed;
    double  starlane_speed;
    double  stealth;
    double  structure;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar  & BOOST_SERIALIZATION_NVP(fuel)
            & BOOST_SERIALIZATION_NVP(battle_speed)
            & BOOST_SERIALIZATION_NVP(starlane_speed)
            & BOOST_SERIALIZATION_NVP(stealth)
            & BOOST_SERIALIZATION_NVP(structure);
    }
};

/** Specification for the hull, or base, on which ship designs are created by
  * adding parts.  The hull determines some final design characteristics
  * directly, and also determine how many parts can be added to the design. */
class HullType {
public:
    struct Slot {
        Slot() :
            type(INVALID_SHIP_SLOT_TYPE), x(0.5), y(0.5)
        {}
        Slot(ShipSlotType slot_type, double x_, double y_) :
            type(slot_type), x(x_), y(y_)
        {}
        ShipSlotType type;
        double x, y;
    };

    /** \name Structors */ //@{
    HullType() :
        m_name("generic hull type"),
        m_description("indescribable"),
        m_battle_speed(1.0),
        m_starlane_speed(1.0),
        m_fuel(0.0),
        m_stealth(0.0),
        m_structure(0.0),
        m_production_cost(1.0),
        m_production_time(1),
        m_producible(false),
        m_slots(),
        m_tags(),
        m_location(0),
        m_effects(),
        m_graphic(),
        m_icon()
    {}

    HullType(const std::string& name, const std::string& description,
             double fuel, double battle_speed, double starlane_speed,
             double stealth, double structure,
             const PartHullCommonParams& common_params,
             const std::vector<Slot>& slots,
             const std::string& graphic) :
        m_name(name),
        m_description(description),
        m_battle_speed(battle_speed),
        m_starlane_speed(starlane_speed),
        m_fuel(fuel),
        m_stealth(stealth),
        m_structure(structure),
        m_production_cost(common_params.production_cost),
        m_production_time(common_params.production_time),
        m_producible(common_params.producible),
        m_slots(slots),
        m_tags(common_params.tags),
        m_location(common_params.location),
        m_effects(),
        m_graphic(graphic),
        m_icon(common_params.icon)
    { Init(common_params.effects); }

    HullType(const std::string& name, const std::string& description,
             const HullTypeStats& stats,
             const PartHullCommonParams& common_params,
             const std::vector<Slot>& slots,
             const std::string& graphic) :
        m_name(name),
        m_description(description),
        m_battle_speed(stats.battle_speed),
        m_starlane_speed(stats.starlane_speed),
        m_fuel(stats.fuel),
        m_stealth(stats.stealth),
        m_structure(stats.structure),
        m_production_cost(common_params.production_cost),
        m_production_time(common_params.production_time),
        m_producible(common_params.producible),
        m_slots(slots),
        m_tags(common_params.tags),
        m_location(common_params.location),
        m_effects(),
        m_graphic(graphic),
        m_icon(common_params.icon)
    { Init(common_params.effects); }

    ~HullType();
    //@}

    /** \name Accessors */ //@{
    const std::string&  Name() const            { return m_name; }              ///< returns name of hull
    const std::string&  Description() const     { return m_description; }       ///< returns description, including a description of the stats and effects of this hull
    std::string         StatDescription() const;                                ///< returns autogenerated list of various stats associated with part, such as speeds or fuel capacity

    double              BattleSpeed() const     { return m_battle_speed; }      ///< returns battle speed of hull
    double              StarlaneSpeed() const   { return m_starlane_speed; }    ///< returns starlane speed of hull
    double              Fuel() const            { return m_fuel; }              ///< returns fuel capacity of hull
    double              Stealth() const         { return m_stealth; }           ///< returns stealth of hull
    double              Structure() const       { return m_structure; }         ///< returns structure of hull
    double              Shields() const         { return 0.0; }                 ///< returns shields of hull
    double              ColonyCapacity() const  { return 0.0; }                 ///< returns colonist capacity of hull
    double              TroopCapacity() const   { return 0.0; }                 ///< returns the troop capacity of hull
    double              Detection() const       { return 0.0; }                 ///< returns detection ability of hull

    double              ProductionCost() const  { return m_production_cost; }   ///< returns total cost of hull
    int                 ProductionTime() const  { return m_production_time; }   ///< returns base build time for this hull, before parts are added
    bool                Producible() const      { return m_producible; }        ///< returns whether this hull type is producible by players and appears on the design screen

    unsigned int        NumSlots() const        { return m_slots.size(); }      ///< returns total number of of slots in hull
    unsigned int        NumSlots(ShipSlotType slot_type) const;                 ///< returns number of of slots of indicated type in hull
    const std::vector<Slot>&    Slots() const   { return m_slots; }             ///< returns vector of slots in hull

    const std::vector<std::string>& Tags() const{ return m_tags; }

    const Condition::ConditionBase* Location() const
    { return m_location; }                                                      ///< returns the condition that determines the locations where ShipDesign containing hull can be produced
    const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& Effects() const
    { return m_effects; }                                                       ///< returns the EffectsGroups that encapsulate the effects this part hull has

    const std::string&  Graphic() const         { return m_graphic; }           ///< returns the image that represents the hull on the design screen
    const std::string&  Icon() const            { return m_icon; }              ///< returns the small icon to represent hull
    //@}

private:
    void                Init(const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects);

    std::string                 m_name;
    std::string                 m_description;
    double                      m_battle_speed;
    double                      m_starlane_speed;
    double                      m_fuel;
    double                      m_stealth;
    double                      m_structure;
    double                      m_production_cost;  // in PP
    int                         m_production_time;  // in turns
    bool                        m_producible;
    std::vector<Slot>           m_slots;

    std::vector<std::string>    m_tags;

    const Condition::ConditionBase*
                                m_location;
    std::vector<boost::shared_ptr<const Effect::EffectsGroup> >
                                m_effects;

    std::string                 m_graphic;
    std::string                 m_icon;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Holds FreeOrion hull types */
class HullTypeManager {
public:
    typedef std::map<std::string, HullType*>::const_iterator iterator;

    /** \name Accessors */ //@{
    /** returns the hull type with the name \a name; you should use the free function GetHullType() instead */
    const HullType* GetHullType(const std::string& name) const;

    /** iterator to the first hull type */
    iterator begin() const;

    /** iterator to the last + 1th hull type */
    iterator end() const;

    /** returns the instance of this singleton class; you should use the free function GetHullTypeManager() instead */
    static const HullTypeManager& GetHullTypeManager();
    //@}

private:
    HullTypeManager();
    ~HullTypeManager();

    std::map<std::string, HullType*> m_hulls;
    static HullTypeManager* s_instance;
};

/** returns the singleton hull type manager */
const HullTypeManager& GetHullTypeManager();

/** Returns the ship HullType specification object with name \a name.  If no such HullType exists,
  * 0 is returned instead. */
const HullType* GetHullType(const std::string& name);


class ShipDesign {
public:
    /** \name Structors */ //@{
    ShipDesign();
    ShipDesign(const std::string& name, const std::string& description, int designed_by_empire_id,
               int designed_on_turn, const std::string& hull, const std::vector<std::string>& parts,
               const std::string& icon, const std::string& model,
               bool name_desc_in_stringtable = false, bool monster = false);
    //@}

    /** \name Accessors */ //@{
    int                             ID() const  { return m_id; }    ///< returns id number of design
    /** returns name of design.  if \a stringtable_lookup is true and the
      * design was constructed specifying name_desc_in_stringtable true,
      * the name string is looked up in the stringtable before being returned.
      * otherwise, the raw name string is returned. */
    const std::string&              Name(bool stringtable_lookup = true) const;

    /** returns description of design.  if \a stringtable_lookup is true and
      * the design was constructed specifying name_desc_in_stringtable true,
      * the description string is looked up in the stringtable before being
      * returned.  otherwise, the raw name string is returned. */
    const std::string&              Description(bool stringtable_lookup = true) const;

    int                             DesignedByEmpire() const{ return m_designed_by_empire_id; } ///< returns id of empire that created design
    int                             DesignedOnTurn() const  { return m_designed_on_turn; };     ///< returns turn on which design was created

    double                          ProductionCost() const;     ///< returns the total cost to build a ship of this design
    double                          PerTurnCost() const;        ///< returns the maximum per-turn number of production points that can be spent on building a ship of this design
    int                             ProductionTime() const;     ///< returns the time in turns it takes to build a ship of this design
    bool                            Producible() const      { return m_producible; }            ///< returns whether this design is producible by players and appears on the production screen list

    double                          BattleSpeed() const     { return m_battle_speed; }          ///< returns design speed on the battle map
    double                          StarlaneSpeed() const   { return m_starlane_speed; }        ///< returns design speed along starlanes

    double                          Structure() const       { return m_structure; }             ///< returns the max structure of this design
    double                          Shields() const         { return m_shields; }               ///< returns the max shields of this design
    double                          Fuel() const            { return m_fuel; }                  ///< returns the max fuel capacity of this design
    double                          Detection() const       { return m_detection; }             ///< returns the detection ability of this design
    double                          ColonyCapacity() const  { return m_colony_capacity; }       ///< returns the colonization capacity of this design
    double                          TroopCapacity() const   { return m_troop_capacity; }        ///< returns the troop capacity of this design
    double                          Stealth() const         { return m_stealth; }               ///< returns the stealth of this design

    bool                            CanColonize() const     { return (m_colony_capacity > 0.0); }
    bool                            HasTroops() const       { return (m_troop_capacity > 0.0); }
    bool                            IsArmed() const         { return m_is_armed; }
    bool                            IsMonster() const       { return m_is_monster; }

    /** Return maps from ranges to stats for various types of weapons in design */
    const std::multimap<double, const PartType*>& SRWeapons() const { return m_SR_weapons; }
    const std::multimap<double, const PartType*>& LRWeapons() const { return m_LR_weapons; }
    const std::multimap<double, const PartType*>& PDWeapons() const { return m_PD_weapons; }
    /** Returns the set of Fighter weapons in this design. */
    const std::vector<const PartType*>&           FWeapons() const  { return m_F_weapons; }

    double  MinSRRange() const          { return m_min_SR_range; }
    double  MaxSRRange() const          { return m_max_SR_range; }
    double  MinLRRange() const          { return m_min_LR_range; }
    double  MaxLRRange() const          { return m_max_LR_range; }
    double  MinPDRange() const          { return m_min_PD_range; }
    double  MaxPDRange() const          { return m_max_PD_range; }
    double  MinWeaponRange() const      { return m_min_weapon_range; }
    double  MaxWeaponRange() const      { return m_max_weapon_range; }
    double  MinNonPDWeaponRange() const { return m_min_non_PD_weapon_range; }
    double  MaxNonPDWeaponRange() const { return m_max_non_PD_weapon_range; }

    /////// TEMPORARY ///////
    double  Defense() const;
    double  Attack() const;
    /////// TEMPORARY ///////


    const std::string&              Hull() const            { return m_hull; }      ///< returns name of hull on which design is based
    const HullType*                 GetHull() const
    { return GetHullTypeManager().GetHullType(m_hull); }                            ///< returns HullType on which design is based

    const std::vector<std::string>& Parts() const           { return m_parts; }     ///< returns vector of names of all parts in design
    std::vector<std::string>        Parts(ShipSlotType slot_type) const;            ///< returns vector of names of parts in slots of indicated type

    std::vector<std::string>        Tags() const;

    const std::string&              Icon() const            { return m_icon; }      ///< returns filename for small-size icon graphic for design
    const std::string&              Model() const           { return m_3D_model; }  ///< returns filename of 3D model that represents ships of design
    bool                            LookupInStringtable() const { return m_name_desc_in_stringtable; }

    std::string                     Dump() const;                                   ///< returns a data file format representation of this object
    //@}

    bool                            ProductionLocation(int empire_id, int location_id) const;   ///< returns true iff the empire with ID empire_id can produce this design at the location with location_id

    /** \name Mutators */ //@{
    void                            SetID(int id);                          ///< sets the ID number of the design to \a id .  Should only be used by Universe class when inserting new design into Universe.
    void                            Rename(const std::string& name) { m_name = name; }  ///< renames this design to \a name
    //@}

    ///< returns true if the \a hull and parts vectors passed make a valid ShipDesign
    static bool                     ValidDesign(const std::string& hull, const std::vector<std::string>& parts);

    /** returns true if the \a design passed is a valid ShipDesign in terms of
      * (its hull and parts.  does not check any other member variables */
    static bool                     ValidDesign(const ShipDesign& design)
    { return ValidDesign(design.m_hull, design.m_parts); }

    static const int                INVALID_DESIGN_ID;
    static const int                MAX_ID;

private:
    void BuildStatCaches();

    int                         m_id;

    std::string                 m_name;
    std::string                 m_description;

    int                         m_designed_by_empire_id;
    int                         m_designed_on_turn;

    std::string                 m_hull;
    std::vector<std::string>    m_parts;
    bool                        m_is_monster;

    std::string                 m_icon;
    std::string                 m_3D_model;

    bool                        m_name_desc_in_stringtable;

    // Note that these are fine to compute on demand and cache here -- it is
    // not necessary to serialize them.
    bool    m_is_armed;
    double  m_detection;
    double  m_colony_capacity;
    double  m_troop_capacity;
    double  m_stealth;
    double  m_fuel;
    double  m_shields;
    double  m_structure;
    double  m_battle_speed;
    double  m_starlane_speed;
    double  m_production_cost;
    int     m_production_time;
    bool    m_producible;
    std::multimap<double, const PartType*>  m_SR_weapons;
    std::multimap<double, const PartType*>  m_LR_weapons;
    std::multimap<double, const PartType*>  m_PD_weapons;
    std::vector<const PartType*>            m_F_weapons;
    double  m_min_SR_range;
    double  m_max_SR_range;
    double  m_min_LR_range;
    double  m_max_LR_range;
    double  m_min_PD_range;
    double  m_max_PD_range;
    double  m_min_weapon_range;
    double  m_max_weapon_range;
    double  m_min_non_PD_weapon_range;
    double  m_max_non_PD_weapon_range;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Returns the ShipDesign specification object with id \a ship_design_id.  If
  * no such ShipDesign is present in the Universe (because it doesn't exist,
  * or isn't know to this client), 0 is returned instead. */
const ShipDesign* GetShipDesign(int ship_design_id);


class PredefinedShipDesignManager {
public:
    typedef std::map<std::string, ShipDesign*>::const_iterator iterator;
    typedef std::map<std::string, int>::const_iterator generic_iterator;

    /** \name Accessors */ //@{
    /** Returns iterator pointing to first ship design. */
    iterator            begin() const;

    /** Returns iterator pointing one past last ship design. */
    iterator            end() const;

    /** Returns iterator pointing to first monster design. */
    iterator            begin_monsters() const;

    /** Returns iterator pointing one past last monster design. */
    iterator            end_monsters() const;

    /** Returns iterator pointing to first generic design name and id. */
    generic_iterator    begin_generic() const;

    /** Returns iterator pointing one past the last generic design name and id. */
    generic_iterator    end_generic() const;

    /** Returns the ID for the generic (not created by any empire) design in
      * the Universe for the predefined design with the specified \a name.  If
      * there is no generic design available for the specified \a name, then
      * INVALID_DESIGN_ID is returned. */
    int                 GenericDesignID(const std::string& name) const;
    //@}

    /** Adds designs in this manager to the specified \a empire using that
      * Empire's AddShipDesign(ShipDesign*) function.  Designs are added with
      * the specified empire as the creator of the design, so each empire
      * for which designs are added actually creates a separate copy of the
      * design, with different designed-by empire for each  Returns a map from
      * ship design name to design id in universe.  Only designs specified in
      * \a design_names are added; others in this manager are not. */
    std::map<std::string, int>          AddShipDesignsToEmpire(Empire* empire, const std::vector<std::string>& design_names) const;

    /** Adds designs in this manager to the universe with the design creator
      * left as no empire.  Returns a map from ship design name to design id in
      * the universe. */
    const std::map<std::string, int>&   AddShipDesignsToUniverse() const;

    /** Returns the predefined ShipDesign with the name \a name.  If no such
      * ship design exists, 0 is returned instead. */
    static PredefinedShipDesignManager& GetPredefinedShipDesignManager();

private:
    PredefinedShipDesignManager();
    ~PredefinedShipDesignManager();

    std::map<std::string, ShipDesign*>  m_ship_designs;
    std::map<std::string, ShipDesign*>  m_monster_designs;
    mutable std::map<std::string, int>  m_design_generic_ids;   // ids of designs from this manager that have been added to the universe with no empire as the creator

    static PredefinedShipDesignManager* s_instance;
};

/** returns the singleton predefined ship design manager type manager */
const PredefinedShipDesignManager& GetPredefinedShipDesignManager();

/** Returns the predefined ShipDesign with the name \a name.  If no such
  * ship design exists, 0 is returned instead. */
const ShipDesign* GetPredefinedShipDesign(const std::string& name);


// template implementations
template <class Archive>
void PartType::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_description)
        & BOOST_SERIALIZATION_NVP(m_class)
        & BOOST_SERIALIZATION_NVP(m_stats)
        & BOOST_SERIALIZATION_NVP(m_production_cost)
        & BOOST_SERIALIZATION_NVP(m_production_time)
        & BOOST_SERIALIZATION_NVP(m_mountable_slot_types)
        & BOOST_SERIALIZATION_NVP(m_tags)
        & BOOST_SERIALIZATION_NVP(m_location)
        & BOOST_SERIALIZATION_NVP(m_effects)
        & BOOST_SERIALIZATION_NVP(m_icon);
}

template <class Archive>
void HullType::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_description)
        & BOOST_SERIALIZATION_NVP(m_battle_speed)
        & BOOST_SERIALIZATION_NVP(m_starlane_speed)
        & BOOST_SERIALIZATION_NVP(m_fuel)
        & BOOST_SERIALIZATION_NVP(m_stealth)
        & BOOST_SERIALIZATION_NVP(m_structure)
        & BOOST_SERIALIZATION_NVP(m_production_cost)
        & BOOST_SERIALIZATION_NVP(m_production_time)
        & BOOST_SERIALIZATION_NVP(m_slots)
        & BOOST_SERIALIZATION_NVP(m_tags)
        & BOOST_SERIALIZATION_NVP(m_location)
        & BOOST_SERIALIZATION_NVP(m_effects)
        & BOOST_SERIALIZATION_NVP(m_graphic)
        & BOOST_SERIALIZATION_NVP(m_icon);
}

#endif // _ShipDesign_h_
