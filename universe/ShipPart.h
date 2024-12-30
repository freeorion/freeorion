#ifndef _ShipPart_h_
#define _ShipPart_h_


#include "CommonParams.h"
#include "ConstantsFwd.h"
#include "../util/Enum.h"
#include "../util/Pending.h"

struct ScriptingContext;

//! Classifies ShipParts by general function.
FO_ENUM(
    (ShipPartClass),
    ((INVALID_SHIP_PART_CLASS, -1))
    ((PC_DIRECT_WEAPON))       //!< direct-fire weapons
    ((PC_FIGHTER_BAY))         //!< launch aparatus for fighters, which are self-propelled platforms that function independently of ships in combat, but don't exist on the main game map
    ((PC_FIGHTER_HANGAR))      //!< storage for fighters, also determines their weapon strength stat
    ((PC_SHIELD))              //!< energy-based defense
    ((PC_ARMOUR))              //!< defensive material on hull of ship
    ((PC_TROOPS))              //!< ground troops, used to conquer planets
    ((PC_DETECTION))           //!< range of vision and seeing through stealth
    ((PC_STEALTH))             //!< hiding from enemies
    ((PC_FUEL))                //!< distance that can be traveled away from resupply
    ((PC_COLONY))              //!< transports colonists and allows ships to make new colonies
    ((PC_SPEED))               //!< affects ship speed on starlanes
    ((PC_GENERAL))             //!< special purpose parts that don't fall into another class
    ((PC_BOMBARD))             //!< permit orbital bombardment by ships against planets
    ((PC_INDUSTRY))            //!< generates production points for owner at its location
    ((PC_RESEARCH))            //!< generates research points for owner
    ((PC_INFLUENCE))           //!< generates influence points for owner
    ((PC_PRODUCTION_LOCATION)) //!< allows production items to be produced at its location
    ((NUM_SHIP_PART_CLASSES))
)


//! Describes an equipable part for a ship.
class FO_COMMON_API ShipPart {
public:
    ShipPart() = delete;

    ShipPart(ShipPartClass part_class, double capacity, double stat2,
             CommonParams&& common_params, std::string&& name,
             std::string&& description, std::set<std::string>&& exclusions,
             std::vector<ShipSlotType> mountable_slot_types,
             std::string&& icon, bool add_standard_capacity_effect = true,
             std::unique_ptr<Condition::Condition>&& combat_targets = nullptr,
             std::unique_ptr<ValueRef::ValueRef<double>>&& total_fighter_damage = nullptr,
             std::unique_ptr<ValueRef::ValueRef<double>>&& total_ship_damage = nullptr);

    ~ShipPart();

    [[nodiscard]] bool operator==(const ShipPart& rhs) const;

    [[nodiscard]] auto& Name() const noexcept { return m_name; }
    [[nodiscard]] auto& Description() const noexcept { return m_description; }
    [[nodiscard]] auto Class() const noexcept { return m_class; }
    [[nodiscard]] auto Capacity() const -> float;

    //! translated description of the part capacity, with numeric value
    [[nodiscard]] std::string CapacityDescription() const;
    [[nodiscard]] float SecondaryStat() const;

    //! true if this part can be placed in a slot of the indicated type
    [[nodiscard]] bool CanMountInSlotType(ShipSlotType slot_type) const;

    //! value ref estimating maximum damage against fighters in a combat.
    //! may be nullptr if no value ref was specified
    [[nodiscard]] const auto* TotalFighterDamage() const noexcept { return m_total_fighter_damage.get(); }

    //! value ref estimating maximum damage against ships in a combat.
    //! may be nullptr if no value ref was specified
    [[nodiscard]] const auto* TotalShipDamage() const noexcept { return m_total_ship_damage.get(); }

    //! condition for possible targets. may be nullptr if no condition was specified.
    [[nodiscard]] const auto* CombatTargets() const noexcept { return m_combat_targets.get(); }

    [[nodiscard]] auto& MountableSlotTypes() const noexcept { return m_mountable_slot_types; }

    //! Returns true if the production cost and time are invariant
    //! (does not depend on) the location
    [[nodiscard]] auto ProductionCostTimeLocationInvariant() const -> bool;

    //! Returns the number of production points required to produce this part
    [[nodiscard]] auto ProductionCost(int empire_id, int location_id, const ScriptingContext& context,
                                      int in_design_id = INVALID_DESIGN_ID) const -> float;

    //! Returns the number of turns required to produce this part
    [[nodiscard]] auto ProductionTime(int empire_id, int location_id, const ScriptingContext& context,
                                      int in_design_id = INVALID_DESIGN_ID) const -> int;

