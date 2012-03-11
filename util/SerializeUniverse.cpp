#include "Serialize.h"

#include "Serialize.ipp"

#include "../universe/Building.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/Planet.h"
#include "../universe/ShipDesign.h"
#include "../universe/System.h"

BOOST_CLASS_EXPORT(System)
BOOST_CLASS_EXPORT(Planet)
BOOST_CLASS_EXPORT(Building)
BOOST_CLASS_EXPORT(Fleet)
BOOST_CLASS_EXPORT(Ship)
BOOST_CLASS_VERSION(Ship, 1)
BOOST_CLASS_EXPORT(ShipDesign)
BOOST_CLASS_VERSION(ShipDesign, 1)

template <class Archive>
void ObjectMap::serialize(Archive& ar, const unsigned int version)
{
    ar & BOOST_SERIALIZATION_NVP(m_objects);

    if (Archive::is_loading::value) {
        CopyObjectsToConstObjects();
    }
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

    ar.template register_type<System>();

    if (Archive::is_saving::value) {
        GetObjectsToSerialize(              objects,                            s_encoding_empire);
        GetEmpireKnownObjectsToSerialize(   empire_latest_known_objects,        s_encoding_empire);
        GetEmpireObjectVisibilityMap(       empire_object_visibility,           s_encoding_empire);
        GetEmpireObjectVisibilityTurnMap(   empire_object_visibility_turns,     s_encoding_empire);
        GetEmpireKnownDestroyedObjects(     empire_known_destroyed_object_ids,  s_encoding_empire);
        GetShipDesignsToSerialize(          ship_designs,                       s_encoding_empire);
    }

    if (Archive::is_loading::value) {
        Clear();    // clean up any existing dynamically allocated contents before replacing containers with deserialized data
    }

    ar  & BOOST_SERIALIZATION_NVP(s_universe_width)
        & BOOST_SERIALIZATION_NVP(ship_designs)
        & BOOST_SERIALIZATION_NVP(m_empire_known_ship_design_ids)
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
        m_objects.swap(objects);
        m_empire_latest_known_objects.swap(empire_latest_known_objects);
        m_empire_object_visibility.swap(empire_object_visibility);
        m_empire_object_visibility_turns.swap(empire_object_visibility_turns);
        m_empire_known_destroyed_object_ids.swap(empire_known_destroyed_object_ids);
        m_ship_designs.swap(ship_designs);
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
        & BOOST_SERIALIZATION_NVP(m_owner_empire_id)
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
        & BOOST_SERIALIZATION_NVP(m_starlanes_wormholes)
        & BOOST_SERIALIZATION_NVP(m_last_turn_battle_here);
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
        & BOOST_SERIALIZATION_NVP(m_is_about_to_be_colonized)
        & BOOST_SERIALIZATION_NVP(m_is_about_to_be_invaded)
        & BOOST_SERIALIZATION_NVP(m_last_turn_attacked_by_ship);
}

template <class Archive>
void Building::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(UniverseObject)
        & BOOST_SERIALIZATION_NVP(m_building_type)
        & BOOST_SERIALIZATION_NVP(m_planet_id)
        & BOOST_SERIALIZATION_NVP(m_ordered_scrapped)
        & BOOST_SERIALIZATION_NVP(m_produced_by_empire_id);
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
        & BOOST_SERIALIZATION_NVP(m_ordered_colonize_planet_id)
        & BOOST_SERIALIZATION_NVP(m_ordered_invade_planet_id)
        & BOOST_SERIALIZATION_NVP(m_fighters)
        & BOOST_SERIALIZATION_NVP(m_missiles)
        & BOOST_SERIALIZATION_NVP(m_part_meters)
        & BOOST_SERIALIZATION_NVP(m_species_name)
        & BOOST_SERIALIZATION_NVP(m_produced_by_empire_id);
    if (version >= 1) {
        ar  & BOOST_SERIALIZATION_NVP(m_last_turn_active_in_combat);
    }
}

template <class Archive>
void ShipDesign::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_id)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_description)
        & BOOST_SERIALIZATION_NVP(m_designed_by_empire_id)
        & BOOST_SERIALIZATION_NVP(m_designed_on_turn)
        & BOOST_SERIALIZATION_NVP(m_hull)
        & BOOST_SERIALIZATION_NVP(m_parts)
        & BOOST_SERIALIZATION_NVP(m_is_monster);
    //if (version < 1) {
    //    std::string m_graphic;
    //    ar & BOOST_SERIALIZATION_NVP(m_graphic);
    //    m_icon = m_graphic;
    //} else /*if (version >= 1)*/ {
    //    ar  & BOOST_SERIALIZATION_NVP(m_icon);
    //}
    ar  & BOOST_SERIALIZATION_NVP(m_icon);

    ar  & BOOST_SERIALIZATION_NVP(m_3D_model)
        & BOOST_SERIALIZATION_NVP(m_name_desc_in_stringtable);
    if (Archive::is_loading::value)
        BuildStatCaches();
}

// explicit template initialization of System::serialize needed to avoid bug with GCC 4.5.2.
template
void System::serialize<FREEORION_OARCHIVE_TYPE>(FREEORION_OARCHIVE_TYPE& ar, const unsigned int version);

// explicit template initialization of System::serialize needed to avoid bug with GCC 4.5.2.
template
void System::serialize<FREEORION_IARCHIVE_TYPE>(FREEORION_IARCHIVE_TYPE& ar, const unsigned int version);

void Serialize(FREEORION_OARCHIVE_TYPE& oa, const Universe& universe)
{ oa << BOOST_SERIALIZATION_NVP(universe); }

void Serialize(FREEORION_OARCHIVE_TYPE& oa, const std::map<int, UniverseObject*>& objects)
{ oa << BOOST_SERIALIZATION_NVP(objects); }

void Deserialize(FREEORION_IARCHIVE_TYPE& ia, Universe& universe)
{ ia >> BOOST_SERIALIZATION_NVP(universe); }

void Deserialize(FREEORION_IARCHIVE_TYPE& ia, std::map<int, UniverseObject*>& objects)
{ ia >> BOOST_SERIALIZATION_NVP(objects); }
