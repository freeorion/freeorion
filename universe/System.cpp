#include "System.h"

#include "Fleet.h"
#include "Planet.h"
#include "Predicates.h"

#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"

#include "../util/Math.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"

#include <boost/lexical_cast.hpp>
#include <stdexcept>

System::System() :
    UniverseObject(),
    m_star(INVALID_STAR_TYPE),
    m_orbits(0),
    m_last_turn_battle_here(INVALID_GAME_TURN)
{
    //Logger().debugStream() << "System::System()";
}

System::System(StarType star, int orbits, const std::string& name, double x, double y,
               const std::set<int>& owners/* = std::set<int>()*/) :
    UniverseObject(name, x, y, owners),
    m_star(star),
    m_orbits(orbits),
    m_last_turn_battle_here(INVALID_GAME_TURN)
{
    //Logger().debugStream() << "System::System(" << star << ", " << orbits << ", " << name << ", " << x << ", " << y << ")";
    if (m_star < INVALID_STAR_TYPE || NUM_STAR_TYPES < m_star)
        throw std::invalid_argument("System::System : Attempted to create a system \"" + Name() + "\" with an invalid star type.");
    if (m_orbits < 0)
        throw std::invalid_argument("System::System : Attempted to create a system \"" + Name() + "\" with fewer than 0 orbits.");

    UniverseObject::Init();
}

System::System(StarType star, int orbits, const StarlaneMap& lanes_and_holes,
               const std::string& name, double x, double y, const std::set<int>& owners/* = std::set<int>()*/) :
    UniverseObject(name, x, y, owners),
    m_star(star),
    m_orbits(orbits),
    m_starlanes_wormholes(lanes_and_holes),
    m_last_turn_battle_here(INVALID_GAME_TURN)
{
    //Logger().debugStream() << "System::System(" << star << ", " << orbits << ", (StarlaneMap), " << name << ", " << x << ", " << y << ")";
    if (m_star < INVALID_STAR_TYPE || NUM_STAR_TYPES < m_star)
        throw std::invalid_argument("System::System : Attempted to create a system \"" + Name() + "\" with an invalid star type.");
    if (m_orbits < 0)
        throw std::invalid_argument("System::System : Attempted to create a system \"" + Name() + "\" with fewer than 0 orbits.");
    SetSystem(ID());

    UniverseObject::Init();
}

System::System(const System& rhs) :
    m_star(rhs.m_star),
    m_orbits(rhs.m_orbits),
    m_objects(rhs.m_objects),
    m_starlanes_wormholes(rhs.m_starlanes_wormholes),
    m_last_turn_battle_here(rhs.m_last_turn_battle_here)
{}

System* System::Clone(int empire_id) const
{
    Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(this->ID(), empire_id);

    if (!(vis >= VIS_BASIC_VISIBILITY && vis <= VIS_FULL_VISIBILITY))
        return 0;

    System* retval = new System();
    retval->Copy(this, empire_id);
    return retval;
}

