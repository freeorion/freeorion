// -*- C++ -*-
#ifndef _ShipDesign_h_
#define _ShipDesign_h_

#include "ValueRefFwd.h"

#include <string>
#include <vector>
#include <map>
#include <set>

#include <boost/shared_ptr.hpp>
#include <boost/variant.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include "Enums.h"

#include "../util/Export.h"

namespace Condition {
    struct ConditionBase;
}
namespace Effect {
    class EffectsGroup;
}
class Empire;

/** Common parameters for PartType and HullType constructors.  Used as temporary
  * storage for parsing to reduce number of sub-items parsed per item. */
struct PartHullCommonParams {
    PartHullCommonParams() :
        production_cost(0),
        production_time(0),
        producible(false),
        tags(),
        location(0),
        effects(),
        icon()
    {}
    PartHullCommonParams(const ValueRef::ValueRefBase<double>* production_cost_,
                         const ValueRef::ValueRefBase<int>* production_time_,
                         bool producible_,
                         const std::set<std::string>& tags_,
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
    const ValueRef::ValueRefBase<double>*   production_cost;
    const ValueRef::ValueRefBase<int>*      production_time;
    bool                                    producible;
    std::set<std::string>                   tags;
    const Condition::ConditionBase*         location;
    std::vector<boost::shared_ptr<const Effect::EffectsGroup> >
                                            effects;
    std::string                             icon;
};

/** A type of ship part */
class FO_COMMON_API PartType {
public:
    /** \name Structors */ //@{
    PartType() :
        m_name("invalid part type"),
        m_description("indescribable"),
        m_class(INVALID_SHIP_PART_CLASS),
        m_capacity(0.0f),
        m_production_cost(0),
        m_production_time(0),
        m_mountable_slot_types(),
        m_tags(),
        m_location(0),
        m_effects(),
        m_icon()
    {}
    PartType(const std::string& name, const std::string& description,
             ShipPartClass part_class, double capacity,
             const PartHullCommonParams& common_params,
             std::vector<ShipSlotType> mountable_slot_types) :
        m_name(name),
        m_description(description),
        m_class(part_class),
        m_capacity(capacity),
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
    const std::string&      Name() const            { return m_name; };             ///< returns name of part
    const std::string&      Description() const     { return m_description; }       ///< returns description, including a description of the stats and effects of this part
    ShipPartClass           Class() const           { return m_class; }             ///< returns that class of part that this is.
    float                   Capacity() const;
    bool                    CanMountInSlotType(ShipSlotType slot_type) const;       ///< returns true if this part can be placed in a slot of the indicated type
    const std::vector<ShipSlotType>&
                            MountableSlotTypes() const  { return m_mountable_slot_types; }

    bool                    ProductionCostTimeLocationInvariant() const;            ///< returns true if the production cost and time are invariant (does not depend on) the location
    float                   ProductionCost(int empire_id, int location_id) const;   ///< returns the number of production points required to produce this part
    int                     ProductionTime(int empire_id, int location_id) const;   ///< returns the number of turns required to produce this part
    bool                    Producible() const      { return m_producible; }        ///< returns whether this part type is producible by players and appears on the design screen

    const std::set<std::string>& Tags() const       { return m_tags; }
    const Condition::ConditionBase* Location() const{ return m_location; }          ///< returns the condition that determines the locations where ShipDesign containing part can be produced
    const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& Effects() const
                                                    { return m_effects; }           ///< returns the EffectsGroups that encapsulate the effects this part has
    const std::string&      Icon() const            { return m_icon; }              ///< returns icon graphic that represents part in UI
    //@}

private:
    void                    Init(const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects);

