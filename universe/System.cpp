#include "System.h"

#include "Fleet.h"
#include "Ship.h"
#include "Planet.h"
#include "Building.h"
#include "Predicates.h"
#include "Universe.h"
#include "Enums.h"

#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"

#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"

#include <boost/lexical_cast.hpp>
#include <stdexcept>

System::System() :
    m_star(INVALID_STAR_TYPE)
{}

System::System(StarType star, const std::string& name, double x, double y) :
    UniverseObject(name, x, y),
    m_star(star)
{
    //DebugLogger() << "System::System(" << star << ", " << orbits << ", " << name << ", " << x << ", " << y << ")";
    if (m_star < INVALID_STAR_TYPE || NUM_STAR_TYPES < m_star)
        throw std::invalid_argument("System::System : Attempted to create a system \"" +
                                    Name() + "\" with an invalid star type.");

    m_orbits.assign(SYSTEM_ORBITS, INVALID_OBJECT_ID);

    UniverseObject::Init();
}

System::System(StarType star, const std::map<int, bool>& lanes_and_holes,
               const std::string& name, double x, double y) :
    UniverseObject(name, x, y),
    m_star(star),
    m_starlanes_wormholes(lanes_and_holes)
{
    //DebugLogger() << "System::System(" << star << ", " << orbits << ", (StarlaneMap), " << name << ", " << x << ", " << y << ")";
    if (m_star < INVALID_STAR_TYPE || NUM_STAR_TYPES < m_star)
        throw std::invalid_argument("System::System : Attempted to create a system \"" +
                                    Name() + "\" with an invalid star type.");

    m_orbits.assign(SYSTEM_ORBITS, INVALID_OBJECT_ID);

    SetSystem(ID());

    UniverseObject::Init();
}

System* System::Clone(int empire_id) const {
    Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(this->ID(), empire_id);

    if (!(vis >= VIS_BASIC_VISIBILITY && vis <= VIS_FULL_VISIBILITY))
        return nullptr;

    System* retval = new System();
    retval->Copy(shared_from_this(), empire_id);
    return retval;
}

void System::Copy(std::shared_ptr<const UniverseObject> copied_object, int empire_id) {
    if (copied_object.get() == this)
        return;
    std::shared_ptr<const System> copied_system = std::dynamic_pointer_cast<const System>(copied_object);
    if (!copied_system) {
        ErrorLogger() << "System::Copy passed an object that wasn't a System";
        return;
    }

    int copied_object_id = copied_object->ID();
    Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(copied_object_id, empire_id);
    std::set<std::string> visible_specials = GetUniverse().GetObjectVisibleSpecialsByEmpire(copied_object_id, empire_id);

    UniverseObject::Copy(copied_object, vis, visible_specials);

    if (vis >= VIS_BASIC_VISIBILITY) {
        // add any visible lanes, without removing existing entries
        std::map<int, bool> visible_lanes_holes = copied_system->VisibleStarlanesWormholes(empire_id);
        for (const auto& entry : visible_lanes_holes)
        { this->m_starlanes_wormholes[entry.first] = entry.second; }

        // copy visible info of visible contained objects
        this->m_objects = copied_system->VisibleContainedObjectIDs(empire_id);

        // only copy orbit info for visible planets
        size_t orbits_size = m_orbits.size();
        m_orbits.clear();
        m_orbits.assign(orbits_size, INVALID_OBJECT_ID);
        for (int o = 0; o < static_cast<int>(copied_system->m_orbits.size()); ++o) {
            int planet_id = copied_system->m_orbits[o];
            if (m_objects.count(planet_id))
                m_orbits[o] = planet_id;
        }

        // copy visible contained object per-type info
        m_planets.clear();
        for (int planet_id : copied_system->m_planets)
        {
            if (m_objects.count(planet_id))
                m_planets.insert(planet_id);
        }

        m_buildings.clear();
        for (int building_id : copied_system->m_buildings) {
            if (m_objects.count(building_id))
                m_buildings.insert(building_id);
        }

        m_fleets.clear();
        for (int fleet_id : copied_system->m_fleets) {
            if (m_objects.count(fleet_id))
                m_fleets.insert(fleet_id);
        }

        m_ships.clear();
        for (int ship_id : copied_system->m_ships) {
            if (m_objects.count(ship_id))
                m_ships.insert(ship_id);
        }

        m_fields.clear();
        for (int field_id : copied_system->m_fields) {
            if (m_objects.count(field_id))
                m_fields.insert(field_id);
        }


        if (vis >= VIS_PARTIAL_VISIBILITY) {
            this->m_name =                  copied_system->m_name;
            this->m_star =                  copied_system->m_star;
            this->m_last_turn_battle_here = copied_system->m_last_turn_battle_here;

            // remove any not-visible lanes that were previously known: with
            // partial vis, they should be seen, but aren't, so are known not
            // to exist any more

            // remove previously known lanes that aren't currently visible
            for (auto entry_it = m_starlanes_wormholes.begin(); entry_it != m_starlanes_wormholes.end();
                 /* conditional increment in deleting loop */)
            {
                int lane_end_sys_id = entry_it->first;
                if (!visible_lanes_holes.count(lane_end_sys_id)) {
                    entry_it = m_starlanes_wormholes.erase(entry_it);
                } else {
                    ++entry_it;
                }
            }
        }
    }
}

