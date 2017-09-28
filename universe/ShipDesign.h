#ifndef _ShipDesign_h_
#define _ShipDesign_h_


#include "ValueRefFwd.h"

#include <map>
#include <memory>
#include <set>
#include <unordered_map>
#include <string>
#include <vector>
#include <stdexcept>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/functional/hash.hpp>
#include <boost/optional/optional.hpp>
#include <boost/variant.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/nil_generator.hpp>

#include "EnumsFwd.h"

#include "../util/Export.h"

FO_COMMON_API extern const int INVALID_OBJECT_ID;
namespace Condition {
    struct ConditionBase;
}
namespace Effect {
    class EffectsGroup;
}
class Empire;

/** Common parameters for PartType and HullType constructors.  Used as temporary
  * storage for parsing to reduce number of sub-items parsed per item. */
struct CommonParams {
    CommonParams() :
        production_cost(nullptr),
        production_time(nullptr),
        producible(false),
        tags(),
        production_meter_consumption(),
        production_special_consumption(),
        location(nullptr),
        enqueue_location(nullptr),
        effects()
    {}
    CommonParams(ValueRef::ValueRefBase<double>* production_cost_,
                 ValueRef::ValueRefBase<int>* production_time_,
                 bool producible_,
                 const std::set<std::string>& tags_,
                 Condition::ConditionBase* location_,
                 const std::vector<std::shared_ptr<Effect::EffectsGroup>>& effects_,
                 std::map<MeterType, std::pair<ValueRef::ValueRefBase<double>*, Condition::ConditionBase*>> production_meter_consumption_,
                 std::map<std::string, std::pair<ValueRef::ValueRefBase<double>*, Condition::ConditionBase*>> production_special_consumption_,
                 Condition::ConditionBase* enqueue_location_) :
        production_cost(production_cost_),
        production_time(production_time_),
        producible(producible_),
        tags(),
        production_meter_consumption(production_meter_consumption_),
        production_special_consumption(production_special_consumption_),
        location(location_),
        enqueue_location(enqueue_location_),
        effects(effects_)
    {
        for (const std::string& tag : tags_)
            tags.insert(boost::to_upper_copy<std::string>(tag));
    }

    ValueRef::ValueRefBase<double>*                         production_cost;
    ValueRef::ValueRefBase<int>*                            production_time;
    bool                                                    producible;
    std::set<std::string>                                   tags;
    std::map<MeterType, std::pair<ValueRef::ValueRefBase<double>*, Condition::ConditionBase*>>
                                                            production_meter_consumption;
    std::map<std::string, std::pair<ValueRef::ValueRefBase<double>*, Condition::ConditionBase*>>
                                                            production_special_consumption;
    Condition::ConditionBase*                               location;
    Condition::ConditionBase*                               enqueue_location;
    std::vector<std::shared_ptr<Effect::EffectsGroup>> effects;
};

struct MoreCommonParams {
    MoreCommonParams() :
        name(),
        description(),
        exclusions()
    {}
    MoreCommonParams(const std::string& name_, const std::string& description_,
                     const std::set<std::string>& exclusions_) :
        name(name_),
        description(description_),
        exclusions(exclusions_)
    {}
    std::string             name;
    std::string             description;
    std::set<std::string>   exclusions;
};

/** A type of ship part */
class FO_COMMON_API PartType {
public:
    /** \name Structors */ //@{
    PartType();
    PartType(ShipPartClass part_class, double capacity, double stat2,
             const CommonParams& common_params, const MoreCommonParams& more_common_params,
             std::vector<ShipSlotType> mountable_slot_types,
             const std::string& icon, bool add_standard_capacity_effect = true);

    ~PartType();
    //@}

