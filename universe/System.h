// -*- C++ -*-
#ifndef _System_h_
#define _System_h_

#ifndef _UniverseObject_h_
#include "UniverseObject.h"
#endif

#include <boost/type_traits/remove_const.hpp>

#include <map>

struct UniverseObjectVisitor;

/** contains UniverseObjects and connections to other systems (starlanes and
   wormholes).  All systems are UniversObjects contained within the universe,
   and (nearly) all systems also contain UniverseObjects.  Systems are
   searchable using arbirary predicates, much in the same way that Universes
   are.  Each System consists of a star and 0 or more orbits.  UniversObjects
   can be placed in the system "at large" or in a particular orbit.  No
   checking is done to determine whether an object is orbit-bound (like a
   Planet) or not (like a Fleet) when placing them with the Insert()
   functions.  Iteration is available over all starlanes and wormholes
   (together), all system objects, all free system objects (those not in an
   orbit), and all objects in a paricular orbit.*/
class System : public UniverseObject
{
private:
    typedef std::multimap<int, int>             ObjectMultimap;

public:
    typedef std::vector<UniverseObject*>        ObjectVec;              ///< the return type of FindObjects()
    typedef std::vector<const UniverseObject*>  ConstObjectVec;         ///< the return type of FindObjects()
    typedef std::vector<int>                    ObjectIDVec;            ///< the return type of FindObjectIDs()
    typedef std::map<int, bool>                 StarlaneMap;            ///< the return type of VisibleStarlanes()

    typedef ObjectMultimap::iterator            orbit_iterator;         ///< iterator for system objects
    typedef ObjectMultimap::const_iterator      const_orbit_iterator;   ///< const_iterator for system objects
    typedef StarlaneMap::iterator               lane_iterator;          ///< iterator for starlanes and wormholes
    typedef StarlaneMap::const_iterator         const_lane_iterator;    ///< const_iterator for starlanes and wormholes

    /** \name Structors */ //@{
    System();                                                                   ///< default ctor

    /** general ctor.  \throw std::invalid_arugment May throw std::invalid_arugment if \a star is out of the range
        of StarType, \a orbits is negative, or either x or y coordinate is outside the map area.*/
    System(StarType star, int orbits, const std::string& name, double x, double y,
           const std::set<int>& owners = std::set<int>());

    /** general ctor.  \throw std::invalid_arugment May throw std::invalid_arugment if \a star is out of the range
        of StarType, \a orbits is negative, or either x or y coordinate is outside the map area.*/
    System(StarType star, int orbits, const StarlaneMap& lanes_and_holes,
           const std::string& name, double x, double y, const std::set<int>& owners = std::set<int>());

    System(const System& rhs);                                                  ///< copy ctor

    virtual System*         Clone(int empire_id = ALL_EMPIRES) const;  ///< returns new copy of this System
    //@}

    /** \name Accessors */ //@{
    StarType                GetStarType() const;            ///< returns the type of star for this system
    int                     Orbits() const;                 ///< returns the number of orbits in this system

    int                     Starlanes() const;              ///< returns the number of starlanes from this system to other systems
    int                     Wormholes() const;              ///< returns the number of wormholes from this system to other systems
    bool                    HasStarlaneTo(int id) const;    ///< returns true if there is a starlane from this system to the system with ID number \a id
    bool                    HasWormholeTo(int id) const;    ///< returns true if there is a wormhole from this system to the system with ID number \a id

    virtual int             SystemID() const;               ///< returns this->ID()

    virtual std::vector<UniverseObject*>
                            FindObjects() const;                ///< returns objects contained within this system
    virtual std::vector<int>
                            FindObjectIDs() const;              ///< returns ids of objects contained within this system

    ObjectIDVec             FindObjectIDs(const UniverseObjectVisitor& visitor) const;  ///< returns the IDs of all the objects that match \a visitor

    template <class T>
    ObjectIDVec             FindObjectIDs() const;                                      ///< returns the IDs of all the objects of type T

    ObjectIDVec             FindObjectIDsInOrbit(int orbit, const UniverseObjectVisitor& visitor) const;    ///< returns the IDs of all the objects that match \a visitor

    template <class T>
    ObjectIDVec             FindObjectIDsInOrbit(int orbit) const;                      ///< returns the IDs of all the objects of type T in orbit \a orbit

