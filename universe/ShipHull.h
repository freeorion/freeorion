#ifndef _ShipHull_h_
#define _ShipHull_h_


#include "CommonParams.h"
#include "ScriptingContext.h"
#include "../util/Enum.h"
#include "../util/Pending.h"


//! Types of slots in ShipHull%s
//! Parts may be restricted to only certain slot types
FO_ENUM(
    (ShipSlotType),
    ((INVALID_SHIP_SLOT_TYPE, -1))
    //! External slots.  more easily damaged
    ((SL_EXTERNAL))
    //! Internal slots.  more protected, fewer in number
    ((SL_INTERNAL))
    ((SL_CORE))
    ((NUM_SHIP_SLOT_TYPES))
)


//! Specification for the hull, or base, on which ship designs are created by
//! adding parts.  The hull determines some final design characteristics
//! directly, and also determine how many parts can be added to the design.
class FO_COMMON_API ShipHull {
public:
    struct Slot {
        Slot() = default;

        Slot(ShipSlotType slot_type, double x_, double y_) :
            type(slot_type), x(x_), y(y_)
        {}

        [[nodiscard]] bool operator==(const Slot& rhs) const noexcept
        { return type == rhs.type && x == rhs.x && y == rhs.y; }

        ShipSlotType type = ShipSlotType::INVALID_SHIP_SLOT_TYPE;
        double x = 0.5, y = 0.5;
    };

    ShipHull() = delete;

    ShipHull(float fuel, float speed, float stealth, float structure,
             bool default_fuel_effects, bool default_speed_effects,
             bool default_stealth_effects, bool default_structure_effects,
             CommonParams&& common_params,
             std::string&& name, std::string&& description,
             std::set<std::string>&& exclusions, std::vector<Slot>&& slots,
             std::string&& icon, std::string&& graphic);

    ~ShipHull();

    [[nodiscard]] bool operator==(const ShipHull& rhs) const;

    //! Returns name of hull
    [[nodiscard]] const auto& Name() const noexcept { return m_name; }

    //! Returns description, including a description of the stats and effects
    //! of this hull
    [[nodiscard]] const auto& Description() const noexcept { return m_description; }

    //! Returns starlane speed of hull
    [[nodiscard]] auto Speed() const -> float;

    //! Returns fuel capacity of hull
    [[nodiscard]] auto Fuel() const noexcept { return m_fuel; }

    //! Returns stealth of hull
    [[nodiscard]] auto Stealth() const noexcept { return m_stealth; }

    //! Returns structure of hull
    [[nodiscard]] auto Structure() const -> float;

    //! Returns shields of hull
    [[nodiscard]] auto Shields() const noexcept { return 0.0f; }

    //! Returns colonist capacity of hull
    [[nodiscard]] auto ColonyCapacity() const noexcept { return 0.0f; }

    //! Returns the troop capacity of hull
    [[nodiscard]] auto TroopCapacity() const noexcept { return 0.0f; }

    //! Returns detection ability of hull
    [[nodiscard]] auto Detection() const noexcept { return 0.0f; }

    //! Returns true if the production cost and time are invariant (does not
    //! depend on) the location
    [[nodiscard]] auto ProductionCostTimeLocationInvariant() const -> bool;

    //! Returns the number of production points required to produce this hull
    [[nodiscard]] auto ProductionCost(int empire_id, int location_id, const ScriptingContext& parent_context,
                                      int in_design_id = INVALID_DESIGN_ID) const -> float;

    //! Returns the number of turns required to produce this hull
    [[nodiscard]] auto ProductionTime(int empire_id, int location_id, const ScriptingContext& parent_context,
                                      int in_design_id = INVALID_DESIGN_ID) const -> int;

    //! Returns whether this hull type is producible by players and appears on
    //! the design screen
    [[nodiscard]] auto Producible() const noexcept { return m_producible; }

    [[nodiscard]] const auto& ProductionMeterConsumption() const noexcept { return m_production_meter_consumption; }

    [[nodiscard]] const auto& ProductionSpecialConsumption() const noexcept { return m_production_special_consumption; }

    //! Returns total number of of slots in hull
    [[nodiscard]] uint32_t NumSlots() const noexcept { return static_cast<uint32_t>(m_slots.size()); }

    //! Returns number of of slots of indicated type in hull
    [[nodiscard]] uint32_t NumSlots(ShipSlotType slot_type) const noexcept;

    //! Returns vector of slots in hull
    [[nodiscard]] const auto& Slots() const noexcept { return m_slots; }

    [[nodiscard]] const auto& Tags() const noexcept { return m_tags; }