UniverseObjectType System::ObjectType() const
{ return OBJ_SYSTEM; }

std::string System::Dump(unsigned short ntabs) const {
    std::stringstream os;
    os << UniverseObject::Dump(ntabs);
    os << " star type: " << m_star
       << "  last combat on turn: " << m_last_turn_battle_here
       << "  total orbits: " << m_orbits.size();

    if (m_orbits.size() > 0) {
        os << "  objects per orbit: ";

        int orbit_index = 0;
        for (auto it = m_orbits.begin();
            it != m_orbits.end();)
        {
            os << "[" << orbit_index << "]" << *it;
            ++it;
            if (it != m_orbits.end())
                os << ", ";
            ++orbit_index;
        }
    }

    os << "  starlanes: ";
    for (auto it = m_starlanes_wormholes.begin();
         it != m_starlanes_wormholes.end();)
    {
        int lane_end_id = it->first;
        ++it;
        os << lane_end_id << (it == m_starlanes_wormholes.end() ? "" : ", ");
    }

    os << "  objects: ";
    for (auto it = m_objects.begin(); it != m_objects.end();) {
        int obj_id = *it;
        ++it;
        if (obj_id == INVALID_OBJECT_ID)
            continue;
        os << obj_id << (it == m_objects.end() ? "" : ", ");
    }
    return os.str();
}

const std::string& System::ApparentName(int empire_id, bool blank_unexplored_and_none/* = false*/) const {
    static const std::string EMPTY_STRING;

    if (empire_id == ALL_EMPIRES)
        return this->PublicName(empire_id);

    // has the indicated empire ever detected this system?
    const auto& vtm = GetUniverse().GetObjectVisibilityTurnMapByEmpire(this->ID(), empire_id);
    if (!vtm.count(VIS_PARTIAL_VISIBILITY)) {
        if (blank_unexplored_and_none)
            return EMPTY_STRING;

        if (m_star == INVALID_STAR_TYPE)
            return UserString("UNEXPLORED_REGION");
        else
            return UserString("UNEXPLORED_SYSTEM");
    }

    if (m_star == STAR_NONE) {
        // determine if there are any planets in the system
        for (const auto& entry : Objects().ExistingPlanets()) {
            if (entry.second->SystemID() == this->ID())
                return this->PublicName(empire_id);
        }
        if (blank_unexplored_and_none) {
            //DebugLogger() << "System::ApparentName No-Star System (" << ID() << "), returning name "<< EMPTY_STRING;
            return EMPTY_STRING;
        }
        //DebugLogger() << "System::ApparentName No-Star System (" << ID() << "), returning name "<< UserString("EMPTY_SPACE");
        return UserString("EMPTY_SPACE");
    }

    return this->PublicName(empire_id);
}

