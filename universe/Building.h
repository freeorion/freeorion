#ifndef _Building_h_
#define _Building_h_

#include "UniverseObject.h"
#include "ObjectMap.h"
#include "ValueRefFwd.h"
#include "ShipDesign.h"
#include "../util/Export.h"
#include "../util/Pending.h"

#include <boost/algorithm/string/case_conv.hpp>

class BuildingType;
namespace Effect {
    class EffectsGroup;
}
namespace Condition {
    struct ConditionBase;
}

/** A Building UniverseObject type. */
class FO_COMMON_API Building : public UniverseObject {
public:
    /** \name Accessors */ //@{
    bool                    HostileToEmpire(int empire_id) const override;
    std::set<std::string>   Tags() const override;
    bool                    HasTag(const std::string& name) const override;
    UniverseObjectType      ObjectType() const override;
    std::string             Dump(unsigned short ntabs = 0) const override;
    int                     ContainerObjectID() const override { return m_planet_id; }
    bool                    ContainedBy(int object_id) const override;

    std::shared_ptr<UniverseObject> Accept(const UniverseObjectVisitor& visitor) const override;

    /** Returns the name of the BuildingType object for this building. */
    const std::string&      BuildingTypeName() const    { return m_building_type; };
    int                     PlanetID() const            { return m_planet_id; }             ///< returns the ID number of the planet this building is on
    int                     ProducedByEmpireID() const  { return m_produced_by_empire_id; } ///< returns the empire ID of the empire that produced this building
    bool                    OrderedScrapped() const     { return m_ordered_scrapped; }
    //@}

    /** \name Mutators */ //@{
    void Copy(std::shared_ptr<const UniverseObject> copied_object, int empire_id = ALL_EMPIRES) override;
    void SetPlanetID(int planet_id);         ///< sets the planet on which the building is located
    void Reset();                            ///< resets any building state, and removes owners
    void SetOrderedScrapped(bool b = true);  ///< flags building for scrapping
    void ResetTargetMaxUnpairedMeters() override;
    //@}

protected:
    friend class Universe;
    /** \name Structors */ //@{
    Building() {}

public:
    Building(int empire_id, const std::string& building_type,
             int produced_by_empire_id = ALL_EMPIRES);

protected:
    template <class T> friend void boost::python::detail::value_destroyer<false>::execute(T const volatile* p);

public:
    ~Building() {}

protected:
    /** Returns new copy of this Building. */
    Building* Clone(int empire_id = ALL_EMPIRES) const override;
    //@}

private:
    std::string m_building_type;
    int         m_planet_id = INVALID_OBJECT_ID;
    bool        m_ordered_scrapped = false;
    int         m_produced_by_empire_id = ALL_EMPIRES;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** A specification for a building of a certain type.  Each building type must
  * have a \a unique name string, by which it can be looked up using
  * GetBuildingType(...). */
class FO_COMMON_API BuildingType {
public:
    /** \name Structors */ //@{
    BuildingType(const std::string& name,
                 const std::string& description,
                 CommonParams& common_params,
                 CaptureResult capture_result,
                 const std::string& icon);

    ~BuildingType();
    //@}

    /** \name Accessors */ //@{
    const std::string&              Name() const            { return m_name; }              ///< returns the unique name for this type of building
    const std::string&              Description() const     { return m_description; }       ///< returns a text description of this type of building
    std::string                     Dump(unsigned short ntabs = 0) const;                   ///< returns a data file format representation of this object

    bool                            ProductionCostTimeLocationInvariant() const;            ///< returns true if the production cost and time are invariant (does not depend on) the location
    float                           ProductionCost(int empire_id, int location_id) const;   ///< returns the number of production points required to build this building at this location by this empire
    float                           PerTurnCost(int empire_id, int location_id) const;      ///< returns the maximum number of production points per turn that can be spend on this building
    int                             ProductionTime(int empire_id, int location_id) const;   ///< returns the number of turns required to build this building at this location by this empire

    const ValueRef::ValueRefBase<double>* Cost() const      { return m_production_cost.get(); }   ///< returns the ValueRef that determines ProductionCost()
    const ValueRef::ValueRefBase<int>*    Time() const      { return m_production_time.get(); }   ///< returns the ValueRef that determines ProductionTime()

    bool                            Producible() const      { return m_producible; }        ///< returns whether this building type is producible by players and appears on the production screen

    const CommonParams::ConsumptionMap<MeterType>&
                                    ProductionMeterConsumption() const  { return m_production_meter_consumption; }
    const CommonParams::ConsumptionMap<std::string>&
                                    ProductionSpecialConsumption() const{ return m_production_special_consumption; }

    const std::set<std::string>&    Tags() const            { return m_tags; }
    const Condition::ConditionBase* Location() const        { return m_location.get(); }          ///< returns the condition that determines the locations where this building can be produced
    
