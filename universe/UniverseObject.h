#ifndef _UniverseObject_h_
#define _UniverseObject_h_


#include <concepts>
#include <set>
#include <span>
#include <string>
#include <vector>
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/python/detail/destroy.hpp>
#include <boost/signals2/optional_last_value.hpp>
#include <boost/signals2/signal.hpp>
#include "ConstantsFwd.h"
#include "Enums.h"
#include "Meter.h"
#include "../util/blocking_combiner.h"
#include "../util/Enum.h"
#include "../util/Export.h"


class System;
class SitRepEntry;
class EmpireManager;
class ObjectMap;
class Universe;
struct ScriptingContext;

// The ID number assigned to temporary universe objects
inline constexpr int TEMPORARY_OBJECT_ID = -2;


//! The various major subclasses of UniverseObject
FO_ENUM(
    (UniverseObjectType),
    ((INVALID_UNIVERSE_OBJECT_TYPE, -1))
    ((OBJ_BUILDING))
    ((OBJ_SHIP))
    ((OBJ_FLEET))
    ((OBJ_PLANET))
    ((OBJ_SYSTEM))
    ((OBJ_FIELD))
    ((OBJ_FIGHTER))
    ((NUM_OBJ_TYPES))
)

//! Degrees of visibility an Empire or UniverseObject can have for an
//! UniverseObject.  Determines how much information the empire gets about
//!the (non)visible object.
FO_ENUM(
    (Visibility),
    ((INVALID_VISIBILITY, -1))
    ((VIS_NO_VISIBILITY))
    ((VIS_BASIC_VISIBILITY))
    ((VIS_PARTIAL_VISIBILITY))
    ((VIS_FULL_VISIBILITY))
    ((NUM_VISIBILITIES))
)

[[nodiscard]] constexpr std::string_view DumpEnum(UniverseObjectType value) noexcept {
    switch (value) {
    case UniverseObjectType::OBJ_BUILDING:    return "Building";
    case UniverseObjectType::OBJ_SHIP:        return "Ship";
    case UniverseObjectType::OBJ_FLEET:       return "Fleet";
    case UniverseObjectType::OBJ_PLANET:      return "Planet";
    case UniverseObjectType::OBJ_SYSTEM:      return "System";
    case UniverseObjectType::OBJ_FIELD:       return "Field";
    default:                                  return "?";
    }
}

[[nodiscard]] constexpr std::string_view DumpEnum(Visibility value) noexcept {
    switch (value) {
    case Visibility::VIS_NO_VISIBILITY:      return "Invisible";
    case Visibility::VIS_BASIC_VISIBILITY:   return "Basic";
    case Visibility::VIS_PARTIAL_VISIBILITY: return "Partial";
    case Visibility::VIS_FULL_VISIBILITY:    return "Full";
    default:                                 return "Unknown";
    }
}


/** Types for Meters
* Only active paired meters should lie between MeterType::METER_POPULATION and MeterType::METER_TROOPS
* (See: UniverseObject::ResetPairedActiveMeters())
*/
FO_ENUM(
    (MeterType),
    ((INVALID_METER_TYPE, -1))
    ((METER_TARGET_POPULATION))
    ((METER_TARGET_INDUSTRY))
    ((METER_TARGET_RESEARCH))
    ((METER_TARGET_INFLUENCE))
    ((METER_TARGET_CONSTRUCTION))
    ((METER_TARGET_HAPPINESS))

    ((METER_MAX_CAPACITY))
    ((METER_MAX_SECONDARY_STAT))

    ((METER_MAX_FUEL))
    ((METER_MAX_SHIELD))
    ((METER_MAX_STRUCTURE))
    ((METER_MAX_DEFENSE))
    ((METER_MAX_SUPPLY))
    ((METER_MAX_STOCKPILE))
    ((METER_MAX_TROOPS))

    ((METER_POPULATION))
    ((METER_INDUSTRY))
    ((METER_RESEARCH))
    ((METER_INFLUENCE))
    ((METER_CONSTRUCTION))
    ((METER_HAPPINESS))

    ((METER_CAPACITY))
    ((METER_SECONDARY_STAT))

    ((METER_FUEL))
    ((METER_SHIELD))
    ((METER_STRUCTURE))
    ((METER_DEFENSE))
    ((METER_SUPPLY))
    ((METER_STOCKPILE))
    ((METER_TROOPS))

    ((METER_REBEL_TROOPS))
    ((METER_SIZE))
    ((METER_STEALTH))
    ((METER_DETECTION))
    ((METER_SPEED))

    ((NUM_METER_TYPES))
)

