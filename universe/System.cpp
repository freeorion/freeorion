#include "System.h"

#include "Fleet.h"
#include "Ship.h"
#include "Planet.h"
#include "Building.h"
#include "Predicates.h"
#include "Universe.h"

#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"

#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/Math.h"
#include "../util/OptionsDB.h"

#include <boost/lexical_cast.hpp>
#include <stdexcept>

namespace {
    const int SYSTEM_ORBITS = 9;
}

System::System() :
    UniverseObject(),
    m_star(INVALID_STAR_TYPE),
    m_last_turn_battle_here(INVALID_GAME_TURN),
    m_overlay_texture(),
    m_overlay_size(1.0)
{
    m_orbits.assign(SYSTEM_ORBITS, INVALID_OBJECT_ID);
}

System::System(StarType star, const std::string& name, double x, double y) :
    UniverseObject(name, x, y),
    m_star(star),
    m_last_turn_battle_here(INVALID_GAME_TURN),
    m_overlay_texture(),
    m_overlay_size(1.0)
{
    m_orbits.assign(SYSTEM_ORBITS, INVALID_OBJECT_ID);

    //Logger().debugStream() << "System::System(" << star << ", " << orbits << ", " << name << ", " << x << ", " << y << ")";
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
    m_starlanes_wormholes(lanes_and_holes),
    m_last_turn_battle_here(INVALID_GAME_TURN),
    m_overlay_texture(),
    m_overlay_size(1.0)
{
    m_orbits.assign(SYSTEM_ORBITS, INVALID_OBJECT_ID);

    //Logger().debugStream() << "System::System(" << star << ", " << orbits << ", (StarlaneMap), " << name << ", " << x << ", " << y << ")";
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
        return 0;

    System* retval = new System();
    retval->Copy(TemporaryFromThis(), empire_id);
    return retval;
}

void System::Copy(TemporaryPtr<const UniverseObject> copied_object, int empire_id) {
    if (copied_object == this)
        return;
    TemporaryPtr<const System> copied_system = universe_object_ptr_cast<System>(copied_object);
    if (!copied_system) {
        Logger().errorStream() << "System::Copy passed an object that wasn't a System";
        return;
    }

    int copied_object_id = copied_object->ID();
    Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(copied_object_id, empire_id);
    std::set<std::string> visible_specials = GetUniverse().GetObjectVisibleSpecialsByEmpire(copied_object_id, empire_id);

    UniverseObject::Copy(copied_object, vis, visible_specials);

    if (vis >= VIS_BASIC_VISIBILITY) {
        // add any visible lanes, without removing existing entries
        std::map<int, bool> visible_lanes_holes = copied_system->VisibleStarlanesWormholes(empire_id);
        for (std::map<int, bool>::const_iterator it = visible_lanes_holes.begin();
             it != visible_lanes_holes.end(); ++it)
        { this->m_starlanes_wormholes[it->first] = it->second; }

        // copy visible info of visible contained objects
        this->m_objects = copied_system->VisibleContainedObjectIDs(empire_id);

        // only copy orbit info for visible planets
        m_orbits.clear();
        for (int o = 0; o < static_cast<int>(copied_system->m_orbits.size()); ++o) {
            int planet_id = copied_system->m_orbits[o];
            if (m_objects.find(planet_id) != m_objects.end())
                m_orbits[o] = planet_id;
        }

        // copy visible contained object per-type info
        m_planets.clear();
        for (std::set<int>::const_iterator it = copied_system->m_planets.begin();
             it != copied_system->m_planets.end(); ++it)
        {
            if (m_objects.find(*it) != m_objects.end())
                m_planets.insert(*it);
        }

        m_buildings.clear();
        for (std::set<int>::const_iterator it = copied_system->m_buildings.begin();
             it != copied_system->m_buildings.end(); ++it)
        {
            if (m_objects.find(*it) != m_objects.end())
                m_buildings.insert(*it);
        }

        m_fleets.clear();
        for (std::set<int>::const_iterator it = copied_system->m_fleets.begin();
             it != copied_system->m_fleets.end(); ++it)
        {
            if (m_objects.find(*it) != m_objects.end())
                m_fleets.insert(*it);
        }

        m_ships.clear();
        for (std::set<int>::const_iterator it = copied_system->m_ships.begin();
             it != copied_system->m_ships.end(); ++it)
        {
            if (m_objects.find(*it) != m_objects.end())
                m_ships.insert(*it);
        }

        m_fields.clear();
        for (std::set<int>::const_iterator it = copied_system->m_fields.begin();
             it != copied_system->m_fields.end(); ++it)
            {
                if (m_objects.find(*it) != m_objects.end())
                    m_fields.insert(*it);
            }


        if (vis >= VIS_PARTIAL_VISIBILITY) {
            this->m_name =                  copied_system->m_name;
            this->m_star =                  copied_system->m_star;
            this->m_last_turn_battle_here = copied_system->m_last_turn_battle_here;

            // remove any not-visible lanes that were previously known: with
            // partial vis, they should be seen, but aren't, so are known not
            // to exist any more

            // assemble all previously known lanes
            std::vector<int> initial_known_lanes;
            for (std::map<int, bool>::const_iterator it = this->m_starlanes_wormholes.begin();
                 it != this->m_starlanes_wormholes.end(); ++it)
            { initial_known_lanes.push_back(it->first); }

            // remove previously known lanes that aren't currently visible
            for (std::vector<int>::const_iterator it = initial_known_lanes.begin(); it != initial_known_lanes.end(); ++it) {
                int lane_end_sys_id = *it;
                if (visible_lanes_holes.find(lane_end_sys_id) == visible_lanes_holes.end())
                    this->m_starlanes_wormholes.erase(lane_end_sys_id);
            }
        }
    }
}

const std::string& System::TypeName() const
{ return UserString("SYSTEM"); }

UniverseObjectType System::ObjectType() const
{ return OBJ_SYSTEM; }

std::string System::Dump() const {
    std::stringstream os;
    os << UniverseObject::Dump();
    os << " star type: " << UserString(GG::GetEnumMap<StarType>().FromEnum(m_star))
       << "  last combat on turn: " << m_last_turn_battle_here
       << "  starlanes: ";

    for (std::map<int, bool>::const_iterator it = m_starlanes_wormholes.begin();
         it != m_starlanes_wormholes.end();)
    {
        int lane_end_id = it->first;
        ++it;
        os << lane_end_id << (it == m_starlanes_wormholes.end() ? "" : ", ");
    }

    os << "  objects: ";
    for (std::set<int>::const_iterator it = m_objects.begin(); it != m_objects.end();) {
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
    if (!this)
        return EMPTY_STRING;

    if (empire_id == ALL_EMPIRES)
        return this->PublicName(empire_id);

    // has the indicated empire ever detected this system?
    const Universe::VisibilityTurnMap& vtm = GetUniverse().GetObjectVisibilityTurnMapByEmpire(this->ID(), empire_id);
    if (vtm.find(VIS_PARTIAL_VISIBILITY) == vtm.end()) {
        if (blank_unexplored_and_none)
            return EMPTY_STRING;

        if (m_star == INVALID_STAR_TYPE)
            return UserString("UNEXPLORED_REGION");
        else
            return UserString("UNEXPLORED_SYSTEM");
    }

    if (m_star == STAR_NONE) {
        // determine if there are any planets in the system
        for (std::map<int, TemporaryPtr<UniverseObject> >::iterator it = Objects().ExistingPlanetsBegin();
             it != Objects().ExistingPlanetsEnd(); ++it)
        {
            if (it->second->SystemID() == this->ID())
                return this->PublicName(empire_id);
        }
        if (blank_unexplored_and_none) {
            //Logger().debugStream() << "System::ApparentName No-Star System (" << ID() << "), returning name "<< EMPTY_STRING;
            return EMPTY_STRING;
        }
        //Logger().debugStream() << "System::ApparentName No-Star System (" << ID() << "), returning name "<< UserString("EMPTY_SPACE");
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
    for (std::map<int, bool>::const_iterator it = m_starlanes_wormholes.begin();
         it != m_starlanes_wormholes.end(); ++it)
    {
        if (!it->second)
            ++retval;
    }
    return retval;
}

int System::NumWormholes() const {
    int retval = 0;
    for (std::map<int, bool>::const_iterator it = m_starlanes_wormholes.begin();
         it != m_starlanes_wormholes.end(); ++it)
    {
        if (it->second)
            ++retval;
    }
    return retval;
}

bool System::HasStarlaneTo(int id) const {
    std::map<int, bool>::const_iterator it = m_starlanes_wormholes.find(id);
    return (it == m_starlanes_wormholes.end() ? false : it->second == false);
}

bool System::HasWormholeTo(int id) const {
    std::map<int, bool>::const_iterator it = m_starlanes_wormholes.find(id);
    return (it == m_starlanes_wormholes.end() ? false : it->second == true);
}

const std::set<int>& System::ContainedObjectIDs() const
{ return m_objects; }

bool System::Contains(int object_id) const {
    if (object_id == INVALID_OBJECT_ID)
        return false;
    return m_objects.find(object_id) != m_objects.end();
}

TemporaryPtr<UniverseObject> System::Accept(const UniverseObjectVisitor& visitor) const
{ return visitor.Visit(boost::const_pointer_cast<System>(boost::static_pointer_cast<const System>(TemporaryFromThis()))); }

void System::Insert(TemporaryPtr<UniverseObject> obj, int orbit/* = -1*/) {
    if (!obj) {
        Logger().errorStream() << "System::Insert() : Attempted to place a null object in a System";
        return;
    }
    if (orbit < -1 || orbit >= static_cast<int>(m_orbits.size())) {
        Logger().errorStream() << "System::Insert() : Attempted to place an object in invalid orbit";
        return;
    }

    obj->MoveTo(this->X(), this->Y());
    obj->SetSystem(this->ID());

    if (obj->ObjectType() == OBJ_PLANET) {
        if (orbit == -1) {
            bool already_in_orbit = false;
            for (int o = 0; o < static_cast<int>(m_orbits.size()); ++o) {
                if (m_orbits[o] = obj->ID()) {
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
            }
        }
    }
    // if not a planet, don't need to put into an orbit

    switch (obj->ObjectType()) {
    case OBJ_SHIP:
        m_ships.insert(obj->ID());
        break;
    case OBJ_FLEET: {
        m_fleets.insert(obj->ID());
        std::vector<TemporaryPtr<Fleet> > fleets;
        fleets.push_back(boost::dynamic_pointer_cast<Fleet>(obj));
        FleetsInsertedSignal(fleets);
        break;
    }
    case OBJ_PLANET:
        m_planets.insert(obj->ID());
        break;
    case OBJ_FIELD:
        m_fields.insert(obj->ID());
        break;
    case OBJ_SYSTEM:
        Logger().errorStream() << "System::Insert inserting a system into another system...??";
        break;
    case OBJ_BUILDING:
        m_buildings.insert(obj->ID());
        break;
    default:
        Logger().errorStream() << "System::Insert inserting an unknown object type";
    }
    m_objects.insert(obj->ID());

    StateChangedSignal();
}

void System::Remove(int id) {
    if (id == INVALID_OBJECT_ID)
        return;

    bool removed_fleet = false;

    std::set<int>::iterator it = m_fleets.find(id);
    if (it != m_fleets.end()) {
        m_fleets.erase(it);
        removed_fleet = true;
    }

    it = m_planets.find(id);
    if (it != m_planets.end()) {
        m_planets.erase(it);
        for (int i = 0; i < static_cast<int>(m_orbits.size()); ++i)
            if (m_orbits[i] == id)
                m_orbits[i] = INVALID_OBJECT_ID;
    }

    m_ships.erase(id);
    m_fields.erase(id);
    m_buildings.erase(id);
    m_objects.erase(id);

    if (removed_fleet) {
        if (TemporaryPtr<Fleet> fleet = GetFleet(id)) {
            std::vector<TemporaryPtr<Fleet> > fleets;
            fleets.push_back(fleet);
            FleetsRemovedSignal(fleets);
        }
    }
    StateChangedSignal();
}

void System::SetStarType(StarType type) {
    m_star = type;
    if (m_star <= INVALID_STAR_TYPE || NUM_STAR_TYPES <= m_star)
        Logger().errorStream() << "System::SetStarType set star type to " << boost::lexical_cast<std::string>(type);
    StateChangedSignal();
}

void System::AddStarlane(int id) {
    if (!HasStarlaneTo(id) && id != this->ID()) {
        m_starlanes_wormholes[id] = false;
        StateChangedSignal();
        if (GetOptionsDB().Get<bool>("verbose-logging"))
            Logger().debugStream() << "Added starlane from system " << this->Name() << " (" << this->ID() << ") system " << id;
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
    if (retval = HasStarlaneTo(id)) {
        m_starlanes_wormholes.erase(id);
        StateChangedSignal();
    }
    return retval;
}

bool System::RemoveWormhole(int id) {
    bool retval = false;
    if (retval = HasWormholeTo(id)) {
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
        stealth->AddToCurrent(0.01f);
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
    for (std::map<int, bool>::const_iterator it = m_starlanes_wormholes.begin();
         it != m_starlanes_wormholes.end(); ++it)
    {
        int lane_end_sys_id = it->first;
        if (universe.GetObjectVisibilityByEmpire(lane_end_sys_id, empire_id) >= VIS_PARTIAL_VISIBILITY)
            retval[lane_end_sys_id] = it->second;
    }


    // early exit check... can't see any more lanes than exist, so don't need to check for more if all lanes are already visible
    if (retval == m_starlanes_wormholes)
        return retval;


    // check if any fleets owned by empire are moving along a starlane connected to this system...

    // get moving fleets owned by empire
    std::vector<TemporaryPtr<const Fleet> > moving_empire_fleets;
    std::vector<TemporaryPtr<const UniverseObject> > moving_fleet_objects = objects.FindObjects(MovingFleetVisitor());
    for (std::vector<TemporaryPtr<const UniverseObject> >::const_iterator
         it = moving_fleet_objects.begin(); it != moving_fleet_objects.end(); ++it) 
    {
        if (TemporaryPtr<const Fleet> fleet = universe_object_ptr_cast<const Fleet>(*it))
            if (fleet->OwnedBy(empire_id))
                moving_empire_fleets.push_back(fleet);
    }

    // add any lanes an owned fleet is moving along that connect to this system
    for (std::vector<TemporaryPtr<const Fleet> >::const_iterator it = moving_empire_fleets.begin();
         it != moving_empire_fleets.end(); ++it)
    {
        TemporaryPtr<const Fleet> fleet = *it;
        if (fleet->SystemID() != INVALID_OBJECT_ID) {
            Logger().errorStream() << "System::VisibleStarlanesWormholes somehow got a moving fleet that had a valid system id?";
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
            std::map<int, bool>::const_iterator lane_it = m_starlanes_wormholes.find(other_lane_end_sys_id);
            if (lane_it == m_starlanes_wormholes.end()) {
                Logger().errorStream() << "System::VisibleStarlanesWormholes found an owned fleet moving along a starlane connected to this system that isn't also connected to one of this system's starlane-connected systems...?";
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

// free functions

double SystemRadius()
{ return 1000.0 + 50.0; }

double StarRadius()
{ return 80.0; }

double OrbitalRadius(unsigned int orbit) {
    assert(orbit < 10);
    return (SystemRadius() - 50.0) / 10 * (orbit + 1) - 20.0;
}

double StarlaneEntranceOrbitalRadius()
{ return SystemRadius() - StarlaneEntranceRadialAxis(); }

double StarlaneEntranceRadialAxis()
{ return 40.0; }

double StarlaneEntranceTangentAxis()
{ return 80.0; }

double StarlaneEntranceOrbitalPosition(int from_system, int to_system) {
    TemporaryPtr<const System> system_1 = GetSystem(from_system);
    TemporaryPtr<const System> system_2 = GetSystem(to_system);
    if (!system_1 || !system_2) {
        Logger().errorStream() << "StarlaneEntranceOrbitalPosition passed invalid system id";
        return 0.0;
    }
    return std::atan2(system_2->Y() - system_1->Y(), system_2->X() - system_1->X());
}

bool PointInStarlaneEllipse(double x, double y, int from_system, int to_system) {
    double rads = StarlaneEntranceOrbitalPosition(from_system, to_system);
    double ellipse_x = StarlaneEntranceOrbitalRadius() * std::cos(rads);
    double ellipse_y = StarlaneEntranceOrbitalRadius() * std::sin(rads);
    return PointInEllipse(x, y, ellipse_x, ellipse_y,
                          StarlaneEntranceRadialAxis(), StarlaneEntranceTangentAxis(),
                          rads);
}
