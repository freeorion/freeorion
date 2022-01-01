#include "System.h"

#include "Building.h"
#include "Fleet.h"
#include "Planet.h"
#include "Ship.h"
#include "Universe.h"
#include "UniverseObjectVisitors.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Empire.h"
#include "../util/AppInterface.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/GameRules.h"
#include "../util/i18n.h"


namespace {
    void AddRules(GameRules& rules) {
        // makes all PRNG be reseeded frequently
        rules.Add<bool>(UserStringNop("RULE_BASIC_VIS_SYSTEM_INFO_SHOWN"),
                        UserStringNop("RULE_BASIC_VIS_SYSTEM_INFO_SHOWN_DESC"),
                        "", false, true);
    }
    bool temp_bool = RegisterGameRules(&AddRules);
}


System::System(StarType star, const std::string& name, double x, double y) :
    UniverseObject(name, x, y),
    m_star(star)
{
    if (m_star < StarType::INVALID_STAR_TYPE || StarType::NUM_STAR_TYPES < m_star)
        m_star = StarType::INVALID_STAR_TYPE;

    m_orbits.assign(SYSTEM_ORBITS, INVALID_OBJECT_ID);

    UniverseObject::Init();
}

System* System::Clone(const Universe& universe, int empire_id) const {
    Visibility vis = universe.GetObjectVisibilityByEmpire(this->ID(), empire_id);

    if (!(vis >= Visibility::VIS_BASIC_VISIBILITY && vis <= Visibility::VIS_FULL_VISIBILITY))
        return nullptr;

    auto retval = std::make_unique<System>();
    retval->Copy(shared_from_this(), universe, empire_id);
    return retval.release();
}

