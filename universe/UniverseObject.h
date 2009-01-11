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

/** Signal return value combiner used by ResourceCenter, PopCenter, and any other UniverseObject decorator that needs
    access to its UniverseObject subclass. */
struct Default0Combiner
{
    typedef UniverseObject* result_type;
    template <class Iter>
    UniverseObject* operator()(Iter first, Iter last);
};

class Meter;
class System;
class SitRepEntry;
struct UniverseObjectVisitor;

/** the abstract base class for all objects in the universe.  The UniverseObject class itself has only an ID, a name, 
    a position, possibly a System in which it is, and zero or more owners.  The position can range from 0 (left) to 1000 
    (right) in X, and 0 (top) to 1000 (bottom) in Y.  This coordinate system was chosen to help with conversion to 
    and from screen coordinates, which originate at the upper-left corner of the screen and increase down and to the 
    right.  Each UniverseObject-derived class inherits serveral pure virtual members that perform its actions during 
    various game phases, such as the movement phase.  These subclasses must define what actions to perform during 
    those phases.  UniverseObjects advertise changes to themselves via the StateChanged signal.  This means that all
    mutators on UniverseObject and its subclasses need to emit this signal.  This is how the UI becomes aware that an
    object that is being dislayed has changed.*/
class UniverseObject
{
public:
    /** \name Signal Types */ //@{
    typedef boost::signal<void ()> StateChangedSignalBaseType; ///< used to define StateChangedSignalType
    typedef InhibitableSignal<StateChangedSignalBaseType> StateChangedSignalType; ///< emitted when the UniverseObject is altered in any way
    //@}

    /** \name Slot Types */ //@{
    typedef StateChangedSignalType::slot_type StateChangedSlotType; ///< type of functor(s) invoked on a StateChangedSignalType
    //@}

    /** \name Structors */ //@{
    UniverseObject();    ///< default ctor

    /** general ctor.  \throw std::invalid_argument May throw
        std::invalid_argument if the either x or y coordinate is
        outside the map area.*/
    UniverseObject(const std::string name, double x, double y, const std::set<int>& owners = std::set<int>());

    virtual ~UniverseObject();   ///< dtor
    //@}

    /** \name Accessors */ //@{
    int                     ID() const;                         ///< returns the ID number of this object.  Each object in FreeOrion has a unique ID number.
    const std::string&      Name() const;                       ///< returns the name of this object; some valid objects will have no name
    double                  X() const;                          ///< the X-coordinate of this object
    double                  Y() const;                          ///< the Y-coordinate of this object
    const std::set<int>&    Owners() const;                     ///< returns the set of IDs of Empires owning all or part of this object.  \note This may be empty or have an arbitrary number of elements.
    virtual int             SystemID() const;                   ///< returns the ID number of the system in which this object can be found, or INVALID_OBJECT_ID if the object is not within any system
    System*                 GetSystem() const;                  ///< returns system in which this object can be found, or null if the object is not within any system
    const std::set<std::string>&
                            Specials() const;                   ///< returns the set of names of the Specials attached to this object

    virtual std::vector<UniverseObject*>
                            FindObjects() const;                ///< returns objects contained within this object
    virtual std::vector<int>
                            FindObjectIDs() const;              ///< returns ids of objects contained within this object

    virtual bool            Contains(int object_id) const;                  ///< returns true if there is an object with id \a object_id is contained within this UniverseObject
    virtual bool            ContainedBy(int object_id) const;               ///< returns true if there is an object with id \a object_id that contains this UniverseObject

    const Meter*            GetMeter(MeterType type) const;                 ///< returns the requested Meter, or 0 if no such Meter of that type is found in this object
    virtual double          ProjectedCurrentMeter(MeterType type) const;    ///< returns expected value of  specified meter current value on the next turn
    virtual double          MeterPoints(MeterType type) const;              ///< returns "true amount" associated with a meter.  In some cases (METER_POPULATION) this is just the meter value.  In other cases (METER_FARMING) this is some other value (a function of population and meter value)
    virtual double          ProjectedMeterPoints(MeterType type) const;     ///< returns expected "true amount" associated with a meter on the next turn

    bool                    Unowned() const;                    ///< returns true iff there are no owners of this object
    bool                    OwnedBy(int empire) const;          ///< returns true iff the empire with id \a empire is an owner of this object
    bool                    WhollyOwnedBy(int empire) const;    ///< returns true iff the empire with id \a empire is the only owner of this object

    virtual Visibility      GetVisibility(int empire_id) const; ///< returns the visibility status of this universe object relative to the input empire.
    virtual const std::string&
                            PublicName(int empire_id) const;    ///< returns the name of this objectas it appears to empire \a empire_id

    /** accepts a visitor object \see UniverseObjectVisitor */
    virtual UniverseObject* Accept(const UniverseObjectVisitor& visitor) const;

    int                     CreationTurn() const;               ///< returns game turn on which object was created
    int                     AgeInTurns() const;                 ///< returns elapsed number of turns between turn object was created and current game turn

