#include "Serialize.h"

#include "Logger.h"
#include "Serialize.ipp"

#include "../universe/Building.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/Planet.h"
#include "../universe/ShipDesign.h"
#include "../universe/Species.h"
#include "../universe/System.h"
#include "../universe/Field.h"
#include "../universe/Universe.h"

BOOST_CLASS_EXPORT(System)
BOOST_CLASS_EXPORT(Field)
BOOST_CLASS_EXPORT(Planet)
BOOST_CLASS_EXPORT(Building)
BOOST_CLASS_EXPORT(Fleet)
BOOST_CLASS_EXPORT(Ship)
BOOST_CLASS_VERSION(Ship, 1)
//BOOST_CLASS_EXPORT(ShipDesign)
//BOOST_CLASS_VERSION(ShipDesign, 1)

template <class Archive>
void ObjectMap::serialize(Archive& ar, const unsigned int version)
{
    ar & BOOST_SERIALIZATION_NVP(m_objects);

    // If loading from the archive, propagate the changes to the specialized maps.
    if (Archive::is_loading::value)
        CopyObjectsToSpecializedMaps();
}

template <class Archive>
void Universe::serialize(Archive& ar, const unsigned int version)
{
    ObjectMap                       objects;
    std::set<int>                   destroyed_object_ids;
    EmpireObjectMap                 empire_latest_known_objects;
    EmpireObjectVisibilityMap       empire_object_visibility;
    EmpireObjectVisibilityTurnMap   empire_object_visibility_turns;
    ObjectKnowledgeMap              empire_known_destroyed_object_ids;
    ObjectKnowledgeMap              empire_stale_knowledge_object_ids;
    ShipDesignMap                   ship_designs;

    ar.template register_type<System>();

    if (Archive::is_saving::value) {
        Logger().debugStream() << "Universe::serialize : Getting gamestate data";
        GetObjectsToSerialize(              objects,                            m_encoding_empire);
        GetDestroyedObjectsToSerialize(     destroyed_object_ids,               m_encoding_empire);
        GetEmpireKnownObjectsToSerialize(   empire_latest_known_objects,        m_encoding_empire);
        GetEmpireObjectVisibilityMap(       empire_object_visibility,           m_encoding_empire);
        GetEmpireObjectVisibilityTurnMap(   empire_object_visibility_turns,     m_encoding_empire);
        GetEmpireKnownDestroyedObjects(     empire_known_destroyed_object_ids,  m_encoding_empire);
        GetEmpireStaleKnowledgeObjects(     empire_stale_knowledge_object_ids,  m_encoding_empire);
        GetShipDesignsToSerialize(          ship_designs,                       m_encoding_empire);
    }

    if (Archive::is_loading::value) {
        Clear();    // clean up any existing dynamically allocated contents before replacing containers with deserialized data
    }

    Logger().debugStream() << "Universe::serialize : (de)serializing universe width";
    ar  & BOOST_SERIALIZATION_NVP(m_universe_width);
    Logger().debugStream() << "Universe::serialize : (de)serializing ship designs";
    ar  & BOOST_SERIALIZATION_NVP(ship_designs);
    ar  & BOOST_SERIALIZATION_NVP(m_empire_known_ship_design_ids);
    Logger().debugStream() << "Universe::serialize : (de)serializing empire object visibility";
    ar  & BOOST_SERIALIZATION_NVP(empire_object_visibility);
    ar  & BOOST_SERIALIZATION_NVP(empire_object_visibility_turns);
    ar  & BOOST_SERIALIZATION_NVP(empire_known_destroyed_object_ids);
    ar  & BOOST_SERIALIZATION_NVP(empire_stale_knowledge_object_ids);
    Logger().debugStream() << "Universe::serialize : (de)serializing actual objects";
    ar  & BOOST_SERIALIZATION_NVP(objects)
        & BOOST_SERIALIZATION_NVP(destroyed_object_ids);
    Logger().debugStream() << "Universe::serialize : (de)serializing empire known objects";
    ar  & BOOST_SERIALIZATION_NVP(empire_latest_known_objects);
    Logger().debugStream() << "Universe::serialize : (de)serializing last allocated ids";
    ar  & BOOST_SERIALIZATION_NVP(m_last_allocated_object_id);
    ar  & BOOST_SERIALIZATION_NVP(m_last_allocated_design_id);
    Logger().debugStream() << "Universe::serialize : (de)serializing stats";
    ar  & BOOST_SERIALIZATION_NVP(m_stat_records);
    Logger().debugStream() << "Universe::serialize : (de)serializing done";

    if (Archive::is_saving::value) {
        Logger().debugStream() << "Universe::serialize : Cleaning up temporary data";
        // clean up temporary objects in temporary ObjectMaps
        objects.Clear();
        for (EmpireObjectMap::iterator it = empire_latest_known_objects.begin();
             it != empire_latest_known_objects.end(); ++it)
        { it->second.Clear(); }
    }

    if (Archive::is_loading::value) {
        Logger().debugStream() << "Universe::serialize : Swapping old/new data";
        m_objects.swap(objects);
        m_destroyed_object_ids.swap(destroyed_object_ids);
        m_empire_latest_known_objects.swap(empire_latest_known_objects);
        m_empire_object_visibility.swap(empire_object_visibility);
        m_empire_object_visibility_turns.swap(empire_object_visibility_turns);
        m_empire_known_destroyed_object_ids.swap(empire_known_destroyed_object_ids);
        m_empire_stale_knowledge_object_ids.swap(empire_stale_knowledge_object_ids);
        m_ship_designs.swap(ship_designs);
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
void Field::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(UniverseObject)
        & BOOST_SERIALIZATION_NVP(m_type_name);
}

template <class Archive>
void Planet::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(UniverseObject)
        & BOOST_SERIALIZATION_BASE_OBJECT_NVP(PopCenter)
        & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ResourceCenter)
        & BOOST_SERIALIZATION_NVP(m_type)
        & BOOST_SERIALIZATION_NVP(m_original_type)
        & BOOST_SERIALIZATION_NVP(m_size)
        & BOOST_SERIALIZATION_NVP(m_orbital_period)
        & BOOST_SERIALIZATION_NVP(m_initial_orbital_position)
        & BOOST_SERIALIZATION_NVP(m_rotational_period)
        & BOOST_SERIALIZATION_NVP(m_axial_tilt)
        & BOOST_SERIALIZATION_NVP(m_buildings)
        & BOOST_SERIALIZATION_NVP(m_just_conquered)
        & BOOST_SERIALIZATION_NVP(m_is_about_to_be_colonized)
        & BOOST_SERIALIZATION_NVP(m_is_about_to_be_invaded)
        & BOOST_SERIALIZATION_NVP(m_is_about_to_be_bombarded)
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
        & BOOST_SERIALIZATION_NVP(m_prev_system)
        & BOOST_SERIALIZATION_NVP(m_next_system)
        & BOOST_SERIALIZATION_NVP(m_aggressive)
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
        & BOOST_SERIALIZATION_NVP(m_ordered_bombard_planet_id)
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
        & BOOST_SERIALIZATION_NVP(m_designed_on_turn)
        & BOOST_SERIALIZATION_NVP(m_hull)
        & BOOST_SERIALIZATION_NVP(m_parts)
        & BOOST_SERIALIZATION_NVP(m_is_monster)
        & BOOST_SERIALIZATION_NVP(m_icon)
        & BOOST_SERIALIZATION_NVP(m_3D_model)
        & BOOST_SERIALIZATION_NVP(m_name_desc_in_stringtable);
    if (Archive::is_loading::value)
        BuildStatCaches();
}

