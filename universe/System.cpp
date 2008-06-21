#include "System.h"

#include "Fleet.h"
#include "Planet.h"

#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"

#include "../util/MultiplayerCommon.h"
#include "Predicates.h"

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

#include <stdexcept>



System::System() :
   UniverseObject(),
   m_star(INVALID_STAR_TYPE),
   m_orbits(0)
{}

System::System(StarType star, int orbits, const std::string& name, double x, double y, 
               const std::set<int>& owners/* = std::set<int>()*/) : 
   UniverseObject(name, x, y, owners),
   m_star(star),
   m_orbits(orbits)
{
   if (m_star < INVALID_STAR_TYPE || NUM_STAR_TYPES < m_star)
      throw std::invalid_argument("System::System : Attempted to create a system \"" + Name() + "\" with an invalid star type.");
   if (m_orbits < 0)
      throw std::invalid_argument("System::System : Attempted to create a system \"" + Name() + "\" with fewer than 0 orbits.");
}

System::System(StarType star, int orbits, const StarlaneMap& lanes_and_holes, 
               const std::string& name, double x, double y, const std::set<int>& owners/* = std::set<int>()*/) : 
   UniverseObject(name, x, y, owners),
   m_star(star),
   m_orbits(orbits),
   m_starlanes_wormholes(lanes_and_holes)
{
   if (m_star < INVALID_STAR_TYPE || NUM_STAR_TYPES < m_star)
      throw std::invalid_argument("System::System : Attempted to create a system \"" + Name() + "\" with an invalid star type.");
   if (m_orbits < 0)
      throw std::invalid_argument("System::System : Attempted to create a system \"" + Name() + "\" with fewer than 0 orbits.");
}

System::~System()
{}

StarType System::Star() const
{
    return m_star;
}

int System::Orbits() const
{
    return m_orbits;
}

int System::Starlanes() const
{
    int retval = 0;
    for (const_lane_iterator it = begin_lanes(); it != end_lanes(); ++it) {
        if (!it->second)
            ++retval;
    }
    return retval;
}

int System::Wormholes() const
{
    int retval = 0;
    for (const_lane_iterator it = begin_lanes(); it != end_lanes(); ++it) {
        if (it->second)
            ++retval;
    }
    return retval;
}

bool System::HasStarlaneTo(int id) const
{
   const_lane_iterator it = m_starlanes_wormholes.find(id);
   return (it == m_starlanes_wormholes.end() ? false : it->second == false);
}

bool System::HasWormholeTo(int id) const
{
   const_lane_iterator it = m_starlanes_wormholes.find(id);
   return (it == m_starlanes_wormholes.end() ? false : it->second == true);
}

std::vector<UniverseObject*> System::FindObjects() const
{
    Universe& universe = GetUniverse();
    std::vector<UniverseObject*> retval;
    // add objects contained in this system, and objects contained in those objects
    for (ObjectMultimap::const_iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        UniverseObject* object = universe.Object(it->second);
        retval.push_back(object);
        std::vector<UniverseObject*> contained_objects = object->FindObjects();
        std::copy(contained_objects.begin(), contained_objects.end(), std::back_inserter(retval));
    }
    return retval;
}

std::vector<int> System::FindObjectIDs() const
{
    Universe& universe = GetUniverse();
    std::vector<int> retval;
    // add objects contained in this system, and objects contained in those objects
    for (ObjectMultimap::const_iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        UniverseObject* object = universe.Object(it->second);
        retval.push_back(it->second);
        std::vector<int> contained_objects = object->FindObjectIDs();
        std::copy(contained_objects.begin(), contained_objects.end(), std::back_inserter(retval));
    }
    return retval;
}

System::ObjectIDVec System::FindObjectIDs(const UniverseObjectVisitor& visitor) const
{
    const Universe& universe = GetUniverse();
    ObjectIDVec retval;
    for (ObjectMultimap::const_iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (universe.Object(it->second)->Accept(visitor))
            retval.push_back(it->second);
    }
    return retval;
}

System::ObjectIDVec System::FindObjectIDsInOrbit(int orbit, const UniverseObjectVisitor& visitor) const
{
    const Universe& universe = GetUniverse();
    ObjectIDVec retval;
    std::pair<ObjectMultimap::const_iterator, ObjectMultimap::const_iterator> range = m_objects.equal_range(orbit);
    for (ObjectMultimap::const_iterator it = range.first; it != range.second; ++it) {
        if (universe.Object(it->second)->Accept(visitor))
            retval.push_back(it->second);
    }
    return retval;
}

System::ConstObjectVec System::FindObjects(const UniverseObjectVisitor& visitor) const
{
    const Universe& universe = GetUniverse();
    ConstObjectVec retval;
    for (ObjectMultimap::const_iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (const UniverseObject* obj = universe.Object(it->second)->Accept(visitor))
            retval.push_back(obj);
    }
    return retval;
}

System::const_orbit_iterator System::begin() const
{
    return m_objects.begin();
}

System::const_orbit_iterator System::end() const
{
    return m_objects.end();
}

