// -*- C++ -*-
#ifndef _System_h_
#define _System_h_

#ifndef _UniverseObject_h_
#include "UniverseObject.h"
#endif

#ifndef _AppInterface_h_
#include "../util/AppInterface.h"
#endif

#include <boost/type_traits/remove_const.hpp>

#include <map>

struct UniverseObjectVisitor;

/** contains UniverseObjects and connections to other systems (starlanes and wormholes).  All systems are UniversObjects
   contained within the universe, and (nearly) all systems also contain UniverseObjects.  Systems are searchable using
   arbirary predicates, much in the same way that the Client- and ServerUniverses are.  Each System consists of a star
   and 0 or more orbits.  UniversObjects can be placed in the system "at large" or in a particular orbit.  No checking
   is done to determine whether an object is orbit-bound (like a Planet) or not (like a Fleet) when placing them with
   the Insert() functions.  Iteration is available over all starlanes and wormholes (together), all system objects,
   all free system objects (those not in an orbit), and all objects in a paricular orbit.*/
class System : public UniverseObject
{
private:
    typedef std::multimap<int, int>  ObjectMultimap;
    typedef std::map<int, bool>      StarlaneMap;

public:
    typedef std::vector<UniverseObject*>       ObjectVec;          ///< the return type of FindObjects()
    typedef std::vector<const UniverseObject*> ConstObjectVec;     ///< the return type of FindObjects()
    typedef std::vector<int>                   ObjectIDVec;        ///< the return type of FindObjectIDs()

    typedef ObjectMultimap::iterator       orbit_iterator;         ///< iterator for system objects
    typedef ObjectMultimap::const_iterator const_orbit_iterator;   ///< const_iterator for system objects
    typedef StarlaneMap::iterator          lane_iterator;          ///< iterator for starlanes and wormholes
    typedef StarlaneMap::const_iterator    const_lane_iterator;    ///< const_iterator for starlanes and wormholes

    /** \name Signal Types */ //@{
    typedef boost::signal<void (const Fleet&)> FleetSignalType;    ///< emitted when a fleet is inserted or removed from a system

    /** \name Structors */ //@{
    System();    ///< default ctor

    /** general ctor.  \throw std::invalid_arugment May throw std::invalid_arugment if \a star is out of the range
	of StarType, \a orbits is negative, or either x or y coordinate is outside the map area.*/
    System(StarType star, int orbits, const std::string& name, double x, double y,
	   const std::set<int>& owners = std::set<int>());

    /** general ctor.  \throw std::invalid_arugment May throw std::invalid_arugment if \a star is out of the range
	of StarType, \a orbits is negative, or either x or y coordinate is outside the map area.*/
    System(StarType star, int orbits, const StarlaneMap& lanes_and_holes,
	   const std::string& name, double x, double y, const std::set<int>& owners = std::set<int>());

    System(const GG::XMLElement& elem); ///< ctor that constructs a System object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a System object
    ~System();   ///< dtor
    //@}

    /** \name Accessors */ //@{
    StarType Star() const   {return m_star;}  ///< returns the type of star for this system
    int      Orbits() const {return m_orbits;}///< returns the number of orbits in this system

    int  Starlanes() const;                   ///< returns the number of starlanes from this system to other systems
    int  Wormholes() const;                   ///< returns the number of wormholes from this system to other systems
    bool HasStarlaneTo(int id) const;         ///< returns true if there is a starlane from this system to the system with ID number \a id
    bool HasWormholeTo(int id) const;         ///< returns true if there is a wormhole from this system to the system with ID number \a id

    /** returns the IDs of all the objects that match \a visitor */
    ObjectIDVec FindObjectIDs(const UniverseObjectVisitor& visitor) const;

    /** returns the IDs of all the objects of type T. */
    template <class T> ObjectIDVec FindObjectIDs() const;

    /** returns the IDs of all the objects that match \a visitor */
    ObjectIDVec FindObjectIDsInOrbit(int orbit, const UniverseObjectVisitor& visitor) const;

    /** returns the IDs of all the objects of type T in orbit \a orbit. */
    template <class T> ObjectIDVec FindObjectIDsInOrbit(int orbit) const;

    /** returns all the objects that match \a visitor */
    ConstObjectVec FindObjects(const UniverseObjectVisitor& visitor) const;

    /** returns all the objects of type T. */
    template <class T> std::vector<const T*> FindObjects() const;

    /** returns all the objects that match \a visitor in orbit \a orbit */
    ConstObjectVec FindObjectsInOrbit(int orbit, const UniverseObjectVisitor& visitor) const;