    [[nodiscard]] bool HasTag(std::string_view tag) const
    { return std::any_of(m_tags.begin(), m_tags.end(), [&tag](const auto& t) { return t == tag; }); }

    //! Returns the condition that determines the locations where ShipDesign
    //! containing hull can be produced
    [[nodiscard]] const auto* Location() const noexcept { return m_location.get(); }

    //! Returns the names of other content that cannot be used in the same
    //! ship design as this part
    [[nodiscard]] const auto& Exclusions() const noexcept { return m_exclusions; }

    //! Returns the EffectsGroups that encapsulate the effects this part hull
    //! has.
    [[nodiscard]] const auto& Effects() const noexcept { return m_effects; }

    //! Returns the image that represents the hull on the design screen
    [[nodiscard]] const auto& Graphic() const noexcept { return m_graphic; }

    //! Returns the small icon to represent hull
    [[nodiscard]] const auto& Icon() const noexcept { return m_icon; }

    //! Returns a number, calculated from the contained data, which should be
    //! different for different contained data, and must be the same for
    //! the same contained data, and must be the same on different platforms
    //! and executions of the program and the function. Useful to verify that
    //! the parsed content is consistent without sending it all between
    //! clients and server.
    [[nodiscard]] auto GetCheckSum() const -> uint32_t;

private:
    const std::string m_name;
    const std::string m_description;
    const float       m_speed = 1.0f;
    const float       m_fuel = 0.0f;
    const float       m_stealth = 0.0f;
    const float       m_structure = 0.0f;
    const bool        m_default_speed_effects = false;
    const bool        m_default_structure_effects = false;

    const bool                                              m_producible = false;
    const std::unique_ptr<const ValueRef::ValueRef<double>> m_production_cost;
    const std::unique_ptr<const ValueRef::ValueRef<int>>    m_production_time;
    const std::vector<Slot>                                 m_slots;
    const std::string                                       m_tags_concatenated;
    const std::vector<std::string_view>                     m_tags;
    const ConsumptionMap<MeterType>                         m_production_meter_consumption;
    const ConsumptionMap<std::string>                       m_production_special_consumption;
    const std::unique_ptr<const Condition::Condition>       m_location;
    const std::vector<std::string>                          m_exclusions;
    const std::vector<Effect::EffectsGroup>                 m_effects;
    const std::string                                       m_graphic;
    const std::string                                       m_icon;
};


namespace CheckSums {
    FO_COMMON_API void CheckSumCombine(uint32_t& sum, const ShipHull::Slot& slot);
}


//! Holds FreeOrion hull types
class FO_COMMON_API ShipHullManager {
public:
    using container_type = std::map<std::string, std::unique_ptr<ShipHull>, std::less<>>;
    using iterator = container_type::const_iterator;
    using const_iterator = iterator;

    //! Returns the hull type with the name @a name; you should use the free
    //! function GetShipHull() instead
    [[nodiscard]] auto GetShipHull(std::string_view name) const -> const ShipHull*;

    //! iterator to the first hull type
    [[nodiscard]] auto begin() const -> iterator;

    //! iterator to the last + 1th hull type
    [[nodiscard]] auto end() const -> iterator;

    //! How many hulls are known?
    [[nodiscard]] auto size() const -> std::size_t;

    //! Returns the instance of this singleton class; you should use the free
    //! function GetShipHullManager() instead
    [[nodiscard]] static auto GetShipHullManager() -> ShipHullManager&;

    //! Returns a number, calculated from the contained data, which should be
    //! different for different contained data, and must be the same for
    //! the same contained data, and must be the same on different platforms
    //! and executions of the program and the function. Useful to verify that
    //! the parsed content is consistent without sending it all between
    //! clients and server.
    [[nodiscard]] auto GetCheckSum() const -> uint32_t;

    //! Sets hull types to the future value of \p pending_ship_hulls.
    FO_COMMON_API void SetShipHulls(Pending::Pending<container_type>&& pending_ship_hulls);

private:
    ShipHullManager();

    //! Assigns any m_pending_ship_hulls to m_bulding_types.
    void CheckPendingShipHulls() const;

    //! Future hull type being parsed by parser.
    mutable boost::optional<Pending::Pending<container_type>> m_pending_ship_hulls = boost::none;

    //! Set of hull types.
    mutable container_type m_hulls;

    static ShipHullManager* s_instance;
};


//! Returns the singleton hull type manager
[[nodiscard]] FO_COMMON_API auto GetShipHullManager() -> ShipHullManager&;

//! Returns the ship ShipHull specification object with name @p name.  If no
//! such ShipHull exists, nullptr is returned instead.
[[nodiscard]] FO_COMMON_API auto GetShipHull(std::string_view name) -> const ShipHull*;

#endif
