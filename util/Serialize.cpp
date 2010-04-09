#include "Serialize.h"

#include "../combat/OpenSteer/AsteroidBeltObstacle.h"
#include "../combat/OpenSteer/CombatShip.h"
#include "../combat/OpenSteer/CombatFighter.h"
#include "../combat/OpenSteer/Missile.h"
#include "../combat/OpenSteer/Obstacle.h"
#include "../combat/OpenSteer/PathingEngine.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../universe/Building.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/Planet.h"
#include "../universe/ShipDesign.h"
#include "../universe/System.h"
#include "../util/OrderSet.h"


#if defined(_MSC_VER) && defined(int64_t)
#undef int64_t
#endif

#include <boost/static_assert.hpp>
#include <boost/detail/endian.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/weak_ptr.hpp>
#include <boost/ptr_container/serialize_ptr_vector.hpp>


// exports for boost serialization of polymorphic UniverseObject hierarchy
BOOST_CLASS_EXPORT(System)
BOOST_CLASS_EXPORT(Planet)
BOOST_CLASS_EXPORT(Building)
BOOST_CLASS_EXPORT(Fleet)
BOOST_CLASS_EXPORT(Ship)

// exports for boost serialization of polymorphic Order hierarchy
BOOST_CLASS_EXPORT(RenameOrder)
BOOST_CLASS_EXPORT(NewFleetOrder)
BOOST_CLASS_EXPORT(FleetMoveOrder)
BOOST_CLASS_EXPORT(FleetTransferOrder)
BOOST_CLASS_EXPORT(FleetColonizeOrder)
BOOST_CLASS_EXPORT(DeleteFleetOrder)
BOOST_CLASS_EXPORT(ChangeFocusOrder)
BOOST_CLASS_EXPORT(ResearchQueueOrder)
BOOST_CLASS_EXPORT(ProductionQueueOrder)
BOOST_CLASS_EXPORT(ShipDesignOrder)
BOOST_CLASS_EXPORT(ScrapOrder)

// exports for boost serialization of PathingEngine-related classes
BOOST_CLASS_EXPORT(CombatShip)
BOOST_CLASS_EXPORT(CombatFighter)
BOOST_CLASS_EXPORT(Missile)
BOOST_CLASS_EXPORT(OpenSteer::SphereObstacle)
BOOST_CLASS_EXPORT(OpenSteer::BoxObstacle)
BOOST_CLASS_EXPORT(OpenSteer::PlaneObstacle)
BOOST_CLASS_EXPORT(OpenSteer::RectangleObstacle)
BOOST_CLASS_EXPORT(AsteroidBeltObstacle)

// some endianness and size checks to ensure portability of binary save files;
// of one or more of these fails, it means that FreeOrion is not supported on
// your platform/compiler pair, and must be modified to provide data of the
// appropriate size(s).
#ifndef BOOST_LITTLE_ENDIAN
#  error "Incompatible endianness for binary serialization."
#endif
BOOST_STATIC_ASSERT(sizeof(char) == 1);
BOOST_STATIC_ASSERT(sizeof(short) == 2);
BOOST_STATIC_ASSERT(sizeof(int) == 4);
BOOST_STATIC_ASSERT(sizeof(long long) == 8);
BOOST_STATIC_ASSERT(sizeof(float) == 4);
BOOST_STATIC_ASSERT(sizeof(double) == 8);

// This is commented out, but left here by way of explanation.  This assert is
// the only one that seems to fail on 64-bit systems.  It would seem that
// short of writing some Boost.Serialization archive that handles longs
// portably, we cannot transmit longs across machines with different bit-size
// architectures.  So, don't use longs -- use long longs instead if you need
// something bigger than an int for some reason.
//BOOST_STATIC_ASSERT(sizeof(long) == 4);

// These implementations of the serialize() member should (and, now, can) only
// be used from within this file.  Putting them here should dramatically
// reduce compile times when tweaking them.

