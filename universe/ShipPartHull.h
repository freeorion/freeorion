#ifndef _ShipPartHull_h_
#define _ShipPartHull_h_

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include "CommonParams.h"
#include "../util/Pending.h"


FO_COMMON_API extern const int INVALID_OBJECT_ID;
FO_COMMON_API extern const int INVALID_DESIGN_ID;
FO_COMMON_API extern const int ALL_EMPIRES;
FO_COMMON_API extern const int INVALID_GAME_TURN;

/** Hull stats.  Used by parser due to limits on number of sub-items per
  * parsed main item. */
struct HullTypeStats {
    HullTypeStats() = default;

    HullTypeStats(float fuel_,
                  float speed_,
                  float stealth_,
                  float structure_,
                  bool no_default_fuel_effects_,
                  bool no_default_speed_effects_,
                  bool no_default_stealth_effects_,
                  bool no_default_structure_effects_) :
        fuel(fuel_),
        speed(speed_),
        stealth(stealth_),
        structure(structure_),
        default_fuel_effects(!no_default_fuel_effects_),
        default_speed_effects(!no_default_speed_effects_),
        default_stealth_effects(!no_default_stealth_effects_),
        default_structure_effects(!no_default_structure_effects_)
    {}

    float   fuel = 0.0f;
    float   speed = 0.0f;
    float   stealth = 0.0f;
    float   structure = 0.0f;
    bool    default_fuel_effects = true;
    bool    default_speed_effects = true;
    bool    default_stealth_effects = true;
    bool    default_structure_effects = true;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar  & BOOST_SERIALIZATION_NVP(fuel)
            & BOOST_SERIALIZATION_NVP(speed)
            & BOOST_SERIALIZATION_NVP(stealth)
            & BOOST_SERIALIZATION_NVP(structure)
            & BOOST_SERIALIZATION_NVP(default_fuel_effects)
            & BOOST_SERIALIZATION_NVP(default_speed_effects)
            & BOOST_SERIALIZATION_NVP(default_stealth_effects)
            & BOOST_SERIALIZATION_NVP(default_structure_effects);
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
        double x = 0.5, y = 0.5;
    };

    /** \name Structors */ //@{
    HullType();
    HullType(const HullTypeStats& stats,
             CommonParams&& common_params,
             const MoreCommonParams& more_common_params,
             const std::vector<Slot>& slots,
             const std::string& icon, const std::string& graphic);

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
    float               ProductionCost(int empire_id, int location_id, int in_design_id = INVALID_DESIGN_ID) const; ///< returns the number of production points required to produce this hull
    int                 ProductionTime(int empire_id, int location_id, int in_design_id = INVALID_DESIGN_ID) const; ///< returns the number of turns required to produce this hull
    bool                Producible() const      { return m_producible; }        ///< returns whether this hull type is producible by players and appears on the design screen

    const ConsumptionMap<MeterType>&    ProductionMeterConsumption() const  { return m_production_meter_consumption; }
    const ConsumptionMap<std::string>&  ProductionSpecialConsumption() const{ return m_production_special_consumption; }

    unsigned int        NumSlots() const        { return m_slots.size(); }      ///< returns total number of of slots in hull
    unsigned int        NumSlots(ShipSlotType slot_type) const;                 ///< returns number of of slots of indicated type in hull
    const std::vector<Slot>& Slots() const      { return m_slots; }             ///< returns vector of slots in hull

    const std::set<std::string>& Tags() const   { return m_tags; }
    bool HasTag(const std::string& tag) const   { return m_tags.count(tag) != 0; }

    const Condition::Condition* Location() const    { return m_location.get(); }///< returns the condition that determines the locations where ShipDesign containing hull can be produced
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
    void Init(std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects,
              const HullTypeStats& stats);

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

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

namespace CheckSums {
    FO_COMMON_API void CheckSumCombine(unsigned int& sum, const HullType::Slot& slot);
}

/** Holds FreeOrion hull types */
class FO_COMMON_API HullTypeManager {
public:
    using HullTypeMap = std::map<std::string, std::unique_ptr<HullType>>;
    using iterator = HullTypeMap::const_iterator;

    /** \name Accessors */ //@{
    /** returns the hull type with the name \a name; you should use the free function GetHullType() instead */
    const HullType* GetHullType(const std::string& name) const;

    /** iterator to the first hull type */
    iterator begin() const;

    /** iterator to the last + 1th hull type */
    iterator end() const;

    /** how many hulls are known? */
    std::size_t size() const;

    /** returns the instance of this singleton class; you should use the free function GetHullTypeManager() instead */
    static HullTypeManager& GetHullTypeManager();

    /** Returns a number, calculated from the contained data, which should be
      * different for different contained data, and must be the same for
      * the same contained data, and must be the same on different platforms
      * and executions of the program and the function. Useful to verify that
      * the parsed content is consistent without sending it all between
      * clients and server. */
    unsigned int GetCheckSum() const;
    //@}

    /** Sets hull types to the future value of \p pending_hull_types. */
    FO_COMMON_API void SetHullTypes(Pending::Pending<HullTypeMap>&& pending_hull_types);

private:
    HullTypeManager();


    /** Assigns any m_pending_hull_types to m_bulding_types. */
    void CheckPendingHullTypes() const;

    /** Future hull type being parsed by parser.  mutable so that it can
        be assigned to m_hull_types when completed.*/
    mutable boost::optional<Pending::Pending<HullTypeMap>> m_pending_hull_types = boost::none;

    /** Set of hull types.  mutable so that when the parse completes it can
        be updated. */
    mutable HullTypeMap m_hulls;

    static HullTypeManager* s_instance;
};

/** returns the singleton hull type manager */
FO_COMMON_API HullTypeManager& GetHullTypeManager();

/** Returns the ship HullType specification object with name \a name.  If no such HullType exists,
  * 0 is returned instead. */
FO_COMMON_API const HullType* GetHullType(const std::string& name);


// template implementations
template <typename Archive>
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


#endif // _ShipPartHull_h_
