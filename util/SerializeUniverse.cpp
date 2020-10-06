#include "Serialize.h"

#include "Logger.h"
#include "Serialize.ipp"

#include "../universe/IDAllocator.h"
#include "../universe/Building.h"
#include "../universe/Enums.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/Planet.h"
#include "../universe/ShipDesign.h"
#include "../universe/Species.h"
#include "../universe/System.h"
#include "../universe/Field.h"
#include "../universe/Universe.h"
#include "ScopedTimer.h"
#include "AppInterface.h"
#include "OptionsDB.h"

#include <boost/lexical_cast.hpp>
#include <boost/uuid/nil_generator.hpp>

BOOST_CLASS_EXPORT(Field)
BOOST_CLASS_EXPORT(Universe)
BOOST_CLASS_VERSION(Universe, 1)

template <typename Archive>
void serialize(Archive& ar, PopCenter& p, unsigned int const version)
{
    ar  & boost::serialization::make_nvp("m_species_name", p.m_species_name);
}

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, PopCenter&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, PopCenter&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, PopCenter&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, PopCenter&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, ResourceCenter& rs, unsigned int const version)
{
    using namespace boost::serialization;

    ar  & make_nvp("m_focus", rs.m_focus)
        & make_nvp("m_last_turn_focus_changed", rs.m_last_turn_focus_changed)
        & make_nvp("m_focus_turn_initial", rs.m_focus_turn_initial)
        & make_nvp("m_last_turn_focus_changed_turn_initial", rs.m_last_turn_focus_changed_turn_initial);
}

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, ResourceCenter&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, ResourceCenter&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, ResourceCenter&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, ResourceCenter&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, ObjectMap& objmap, unsigned int const version)
{
    ar & boost::serialization::make_nvp("m_objects", objmap.m_objects);

    // If loading from the archive, propagate the changes to the specialized maps.
    if (Archive::is_loading::value)
        objmap.CopyObjectsToSpecializedMaps();
}