template <class Archive>
void ResearchQueue::Element::serialize(Archive& ar, const unsigned int version)
{
    std::string tech_name;
    if (Archive::is_saving::value) {
        assert(tech);
        tech_name = tech->Name();
    }
    ar  & BOOST_SERIALIZATION_NVP(tech_name)
        & BOOST_SERIALIZATION_NVP(allocated_rp)
        & BOOST_SERIALIZATION_NVP(turns_left);
    if (Archive::is_loading::value) {
        assert(tech_name != "");
        tech = GetTech(tech_name);
    }
}

template <class Archive>
void ResearchQueue::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_queue)
        & BOOST_SERIALIZATION_NVP(m_projects_in_progress)
        & BOOST_SERIALIZATION_NVP(m_total_RPs_spent);
}

template <class Archive>
void ProductionQueue::ProductionItem::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(build_type)
        & BOOST_SERIALIZATION_NVP(name)
        & BOOST_SERIALIZATION_NVP(design_id);
}

template <class Archive>
void ProductionQueue::Element::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(item)
        & BOOST_SERIALIZATION_NVP(ordered)
        & BOOST_SERIALIZATION_NVP(remaining)
        & BOOST_SERIALIZATION_NVP(location)
        & BOOST_SERIALIZATION_NVP(allocated_pp)
        & BOOST_SERIALIZATION_NVP(turns_left_to_next_item)
        & BOOST_SERIALIZATION_NVP(turns_left_to_completion);
}

template <class Archive>
void ProductionQueue::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_queue)
        & BOOST_SERIALIZATION_NVP(m_projects_in_progress)
        & BOOST_SERIALIZATION_NVP(m_system_group_allocated_pp);
}

template <class Archive>
void Empire::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_id)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_player_name)
        & BOOST_SERIALIZATION_NVP(m_color);
    if (Universe::ALL_OBJECTS_VISIBLE ||
        Universe::s_encoding_empire == ALL_EMPIRES || m_id == Universe::s_encoding_empire) {
        ar  & BOOST_SERIALIZATION_NVP(m_homeworld_id)
            & BOOST_SERIALIZATION_NVP(m_capitol_id)
            & BOOST_SERIALIZATION_NVP(m_techs)
            & BOOST_SERIALIZATION_NVP(m_research_queue)
            & BOOST_SERIALIZATION_NVP(m_research_progress)
            & BOOST_SERIALIZATION_NVP(m_production_queue)
            & BOOST_SERIALIZATION_NVP(m_production_progress)
            & BOOST_SERIALIZATION_NVP(m_available_building_types)
            & BOOST_SERIALIZATION_NVP(m_available_part_types)
            & BOOST_SERIALIZATION_NVP(m_available_hull_types)
            & BOOST_SERIALIZATION_NVP(m_explored_systems)
            & BOOST_SERIALIZATION_NVP(m_ship_designs)
            & BOOST_SERIALIZATION_NVP(m_sitrep_entries)
            & BOOST_SERIALIZATION_NVP(m_resource_pools)
            & BOOST_SERIALIZATION_NVP(m_population_pool)
            & BOOST_SERIALIZATION_NVP(m_maintenance_total_cost)
            & BOOST_SERIALIZATION_NVP(m_ship_names_used);
    }
}

template <class Archive>
void EmpireManager::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_empire_map)
        & BOOST_SERIALIZATION_NVP(m_eliminated_empires);
}