void System::Copy(std::shared_ptr<const UniverseObject> copied_object,
                  const Universe& universe, int empire_id)
{
    if (copied_object.get() == this)
        return;
    std::shared_ptr<const System> copied_system = std::dynamic_pointer_cast<const System>(copied_object);
    if (!copied_system) {
        ErrorLogger() << "System::Copy passed an object that wasn't a System";
        return;
    }

    int copied_object_id = copied_object->ID();
    Visibility vis = universe.GetObjectVisibilityByEmpire(copied_object_id, empire_id);
    auto visible_specials = universe.GetObjectVisibleSpecialsByEmpire(copied_object_id, empire_id);

    UniverseObject::Copy(std::move(copied_object), vis, visible_specials, universe);

    if (vis >= Visibility::VIS_BASIC_VISIBILITY) {
        if (GetGameRules().Get<bool>("RULE_BASIC_VIS_SYSTEM_INFO_SHOWN")) {
            this->m_name = copied_system->m_name;
            this->m_star = copied_system->m_star;
        }

        // copy visible info of visible contained objects
        this->m_objects = copied_system->VisibleContainedObjectIDs(empire_id);

        // only copy orbit info for visible planets
        size_t orbits_size = m_orbits.size();
        m_orbits.clear();
        m_orbits.assign(orbits_size, INVALID_OBJECT_ID);
        for (std::size_t o = 0; o < copied_system->m_orbits.size(); ++o) {
            int planet_id = copied_system->m_orbits[o];
            if (m_objects.count(planet_id))
                m_orbits[o] = planet_id;
        }

        // copy visible contained object per-type info
        m_planets.clear();
        for (int planet_id : copied_system->m_planets) {
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


        if (vis >= Visibility::VIS_PARTIAL_VISIBILITY) {
            this->m_name =                  copied_system->m_name;
            this->m_star =                  copied_system->m_star;
            this->m_last_turn_battle_here = copied_system->m_last_turn_battle_here;

            // update lanes to be just those that are visible, erasing any
            // previously known that aren't visible now, as these are thus
            // known not to exist any more
            this->m_starlanes_wormholes = copied_system->VisibleStarlanesWormholes(empire_id, universe);

        } else {
            // add any visible lanes, without removing existing entries
            for (const auto& [lane_id, lane_or_hole] :
                 copied_system->VisibleStarlanesWormholes(empire_id, universe))
            { this->m_starlanes_wormholes[lane_id] = lane_or_hole; }
        }
    }
}

UniverseObjectType System::ObjectType() const
{ return UniverseObjectType::OBJ_SYSTEM; }

std::string System::Dump(unsigned short ntabs) const {
    std::string retval = UniverseObject::Dump(ntabs);
    retval.reserve(2048);
    retval.append(" star type: ").append(to_string(m_star))
          .append("  last combat on turn: ").append(std::to_string(m_last_turn_battle_here))
          .append("  total orbits: ").append(std::to_string(m_orbits.size()));

    if (m_orbits.size() > 0) {
        retval.append("  objects per orbit: ");

        int orbit_index = 0;
        for (auto it = m_orbits.begin(); it != m_orbits.end();) {
            retval.append("[").append(std::to_string(orbit_index)).append("]")
                  .append(std::to_string(*it));
            ++it;
            if (it != m_orbits.end())
                retval.append(", ");
            ++orbit_index;
        }
    }

    retval.append("  starlanes: ");
    for (auto it = m_starlanes_wormholes.begin(); it != m_starlanes_wormholes.end();) {
        int lane_end_id = it->first;
        ++it;
        retval.append(std::to_string(lane_end_id)).append(it == m_starlanes_wormholes.end() ? "" : ", ");
    }

    retval.append("  objects: ");
    for (auto it = m_objects.begin(); it != m_objects.end();) {
        int obj_id = *it;
        ++it;
        if (obj_id != INVALID_OBJECT_ID)
            retval.append(std::to_string(obj_id)).append(it == m_objects.end() ? "" : ", ");
    }
    return retval;
}

const std::string& System::ApparentName(int empire_id, const Universe& u,
                                        bool blank_unexplored_and_none) const
{
    static const std::string EMPTY_STRING;

    const ObjectMap& o = u.Objects();

    if (empire_id == ALL_EMPIRES)
        return this->PublicName(empire_id, u);

    // has the indicated empire ever detected this system?
    const auto& vtm = u.GetObjectVisibilityTurnMapByEmpire(this->ID(), empire_id);
    if (!vtm.count(Visibility::VIS_PARTIAL_VISIBILITY)) {
        if (blank_unexplored_and_none)
            return EMPTY_STRING;

        if (m_star == StarType::INVALID_STAR_TYPE)
            return UserString("UNEXPLORED_REGION");
        else
            return UserString("UNEXPLORED_SYSTEM");
    }

    if (m_star == StarType::STAR_NONE) {
        // determine if there are any planets in the system
        for (const auto& entry : o.ExistingPlanets()) {
            if (entry.second->SystemID() == this->ID())
                return this->PublicName(empire_id, u);
        }
        if (blank_unexplored_and_none) {
            //DebugLogger() << "System::ApparentName No-Star System (" << ID() << "), returning name "<< EMPTY_STRING;
            return EMPTY_STRING;
        }
        //DebugLogger() << "System::ApparentName No-Star System (" << ID() << "), returning name "<< UserString("EMPTY_SPACE");
        return UserString("EMPTY_SPACE");
    }

    return this->PublicName(empire_id, u); // todo get Objects from inputs
}

StarType System::NextOlderStarType() const {
    if (m_star <= StarType::INVALID_STAR_TYPE || m_star >= StarType::NUM_STAR_TYPES)
        return StarType::INVALID_STAR_TYPE;
    if (m_star >= StarType::STAR_RED)
        return m_star;                  // STAR_RED, STAR_NEUTRON, STAR_BLACK, STAR_NONE
    return StarType(int(m_star) + 1);   // STAR_BLUE -> STAR_WHITE -> STAR_YELLOW -> STAR_ORANGE -> STAR_RED
}

StarType System::NextYoungerStarType() const {
    if (m_star <= StarType::INVALID_STAR_TYPE || m_star >= StarType::NUM_STAR_TYPES)
        return StarType::INVALID_STAR_TYPE;
    if (m_star > StarType::STAR_RED)
        return m_star;                  // STAR_NEUTRON, STAR_BLACK, STAR_NONE
    if (m_star <= StarType::STAR_BLUE)
        return StarType::STAR_BLUE;     // STAR_BLUE
    return StarType(int(m_star) - 1);   // STAR_BLUE <- STAR_WHITE <- STAR_YELLOW <- STAR_ORANGE <- STAR_RED
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

int System::EffectiveOwner(const ObjectMap& objects) const {
    // Check if all of the owners are the same empire.
    int first_owner_found = ALL_EMPIRES;
    for (const auto& planet : objects.find<Planet>(m_planets)) {
        const int owner = planet->Owner();
        if (owner == ALL_EMPIRES)
            continue;
        if (first_owner_found == ALL_EMPIRES)
            first_owner_found = owner;
        if (first_owner_found != owner)
            return ALL_EMPIRES;
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

    if (obj->ObjectType() == UniverseObjectType::OBJ_PLANET) {
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
    case UniverseObjectType::OBJ_SHIP: {
        m_ships.insert(obj->ID());
        if (auto ship = std::dynamic_pointer_cast<Ship>(obj))
            ship->SetArrivedOnTurn(CurrentTurn());
        break;
    }
    case UniverseObjectType::OBJ_FLEET: {
        m_fleets.insert(obj->ID());
        FleetsInsertedSignal({std::dynamic_pointer_cast<Fleet>(obj)});
        break;
    }
    case UniverseObjectType::OBJ_PLANET:
        m_planets.insert(obj->ID());
        break;
    case UniverseObjectType::OBJ_FIELD:
        m_fields.insert(obj->ID());
        break;
    case UniverseObjectType::OBJ_SYSTEM:
        ErrorLogger() << "System::Insert inserting a system into another system...??";
        break;
    case UniverseObjectType::OBJ_BUILDING:
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
        if (auto fleet = Objects().get<Fleet>(id))
            FleetsRemovedSignal({fleet});
    }
    StateChangedSignal();
}

void System::SetStarType(StarType type) {
    m_star = type;
    if (m_star <= StarType::INVALID_STAR_TYPE || StarType::NUM_STAR_TYPES <= m_star)
        ErrorLogger() << "System::SetStarType set star type to " << type;
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
    if (Meter* stealth = GetMeter(MeterType::METER_STEALTH)) {
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

std::map<int, bool> System::VisibleStarlanesWormholes(int empire_id, const Universe& universe) const {
    if (empire_id == ALL_EMPIRES)
        return m_starlanes_wormholes;

    const ObjectMap& objects = universe.Objects();
    Visibility this_system_vis = universe.GetObjectVisibilityByEmpire(this->ID(), empire_id);

    //visible starlanes are:
    //  - those connected to systems with vis >= partial
    //  - those with visible ships travelling along them


    // return all starlanes if partially visible or better
    if (this_system_vis >= Visibility::VIS_PARTIAL_VISIBILITY)
        return m_starlanes_wormholes;


    // compile visible lanes connected to this only basically-visible system
    std::map<int, bool> retval;


    // check if any of the adjacent systems are partial or better visible
    for (const auto& entry : m_starlanes_wormholes) {
        int lane_end_sys_id = entry.first;
        if (universe.GetObjectVisibilityByEmpire(lane_end_sys_id, empire_id) >= Visibility::VIS_PARTIAL_VISIBILITY)
            retval[lane_end_sys_id] = entry.second;
    }


    // early exit check... can't see any more lanes than exist, so don't need to check for more if all lanes are already visible
    if (retval == m_starlanes_wormholes)
        return retval;


    // check if any fleets owned by empire are moving along a starlane connected to this system...

    // get moving fleets owned by empire
    std::vector<const Fleet*> moving_empire_fleets;
    moving_empire_fleets.reserve(objects.size<Fleet>());
    static const MovingFleetVisitor moving_fleet_visitor;
    for (auto& object : objects.find(moving_fleet_visitor)) {
        if (object && object->ObjectType() == UniverseObjectType::OBJ_FLEET && object->OwnedBy(empire_id))
            moving_empire_fleets.emplace_back(static_cast<const Fleet*>(object.get()));
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