    /** returns all the objects of type T in orbit \a orbit. */
    template <class T> std::vector<const T*> FindObjectsInOrbit(int orbit) const;

    const_orbit_iterator begin() const  {return m_objects.begin();}   ///< begin iterator for all system objects
    const_orbit_iterator end() const    {return m_objects.end();}     ///< end iterator for all system objects

    /** returns begin and end iterators for all system objects in orbit \a o*/
    std::pair<const_orbit_iterator, const_orbit_iterator> orbit_range(int o) const {return m_objects.equal_range(o);}

    /** returns begin and end iterators for all system objects not in an orbit*/
    std::pair<const_orbit_iterator, const_orbit_iterator> non_orbit_range() const {return m_objects.equal_range(-1);}

    const_lane_iterator  begin_lanes() const  {return m_starlanes_wormholes.begin();}   ///< begin iterator for all starlanes and wormholes terminating in this system
    const_lane_iterator  end_lanes() const    {return m_starlanes_wormholes.end();}     ///< end iterator for all starlanes and wormholes terminating in this system

    virtual UniverseObject::Visibility GetVisibility(int empire_id) const; ///< returns the visibility status of this universe object relative to the input empire.
    virtual GG::XMLElement XMLEncode(int empire_id = Universe::ALL_EMPIRES) const; ///< constructs an XMLElement from a System object with visibility limited relative to the input empire

    virtual UniverseObject* Accept(const UniverseObjectVisitor& visitor) const;
 
    FleetSignalType& FleetAddedSignal  () const {return m_fleet_added_sig;} ///< returns the fleet added signal object for this System
    FleetSignalType& FleetRemovedSignal() const {return m_fleet_removed_sig;} ///< returns the fleet removed changed signal object for this System
    
    //@}

    /** \name Mutators */ //@{
    /** inserts a UniversObject into the system, though not in any particular orbit.  Only objects free of any
	particular orbit, such as ships, should be inserted using this function.  This function calls obj->SetSystem(this).*/
    int Insert(UniverseObject* obj);

    /** inserts an object into a specific orbit position.  Only orbit-bound objects, such as Planets, and planet-bound
	objects should be inserted with this function.  This function calls obj->SetSystem(this).  \throw
	std::invalid_arugment May throw std::invalid_arugment if \a orbit is out of the range [0, Orbits()].*/
    int Insert(UniverseObject* obj, int orbit);

    /** inserts an object into a specific orbit position.  Only orbit-bound objects, such as Planets, and planet-bound
	objects should be inserted with this function. NOTE: This function is primarily intended for XML decoding purposes
	and does not set the object's system to point to this system since it is assumed that this has already been done prior
	to encoding. If used for other purposes you must set the object's System ID manually. */
    int Insert(int obj_id, int orbit);

    /** removes the object with ID number \a id from the system, and returns it; returns 0 if there is no such object*/
    bool Remove(int id);

    void SetStarType(StarType type); ///< sets the type of the star in this Systems to \a StarType
    void AddStarlane(int id);     ///< adds a starlane between this system and the system with ID number \a id.  \note Adding a starlane to a system to which there is already a wormhole erases the wormhole; you may want to check for a wormhole before calling this function.
    void AddWormhole(int id);     ///< adds a wormhole between this system and the system with ID number \a id  \note Adding a wormhole to a system to which there is already a starlane erases the starlane; you may want to check for a starlane before calling this function.
    bool RemoveStarlane(int id);  ///< removes a starlane between this system and the system with ID number \a id.  Returns false if there was no starlane from this system to system \a id.
    bool RemoveWormhole(int id);  ///< removes a wormhole between this system and the system with ID number \a id.  Returns false if there was no wormhole from this system to system \a id.

    /** returns all the objects that match \a visitor */
    ObjectVec FindObjects(const UniverseObjectVisitor& visitor);

    /** returns all the objects of type T. */
    template <class T> std::vector<T*> FindObjects();

    /** returns all the objects that match \a visitor in orbit \a orbit. */
    ObjectVec FindObjectsInOrbit(int orbit, const UniverseObjectVisitor& visitor);

    /** returns all the objects of type T in orbit \a orbit. */
    template <class T> std::vector<T*> FindObjectsInOrbit(int orbit);

    virtual void AddOwner   (int id);  ///< adding owner to system objects is not allowed
    virtual void RemoveOwner(int id);  ///< removing owner from system objects is not allowed

    virtual void MovementPhase( );
    virtual void PopGrowthProductionResearchPhase( );

