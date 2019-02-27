#include "Serialize.h"

#include "Logger.h"
#include "Serialize.ipp"

#include "../universe/IDAllocator.h"
#include "../universe/Building.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/Planet.h"
#include "../universe/ShipDesign.h"
#include "../universe/Species.h"
#include "../universe/System.h"
#include "../universe/Field.h"
#include "../universe/Universe.h"

#include "OptionsDB.h"

#include <boost/lexical_cast.hpp>
#include <boost/uuid/nil_generator.hpp>

BOOST_CLASS_EXPORT(System)
BOOST_CLASS_EXPORT(Field)
BOOST_CLASS_EXPORT(Planet)
BOOST_CLASS_VERSION(Planet, 1)
BOOST_CLASS_EXPORT(Building)
BOOST_CLASS_EXPORT(Fleet)
BOOST_CLASS_VERSION(Fleet, 3)
BOOST_CLASS_EXPORT(Ship)
BOOST_CLASS_VERSION(Ship, 2)
BOOST_CLASS_EXPORT(ShipDesign)
BOOST_CLASS_VERSION(ShipDesign, 2)
BOOST_CLASS_EXPORT(Universe)
BOOST_CLASS_VERSION(Universe, 1)

template <class Archive>
void ObjectMap::serialize(Archive& ar, const unsigned int version)
{
    ar & BOOST_SERIALIZATION_NVP(m_objects);

    // If loading from the archive, propagate the changes to the specialized
    // maps.
    if (Archive::is_loading::value) {
        CopyObjectsToSpecializedMaps();
    }
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

    std::string serializing_label = (Archive::is_loading::value ? "deserializing" : "serializing");

    if (Archive::is_saving::value) {
        DebugLogger() << "Universe::serialize : Getting gamestate data";
        GetObjectsToSerialize(              objects,                            m_encoding_empire);
        GetDestroyedObjectsToSerialize(     destroyed_object_ids,               m_encoding_empire);
        GetEmpireKnownObjectsToSerialize(   empire_latest_known_objects,        m_encoding_empire);
        GetEmpireObjectVisibilityMap(       empire_object_visibility,           m_encoding_empire);
        GetEmpireObjectVisibilityTurnMap(   empire_object_visibility_turns,     m_encoding_empire);
        //GetEffectSpecifiedVisibilities(     effect_specified_visibilities,      m_encoding_empire);
        GetEmpireKnownDestroyedObjects(     empire_known_destroyed_object_ids,  m_encoding_empire);
        GetEmpireStaleKnowledgeObjects(     empire_stale_knowledge_object_ids,  m_encoding_empire);
        GetShipDesignsToSerialize(          ship_designs,                       m_encoding_empire);
    }

    if (Archive::is_loading::value) {
        // clean up any existing dynamically allocated contents before replacing
        // containers with deserialized data.
        Clear();
    }

    ar  & BOOST_SERIALIZATION_NVP(m_universe_width);
    DebugLogger() << "Universe::serialize : " << serializing_label << " universe width: " << m_universe_width;

    ar  & BOOST_SERIALIZATION_NVP(ship_designs);
    if (Archive::is_loading::value)
        m_ship_designs.swap(ship_designs);
    DebugLogger() << "Universe::serialize : " << serializing_label << " " << ship_designs.size() << " ship designs";

    ar  & BOOST_SERIALIZATION_NVP(m_empire_known_ship_design_ids);

    ar  & BOOST_SERIALIZATION_NVP(empire_object_visibility);
    ar  & BOOST_SERIALIZATION_NVP(empire_object_visibility_turns);
    //ar  & BOOST_SERIALIZATION_NVP(effect_specified_visibilities);
    ar  & BOOST_SERIALIZATION_NVP(empire_known_destroyed_object_ids);
    ar  & BOOST_SERIALIZATION_NVP(empire_stale_knowledge_object_ids);
    DebugLogger() << "Universe::serialize : " << serializing_label
                  << " empire object visibility for " << empire_object_visibility.size() << ", "
                  << empire_object_visibility_turns.size() << ", "
                  //<< effect_specified_visibilities.size() << ", "
                  << empire_known_destroyed_object_ids.size() << ", "
                  << empire_stale_knowledge_object_ids.size() <<  " empires";
    if (Archive::is_loading::value) {
        m_empire_object_visibility.swap(empire_object_visibility);
        m_empire_object_visibility_turns.swap(empire_object_visibility_turns);
        //m_effect_specified_empire_object_visibilities.swap(effect_specified_visibilities);
        m_empire_known_destroyed_object_ids.swap(empire_known_destroyed_object_ids);
        m_empire_stale_knowledge_object_ids.swap(empire_stale_knowledge_object_ids);
    }

    ar  & BOOST_SERIALIZATION_NVP(objects);
    DebugLogger() << "Universe::serialize : " << serializing_label << " " << objects.NumObjects() << " objects";
    if (Archive::is_loading::value) {
        m_objects.swap(objects);
    }

    ar  & BOOST_SERIALIZATION_NVP(destroyed_object_ids);
    DebugLogger() << "Universe::serialize : " << serializing_label << " " << destroyed_object_ids.size() << " destroyed object ids";
    if (Archive::is_loading::value) {
        m_destroyed_object_ids.swap(destroyed_object_ids);
        m_objects.UpdateCurrentDestroyedObjects(m_destroyed_object_ids);
    }

    ar  & BOOST_SERIALIZATION_NVP(empire_latest_known_objects);
    DebugLogger() << "Universe::serialize : " << serializing_label << " empire known objects for " << empire_latest_known_objects.size() << " empires";
    if (Archive::is_loading::value) {
        m_empire_latest_known_objects.swap(empire_latest_known_objects);
    }

    if (version >= 1) {
        DebugLogger() << "Universe::serialize : " << serializing_label << " id allocator version = " << version;
        m_object_id_allocator->SerializeForEmpire(ar, version, m_encoding_empire);
        m_design_id_allocator->SerializeForEmpire(ar, version, m_encoding_empire);
    } else {
        if (Archive::is_loading::value) {
            int dummy_last_allocated_object_id;
            int dummy_last_allocated_design_id;
            DebugLogger() << "Universe::serialize : " << serializing_label << " legacy last allocated ids version = " << version;
            ar  & boost::serialization::make_nvp("m_last_allocated_object_id", dummy_last_allocated_object_id);
            DebugLogger() << "Universe::serialize : " << serializing_label << " legacy last allocated ids2";
            ar  & boost::serialization::make_nvp("m_last_allocated_design_id", dummy_last_allocated_design_id);

            DebugLogger() << "Universe::serialize : " << serializing_label << " legacy id allocator";
            // For legacy loads pre-dating the use of the IDAllocator the server
            // allocators need to be initialized with a list of the empires.
            std::vector<int> allocating_empire_ids(m_empire_latest_known_objects.size());
            std::transform(m_empire_latest_known_objects.begin(), m_empire_latest_known_objects.end(),
                           allocating_empire_ids.begin(),
                           [](const std::pair<int, ObjectMap> ii) { return ii.first; });

            ResetAllIDAllocation(allocating_empire_ids);
        }
    }

    if (Archive::is_saving::value && (!GetOptionsDB().Get<bool>("network.server.publish-statistics")) && m_encoding_empire != ALL_EMPIRES) {
        std::map<std::string, std::map<int, std::map<int, double>>> dummy_stat_records;
        ar  & boost::serialization::make_nvp("m_stat_records", dummy_stat_records);
    } else {
        ar  & BOOST_SERIALIZATION_NVP(m_stat_records);
        DebugLogger() << "Universe::serialize : " << serializing_label << " " << m_stat_records.size() << " types of statistic";
    }

    DebugLogger() << "Universe::serialize : " << serializing_label << " done";

    if (Archive::is_saving::value) {
        DebugLogger() << "Universe::serialize : Cleaning up temporary data";
        // clean up temporary objects in temporary ObjectMaps.
        objects.Clear();
        for (auto& elko : empire_latest_known_objects)
        { elko.second.Clear(); }
    }

    if (Archive::is_loading::value) {
        DebugLogger() << "Universe::serialize : updating empires' latest known object destruction states";
        // update known destroyed objects state in each empire's latest known
        // objects.
        for (auto& elko : m_empire_latest_known_objects) {
            auto destroyed_ids_it = m_empire_known_destroyed_object_ids.find(elko.first);
            if (destroyed_ids_it != m_empire_known_destroyed_object_ids.end())
                elko.second.UpdateCurrentDestroyedObjects(destroyed_ids_it->second);
        }
    }
    DebugLogger() << "Universe::serialize done";
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
        & BOOST_SERIALIZATION_NVP(m_planets)
        & BOOST_SERIALIZATION_NVP(m_buildings)
        & BOOST_SERIALIZATION_NVP(m_fleets)
        & BOOST_SERIALIZATION_NVP(m_ships)
        & BOOST_SERIALIZATION_NVP(m_fields)
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
   ar   & BOOST_SERIALIZATION_BASE_OBJECT_NVP(UniverseObject)
        & BOOST_SERIALIZATION_BASE_OBJECT_NVP(PopCenter)
        & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ResourceCenter)
        & BOOST_SERIALIZATION_NVP(m_type)
        & BOOST_SERIALIZATION_NVP(m_original_type)
        & BOOST_SERIALIZATION_NVP(m_size)
        & BOOST_SERIALIZATION_NVP(m_orbital_period)
        & BOOST_SERIALIZATION_NVP(m_initial_orbital_position)
        & BOOST_SERIALIZATION_NVP(m_rotational_period)
        & BOOST_SERIALIZATION_NVP(m_axial_tilt)
        & BOOST_SERIALIZATION_NVP(m_buildings);
    if (version < 1) {
        bool dummy = false;
        ar   & boost::serialization::make_nvp("m_just_conquered", dummy);
    } else {
        ar   & BOOST_SERIALIZATION_NVP(m_turn_last_conquered);
    }
    ar  & BOOST_SERIALIZATION_NVP(m_is_about_to_be_colonized)
        & BOOST_SERIALIZATION_NVP(m_is_about_to_be_invaded)
        & BOOST_SERIALIZATION_NVP(m_is_about_to_be_bombarded)
        & BOOST_SERIALIZATION_NVP(m_ordered_given_to_empire_id)
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
        & BOOST_SERIALIZATION_NVP(m_prev_system)
        & BOOST_SERIALIZATION_NVP(m_next_system)
        & BOOST_SERIALIZATION_NVP(m_aggressive)
        & BOOST_SERIALIZATION_NVP(m_ordered_given_to_empire_id)
        & BOOST_SERIALIZATION_NVP(m_travel_route);
    if (version < 3) {
        double dummy_travel_distance;
        ar & boost::serialization::make_nvp("m_travel_distance", dummy_travel_distance);
    }
    ar  & BOOST_SERIALIZATION_NVP(m_arrived_this_turn)
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
        & BOOST_SERIALIZATION_NVP(m_part_meters)
        & BOOST_SERIALIZATION_NVP(m_species_name)
        & BOOST_SERIALIZATION_NVP(m_produced_by_empire_id)
        & BOOST_SERIALIZATION_NVP(m_arrived_on_turn);
    if (version >= 1) {
        ar  & BOOST_SERIALIZATION_NVP(m_last_turn_active_in_combat);
        if (version >= 2) {
            ar  & BOOST_SERIALIZATION_NVP(m_last_resupplied_on_turn);
        }
    }
}