template <typename Archive>
void serialize(Archive& ar, Universe& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ObjectMap                                 objects;
    std::set<int>                             destroyed_object_ids;
    Universe::EmpireObjectMap                 empire_latest_known_objects;
    Universe::EmpireObjectVisibilityMap       empire_object_visibility;
    Universe::EmpireObjectVisibilityTurnMap   empire_object_visibility_turns;
    Universe::ObjectKnowledgeMap              empire_known_destroyed_object_ids;
    Universe::ObjectKnowledgeMap              empire_stale_knowledge_object_ids;
    Universe::ShipDesignMap                   ship_designs;

    ar.template register_type<System>();

    std::string serializing_label = (Archive::is_loading::value ? "deserializing" : "serializing");

    SectionedScopedTimer timer("Universe " + serializing_label);

    if (Archive::is_saving::value) {
        DebugLogger() << "Universe::serialize : Getting gamestate data";
        timer.EnterSection("collecting data");
        obj.GetObjectsToSerialize(              objects,                            GlobalSerializationEncodingForEmpire());
        obj.GetDestroyedObjectsToSerialize(     destroyed_object_ids,               GlobalSerializationEncodingForEmpire());
        obj.GetEmpireKnownObjectsToSerialize(   empire_latest_known_objects,        GlobalSerializationEncodingForEmpire());
        obj.GetEmpireObjectVisibilityMap(       empire_object_visibility,           GlobalSerializationEncodingForEmpire());
        obj.GetEmpireObjectVisibilityTurnMap(   empire_object_visibility_turns,     GlobalSerializationEncodingForEmpire());
        obj.GetEmpireKnownDestroyedObjects(     empire_known_destroyed_object_ids,  GlobalSerializationEncodingForEmpire());
        obj.GetEmpireStaleKnowledgeObjects(     empire_stale_knowledge_object_ids,  GlobalSerializationEncodingForEmpire());
        obj.GetShipDesignsToSerialize(          ship_designs,                       GlobalSerializationEncodingForEmpire());
        timer.EnterSection("");
    }

    if (Archive::is_loading::value) {
        // clean up any existing dynamically allocated contents before replacing
        // containers with deserialized data.
        obj.Clear();
    }

    ar  & make_nvp("m_universe_width", obj.m_universe_width);
    DebugLogger() << "Universe::serialize : " << serializing_label << " universe width: " << obj.m_universe_width;

    timer.EnterSection("designs");
    ar  & make_nvp("ship_designs", ship_designs);
    if (Archive::is_loading::value)
        obj.m_ship_designs.swap(ship_designs);
    DebugLogger() << "Universe::serialize : " << serializing_label << " " << ship_designs.size() << " ship designs";

    ar  & make_nvp("m_empire_known_ship_design_ids", obj.m_empire_known_ship_design_ids);

    timer.EnterSection("vis / known");
    ar  & make_nvp("empire_object_visibility", empire_object_visibility);
    ar  & make_nvp("empire_object_visibility_turns", empire_object_visibility_turns);
    ar  & make_nvp("empire_known_destroyed_object_ids", empire_known_destroyed_object_ids);
    ar  & make_nvp("empire_stale_knowledge_object_ids", empire_stale_knowledge_object_ids);
    DebugLogger() << "Universe::serialize : " << serializing_label
                  << " empire object visibility for " << empire_object_visibility.size() << ", "
                  << empire_object_visibility_turns.size() << ", "
                  << empire_known_destroyed_object_ids.size() << ", "
                  << empire_stale_knowledge_object_ids.size() <<  " empires";
    timer.EnterSection("");
    if (Archive::is_loading::value) {
        timer.EnterSection("load swap");
        obj.m_empire_object_visibility.swap(empire_object_visibility);
        obj.m_empire_object_visibility_turns.swap(empire_object_visibility_turns);
        obj.m_empire_known_destroyed_object_ids.swap(empire_known_destroyed_object_ids);
        obj.m_empire_stale_knowledge_object_ids.swap(empire_stale_knowledge_object_ids);
        timer.EnterSection("");
    }

    timer.EnterSection("objects");
    ar  & make_nvp("objects", objects);
    DebugLogger() << "Universe::serialize : " << serializing_label << " " << objects.size() << " objects";
    if (Archive::is_loading::value) {
        obj.m_objects.swap(objects);
    }

    timer.EnterSection("destroyed ids");
    ar  & make_nvp("destroyed_object_ids", destroyed_object_ids);
    DebugLogger() << "Universe::serialize : " << serializing_label << " " << destroyed_object_ids.size() << " destroyed object ids";
    if (Archive::is_loading::value) {
        obj.m_destroyed_object_ids.swap(destroyed_object_ids);
        obj.m_objects.UpdateCurrentDestroyedObjects(obj.m_destroyed_object_ids);
    }

    timer.EnterSection("latest known objects");
    ar  & make_nvp("empire_latest_known_objects", empire_latest_known_objects);
    DebugLogger() << "Universe::serialize : " << serializing_label << " empire known objects for " << empire_latest_known_objects.size() << " empires";
    if (Archive::is_loading::value) {
        obj.m_empire_latest_known_objects.swap(empire_latest_known_objects);
    }

    timer.EnterSection("id allocator");
    if (version >= 1) {
        DebugLogger() << "Universe::serialize : " << serializing_label << " id allocator version = " << version;
        obj.m_object_id_allocator->SerializeForEmpire(ar, version, GlobalSerializationEncodingForEmpire());
        obj.m_design_id_allocator->SerializeForEmpire(ar, version, GlobalSerializationEncodingForEmpire());
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
            std::vector<int> allocating_empire_ids(obj.m_empire_latest_known_objects.size());
            std::transform(obj.m_empire_latest_known_objects.begin(), obj.m_empire_latest_known_objects.end(),
                           allocating_empire_ids.begin(),
                           [](const std::pair<int, ObjectMap> ii) { return ii.first; });

            obj.ResetAllIDAllocation(allocating_empire_ids);
        }
    }

    timer.EnterSection("stats");
    if (Archive::is_saving::value && GlobalSerializationEncodingForEmpire() != ALL_EMPIRES && (!GetOptionsDB().Get<bool>("network.server.publish-statistics"))) {
        std::map<std::string, std::map<int, std::map<int, double>>> dummy_stat_records;
        ar  & boost::serialization::make_nvp("m_stat_records", dummy_stat_records);
    } else {
        ar  & make_nvp("m_stat_records", obj.m_stat_records);
        DebugLogger() << "Universe::serialize : " << serializing_label << " " << obj.m_stat_records.size() << " types of statistic";
    }
    timer.EnterSection("");

    if (Archive::is_saving::value) {
        DebugLogger() << "Universe::serialize : Cleaning up temporary data";
        // clean up temporary objects in temporary ObjectMaps.
        objects.clear();
        for (auto& elko : empire_latest_known_objects)
        { elko.second.clear(); }
    }

    if (Archive::is_loading::value) {
        DebugLogger() << "Universe::serialize : updating empires' latest known object destruction states";
        // update known destroyed objects state in each empire's latest known
        // objects.
        for (auto& elko : obj.m_empire_latest_known_objects) {
            auto destroyed_ids_it = obj.m_empire_known_destroyed_object_ids.find(elko.first);
            if (destroyed_ids_it != obj.m_empire_known_destroyed_object_ids.end())
                elko.second.UpdateCurrentDestroyedObjects(destroyed_ids_it->second);
        }
    }
    DebugLogger() << "Universe " << serializing_label << " done";
}

