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
#include "../util/GameRuleRanks.h"
#include "../util/i18n.h"


namespace {
    void AddRules(GameRules& rules) {
        // whether systems detected only at basic visibility have their name and star type revealed or not
        rules.Add<bool>(UserStringNop("RULE_BASIC_VIS_SYSTEM_INFO_SHOWN"),
                        UserStringNop("RULE_BASIC_VIS_SYSTEM_INFO_SHOWN_DESC"),
                        GameRuleCategories::GameRuleCategory::GENERAL, false, true,
                        GameRuleRanks::RULE_BASIC_VIS_SYSTEM_INFO_SHOWN_RANK);
    }
    bool temp_bool = RegisterGameRules(&AddRules);
}

namespace {
    constexpr auto first_orbit = System::NO_ORBIT + 1;
    static_assert(first_orbit == 0);
}

System::System(StarType star, std::string name, double x, double y, int current_turn) :
    UniverseObject{UniverseObjectType::OBJ_SYSTEM, std::move(name), x, y, ALL_EMPIRES, current_turn},
    m_star(star)
{
    if (m_star < StarType::INVALID_STAR_TYPE || StarType::NUM_STAR_TYPES < m_star)
        m_star = StarType::INVALID_STAR_TYPE;

    m_orbits.assign(SYSTEM_ORBITS, INVALID_OBJECT_ID);
}

std::shared_ptr<UniverseObject> System::Clone(const Universe& universe, int empire_id) const {
    const Visibility vis = empire_id == ALL_EMPIRES ?
        Visibility::VIS_FULL_VISIBILITY : universe.GetObjectVisibilityByEmpire(this->ID(), empire_id);

    if (!(vis >= Visibility::VIS_BASIC_VISIBILITY && vis <= Visibility::VIS_FULL_VISIBILITY))
        return nullptr;

    auto retval = std::make_shared<System>();
    retval->Copy(*this, universe, empire_id);
    return retval;
}

void System::Copy(const UniverseObject& copied_object, const Universe& universe, int empire_id) {
    if (&copied_object == this)
        return;
    if (copied_object.ObjectType() != UniverseObjectType::OBJ_SYSTEM) {
        ErrorLogger() << "System::Copy passed an object that wasn't a System";
        return;
    }

    Copy(static_cast<const System&>(copied_object), universe, empire_id);
}

void System::Copy(const System& copied_system, const Universe& universe, int empire_id) {
    if (&copied_system == this)
        return;

    const int copied_object_id = copied_system.ID();
    const Visibility vis = empire_id == ALL_EMPIRES ?
        Visibility::VIS_FULL_VISIBILITY : universe.GetObjectVisibilityByEmpire(copied_object_id, empire_id);
    const auto visible_specials = universe.GetObjectVisibleSpecialsByEmpire(copied_object_id, empire_id);

    UniverseObject::Copy(copied_system, vis, visible_specials, universe);

    if (vis >= Visibility::VIS_BASIC_VISIBILITY) {
        // copy visible info of visible contained objects
        this->m_objects = copied_system.VisibleContainedObjectIDs(
            empire_id, universe.GetEmpireObjectVisibility());

        // only copy orbit info for visible planets
        auto orbits_size = m_orbits.size();
        m_orbits.clear();
        m_orbits.assign(orbits_size, INVALID_OBJECT_ID);
        for (std::size_t o = 0u; o < copied_system.m_orbits.size(); ++o) {
            int planet_id = copied_system.m_orbits[o];
            if (m_objects.contains(planet_id))
                m_orbits[o] = planet_id;
        }

        // copy visible contained object per-type info
        m_planets.clear();
        for (int planet_id : copied_system.m_planets) {
            if (m_objects.contains(planet_id))
                m_planets.insert(planet_id);
        }

        m_buildings.clear();
        for (int building_id : copied_system.m_buildings) {
            if (m_objects.contains(building_id))
                m_buildings.insert(building_id);
        }

        m_fleets.clear();
        for (int fleet_id : copied_system.m_fleets) {
            if (m_objects.contains(fleet_id))
                m_fleets.insert(fleet_id);
        }

        m_ships.clear();
        for (int ship_id : copied_system.m_ships) {
            if (m_objects.contains(ship_id))
                m_ships.insert(ship_id);
        }

        m_fields.clear();
        for (int field_id : copied_system.m_fields) {
            if (m_objects.contains(field_id))
                m_fields.insert(field_id);
        }


        if (vis >= Visibility::VIS_PARTIAL_VISIBILITY) {
            this->m_name =                  copied_system.m_name;
            this->m_star =                  copied_system.m_star;
            this->m_last_turn_battle_here = copied_system.m_last_turn_battle_here;

            // update lanes to be just those that are visible, erasing any
            // previously known that aren't visible now, as these are thus
            // known not to exist any more
            this->m_starlanes = copied_system.VisibleStarlanes(empire_id, universe);

        } else {
            if (GetGameRules().Get<bool>("RULE_BASIC_VIS_SYSTEM_INFO_SHOWN")) {
                this->m_name = copied_system.m_name;
                this->m_star = copied_system.m_star;
            }

            // add any visible lanes, without removing existing entries
            this->m_starlanes.merge(copied_system.VisibleStarlanes(empire_id, universe));
        }
    }
}