namespace {
    constexpr inline std::array<std::pair<MeterType, MeterType>, 13> assoc_meters{{
        {MeterType::METER_POPULATION,   MeterType::METER_TARGET_POPULATION},
        {MeterType::METER_INDUSTRY,     MeterType::METER_TARGET_INDUSTRY},
        {MeterType::METER_RESEARCH,     MeterType::METER_TARGET_RESEARCH},
        {MeterType::METER_INFLUENCE,    MeterType::METER_TARGET_INFLUENCE},
        {MeterType::METER_CONSTRUCTION, MeterType::METER_TARGET_CONSTRUCTION},
        {MeterType::METER_HAPPINESS,    MeterType::METER_TARGET_HAPPINESS},
        {MeterType::METER_FUEL,         MeterType::METER_MAX_FUEL},
        {MeterType::METER_SHIELD,       MeterType::METER_MAX_SHIELD},
        {MeterType::METER_STRUCTURE,    MeterType::METER_MAX_STRUCTURE},
        {MeterType::METER_DEFENSE,      MeterType::METER_MAX_DEFENSE},
        {MeterType::METER_TROOPS,       MeterType::METER_MAX_TROOPS},
        {MeterType::METER_SUPPLY,       MeterType::METER_MAX_SUPPLY},
        {MeterType::METER_STOCKPILE,    MeterType::METER_MAX_STOCKPILE}}};
}

/** Returns mapping from active to target or max meter types that correspond.
  * eg. MeterType::METER_RESEARCH -> MeterType::METER_TARGET_RESEARCH */
inline consteval auto& AssociatedMeterTypes() noexcept { return assoc_meters; }

/** Returns the target or max meter type that is associated with the given active meter type.
  * If no associated meter type exists, MeterType::INVALID_METER_TYPE is returned. */
constexpr MeterType AssociatedMeterType(MeterType meter_type) {
    const auto mt_pair_it = std::find_if(assoc_meters.begin(), assoc_meters.end(),
                                         [meter_type](const auto& mm) noexcept { return meter_type == mm.first; });
    return (mt_pair_it != assoc_meters.end()) ? mt_pair_it->second : MeterType::INVALID_METER_TYPE;
}

#if !defined(CONSTEXPR_STRING)
#  if defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 11))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934)))
#    define CONSTEXPR_STRING constexpr
#  else
#    define CONSTEXPR_STRING
#  endif
#endif
#if !defined(CONSTEXPR_VEC)
#  if defined(__cpp_lib_constexpr_vector)
#    define CONSTEXPR_VEC constexpr
#  else
#    define CONSTEXPR_VEC
#  endif
#endif

class FO_COMMON_API UniverseObjectCXBase {
public:
    [[nodiscard]] virtual const std::string& Name() const noexcept { return EMPTY_STRING; }///< returns the name of this object; some valid objects will have no name

    [[nodiscard]] constexpr UniverseObjectType ObjectType() const noexcept { return m_type; }

    [[nodiscard]] constexpr int             ID() const noexcept { return m_id; }    ///< returns the ID number of this object.  Each object in FreeOrion has a unique ID number.
    [[nodiscard]] constexpr double          X() const noexcept { return m_x; }      ///< the X-coordinate of this object
    [[nodiscard]] constexpr double          Y() const noexcept { return m_y; }      ///< the Y-coordinate of this object
    [[nodiscard]] constexpr int             SystemID() const noexcept { return m_system_id; };  ///< ID number of the system in which this object can be found, or INVALID_OBJECT_ID if the object is not within any system