    std::string                             m_name;
    std::string                             m_description;
    ShipPartClass                           m_class;
    float                                   m_capacity;
    const ValueRef::ValueRefBase<double>*   m_production_cost;
    const ValueRef::ValueRefBase<int>*      m_production_time;
    bool                                    m_producible;
    std::vector<ShipSlotType>               m_mountable_slot_types;
    std::set<std::string>                   m_tags;
    const Condition::ConditionBase*         m_location;
    std::vector<boost::shared_ptr<const Effect::EffectsGroup> >
                                            m_effects;
    std::string                             m_icon;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Holds FreeOrion ship part types */
class FO_COMMON_API PartTypeManager {
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
FO_COMMON_API const PartTypeManager& GetPartTypeManager();

/** Returns the ship PartType specification object with name \a name.  If no
  * such PartType exists, 0 is returned instead. */
FO_COMMON_API const PartType* GetPartType(const std::string& name);

/** Hull stats.  Used by parser due to limits on number of sub-items per
  * parsed main item. */
struct HullTypeStats {
    HullTypeStats() :
        fuel(0.0f),
        battle_speed(0.0f),
        starlane_speed(0.0f),
        stealth(0.0f),
        structure(0.0f)
    {}

    HullTypeStats(float fuel_,
                  float battle_speed_,
                  float starlane_speed_,
                  float stealth_,
                  float structure_) :
        fuel(fuel_),
        battle_speed(battle_speed_),
        starlane_speed(starlane_speed_),
        stealth(stealth_),
        structure(structure_)
    {}

    float   fuel;
    float   battle_speed;
    float   starlane_speed;
    float   stealth;
    float   structure;

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
class FO_COMMON_API HullType {
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
        m_battle_speed(1.0f),
        m_starlane_speed(1.0f),
        m_fuel(0.0f),
        m_stealth(0.0f),
        m_structure(0.0f),
        m_production_cost(0),
        m_production_time(0),
        m_producible(false),
        m_slots(),
        m_tags(),
        m_location(0),
        m_effects(),
        m_graphic(),
        m_icon()
    {}