template <class Archive>
void Universe::serialize(Archive& ar, const unsigned int version)
{
    ObjectMap                       objects;
    EmpireObjectMap                 empire_latest_known_objects;
    EmpireObjectVisibilityMap       empire_object_visibility;
    EmpireObjectVisibilityTurnMap   empire_object_visibility_turns;
    ObjectKnowledgeMap              empire_known_destroyed_object_ids;
    ShipDesignMap                   ship_designs;

    if (Archive::is_saving::value) {
        GetObjectsToSerialize(              objects,                            s_encoding_empire);
        GetEmpireKnownObjectsToSerialize(   empire_latest_known_objects,        s_encoding_empire);
        GetEmpireObjectVisibilityMap(       empire_object_visibility,           s_encoding_empire);
        GetEmpireObjectVisibilityTurnMap(   empire_object_visibility_turns,     s_encoding_empire);
        GetEmpireKnownDestroyedObjects(     empire_known_destroyed_object_ids,  s_encoding_empire);
        GetShipDesignsToSerialize(          objects,    ship_designs,           s_encoding_empire);
    }

    ar  & BOOST_SERIALIZATION_NVP(s_universe_width)
        & BOOST_SERIALIZATION_NVP(ship_designs)
        & BOOST_SERIALIZATION_NVP(empire_object_visibility)
        & BOOST_SERIALIZATION_NVP(empire_object_visibility_turns)
        & BOOST_SERIALIZATION_NVP(empire_known_destroyed_object_ids)
        & BOOST_SERIALIZATION_NVP(objects)
        & BOOST_SERIALIZATION_NVP(empire_latest_known_objects)
        & BOOST_SERIALIZATION_NVP(m_last_allocated_object_id)
        & BOOST_SERIALIZATION_NVP(m_last_allocated_design_id);

    if (Archive::is_saving::value) {
        // clean up temporary objects in temporary ObjectMaps
        objects.Clear();
        for (EmpireObjectMap::iterator it = empire_latest_known_objects.begin(); it != empire_latest_known_objects.end(); ++it)
            it->second.Clear();
    }

    if (Archive::is_loading::value) {
        m_objects =                             objects;
        m_empire_latest_known_objects =         empire_latest_known_objects;
        m_empire_object_visibility =            empire_object_visibility;
        m_empire_object_visibility_turns =      empire_object_visibility_turns;
        m_empire_known_destroyed_object_ids =   empire_known_destroyed_object_ids;
        m_ship_designs =                        ship_designs;
        InitializeSystemGraph(s_encoding_empire);
    }
}

template <class Archive>
void UniverseObject::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_id)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_x)
        & BOOST_SERIALIZATION_NVP(m_y)
        & BOOST_SERIALIZATION_NVP(m_owners)
        & BOOST_SERIALIZATION_NVP(m_system_id)
        & BOOST_SERIALIZATION_NVP(m_specials)
        & BOOST_SERIALIZATION_NVP(m_meters)
        & BOOST_SERIALIZATION_NVP(m_created_on_turn);
}

template <class Archive>
void System::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(UniverseObject)
        & BOOST_SERIALIZATION_NVP(m_star)
        & BOOST_SERIALIZATION_NVP(m_orbits)
        & BOOST_SERIALIZATION_NVP(m_objects)
        & BOOST_SERIALIZATION_NVP(m_starlanes_wormholes);
}

template <class Archive>
void Planet::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(UniverseObject)
        & BOOST_SERIALIZATION_BASE_OBJECT_NVP(PopCenter)
        & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ResourceCenter)
        & BOOST_SERIALIZATION_NVP(m_type)
        & BOOST_SERIALIZATION_NVP(m_size)
        & BOOST_SERIALIZATION_NVP(m_orbital_period)
        & BOOST_SERIALIZATION_NVP(m_initial_orbital_position)
        & BOOST_SERIALIZATION_NVP(m_rotational_period)
        & BOOST_SERIALIZATION_NVP(m_axial_tilt)
        & BOOST_SERIALIZATION_NVP(m_buildings)
        & BOOST_SERIALIZATION_NVP(m_available_trade)
        & BOOST_SERIALIZATION_NVP(m_just_conquered)
        & BOOST_SERIALIZATION_NVP(m_is_about_to_be_colonized);
}