    /** \name Accessors */ //@{
    const std::string&      Name() const            { return m_name; };             ///< returns name of part
    const std::string&      Description() const     { return m_description; }       ///< returns description string, generally a UserString key.
    ShipPartClass           Class() const           { return m_class; }             ///< returns that class of part that this is.
    float                   Capacity() const;
    std::string             CapacityDescription() const;                            ///< returns a translated description of the part capacity, with numeric value
    float                   SecondaryStat() const;

    bool                    CanMountInSlotType(ShipSlotType slot_type) const;       ///< returns true if this part can be placed in a slot of the indicated type
    const std::vector<ShipSlotType>&
                            MountableSlotTypes() const  { return m_mountable_slot_types; }

    bool                    ProductionCostTimeLocationInvariant() const;            ///< returns true if the production cost and time are invariant (does not depend on) the location
    float                   ProductionCost(int empire_id, int location_id) const;   ///< returns the number of production points required to produce this part
    int                     ProductionTime(int empire_id, int location_id) const;   ///< returns the number of turns required to produce this part
    bool                    Producible() const      { return m_producible; }        ///< returns whether this part type is producible by players and appears on the design screen

    const std::map<MeterType, std::pair<ValueRef::ValueRefBase<double>*, Condition::ConditionBase*>>&
                            ProductionMeterConsumption() const  { return m_production_meter_consumption; }
    const std::map<std::string, std::pair<ValueRef::ValueRefBase<double>*, Condition::ConditionBase*>>&
                            ProductionSpecialConsumption() const{ return m_production_special_consumption; }

    const std::set<std::string>& Tags() const       { return m_tags; }
    const Condition::ConditionBase* Location() const{ return m_location; }          ///< returns the condition that determines the locations where ShipDesign containing part can be produced
    const std::set<std::string>& Exclusions() const { return m_exclusions; }        ///< returns the names of other content that cannot be used in the same ship design as this part

    /** Returns the EffectsGroups that encapsulate the effects this part has. */
    const std::vector<std::shared_ptr<Effect::EffectsGroup>>& Effects() const
    { return m_effects; }

    const std::string&      Icon() const            { return m_icon; }              ///< returns icon graphic that represents part in UI

    /** Returns a number, calculated from the contained data, which should be
      * different for different contained data, and must be the same for
      * the same contained data, and must be the same on different platforms
      * and executions of the program and the function. Useful to verify that
      * the parsed content is consistent without sending it all between
      * clients and server. */
    unsigned int GetCheckSum() const;
    //@}

private:
    void Init(const std::vector<std::shared_ptr<Effect::EffectsGroup>>& effects);

    std::string                     m_name;
    std::string                     m_description;
    ShipPartClass                   m_class;
    float                           m_capacity;
    float                           m_secondary_stat;   // damage for a hangar bay, shots per turn for a weapon, etc.
    ValueRef::ValueRefBase<double>* m_production_cost;
    ValueRef::ValueRefBase<int>*    m_production_time;
    bool                            m_producible;
    std::vector<ShipSlotType>       m_mountable_slot_types;
    std::set<std::string>           m_tags;
    std::map<MeterType, std::pair<ValueRef::ValueRefBase<double>*, Condition::ConditionBase*>>
                                    m_production_meter_consumption;
    std::map<std::string, std::pair<ValueRef::ValueRefBase<double>*, Condition::ConditionBase*>>
                                    m_production_special_consumption;
    Condition::ConditionBase*       m_location;
    std::set<std::string>           m_exclusions;
    std::vector<std::shared_ptr<Effect::EffectsGroup>> m_effects;
    std::string                     m_icon;
    bool                            m_add_standard_capacity_effect;

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

    /** Returns a number, calculated from the contained data, which should be
      * different for different contained data, and must be the same for
      * the same contained data, and must be the same on different platforms
      * and executions of the program and the function. Useful to verify that
      * the parsed content is consistent without sending it all between
      * clients and server. */
    unsigned int GetCheckSum() const;
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
        speed(0.0f),
        stealth(0.0f),
        structure(0.0f)
    {}