    [[nodiscard]] constexpr int             Owner() const noexcept   { return m_owner_empire_id; };               ///< returns the ID of the empire that owns this object, or ALL_EMPIRES if there is no owner
    [[nodiscard]] constexpr bool            Unowned() const noexcept { return m_owner_empire_id == ALL_EMPIRES; } ///< returns true iff there are no owners of this object
    [[nodiscard]] constexpr bool            OwnedBy(int empire) const noexcept { return empire != ALL_EMPIRES && empire == m_owner_empire_id; }; ///< returns true iff the empire with id \a empire owns this object; unowned objects always return false;

    using SpecialMap = boost::container::flat_map<std::string, std::pair<int, float>>;
    [[nodiscard]] virtual const SpecialMap& Specials() const noexcept { return EMPTY_SPECIALS; }
    [[nodiscard]] constexpr virtual bool    HasSpecial(std::string_view) const { return false; }                     ///< true iff this object has a special with the indicated \a name
    [[nodiscard]] constexpr virtual int     SpecialAddedOnTurn(std::string_view) const { return INVALID_GAME_TURN; } ///< turn on which the special with name \a name was added to this object, or INVALID_GAME_TURN if that special is not present
    [[nodiscard]] constexpr virtual float   SpecialCapacity(std::string_view) const { return 0.0f; }                 ///> capacity of the special with name \a name or 0 if that special is not present

    /** Return human readable string description of object offset \p ntabs from margin. */
    [[nodiscard]] virtual std::string       Dump(uint8_t ntabs = 0) const { return ""; };

    [[nodiscard]] constexpr int             CreationTurn() const noexcept { return m_created_on_turn; }; ///< returns game turn on which object was created

    ///< elapsed number of turns between turn object was created and current game turn
    [[nodiscard]] constexpr int AgeInTurns(int current_turn) const noexcept {
        if (m_created_on_turn == BEFORE_FIRST_TURN)
            return SINCE_BEFORE_TIME_AGE;
        if ((m_created_on_turn == INVALID_GAME_TURN) || (current_turn == INVALID_GAME_TURN))
            return INVALID_OBJECT_AGE;
        return current_turn - m_created_on_turn;
    }

    /** Returns ids of objects contained within this object. */
    [[nodiscard]] constexpr virtual std::span<const int> ContainedObjectIDs() const { return {}; }

    /** Returns true if there is an object with id \a object_id is contained
        within this UniverseObject. */
    [[nodiscard]] constexpr virtual bool Contains(int object_id) const {
        if (object_id == INVALID_OBJECT_ID)
            return false;
        const auto cids{this->ContainedObjectIDs()};
        return std::any_of(cids.begin(), cids.end(),
                           [object_id](const auto id) noexcept { return object_id == id; });
    }

    /* Returns true if there is an object with id \a object_id that contains this UniverseObject. */
    [[nodiscard]] constexpr virtual bool ContainedBy(int object_id) const noexcept
    { return object_id != INVALID_OBJECT_ID && ContainerObjectID() == object_id; }

    /** Returns id of the object that directly contains this object, if any, or
    INVALID_OBJECT_ID if this object is not contained by any other. */
    [[nodiscard]] constexpr virtual int ContainerObjectID() const noexcept { return INVALID_OBJECT_ID; }

    using MeterTypeVal = std::pair<MeterType, Meter>;
    using MeterSpan = std::span<MeterTypeVal>;
    using ConstMeterSpan = std::span<const MeterTypeVal>;

    [[nodiscard]] constexpr virtual MeterSpan Meters() { return {}; }
    [[nodiscard]] constexpr virtual ConstMeterSpan Meters() const { return {}; }

    ///< returns the requested Meter, or nullptr if no such Meter of that type is found in this object
    [[nodiscard]] constexpr Meter* GetMeter(MeterType type) noexcept {
        const auto meters = Meters();
        auto m_it = std::find_if(meters.begin(), meters.end(),
                                 [type](const auto& p) noexcept { return p.first == type; });
        return (m_it == meters.end()) ? nullptr : std::addressof(m_it->second);
    };
    [[nodiscard]] constexpr const Meter* GetMeter(MeterType type) const noexcept {
        const auto meters = Meters();
        auto m_it = std::find_if(meters.begin(), meters.end(),
                                 [type](const auto& p) noexcept { return p.first == type; });
        return (m_it == meters.end()) ? nullptr : std::addressof(m_it->second);
    };