std::string System::Dump(uint8_t ntabs) const {
    std::string retval = UniverseObject::Dump(ntabs);
    retval.reserve(2048); // guesstimate
    retval.append(" star type: ").append(to_string(m_star))
          .append("  last combat on turn: ").append(std::to_string(m_last_turn_battle_here))
          .append("  total orbits: ").append(std::to_string(m_orbits.size()));

    if (!m_orbits.empty()) {
        retval.append("  objects per orbit: ");

        int orbit_index = first_orbit;
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
    for (auto it = m_starlanes.begin(); it != m_starlanes.end();) {
        const int lane_end_id = *it;
        ++it;
        retval.append(std::to_string(lane_end_id)).append(it == m_starlanes.end() ? "" : ", ");
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

std::string System::ApparentName(int empire_id, const Universe& u, bool blank_unexplored_and_none) const {
    // this one line requires a higher __GNUC__ version to compile for several Docker / Fedora test builds. No idea why it's different from the other similar cases.
#if defined(__cpp_lib_constexpr_string) && (!defined(__GNUC__) || (__GNUC__ > 12) || (__GNUC__ == 12 && __GNU_MINOR__ >= 2)) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934))) && ((!defined(__clang_major__) || (__clang_major__ >= 10)))
    static constexpr std::string EMPTY_STRING;
#else
    static const std::string EMPTY_STRING;
#endif

    const ObjectMap& o = u.Objects();

    if (empire_id == ALL_EMPIRES)
        return this->PublicName(empire_id, u);

    // has the indicated empire ever detected this system?
    const auto& vtm = u.GetObjectVisibilityTurnMapByEmpire(this->ID(), empire_id);
    if (!vtm.contains(Visibility::VIS_PARTIAL_VISIBILITY)) {
        if (blank_unexplored_and_none)
            return EMPTY_STRING;

        if (m_star == StarType::INVALID_STAR_TYPE)
            return m_name + UserString("UNEXPLORED_REGION");
        else
            return m_name + UserString("UNEXPLORED_SYSTEM");
    }

    if (m_star == StarType::STAR_NONE) {
        // determine if there are any planets in the system
        for (const auto& entry : o.allExisting<Planet>()) {
            if (entry.second->SystemID() == this->ID())
                return this->PublicName(empire_id, u);
        }
        if (blank_unexplored_and_none) {
            //DebugLogger() << "System::ApparentName No-Star System (" << ID() << "), returning name "<< EMPTY_STRING;
            return EMPTY_STRING;
        }
        return m_name + UserString("EMPTY_SPACE");
    }

    return this->PublicName(empire_id, u);
}

StarType System::NextOlderStarType() const noexcept {
    if (m_star <= StarType::INVALID_STAR_TYPE || m_star >= StarType::NUM_STAR_TYPES)
        return StarType::INVALID_STAR_TYPE;
    if (m_star >= StarType::STAR_RED)
        return m_star;                  // STAR_RED, STAR_NEUTRON, STAR_BLACK, STAR_NONE
    return StarType(int(m_star) + 1);   // STAR_BLUE -> STAR_WHITE -> STAR_YELLOW -> STAR_ORANGE -> STAR_RED
}

StarType System::NextYoungerStarType() const noexcept {
    if (m_star <= StarType::INVALID_STAR_TYPE || m_star >= StarType::NUM_STAR_TYPES)
        return StarType::INVALID_STAR_TYPE;
    if (m_star > StarType::STAR_RED)
        return m_star;                  // STAR_NEUTRON, STAR_BLACK, STAR_NONE
    if (m_star <= StarType::STAR_BLUE)
        return StarType::STAR_BLUE;     // STAR_BLUE
    return StarType(int(m_star) - 1);   // STAR_BLUE <- STAR_WHITE <- STAR_YELLOW <- STAR_ORANGE <- STAR_RED
}

bool System::HasStarlaneTo(int id) const
{ return m_starlanes.contains(id); }

bool System::Contains(int object_id) const {
    if (object_id == INVALID_OBJECT_ID)
        return false;
    return m_objects.contains(object_id);
}

std::size_t System::SizeInMemory() const {
    std::size_t retval = UniverseObject::SizeInMemory();
    retval += sizeof(System) - sizeof(UniverseObject);

    retval += sizeof(decltype(m_orbits)::value_type)*m_orbits.capacity();
    retval += sizeof(decltype(m_objects)::value_type)*m_objects.capacity();
    retval += sizeof(decltype(m_planets)::value_type)*m_planets.capacity();
    retval += sizeof(decltype(m_buildings)::value_type)*m_buildings.capacity();
    retval += sizeof(decltype(m_fleets)::value_type)*m_fleets.capacity();
    retval += sizeof(decltype(m_ships)::value_type)*m_ships.capacity();
    retval += sizeof(decltype(m_fields)::value_type)*m_fields.capacity();
    retval += sizeof(decltype(m_starlanes)::value_type)*m_starlanes.capacity();
    retval += sizeof(decltype(m_overlay_texture)::value_type)*m_overlay_texture.capacity();

    return retval;
}

void System::Insert(std::shared_ptr<UniverseObject> obj, int orbit, int current_turn, const ObjectMap& objects)
{ Insert(obj.get(), orbit, current_turn, objects); }

void System::Insert(UniverseObject* obj, int orbit, int current_turn, const ObjectMap& objects) {
    if (!obj) {
        ErrorLogger() << "System::Insert() : Attempted to place a null object in a System";
        return;
    }
    if (orbit < NO_ORBIT || orbit >= static_cast<int>(m_orbits.size())) {
        ErrorLogger() << "System::Insert() : Attempted to place an object in invalid orbit";
        return;
    }

    obj->MoveTo(this->X(), this->Y());
    obj->SetSystem(this->ID());

    if (obj->ObjectType() == UniverseObjectType::OBJ_PLANET) {
        if (orbit == NO_ORBIT) {
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
                for (int o = first_orbit; o < static_cast<int>(m_orbits.size()); ++o) {
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
                for (int o = first_orbit; o < static_cast<int>(m_orbits.size()); ++o) {
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
                const auto& free_orbits = FreeOrbits();
                if (!free_orbits.empty() && OrbitOfPlanet(obj->ID()) == NO_ORBIT) {
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
        if (auto ship = static_cast<Ship*>(obj))
            ship->SetArrivedOnTurn(current_turn);
        break;
    }
    case UniverseObjectType::OBJ_FLEET: {
        m_fleets.insert(obj->ID());
        FleetsInsertedSignal({obj->ID()}, objects);
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

    if (removed_fleet)
        FleetsRemovedSignal(std::vector<int>{id});
    StateChangedSignal();
}

void System::SetStarType(StarType type) {
    m_star = type;
    if (m_star <= StarType::INVALID_STAR_TYPE || StarType::NUM_STAR_TYPES <= m_star)
        ErrorLogger() << "System::SetStarType set star type to " << type;
    StateChangedSignal();
}

void System::AddStarlane(int id) {
    const auto added = m_starlanes.insert(id).second;
    if (added) {
        StateChangedSignal();
        TraceLogger() << "Added starlane from system " << this->Name() << " (" << this->ID() << ") system " << id;
    }
}

bool System::RemoveStarlane(int id) {
    const auto erased_count = m_starlanes.erase(id);
    if (erased_count > 0)
        StateChangedSignal();
    return erased_count > 0;
}

void System::ResetTargetMaxUnpairedMeters() {
    UniverseObject::ResetTargetMaxUnpairedMeters();

    static_assert(noexcept(GetMeter(MeterType::METER_STEALTH)->ResetCurrent()));
    // give systems base stealth slightly above zero, so that they can't be
    // seen from a distance without high detection ability
    if (Meter* stealth = GetMeter(MeterType::METER_STEALTH)) {
        stealth->ResetCurrent();
        //stealth->AddToCurrent(0.01f);
    }
}

bool System::OrbitOccupied(int orbit) const {
    if (orbit < first_orbit || orbit >= static_cast<int>(m_orbits.size()))
        return false;
    return m_orbits[orbit] != INVALID_OBJECT_ID;
}

int System::PlanetInOrbit(int orbit) const {
    if (orbit < first_orbit || orbit >= static_cast<int>(m_orbits.size()))
        return INVALID_OBJECT_ID;
    return m_orbits[orbit];
}

int System::OrbitOfPlanet(int object_id) const {
    if (object_id == INVALID_OBJECT_ID)
        return NO_ORBIT;
    for (int o = first_orbit; o < static_cast<int>(m_orbits.size()); ++o)
        if (m_orbits[o] == object_id)
            return o;
    return NO_ORBIT;
}

std::set<int> System::FreeOrbits() const { // TODO: return something better
    std::set<int> retval;
    for (int o = first_orbit; o < static_cast<int>(m_orbits.size()); ++o)
        if (m_orbits[o] == INVALID_OBJECT_ID)
            retval.insert(o);
    return retval;
}

System::IDSet System::VisibleStarlanes(int empire_id, const Universe& universe) const {
    if (empire_id == ALL_EMPIRES)
        return m_starlanes;

    const ObjectMap& objects = universe.Objects();
    const Visibility this_system_vis = universe.GetObjectVisibilityByEmpire(this->ID(), empire_id);

    //visible starlanes are:
    //  - those connected to systems with vis >= partial
    //  - those with visible ships travelling along them


    // return all starlanes if partially visible or better
    if (this_system_vis >= Visibility::VIS_PARTIAL_VISIBILITY)
        return m_starlanes;


    // compile visible lanes connected to this only basically-visible system
    // by checking if any of the adjacent systems are partial or better visible
    System::IDSet retval;
    retval.reserve(m_starlanes.size());
    std::copy_if(m_starlanes.begin(), m_starlanes.end(), std::inserter(retval, retval.end()),
                 [&universe, empire_id](auto lane_end_sys_id) {
                     return universe.GetObjectVisibilityByEmpire(lane_end_sys_id, empire_id) >=
                        Visibility::VIS_PARTIAL_VISIBILITY;
                  });

    // early exit check... can't see any more lanes than exist, so don't need to check for more if all lanes are already visible
    if (retval == m_starlanes)
        return retval;


    // check if any fleets owned by empire are moving along a starlane connected to this system...

    // get moving fleets owned by empire
    auto is_owned_moving_fleet = [empire_id](const Fleet* fleet) {
        return fleet->FinalDestinationID() != INVALID_OBJECT_ID &&
            fleet->SystemID() == INVALID_OBJECT_ID &&
            fleet->OwnedBy(empire_id);
    };

    // add any lanes an owned fleet is moving along that connect to this system
    for (auto* fleet : objects.findRaw<const Fleet>(is_owned_moving_fleet)) {
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
            if (m_starlanes.contains(other_lane_end_sys_id))
                retval.insert(other_lane_end_sys_id);
            else
                ErrorLogger() << "System::VisibleStarlanesWormholes found an owned fleet moving along a starlane connected to this system that isn't also connected to one of this system's starlane-connected systems...?";
        }
    }

    return retval;
}

void System::SetOverlayTexture(const std::string& texture, double size) {
    m_overlay_texture = texture;
    m_overlay_size = size;
    StateChangedSignal();
}