    HullTypeStats(float fuel_,
                  float speed_,
                  float stealth_,
                  float structure_) :
        fuel(fuel_),
        speed(speed_),
        stealth(stealth_),
        structure(structure_)
    {}

    float   fuel;
    float   speed;
    float   stealth;
    float   structure;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar  & BOOST_SERIALIZATION_NVP(fuel)
            & BOOST_SERIALIZATION_NVP(speed)
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
        Slot();
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
        m_speed(1.0f),
        m_fuel(0.0f),
        m_stealth(0.0f),
        m_structure(0.0f),
        m_production_cost(nullptr),
        m_production_time(nullptr),
        m_producible(false),
        m_slots(),
        m_tags(),
        m_production_meter_consumption(),
        m_production_special_consumption(),
        m_location(nullptr),
        m_effects(),
        m_graphic(),
        m_icon()
    {}

    HullType(const HullTypeStats& stats, const CommonParams& common_params,
             const MoreCommonParams& more_common_params,
             const std::vector<Slot>& slots,
             const std::string& icon, const std::string& graphic) :
        m_name(more_common_params.name),
        m_description(more_common_params.description),
        m_speed(stats.speed),
        m_fuel(stats.fuel),
        m_stealth(stats.stealth),
        m_structure(stats.structure),
        m_production_cost(common_params.production_cost),
        m_production_time(common_params.production_time),
        m_producible(common_params.producible),
        m_slots(slots),
        m_tags(),
        m_production_meter_consumption(common_params.production_meter_consumption),
        m_production_special_consumption(common_params.production_special_consumption),
        m_location(common_params.location),
        m_exclusions(more_common_params.exclusions),
        m_effects(),
        m_graphic(graphic),
        m_icon(icon)
    {
        //TraceLogger() << "hull type: " << m_name << " producible: " << m_producible << std::endl;
        Init(common_params.effects);
        for (const std::string& tag : common_params.tags)
            m_tags.insert(boost::to_upper_copy<std::string>(tag));
    }

    ~HullType();
    //@}

    /** \name Accessors */ //@{
    const std::string&  Name() const            { return m_name; }              ///< returns name of hull
    const std::string&  Description() const     { return m_description; }       ///< returns description, including a description of the stats and effects of this hull

    float               Speed() const;                                          ///< returns starlane speed of hull
    float               Fuel() const            { return m_fuel; }              ///< returns fuel capacity of hull
    float               Stealth() const         { return m_stealth; }           ///< returns stealth of hull
    float               Structure() const;                                      ///< returns structure of hull
    float               Shields() const         { return 0.0f; }                ///< returns shields of hull
    float               ColonyCapacity() const  { return 0.0f; }                ///< returns colonist capacity of hull
    float               TroopCapacity() const   { return 0.0f; }                ///< returns the troop capacity of hull
    float               Detection() const       { return 0.0f; }                ///< returns detection ability of hull

    bool                ProductionCostTimeLocationInvariant() const;            ///< returns true if the production cost and time are invariant (does not depend on) the location
    float               ProductionCost(int empire_id, int location_id) const;   ///< returns the number of production points required to produce this hull
    int                 ProductionTime(int empire_id, int location_id) const;   ///< returns the number of turns required to produce this hull
    bool                Producible() const      { return m_producible; }        ///< returns whether this hull type is producible by players and appears on the design screen

    const std::map<MeterType, std::pair<ValueRef::ValueRefBase<double>*, Condition::ConditionBase*>>&
                        ProductionMeterConsumption() const  { return m_production_meter_consumption; }
    const std::map<std::string, std::pair<ValueRef::ValueRefBase<double>*, Condition::ConditionBase*>>&
                        ProductionSpecialConsumption() const{ return m_production_special_consumption; }

    unsigned int        NumSlots() const        { return m_slots.size(); }      ///< returns total number of of slots in hull
    unsigned int        NumSlots(ShipSlotType slot_type) const;                 ///< returns number of of slots of indicated type in hull
    const std::vector<Slot>& Slots() const      { return m_slots; }             ///< returns vector of slots in hull