template <class Archive>
void Building::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(UniverseObject)
        & BOOST_SERIALIZATION_NVP(m_building_type)
        & BOOST_SERIALIZATION_NVP(m_planet_id)
        & BOOST_SERIALIZATION_NVP(m_ordered_scrapped);
}

template <class Archive>
void BuildingType::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_description)
        & BOOST_SERIALIZATION_NVP(m_build_cost)
        & BOOST_SERIALIZATION_NVP(m_build_time)
        & BOOST_SERIALIZATION_NVP(m_maintenance_cost)
        & BOOST_SERIALIZATION_NVP(m_location)
        & BOOST_SERIALIZATION_NVP(m_effects)
        & BOOST_SERIALIZATION_NVP(m_graphic);
}

template <class Archive>
void Fleet::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(UniverseObject)
        & BOOST_SERIALIZATION_NVP(m_ships)
        & BOOST_SERIALIZATION_NVP(m_moving_to)
        & BOOST_SERIALIZATION_NVP(m_speed)
        & BOOST_SERIALIZATION_NVP(m_prev_system)
        & BOOST_SERIALIZATION_NVP(m_next_system)
        & BOOST_SERIALIZATION_NVP(m_travel_route)
        & BOOST_SERIALIZATION_NVP(m_travel_distance)
        & BOOST_SERIALIZATION_NVP(m_arrived_this_turn)
        & BOOST_SERIALIZATION_NVP(m_arrival_starlane);
}

template <class Archive>
void Ship::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(UniverseObject)
        & BOOST_SERIALIZATION_NVP(m_design_id)
        & BOOST_SERIALIZATION_NVP(m_fleet_id)
        & BOOST_SERIALIZATION_NVP(m_ordered_scrapped)
        & BOOST_SERIALIZATION_NVP(m_fighters)
        & BOOST_SERIALIZATION_NVP(m_missiles)
        & BOOST_SERIALIZATION_NVP(m_part_meters);
}

template <class Archive>
void Order::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_empire)
        & BOOST_SERIALIZATION_NVP(m_executed);
}

template <class Archive>
void RenameOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_object)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <class Archive>
void NewFleetOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_fleet_name)
        & BOOST_SERIALIZATION_NVP(m_system_id)
        & BOOST_SERIALIZATION_NVP(m_new_id)
        & BOOST_SERIALIZATION_NVP(m_ship_ids);
}

template <class Archive>
void FleetMoveOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_fleet)
        & BOOST_SERIALIZATION_NVP(m_start_system)
        & BOOST_SERIALIZATION_NVP(m_dest_system)
        & BOOST_SERIALIZATION_NVP(m_route);
}

template <class Archive>
void FleetTransferOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_fleet_from)
        & BOOST_SERIALIZATION_NVP(m_fleet_to)
        & BOOST_SERIALIZATION_NVP(m_add_ships);
}

template <class Archive>
void FleetColonizeOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_ship)
        & BOOST_SERIALIZATION_NVP(m_planet)
        & BOOST_SERIALIZATION_NVP(m_colony_fleet_id)
        & BOOST_SERIALIZATION_NVP(m_colony_fleet_name);
}

template <class Archive>
void DeleteFleetOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_fleet);
}

template <class Archive>
void ChangeFocusOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_planet)
        & BOOST_SERIALIZATION_NVP(m_focus)
        & BOOST_SERIALIZATION_NVP(m_primary);
}

template <class Archive>
void ResearchQueueOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_tech_name)
        & BOOST_SERIALIZATION_NVP(m_position)
        & BOOST_SERIALIZATION_NVP(m_remove);
}

template <class Archive>
void ProductionQueueOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_build_type)
        & BOOST_SERIALIZATION_NVP(m_item_name)
        & BOOST_SERIALIZATION_NVP(m_design_id)
        & BOOST_SERIALIZATION_NVP(m_number)
        & BOOST_SERIALIZATION_NVP(m_location)
        & BOOST_SERIALIZATION_NVP(m_index)
        & BOOST_SERIALIZATION_NVP(m_new_quantity)
        & BOOST_SERIALIZATION_NVP(m_new_index);
}