BOOST_CLASS_EXPORT(UniverseObject)
BOOST_CLASS_VERSION(UniverseObject, 2)

template <typename Archive>
void serialize(Archive& ar, UniverseObject& o, unsigned int const version)
{
    using namespace boost::serialization;

    ar  & make_nvp("m_id", o.m_id)
        & make_nvp("m_name", o.m_name)
        & make_nvp("m_x", o.m_x)
        & make_nvp("m_y", o.m_y)
        & make_nvp("m_owner_empire_id", o.m_owner_empire_id)
        & make_nvp("m_system_id", o.m_system_id)
        & make_nvp("m_specials", o.m_specials);
    if (version < 2) {
        std::map<MeterType, Meter> meter_map;
        ar  & make_nvp("m_meters", meter_map);
        o.m_meters.reserve(meter_map.size());
        o.m_meters.insert(meter_map.begin(), meter_map.end());
    } else {
        ar  & make_nvp("m_meters", o.m_meters);

        // loading the internal vector, like so, was no faster than loading the map
        //auto meters{m_meters.extract_sequence()};
        //ar  & make_nvp("meters", o.meters);
        //m_meters.adopt_sequence(std::move(meters));
    }
    ar  & make_nvp("m_created_on_turn", o.m_created_on_turn);
}


template <typename Archive>
void load_construct_data(Archive& ar, System* obj, unsigned int const version)
{ ::new(obj)System(StarType::INVALID_STAR_TYPE, "", 0.0, 0.0); }

template <typename Archive>
void serialize(Archive& ar, System& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar  & make_nvp("UniverseObject", base_object<UniverseObject>(obj))
        & make_nvp("m_star", obj.m_star)
        & make_nvp("m_orbits", obj.m_orbits)
        & make_nvp("m_objects", obj.m_objects)
        & make_nvp("m_planets", obj.m_planets)
        & make_nvp("m_buildings", obj.m_buildings)
        & make_nvp("m_fleets", obj.m_fleets)
        & make_nvp("m_ships", obj.m_ships)
        & make_nvp("m_fields", obj.m_fields)
        & make_nvp("m_starlanes_wormholes", obj.m_starlanes_wormholes)
        & make_nvp("m_last_turn_battle_here", obj.m_last_turn_battle_here);
}

BOOST_CLASS_EXPORT(System)

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, System&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, System&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, System&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, System&, unsigned int const);


template <typename Archive>
void load_construct_data(Archive& ar, Field* obj, unsigned int const version)
{ ::new(obj)Field("", 0.0, 0.0, 0.0); }

template <typename Archive>
void serialize(Archive& ar, Field& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar  & make_nvp("UniverseObject", base_object<UniverseObject>(obj))
        & make_nvp("m_type_name", obj.m_type_name);
}


template <typename Archive>
void load_construct_data(Archive& ar, Planet* obj, unsigned int const version)
{ ::new(obj)Planet(PlanetType::PT_TERRAN, PlanetSize::SZ_MEDIUM); }