    const std::set<std::string>& Tags() const   { return m_tags; }

    bool HasTag(const std::string& tag) const   { return m_tags.count(tag) != 0; }

    const Condition::ConditionBase* Location() const{ return m_location; }      ///< returns the condition that determines the locations where ShipDesign containing hull can be produced
    const std::set<std::string>& Exclusions() const { return m_exclusions; }    ///< returns the names of other content that cannot be used in the same ship design as this part

    /** Returns the EffectsGroups that encapsulate the effects this part hull
        has. */
    const std::vector<std::shared_ptr<Effect::EffectsGroup>>& Effects() const
    { return m_effects; }

    const std::string&  Graphic() const         { return m_graphic; }           ///< returns the image that represents the hull on the design screen
    const std::string&  Icon() const            { return m_icon; }              ///< returns the small icon to represent hull

    /** Returns a number, calculated from the contained data, which should be
      * different for different contained data, and must be the same for
      * the same contained data, and must be the same on different platforms
      * and executions of the program and the function. Useful to verify that
      * the parsed content is consistent without sending it all between
      * clients and server. */
    unsigned int GetCheckSum() const;
    //@}

private:
    void Init(const std::vector<std::shared_ptr<Effect::EffectsGroup>>& effects);

    std::string                                             m_name;
    std::string                                             m_description;
    float                                                   m_speed;
    float                                                   m_fuel;
    float                                                   m_stealth;
    float                                                   m_structure;
    ValueRef::ValueRefBase<double>*                         m_production_cost;
    ValueRef::ValueRefBase<int>*                            m_production_time;
    bool                                                    m_producible;
    std::vector<Slot>                                       m_slots;
    std::set<std::string>                                   m_tags;
    std::map<MeterType, std::pair<ValueRef::ValueRefBase<double>*, Condition::ConditionBase*>>
                                                            m_production_meter_consumption;
    std::map<std::string, std::pair<ValueRef::ValueRefBase<double>*, Condition::ConditionBase*>>
                                                            m_production_special_consumption;
    Condition::ConditionBase*                               m_location;
    std::set<std::string>                                   m_exclusions;
    std::vector<std::shared_ptr<Effect::EffectsGroup>>      m_effects;
    std::string                                             m_graphic;
    std::string                                             m_icon;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

namespace CheckSums {
    FO_COMMON_API void CheckSumCombine(unsigned int& sum, const HullType::Slot& slot);
}

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

    /** Returns a number, calculated from the contained data, which should be
      * different for different contained data, and must be the same for
      * the same contained data, and must be the same on different platforms
      * and executions of the program and the function. Useful to verify that
      * the parsed content is consistent without sending it all between
      * clients and server. */
    unsigned int GetCheckSum() const;
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
private:
    /** The ShipDesign() constructor constructs invalid designs and is only used by boost
        serialization. */
    ShipDesign();
public:
    /** The public ShipDesign constructor will only construct valid ship
        designs, as long as the HullTypeManager has at least one hull.

        If \p should_throw is not boost::none and the passed in parameters (\p
        hull and \p parts) would result in an invalid design it generates an
        explicit log message showing the FOCS corresponding to the passed in
        parameters and the FOCS corresponding to a corrected valid design and
        then throws std::invalid_argument.  This can be used to test design
        validity and provide an explcit log message.

        should_throw is not used but it is a literal reminder that
        std::invalid_argument should be caught.

        If \p should_throw is boost::none it will correct the errors in
        parameters, print a log message and return a valid ship design.  The
        only exception is if there are no ship hull in the HullManager.  Then
        the constructor will not throw, but will return an invalid ship design
        with a empty "" hull.
    */
    ShipDesign(const boost::optional<std::invalid_argument>& should_throw,
               const std::string& name, const std::string& description,
               int designed_on_turn, int designed_by_empire, const std::string& hull,
               const std::vector<std::string>& parts,
               const std::string& icon, const std::string& model,
               bool name_desc_in_stringtable = false, bool monster = false,
               const boost::uuids::uuid& uuid = boost::uuids::nil_uuid()
              );

