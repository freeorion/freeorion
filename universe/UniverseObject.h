// -*- C++ -*-
#ifndef _UniverseObject_h_
#define _UniverseObject_h_

#ifndef _Universe_h_
#include "Universe.h"
#endif

#ifndef BOOST_SIGNAL_HPP
#include <boost/signal.hpp>
#endif

#include <set>
#include <string>
#include <vector>

class Meter;
class System;
class SitRepEntry;
namespace GG {class XMLElement;};

/* the various major subclasses of UniverseObject */
enum UniverseObjectType {
    INVALID_UNIVERSE_OBJECT_TYPE,
    OBJ_BUILDING,
    OBJ_SHIP,
    OBJ_FLEET, 
    OBJ_PLANET,
    OBJ_POP_CENTER,
    OBJ_PROD_CENTER,
    OBJ_SYSTEM,
    NUM_OBJ_TYPES
};

namespace GG {
    ENUM_MAP_BEGIN(UniverseObjectType)
	ENUM_MAP_INSERT(INVALID_UNIVERSE_OBJECT_TYPE)
	ENUM_MAP_INSERT(OBJ_BUILDING)
	ENUM_MAP_INSERT(OBJ_SHIP)
	ENUM_MAP_INSERT(OBJ_FLEET)
	ENUM_MAP_INSERT(OBJ_PLANET)
	ENUM_MAP_INSERT(OBJ_POP_CENTER)
	ENUM_MAP_INSERT(OBJ_PROD_CENTER)
	ENUM_MAP_INSERT(OBJ_SYSTEM)
    ENUM_MAP_END
}
ENUM_STREAM_IN(UniverseObjectType)
ENUM_STREAM_OUT(UniverseObjectType)


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
    /** the three different visibility levels */
    enum Visibility { 
        FULL_VISIBILITY,
        PARTIAL_VISIBILITY,
        NO_VISIBILITY
    };

    /** \name Signal Types */ //@{
    typedef boost::signal<void ()> StateChangedSignalType; ///< emitted when the UniverseObject is altered in any way
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
   
    UniverseObject(const GG::XMLElement& elem); ///< ctor that constructs a UniverseObject object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a UniverseObject object
    virtual ~UniverseObject();   ///< dtor
    //@}
   
    /** \name Accessors */ //@{
    int                  ID() const     {return m_id;}       ///< returns the ID number of this object.  Each object in FreeOrion has a unique ID number.
    const std::string&   Name() const   {return m_name;}     ///< returns the name of this object; some valid objects will have no name
    double               X() const      {return m_x;}        ///< the X-coordinate of this object
    double               Y() const      {return m_y;}        ///< the Y-coordinate of this object
    const std::set<int>& Owners() const {return m_owners;}   ///< returns the set of IDs of Empires owning all or part of this object.  \note This may be empty or have an arbitrary number of elements.
    int                  SystemID() const{return m_system_id;}///< returns the ID number of the system in which this object can be found, or INVALID_OBJECT_ID if the object is not within any system
    System*              GetSystem() const;                  ///< returns system in which this object can be found, or null if the object is not within any system

    virtual const Meter* GetMeter(MeterType type) const;  ///< returns the requested Meter, or 0 if no such Meter of that type is found in this object

    bool                 Unowned() const;                 ///< returns true iff there are no owners of this object
    bool                 OwnedBy(int empire) const;       ///< returns true iff the empire with id \a empire is an owner of this object
    bool                 WhollyOwnedBy(int empire) const; ///< returns true iff the empire with id \a empire is the only owner of this object

    virtual Visibility GetVisibility(int empire_id) const; ///< returns the visibility status of this universe object relative to the input empire.
   
    virtual GG::XMLElement XMLEncode(int empire_id = Universe::ALL_EMPIRES) const; ///< constructs an XMLElement from a UniverseObject object with visibility limited relative to the input empire

    StateChangedSignalType& StateChangedSignal() const {return m_changed_sig;} ///< returns the state changed signal object for this UniverseObject
    //@}
   
    /** \name Mutators */ //@{
    void SetID(int id)                   {m_id = id; m_changed_sig();}      ///< sets the ID number of the object to \a id
    void Rename(const std::string& name) {m_name = name; m_changed_sig();}  ///< renames this object to \a name
   
    /** moves the object using the vector (x, y). \throw std::runtime_error May throw std::runtime_error if the result 
        of the move would place either coordinate outside the map area.*/
    void Move(double x, double y);
   
    /** moves the object to coordinates (x, y). \throw std::invalid_arugment May throw std::invalid_arugment if the 
        either coordinate of the move is outside the map area.*/
    void MoveTo(double x, double y);
   
    virtual Meter* GetMeter(MeterType type);  ///< returns the requested Meter, or 0 if no such Meter of that type is found in this object

    virtual void AddOwner(int id);  ///< adds the Empire with ID \a id to the list of owners of this object
    virtual void RemoveOwner(int id);   ///< removes the Empire with ID \a id to the list of owners of this object
    void SetSystem(int sys)  {m_system_id = sys; m_changed_sig();}    ///< assigns this object to a System
   
    /** performs the movement that this object is responsible for this object's actions during the movement phase of 
        a turn.  Called by ServerUniverse::MovementPhase().*/
    virtual void MovementPhase( ) = 0;
   
    /** performs the movement that this object is responsible for this object's actions during the pop growth/production/
        research phase of a turn.  Called by ServerUniverse::PopGrowthProductionResearchPhase().*/
    virtual void PopGrowthProductionResearchPhase( ) = 0;

    //@}
   
    static const double INVALID_POSITION;  ///< the position in x and y at which default-constructed objects are placed
    static const int    INVALID_OBJECT_ID; ///< the ID number assigned to a UniverseObject upon construction; it is assigned an ID later when it is placed in the universe
    static const int    MAX_ID; ///< the max ID number 

private:
    int            m_id;
    std::string    m_name;
    double         m_x;
    double         m_y;
    std::set<int>  m_owners;
    int            m_system_id;

    mutable StateChangedSignalType m_changed_sig;
};

#endif // _UniverseObject_h_