    orbit_iterator begin()  {return m_objects.begin();}   ///< begin iterator for all system objects
    orbit_iterator end()    {return m_objects.end();}     ///< end iterator for all system objects

    /** returns begin and end iterators for all system objects in orbit \a o*/
    std::pair<orbit_iterator, orbit_iterator> orbit_range(int o) {return m_objects.equal_range(o);}

    /** returns begin and end iterators for all system objects not in an orbit*/
    std::pair<orbit_iterator, orbit_iterator> non_orbit_range() {return m_objects.equal_range(-1);}

    lane_iterator  begin_lanes()  {return m_starlanes_wormholes.begin();}   ///< begin iterator for all starlanes and wormholes terminating in this system
    lane_iterator  end_lanes()    {return m_starlanes_wormholes.end();}     ///< end iterator for all starlanes and wormholes terminating in this system
    //@}

private:
    ObjectMultimap PartiallyVisibleObjects(int empire_id) const; ///< returns the subset of m_objects that is visible when this System's visibility is only PARTIAL_VISIBILITY

    StarType       m_star;
    int            m_orbits;
    ObjectMultimap m_objects;              ///< each key value represents an orbit (-1 represents general system contents not in any orbit); there may be many or no objects at each orbit (including -1)
    StarlaneMap    m_starlanes_wormholes;  ///< the ints represent the IDs of other connected systems; the bools indicate whether the connection is a wormhole (true) or a starlane (false)

    mutable FleetSignalType m_fleet_added_sig; 
    mutable FleetSignalType m_fleet_removed_sig;
};


inline std::pair<std::string, std::string> SystemRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}


// template implementations

template <class T>
System::ObjectIDVec System::FindObjectIDs() const
{
    const Universe& universe = GetUniverse();
    ObjectIDVec retval;
    for (ObjectMultimap::const_iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (universe.Object(it->second)->Accept(UniverseObjectSubclassVisitor<typename boost::remove_const<T>::type>()))
            retval.push_back(it->second);
    }
    return retval;
}

template <class T>
System::ObjectIDVec System::FindObjectIDsInOrbit(int orbit) const
{
    const Universe& universe = GetUniverse();
    ObjectIDVec retval;
    std::pair<ObjectMultimap::const_iterator, ObjectMultimap::const_iterator> range = m_objects.equal_range(orbit);
    for (ObjectMultimap::const_iterator it = range.first; it != range.second; ++it) {
        if (universe.Object(it->second)->Accept(UniverseObjectSubclassVisitor<typename boost::remove_const<T>::type>()))
            retval.push_back(it->second);
    }
    return retval;
}

template <class T>
std::vector<const T*> System::FindObjects() const
{
    const Universe& universe = GetUniverse();
    std::vector<const T*> retval;
    for (ObjectMultimap::const_iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (const T* obj = static_cast<const T*>(universe.Object(it->second)->Accept(UniverseObjectSubclassVisitor<typename boost::remove_const<T>::type>())))
            retval.push_back(obj);
    }
    return retval;
}

template <class T>
std::vector<const T*> System::FindObjectsInOrbit(int orbit) const
{
    const Universe& universe = GetUniverse();
    std::vector<const T*> retval;
    std::pair<ObjectMultimap::const_iterator, ObjectMultimap::const_iterator> range = m_objects.equal_range(orbit);
    for (ObjectMultimap::const_iterator it = range.first; it != range.second; ++it) {
        if (const T* obj = static_cast<const T*>(universe.Object(it->second)->Accept(UniverseObjectSubclassVisitor<typename boost::remove_const<T>::type>())))
            retval.push_back(obj);
    }
    return retval;
}

template <class T>
std::vector<T*> System::FindObjects()
{
    Universe& universe = GetUniverse();
    std::vector<T*> retval;
    for (ObjectMultimap::iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (T* obj = static_cast<T*>(universe.Object(it->second)->Accept(UniverseObjectSubclassVisitor<typename boost::remove_const<T>::type>())))
            retval.push_back(obj);
    }
    return retval;
}

template <class T>
std::vector<T*> System::FindObjectsInOrbit(int orbit)
{
    Universe& universe = GetUniverse();
    std::vector<T*> retval;
    std::pair<ObjectMultimap::iterator, ObjectMultimap::iterator> range = m_objects.equal_range(orbit);
    for (ObjectMultimap::iterator it = range.first; it != range.second; ++it) {
        if (T* obj = static_cast<T*>(universe.Object(it->second)->Accept(UniverseObjectSubclassVisitor<typename boost::remove_const<T>::type>())))
            retval.push_back(obj);
    }
    return retval;
}


#endif // _System_h_