    /**  The public ShipDesign constructor will only construct valid ship
         designs, as long as the HullTypeManager has at least one hull. */
    ShipDesign(const std::string& name, const std::string& description,
               int designed_on_turn, int designed_by_empire, const std::string& hull,
               const std::vector<std::string>& parts,
               const std::string& icon, const std::string& model,
               bool name_desc_in_stringtable = false, bool monster = false,
               const boost::uuids::uuid& uuid = boost::uuids::nil_uuid()
              );
    //@}

    /** \name Accessors */ //@{
    int                             ID() const  { return m_id; }    ///< returns id number of design
    /** returns name of design.  if \a stringtable_lookup is true and the
      * design was constructed specifying name_desc_in_stringtable true,
      * the name string is looked up in the stringtable before being returned.
      * otherwise, the raw name string is returned. */
    const std::string&              Name(bool stringtable_lookup = true) const;
    void                            SetName(const std::string& name);

    /** Return the UUID. */
    boost::uuids::uuid              UUID() const { return m_uuid; }

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

    float                           Speed() const           { return m_speed; }                 ///< returns design speed along starlanes

    float                           Structure() const       { return m_structure; }             ///< returns the max structure of this design
    float                           Shields() const         { return m_shields; }               ///< returns the max shields of this design
    float                           Fuel() const            { return m_fuel; }                  ///< returns the max fuel capacity of this design
    float                           Detection() const       { return m_detection; }             ///< returns the detection ability of this design
    float                           ColonyCapacity() const  { return m_colony_capacity; }       ///< returns the colonization capacity of this design
    float                           TroopCapacity() const   { return m_troop_capacity; }        ///< returns the troop capacity of this design
    float                           Stealth() const         { return m_stealth; }               ///< returns the stealth of this design

    float                           IndustryGeneration() const  { return m_industry_generation; }   ///< returns the industry output from this ship design
    float                           ResearchGeneration() const  { return m_research_generation; }   ///< returns the research output from this ship design
    float                           TradeGeneration() const     { return m_trade_generation; }      ///< returns the trade output from this ship design
    bool                            IsProductionLocation() const{ return m_is_production_location;} ///< returns true if this ship design can be a production location

    bool                            CanColonize() const;
    bool                            HasTroops() const       { return (m_troop_capacity > 0.0f); }
    bool                            CanBombard() const      { return m_can_bombard; }
    bool                            IsArmed() const         { return m_is_armed; }
    bool                            HasFighters() const     { return m_has_fighters; }
    bool                            IsMonster() const       { return m_is_monster; }

    float                           Attack() const;
    float                           AdjustedAttack(float shield) const;
    float                           Defense() const;

    const std::string&              Hull() const            { return m_hull; }      ///< returns name of hull on which design is based
    const HullType*                 GetHull() const
    { return GetHullTypeManager().GetHullType(m_hull); }                            ///< returns HullType on which design is based

    const std::vector<std::string>& Parts() const           { return m_parts; }     ///< returns vector of names of all parts in this design, with position in vector corresponding to slot positions
    std::vector<std::string>        Parts(ShipSlotType slot_type) const;            ///< returns vector of names of parts in slots of indicated type in this design, unrelated to slot positions
    std::vector<std::string>        Weapons() const;                                ///< returns vector of names of weapon parts in, unrelated to slot positions

    const std::string&              Icon() const            { return m_icon; }      ///< returns filename for small-size icon graphic for design
    const std::string&              Model() const           { return m_3D_model; }  ///< returns filename of 3D model that represents ships of design
    bool                            LookupInStringtable() const { return m_name_desc_in_stringtable; }