    /** Returns a condition that can be used by the UI to further filter (beyond the Location() requirement) where this building 
        will be presented for enqueuing onto the production queue, to avoid clutter in the BuildDesignatorWnd. */
    const Condition::ConditionBase* EnqueueLocation() const { return m_enqueue_location.get(); }  

    /** Returns the EffectsGroups that encapsulate the effects that buildings ofi
        this type have when operational. */
    const std::vector<std::shared_ptr<Effect::EffectsGroup>>& Effects() const
    { return m_effects; }

    const std::string&              Icon() const            { return m_icon; }              ///< returns the name of the grapic file for this building type

    bool ProductionLocation(int empire_id, int location_id) const;  ///< returns true iff the empire with ID empire_id can produce this building at the location with location_id
    bool EnqueueLocation(int empire_id, int location_id) const;     ///< returns true iff the empire with ID empire_id meets the requirements of the EnqueueLocation() UI filter method for this building at the location with location_id

    /** returns CaptureResult for empire with ID \a to_empire_id capturing from
      * empire with IDs \a from_empire_id the planet (or other UniverseObject)
      * with id \a location_id on which this type of Building is located (if 
      * \a as_production_item is false) or which is the location of a Production
      * Queue BuildItem for a building of this type (otherwise) */
    CaptureResult                   GetCaptureResult(int from_empire_id, int to_empire_id,
                                                     int location_id, bool as_production_item) const
    { return m_capture_result; }

    /** Returns a number, calculated from the contained data, which should be
      * different for different contained data, and must be the same for
      * the same contained data, and must be the same on different platforms
      * and executions of the program and the function. Useful to verify that
      * the parsed content is consistent without sending it all between
      * clients and server. */
    unsigned int GetCheckSum() const;
    //@}

private:
    void Init();

    std::string                                         m_name;
    std::string                                         m_description;
    std::unique_ptr<ValueRef::ValueRefBase<double>>     m_production_cost;
    std::unique_ptr<ValueRef::ValueRefBase<int>>        m_production_time;
    bool                                                m_producible;
    CaptureResult                                       m_capture_result;
    std::set<std::string>                               m_tags;
    CommonParams::ConsumptionMap<MeterType>             m_production_meter_consumption;
    CommonParams::ConsumptionMap<std::string>           m_production_special_consumption;
    std::unique_ptr<Condition::ConditionBase>           m_location;
    std::unique_ptr<Condition::ConditionBase>           m_enqueue_location;
    std::vector<std::shared_ptr<Effect::EffectsGroup>>  m_effects;
    std::string                                         m_icon;
};

/** Holds all FreeOrion building types.  Types may be looked up by name. */
class BuildingTypeManager {
public:
    using BuildingTypeMap = std::map<std::string, std::unique_ptr<BuildingType>>;
    using iterator = BuildingTypeMap::const_iterator;

    /** \name Accessors */ //@{
    /** returns the building type with the name \a name; you should use the
      * free function GetBuildingType(...) instead, mainly to save some typing. */
    const BuildingType*         GetBuildingType(const std::string& name) const;

    /** iterator to the first building type */
    FO_COMMON_API iterator                    begin() const;

    /** iterator to the last + 1th building type */
    FO_COMMON_API iterator                    end() const;

    /** returns the instance of this singleton class; you should use the free
      * function GetBuildingTypeManager() instead */
    static BuildingTypeManager& GetBuildingTypeManager();

    /** Returns a number, calculated from the contained data, which should be
      * different for different contained data, and must be the same for
      * the same contained data, and must be the same on different platforms
      * and executions of the program and the function. Useful to verify that
      * the parsed content is consistent without sending it all between
      * clients and server. */
    unsigned int                GetCheckSum() const;
    //@}

    /** Sets building types to the future value of \p pending_building_types. */
    FO_COMMON_API void SetBuildingTypes(Pending::Pending<BuildingTypeMap>&& pending_building_types);

private:
    BuildingTypeManager();

    /** Assigns any m_pending_building_types to m_bulding_types. */
    void CheckPendingBuildingTypes() const;

    /** Future building type being parsed by parser.  mutable so that it can
        be assigned to m_building_types when completed.*/
    mutable boost::optional<Pending::Pending<BuildingTypeMap>> m_pending_building_types = boost::none;

    /** Set of building types.  mutable so that when the parse complete it can
        be updated. */
    mutable BuildingTypeMap m_building_types;

    static BuildingTypeManager* s_instance;
};

/** returns the singleton building type manager */
FO_COMMON_API BuildingTypeManager& GetBuildingTypeManager();

/** Returns the BuildingType specification object for a building of
  * type \a name.  If no such BuildingType exists, 0 is returned instead. */
FO_COMMON_API const BuildingType* GetBuildingType(const std::string& name);

#endif // _Building_h_