    ConstObjectVec          FindObjects(const UniverseObjectVisitor& visitor) const;    ///< returns all the objects that match \a visitor

    template <class T>
    std::vector<const T*>   FindObjects() const;                                        ///< returns all the objects of type T

    ConstObjectVec          FindObjectsInOrbit(int orbit, const UniverseObjectVisitor& visitor) const;      ///< returns all the objects that match \a visitor in orbit \a orbit

    template <class T>
    std::vector<const T*>   FindObjectsInOrbit(int orbit) const;                        ///< returns all the objects of type T in orbit \a orbit

    virtual bool            Contains(int object_id) const;  ///< returns true if object with id \a object_id is in this system

    const_orbit_iterator    begin() const;                  ///< begin iterator for all system objects
    const_orbit_iterator    end() const;                    ///< end iterator for all system objects

    std::pair<const_orbit_iterator, const_orbit_iterator>
                            orbit_range(int o) const;       ///< returns begin and end iterators for all system objects in orbit
    std::pair<const_orbit_iterator, const_orbit_iterator>
                            non_orbit_range() const;        ///< returns begin and end iterators for all system objects not in an orbit

    bool                    OrbitOccupied(int orbit) const; ///< returns true if there is an object in \a orbit
    std::set<int>           FreeOrbits() const;             ///< returns the set of orbit numbers that are unoccupied

    const_lane_iterator     begin_lanes() const;            ///< begin iterator for all starlanes and wormholes terminating in this system
    const_lane_iterator     end_lanes() const;              ///< end iterator for all starlanes and wormholes terminating in this system

    /** returns a map of the starlanes and wormholes visible to empire
      * \a empire_id; the map contains keys that are IDs of connected systems,
      * and bool values indicating whether each is a starlane (false) or a
      * wormhole (true)*/
    StarlaneMap             VisibleStarlanes(int empire_id) const;

    virtual UniverseObject* Accept(const UniverseObjectVisitor& visitor) const;

    mutable boost::signal<void (Fleet& fleet)> FleetInsertedSignal; ///< fleet is inserted into system
    mutable boost::signal<void (Fleet& fleet)> FleetRemovedSignal;  ///< fleet is removed from system
    //@}

    /** \name Mutators */ //@{
    virtual void            Copy(const UniverseObject* copied_object, int empire_id = ALL_EMPIRES);

    /** inserts a UniversObject into the system, though not in any particular orbit.  Only objects free of any
        particular orbit, such as ships, should be inserted using this function.  This function calls obj->SetSystem(this),
        and obj->MoveTo( this system's position )*/
    int                     Insert(UniverseObject* obj);

    /** inserts an object into a specific orbit position.  Only orbit-bound objects, such as Planets, and planet-bound
        objects should be inserted with this function.  This function calls obj->SetSystem(this) and obj->MoveTo( here )
        \throw std::invalid_arugment May throw std::invalid_arugment if \a orbit is out of the range [0, Orbits()].*/
    int                     Insert(UniverseObject* obj, int orbit);

    /** inserts an object into a specific orbit position.  Only orbit-bound objects, such as Planets, and planet-bound
        objects should be inserted with this function. */
    int                     Insert(int obj_id, int orbit);

    /** removes object \a obj from this system. */
    void                    Remove(UniverseObject* obj);

    /** removes the object with ID number \a id from this system. */
    void                    Remove(int id);

    void                    SetStarType(StarType type);     ///< sets the type of the star in this Systems to \a StarType
    void                    AddStarlane(int id);            ///< adds a starlane between this system and the system with ID number \a id.  \note Adding a starlane to a system to which there is already a wormhole erases the wormhole; you may want to check for a wormhole before calling this function.
    void                    AddWormhole(int id);            ///< adds a wormhole between this system and the system with ID number \a id  \note Adding a wormhole to a system to which there is already a starlane erases the starlane; you may want to check for a starlane before calling this function.
    bool                    RemoveStarlane(int id);         ///< removes a starlane between this system and the system with ID number \a id.  Returns false if there was no starlane from this system to system \a id.
    bool                    RemoveWormhole(int id);         ///< removes a wormhole between this system and the system with ID number \a id.  Returns false if there was no wormhole from this system to system \a id.

    ObjectVec               FindObjects(const UniverseObjectVisitor& visitor);                      ///< returns all the objects that match \a visitor

