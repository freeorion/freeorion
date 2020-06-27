#ifndef _BuildingType_h_
#define _BuildingType_h_


#include "CommonParams.h"
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


//! Class to specify a kind of building.
//!
//! Each building type must have a unique @a name string, by which it can be
//! looked up using GetBuildingType(...).
class FO_COMMON_API BuildingType {
public:
    BuildingType(std::string&& name, std::string&& description,
                 CommonParams&& common_params, CaptureResult capture_result,
                 std::string&& icon);
    ~BuildingType();

    //! Returns the unique name for this type of building
    auto Name() const -> const std::string&
    { return m_name; }

    //! Returns a text description of this type of building
    auto Description() const -> const std::string&
    { return m_description; }

    //! Returns a data file format representation of this object
    auto Dump(unsigned short ntabs = 0) const -> std::string;

    //! Returns true if the production cost and time are invariant (does not
    //! depend on) the location
    auto ProductionCostTimeLocationInvariant() const -> bool;

    //! Returns the number of production points required to build this building
    //! at this location by this empire
    auto ProductionCost(int empire_id, int location_id) const -> float;

    //! Returns the maximum number of production points per turn that can be
    //! spend on this building
    auto PerTurnCost(int empire_id, int location_id) const -> float;

    //! Returns the number of turns required to build this building at this
    //! location by this empire
    auto ProductionTime(int empire_id, int location_id) const -> int;

    //! Returns the ValueRef that determines ProductionCost()
    auto Cost() const -> const ValueRef::ValueRef<double>*
    { return m_production_cost.get(); }

    //! Returns the ValueRef that determines ProductionTime()
    auto Time() const -> const ValueRef::ValueRef<int>*
    { return m_production_time.get(); }

    //! Returns whether this building type is producible by players and appears
    //! on the production screen
    auto Producible() const -> bool
    { return m_producible; }

    auto ProductionMeterConsumption() const -> const ConsumptionMap<MeterType>&
    { return m_production_meter_consumption; }

    auto ProductionSpecialConsumption() const -> const ConsumptionMap<std::string>&
    { return m_production_special_consumption; }

    auto Tags() const -> const std::set<std::string>&
    { return m_tags; }

    //! Returns the condition that determines the locations where this building
    //! can be produced
    auto Location() const -> const Condition::Condition*
    { return m_location.get(); }

    //! Returns a condition that can be used by the UI to further filter (beyond
    //! the Location() requirement) where this building will be presented for
    //! enqueuing onto the production queue, to avoid clutter in the
    //! BuildDesignatorWnd. Example usage: Buildings that are already enqueued
    //! at a production location are hidden so they don't appear in the list of
    //! available items that can be enqueued/produced (again) at that location.
    auto EnqueueLocation() const -> const Condition::Condition*
    { return m_enqueue_location.get(); }

    //! Returns the EffectsGroups that encapsulate the effects that buildings of
    //! this type have when operational.
    auto Effects() const -> const std::vector<std::shared_ptr<Effect::EffectsGroup>>&
    { return m_effects; }

    //! Returns the name of the grapic file for this building type
    auto Icon() const -> const std::string&
    { return m_icon; }

    //! Returns true iff the empire with ID empire_id can produce this building
    //! at the location with location_id
    auto ProductionLocation(int empire_id, int location_id) const -> bool;

    //! Returns true iff the empire with ID empire_id meets the requirements of
    //! the EnqueueLocation() UI filter method for this building at the
    //! location with location_id
    auto EnqueueLocation(int empire_id, int location_id) const -> bool;

    //! Returns CaptureResult for empire with ID @p to_empire_id capturing from
    //! empire with IDs @p from_empire_id the planet (or other UniverseObject)
    //! with id @p location_id on which this type of Building is located (if
    //! @p as_production_item is false) or which is the location of a Production
    //! Queue BuildItem for a building of this type (otherwise)
    auto GetCaptureResult(int from_empire_id, int to_empire_id, int location_id, bool as_production_item) const -> CaptureResult
    { return m_capture_result; }

    //! Returns a number, calculated from the contained data, which should be
    //! different for different contained data, and must be the same for
    //! the same contained data, and must be the same on different platforms
    //! and executions of the program and the function. Useful to verify that
    //! the parsed content is consistent without sending it all between
    //! clients and server.
    auto GetCheckSum() const -> unsigned int;

private:
    void Init();

    std::string                                         m_name;
    std::string                                         m_description;
    std::unique_ptr<ValueRef::ValueRef<double>>         m_production_cost;
    std::unique_ptr<ValueRef::ValueRef<int>>            m_production_time;
    bool                                                m_producible = true;
    CaptureResult                                       m_capture_result;
    std::set<std::string>                               m_tags;
    ConsumptionMap<MeterType>                           m_production_meter_consumption;
    ConsumptionMap<std::string>                         m_production_special_consumption;
    std::unique_ptr<Condition::Condition>               m_location;
    std::unique_ptr<Condition::Condition>               m_enqueue_location;
    std::vector<std::shared_ptr<Effect::EffectsGroup>>  m_effects;
    std::string                                         m_icon;
};

//! Holds all FreeOrion BuildingType%s.  Types may be looked up by name.
class BuildingTypeManager {
public:
    using container_type = std::map<std::string, std::unique_ptr<BuildingType>>;
    using iterator = container_type::const_iterator;

    //! Returns the building type with the name @p name; you should use the
    //! free function GetBuildingType(...) instead, mainly to save some typing.
    auto GetBuildingType(const std::string& name) const -> const BuildingType*;

    auto NumBuildingTypes() const -> std::size_t { return m_building_types.size(); }

    //! iterator to the first building type
    FO_COMMON_API auto begin() const -> iterator;

    //! iterator to the last + 1th building type
    FO_COMMON_API auto end() const -> iterator;

    //! Returns the instance of this singleton class; you should use the free
    //! function GetBuildingTypeManager() instead
    static auto GetBuildingTypeManager() -> BuildingTypeManager&;

    //! Returns a number, calculated from the contained data, which should be
    //! different for different contained data, and must be the same for
    //! the same contained data, and must be the same on different platforms
    //! and executions of the program and the function. Useful to verify that
    //! the parsed content is consistent without sending it all between
    //! clients and server.
    auto GetCheckSum() const -> unsigned int;

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
FO_COMMON_API auto GetBuildingTypeManager() -> BuildingTypeManager&;

//! Returns the BuildingType specification object for a building of type
//! @p name.  If no such BuildingType exists, nullptr is returned instead.
FO_COMMON_API auto GetBuildingType(const std::string& name) -> const BuildingType*;


#endif // _BuildingType_h_