StarType System::NextOlderStarType() const {
    if (m_star <= INVALID_STAR_TYPE || m_star >= NUM_STAR_TYPES)
        return INVALID_STAR_TYPE;
    if (m_star >= STAR_RED)
        return m_star;  // STAR_RED, STAR_NEUTRON, STAR_BLACK, STAR_NONE
    if (m_star <= STAR_BLUE)
        return STAR_BLUE;
    return StarType(m_star + 1);
}

StarType System::NextYoungerStarType() const {
    if (m_star <= INVALID_STAR_TYPE || m_star >= NUM_STAR_TYPES)
        return INVALID_STAR_TYPE;
    if (m_star >= STAR_RED)
        return m_star;  // STAR_RED, STAR_NEUTRON, STAR_BLACK, STAR_NONE
    if (m_star <= STAR_BLUE)
        return STAR_BLUE;
    return StarType(m_star - 1);
}

int System::NumStarlanes() const {
    int retval = 0;
    for (const auto& entry : m_starlanes_wormholes) {
        if (!entry.second)
            ++retval;
    }
    return retval;
}

int System::NumWormholes() const {
    int retval = 0;
    for (const auto& entry : m_starlanes_wormholes) {
        if (entry.second)
            ++retval;
    }
    return retval;
}

bool System::HasStarlaneTo(int id) const {
    auto it = m_starlanes_wormholes.find(id);
    return (it == m_starlanes_wormholes.end() ? false : it->second == false);
}

bool System::HasWormholeTo(int id) const {
    auto it = m_starlanes_wormholes.find(id);
    return (it == m_starlanes_wormholes.end() ? false : it->second == true);
}

int  System::Owner() const {
    // Check if all of the owners are the same empire.
    int first_owner_found(ALL_EMPIRES);
    for (int planet_id : m_planets) {
        if (auto planet = GetPlanet(planet_id)) {
            const int owner = planet->Owner();
            if (owner == ALL_EMPIRES)
                continue;
            if (first_owner_found == ALL_EMPIRES)
                first_owner_found = owner;
            if (first_owner_found != owner)
                return ALL_EMPIRES;
        }
    }
    return first_owner_found;
}

const std::set<int>& System::ContainedObjectIDs() const
{ return m_objects; }

bool System::Contains(int object_id) const {
    if (object_id == INVALID_OBJECT_ID)
        return false;
    return m_objects.count(object_id);
}

std::shared_ptr<UniverseObject> System::Accept(const UniverseObjectVisitor& visitor) const
{ return visitor.Visit(std::const_pointer_cast<System>(std::static_pointer_cast<const System>(shared_from_this()))); }