template <class Archive>
void ShipDesign::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_id)
        & BOOST_SERIALIZATION_NVP(m_name);

    TraceLogger() << "ship design serialize version: " << version << " : " << (Archive::is_saving::value ? "saving" : "loading");

    if (version >= 1) {
        // Serialization of m_uuid as a primitive doesn't work as expected from
        // the documentation.  This workaround instead serializes a string
        // representation.
        if (Archive::is_saving::value) {
            auto string_uuid = boost::uuids::to_string(m_uuid);
            ar & BOOST_SERIALIZATION_NVP(string_uuid);
        } else {
            std::string string_uuid;
            ar & BOOST_SERIALIZATION_NVP(string_uuid);
            try {
                m_uuid = boost::lexical_cast<boost::uuids::uuid>(string_uuid);
            } catch (const boost::bad_lexical_cast&) {
                m_uuid = boost::uuids::nil_generator()();
            }
        }
    } else if (Archive::is_loading::value) {
        m_uuid = boost::uuids::nil_generator()();
    }

    ar  & BOOST_SERIALIZATION_NVP(m_description)
        & BOOST_SERIALIZATION_NVP(m_designed_on_turn);
    if (version >= 2)
        ar  & BOOST_SERIALIZATION_NVP(m_designed_by_empire);
    ar  & BOOST_SERIALIZATION_NVP(m_hull)
        & BOOST_SERIALIZATION_NVP(m_parts)
        & BOOST_SERIALIZATION_NVP(m_is_monster)
        & BOOST_SERIALIZATION_NVP(m_icon)
        & BOOST_SERIALIZATION_NVP(m_3D_model)
        & BOOST_SERIALIZATION_NVP(m_name_desc_in_stringtable);
    if (Archive::is_loading::value) {
        ForceValidDesignOrThrow(boost::none, true);
        BuildStatCaches();
    }
}

