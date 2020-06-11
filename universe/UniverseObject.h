#ifndef _UniverseObject_h_
#define _UniverseObject_h_


#include <set>
#include <string>
#include <boost/container/flat_map.hpp>
#include <boost/python/detail/destroy.hpp>
#include <boost/serialization/access.hpp>
#include <boost/signals2/optional_last_value.hpp>
#include <boost/signals2/signal.hpp>
#include "EnumsFwd.h"
#include "Meter.h"
#include "../util/blocking_combiner.h"
#include "../util/Export.h"


using boost::container::flat_map;

struct UniverseObjectVisitor;
FO_COMMON_API extern const int ALL_EMPIRES;
FO_COMMON_API extern const int INVALID_GAME_TURN;


//! The ID number assigned to a UniverseObject upon construction;
//! It is assigned an ID later when it is placed in the universe
FO_COMMON_API extern int const INVALID_OBJECT_ID;


//! The ID number assigned to temporary universe objects
FO_COMMON_API extern int const TEMPORARY_OBJECT_ID;


//! The abstract base class for all objects in the universe
//!
//! The UniverseObject class itself has an ID number, a name, a position, an ID
//! of the system in which it is, a list of zero or more owners, and other
//! common object data.
//!
//! Position in the Universe can range from 0 (left) to 1000 (right) in X, and
//! 0 (top) to 1000 (bottom) in Y.  This coordinate system was chosen to help
//! with conversion to and from screen coordinates, which originate at the
//! upper-left corner of the screen and increase down and to the right.
//!
//! Each UniverseObject-derived class inherits several pure virtual members
//! that perform its actions during various game phases, such as the movement
//! phase.  These subclasses must define what actions to perform during those
//! phases.
//!
//! UniverseObjects advertise changes to themselves via the StateChanged
//! Signal.  This means that all mutators on UniverseObject and its subclasses
//! need to emit this signal.  This is how the UI becomes aware that an object
//! that is being displayed has changed.
class FO_COMMON_API UniverseObject : virtual public std::enable_shared_from_this<UniverseObject> {
public:
    using MeterMap = flat_map<MeterType, Meter, std::less<MeterType>>;

    using StateChangedSignalType = boost::signals2::signal<void (), blocking_combiner<boost::signals2::optional_last_value<void>>>;

    using StateChangedSlotType = StateChangedSignalType::slot_type;

    virtual ~UniverseObject();

    //! Returns the ID number of this object.  Each object in FreeOrion has
    //! a unique ID number.
    auto ID() const -> int
    { return m_id; }

    void SetID(int id);

    //! Returns the name of this object; some valid objects will have no name.
    auto Name() const -> std::string const&
    { return m_name; }

    //! Renames this object to @p name
    void Rename(std::string const& name);

    //! The X-coordinate of this object.
    auto X() const -> double
    { return m_x; }

    //! The Y-coordinate of this object.
    auto Y() const -> double
    { return m_y; }

    //! Moves this object by relative displacements x and y.
    void Move(double x, double y);

    //! Moves this object to exact map coordinates of the object identified by
    //! @p object_id
    void MoveTo(int object_id);

    //! Moves this object to exact map coordinates of specified @p object.
    void MoveTo(std::shared_ptr<UniverseObject> object);

    //! Moves this object to map coordinates (x, y).
    void MoveTo(double x, double y);

    //! Returns the ID of the empire that owns this object, or ALL_EMPIRES if
    //! there is no owner.
    virtual auto Owner() const -> int
    { return m_owner_empire_id; }

    //! Returns true iff there are no owners of this object.
    auto Unowned() const -> bool
    { return Owner() == ALL_EMPIRES; }

    //! Returns true iff the empire with id @p empire owns this object; unowned
    //! objects always return false;
    auto OwnedBy(int empire) const -> bool
    { return empire != ALL_EMPIRES && empire == Owner(); }