template <class Archive>
void ShipDesignOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_ship_design)
        & BOOST_SERIALIZATION_NVP(m_design_id)
        & BOOST_SERIALIZATION_NVP(m_delete_design_from_empire)
        & BOOST_SERIALIZATION_NVP(m_create_new_design);
}

template <class Archive>
void ScrapOrder::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Order)
        & BOOST_SERIALIZATION_NVP(m_object_id);
}

template <class Archive>
void CombatShip::DirectWeapon::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_range)
        & BOOST_SERIALIZATION_NVP(m_damage);
}

template <class Archive>
void CombatShip::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CombatObject)
        & BOOST_SERIALIZATION_NVP(m_proximity_token)
        & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_ship)
        & BOOST_SERIALIZATION_NVP(m_last_steer)
        & BOOST_SERIALIZATION_NVP(m_mission_queue)
        & BOOST_SERIALIZATION_NVP(m_mission_weight)
        & BOOST_SERIALIZATION_NVP(m_mission_destination)
        & BOOST_SERIALIZATION_NVP(m_mission_subtarget)
        & BOOST_SERIALIZATION_NVP(m_last_queue_update_turn)
        & BOOST_SERIALIZATION_NVP(m_next_LR_fire_turns)
        & BOOST_SERIALIZATION_NVP(m_turn_start_health)
        & BOOST_SERIALIZATION_NVP(m_turn)
        & BOOST_SERIALIZATION_NVP(m_enter_starlane_start_turn)
        & BOOST_SERIALIZATION_NVP(m_pathing_engine)
        & BOOST_SERIALIZATION_NVP(m_raw_PD_strength)
        & BOOST_SERIALIZATION_NVP(m_raw_SR_strength)
        & BOOST_SERIALIZATION_NVP(m_raw_LR_strength)
        & BOOST_SERIALIZATION_NVP(m_is_PD_ship)
        & BOOST_SERIALIZATION_NVP(m_unfired_SR_weapons)
        & BOOST_SERIALIZATION_NVP(m_unfired_PD_weapons)
        & BOOST_SERIALIZATION_NVP(m_unlaunched_fighters)
        & BOOST_SERIALIZATION_NVP(m_launched_formations)
        & BOOST_SERIALIZATION_NVP(m_instrument)
        & BOOST_SERIALIZATION_NVP(m_last_mission);
}

template <class Archive>
void CombatFighterFormation::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_leader)
        & BOOST_SERIALIZATION_NVP(m_members)
        & BOOST_SERIALIZATION_NVP(m_pathing_engine);
}

template <class Archive>
void CombatFighter::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CombatObject)
        & BOOST_SERIALIZATION_NVP(m_proximity_token)
        & BOOST_SERIALIZATION_NVP(m_leader)
        & BOOST_SERIALIZATION_NVP(m_part_name)
        & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_id)
        & BOOST_SERIALIZATION_NVP(m_last_steer)
        & BOOST_SERIALIZATION_NVP(m_mission_queue)
        & BOOST_SERIALIZATION_NVP(m_mission_weight)
        & BOOST_SERIALIZATION_NVP(m_mission_destination)
        & BOOST_SERIALIZATION_NVP(m_mission_subtarget)
        & BOOST_SERIALIZATION_NVP(m_base)
        & BOOST_SERIALIZATION_NVP(m_formation_position)
        & BOOST_SERIALIZATION_NVP(m_formation)
        & BOOST_SERIALIZATION_NVP(m_out_of_formation)
        & BOOST_SERIALIZATION_NVP(m_health)
        & BOOST_SERIALIZATION_NVP(m_last_queue_update_turn)
        & BOOST_SERIALIZATION_NVP(m_last_fired_turn)
        & BOOST_SERIALIZATION_NVP(m_turn)
        & BOOST_SERIALIZATION_NVP(m_stats)
        & BOOST_SERIALIZATION_NVP(m_pathing_engine);
}