    HullType(const std::string& name, const std::string& description,
             float fuel, float battle_speed, float starlane_speed,
             float stealth, float structure,
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

    float               BattleSpeed() const     { return m_battle_speed; }      ///< returns battle speed of hull
    float               StarlaneSpeed() const   { return m_starlane_speed; }    ///< returns starlane speed of hull
    float               Fuel() const            { return m_fuel; }              ///< returns fuel capacity of hull
    float               Stealth() const         { return m_stealth; }           ///< returns stealth of hull
    float               Structure() const       { return m_structure; }         ///< returns structure of hull
    float               Shields() const         { return 0.0f; }                ///< returns shields of hull
    float               ColonyCapacity() const  { return 0.0f; }                ///< returns colonist capacity of hull
    float               TroopCapacity() const   { return 0.0f; }                ///< returns the troop capacity of hull
    float               Detection() const       { return 0.0f; }                ///< returns detection ability of hull

    bool                ProductionCostTimeLocationInvariant() const;            ///< returns true if the production cost and time are invariant (does not depend on) the location
    float               ProductionCost(int empire_id, int location_id) const;   ///< returns the number of production points required to produce this hull
    int                 ProductionTime(int empire_id, int location_id) const;   ///< returns the number of turns required to produce this hull
    bool                Producible() const      { return m_producible; }        ///< returns whether this hull type is producible by players and appears on the design screen

    unsigned int        NumSlots() const        { return m_slots.size(); }      ///< returns total number of of slots in hull
    unsigned int        NumSlots(ShipSlotType slot_type) const;                 ///< returns number of of slots of indicated type in hull
    const std::vector<Slot>&    Slots() const   { return m_slots; }             ///< returns vector of slots in hull

    const std::set<std::string>& Tags() const   { return m_tags; }

    const Condition::ConditionBase* Location() const
    { return m_location; }                                                      ///< returns the condition that determines the locations where ShipDesign containing hull can be produced
    const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& Effects() const
    { return m_effects; }                                                       ///< returns the EffectsGroups that encapsulate the effects this part hull has

    const std::string&  Graphic() const         { return m_graphic; }           ///< returns the image that represents the hull on the design screen
    const std::string&  Icon() const            { return m_icon; }              ///< returns the small icon to represent hull
    //@}

private:
    void                Init(const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects);

    std::string                             m_name;
    std::string                             m_description;
    float                                   m_battle_speed;
    float                                   m_starlane_speed;
    float                                   m_fuel;
    float                                   m_stealth;
    float                                   m_structure;
    const ValueRef::ValueRefBase<double>*   m_production_cost;
    const ValueRef::ValueRefBase<int>*      m_production_time;
    bool                                    m_producible;
    std::vector<Slot>                       m_slots;
    std::set<std::string>                   m_tags;
    const Condition::ConditionBase*         m_location;
    std::vector<boost::shared_ptr<const Effect::EffectsGroup> >
                                            m_effects;
    std::string                             m_graphic;
    std::string                             m_icon;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Holds FreeOrion hull types */
class FO_COMMON_API HullTypeManager {
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
FO_COMMON_API const HullTypeManager& GetHullTypeManager();

/** Returns the ship HullType specification object with name \a name.  If no such HullType exists,
  * 0 is returned instead. */
FO_COMMON_API const HullType* GetHullType(const std::string& name);

class FO_COMMON_API ShipDesign {
public:
    /** \name Structors */ //@{
    ShipDesign();
    ShipDesign(const std::string& name, const std::string& description,
               int designed_on_turn, int designed_by_empire, const std::string& hull,
               const std::vector<std::string>& parts,
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
    void                            SetName(const std::string& name);

    /** returns description of design.  if \a stringtable_lookup is true and
      * the design was constructed specifying name_desc_in_stringtable true,
      * the description string is looked up in the stringtable before being
      * returned.  otherwise, the raw name string is returned. */
    const std::string&              Description(bool stringtable_lookup = true) const;
    void                            SetDescription(const std::string& description);

    int                             DesignedOnTurn() const  { return m_designed_on_turn; };     ///< returns turn on which design was created
    int                             DesignedByEmpire() const { return m_designed_by_empire; };  ///< returns id of empire that created this design

    bool                            ProductionCostTimeLocationInvariant() const;                ///< returns true if the production cost and time are invariant (does not depend on) the location
    float                           ProductionCost(int empire_id, int location_id) const;       ///< returns the total cost to build a ship of this design
    float                           PerTurnCost(int empire_id, int location_id) const;          ///< returns the maximum per-turn number of production points that can be spent on building a ship of this design
    int                             ProductionTime(int empire_id, int location_id) const;       ///< returns the time in turns it takes to build a ship of this design
    bool                            Producible() const      { return m_producible; }            ///< returns whether this design is producible by players and appears on the production screen list

    float                           BattleSpeed() const     { return m_battle_speed; }          ///< returns design speed on the battle map
    float                           StarlaneSpeed() const   { return m_starlane_speed; }        ///< returns design speed along starlanes

    float                           Structure() const       { return m_structure; }             ///< returns the max structure of this design
    float                           Shields() const         { return m_shields; }               ///< returns the max shields of this design
    float                           Fuel() const            { return m_fuel; }                  ///< returns the max fuel capacity of this design
    float                           Detection() const       { return m_detection; }             ///< returns the detection ability of this design
    float                           ColonyCapacity() const  { return m_colony_capacity; }       ///< returns the colonization capacity of this design
    float                           TroopCapacity() const   { return m_troop_capacity; }        ///< returns the troop capacity of this design
    float                           Stealth() const         { return m_stealth; }               ///< returns the stealth of this design

    float                           IndustryGeneration() const { return m_industry_generation; }///< returns the industry output from this ship design
    float                           ResearchGeneration() const { return m_research_generation; }///< returns the research output from this ship design
    float                           TradeGeneration() const    { return m_trade_generation; }   ///< returns the trade output from this ship design
    bool                            IsProductionLocation() const { return m_is_production_location;}///< returns true if this ship design can be a production location

    bool                            CanColonize() const;
    bool                            HasTroops() const       { return (m_troop_capacity > 0.0f); }
    bool                            CanBombard() const      { return m_can_bombard; }
    bool                            IsArmed() const         { return m_is_armed; }
    bool                            IsMonster() const       { return m_is_monster; }

    /////// TEMPORARY ///////
    float  Defense() const;
    float  Attack() const;
    float  AdjustedAttack(float shield) const;
    /////// TEMPORARY ///////


    const std::string&              Hull() const            { return m_hull; }      ///< returns name of hull on which design is based
    const HullType*                 GetHull() const
    { return GetHullTypeManager().GetHullType(m_hull); }                            ///< returns HullType on which design is based

    const std::vector<std::string>& Parts() const           { return m_parts; }     ///< returns vector of names of all parts in this design, with position in vector corresponding to slot positions
    std::vector<std::string>        Parts(ShipSlotType slot_type) const;            ///< returns vector of names of parts in slots of indicated type in this design, unrelated to slot positions
    std::vector<std::string>        Weapons() const;                                ///< returns vector of names of weapon parts in, unrelated to slot positions

    std::vector<std::string>        Tags() const;                                   ///< returns tags that would apply to ships of this design, from parts or the hull (but not from species or any other source of tags)

    const std::string&              Icon() const            { return m_icon; }      ///< returns filename for small-size icon graphic for design
    const std::string&              Model() const           { return m_3D_model; }  ///< returns filename of 3D model that represents ships of design
    bool                            LookupInStringtable() const { return m_name_desc_in_stringtable; }

    std::string                     Dump() const;                                   ///< returns a data file format representation of this object

    friend bool operator ==(const ShipDesign& first, const ShipDesign& second);
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

    int                         m_designed_on_turn;
    int                         m_designed_by_empire;

    std::string                 m_hull;
    std::vector<std::string>    m_parts;
    bool                        m_is_monster;

    std::string                 m_icon;
    std::string                 m_3D_model;

    bool                        m_name_desc_in_stringtable;

    // Note that these are fine to compute on demand and cache here -- it is
    // not necessary to serialize them.
    bool    m_is_armed;
    bool    m_can_bombard;
    float   m_detection;
    float   m_colony_capacity;
    float   m_troop_capacity;
    float   m_stealth;
    float   m_fuel;
    float   m_shields;
    float   m_structure;
    float   m_battle_speed;
    float   m_starlane_speed;
    float   m_research_generation;
    float   m_industry_generation;
    float   m_trade_generation;
    bool    m_is_production_location;

    bool    m_producible;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

///< Returns true if the two designs have the same hull and parts.
FO_COMMON_API bool operator ==(const ShipDesign& first, const ShipDesign& second);

/** Returns the ShipDesign specification object with id \a ship_design_id.  If
  * no such ShipDesign is present in the Universe (because it doesn't exist,
  * or isn't know to this client), 0 is returned instead. */
FO_COMMON_API const ShipDesign* GetShipDesign(int ship_design_id);


class FO_COMMON_API PredefinedShipDesignManager {
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

    /** Returns the ID for the design in the Universe for the predefined design
      * with the specified \a name.  If there is generic design available for
      * the specified \a name, then INVALID_DESIGN_ID is returned. */
    int                 GetDesignID(const std::string& name) const;
    //@}

    /** Adds designs in this manager to the specified \a empire using that
      * Empire's AddShipDesign(ShipDesign*) function.  Only designs specified in
      * \a design_names are added; others in this manager are not. */
    void                AddShipDesignsToEmpire(Empire* empire, const std::vector<std::string>& design_names) const;

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
const FO_COMMON_API PredefinedShipDesignManager& GetPredefinedShipDesignManager();

/** Returns the predefined ShipDesign with the name \a name.  If no such
  * ship design exists, 0 is returned instead. */
FO_COMMON_API const ShipDesign* GetPredefinedShipDesign(const std::string& name);


// template implementations
template <class Archive>
void PartType::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_description)
        & BOOST_SERIALIZATION_NVP(m_class)
        & BOOST_SERIALIZATION_NVP(m_capacity)
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
