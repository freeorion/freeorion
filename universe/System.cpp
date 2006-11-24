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
{
}
   
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
    if (orbit < -1)
        throw std::invalid_argument("System::Insert() : Attempted to place an object in an orbit less than -1");
    obj->SetSystem(ID());
    obj->MoveTo(X(), Y());
    return Insert(obj->ID(), orbit);
}

int System::Insert(int obj_id, int orbit)
{
    if (orbit < -1)
        throw std::invalid_argument("System::Insert() : Attempted to place an object in an orbit less than -1");
    if (!GetUniverse().Object(obj_id))
        throw std::invalid_argument("System::Insert() : Attempted to place an object in a System, when the object is not already in the Universe");
    if (m_orbits <= orbit)
        m_orbits = orbit + 1;
    std::pair<int, int> insertion(orbit, obj_id);
    bool already_in_system = false;
    for (orbit_iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (it->second == obj_id) {
            already_in_system = true;
            break;
        }
    }
    if (!already_in_system) {
        m_objects.insert(insertion);
        if (GetUniverse().Object<Planet>(obj_id))
            UpdateOwnership();
        if (Fleet *fleet = GetUniverse().Object<Fleet>(obj_id))
            FleetAddedSignal(*fleet);
        StateChangedSignal();
    }
    return orbit;
}

bool System::Remove(int id)
{
    bool retval = false;
    for (ObjectMultimap::iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (it->second == id) {
            m_objects.erase(it);
            retval = true;
            if (GetUniverse().Object<Planet>(id))
                UpdateOwnership();
            if (Fleet *fleet = GetUniverse().Object<Fleet>(id))
                FleetRemovedSignal(*fleet);
            StateChangedSignal();
            break;
        }
    }
    return retval;
}

void System::SetStarType(StarType type)
{
    m_star = type;
    if (m_star <= INVALID_STAR_TYPE)
        m_star = STAR_BLUE;
    if (NUM_STAR_TYPES <= m_star)
        m_star = STAR_BLACK;
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