template
void SpeciesManager::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, const unsigned int version);
template
void SpeciesManager::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, const unsigned int version);
template
void SpeciesManager::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, const unsigned int version);
template
void SpeciesManager::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, const unsigned int version);

template <class Archive>
void SpeciesManager::serialize(Archive& ar, const unsigned int version)
{
    // Don't need to send all the data about species, as this is derived from
    // content data files in scripting/species that should be available to any
    // client or server.  Instead, just need to send the gamestate portion of
    // species: their homeworlds in the current game, and their opinions of
    // empires and eachother

    std::map<std::string, std::set<int>>                species_homeworlds;
    std::map<std::string, std::map<int, float>>         empire_opinions;
    std::map<std::string, std::map<std::string, float>> other_species_opinions;
    std::map<std::string, std::map<int, float>>         species_object_populations;
    std::map<std::string, std::map<std::string, int>>   species_ships_destroyed;

    if (Archive::is_saving::value) {
        species_homeworlds =        GetSpeciesHomeworldsMap(GetUniverse().EncodingEmpire());
        empire_opinions =           GetSpeciesEmpireOpinionsMap(GetUniverse().EncodingEmpire());
        other_species_opinions =    GetSpeciesSpeciesOpinionsMap(GetUniverse().EncodingEmpire());
        species_object_populations =SpeciesObjectPopulations(GetUniverse().EncodingEmpire());
        species_ships_destroyed =   SpeciesShipsDestroyed(GetUniverse().EncodingEmpire());
    }

    ar  & BOOST_SERIALIZATION_NVP(species_homeworlds)
        & BOOST_SERIALIZATION_NVP(empire_opinions)
        & BOOST_SERIALIZATION_NVP(other_species_opinions)
        & BOOST_SERIALIZATION_NVP(species_object_populations)
        & BOOST_SERIALIZATION_NVP(species_ships_destroyed)
    ;

    if (Archive::is_loading::value) {
        SetSpeciesHomeworlds(species_homeworlds);
        SetSpeciesEmpireOpinions(empire_opinions);
        SetSpeciesSpeciesOpinions(other_species_opinions);
        m_species_object_populations = std::move(species_object_populations);
        m_species_species_ships_destroyed = std::move(species_ships_destroyed);
    }
}