void System::Insert(std::shared_ptr<UniverseObject> obj, int orbit/* = -1*/) {
    if (!obj) {
        ErrorLogger() << "System::Insert() : Attempted to place a null object in a System";
        return;
    }
    if (orbit < -1 || orbit >= static_cast<int>(m_orbits.size())) {
        ErrorLogger() << "System::Insert() : Attempted to place an object in invalid orbit";
        return;
    }

    obj->MoveTo(this->X(), this->Y());
    obj->SetSystem(this->ID());

    if (obj->ObjectType() == OBJ_PLANET) {
        if (orbit == -1) {
            bool already_in_orbit = false;
            for (int planet_id : m_orbits) {
                if (planet_id == obj->ID()) {
                    already_in_orbit = true;
                    break;
                }
            }
            // if planet already in an orbit, do nothing
            // if planet not in an orbit, find orbit to put planet in
            if (!already_in_orbit) {
                for (int o = 0; o < static_cast<int>(m_orbits.size()); ++o) {
                    if (m_orbits[o] == INVALID_OBJECT_ID) {
                        orbit = o;
                        m_orbits[orbit] = obj->ID();
                        break;
                    }
                }
            }
        } else {
            // check if desired orbit is occupied
            // if yes, it is either already occupied by the inserted object or
            // by another object. in either case, do nothing.
            // if not, put object into the orbit, and remove from any other
            // orbit it currently occupies.
            if (!OrbitOccupied(orbit)) {
                // check if object already in any different orbit
                // ... if yes, remove it
                for (int o = 0; o < static_cast<int>(m_orbits.size()); ++o) {
                    if (o != orbit && m_orbits[o] == obj->ID())
                        m_orbits[o] = INVALID_OBJECT_ID;
                }
                // put object into desired orbit
                m_orbits[orbit] = obj->ID();
            } else {  // Log as an error, if no current orbit attempt to assign to a free orbit
                ErrorLogger() << "System::Insert() Planet " << obj->ID()
                              << " requested orbit " << orbit
                              << " in system " << ID()
                              << ", which is occupied by" << m_orbits[orbit];
                const std::set<int>& free_orbits = FreeOrbits();
                if (free_orbits.size() > 0 && OrbitOfPlanet(obj->ID()) == -1) {
                    int new_orbit = *(free_orbits.begin());
                    m_orbits[new_orbit] = obj->ID();
                    DebugLogger() << "System::Insert() Planet " << obj->ID()
                                  << " assigned to orbit " << new_orbit;
                }
            }
        }
        //TODO If planet not assigned to an orbit, reject insertion of planet, provide feedback to caller
    }
    // if not a planet, don't need to put into an orbit

    switch (obj->ObjectType()) {
    case OBJ_SHIP: {
        m_ships.insert(obj->ID());
        if (std::shared_ptr<Ship> ship = std::dynamic_pointer_cast<Ship>(obj))
            ship->SetArrivedOnTurn(CurrentTurn());
        break;
    }
    case OBJ_FLEET: {
        m_fleets.insert(obj->ID());
        FleetsInsertedSignal({std::dynamic_pointer_cast<Fleet>(obj)});
        break;
    }
    case OBJ_PLANET:
        m_planets.insert(obj->ID());
        break;
    case OBJ_FIELD:
        m_fields.insert(obj->ID());
        break;
    case OBJ_SYSTEM:
        ErrorLogger() << "System::Insert inserting a system into another system...??";
        break;
    case OBJ_BUILDING:
        m_buildings.insert(obj->ID());
        break;
    default:
        ErrorLogger() << "System::Insert inserting an unknown object type";
    }
    m_objects.insert(obj->ID());

    StateChangedSignal();
}

void System::Remove(int id) {
    if (id == INVALID_OBJECT_ID)
        return;

    bool removed_fleet = false;

    auto it = m_fleets.find(id);
    if (it != m_fleets.end()) {
        m_fleets.erase(it);
        removed_fleet = true;
    }

    it = m_planets.find(id);
    if (it != m_planets.end()) {
        m_planets.erase(it);
        for (int& planet_id : m_orbits)
            if (planet_id == id)
                planet_id = INVALID_OBJECT_ID;
    }

    m_ships.erase(id);
    m_fields.erase(id);
    m_buildings.erase(id);
    m_objects.erase(id);

    if (removed_fleet) {
        if (auto fleet = GetFleet(id))
            FleetsRemovedSignal({fleet});
    }
    StateChangedSignal();
}

void System::SetStarType(StarType type) {
    m_star = type;
    if (m_star <= INVALID_STAR_TYPE || NUM_STAR_TYPES <= m_star)
        ErrorLogger() << "System::SetStarType set star type to " << boost::lexical_cast<std::string>(type);
    StateChangedSignal();
}

void System::AddStarlane(int id) {
    if (!HasStarlaneTo(id) && id != this->ID()) {
        m_starlanes_wormholes[id] = false;
        StateChangedSignal();
        TraceLogger() << "Added starlane from system " << this->Name() << " (" << this->ID() << ") system " << id;
    }
}

void System::AddWormhole(int id) {
    if (!HasWormholeTo(id) && id != this->ID()) {
        m_starlanes_wormholes[id] = true;
        StateChangedSignal();
    }
}

bool System::RemoveStarlane(int id) {
    bool retval = false;
    if ((retval = HasStarlaneTo(id))) {
        m_starlanes_wormholes.erase(id);
        StateChangedSignal();
    }
    return retval;
}

bool System::RemoveWormhole(int id) {
    bool retval = false;
    if ((retval = HasWormholeTo(id))) {
        m_starlanes_wormholes.erase(id);
        StateChangedSignal();
    }
    return retval;
}

void System::SetLastTurnBattleHere(int turn)
{ m_last_turn_battle_here = turn; }