template <typename Archive>
void serialize(Archive& ar, Planet& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar  & make_nvp("UniverseObject", base_object<UniverseObject>(obj))
        & make_nvp("PopCenter", base_object<PopCenter>(obj))
        & make_nvp("ResourceCenter", base_object<ResourceCenter>(obj))
        & make_nvp("m_type", obj.m_type)
        & make_nvp("m_original_type", obj.m_original_type)
        & make_nvp("m_size", obj.m_size)
        & make_nvp("m_orbital_period", obj.m_orbital_period)
        & make_nvp("m_initial_orbital_position", obj.m_initial_orbital_position)
        & make_nvp("m_rotational_period", obj.m_rotational_period)
        & make_nvp("m_axial_tilt", obj.m_axial_tilt)
        & make_nvp("m_buildings", obj.m_buildings);
    if (version < 2) {
        // if deserializing an old save, default to standard default never-colonized turn
        obj.m_turn_last_colonized = INVALID_GAME_TURN;
        if (!obj.SpeciesName().empty()) // but if a planet has a species, it must have been colonized, so default to the previous turn
            obj.m_turn_last_colonized = CurrentTurn() - 1;
    } else {
        ar   & make_nvp("m_turn_last_colonized", obj.m_turn_last_colonized);
    }
    if (version < 1) {
        bool dummy = false;
        ar   & boost::serialization::make_nvp("m_just_conquered", dummy);
    } else {
        ar   & make_nvp("m_turn_last_conquered", obj.m_turn_last_conquered);
    }
    ar  & make_nvp("m_is_about_to_be_colonized", obj.m_is_about_to_be_colonized)
        & make_nvp("m_is_about_to_be_invaded", obj.m_is_about_to_be_invaded)
        & make_nvp("m_is_about_to_be_bombarded", obj.m_is_about_to_be_bombarded)
        & make_nvp("m_ordered_given_to_empire_id", obj.m_ordered_given_to_empire_id)
        & make_nvp("m_last_turn_attacked_by_ship", obj.m_last_turn_attacked_by_ship);
}

BOOST_CLASS_EXPORT(Planet)
BOOST_CLASS_VERSION(Planet, 2)


template <typename Archive>
void load_construct_data(Archive& ar, Building* obj, unsigned int const version)
{ ::new(obj)Building(ALL_EMPIRES, "", ALL_EMPIRES); }

template <typename Archive>
void serialize(Archive& ar, Building& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar  & make_nvp("UniverseObject", base_object<UniverseObject>(obj))
        & make_nvp("m_building_type", obj.m_building_type)
        & make_nvp("m_planet_id", obj.m_planet_id)
        & make_nvp("m_ordered_scrapped", obj.m_ordered_scrapped)
        & make_nvp("m_produced_by_empire_id", obj.m_produced_by_empire_id);
}

BOOST_CLASS_EXPORT(Building)


template <typename Archive>
void load_construct_data(Archive& ar, Fleet* obj, unsigned int const version)
{ ::new(obj)Fleet("", 0.0, 0.0, ALL_EMPIRES); }

template <typename Archive>
void serialize(Archive& ar, Fleet& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar  & make_nvp("UniverseObject", base_object<UniverseObject>(obj))
        & make_nvp("m_ships", obj.m_ships)
        & make_nvp("m_prev_system", obj.m_prev_system)
        & make_nvp("m_next_system", obj.m_next_system);

    if (Archive::is_loading::value && version < 5) {
        bool aggressive = false;
        ar  & make_nvp("m_aggressive", aggressive);
        obj.m_aggression = aggressive ? FleetAggression::FLEET_AGGRESSIVE : FleetAggression::FLEET_PASSIVE;

    } else {
        ar  & make_nvp("m_aggression", obj.m_aggression);
    }

    ar  & make_nvp("m_ordered_given_to_empire_id", obj.m_ordered_given_to_empire_id)
        & make_nvp("m_travel_route", obj.m_travel_route);
    if (version < 3) {
        double dummy_travel_distance;
        ar & boost::serialization::make_nvp("m_travel_distance", dummy_travel_distance);
    }
    if (version >= 4) {
        ar & boost::serialization::make_nvp("m_last_turn_move_ordered", obj.m_last_turn_move_ordered);
    }
    ar  & make_nvp("m_arrived_this_turn", obj.m_arrived_this_turn)
        & make_nvp("m_arrival_starlane", obj.m_arrival_starlane);
}