    template <class T> std::vector<T*>
                            FindObjects();                                                          ///< returns all the objects of type T

    ObjectVec               FindObjectsInOrbit(int orbit, const UniverseObjectVisitor& visitor);    ///< returns all the objects that match \a visitor in orbit \a orbit

    template <class T> std::vector<T*>
                            FindObjectsInOrbit(int orbit);                                          ///< returns all the objects of type T in orbit \a orbit

    virtual void            AddOwner(int id);               ///< adding owner to system objects is a no-op
    virtual void            RemoveOwner(int id);            ///< removing owner from system objects is a no-op

    virtual void            ApplyUniverseTableMaxMeterAdjustments(MeterType meter_type = INVALID_METER_TYPE);

    orbit_iterator          begin();                        ///< begin iterator for all system objects
    orbit_iterator          end();                          ///< end iterator for all system objects

    std::pair<orbit_iterator, orbit_iterator>
                            orbit_range(int o);             ///< returns begin and end iterators for all system objects in orbit \a o

    std::pair<orbit_iterator, orbit_iterator>
                            non_orbit_range();              ///< returns begin and end iterators for all system objects not in an orbit

    lane_iterator           begin_lanes();                  ///< begin iterator for all starlanes and wormholes terminating in this system
    lane_iterator           end_lanes();                    ///< end iterator for all starlanes and wormholes terminating in this system
    //@}

private:
    /** returns the subset of m_objects that is visible to empire with id
      * \a empire_id */
    ObjectMultimap          VisibleContainedObjects(int empire_id) const;

    void                    UpdateOwnership();

    StarType        m_star;
    int             m_orbits;
    ObjectMultimap  m_objects;              ///< each key value represents an orbit (-1 represents general system contents not in any orbit); there may be many or no objects at each orbit (including -1)
    StarlaneMap     m_starlanes_wormholes;  ///< the ints represent the IDs of other connected systems; the bools indicate whether the connection is a wormhole (true) or a starlane (false)

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


// Tactical combat system geometry free functions:

/** Returns the radius, in tactical combat units, of a system.  Note that the
    tactical combat map is square. */
double SystemRadius();

/** Returns the radius, in tactical combat units, of the star at the center of a system. */
double StarRadius();

/** Returns the radius, in tactical combat units, of orbit \a orbit of a
    system.  \a orbit must be < 10. */
double OrbitalRadius(unsigned int orbit);

/** Returns the orbital radius, in tactical combat units, of starlane entrance
    points out of a system.  */
double StarlaneEntranceOrbitalRadius();

/** Returns the angular position, in radians, of a starlane entrance point out
    of the system with id \a from_system.  */
double StarlaneEntranceOrbitalPosition(int from_system, int to_system);

/** Returns the radius, in tactical combat units, of starlane entrance points
    out of a system.  */
double StarlaneEntranceRadius();


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

template <class Archive>
void System::serialize(Archive& ar, const unsigned int version)
{
    Visibility vis;
    int orbits = 0;
    ObjectMultimap objects;
    StarlaneMap starlanes_wormholes;

    if (Archive::is_saving::value) {
        vis = GetVisibility(Universe::s_encoding_empire);
        if (vis == VIS_FULL_VISIBILITY)
            orbits = m_orbits;

        objects =               VisibleContainedObjects(Universe::s_encoding_empire);
        starlanes_wormholes =   VisibleStarlanes(       Universe::s_encoding_empire);
    }

    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(UniverseObject)
        & BOOST_SERIALIZATION_NVP(vis)
        & BOOST_SERIALIZATION_NVP(m_star)
        & BOOST_SERIALIZATION_NVP(orbits)
        & BOOST_SERIALIZATION_NVP(objects)
        & BOOST_SERIALIZATION_NVP(starlanes_wormholes);

    if (Archive::is_loading::value) {
        m_orbits = orbits;
        m_objects = objects;
        m_starlanes_wormholes = starlanes_wormholes;
        //// DEBUG
        //Logger().debugStream() << "system " << this->Name() << " deserialized objects ids:";
        //for (ObjectMultimap::const_iterator it = m_objects.begin(); it != m_objects.end(); ++it)
        //    Logger().debugStream() << ".... " << it->second;
        //// END DEBUG
    }
}

#endif // _System_h_
