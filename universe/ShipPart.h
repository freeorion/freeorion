#ifndef _ShipPart_h_
#define _ShipPart_h_

#include <boost/serialization/nvp.hpp>

#include <GG/Enum.h>
#include "CommonParams.h"
#include "../util/Pending.h"


FO_COMMON_API extern const int INVALID_DESIGN_ID;


/** Types "classes" of ship parts */
GG_ENUM(ShipPartClass,
    INVALID_SHIP_PART_CLASS = -1,
    PC_DIRECT_WEAPON,       ///< direct-fire weapons
    PC_FIGHTER_BAY,         ///< launch aparatus for fighters, which are self-propelled platforms that function independently of ships in combat, but don't exist on the main game map
    PC_FIGHTER_HANGAR,      ///< storage for fighters, also determines their weapon strength stat
    PC_SHIELD,              ///< energy-based defense
    PC_ARMOUR,              ///< defensive material on hull of ship
    PC_TROOPS,              ///< ground troops, used to conquer planets
    PC_DETECTION,           ///< range of vision and seeing through stealth
    PC_STEALTH,             ///< hiding from enemies
    PC_FUEL,                ///< distance that can be traveled away from resupply
    PC_COLONY,              ///< transports colonists and allows ships to make new colonies
    PC_SPEED,               ///< affects ship speed on starlanes
    PC_GENERAL,             ///< special purpose parts that don't fall into another class
    PC_BOMBARD,             ///< permit orbital bombardment by ships against planets
    PC_INDUSTRY,            ///< generates production points for owner at its location
    PC_RESEARCH,            ///< generates research points for owner
    PC_TRADE,               ///< generates trade points for owner
    PC_PRODUCTION_LOCATION, ///< allows production items to be produced at its location
    NUM_SHIP_PART_CLASSES
)


/** A type of ship part */
class FO_COMMON_API PartType {
public:
    /** \name Structors */ //@{
    PartType();
    PartType(ShipPartClass part_class, double capacity, double stat2,
             CommonParams& common_params, const MoreCommonParams& more_common_params,
             std::vector<ShipSlotType> mountable_slot_types,
             const std::string& icon, bool add_standard_capacity_effect = true,
             std::unique_ptr<Condition::Condition>&& combat_targets = nullptr);

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
    const Condition::Condition*
                            CombatTargets() const { return m_combat_targets.get(); }///< returns the condition for possible targets. may be nullptr if no condition was specified.
    const std::vector<ShipSlotType>&
                            MountableSlotTypes() const { return m_mountable_slot_types; }

    bool                    ProductionCostTimeLocationInvariant() const;            ///< returns true if the production cost and time are invariant (does not depend on) the location
    float                   ProductionCost(int empire_id, int location_id, int in_design_id = INVALID_DESIGN_ID) const; ///< returns the number of production points required to produce this part
    int                     ProductionTime(int empire_id, int location_id, int in_design_id = INVALID_DESIGN_ID) const; ///< returns the number of turns required to produce this part
    bool                    Producible() const { return m_producible; }             ///< returns whether this part type is producible by players and appears on the design screen

    const ConsumptionMap<MeterType>&    ProductionMeterConsumption() const  { return m_production_meter_consumption; }
    const ConsumptionMap<std::string>&  ProductionSpecialConsumption() const{ return m_production_special_consumption; }

    const std::set<std::string>& Tags() const       { return m_tags; }
    const Condition::Condition* Location() const{ return m_location.get(); }          ///< returns the condition that determines the locations where ShipDesign containing part can be produced
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
    void Init(std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects);

    std::string     m_name;
    std::string     m_description;
    ShipPartClass   m_class;
    float           m_capacity = 0.0f;
    float           m_secondary_stat = 0.0f;    // damage for a hangar bay, shots per turn for a weapon, etc.
    bool            m_producible = false;

    std::unique_ptr<ValueRef::ValueRef<double>>         m_production_cost;
    std::unique_ptr<ValueRef::ValueRef<int>>            m_production_time;
    std::vector<ShipSlotType>                           m_mountable_slot_types;
    std::set<std::string>                               m_tags;
    ConsumptionMap<MeterType>                           m_production_meter_consumption;
    ConsumptionMap<std::string>                         m_production_special_consumption;
    std::unique_ptr<Condition::Condition>               m_location;
    std::set<std::string>                               m_exclusions;
    std::vector<std::shared_ptr<Effect::EffectsGroup>>  m_effects;
    std::string                                         m_icon;
    bool                                                m_add_standard_capacity_effect = false;
    std::unique_ptr<Condition::Condition>               m_combat_targets;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


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
        & BOOST_SERIALIZATION_NVP(m_add_standard_capacity_effect)
        & BOOST_SERIALIZATION_NVP(m_combat_targets);
}


/** Holds FreeOrion ship part types */
class FO_COMMON_API PartTypeManager {
public:
    using PartTypeMap = std::map<std::string, std::unique_ptr<PartType>>;
    using iterator = PartTypeMap::const_iterator;

    /** \name Accessors */ //@{
    /** returns the part type with the name \a name; you should use the free function GetPartType() instead */
    const PartType* GetPartType(const std::string& name) const;

    /** iterator to the first part type */
    iterator begin() const;

    /** iterator to the last + 1th part type */
    iterator end() const;

    /** returns the instance of this singleton class; you should use the free function GetPartTypeManager() instead */
    static PartTypeManager& GetPartTypeManager();

    /** Returns a number, calculated from the contained data, which should be
      * different for different contained data, and must be the same for
      * the same contained data, and must be the same on different platforms
      * and executions of the program and the function. Useful to verify that
      * the parsed content is consistent without sending it all between
      * clients and server. */
    unsigned int GetCheckSum() const;
    //@}

    /** Sets part types to the future value of \p pending_part_types. */
    FO_COMMON_API void SetPartTypes(Pending::Pending<PartTypeMap>&& pending_part_types);

private:
    PartTypeManager();

    /** Assigns any m_pending_part_types to m_bulding_types. */
    void CheckPendingPartTypes() const;

    /** Future part type being parsed by parser.  mutable so that it can
        be assigned to m_part_types when completed.*/
    mutable boost::optional<Pending::Pending<PartTypeMap>>      m_pending_part_types = boost::none;

    /** Set of part types.  mutable so that when the parse completes it can
        be updated. */
    mutable std::map<std::string, std::unique_ptr<PartType>>    m_parts;
    static PartTypeManager*                                     s_instance;
};


/** returns the singleton part type manager */
FO_COMMON_API PartTypeManager& GetPartTypeManager();


/** Returns the ship PartType specification object with name \a name.  If no
  * such PartType exists, 0 is returned instead. */
FO_COMMON_API const PartType* GetPartType(const std::string& name);


#endif // _ShipPart_h_