BOOST_CLASS_EXPORT(Fleet)
BOOST_CLASS_VERSION(Fleet, 5)


template <typename Archive>
void serialize(Archive& ar, Ship& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar  & make_nvp("UniverseObject", base_object<UniverseObject>(obj))
        & make_nvp("m_design_id", obj.m_design_id)
        & make_nvp("m_fleet_id", obj.m_fleet_id)
        & make_nvp("m_ordered_scrapped", obj.m_ordered_scrapped)
        & make_nvp("m_ordered_colonize_planet_id", obj.m_ordered_colonize_planet_id)
        & make_nvp("m_ordered_invade_planet_id", obj.m_ordered_invade_planet_id)
        & make_nvp("m_ordered_bombard_planet_id", obj.m_ordered_bombard_planet_id)
        & make_nvp("m_part_meters", obj.m_part_meters)
        & make_nvp("m_species_name", obj.m_species_name)
        & make_nvp("m_produced_by_empire_id", obj.m_produced_by_empire_id)
        & make_nvp("m_arrived_on_turn", obj.m_arrived_on_turn);
    if (version >= 1) {
        ar  & make_nvp("m_last_turn_active_in_combat", obj.m_last_turn_active_in_combat);
        if (version >= 2) {
            ar  & make_nvp("m_last_resupplied_on_turn", obj.m_last_resupplied_on_turn);
        }
    }
}

BOOST_CLASS_EXPORT(Ship)
BOOST_CLASS_VERSION(Ship, 2)


template <typename Archive>
void serialize(Archive& ar, ShipDesign& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar  & make_nvp("m_id", obj.m_id)
        & make_nvp("m_name", obj.m_name);

    TraceLogger() << "ship design serialize version: " << version << " : " << (Archive::is_saving::value ? "saving" : "loading");

    if (version >= 1) {
        // Serialization of m_uuid as a primitive doesn't work as expected from
        // the documentation.  This workaround instead serializes a string
        // representation.
        if (Archive::is_saving::value) {
            auto string_uuid = boost::uuids::to_string(obj.m_uuid);
            ar & make_nvp("string_uuid", string_uuid);
        } else {
            std::string string_uuid;
            ar & make_nvp("string_uuid", string_uuid);
            try {
                obj.m_uuid = boost::lexical_cast<boost::uuids::uuid>(string_uuid);
            } catch (const boost::bad_lexical_cast&) {
                obj.m_uuid = boost::uuids::nil_generator()();
            }
        }
    } else if (Archive::is_loading::value) {
        obj.m_uuid = boost::uuids::nil_generator()();
    }

    ar  & make_nvp("m_description", obj.m_description)
        & make_nvp("m_designed_on_turn", obj.m_designed_on_turn);
    if (version >= 2)
        ar  & make_nvp("m_designed_by_empire", obj.m_designed_by_empire);
    ar  & make_nvp("m_hull", obj.m_hull)
        & make_nvp("m_parts", obj.m_parts)
        & make_nvp("m_is_monster", obj.m_is_monster)
        & make_nvp("m_icon", obj.m_icon)
        & make_nvp("m_3D_model", obj.m_3D_model)
        & make_nvp("m_name_desc_in_stringtable", obj.m_name_desc_in_stringtable);
    if (Archive::is_loading::value) {
        obj.ForceValidDesignOrThrow(boost::none, true);
        obj.BuildStatCaches();
    }
}

BOOST_CLASS_EXPORT(ShipDesign)
BOOST_CLASS_VERSION(ShipDesign, 2)