// explicit template initialization of System::serialize needed to avoid bug with GCC 4.5.2.
template
void System::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, const unsigned int version);
template
void System::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, const unsigned int version);

// explicit template initialization of System::serialize needed to avoid bug with GCC 4.5.2.
template
void System::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, const unsigned int version);
template
void System::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, const unsigned int version);

template <class Archive>
void Serialize(Archive& oa, const Universe& universe)
{ oa << BOOST_SERIALIZATION_NVP(universe); }
template FO_COMMON_API void Serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& oa, const Universe& universe);
template FO_COMMON_API void Serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& oa, const Universe& universe);

template <class Archive>
void Serialize(Archive& oa, const std::map<int, std::shared_ptr<UniverseObject>>& objects)
{ oa << BOOST_SERIALIZATION_NVP(objects); }
template void Serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& oa, const std::map<int, std::shared_ptr<UniverseObject>>& objects);
template void Serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& oa, const std::map<int, std::shared_ptr<UniverseObject>>& objects);

template <class Archive>
void Deserialize(Archive& ia, Universe& universe)
{ ia >> BOOST_SERIALIZATION_NVP(universe); }
template FO_COMMON_API void Deserialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ia, Universe& universe);
template FO_COMMON_API void Deserialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ia, Universe& universe);

template <class Archive>
void Deserialize(Archive& ia, std::map<int, std::shared_ptr<UniverseObject>>& objects)
{ ia >> BOOST_SERIALIZATION_NVP(objects); }
template void Deserialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ia, std::map<int, std::shared_ptr<UniverseObject>>& objects);
template void Deserialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ia, std::map<int, std::shared_ptr<UniverseObject>>& objects);