    //! Sets the empire that owns this object.
    virtual void SetOwner(int id);

    //! Object owner is at war with empire @p empire_id
    virtual auto HostileToEmpire(int empire_id) const -> bool
    { return false; }

    //! Returns the ID number of the system in which this object can be found,
    //! or INVALID_OBJECT_ID if the object is not within any system
    virtual auto SystemID() const -> int
    { return m_system_id; }

    //! Assigns this object to a System.  Does not actually move object in
    //! universe.
    void SetSystem(int sys);

    //! Returns the Specials attached to this object
    auto Specials() const -> std::map<std::string, std::pair<int, float>> const&
    { return m_specials; }

    //! Returns true iff this object has a special with the indicated @p name
    auto HasSpecial(const std::string& name) const -> bool
    { return m_specials.count(name); }

    //! Returns the turn on which the special with name @p name was added to
    //! this object, or INVALID_GAME_TURN if that special is not present
    auto SpecialAddedOnTurn(std::string const& name) const -> int;

    //! Returns the capacity of the special with name @p name or 0 if that
    //! special is not present
    auto SpecialCapacity(std::string const& name) const -> float;

    //! Adds the Special @p name to this object, if it is not already present
    virtual void AddSpecial(std::string const& name, float capacity = 0.0f);

    //! Removes the Special @p name from this object, if it is already present
    virtual void RemoveSpecial(std::string const& name);

    void SetSpecialCapacity(std::string const& name, float capacity);

    //! Returns all tags this object has.
    virtual auto Tags() const -> std::set<std::string>
    { return {}; }

    //! Returns true iff this object has the tag with the indicated @p name.
    virtual auto HasTag(const std::string& name) const -> bool
    { return false; }

    virtual auto ObjectType() const -> UniverseObjectType;

    //! Return human readable string description of object offset @p ntabs from
    //! margin.
    virtual auto Dump(unsigned short ntabs = 0) const -> std::string;

    //! Returns id of the object that directly contains this object, if any, or
    //! INVALID_OBJECT_ID if this object is not contained by any other.
    virtual auto ContainerObjectID() const -> int
    { return INVALID_OBJECT_ID; }

    //! Returns ids of objects contained within this object.
    virtual auto ContainedObjectIDs() const -> std::set<int> const&;

    //! Returns true if there is an object with id @p object_id is contained
    //! within this UniverseObject.
    virtual auto Contains(int object_id) const -> bool
    { return false; }

    //! Returns true if there is an object with id @p object_id that contains
    //!this UniverseObject.
    virtual auto ContainedBy(int object_id) const -> bool
    { return false; }

    //! Returns the subset of contained object IDs that is visible to empire
    //! with id @p empire_id
    auto VisibleContainedObjectIDs(int empire_id) const -> std::set<int>;

    //! Returns this UniverseObject's meters
    // @{
    auto Meters() -> MeterMap&
    { return m_meters; }

    auto Meters() const -> MeterMap const&
    { return m_meters; }
    // @}

    //! Returns the requested Meter, or nullptr if no such Meter of that type
    //! is found in this object
    // @{
    auto GetMeter(MeterType type) -> Meter*;
    auto GetMeter(MeterType type) const -> Meter const*;
    // @}

    //! Sets all this UniverseObject's meters' initial values equal to their
    //! current values.
    virtual void BackPropagateMeters();

    //! Sets current value of max, target and unpaired meters in in this
    //! UniverseObject to Meter::DEFAULT_VALUE.  This should be done before any
    //! Effects that alter these meter(s) act on the object.
    virtual void ResetTargetMaxUnpairedMeters();

    //! Sets current value of active paired meters (the non-max non-target
    //! meters that have a max or target meter associated with them) back to
    //! the initial value the meter had at the start of this turn.
    virtual void ResetPairedActiveMeters();

    //! calls Clamp(min, max) on meters each meter in this UniverseObject, to
    //! ensure that meter current values aren't outside the valid range for
    //! each meter.
    virtual void ClampMeters();


