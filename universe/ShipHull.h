#ifndef _ShipHull_h_
#define _ShipHull_h_


#include <GG/Enum.h>
#include "CommonParams.h"
#include "../util/Pending.h"


FO_COMMON_API extern const int INVALID_DESIGN_ID;


//! Types of slots in ShipHull%s
//! Parts may be restricted to only certain slot types
GG_ENUM(ShipSlotType,
    INVALID_SHIP_SLOT_TYPE = -1,
    //! External slots.  more easily damaged
    SL_EXTERNAL,
    //! Internal slots.  more protected, fewer in number
    SL_INTERNAL,
    SL_CORE,
    NUM_SHIP_SLOT_TYPES
)


//! Specification for the hull, or base, on which ship designs are created by
//! adding parts.  The hull determines some final design characteristics
//! directly, and also determine how many parts can be added to the design.
class FO_COMMON_API ShipHull {
public:
    struct Slot {
        Slot();

        Slot(ShipSlotType slot_type, double x_, double y_) :
            type(slot_type), x(x_), y(y_)
        {}

        ShipSlotType type;
        double x = 0.5, y = 0.5;
    };

    ShipHull();

    ShipHull(float fuel, float speed, float stealth, float structure,
             bool default_fuel_effects, bool default_speed_effects,
             bool default_stealth_effects, bool default_structure_effects,
             CommonParams&& common_params,
             std::string&& name, std::string&& description,
             std::set<std::string>&& exclusions, std::vector<Slot>&& slots,
             std::string&& icon, std::string&& graphic);

    ~ShipHull();

    //! Returns name of hull
    auto Name() const -> const std::string&
    { return m_name; }

    //! Returns description, including a description of the stats and effects
    //! of this hull
    auto Description() const -> const std::string&
    { return m_description; }

    //! Returns starlane speed of hull
    auto Speed() const -> float;

    //! Returns fuel capacity of hull
    auto Fuel() const -> float
    { return m_fuel; }

    //! Returns stealth of hull
    auto Stealth() const -> float
    { return m_stealth; }

    //! Returns structure of hull
    auto Structure() const -> float;

    //! Returns shields of hull
    auto Shields() const -> float
    { return 0.0f; }

    //! Returns colonist capacity of hull
    auto ColonyCapacity() const -> float
    { return 0.0f; }

    //! Returns the troop capacity of hull
    auto TroopCapacity() const -> float
    { return 0.0f; }

    //! Returns detection ability of hull
    auto Detection() const -> float
    { return 0.0f; }

    //! Returns true if the production cost and time are invariant (does not
    //! depend on) the location
    auto ProductionCostTimeLocationInvariant() const -> bool;

    //! Returns the number of production points required to produce this hull
    auto ProductionCost(int empire_id, int location_id, int in_design_id = INVALID_DESIGN_ID) const -> float;

    //! Returns the number of turns required to produce this hull
    auto ProductionTime(int empire_id, int location_id, int in_design_id = INVALID_DESIGN_ID) const -> int;

    //! Returns whether this hull type is producible by players and appears on
    //! the design screen
    auto Producible() const -> bool
    { return m_producible; }

    auto ProductionMeterConsumption() const -> const ConsumptionMap<MeterType>&
    { return m_production_meter_consumption; }

    auto ProductionSpecialConsumption() const -> const ConsumptionMap<std::string>&
    { return m_production_special_consumption; }

    //! Returns total number of of slots in hull
    auto NumSlots() const -> unsigned int
    { return m_slots.size(); }

    //! Returns number of of slots of indicated type in hull
    auto NumSlots(ShipSlotType slot_type) const -> unsigned int;

    //! Returns vector of slots in hull
    auto Slots() const -> const std::vector<Slot>&
    { return m_slots; }

    auto Tags() const -> const std::set<std::string>&
    { return m_tags; }

    auto HasTag(const std::string& tag) const -> bool
    { return m_tags.count(tag) != 0; }