bool System::Contains(int object_id) const
{
    const UniverseObject* object = GetUniverse().Object(object_id);
    if (object)
        return (ID() == object->SystemID());
    else
        return false;
}

std::pair<System::const_orbit_iterator, System::const_orbit_iterator> System::orbit_range(int o) const
{
    return m_objects.equal_range(o);
}

std::pair<System::const_orbit_iterator, System::const_orbit_iterator> System::non_orbit_range() const
{
    return m_objects.equal_range(-1);
}

System::const_lane_iterator System::begin_lanes() const
{
    return m_starlanes_wormholes.begin();
}

System::const_lane_iterator System::end_lanes() const
{
    return m_starlanes_wormholes.end();
}

System::ConstObjectVec System::FindObjectsInOrbit(int orbit, const UniverseObjectVisitor& visitor) const
{
    const Universe& universe = GetUniverse();
    ConstObjectVec retval;
    std::pair<ObjectMultimap::const_iterator, ObjectMultimap::const_iterator> range = m_objects.equal_range(orbit);
    for (ObjectMultimap::const_iterator it = range.first; it != range.second; ++it) {
        if (const UniverseObject* obj = universe.Object(it->second)->Accept(visitor))
            retval.push_back(obj);
    }
    return retval;
}

UniverseObject::Visibility System::GetVisibility(int empire_id) const
{
    // if system is at least partially owned by this empire it is fully visible, if it has been explored it is partially visible, 
    // and otherwise it will be partially visible
    Empire* empire = 0;
    if (Universe::ALL_OBJECTS_VISIBLE || empire_id == ALL_EMPIRES || OwnedBy(empire_id))
        return FULL_VISIBILITY;
    else if ((empire = Empires().Lookup(empire_id)) && empire->HasExploredSystem(ID()))
        return PARTIAL_VISIBILITY;
    else
        return NO_VISIBILITY;
}

UniverseObject* System::Accept(const UniverseObjectVisitor& visitor) const
{
    return visitor.Visit(const_cast<System* const>(this));
}

int System::Insert(UniverseObject* obj)
{
    return Insert(obj, -1);
}

int System::Insert(UniverseObject* obj, int orbit)
{
    if (!obj)
        throw std::invalid_argument("System::Insert() : Attempted to place a null object in a System");

    //Logger().debugStream() << "System::Insert(" << obj->Name() <<", " << orbit << ")";
    //Logger().debugStream() << "..initial objects in system: ";
    //for (ObjectMultimap::iterator it = m_objects.begin(); it != m_objects.end(); ++it)
    //    Logger().debugStream() << "...." << GetUniverse().Object(it->second)->Name();
    //Logger().debugStream() << "..initial object systemid: " << obj->SystemID();

    int obj_id = obj->ID();

    if (orbit < -1)
        throw std::invalid_argument("System::Insert() : Attempted to place an object in an orbit less than -1");


    // ensure object and its contents are all inserted
    std::vector<UniverseObject*> contained_objects = obj->FindObjects();
    for (std::vector<UniverseObject*>::iterator it = contained_objects.begin(); it != contained_objects.end(); ++it)
        this->Insert(*it, orbit);


    obj->MoveTo(this);          // ensure object is physically at same location in universe as this system
    obj->SetSystem(this->ID()); // should do nothing if object is already in this system, but just to ensure things are consistent...

    // ensure object isn't already in this system (as far as the system is concerned).  If the system already
    // thinks the object is here, just return its current orbit and don't change anything.
    for (orbit_iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (it->second == obj_id) {
            return it->first;
        }
    }

    // ensure this system has enough orbits to contain object
    if (m_orbits <= orbit)
        m_orbits = orbit + 1;

    // record object in this system's record of objects in each orbit
    std::pair<int, int> insertion(orbit, obj_id);
    m_objects.insert(insertion);


    // special cases for if object is a planet or fleet
    if (universe_object_cast<Planet*>(obj))
        UpdateOwnership();
    else if (Fleet* fleet = universe_object_cast<Fleet*>(obj))
        FleetInsertedSignal(*fleet);

    StateChangedSignal();

    return orbit;
}

int System::Insert(int obj_id, int orbit)
{
    if (orbit < -1)
        throw std::invalid_argument("System::Insert() : Attempted to place an object in an orbit less than -1");
    UniverseObject* object = GetUniverse().Object(obj_id);
    if (!object)
        throw std::invalid_argument("System::Insert() : Attempted to place an object in a System, when the object is not already in the Universe");

    return Insert(object, orbit);
}