template <class Archive>
void Missile::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CombatObject)
        & BOOST_SERIALIZATION_NVP(m_proximity_token)
        & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_last_steer)
        & BOOST_SERIALIZATION_NVP(m_destination)
        & BOOST_SERIALIZATION_NVP(m_target)
        & BOOST_SERIALIZATION_NVP(m_health)
        & BOOST_SERIALIZATION_NVP(m_stats)
        & BOOST_SERIALIZATION_NVP(m_pathing_engine);
}

template <class Archive>
void PathingEngine::serialize(Archive& ar, const unsigned int version)
{
    std::set<CombatObjectPtr> objects;
    if (Archive::is_saving::value) {
        for (std::set<CombatObjectPtr>::iterator it = m_objects.begin();
             it != m_objects.end();
             ++it) {
            // TODO: only copy the objects that are visible from m_objects into objects
            objects.insert(*it);
        }
    }

    ar  & BOOST_SERIALIZATION_NVP(m_next_fighter_id)
        & BOOST_SERIALIZATION_NVP(m_update_number)
        & BOOST_SERIALIZATION_NVP(objects)
        & BOOST_SERIALIZATION_NVP(m_fighter_formations)
        & BOOST_SERIALIZATION_NVP(m_attackees)
        & BOOST_SERIALIZATION_NVP(m_proximity_database)
        & BOOST_SERIALIZATION_NVP(m_obstacles);

    if (Archive::is_loading::value)
        m_objects.swap(objects);
}

void Serialize(FREEORION_OARCHIVE_TYPE& oa, const Empire& empire)
{ oa << BOOST_SERIALIZATION_NVP(empire); }

void Serialize(FREEORION_OARCHIVE_TYPE& oa, const EmpireManager& empire_manager)
{ oa << BOOST_SERIALIZATION_NVP(empire_manager); }

void Serialize(FREEORION_OARCHIVE_TYPE& oa, const Universe& universe)
{ oa << BOOST_SERIALIZATION_NVP(universe); }

void Serialize(FREEORION_OARCHIVE_TYPE& oa, const std::map<int, UniverseObject*>& objects)
{ oa << BOOST_SERIALIZATION_NVP(objects); }

void Serialize(FREEORION_OARCHIVE_TYPE& oa, const OrderSet& order_set)
{ oa << BOOST_SERIALIZATION_NVP(order_set); }

void Serialize(FREEORION_OARCHIVE_TYPE& oa, const PathingEngine& pathing_engine)
{ oa << BOOST_SERIALIZATION_NVP(pathing_engine); }

void Deserialize(FREEORION_IARCHIVE_TYPE& ia, Empire& empire)
{ ia >> BOOST_SERIALIZATION_NVP(empire); }

void Deserialize(FREEORION_IARCHIVE_TYPE& ia, EmpireManager& empire_manager)
{ ia >> BOOST_SERIALIZATION_NVP(empire_manager); }

void Deserialize(FREEORION_IARCHIVE_TYPE& ia, Universe& universe)
{ ia >> BOOST_SERIALIZATION_NVP(universe); }

void Deserialize(FREEORION_IARCHIVE_TYPE& ia, std::map<int, UniverseObject*>& objects)
{ ia >> BOOST_SERIALIZATION_NVP(objects); }

void Deserialize(FREEORION_IARCHIVE_TYPE& ia, OrderSet& order_set)
{ ia >> BOOST_SERIALIZATION_NVP(order_set); }

void Deserialize(FREEORION_IARCHIVE_TYPE& ia, PathingEngine& pathing_engine)
{ ia >> BOOST_SERIALIZATION_NVP(pathing_engine); }