    //! Returns the condition that determines the locations where ShipDesign
    //! containing hull can be produced
    auto Location() const -> const Condition::Condition*
    { return m_location.get(); }

    //! Returns the names of other content that cannot be used in the same
    //! ship design as this part
    auto Exclusions() const -> const std::set<std::string>&
    { return m_exclusions; }

    //! Returns the EffectsGroups that encapsulate the effects this part hull
    //! has.
    auto Effects() const -> const std::vector<std::shared_ptr<Effect::EffectsGroup>>&
    { return m_effects; }

    //! Returns the image that represents the hull on the design screen
    auto Graphic() const -> const std::string&
    { return m_graphic; }

    //! Returns the small icon to represent hull
    auto Icon() const -> const std::string&
    { return m_icon; }

    //! Returns a number, calculated from the contained data, which should be
    //! different for different contained data, and must be the same for
    //! the same contained data, and must be the same on different platforms
    //! and executions of the program and the function. Useful to verify that
    //! the parsed content is consistent without sending it all between
    //! clients and server.
    auto GetCheckSum() const -> unsigned int;

private:
    void Init(std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects,
              bool default_fuel_effects,
              bool default_speed_effects,
              bool default_stealth_effects,
              bool default_structure_effects);

    std::string m_name;
    std::string m_description;
    float       m_speed = 1.0f;
    float       m_fuel = 0.0f;
    float       m_stealth = 0.0f;
    float       m_structure = 0.0f;

    std::unique_ptr<ValueRef::ValueRef<double>>         m_production_cost;
    std::unique_ptr<ValueRef::ValueRef<int>>            m_production_time;
    bool                                                m_producible = false;
    std::vector<Slot>                                   m_slots;
    std::set<std::string>                               m_tags;
    ConsumptionMap<MeterType>                           m_production_meter_consumption;
    ConsumptionMap<std::string>                         m_production_special_consumption;
    std::unique_ptr<Condition::Condition>               m_location;
    std::set<std::string>                               m_exclusions;
    std::vector<std::shared_ptr<Effect::EffectsGroup>>  m_effects;
    std::string                                         m_graphic;
    std::string                                         m_icon;
};


namespace CheckSums {
    FO_COMMON_API void CheckSumCombine(unsigned int& sum, const ShipHull::Slot& slot);
}


//! Holds FreeOrion hull types
class FO_COMMON_API ShipHullManager {
public:
    using container_type = std::map<std::string, std::unique_ptr<ShipHull>>;
    using iterator = container_type::const_iterator;

    //! Returns the hull type with the name @a name; you should use the free
    //! function GetShipHull() instead
    auto GetShipHull(const std::string& name) const -> const ShipHull*;

    //! iterator to the first hull type
    auto begin() const -> iterator;

    //! iterator to the last + 1th hull type
    auto end() const -> iterator;

    //! How many hulls are known?
    auto size() const -> std::size_t;

    //! Returns the instance of this singleton class; you should use the free
    //! function GetShipHullManager() instead
    static auto GetShipHullManager() -> ShipHullManager&;

    //! Returns a number, calculated from the contained data, which should be
    //! different for different contained data, and must be the same for
    //! the same contained data, and must be the same on different platforms
    //! and executions of the program and the function. Useful to verify that
    //! the parsed content is consistent without sending it all between
    //! clients and server.
    auto GetCheckSum() const -> unsigned int;

    //! Sets hull types to the future value of \p pending_ship_hulls.
    void SetShipHulls(Pending::Pending<container_type>&& pending_ship_hulls);

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
FO_COMMON_API auto GetShipHullManager() -> ShipHullManager&;

//! Returns the ship ShipHull specification object with name @p name.  If no
//! such ShipHull exists, nullptr is returned instead.
FO_COMMON_API auto GetShipHull(const std::string& name) -> const ShipHull*;


#endif