    mutable StateChangedSignalType StateChangedSignal;          ///< returns the state changed signal object for this UniverseObject
    //@}

    /** \name Mutators */ //@{
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

    virtual void            AddOwner(int id);                       ///< adds the Empire with ID \a id to the list of owners of this object
    virtual void            RemoveOwner(int id);                    ///< removes the Empire with ID \a id to the list of owners of this object
    virtual void            SetSystem(int sys);                     ///< assigns this object to a System.  does not actually move object in universe
    virtual void            AddSpecial(const std::string& name);    ///< adds the Special \a name to this object, if it is not already present
    virtual void            RemoveSpecial(const std::string& name); ///< removes the Special \a name from this object, if it is already present

    /** performs the movement that this object is responsible for this object's actions during the movement phase of 
        a turn. */
    virtual void            MovementPhase() = 0;

    /** sets max meter value(s) for meter(s) in this UniverseObject to Meter::METER_MIN.  This should be done before any
      * Effects that alter meter(s) act on the object.  if \a meter_type is INVALID_METER_TYPE, all meters are reset.  if
      * \a meter_type is a valid meter type, just that meter is reset. */
    void                    ResetMaxMeters(MeterType meter_type = INVALID_METER_TYPE);

    /** adjusts max meter value(s) for meter(s) in this UniverseObject, based on its own properties (ie. not due to effects).
      * if \a meter_type is INVALID_METER_TYPE, all meter(s) are adjusted.  If \a meter_type is a valid meter type, just that 
      * meter is adjusted. */
    virtual void            ApplyUniverseTableMaxMeterAdjustments(MeterType meter_type = INVALID_METER_TYPE);

    /** calls Clamp() on each meter in this UniverseObject, to ensure that no Meter's Max() falls outisde the range
      * [Meter::METER_MIN, METER::METER_MAX]and that no Meter's Current() value exceed its Max() value. */
    void                    ClampMeters();

    /** performs the movement that this object is responsible for this object's actions during the pop growth/production/research
        phase of a turn. */
    virtual void            PopGrowthProductionResearchPhase() = 0;
    //@}

    static const double         INVALID_POSITION;       ///< the position in x and y at which default-constructed objects are placed
    static const int            INVALID_OBJECT_ID;      ///< the ID number assigned to a UniverseObject upon construction; it is assigned an ID later when it is placed in the universe
    static const int            MAX_ID;                 ///< the max ID number 
    static const int            INVALID_OBJECT_AGE;     ///< the age returned by UniverseObject::AgeInTurns() if the current turn is INVALID_GAME_TURN, or if the turn on which an object was created is INVALID_GAME_TURN
    static const int            SINCE_BEFORE_TIME_AGE;  ///< the age returned by UniverseObject::AgeInTurns() if an object was created on turn BEFORE_FIRST_TURN

protected:
    void                    InsertMeter(MeterType meter_type, const Meter& meter);  ///< inserts \a meter into object as the \a meter_type meter.  Should be used by derived classes to add their specialized meters to objects

private:
    int                         m_id;
    std::string                 m_name;
    double                      m_x;
    double                      m_y;
    std::set<int>               m_owners;
    int                         m_system_id;
    std::set<std::string>       m_specials;
    std::map<MeterType, Meter>  m_meters;
    int                         m_created_on_turn;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// template implementations
template <class Iter>
UniverseObject* Default0Combiner::operator()(Iter first, Iter last)
{
    UniverseObject* retval = 0;
    while (first != last) {
        assert(!retval); // ensure we retrieve at most one UniverseObject
        retval = *first++;
    }
    return retval;
}

#include "../util/AppInterface.h"

template <class Archive>
void UniverseObject::serialize(Archive& ar, const unsigned int version)
{
    Visibility vis;
    if (Archive::is_saving::value)
        vis = GetVisibility(Universe::s_encoding_empire);
    ar  & BOOST_SERIALIZATION_NVP(vis)
        & BOOST_SERIALIZATION_NVP(m_id)
        & BOOST_SERIALIZATION_NVP(m_x)
        & BOOST_SERIALIZATION_NVP(m_y)
        & BOOST_SERIALIZATION_NVP(m_system_id)
        & BOOST_SERIALIZATION_NVP(m_meters);
    if (Universe::ALL_OBJECTS_VISIBLE ||
        vis == VIS_PARTIAL_VISIBILITY || vis == VIS_FULL_VISIBILITY) {
        std::string name;
        if (Archive::is_saving::value) {
            // We don't disclose the real object name for some types of objects, since it would look funny if e.g. the
            // user saw an incoming enemy cleet called "Decoy".
            name = PublicName(Universe::s_encoding_empire);
        }
        ar  & BOOST_SERIALIZATION_NVP(name)
            & BOOST_SERIALIZATION_NVP(m_owners)
            & BOOST_SERIALIZATION_NVP(m_specials)
            & BOOST_SERIALIZATION_NVP(m_created_on_turn);
        if (Archive::is_loading::value)
            m_name = name;
    }
}

#endif // _UniverseObject_h_