// explicit template initialization of System::serialize needed to avoid bug with GCC 4.5.2.
template
void SpeciesManager::serialize<freeorion_oarchive>(freeorion_oarchive& ar, const unsigned int version);

// explicit template initialization of System::serialize needed to avoid bug with GCC 4.5.2.
template
void SpeciesManager::serialize<freeorion_iarchive>(freeorion_iarchive& ar, const unsigned int version);

template <class Archive>
void SpeciesManager::serialize(Archive& ar, const unsigned int version)
{
    // Don't need to send all the data about species, as this is derived from
    // content data files in species.txt that should be available to any
    // client or server.  Instead, just need to send the gamestate portion of
    // species: their homeworlds in the current game

    std::map<std::string, std::set<int> > species_homeworlds_map;

    if (Archive::is_saving::value) {
        species_homeworlds_map = GetSpeciesHomeworldsMap(GetUniverse().EncodingEmpire());
    }

    ar  & BOOST_SERIALIZATION_NVP(species_homeworlds_map);

    if (Archive::is_loading::value) {
        SetSpeciesHomeworlds(species_homeworlds_map);
    }
}

// explicit template initialization of System::serialize needed to avoid bug with GCC 4.5.2.
template
void System::serialize<freeorion_oarchive>(freeorion_oarchive& ar, const unsigned int version);

// explicit template initialization of System::serialize needed to avoid bug with GCC 4.5.2.
template
void System::serialize<freeorion_iarchive>(freeorion_iarchive& ar, const unsigned int version);

void Serialize(freeorion_oarchive& oa, const Universe& universe)
{ oa << BOOST_SERIALIZATION_NVP(universe); }

void Serialize(freeorion_oarchive& oa, const std::map<int, TemporaryPtr<UniverseObject> >& objects)
{ oa << BOOST_SERIALIZATION_NVP(objects); }

void Deserialize(freeorion_iarchive& ia, Universe& universe)
{ ia >> BOOST_SERIALIZATION_NVP(universe); }

void Deserialize(freeorion_iarchive& ia, std::map<int, TemporaryPtr<UniverseObject> >& objects)
{ ia >> BOOST_SERIALIZATION_NVP(objects); }