    /** Returns the name of this objectas it appears to empire \a empire_id .*/
    [[nodiscard]] virtual const std::string& PublicName(int empire_id, const Universe& universe) const { return EMPTY_STRING; };

    struct [[nodiscard]] TagVecs {
        constexpr TagVecs() = default;
        constexpr explicit TagVecs(std::span<const std::string_view> vec) noexcept :
            first(vec)
        {}
        constexpr TagVecs(std::span<const std::string_view> vec1, std::span<const std::string_view> vec2) noexcept:
            first(vec1),
            second(vec2)
        {}
        TagVecs(std::vector<std::string_view>&&) = delete;
        [[nodiscard]] constexpr bool empty() const noexcept { return first.empty() && second.empty(); }
        [[nodiscard]] constexpr auto size() const noexcept { return first.size() + second.size(); }
        const std::span<const std::string_view> first;
        const std::span<const std::string_view> second;
    };
    [[nodiscard]] virtual TagVecs Tags(const ScriptingContext&) const { return {}; }; ///< Returns all tags this object has
    [[nodiscard]] virtual bool    HasTag(std::string_view name, const ScriptingContext&) const { return false; } ///< Returns true iff this object has the tag with the indicated \a name


    constexpr virtual void SetID(int id) { m_id = id; }             ///< sets the ID number of the object to \a id

    static constexpr double INVALID_POSITION = -100000.0;           ///< the position in x and y at which default-constructed objects are placed
    static constexpr int    INVALID_OBJECT_AGE = -(1 << 30) - 1;    ///< the age returned by UniverseObject::AgeInTurns() if the current turn is INVALID_GAME_TURN, or if the turn on which an object was created is INVALID_GAME_TURN
    static constexpr int    SINCE_BEFORE_TIME_AGE = (1 << 30) + 1;  ///< the age returned by UniverseObject::AgeInTurns() if an object was created on turn BEFORE_FIRST_TURN

protected:
    constexpr explicit UniverseObjectCXBase(UniverseObjectType type) noexcept : m_type{type} {}
    constexpr UniverseObjectCXBase(UniverseObjectType type, int owner_id, int creation_turn,
                                   double x = INVALID_POSITION, double y = INVALID_POSITION) noexcept :
        m_owner_empire_id{owner_id}, m_created_on_turn{creation_turn}, m_x{x}, m_y{y}, m_type{type}
    {}

    int                m_system_id = INVALID_OBJECT_ID;
    int                m_id = INVALID_OBJECT_ID;
    int                m_owner_empire_id = ALL_EMPIRES;
    int                m_created_on_turn = INVALID_GAME_TURN;
    double             m_x = INVALID_POSITION;
    double             m_y = INVALID_POSITION;
    UniverseObjectType m_type = UniverseObjectType::INVALID_UNIVERSE_OBJECT_TYPE;

    static inline const SpecialMap EMPTY_SPECIALS{};
    static inline CONSTEXPR_STRING const std::string EMPTY_STRING{};
};


/** The abstract base class for all objects in the universe
  * The UniverseObject class itself has an ID number, a name, a position, an ID
  * of the system in which it is, a list of zero or more owners, and other
  * common object data.
  * Position in the Universe can range from 0 (left) to 1000 (right) in X, and
  * 0 (top) to 1000 (bottom) in Y.  This coordinate system was chosen to help
  * with conversion to and from screen coordinates, which originate at the
  * upper-left corner of the screen and increase down and to the right.  Each
  * UniverseObject-derived class inherits several pure virtual members that
  * perform its actions during various game phases, such as the movement phase.
  * These subclasses must define what actions to perform during those phases.
  * UniverseObjects advertise changes to themselves via StateChangedSignal.
  * This means that all mutators on UniverseObject and its subclasses
  * need to emit this signal.  This is how the UI becomes aware that an object
  * that is being displayed has changed.*/
class FO_COMMON_API UniverseObject : public UniverseObjectCXBase {
public:
    using MeterMap = boost::container::flat_map<MeterType, Meter>;
    static_assert(std::is_same_v<boost::container::flat_map<MeterType, Meter, std::less<MeterType>>, MeterMap>);

