#ifndef _ShipPart_h_
#define _ShipPart_h_


#include <boost/serialization/nvp.hpp>
#include <GG/Enum.h>
#include "CommonParams.h"
#include "../util/Pending.h"


FO_COMMON_API extern const int INVALID_DESIGN_ID;


//! Classifies ShipParts by general function.
GG_ENUM(ShipPartClass,
    INVALID_SHIP_PART_CLASS = -1,
    PC_DIRECT_WEAPON,       //!< direct-fire weapons
    PC_FIGHTER_BAY,         //!< launch aparatus for fighters, which are self-propelled platforms that function independently of ships in combat, but don't exist on the main game map
    PC_FIGHTER_HANGAR,      //!< storage for fighters, also determines their weapon strength stat
    PC_SHIELD,              //!< energy-based defense
    PC_ARMOUR,              //!< defensive material on hull of ship
    PC_TROOPS,              //!< ground troops, used to conquer planets
    PC_DETECTION,           //!< range of vision and seeing through stealth
    PC_STEALTH,             //!< hiding from enemies
    PC_FUEL,                //!< distance that can be traveled away from resupply
    PC_COLONY,              //!< transports colonists and allows ships to make new colonies
    PC_SPEED,               //!< affects ship speed on starlanes
    PC_GENERAL,             //!< special purpose parts that don't fall into another class
    PC_BOMBARD,             //!< permit orbital bombardment by ships against planets
    PC_INDUSTRY,            //!< generates production points for owner at its location
    PC_RESEARCH,            //!< generates research points for owner
    PC_INFLUENCE,           ///< generates influence points for owner
    PC_PRODUCTION_LOCATION, //!< allows production items to be produced at its location
    NUM_SHIP_PART_CLASSES
)


//! Describes an equipable part for a ship.
class FO_COMMON_API ShipPart {
public:
    ShipPart();

    ShipPart(ShipPartClass part_class, double capacity, double stat2,
             CommonParams& common_params,
             const std::string& name,
             const std::string& description,
             const std::set<std::string>& exclusions,
             std::vector<ShipSlotType> mountable_slot_types,
             const std::string& icon, bool add_standard_capacity_effect = true,
             std::unique_ptr<Condition::Condition>&& combat_targets = nullptr);

    ~ShipPart();

    //! Returns name of part
    auto Name() const -> const std::string&
    { return m_name; }

    //! Returns description string, generally a UserString key.
    auto Description() const -> const std::string&
    { return m_description; }

    //! Returns that class of part that this is.
    auto Class() const -> ShipPartClass
    { return m_class; }

    auto Capacity() const -> float;

    //! Returns a translated description of the part capacity, with numeric
    //! value
    auto CapacityDescription() const -> std::string;

    auto SecondaryStat() const -> float;

    //! Returns true if this part can be placed in a slot of the indicated type
    auto CanMountInSlotType(ShipSlotType slot_type) const -> bool;

    //! Returns the condition for possible targets. may be nullptr if no
    //! condition was specified.
    auto CombatTargets() const -> const Condition::Condition*
    { return m_combat_targets.get(); }

    auto MountableSlotTypes() const -> const std::vector<ShipSlotType>&
    { return m_mountable_slot_types; }

    //! Returns true if the production cost and time are invariant
    //! (does not depend on) the location
    auto ProductionCostTimeLocationInvariant() const -> bool;

    //! Returns the number of production points required to produce this part
    auto ProductionCost(int empire_id, int location_id, int in_design_id = INVALID_DESIGN_ID) const -> float;

    //! Returns the number of turns required to produce this part
    auto ProductionTime(int empire_id, int location_id, int in_design_id = INVALID_DESIGN_ID) const -> int;

    //! Returns whether this part type is producible by players and appears on
    //! the design screen
    auto Producible() const -> bool
    { return m_producible; }

    auto ProductionMeterConsumption() const -> const ConsumptionMap<MeterType>&
    { return m_production_meter_consumption; }

    auto ProductionSpecialConsumption() const -> const ConsumptionMap<std::string>&
    { return m_production_special_consumption; }

    auto Tags() const -> const std::set<std::string>&
    { return m_tags; }

    //! Returns the condition that determines the locations where ShipDesign
    //! containing part can be produced
    auto Location() const -> const Condition::Condition*
    { return m_location.get(); }

    //! Returns the names of other content that cannot be used in the same
    //! ship design as this part
    auto Exclusions() const -> const std::set<std::string>&
    { return m_exclusions; }

    //! Returns the EffectsGroups that encapsulate the effects this part has.
    auto Effects() const -> const std::vector<std::shared_ptr<Effect::EffectsGroup>>&
    { return m_effects; }

    //! Returns icon graphic that represents part in UI
    auto Icon() const -> const std::string&
    { return m_icon; }

    //! Returns a number, calculated from the contained data, which should be
    //! different for different contained data, and must be the same for
    //! the same contained data, and must be the same on different platforms
    //! and executions of the program and the function. Useful to verify that
    //! the parsed content is consistent without sending it all between
    //! clients and server.
    unsigned int GetCheckSum() const;
    //@}

private:
    void Init(std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects);

    std::string     m_name;
    std::string     m_description;
    ShipPartClass   m_class;
    float           m_capacity = 0.0f;
    //! Damage for a hangar bay, shots per turn for a weapon, etc.
    float           m_secondary_stat = 0.0f;
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
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};


template <typename Archive>
void ShipPart::serialize(Archive& ar, const unsigned int version)
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


//! Holds FreeOrion available ShipParts
class FO_COMMON_API ShipPartManager {
public:
    using ShipPartMap = std::map<std::string, std::unique_ptr<ShipPart>>;
    using iterator = ShipPartMap::const_iterator;

    //! Returns the ShipPart with the name @p name; you should use the free
    //! function GetShipPart() instead
    auto GetShipPart(const std::string& name) const -> const ShipPart*;

    //! Iterator to the first ShipPart
    auto begin() const -> iterator;

    //! Iterator to one after the last ShipPart.
    auto end() const -> iterator;

    //! Returns the instance of this singleton class; you should use the free
    //! function GetShipPartManager() instead.
    static auto GetShipPartManager() -> ShipPartManager&;

    //! Returns a number, calculated from the contained data, which should be
    //! different for different contained data, and must be the same for
    //! the same contained data, and must be the same on different platforms
    //! and executions of the program and the function. Useful to verify that
    //! the parsed content is consistent without sending it all between
    //! clients and server.
    auto GetCheckSum() const -> unsigned int;

    //! Sets part types to the future value of @p pending_ship_parts.
    FO_COMMON_API void SetShipParts(Pending::Pending<ShipPartMap>&& pending_ship_parts);

private:
    ShipPartManager();

    //! Assigns any m_pending_ship_parts to m_parts.
    void CheckPendingShipParts() const;

    //! Future that provides all ShipPart%s after loaded by the parser.
    mutable boost::optional<Pending::Pending<ShipPartMap>> m_pending_ship_parts = boost::none;

    //! Map of ShipPart::Name to ShipPart%s.
    mutable ShipPartMap m_parts;

    static ShipPartManager*  s_instance;
};


//! Returns the singleton ShipPart manager
FO_COMMON_API ShipPartManager& GetShipPartManager();


//! Returns the ShipPart specification object with name @p name.  If no
//! such ShipPart exists, nullptr is returned instead.
FO_COMMON_API const ShipPart* GetShipPart(const std::string& name);


#endif // _ShipPart_h_
