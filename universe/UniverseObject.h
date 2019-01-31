#ifndef _UniverseObject_h_
#define _UniverseObject_h_


#include "EnumsFwd.h"
#include "../util/Export.h"
#include "../util/blocking_combiner.h"

#include <boost/serialization/access.hpp>
#include <boost/python/detail/destroy.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/signals2/optional_last_value.hpp>

#include <set>
#include <string>
#include <vector>

class Meter;
class System;
class SitRepEntry;
struct UniverseObjectVisitor;
FO_COMMON_API extern const int ALL_EMPIRES;
FO_COMMON_API extern const int INVALID_GAME_TURN;

// The ID number assigned to a UniverseObject upon construction;
// It is assigned an ID later when it is placed in the universe
FO_COMMON_API extern const int INVALID_OBJECT_ID;

// The ID number assigned to temporary universe objects
FO_COMMON_API extern const int TEMPORARY_OBJECT_ID;

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
  * UniverseObjects advertise changes to themselves via the StateChanged
  * Signal.  This means that all mutators on UniverseObject and its subclasses
  * need to emit this signal.  This is how the UI becomes aware that an object
  * that is being displayed has changed.*/
class FO_COMMON_API UniverseObject : virtual public std::enable_shared_from_this<UniverseObject> {
public:
    /** \name Signal Types */ //@{
    typedef boost::signals2::signal<void (), blocking_combiner<boost::signals2::optional_last_value<void>>> StateChangedSignalType;
    //@}

    /** \name Slot Types */ //@{
    typedef StateChangedSignalType::slot_type               StateChangedSlotType;
    //@}

    /** \name Accessors */ //@{
    int                         ID() const;                         ///< returns the ID number of this object.  Each object in FreeOrion has a unique ID number.
    const std::string&          Name() const;                       ///< returns the name of this object; some valid objects will have no name
    virtual double              X() const;                          ///< the X-coordinate of this object
    virtual double              Y() const;                          ///< the Y-coordinate of this object

    virtual int                 Owner() const;                      ///< returns the ID of the empire that owns this object, or ALL_EMPIRES if there is no owner
    bool                        Unowned() const;                    ///< returns true iff there are no owners of this object
    bool                        OwnedBy(int empire) const;          ///< returns true iff the empire with id \a empire owns this object; unowned objects always return false;
    /** Object owner is at war with empire @p empire_id */
    virtual bool                HostileToEmpire(int empire_id) const;

    virtual int                 SystemID() const;                   ///< returns the ID number of the system in which this object can be found, or INVALID_OBJECT_ID if the object is not within any system

    const std::map<std::string, std::pair<int, float>>&   Specials() const;         ///< returns the Specials attached to this object
    bool                        HasSpecial(const std::string& name) const;          ///< returns true iff this object has a special with the indicated \a name
    int                         SpecialAddedOnTurn(const std::string& name) const;  ///< returns the turn on which the special with name \a name was added to this object, or INVALID_GAME_TURN if that special is not present
    float                       SpecialCapacity(const std::string& name) const;     ///> returns the capacity of the special with name \a name or 0 if that special is not present

    /** Returns all tags this object has. */
    virtual std::set<std::string>   Tags() const;

    /** Returns true iff this object has the tag with the indicated \a name. */
    virtual bool                HasTag(const std::string& name) const;

    virtual UniverseObjectType  ObjectType() const;

    /** Return human readable string description of object offset \p ntabs from
        margin. */
    virtual std::string         Dump(unsigned short ntabs = 0) const;

    /** Returns id of the object that directly contains this object, if any, or
        INVALID_OBJECT_ID if this object is not contained by any other. */
    virtual int                 ContainerObjectID() const;

    /** Returns ids of objects contained within this object. */
    virtual const std::set<int>&ContainedObjectIDs() const;

    /** Returns true if there is an object with id \a object_id is contained
        within this UniverseObject. */
    virtual bool                Contains(int object_id) const;

    /* Returns true if there is an object with id \a object_id that contains
       this UniverseObject. */
    virtual bool                ContainedBy(int object_id) const;

    std::set<int>               VisibleContainedObjectIDs(int empire_id) const; ///< returns the subset of contained object IDs that is visible to empire with id \a empire_id

    const std::map<MeterType, Meter>&   Meters() const { return m_meters; }     ///< returns this UniverseObject's meters
    const Meter*                GetMeter(MeterType type) const;                 ///< returns the requested Meter, or 0 if no such Meter of that type is found in this object
    float                       CurrentMeterValue(MeterType type) const;        ///< returns current value of the specified meter \a type
    float                       InitialMeterValue(MeterType type) const;        ///< returns this turn's initial value for the speicified meter \a type

    Visibility                  GetVisibility(int empire_id) const; ///< returns the visibility status of this universe object relative to the input empire.

    /** Returns the name of this objectas it appears to empire \a empire_id .*/
    virtual const std::string&  PublicName(int empire_id) const;

    /** Accepts a visitor object \see UniverseObjectVisitor */
    virtual std::shared_ptr<UniverseObject> Accept(const UniverseObjectVisitor& visitor) const;

    int                         CreationTurn() const;               ///< returns game turn on which object was created
    int                         AgeInTurns() const;                 ///< returns elapsed number of turns between turn object was created and current game turn

