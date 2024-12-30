#ifndef _BuildingType_h_
#define _BuildingType_h_


#include "CommonParams.h"
#include "ScriptingContext.h"
#include "../util/AppInterface.h"
#include "../util/Enum.h"
#include "../util/Export.h"
#include "../util/Pending.h"


namespace Effect {
    class EffectsGroup;
}
namespace Condition {
    struct Condition;
}
namespace ValueRef {
    template <typename T>
    struct ValueRef;
}


//! Possible results of a Building being captured by other empires, or an
//! Planet containing the Building being captured.
FO_ENUM(
    (CaptureResult),
    ((INVALID_CAPTURE_RESULT, -1))
    //! Building has ownership by original empire(s) removed, and conquering
    //! empire added
    ((CR_CAPTURE))
    //! Building is destroyed
    ((CR_DESTROY))
    //! Building ownership unchanged: original empire(s) still own object
    ((CR_RETAIN))
)


//! Class to specify a kind of building.
//!
//! Each building type must have a unique @a name string, by which it can be
//! looked up using GetBuildingType(...).
class FO_COMMON_API BuildingType {
public:
    BuildingType(std::string&& name, std::string&& description,
                 CommonParams&& common_params, CaptureResult capture_result,
                 std::string&& icon);
    ~BuildingType(); // needed due to forward-declared Condition held in unique_ptr

    [[nodiscard]] bool operator==(const BuildingType& rhs) const;

    //! Returns the unique name for this type of building
    [[nodiscard]] auto& Name() const noexcept { return m_name; }

    //! Returns a text description of this type of building
    [[nodiscard]] auto& Description() const noexcept { return m_description; }

    //! Returns a data file format representation of this object
    [[nodiscard]] auto Dump(uint8_t ntabs = 0) const -> std::string;

    //! Returns true if the production cost and time are invariant (does not
    //! depend on) the location
    [[nodiscard]] auto ProductionCostTimeLocationInvariant() const -> bool;

    //! Returns the number of production points required to build this building
    //! at this location by this empire
    [[nodiscard]] auto ProductionCost(int empire_id, int location_id, const ScriptingContext& context) const -> float;

    //! Returns the maximum number of production points per turn that can be
    //! spend on this building
    [[nodiscard]] auto PerTurnCost(int empire_id, int location_id, const ScriptingContext& context) const -> float;

    //! Returns the number of turns required to build this building at this
    //! location by this empire
    [[nodiscard]] auto ProductionTime(int empire_id, int location_id, const ScriptingContext& context) const -> int;

    //! Returns the ValueRef that determines ProductionCost()
    [[nodiscard]] const auto* Cost() const noexcept { return m_production_cost.get(); }

    //! Returns the ValueRef that determines ProductionTime()
    [[nodiscard]] const auto* Time() const noexcept { return m_production_time.get(); }

    //! Returns whether this building type is producible by players and appears
    //! on the production screen
    [[nodiscard]] auto Producible() const noexcept { return m_producible; }

    [[nodiscard]] auto& ProductionMeterConsumption() const noexcept { return m_production_meter_consumption; }

    [[nodiscard]] auto& ProductionSpecialConsumption() const noexcept { return m_production_special_consumption; }

    [[nodiscard]] auto& Tags() const noexcept { return m_tags; }

    [[nodiscard]] auto HasTag(std::string_view tag) const -> bool
    { return std::any_of(m_tags.begin(), m_tags.end(), [tag](const auto& t) noexcept { return t == tag; }); }

    //! Returns the condition that determines the locations where this building
    //! can be produced
    [[nodiscard]] const auto* Location() const noexcept { return m_location.get(); }

    //! Returns a condition that can be used by the UI to further filter (beyond
    //! the Location() requirement) where this building will be presented for
    //! enqueuing onto the production queue, to avoid clutter in the
    //! BuildDesignatorWnd. Example usage: Buildings that are already enqueued
    //! at a production location are hidden so they don't appear in the list of
    //! available items that can be enqueued/produced (again) at that location.
    [[nodiscard]] const auto* EnqueueLocation() const noexcept { return m_enqueue_location.get(); }

    //! Returns the EffectsGroups that encapsulate the effects that buildings of
    //! this type have when operational.
    [[nodiscard]] auto& Effects() const noexcept { return m_effects; }

    //! Returns the name of the grapic file for this building type
    [[nodiscard]] auto& Icon() const noexcept { return m_icon; }

