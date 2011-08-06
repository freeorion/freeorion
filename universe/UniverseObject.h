// -*- C++ -*-
// We need to include Universe.h before trying to define
// UniverseObject. This is necessary to make GCC 3.4+ happy.
#ifndef _Universe_h_
#include "Universe.h"
#endif

#ifndef _UniverseObject_h_
#define _UniverseObject_h_

#include "InhibitableSignal.h"

#include <set>
#include <string>
#include <vector>

class Meter;
class System;
class SitRepEntry;
struct UniverseObjectVisitor;

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
class UniverseObject
{
public:
    /** \name Signal Types */ //@{
    typedef boost::signal<void ()>                          StateChangedSignalBaseType;
    typedef InhibitableSignal<StateChangedSignalBaseType>   StateChangedSignalType;
    //@}

    /** \name Slot Types */ //@{
    typedef StateChangedSignalType::slot_type               StateChangedSlotType;
    //@}

    /** \name Structors */ //@{
    UniverseObject();                                           ///< default ctor

    /** general ctor.  \throw std::invalid_argument May throw
        std::invalid_argument if the either x or y coordinate is
        outside the map area.*/
    UniverseObject(const std::string name, double x, double y, const std::set<int>& owners = std::set<int>());

    UniverseObject(const UniverseObject& rhs);                  ///< copy ctor

    virtual ~UniverseObject();                                  ///< dtor

    /** returns new copy of this UniverseObject, limited to only copy data that
      * is visible to the empire with the specified \a empire_id as determined
      * by the detection and visibility system.  Caller takes ownership of
      * returned pointee. */
    virtual UniverseObject* Clone(int empire_id = ALL_EMPIRES) const = 0;
    //@}

    /** \name Accessors */ //@{
    int                         ID() const;                         ///< returns the ID number of this object.  Each object in FreeOrion has a unique ID number.
    const std::string&          Name() const;                       ///< returns the name of this object; some valid objects will have no name
    double                      X() const;                          ///< the X-coordinate of this object
    double                      Y() const;                          ///< the Y-coordinate of this object
    int                         Owner() const;                      ///< returns the ID of the empire that owns this object, or ALL_EMPIRES if there is no owner
    virtual int                 SystemID() const;                   ///< returns the ID number of the system in which this object can be found, or INVALID_OBJECT_ID if the object is not within any system
    const std::set<std::string>&Specials() const;                   ///< returns the set of names of the Specials attached to this object

    virtual const std::string&  TypeName() const;                   ///< returns user-readable string indicating the type of UniverseObject this is
    virtual std::string         Dump() const;                       ///< outputs textual description of object to logger

    virtual std::vector<int>    FindObjectIDs() const;              ///< returns ids of objects contained within this object

    virtual bool                Contains(int object_id) const;      ///< returns true if there is an object with id \a object_id is contained within this UniverseObject
    virtual bool                ContainedBy(int object_id) const;   ///< returns true if there is an object with id \a object_id that contains this UniverseObject

    const Meter*                GetMeter(MeterType type) const;                 ///< returns the requested Meter, or 0 if no such Meter of that type is found in this object
    double                      CurrentMeterValue(MeterType type) const;        ///< returns current value of the specified meter \a type
    double                      InitialMeterValue(MeterType type) const;        ///< returns this turn's initial value for the speicified meter \a type
    virtual double              NextTurnCurrentMeterValue(MeterType type) const;///< returns an estimate of the next turn's current value of the specified meter \a type

    bool                        Unowned() const;                    ///< returns true iff there are no owners of this object
    bool                        OwnedBy(int empire) const;          ///< returns true iff the empire with id \a empire owns this object

    Visibility                  GetVisibility(int empire_id) const; ///< returns the visibility status of this universe object relative to the input empire.
    virtual const std::string&  PublicName(int empire_id) const;    ///< returns the name of this objectas it appears to empire \a empire_id

    /** accepts a visitor object \see UniverseObjectVisitor */
    virtual UniverseObject* Accept(const UniverseObjectVisitor& visitor) const;

    int                     CreationTurn() const;               ///< returns game turn on which object was created
    int                     AgeInTurns() const;                 ///< returns elapsed number of turns between turn object was created and current game turn

    mutable StateChangedSignalType StateChangedSignal;          ///< emitted when the UniverseObject is altered in any way
    //@}

    /** \name Mutators */ //@{
    /** copies data from \a copied_object to this object, limited to only copy
      * data about the copied object that is known to the empire with id
      * \a empire_id (or all data if empire_id is ALL_EMPIRES) */
    virtual void            Copy(const UniverseObject* copied_object, int empire_id = ALL_EMPIRES) = 0;