    mutable StateChangedSignalType StateChangedSignal;              ///< emitted when the UniverseObject is altered in any way
    //@}

    /** \name Mutators */ //@{
    /** copies data from \a copied_object to this object, limited to only copy
      * data about the copied object that is known to the empire with id
      * \a empire_id (or all data if empire_id is ALL_EMPIRES) */
    virtual void    Copy(std::shared_ptr<const UniverseObject> copied_object, int empire_id) = 0;

    void            SetID(int id);                      ///< sets the ID number of the object to \a id
    void            Rename(const std::string& name);    ///< renames this object to \a name

    /** moves this object by relative displacements x and y. */
    void            Move(double x, double y);

    /** calls MoveTo(std::shared_ptr<const UniverseObject>) with the object
        pointed to by \a object_id. */
    void            MoveTo(int object_id);

    /** moves this object to exact map coordinates of specified \a object. */
    void            MoveTo(std::shared_ptr<UniverseObject> object);

    /** moves this object to map coordinates (x, y). */
    void            MoveTo(double x, double y);


    std::map<MeterType, Meter>&
                    Meters() { return m_meters; }           ///< returns this UniverseObject's meters
    Meter*          GetMeter(MeterType type);               ///< returns the requested Meter, or 0 if no such Meter of that type is found in this object

    /** Sets all this UniverseObject's meters' initial values equal to their
        current values. */
    virtual void    BackPropagateMeters();

    /** Sets the empire that owns this object. */
    virtual void    SetOwner(int id);

    void            SetSystem(int sys);                     ///< assigns this object to a System.  does not actually move object in universe
    virtual void    AddSpecial(const std::string& name, float capacity = 0.0f); ///< adds the Special \a name to this object, if it is not already present
    virtual void    RemoveSpecial(const std::string& name); ///< removes the Special \a name from this object, if it is already present
    void            SetSpecialCapacity(const std::string& name, float capacity);

    /** Performs the movement that this object is responsible for this object's
      * actions during the movement phase of a turn. */
    virtual void    MovementPhase()
    {};

    /** Sets current value of max, target and unpaired meters in in this
      * UniverseObject to Meter::DEFAULT_VALUE.  This should be done before any
      * Effects that alter these meter(s) act on the object. */
    virtual void    ResetTargetMaxUnpairedMeters();

    /** Sets current value of active paired meters (the non-max non-target
      * meters that have a max or target meter associated with them) back to
      * the initial value the meter had at the start of this turn. */
    virtual void    ResetPairedActiveMeters();

    /** calls Clamp(min, max) on meters each meter in this UniverseObject, to
      * ensure that meter current values aren't outside the valid range for
      * each meter. */
    virtual void    ClampMeters();

    /** performs the movement that this object is responsible for this object's
        actions during the pop growth/production/research phase of a turn. */
    virtual void    PopGrowthProductionResearchPhase()
    {};
    //@}

    static const double INVALID_POSITION;       ///< the position in x and y at which default-constructed objects are placed
    static const int    INVALID_OBJECT_AGE;     ///< the age returned by UniverseObject::AgeInTurns() if the current turn is INVALID_GAME_TURN, or if the turn on which an object was created is INVALID_GAME_TURN
    static const int    SINCE_BEFORE_TIME_AGE;  ///< the age returned by UniverseObject::AgeInTurns() if an object was created on turn BEFORE_FIRST_TURN

protected:
    friend class Universe;
    friend class ObjectMap;

    /** \name Structors */ //@{
    UniverseObject();
    UniverseObject(const std::string name, double x, double y);

    template <class T> friend void boost::python::detail::value_destroyer<false>::execute(T const volatile* p);

public:
    virtual ~UniverseObject();

protected:
    /** returns new copy of this UniverseObject, limited to only copy data that
      * is visible to the empire with the specified \a empire_id as determined
      * by the detection and visibility system.  Caller takes ownership of
      * returned pointee. */
    virtual UniverseObject* Clone(int empire_id = ALL_EMPIRES) const = 0;
    //@}

    void                    AddMeter(MeterType meter_type); ///< inserts a meter into object as the \a meter_type meter.  Should be used by derived classes to add their specialized meters to objects
    void                    Init();                         ///< adds stealth meter

    /** Used by public UniverseObject::Copy and derived classes' ::Copy methods.
      */
    void Copy(std::shared_ptr<const UniverseObject> copied_object, Visibility vis,
              const std::set<std::string>& visible_specials);

    std::string             m_name;

private:
    std::map<MeterType, Meter>  CensoredMeters(Visibility vis) const;   ///< returns set of meters of this object that are censored based on the specified Visibility \a vis

    int                                             m_id = INVALID_OBJECT_ID;
    double                                          m_x;
    double                                          m_y;
    int                                             m_owner_empire_id = ALL_EMPIRES;
    int                                             m_system_id = INVALID_OBJECT_ID;
    std::map<std::string, std::pair<int, float>>    m_specials; // map from special name to pair of (turn added, capacity)
    std::map<MeterType, Meter>                      m_meters;
    int                                             m_created_on_turn = INVALID_GAME_TURN;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** A function that returns the correct amount of spacing for an indentation of
  * \p ntabs during a dump. */
inline std::string DumpIndent(unsigned short ntabs = 1)
{ return std::string(ntabs * 4 /* conversion to size_t is safe */, ' '); }

#endif // _UniverseObject_h_