    /** returns number of parts in this ship design, indexed by PartType name */
    const std::map<std::string, int>&     PartTypeCount() const { return m_num_part_types; }

    /** returns number of parts in this ship design, indexed by ShipPartClass */
    const std::map<ShipPartClass, int>&   PartClassCount() const { return m_num_part_classes; }

    std::string                     Dump() const;                                   ///< returns a data file format representation of this object

    /** Returns a number, calculated from the contained data, which should be
      * different for different contained data, and must be the same for
      * the same contained data, and must be the same on different platforms
      * and executions of the program and the function. Useful to verify that
      * the parsed content is consistent without sending it all between
      * clients and server. */
    unsigned int                    GetCheckSum() const;

    friend FO_COMMON_API bool operator ==(const ShipDesign& first, const ShipDesign& second);
    //@}

    bool                            ProductionLocation(int empire_id, int location_id) const;   ///< returns true iff the empire with ID empire_id can produce this design at the location with location_id

    /** \name Mutators */ //@{
    void                            SetID(int id);                          ///< sets the ID number of the design to \a id .  Should only be used by Universe class when inserting new design into Universe.
    /** Set the UUID. */
    void                            SetUUID(const boost::uuids::uuid& uuid);
    void                            Rename(const std::string& name) { m_name = name; }  ///< renames this design to \a name
    void                            SetMonster(const bool is_monster) {m_is_monster = is_monster; }
    //@}

    /** Return true if \p hull and \p parts would make a valid design. */
    static bool ValidDesign(const std::string& hull, const std::vector<std::string>& parts);
private:
    /** Return a valid hull and parts pair iff the \p hull and \p parts vectors
        would not make a valid ShipDesign.
        Also pad parts with "" if it is shorter than the \p hull number of slots.
        Otherwise return none. If \p produce_log is true then produce log messages. */
    static boost::optional<std::pair<std::string, std::vector<std::string>>>
        MaybeInvalidDesign(const std::string& hull, std::vector<std::string>& parts, bool produce_log);

    /** Force design invariants to be true. If design invariants are not begin
        met and \p produce_log is true provide an explicit log message about how it
        was corrected and throw std::invalid_argument if \p should_throw is not
        none.

        \p should_throw is not used but it is a literal reminder that
        std::invalid_argument should be caught.
    */
    void ForceValidDesignOrThrow(const boost::optional<std::invalid_argument>& should_throw, bool produce_log);

    void BuildStatCaches();

    int                         m_id = INVALID_OBJECT_ID;

    std::string                 m_name;
    std::string                 m_description;
    boost::uuids::uuid          m_uuid;

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
    bool    m_is_armed = false;
    bool    m_has_fighters = false;
    bool    m_can_bombard = false;
    float   m_detection = 0.0f;
    float   m_colony_capacity = 0.0f;
    float   m_troop_capacity = 0.0f;
    float   m_stealth = 0.0f;
    float   m_fuel = 0.0f;
    float   m_shields = 0.0f;
    float   m_structure = 0.0f;
    float   m_speed = 0.0f;
    float   m_research_generation = 0.0f;
    float   m_industry_generation = 0.0f;
    float   m_trade_generation = 0.0f;
    bool    m_is_production_location = false;
    std::map<std::string, int>      m_num_part_types;
    std::map<ShipPartClass, int>    m_num_part_classes;
    bool    m_producible = false;

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
    /** Return pointers the ShipDesigns in order.*/
    std::vector<const ShipDesign*> GetOrderedShipDesigns() const;

    /** Return pointers the ShipDesigns in order.*/
    std::vector<const ShipDesign*> GetOrderedMonsterDesigns() const;

    /** Returns the ID for the design in the Universe for the predefined design
      * with the specified \a name.  If there is generic design available for
      * the specified \a name, then INVALID_DESIGN_ID is returned. */
    int                 GetDesignID(const std::string& name) const;