    void                    SetID(int id);                      ///< sets the ID number of the object to \a id
    void                    Rename(const std::string& name);    ///< renames this object to \a name

    /** moves this object by relative displacements x and y. \throw std::runtime_error May throw std::runtime_error if the result
        of the move would place either coordinate outside the map area.*/
    void                    Move(double x, double y);

    /** calls MoveTo(const UniverseObject*) with the object pointed to by \a object_id. */
    void                    MoveTo(int object_id);

    /** moves this object and contained objects to exact map coordinates of specified \a object
        If \a object is a system, places this object into that system.
        May throw std::invalid_argument if \a object is not a valid object*/
    void                    MoveTo(UniverseObject* object);

    /** moves this object and contained objects to exact map coordinates (x, y). \throw std::invalid_arugment
        May throw std::invalid_arugment if the either coordinate of the move is outside the map area.*/
    virtual void            MoveTo(double x, double y);


    Meter*                  GetMeter(MeterType type);               ///< returns the requested Meter, or 0 if no such Meter of that type is found in this object
    void                    BackPropegateMeters();                  ///< sets all this UniverseObject's meters' initial values equal to their current values

    virtual void            SetOwner(int id);                       ///< sets the empire that owns this object

    virtual void            SetSystem(int sys);                     ///< assigns this object to a System.  does not actually move object in universe
    virtual void            AddSpecial(const std::string& name);    ///< adds the Special \a name to this object, if it is not already present
    virtual void            RemoveSpecial(const std::string& name); ///< removes the Special \a name from this object, if it is already present

    /** Performs the movement that this object is responsible for this object's
      * actions during the movement phase of a turn. */
    virtual void            MovementPhase() {};

    /** Sets current value of max, target and unpaired meters in in this
      * UniverseObject to Meter::DEFAULT_VALUE.  This should be done before any
      * Effects that alter these meter(s) act on the object.  if \a meter_type
      * is INVALID_METER_TYPE, all meters are reset.  if \a meter_type is a
      * valid meter type, just that meter is reset. */
    virtual void            ResetTargetMaxUnpairedMeters(MeterType meter_type = INVALID_METER_TYPE);

    /** Sets current value of active paired meters (the non-max non-target
      * meters that have a max or target meter associated with them) back to
      * the initial value the meter had at the start of this turn. */
    virtual void            ResetPairedActiveMeters(MeterType meter_type = INVALID_METER_TYPE);

    /** calls Clamp(min, max) on meters each meter in this UniverseObject, to
      * ensure that meter current values aren't outside the valid range for
      * each meter. */
    virtual void            ClampMeters();

    /** performs the movement that this object is responsible for this object's actions during the pop growth/production/research
        phase of a turn. */
    virtual void            PopGrowthProductionResearchPhase() {};
    //@}

    static const double         INVALID_POSITION;       ///< the position in x and y at which default-constructed objects are placed
    static const int            INVALID_OBJECT_ID;      ///< the ID number assigned to a UniverseObject upon construction; it is assigned an ID later when it is placed in the universe
    static const int            MAX_ID;                 ///< the max ID number 
    static const int            INVALID_OBJECT_AGE;     ///< the age returned by UniverseObject::AgeInTurns() if the current turn is INVALID_GAME_TURN, or if the turn on which an object was created is INVALID_GAME_TURN
    static const int            SINCE_BEFORE_TIME_AGE;  ///< the age returned by UniverseObject::AgeInTurns() if an object was created on turn BEFORE_FIRST_TURN

protected:
    void                    AddMeter(MeterType meter_type); ///< inserts a meter into object as the \a meter_type meter.  Should be used by derived classes to add their specialized meters to objects
    void                    Init();                         ///< adds stealth meter

    void                    Copy(const UniverseObject* copied_object, Visibility vis);  ///< used by public UniverseObject::Copy and derived classes' ::Copy methods

private:
    std::map<MeterType, Meter>  CensoredMeters(Visibility vis) const;   ///< returns set of meters of this object that are censored based on the specified Visibility \a vis

    int                         m_id;
    std::string                 m_name;
    double                      m_x;
    double                      m_y;
    int                         m_owner_empire_id;
    int                         m_system_id;
    std::set<std::string>       m_specials;
    std::map<MeterType, Meter>  m_meters;
    int                         m_created_on_turn;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

#endif // _UniverseObject_h_