    //! Returns true iff the empire with ID empire_id can produce this building
    //! at the location with location_id
    [[nodiscard]] auto ProductionLocation(int empire_id, int location_id, const ScriptingContext& context) const -> bool;

    //! Returns true iff the empire with ID empire_id meets the requirements of
    //! the EnqueueLocation() UI filter method for this building at the
    //! location with location_id
    [[nodiscard]] auto EnqueueLocation(int empire_id, int location_id, const ScriptingContext& context) const -> bool;

    //! Returns CaptureResult for an empire capturing a planet with this building
    //! on it or that is the location of a Production Queue BuildItem for a building
    //! of this type
    [[nodiscard]] auto GetCaptureResult(int, int, int, bool) const noexcept { return m_capture_result; }

    //! Returns a number, calculated from the contained data, which should be
    //! different for different contained data, and must be the same for
    //! the same contained data, and must be the same on different platforms
    //! and executions of the program and the function. Useful to verify that
    //! the parsed content is consistent without sending it all between
    //! clients and server.
    [[nodiscard]] auto GetCheckSum() const -> uint32_t;

private:
    const std::string                                 m_name;
    const std::string                                 m_description;
    const std::unique_ptr<const ValueRef::ValueRef<double>> m_production_cost;
    const std::unique_ptr<const ValueRef::ValueRef<int>>    m_production_time;
    const bool                                        m_producible = true;
    const CaptureResult                               m_capture_result;
    const std::string                                 m_tags_concatenated;
    const std::vector<std::string_view>               m_tags;
    const ConsumptionMap<MeterType>                   m_production_meter_consumption;
    const ConsumptionMap<std::string>                 m_production_special_consumption;
    const std::unique_ptr<const Condition::Condition> m_location;
    const std::unique_ptr<const Condition::Condition> m_enqueue_location;
    const std::vector<Effect::EffectsGroup>           m_effects;
    const std::string                                 m_icon;
};

//! Holds all FreeOrion BuildingType%s.  Types may be looked up by name.
class BuildingTypeManager {
public:
    using container_type = std::map<std::string, std::unique_ptr<BuildingType>, std::less<>>;
    using iterator = container_type::const_iterator;
    using const_iterator = iterator;

    //! Returns the building type with the name @p name; you should use the
    //! free function GetBuildingType(...) instead, mainly to save some typing.
    [[nodiscard]] auto GetBuildingType(std::string_view name) const -> const BuildingType*;

    [[nodiscard]] auto NumBuildingTypes() const noexcept { return m_building_types.size(); }

    //! iterator to the first building type
    [[nodiscard]] FO_COMMON_API auto begin() const -> iterator;

    //! iterator to the last + 1th building type
    [[nodiscard]] FO_COMMON_API auto end() const -> iterator;

    //! Returns the instance of this singleton class; you should use the free
    //! function GetBuildingTypeManager() instead
    [[nodiscard]] static auto GetBuildingTypeManager() -> BuildingTypeManager&;

    //! Returns a number, calculated from the contained data, which should be
    //! different for different contained data, and must be the same for
    //! the same contained data, and must be the same on different platforms
    //! and executions of the program and the function. Useful to verify that
    //! the parsed content is consistent without sending it all between
    //! clients and server.
    [[nodiscard]] auto GetCheckSum() const -> uint32_t;

    //! Sets building types to the future value of \p pending_building_types.
    FO_COMMON_API void SetBuildingTypes(Pending::Pending<container_type>&& pending_building_types);

private:
    BuildingTypeManager();

    //! Assigns any m_pending_building_types to m_bulding_types.
    void CheckPendingBuildingTypes() const;

    //! Future building type being parsed by parser.
    //! Mutable so that it can be assigned to m_building_types when completed.
    mutable boost::optional<Pending::Pending<container_type>> m_pending_building_types = boost::none;

    //! Map of building types identified by the BuildingType::Name.
    //! mutable so that when the parse complete it can be updated.
    mutable container_type m_building_types;

    static BuildingTypeManager* s_instance;
};

//! Returns the singleton building type manager
[[nodiscard]] FO_COMMON_API auto GetBuildingTypeManager() -> BuildingTypeManager&;

//! Returns the BuildingType specification object for a building of type
//! @p name.  If no such BuildingType exists, nullptr is returned instead.
[[nodiscard]] FO_COMMON_API auto GetBuildingType(std::string_view name) -> const BuildingType*;


#endif