void System::Copy(const UniverseObject* copied_object, int empire_id)
{
    if (copied_object == this)
        return;
    const System* copied_system = universe_object_cast<System*>(copied_object);
    if (!copied_system) {
        Logger().errorStream() << "System::Copy passed an object that wasn't a System";
        return;
    }

    int copied_object_id = copied_object->ID();
    Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(copied_object_id, empire_id);

    UniverseObject::Copy(copied_object, vis);

    if (vis >= VIS_BASIC_VISIBILITY) {
        this->m_objects =                   copied_system->VisibleContainedObjects(empire_id);

        // add any visible lanes, without removing existing entries
        StarlaneMap visible_lanes_holes = copied_system->VisibleStarlanesWormholes(empire_id);
        for (StarlaneMap::const_iterator it = visible_lanes_holes.begin(); it != visible_lanes_holes.end(); ++it)
            this->m_starlanes_wormholes[it->first] = it->second;

        this->m_orbits =                    copied_system->m_orbits;

        if (vis >= VIS_PARTIAL_VISIBILITY) {
            this->m_star =                  copied_system->m_star;
            this->m_last_turn_battle_here = copied_system->m_last_turn_battle_here;

            // remove any not-visible lanes that were previously known: with
            // partial vis, they should be seen, but aren't, so are known not
            // to exist any more

            // assemble all previously known lanes
            std::vector<int> initial_known_lanes;
            for (StarlaneMap::const_iterator it = this->m_starlanes_wormholes.begin(); it != this->m_starlanes_wormholes.end(); ++it)
                initial_known_lanes.push_back(it->first);

            // remove previously known lanes that aren't currently visible
            for (std::vector<int>::const_iterator it = initial_known_lanes.begin(); it != initial_known_lanes.end(); ++it) {
                int lane_end_sys_id = *it;
                if (visible_lanes_holes.find(lane_end_sys_id) == visible_lanes_holes.end())
                    this->m_starlanes_wormholes.erase(lane_end_sys_id);
            }

            if (vis < VIS_FULL_VISIBILITY) {
                // copy system name if at partial visibility, as it won't be copied
                // by UniverseObject::Copy unless at full visibility, but players
                // should know system names even if they don't own the system
                Universe::InhibitUniverseObjectSignals(true);
                this->Rename(copied_system->Name());
                Universe::InhibitUniverseObjectSignals(false);
            }
        }
    }
}

const std::string& System::TypeName() const
{ return UserString("SYSTEM"); }

std::string System::Dump() const
{
    std::stringstream os;
    os << UniverseObject::Dump();
    os << " star type: " << UserString(GG::GetEnumMap<StarType>().FromEnum(m_star))
       << "  last combat on turn: " << m_last_turn_battle_here
       << "  starlanes: ";
    //typedef std::map<int, bool> StarlaneMap;
    for (StarlaneMap::const_iterator it = m_starlanes_wormholes.begin(); it != m_starlanes_wormholes.end();) {
        int lane_end_id = it->first;
        ++it;
        os << lane_end_id << (it == m_starlanes_wormholes.end() ? "" : ", ");
    }
    os << "  objects: ";
    //typedef std::multimap<int, int>             ObjectMultimap;         ///< each key value represents an orbit (-1 represents general system contents not in any orbit); there may be many or no objects at each orbit (including -1)
    for (ObjectMultimap::const_iterator it = m_objects.begin(); it != m_objects.end();) {
        int obj_id = it->second;
        ++it;
        if (obj_id == UniverseObject::INVALID_OBJECT_ID)
            continue;
        os << obj_id << (it == m_objects.end() ? "" : ", ");
    }
    return os.str();
}

StarType System::GetStarType() const
{ return m_star; }

int System::Orbits() const
{ return m_orbits; }

const std::set<int>& System::ControllingEmpireIDs() const
{ return m_empires_with_planets_here; }

int System::NumStarlanes() const
{
    int retval = 0;
    for (const_lane_iterator it = begin_lanes(); it != end_lanes(); ++it) {
        if (!it->second)
            ++retval;
    }
    return retval;
}

int System::NumWormholes() const
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

int System::SystemID() const
{
    // Systems don't have a valid UniverseObject::m_system_id since it is never set by inserting them into (another) system.
    // When new systems are created, it's also not set because UniverseObject are created without a valid id number, and only
    // get one when being inserted into the Universe.  Universe::Insert overloads could check what is being inserted, but
    // having SystemID be virtual and overriding it it in this class (System) is easier.
    return this->ID();
}

std::vector<int> System::FindObjectIDs() const
{
    const ObjectMap& objects = GetMainObjectMap();
    std::vector<int> retval;
    for (ObjectMultimap::const_iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (objects.Object(it->second)) {
            retval.push_back(it->second);
        } else {
            Logger().errorStream() << "System::FindObjectIDs couldn't get Object with ID " << it->second;
        }
    }
    return retval;
}