    using IDSet = boost::container::flat_set<int32_t>;

    using CombinerType = assignable_blocking_combiner;
    using StateChangedSignalType = boost::signals2::signal<void (), CombinerType>;

    [[nodiscard]] const std::string& Name() const noexcept override final { return m_name; }///< returns the name of this object; some valid objects will have no name

    /** Object owner is at war with empire @p empire_id */
    [[nodiscard]] virtual bool HostileToEmpire(int empire_id, const EmpireManager& empires) const { return false; }

    [[nodiscard]] const SpecialMap& Specials() const noexcept override final { return m_specials; }
    [[nodiscard]] bool              HasSpecial(std::string_view name) const override final;         ///< true iff this object has a special with the indicated \a name
    [[nodiscard]] int               SpecialAddedOnTurn(std::string_view name) const override final; ///< turn on which the special with name \a name was added to this object, or INVALID_GAME_TURN if that special is not present
    [[nodiscard]] float             SpecialCapacity(std::string_view name) const override final;    ///> capacity of the special with name \a name or 0 if that special is not present

    [[nodiscard]] std::string       Dump(uint8_t ntabs = 0) const override;

    using EmpireObjectVisMap = std::map<int, std::map<int, Visibility>>;
    [[nodiscard]] IDSet             VisibleContainedObjectIDs(int empire_id, const EmpireObjectVisMap& vis) const; ///< returns the subset of contained object IDs that is visible to empire with id \a empire_id

protected:
    [[nodiscard]] static constexpr auto ToSpan(auto& in) {
        using InT = std::remove_reference_t<decltype(in)>;
        using in_value_type = typename InT::value_type;
        using OutT = std::conditional_t<std::is_const_v<InT>,
                                        std::span<const in_value_type>, std::span<in_value_type>>;
        if constexpr(requires { OutT(in); })
            return OutT(in);
        else if constexpr(requires { OutT(in.begin(), in.size()); })
            return OutT(in.begin(), in.size());
        else if constexpr(requires { OutT(in.begin(), in.end()); })
            return OutT(in.begin(), in.end());
        else if constexpr(requires { OutT(std::addressof(*in.begin()), in.size()); })
            return in.empty() ? OutT{} : OutT(std::addressof(*in.begin()), in.size());
        else
            static_assert(requires { OutT(in); });
    }

public:
    [[nodiscard]] MeterSpan         Meters() override { return ToSpan(m_meters); }
    [[nodiscard]] ConstMeterSpan    Meters() const override { return ToSpan(m_meters); }

    using EmpireIDtoObjectIDtoVisMap = std::map<int, std::map<int, Visibility>>; // duplicates Universe::EmpireObjectVisibilityMap
    [[nodiscard]] Visibility        GetVisibility(int empire_id, const EmpireIDtoObjectIDtoVisMap& v) const;
    [[nodiscard]] Visibility        GetVisibility(int empire_id, const Universe& u) const;

    [[nodiscard]] const std::string& PublicName(int, const Universe&) const override { return m_name; };

    [[nodiscard]] virtual std::size_t SizeInMemory() const;

    mutable StateChangedSignalType StateChangedSignal; ///< emitted when the UniverseObject is altered in any way

    /** copies data from \a copied_object to this object, limited to only copy
      * data about the copied object that is known to the empire with id
      * \a empire_id (or all data if empire_id is ALL_EMPIRES) */
    virtual void Copy(const UniverseObject& copied_object, const Universe&, int empire_id) = 0;

    void SetID(int id) override;     ///< sets the ID number of the object to \a id
    void Rename(std::string name);   ///< renames this object to \a name

    /** moves this object by relative displacements x and y. */
    void Move(double x, double y);

    /** moves this object to exact map coordinates of specified \a object. */
    void MoveTo(const auto& object) {
        if (!object) [[unlikely]]
            return;
        else if constexpr (requires { object.X(); })
            MoveTo(object.X(), object.Y());
        else if constexpr (requires { object->X(); })
            MoveTo(object->X(), object->Y());
        else
            static_assert(requires { object.X(); } || requires { object->X(); });
    }