void System::Remove(UniverseObject* obj)
{
    //Logger().debugStream() << "System::Remove( " << obj->Name() << " )";
    //Logger().debugStream() << "..objects in system: ";
    //for (ObjectMultimap::iterator it = m_objects.begin(); it != m_objects.end(); ++it)
    //    Logger().debugStream() << ".... " << GetUniverse().Object(it->second)->Name();

    // ensure object and its contents are all removed from system
    std::vector<UniverseObject*> contained_objects = obj->FindObjects();
    for (std::vector<UniverseObject*>::iterator it = contained_objects.begin(); it != contained_objects.end(); ++it)
        this->Remove(*it);


    if (obj->SystemID() != this->ID())
        Logger().debugStream() << "System::Remove tried to remove an object whose system id was not this system.  Its current system id is: " << obj->SystemID();


    // locate object in this system's map of objects, and erase if present
    for (ObjectMultimap::iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        //Logger().debugStream() << "..system object id " << it->second;
        if (it->second == obj->ID()) {
            m_objects.erase(it);

            obj->SetSystem(INVALID_OBJECT_ID);

            //Logger().debugStream() << "....erased from system";

            // UI bookeeping
            if (universe_object_cast<Planet*>(obj))
                UpdateOwnership();
            else if (Fleet* fleet = universe_object_cast<Fleet*>(obj))
                FleetRemovedSignal(*fleet);

            StateChangedSignal();

            return; // assuming object isn't in system twice...
        }
    }


    // didn't find object.  this doesn't necessarily indicate a problem, as removing objects from
    // systems may be done redundantly when inserting or moving objects that contain other objects
    Logger().debugStream() << "System::Remove didn't find object in system";
}

void System::Remove(int id)
{
    Remove(GetUniverse().Object(id));
}

void System::SetStarType(StarType type)
{
    m_star = type;
    if (m_star <= INVALID_STAR_TYPE || NUM_STAR_TYPES <= m_star)
        Logger().errorStream() << "System::SetStarType set star type to " << boost::lexical_cast<std::string>(type);
    StateChangedSignal();
}

void System::AddStarlane(int id)
{
   m_starlanes_wormholes[id] = false;
   StateChangedSignal();
}

void System::AddWormhole(int id)
{
   m_starlanes_wormholes[id] = true;
   StateChangedSignal();
}

bool System::RemoveStarlane(int id)
{
   bool retval = false;
   if (retval = HasStarlaneTo(id)) {
      m_starlanes_wormholes.erase(id);
      StateChangedSignal();
   }
   return retval;
}

bool System::RemoveWormhole(int id)
{
   bool retval = false;
   if (retval = HasWormholeTo(id)) {
      m_starlanes_wormholes.erase(id);
      StateChangedSignal();
   }
   return retval;
}

System::ObjectVec System::FindObjects(const UniverseObjectVisitor& visitor)
{
    Universe& universe = GetUniverse();
    ObjectVec retval;
    for (ObjectMultimap::iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (UniverseObject* obj = universe.Object(it->second)->Accept(visitor))
            retval.push_back(obj);
    }
    return retval;
}

System::ObjectVec System::FindObjectsInOrbit(int orbit, const UniverseObjectVisitor& visitor)
{
    Universe& universe = GetUniverse();
    ObjectVec retval;
    std::pair<ObjectMultimap::iterator, ObjectMultimap::iterator> range = m_objects.equal_range(orbit);
    for (ObjectMultimap::iterator it = range.first; it != range.second; ++it) {
        if (UniverseObject* obj = universe.Object(it->second)->Accept(visitor))
            retval.push_back(obj);
    }
    return retval;
}

void System::AddOwner(int id)
{
}

void System::RemoveOwner(int id)
{
}

void System::MovementPhase()
{
}

void System::PopGrowthProductionResearchPhase()
{
}

System::orbit_iterator System::begin()
{
    return m_objects.begin();
}

System::orbit_iterator System::end()
{
    return m_objects.end();
}

std::pair<System::orbit_iterator, System::orbit_iterator> System::orbit_range(int o)
{
    return m_objects.equal_range(o);
}

std::pair<System::orbit_iterator, System::orbit_iterator> System::non_orbit_range()
{
    return m_objects.equal_range(-1);
}

System::lane_iterator System::begin_lanes()
{
    return m_starlanes_wormholes.begin();
}

System::lane_iterator System::end_lanes()
{
    return m_starlanes_wormholes.end();
}

System::StarlaneMap System::VisibleStarlanes(int empire_id) const
{
    const Empire* empire = Empires().Lookup(empire_id);
    if (empire->HasExploredSystem(ID()))
        return m_starlanes_wormholes;
    StarlaneMap retval;
    for (StarlaneMap::const_iterator it = m_starlanes_wormholes.begin(); it != m_starlanes_wormholes.end(); ++it) {
        if (empire->HasExploredSystem(it->first))
            retval.insert(*it);
    }
    return retval;
}

System::ObjectMultimap System::PartiallyVisibleObjects(int empire_id) const
{
    ObjectMultimap retval;
    const Universe& universe = GetUniverse();
    for (ObjectMultimap::const_iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (universe.Object(it->second)->GetVisibility(empire_id) <= PARTIAL_VISIBILITY)
            retval.insert(*it);
    }
    return retval;
}

void System::UpdateOwnership()
{
    std::set<int> owners = Owners();
    for (std::set<int>::iterator it = owners.begin(); it != owners.end(); ++it) {
        UniverseObject::RemoveOwner(*it);
    }
    for (ObjectMultimap::iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (Planet *planet = GetUniverse().Object<Planet>(it->second)) {
            for (std::set<int>::const_iterator it2 = planet->Owners().begin(); it2 != planet->Owners().end(); ++it2) {
                UniverseObject::AddOwner(*it2);
            }
        }
    }
}