std::vector<int> System::FindObjectIDs(const UniverseObjectVisitor& visitor) const
{
    const ObjectMap& objects = GetMainObjectMap();
    std::vector<int> retval;
    for (ObjectMultimap::const_iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (const UniverseObject* obj = objects.Object(it->second)) {
            if (obj->Accept(visitor))
                retval.push_back(it->second);
        } else {
            Logger().errorStream() << "System::FindObjectIDs couldn't get Object with ID " << it->second;
        }
    }
    return retval;
}

std::vector<int> System::FindObjectIDsInOrbit(int orbit, const UniverseObjectVisitor& visitor) const
{
    const ObjectMap& objects = GetMainObjectMap();
    std::vector<int> retval;
    std::pair<ObjectMultimap::const_iterator, ObjectMultimap::const_iterator> range = m_objects.equal_range(orbit);
    for (ObjectMultimap::const_iterator it = range.first; it != range.second; ++it) {
        if (const UniverseObject* obj = objects.Object(it->second)) {
            if (obj->Accept(visitor))
                retval.push_back(it->second);
        } else {
            Logger().errorStream() << "System::FindObjectIDsInOrbit couldn't get Object with ID " << it->second;
        }
    }
    return retval;
}

System::const_orbit_iterator System::begin() const
{ return m_objects.begin(); }

System::const_orbit_iterator System::end() const
{ return m_objects.end(); }

bool System::Contains(int object_id) const
{
    // checks if this system object thinks it contains an object with the
    // indicated ID.  does not check if there exists such an object or whether
    // that object thinks it is in this system
    for (ObjectMultimap::const_iterator it = m_objects.begin(); it != m_objects.end(); ++it)
        if (it->second == object_id)
            return true;
    return false;
}

std::pair<System::const_orbit_iterator, System::const_orbit_iterator> System::orbit_range(int o) const
{ return m_objects.equal_range(o); }

std::pair<System::const_orbit_iterator, System::const_orbit_iterator> System::non_orbit_range() const
{ return m_objects.equal_range(-1); }

System::const_lane_iterator System::begin_lanes() const
{ return m_starlanes_wormholes.begin(); }

System::const_lane_iterator System::end_lanes() const
{ return m_starlanes_wormholes.end(); }

int System::LastTurnBattleHere() const
{return m_last_turn_battle_here;}

UniverseObject* System::Accept(const UniverseObjectVisitor& visitor) const
{ return visitor.Visit(const_cast<System* const>(this)); }

int System::Insert(UniverseObject* obj)
{ return Insert(obj, -1); }