    /** moves this object to map coordinates (x, y). */
    void MoveTo(double x, double y);

    /** Sets all this UniverseObject's meters' initial values equal to their current values. */
    virtual void BackPropagateMeters() noexcept;

    /** Sets the empire that owns this object. */
    virtual void SetOwner(int id);

    void SetSystem(int sys);                                        ///< assigns this object to a System.  does not actually move object in universe
    void AddSpecial(std::string name, float capacity, int turn);    ///< adds the Special \a name to this object, if it is not already present
    void RemoveSpecial(const std::string& name);                    ///< removes the Special \a name from this object, if it is already present
    void SetSpecialCapacity(std::string name, float capacity, int turn);

    /** Sets current value of max, target and unpaired meters in in this
      * UniverseObject to Meter::DEFAULT_VALUE.  This should be done before any
      * Effects that alter these meter(s) act on the object. */
    virtual void ResetTargetMaxUnpairedMeters();

    /** Sets current value of active paired meters (the non-max non-target
      * meters that have a max or target meter associated with them) back to
      * the initial value the meter had at the start of this turn. */
    virtual void ResetPairedActiveMeters();

    /** calls Clamp(min, max) on meters each meter in this UniverseObject, to
      * ensure that meter current values aren't outside the valid range for
      * each meter. */
    virtual void ClampMeters();

    /** performs the movement that this object is responsible for this object's
        actions during the pop growth/production/research phase of a turn. */
    virtual void PopGrowthProductionResearchPhase(ScriptingContext&) {}

    virtual ~UniverseObject() = default;
    UniverseObject(UniverseObject&&) = default;

    /** returns new copy of this UniverseObject, limited to only copy data that
      * is visible to the empire with the specified \a empire_id as determined
      * by the detection and visibility system.  Caller takes ownership of
      * returned pointee. */
    [[nodiscard]] virtual std::shared_ptr<UniverseObject> Clone(
        const Universe& universe, int empire_id = ALL_EMPIRES) const = 0;

protected:
    friend class Universe;
    friend class ObjectMap;
    template <typename Archive>
    friend void serialize(Archive& ar, Universe& u, unsigned int const version);

    UniverseObject() = delete;
    UniverseObject(UniverseObjectType type) : UniverseObjectCXBase(type) {}
    UniverseObject(UniverseObjectType type, std::string name, double x, double y, int owner_id, int creation_turn);
    UniverseObject(UniverseObjectType type, std::string name, int owner_id, int creation_turn);

    void SetSignalCombiner(const Universe& universe);

    template <typename T> friend void boost::python::detail::value_destroyer<false>::execute(T const volatile* p);

    void AddMeters(const auto& meter_types)
#if !defined(FREEORION_ANDROID)
        requires requires { {*meter_types.begin()} -> std::convertible_to<MeterType>; }
#else
        requires requires { static_cast<MeterType>(*meter_types.begin()); }
#endif
    {
        m_meters.reserve(m_meters.size() + meter_types.size());
        for (MeterType mt : meter_types)
            m_meters.emplace(std::piecewise_construct,
                             std::forward_as_tuple(mt),
                             std::forward_as_tuple());
    }

    /** Used by public UniverseObject::Copy and derived classes' ::Copy methods. */
    void Copy(const UniverseObject& copied_object, Visibility vis,
              const std::set<std::string>& visible_specials,
              const Universe& universe);

    std::string m_name;

private:
    [[nodiscard]] MeterMap CensoredMeters(Visibility vis) const; ///< returns set of meters of this object that are censored based on the specified Visibility \a vis

    MeterMap   m_meters{{MeterType::METER_STEALTH, Meter()}};
    SpecialMap m_specials; // map from special name to pair of (turn added, capacity)

    template <typename Archive>
    friend void serialize(Archive&, UniverseObject&, unsigned int const);
};

/** A function that returns the correct amount of spacing for an indentation of
  * \p ntabs during a dump. */
[[nodiscard]] inline std::string DumpIndent(uint8_t ntabs = 1)
{ return std::string(static_cast<size_t>(ntabs) * 4u, ' '); }


#endif