    /** Returns a number, calculated from the contained data, which should be
      * different for different contained data, and must be the same for
      * the same contained data, and must be the same on different platforms
      * and executions of the program and the function. Useful to verify that
      * the parsed content is consistent without sending it all between
      * clients and server. */
    unsigned int        GetCheckSum() const;
    //@}

    /** Adds designs in this manager to the universe with the design creator
      * left as no empire. */
    void AddShipDesignsToUniverse() const;

    /** Returns the predefined ShipDesign with the name \a name.  If no such
      * ship design exists, 0 is returned instead. */
    static PredefinedShipDesignManager& GetPredefinedShipDesignManager();

private:
    PredefinedShipDesignManager();

    std::unordered_map<boost::uuids::uuid, std::unique_ptr<ShipDesign>,
                       boost::hash<boost::uuids::uuid>> m_designs;

    std::unordered_map<std::string, boost::uuids::uuid> m_name_to_ship_design;
    std::unordered_map<std::string, boost::uuids::uuid> m_name_to_monster_design;
    mutable std::unordered_map<std::string, int> m_design_generic_ids;  // ids of designs from this manager that have been added to the universe with no empire as the creator

    std::vector<boost::uuids::uuid> m_ship_ordering;
    std::vector<boost::uuids::uuid> m_monster_ordering;

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
        & BOOST_SERIALIZATION_NVP(m_secondary_stat)
        & BOOST_SERIALIZATION_NVP(m_production_cost)
        & BOOST_SERIALIZATION_NVP(m_production_time)
        & BOOST_SERIALIZATION_NVP(m_producible)
        & BOOST_SERIALIZATION_NVP(m_mountable_slot_types)
        & BOOST_SERIALIZATION_NVP(m_tags)
        & BOOST_SERIALIZATION_NVP(m_production_meter_consumption)
        & BOOST_SERIALIZATION_NVP(m_production_special_consumption)
        & BOOST_SERIALIZATION_NVP(m_location)
        & BOOST_SERIALIZATION_NVP(m_exclusions)
        & BOOST_SERIALIZATION_NVP(m_effects)
        & BOOST_SERIALIZATION_NVP(m_icon)
        & BOOST_SERIALIZATION_NVP(m_add_standard_capacity_effect);
}

template <class Archive>
void HullType::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_description)
        & BOOST_SERIALIZATION_NVP(m_speed)
        & BOOST_SERIALIZATION_NVP(m_fuel)
        & BOOST_SERIALIZATION_NVP(m_stealth)
        & BOOST_SERIALIZATION_NVP(m_structure)
        & BOOST_SERIALIZATION_NVP(m_production_cost)
        & BOOST_SERIALIZATION_NVP(m_production_time)
        & BOOST_SERIALIZATION_NVP(m_producible)
        & BOOST_SERIALIZATION_NVP(m_slots)
        & BOOST_SERIALIZATION_NVP(m_tags)
        & BOOST_SERIALIZATION_NVP(m_production_meter_consumption)
        & BOOST_SERIALIZATION_NVP(m_production_special_consumption)
        & BOOST_SERIALIZATION_NVP(m_location)
        & BOOST_SERIALIZATION_NVP(m_exclusions)
        & BOOST_SERIALIZATION_NVP(m_effects)
        & BOOST_SERIALIZATION_NVP(m_graphic)
        & BOOST_SERIALIZATION_NVP(m_icon);
}

/** Load all ship designs in \p dir and return a tuple is_error, the map from uuid to ship design and path and the
    ship ordering from the manifest. */
FO_COMMON_API std::tuple<
    bool,
    std::unordered_map<boost::uuids::uuid,
        std::pair<std::unique_ptr<ShipDesign>, boost::filesystem::path>,
        boost::hash<boost::uuids::uuid>>,
    std::vector<boost::uuids::uuid>>
LoadShipDesignsAndManifestOrderFromFileSystem(const boost::filesystem::path& dir);

#endif // _ShipDesign_h_