int System::Insert(UniverseObject* obj, int orbit)
{
    if (!obj) {
        Logger().errorStream() << "System::Insert() : Attempted to place a null object in a System";
        return -1;
    }

    //Logger().debugStream() << "System::Insert system " << this->Name() <<  " (object " << obj->Name() << ", orbit " << orbit << ")";
    //Logger().debugStream() << "..initial objects in system: ";
    //for (ObjectMultimap::iterator it = m_objects.begin(); it != m_objects.end(); ++it)
    //    Logger().debugStream() << ".... id: " << it->second << " name: " << GetUniverse().Object(it->second)->Name();
    //Logger().debugStream() << "..initial object systemid: " << obj->SystemID();

    if (orbit < -1) {
        Logger().errorStream() << "System::Insert() : Attempted to place an object in an orbit less than -1";
        return -1;
    }

    if (orbit >= m_orbits) {
        Logger().errorStream() << "System::Insert() : Attempted to place an object in a non-existing orbit with number higher than the largest numbered orbit";
        return -1;
    }

    // inform obj of its new location and system id.  this should propegate to all objects contained within obj
    obj->MoveTo(this);          // ensure object is physically at same location in universe as this system
    obj->SetSystem(this->ID()); // should do nothing if object is already in this system, but just to ensure things are consistent...


    // update obj and its contents in this System's bookkeeping
    std::list<UniverseObject*> inserted_objects;
    inserted_objects.push_back(obj);
    // recursively get all objects contained within obj
    for (std::list<UniverseObject*>::iterator it = inserted_objects.begin(); it != inserted_objects.end(); ++it) {
        std::vector<int> contained_object_ids = (*it)->FindObjectIDs();
        for (std::vector<int>::const_iterator coit = contained_object_ids.begin(); coit != contained_object_ids.end(); ++coit)
            if (UniverseObject* cobj = GetObject(*coit))
                inserted_objects.push_back(cobj);
    }


    // for all inserted objects, up date this System's bookkeeping about orbit and whether it is contained herein
    for (std::list<UniverseObject*>::iterator it = inserted_objects.begin(); it != inserted_objects.end(); ++it) {
        int cur_id = (*it)->ID();

        // check if obj is already in system's internal bookkeeping.  if it is, check if it is in the wrong
        // orbit.  if it is both, remove it from its current orbit
        for (orbit_iterator orbit_it = m_objects.begin(); orbit_it != m_objects.end(); ++orbit_it) {
            if (orbit != orbit_it->first && orbit_it->second == cur_id) {
                m_objects.erase(orbit_it);
                break;                  // assuming no duplicate entries
            }
        }

        // add obj to system's internal bookkeeping
        std::pair<int, int> insertion(orbit, cur_id);
        m_objects.insert(insertion);
    }


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
    UniverseObject* object = GetMainObjectMap().Object(obj_id);
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

    // ensure object and (recursive) contents are all removed from system
    std::list<UniverseObject*> removed_objects;
    removed_objects.push_back(obj);
    obj = 0;    // to ensure I don't accidentally use obj instead of cur_obj in subsequent code
    // recursively get all objects contained within obj
    for (std::list<UniverseObject*>::iterator it = removed_objects.begin(); it != removed_objects.end(); ++it) {
        std::vector<int> contained_object_ids = (*it)->FindObjectIDs();
        for (std::vector<int>::const_iterator coit = contained_object_ids.begin(); coit != contained_object_ids.end(); ++coit)
            if (UniverseObject* cobj = GetObject(*coit))
                removed_objects.push_back(cobj);
    }


    // keep track of what is removed...
    bool removed_something = false;
    std::vector<Fleet*> removed_fleets;
    bool removed_planet = false;


    // do actual removal
    for (std::list<UniverseObject*>::iterator it = removed_objects.begin(); it != removed_objects.end(); ++it) {
        UniverseObject* cur_obj = *it;

        //if (this->ID() != cur_obj->SystemID())
        //    Logger().debugStream() << "System::Remove tried to remove an object whose system id was not this system.  Its current system id is: " << cur_obj->SystemID();

        for (ObjectMultimap::iterator map_it = m_objects.begin(); map_it != m_objects.end(); ++map_it) {
            if (map_it->second == cur_obj->ID()) {
                m_objects.erase(map_it);
                cur_obj->SetSystem(INVALID_OBJECT_ID);

                removed_something = true;

                if (universe_object_cast<Planet*>(cur_obj))
                    removed_planet = true;
                if (Fleet* fleet = universe_object_cast<Fleet*>(cur_obj))
                    removed_fleets.push_back(fleet);

                break;                  // assuming no duplicate entries
            }
        }
    }

    if (removed_something) {
        // UI bookeeping
        if (removed_planet)
            UpdateOwnership();
        for (std::vector<Fleet*>::const_iterator it = removed_fleets.begin(); it != removed_fleets.end(); ++it)
            FleetRemovedSignal(**it);

        StateChangedSignal();
    } else {
        // didn't find object.  this doesn't necessarily indicate a problem, as removing objects from
        // systems may be done redundantly when inserting or moving objects that contain other objects
        Logger().debugStream() << "System::Remove didn't find object in system";
    }
}

void System::Remove(int id)
{ Remove(GetMainObjectMap().Object(id)); }

void System::SetStarType(StarType type)
{
    m_star = type;
    if (m_star <= INVALID_STAR_TYPE || NUM_STAR_TYPES <= m_star)
        Logger().errorStream() << "System::SetStarType set star type to " << boost::lexical_cast<std::string>(type);
    StateChangedSignal();
}