    //! Returns the visibility status of this universe object relative to the
    //! input empire.
    auto GetVisibility(int empire_id) const -> Visibility;

    //! Returns the name of this object as it appears to empire @p empire_id.
    virtual auto PublicName(int empire_id) const -> std::string const&
    { return m_name; }

    //! Accepts a visitor object @see UniverseObjectVisitor
    virtual auto Accept(UniverseObjectVisitor const& visitor) const -> std::shared_ptr<UniverseObject>;

    //! Returns game turn on which object was created
    auto CreationTurn() const -> int
    { return m_created_on_turn; }

    //! The age value if the current turn is INVALID_GAME_TURN, or if the turn
    //! on which an object was created is INVALID_GAME_TURN.
    static int const INVALID_OBJECT_AGE;

    //! The age value if an object was created on turn BEFORE_FIRST_TURN.
    static int const SINCE_BEFORE_TIME_AGE;

    //! Returns elapsed number of turns between turn object was created and
    //! current game turn
    auto AgeInTurns() const -> int;

    //! Emitted when the UniverseObject is altered in any way
    mutable StateChangedSignalType StateChangedSignal;

    //! copies data from @p copied_object to this object, limited to only copy
    //! data about the copied object that is known to the empire with id
    //! @p empire_id (or all data if empire_id is ALL_EMPIRES)
    virtual void Copy(std::shared_ptr<UniverseObject const> copied_object, int empire_id) = 0;

    //! Performs the movement that this object is responsible for this object's
    //! actions during the movement phase of a turn.
    virtual void MovementPhase()
    {};

    //! Performs the movement that this object is responsible for this object's
    //! actions during the pop growth/production/research phase of a turn.
    virtual void PopGrowthProductionResearchPhase()
    {};

    //! The position in x and y at which default-constructed objects are placed
    static double const INVALID_POSITION;

protected:
    friend class Universe;
    friend class ObjectMap;

    UniverseObject();

    UniverseObject(std::string const name, double x, double y);

    template <typename T>
    friend void boost::python::detail::value_destroyer<false>::execute(T const volatile* p);

    //! returns new copy of this UniverseObject, limited to only copy data that
    //! is visible to the empire with the specified \a empire_id as determined
    //! by the detection and visibility system.  Caller takes ownership of
    //! returned pointee.
    virtual auto Clone(int empire_id = ALL_EMPIRES) const -> UniverseObject* = 0;
    //@}

    //! Inserts a meter into object as the \a meter_type meter.  Should be used
    //! by derived classes to add their specialized meters to objects
    void AddMeter(MeterType meter_type);

    //! Adds meter
    void Init();

    //! Used by public UniverseObject::Copy and derived classes' ::Copy methods.
    void Copy(std::shared_ptr<UniverseObject const> copied_object, Visibility vis,
              std::set<std::string> const& visible_specials);

    std::string             m_name;

private:
    //! Returns set of meters of this object that are censored based on the
    //! specified Visibility @p vis
    auto CensoredMeters(Visibility vis) const -> MeterMap;

    int                                           m_id = INVALID_OBJECT_ID;
    double                                        m_x;
    double                                        m_y;
    int                                           m_owner_empire_id = ALL_EMPIRES;
    int                                           m_system_id = INVALID_OBJECT_ID;
    //! map from special name to pair of (turn added, capacity)
    std::map<std::string, std::pair<int, float>>  m_specials;
    MeterMap                                      m_meters;
    int                                           m_created_on_turn = INVALID_GAME_TURN;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, unsigned int const version);
};


//! A function that returns the correct amount of spacing for an indentation of
//! @p ntabs during a dump.
inline std::string DumpIndent(unsigned short ntabs = 1)
{ return std::string(ntabs * 4 /* conversion to size_t is safe */, ' '); }


#endif // _UniverseObject_h_