template <typename Archive>
void serialize(Archive& ar, SpeciesManager& sm, unsigned int const version)
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
        species_homeworlds = sm.GetSpeciesHomeworldsMap(GlobalSerializationEncodingForEmpire());
        empire_opinions = sm.GetSpeciesEmpireOpinionsMap(GlobalSerializationEncodingForEmpire());
        other_species_opinions = sm.GetSpeciesSpeciesOpinionsMap(GlobalSerializationEncodingForEmpire());
        species_object_populations = sm.SpeciesObjectPopulations(GlobalSerializationEncodingForEmpire());
        species_ships_destroyed = sm.SpeciesShipsDestroyed(GlobalSerializationEncodingForEmpire());
    }

    ar  & BOOST_SERIALIZATION_NVP(species_homeworlds)
        & BOOST_SERIALIZATION_NVP(empire_opinions)
        & BOOST_SERIALIZATION_NVP(other_species_opinions)
        & BOOST_SERIALIZATION_NVP(species_object_populations)
        & BOOST_SERIALIZATION_NVP(species_ships_destroyed);

    if (Archive::is_loading::value) {
        sm.SetSpeciesHomeworlds(std::move(species_homeworlds));
        sm.SetSpeciesEmpireOpinions(std::move(empire_opinions));
        sm.SetSpeciesSpeciesOpinions(std::move(other_species_opinions));
        sm.m_species_object_populations = std::move(species_object_populations);
        sm.m_species_species_ships_destroyed = std::move(species_ships_destroyed);
    }
}

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, SpeciesManager&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, SpeciesManager&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, SpeciesManager&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, SpeciesManager&, unsigned int const);

template <typename Archive>
void Serialize(Archive& oa, const Universe& universe)
{ oa << BOOST_SERIALIZATION_NVP(universe); }
template FO_COMMON_API void Serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& oa, const Universe& universe);
template FO_COMMON_API void Serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& oa, const Universe& universe);

template <typename Archive>
void Serialize(Archive& oa, const std::map<int, std::shared_ptr<UniverseObject>>& objects)
{ oa << BOOST_SERIALIZATION_NVP(objects); }
template void Serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& oa, const std::map<int, std::shared_ptr<UniverseObject>>& objects);
template void Serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& oa, const std::map<int, std::shared_ptr<UniverseObject>>& objects);

template <typename Archive>
void Deserialize(Archive& ia, Universe& universe)
{ ia >> BOOST_SERIALIZATION_NVP(universe); }
template FO_COMMON_API void Deserialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ia, Universe& universe);
template FO_COMMON_API void Deserialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ia, Universe& universe);

template <typename Archive>
void Deserialize(Archive& ia, std::map<int, std::shared_ptr<UniverseObject>>& objects)
{ ia >> BOOST_SERIALIZATION_NVP(objects); }
template void Deserialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ia, std::map<int, std::shared_ptr<UniverseObject>>& objects);
template void Deserialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ia, std::map<int, std::shared_ptr<UniverseObject>>& objects);

namespace boost {
namespace serialization {
    template<class Archive, class Key, class Value>
    void save(Archive& ar, const flat_map<Key, Value>& m, const unsigned int)
    { stl::save_collection<Archive, flat_map<Key, Value>>(ar, m); }

    template<class Archive, class Key, class Value>
    void load(Archive& ar, flat_map<Key, Value>& m, const unsigned int)
    { load_map_collection(ar, m); }

    template<class Archive, class Key, class Value>
    void serialize(Archive& ar, flat_map<Key, Value>& m, const unsigned int file_version)
    { split_free(ar, m, file_version); }


    // Note: I tried loading the internal vector of a flat_map instead of
    //       loading it as a map and constructing elements on the stack.
    //       The result was similar or slightly slower than the stack loader.
    //
    //template<class Archive, class U, class Allocator>
    //inline void save(Archive& ar, const container::vector<U, Allocator>& t,
    //                 const unsigned int file_version)
    //{ stl::save_collection(ar, t); }

    //template<class Archive, class U, class Allocator>
    //inline void load(Archive& ar, container::vector<U, Allocator>& t,
    //                 const unsigned int file_version)
    //{
    //    item_version_type item_version(0);
    //    collection_size_type count;
    //    ar >> BOOST_SERIALIZATION_NVP(count);
    //    ar >> BOOST_SERIALIZATION_NVP(item_version);
    //    t.resize(count);

    //    using Container = container::vector<U, Allocator>;
    //    typedef typename Container::value_type type;

    //    for (auto& s : t)
    //        ar >> make_nvp("item", s);
    //}

    //template<class Archive, class U, class Allocator>
    //inline void serialize(Archive& ar, container::vector<U, Allocator>& t,
    //                      const unsigned int file_version)
    //{ split_free(ar, t, file_version); }
}}