void System::ResetTargetMaxUnpairedMeters() {
    UniverseObject::ResetTargetMaxUnpairedMeters();

    // give systems base stealth slightly above zero, so that they can't be
    // seen from a distance without high detection ability
    if (Meter* stealth = GetMeter(METER_STEALTH)) {
        stealth->ResetCurrent();
        //stealth->AddToCurrent(0.01f);
    }
}

bool System::OrbitOccupied(int orbit) const {
    if (orbit < 0 || orbit >= static_cast<int>(m_orbits.size()))
        return false;
    return m_orbits[orbit] != INVALID_OBJECT_ID;
}

int System::PlanetInOrbit(int orbit) const {
    if (orbit < 0 || orbit >= static_cast<int>(m_orbits.size()))
        return INVALID_OBJECT_ID;
    return m_orbits[orbit];
}

int System::OrbitOfPlanet(int object_id) const {
    if (object_id == INVALID_OBJECT_ID)
        return -1;
    for (int o = 0; o < static_cast<int>(m_orbits.size()); ++o)
        if (m_orbits[o] == object_id)
            return o;
    return -1;
}

std::set<int> System::FreeOrbits() const {
    std::set<int> retval;
    for (int o = 0; o < static_cast<int>(m_orbits.size()); ++o)
        if (m_orbits[o] == INVALID_OBJECT_ID)
            retval.insert(o);
    return retval;
}

const std::map<int, bool>& System::StarlanesWormholes() const
{ return m_starlanes_wormholes; }

std::map<int, bool> System::VisibleStarlanesWormholes(int empire_id) const {
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
    std::map<int, bool> retval;


    // check if any of the adjacent systems are partial or better visible
    for (const auto& entry : m_starlanes_wormholes) {
        int lane_end_sys_id = entry.first;
        if (universe.GetObjectVisibilityByEmpire(lane_end_sys_id, empire_id) >= VIS_PARTIAL_VISIBILITY)
            retval[lane_end_sys_id] = entry.second;
    }


    // early exit check... can't see any more lanes than exist, so don't need to check for more if all lanes are already visible
    if (retval == m_starlanes_wormholes)
        return retval;


    // check if any fleets owned by empire are moving along a starlane connected to this system...

    // get moving fleets owned by empire
    std::vector<std::shared_ptr<const Fleet>> moving_empire_fleets;
    for (auto& object : objects.FindObjects(MovingFleetVisitor())) {
        if (auto fleet = std::dynamic_pointer_cast<const Fleet>(object))
            if (fleet->OwnedBy(empire_id))
                moving_empire_fleets.push_back(fleet);
    }

    // add any lanes an owned fleet is moving along that connect to this system
    for (auto& fleet : moving_empire_fleets) {
        if (fleet->SystemID() != INVALID_OBJECT_ID) {
            ErrorLogger() << "System::VisibleStarlanesWormholes somehow got a moving fleet that had a valid system id?";
            continue;
        }

        int prev_sys_id = fleet->PreviousSystemID();
        int next_sys_id = fleet->NextSystemID();

        // see if previous or next system is this system, and if so, is other
        // system on lane along which ship is moving one of this system's
        // starlanes or wormholes?
        int other_lane_end_sys_id = INVALID_OBJECT_ID;

        if (prev_sys_id == this->ID())
            other_lane_end_sys_id = next_sys_id;
        else if (next_sys_id == this->ID())
            other_lane_end_sys_id = prev_sys_id;

        if (other_lane_end_sys_id != INVALID_OBJECT_ID) {
            auto lane_it = m_starlanes_wormholes.find(other_lane_end_sys_id);
            if (lane_it == m_starlanes_wormholes.end()) {
                ErrorLogger() << "System::VisibleStarlanesWormholes found an owned fleet moving along a starlane connected to this system that isn't also connected to one of this system's starlane-connected systems...?";
                continue;
            }
            retval[other_lane_end_sys_id] = lane_it->second;
        }
    }

    return retval;
}

void System::SetOverlayTexture(const std::string& texture, double size) {
    m_overlay_texture = texture;
    m_overlay_size = size;
    StateChangedSignal();
}