    //! Returns whether this part type is producible by players and appears on the design screen
    [[nodiscard]] auto Producible() const noexcept { return m_producible; }

    [[nodiscard]] auto& ProductionMeterConsumption() const noexcept { return m_production_meter_consumption; }
    [[nodiscard]] auto& ProductionSpecialConsumption() const noexcept { return m_production_special_consumption; }

    [[nodiscard]] const auto& Tags() const noexcept { return m_tags; }
    [[nodiscard]] const auto& PediaTags() const noexcept { return m_pedia_tags; }

    [[nodiscard]] bool HasTag(std::string_view tag) const
    { return std::any_of(m_tags.begin(), m_tags.end(), [&tag](const auto& t) { return t == tag; }); }

    //! Returns the condition that determines the locations where ShipDesign
    //! containing part can be produced
    [[nodiscard]] auto* Location() const noexcept { return m_location.get(); }

    [[nodiscard]] auto& Exclusions() const noexcept { return m_exclusions; }
    [[nodiscard]] auto& Effects() const noexcept { return m_effects; }
    [[nodiscard]] auto& Icon() const noexcept { return m_icon; }

    //! Returns a number, calculated from the contained data, which should be
    //! different for different contained data, and must be the same for
    //! the same contained data, and must be the same on different platforms
    //! and executions of the program and the function. Useful to verify that
    //! the parsed content is consistent without sending it all between
    //! clients and server.
    [[nodiscard]] uint32_t GetCheckSum() const;
    //@}

private:
    std::string     m_name;
    std::string     m_description;
    ShipPartClass   m_class = ShipPartClass::INVALID_SHIP_PART_CLASS;
    float           m_capacity = 0.0f;
    //! Damage for a hangar bay, shots per turn for a weapon, etc.
    float           m_secondary_stat = 0.0f;

    std::unique_ptr<ValueRef::ValueRef<double>> m_production_cost;
    std::unique_ptr<ValueRef::ValueRef<int>>    m_production_time;
    std::vector<ShipSlotType>                   m_mountable_slot_types;
    const std::string                           m_tags_concatenated;
    const std::vector<std::string_view>         m_tags;
    const std::vector<std::string_view>         m_pedia_tags;
    ConsumptionMap<MeterType>                   m_production_meter_consumption;
    ConsumptionMap<std::string>                 m_production_special_consumption;
    std::unique_ptr<Condition::Condition>       m_location;
    std::vector<std::string>                    m_exclusions;
    std::vector<Effect::EffectsGroup>           m_effects;
    std::string                                 m_icon;
    std::unique_ptr<Condition::Condition>       m_combat_targets;
    std::unique_ptr<ValueRef::ValueRef<double>> m_total_fighter_damage;
    std::unique_ptr<ValueRef::ValueRef<double>> m_total_ship_damage;
    bool                                        m_add_standard_capacity_effect = false;
    bool                                        m_producible = false;
};


//! Holds FreeOrion available ShipParts
class FO_COMMON_API ShipPartManager {
public:
    using ShipPartMap = std::map<std::string, std::unique_ptr<ShipPart>, std::less<>>;
    using iterator = ShipPartMap::const_iterator;
    using const_iterator = iterator;

    //! Returns the ShipPart with the name @p name; you should use the free
    //! function GetShipPart() instead
    [[nodiscard]] auto GetShipPart(std::string_view name) const -> const ShipPart*;

    //! Iterator to the first ShipPart
    [[nodiscard]] auto begin() const -> iterator;

    //! Iterator to one after the last ShipPart.
    [[nodiscard]] auto end() const -> iterator;

    //! How many parts are known?
    [[nodiscard]] auto size() const -> std::size_t;

    //! Returns the instance of this singleton class; you should use the free
    //! function GetShipPartManager() instead.
    [[nodiscard]] static auto GetShipPartManager() -> ShipPartManager&;

    //! Returns a number, calculated from the contained data, which should be
    //! different for different contained data, and must be the same for
    //! the same contained data, and must be the same on different platforms
    //! and executions of the program and the function. Useful to verify that
    //! the parsed content is consistent without sending it all between
    //! clients and server.
    [[nodiscard]] auto GetCheckSum() const -> uint32_t;

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
[[nodiscard]] FO_COMMON_API ShipPartManager& GetShipPartManager();


//! Returns the ShipPart specification object with name @p name.  If no
//! such ShipPart exists, nullptr is returned instead.
[[nodiscard]] FO_COMMON_API const ShipPart* GetShipPart(std::string_view name);


#endif