void System::AddStarlane(int id)
{
    if (!HasStarlaneTo(id) && id != this->ID()) {
        m_starlanes_wormholes[id] = false;
        StateChangedSignal();
        if (GetOptionsDB().Get<bool>("verbose-logging"))
            Logger().debugStream() << "Added starlane from system " << this->Name() << " (" << this->ID() << ") system " << id;
    }
}

void System::AddWormhole(int id)
{
    if (!HasWormholeTo(id) && id != this->ID()) {
        m_starlanes_wormholes[id] = true;
        StateChangedSignal();
    }
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

void System::UpdateOwnership()
{
    m_empires_with_planets_here.clear();
    for (ObjectMultimap::iterator it = m_objects.begin(); it != m_objects.end(); ++it)
        if (Planet* planet = GetMainObjectMap().Object<Planet>(it->second))
            if (!planet->Unowned())
                m_empires_with_planets_here.insert(planet->Owner());
}

void System::SetLastTurnBattleHere(int turn)
{ m_last_turn_battle_here = turn; }

void System::ResetTargetMaxUnpairedMeters(MeterType meter_type/* = INVALID_METER_TYPE*/)
{
    UniverseObject::ResetTargetMaxUnpairedMeters(meter_type);
    // give systems base stealth slightly above zero, so that they can't be seen from a distance without high detection ability
    if (meter_type == INVALID_METER_TYPE || meter_type == METER_STEALTH)
        if (Meter* stealth = GetMeter(METER_STEALTH))
            stealth->AddToCurrent(0.01);
}

System::orbit_iterator System::begin()
{ return m_objects.begin(); }

System::orbit_iterator System::end()
{ return m_objects.end(); }

std::pair<System::orbit_iterator, System::orbit_iterator> System::orbit_range(int o)
{ return m_objects.equal_range(o); }

std::pair<System::orbit_iterator, System::orbit_iterator> System::non_orbit_range()
{ return m_objects.equal_range(-1); }

bool System::OrbitOccupied(int orbit) const 
{ return (m_objects.find(orbit) != m_objects.end()); }

std::set<int> System::FreeOrbits() const
{
    std::set<int> occupied;
    for (const_orbit_iterator it = begin(); it != end(); ++it)
        occupied.insert(it->first);
    std::set<int> retval;
    for (int i = 0; i < m_orbits; ++i)
        if (occupied.find(i) == occupied.end())
            retval.insert(i);
    return retval;
}

System::lane_iterator System::begin_lanes()
{ return m_starlanes_wormholes.begin(); }

System::lane_iterator System::end_lanes()
{ return m_starlanes_wormholes.end(); }

System::StarlaneMap System::StarlanesWormholes() const
{ return m_starlanes_wormholes; }

System::StarlaneMap System::VisibleStarlanesWormholes(int empire_id) const
{
    if (empire_id == ALL_EMPIRES)
        return m_starlanes_wormholes;

    const Universe& universe = GetUniverse();
    const ObjectMap& objects = universe.Objects();


    Visibility this_system_vis = universe.GetObjectVisibilityByEmpire(this->ID(), empire_id);

    //visible starlanes are:
    //  - those connected to systems with vis >= partial
    //  - those with visible ships travelling along them


    // return all starlanes if partially visible or better
    if (this_system_vis >= VIS_PARTIAL_VISIBILITY)
        return m_starlanes_wormholes;


    // compile visible lanes connected to this only basically-visible system
    StarlaneMap retval;


    // check if any of the adjacent systems are partial or better visible
    for (StarlaneMap::const_iterator it = m_starlanes_wormholes.begin(); it != m_starlanes_wormholes.end(); ++it) {
        int lane_end_sys_id = it->first;
        if (universe.GetObjectVisibilityByEmpire(lane_end_sys_id, empire_id) >= VIS_PARTIAL_VISIBILITY)
            retval[lane_end_sys_id] = it->second;
    }


    // early exit check... can't see any more lanes than exist, so don't need to check for more if all lanes are already visible
    if (retval == m_starlanes_wormholes)
        return retval;


    // check if any fleets owned by empire are moving along a starlane connected to this system...

    // get moving fleets owned by empire
    std::vector<const Fleet*> moving_empire_fleets;
    std::vector<const UniverseObject*> moving_fleet_objects = objects.FindObjects(MovingFleetVisitor());
    for (std::vector<const UniverseObject*>::const_iterator it = moving_fleet_objects.begin(); it != moving_fleet_objects.end(); ++it)
        if (const Fleet* fleet = universe_object_cast<const Fleet*>(*it))
            if (fleet->OwnedBy(empire_id))
                moving_empire_fleets.push_back(fleet);

    // add any lanes an owned fleet is moving along that connect to this system
    for (std::vector<const Fleet*>::const_iterator it = moving_empire_fleets.begin(); it != moving_empire_fleets.end(); ++it) {
        const Fleet* fleet = *it;
        if (fleet->SystemID() != UniverseObject::INVALID_OBJECT_ID) {
            Logger().errorStream() << "System::VisibleStarlanesWormholes somehow got a moving fleet that had a valid system id?";
            continue;
        }

        int prev_sys_id = fleet->PreviousSystemID();
        int next_sys_id = fleet->NextSystemID();

        // see if previous or next system is this system, and if so, is other
        // system on lane along which ship is moving one of this system's
        // starlanes or wormholes?
        int other_lane_end_sys_id = UniverseObject::INVALID_OBJECT_ID;

        if (prev_sys_id == this->ID())
            other_lane_end_sys_id = next_sys_id;
        else if (next_sys_id == this->ID())
            other_lane_end_sys_id = prev_sys_id;

        if (other_lane_end_sys_id != UniverseObject::INVALID_OBJECT_ID) {
            StarlaneMap::const_iterator lane_it = m_starlanes_wormholes.find(other_lane_end_sys_id);
            if (lane_it == m_starlanes_wormholes.end()) {
                Logger().errorStream() << "System::VisibleStarlanesWormholes found an owned fleet moving along a starlane connected to this system that isn't also connected to one of this system's starlane-connected systems...?";
                continue;
            }
            retval[other_lane_end_sys_id] = lane_it->second;
        }
    }

    return retval;
}

System::ObjectMultimap System::VisibleContainedObjects(int empire_id) const
{
    ObjectMultimap retval;
    const Universe& universe = GetUniverse();
    for (ObjectMultimap::const_iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        int object_id = it->second;
        if (universe.GetObjectVisibilityByEmpire(object_id, empire_id) >= VIS_BASIC_VISIBILITY)
            retval.insert(*it);
    }
    return retval;
}


// free functions

double SystemRadius()
{ return 1000.0 + 50.0; }

double StarRadius()
{ return 80.0; }

double OrbitalRadius(unsigned int orbit)
{
    assert(orbit < 10);
    return (SystemRadius() - 50.0) / 10 * (orbit + 1) - 20.0;
}

double StarlaneEntranceOrbitalRadius()
{ return SystemRadius() - StarlaneEntranceRadialAxis(); }

double StarlaneEntranceRadialAxis()
{ return 40.0; }

double StarlaneEntranceTangentAxis()
{ return 80.0; }

double StarlaneEntranceOrbitalPosition(int from_system, int to_system)
{
    const ObjectMap& objects = GetMainObjectMap();
    const System* system_1 = objects.Object<System>(from_system);
    const System* system_2 = objects.Object<System>(to_system);
    if (!system_1 || !system_2) {
        Logger().errorStream() << "StarlaneEntranceOrbitalPosition passed invalid system id";
        return 0.0;
    }
    return std::atan2(system_2->Y() - system_1->Y(), system_2->X() - system_1->X());
}

bool PointInStarlaneEllipse(double x, double y, int from_system, int to_system)
{
    double rads = StarlaneEntranceOrbitalPosition(from_system, to_system);
    double ellipse_x = StarlaneEntranceOrbitalRadius() * std::cos(rads);
    double ellipse_y = StarlaneEntranceOrbitalRadius() * std::sin(rads);
    return PointInEllipse(x, y,
                          ellipse_x, ellipse_y,
                          StarlaneEntranceRadialAxis(), StarlaneEntranceTangentAxis(),
                          rads);
}
