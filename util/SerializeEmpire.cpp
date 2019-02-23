#include "Serialize.h"

#include "AppInterface.h"
#include "SitRepEntry.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Supply.h"
#include "../Empire/Diplomacy.h"
#include "../universe/Universe.h"

#include "GameRules.h"

#include "Serialize.ipp"
#include <boost/serialization/version.hpp>


template <class Archive>
void ResearchQueue::Element::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(name)
        & BOOST_SERIALIZATION_NVP(empire_id)
        & BOOST_SERIALIZATION_NVP(allocated_rp)
        & BOOST_SERIALIZATION_NVP(turns_left)
        & BOOST_SERIALIZATION_NVP(paused);
}

template void ResearchQueue::Element::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void ResearchQueue::Element::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void ResearchQueue::Element::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void ResearchQueue::Element::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <class Archive>
void ResearchQueue::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_queue)
        & BOOST_SERIALIZATION_NVP(m_projects_in_progress)
        & BOOST_SERIALIZATION_NVP(m_total_RPs_spent)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template void ResearchQueue::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void ResearchQueue::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void ResearchQueue::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void ResearchQueue::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <class Archive>
void ProductionQueue::ProductionItem::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(build_type)
        & BOOST_SERIALIZATION_NVP(name)
        & BOOST_SERIALIZATION_NVP(design_id);
}

template void ProductionQueue::ProductionItem::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void ProductionQueue::ProductionItem::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void ProductionQueue::ProductionItem::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void ProductionQueue::ProductionItem::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <class Archive>
void ProductionQueue::Element::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(item)
        & BOOST_SERIALIZATION_NVP(empire_id)
        & BOOST_SERIALIZATION_NVP(ordered)
        & BOOST_SERIALIZATION_NVP(remaining)
        & BOOST_SERIALIZATION_NVP(blocksize)
        & BOOST_SERIALIZATION_NVP(location)
        & BOOST_SERIALIZATION_NVP(allocated_pp)
        & BOOST_SERIALIZATION_NVP(progress)
        & BOOST_SERIALIZATION_NVP(progress_memory)
        & BOOST_SERIALIZATION_NVP(blocksize_memory)
        & BOOST_SERIALIZATION_NVP(turns_left_to_next_item)
        & BOOST_SERIALIZATION_NVP(turns_left_to_completion)
        & BOOST_SERIALIZATION_NVP(rally_point_id)
        & BOOST_SERIALIZATION_NVP(paused)
        & BOOST_SERIALIZATION_NVP(allowed_imperial_stockpile_use);
}

template void ProductionQueue::Element::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void ProductionQueue::Element::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void ProductionQueue::Element::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void ProductionQueue::Element::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <class Archive>
void ProductionQueue::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_queue)
        & BOOST_SERIALIZATION_NVP(m_projects_in_progress)
        & BOOST_SERIALIZATION_NVP(m_object_group_allocated_pp)
        & BOOST_SERIALIZATION_NVP(m_object_group_allocated_stockpile_pp)
        & BOOST_SERIALIZATION_NVP(m_expected_new_stockpile_amount)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template void ProductionQueue::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void ProductionQueue::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void ProductionQueue::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void ProductionQueue::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <class Archive>
void Empire::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_id)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_player_name)
        & BOOST_SERIALIZATION_NVP(m_color)
        & BOOST_SERIALIZATION_NVP(m_capital_id)
        & BOOST_SERIALIZATION_NVP(m_source_id)
        & BOOST_SERIALIZATION_NVP(m_eliminated)
        & BOOST_SERIALIZATION_NVP(m_victories);

    bool visible = GetUniverse().AllObjectsVisible() ||
        GetUniverse().EncodingEmpire() == ALL_EMPIRES ||
        m_id == GetUniverse().EncodingEmpire();

    if (Archive::is_loading::value && version < 1) {
        // adapt set to map
        std::set<std::string> temp_stringset;
        ar  & boost::serialization::make_nvp("m_techs", temp_stringset);
        m_techs.clear();
        for (auto& entry : temp_stringset)
            m_techs[entry] = BEFORE_FIRST_TURN;
    } else if (Archive::is_saving::value && !visible && !GetGameRules().Get<bool>("RULE_SHOW_DETAILED_EMPIRES_DATA")) {
        std::map<std::string, int> dummy_string_int_map;
        // show other empire tech only if the current empire already knowns this tech without disclosure turn
        // this allows to see effects if the empire learned appropriate tech
        const Empire* encoding_empire = Empires().GetEmpire(GetUniverse().EncodingEmpire());
        if (encoding_empire) {
            for (const auto& tech : encoding_empire->m_techs) {
                const auto it = m_techs.find(tech.first);
                if (it != m_techs.end()) {
                    dummy_string_int_map.emplace(tech.first, BEFORE_FIRST_TURN);
                }
            }
        }
        ar  & boost::serialization::make_nvp("m_techs", dummy_string_int_map);
    } else {
        ar  & BOOST_SERIALIZATION_NVP(m_techs);
    }

    ar  & BOOST_SERIALIZATION_NVP(m_meters);
    if (Archive::is_saving::value && !visible && !GetGameRules().Get<bool>("RULE_SHOW_DETAILED_EMPIRES_DATA")) {
        // don't send what other empires building and researching
        // and which building and ship parts are available to them
        ResearchQueue empty_research_queue(m_id);
        std::map<std::string, float> empty_research_progress;
        ProductionQueue empty_production_queue(m_id);
        std::set<std::string> empty_string_set;
        ar  & boost::serialization::make_nvp("m_research_queue", empty_research_queue)
            & boost::serialization::make_nvp("m_research_progress", empty_research_progress)
            & boost::serialization::make_nvp("m_production_queue", empty_production_queue)
            & boost::serialization::make_nvp("m_available_building_types", empty_string_set)
            & boost::serialization::make_nvp("m_available_part_types", empty_string_set)
            & boost::serialization::make_nvp("m_available_hull_types", empty_string_set);
    } else {
        // processing all data on deserialization, saving to savegame,
        // sending data to the current empire itself,
        // or rule allows to see all data
        ar  & BOOST_SERIALIZATION_NVP(m_research_queue)
            & BOOST_SERIALIZATION_NVP(m_research_progress)
            & BOOST_SERIALIZATION_NVP(m_production_queue)
            & BOOST_SERIALIZATION_NVP(m_available_building_types)
            & BOOST_SERIALIZATION_NVP(m_available_part_types)
            & BOOST_SERIALIZATION_NVP(m_available_hull_types);
    }

    ar  & BOOST_SERIALIZATION_NVP(m_supply_system_ranges)
        & BOOST_SERIALIZATION_NVP(m_supply_unobstructed_systems)
        & BOOST_SERIALIZATION_NVP(m_preserved_system_exit_lanes);

    if (visible) {
        ar  & boost::serialization::make_nvp("m_ship_designs", m_known_ship_designs);
        ar  & BOOST_SERIALIZATION_NVP(m_sitrep_entries)
            & BOOST_SERIALIZATION_NVP(m_resource_pools)
            & BOOST_SERIALIZATION_NVP(m_population_pool)

            & BOOST_SERIALIZATION_NVP(m_explored_systems)
            & BOOST_SERIALIZATION_NVP(m_ship_names_used)

            & BOOST_SERIALIZATION_NVP(m_species_ships_owned)
            & BOOST_SERIALIZATION_NVP(m_ship_designs_owned)
            & BOOST_SERIALIZATION_NVP(m_ship_part_types_owned)
            & BOOST_SERIALIZATION_NVP(m_ship_part_class_owned)
            & BOOST_SERIALIZATION_NVP(m_species_colonies_owned)
            & BOOST_SERIALIZATION_NVP(m_outposts_owned)
            & BOOST_SERIALIZATION_NVP(m_building_types_owned)

            & BOOST_SERIALIZATION_NVP(m_empire_ships_destroyed)
            & BOOST_SERIALIZATION_NVP(m_ship_designs_destroyed)
            & BOOST_SERIALIZATION_NVP(m_species_ships_destroyed)
            & BOOST_SERIALIZATION_NVP(m_species_planets_invaded)

            & BOOST_SERIALIZATION_NVP(m_species_ships_produced)
            & BOOST_SERIALIZATION_NVP(m_ship_designs_produced)
            & BOOST_SERIALIZATION_NVP(m_species_ships_lost)
            & BOOST_SERIALIZATION_NVP(m_ship_designs_lost)
            & BOOST_SERIALIZATION_NVP(m_species_ships_scrapped)
            & BOOST_SERIALIZATION_NVP(m_ship_designs_scrapped)

            & BOOST_SERIALIZATION_NVP(m_species_planets_depoped)
            & BOOST_SERIALIZATION_NVP(m_species_planets_bombed)

            & BOOST_SERIALIZATION_NVP(m_building_types_produced)
            & BOOST_SERIALIZATION_NVP(m_building_types_scrapped);
    }

    if (Archive::is_loading::value && version < 3) {
        m_authenticated = false;
    } else {
        ar  & BOOST_SERIALIZATION_NVP(m_authenticated);
    }
}

BOOST_CLASS_VERSION(Empire, 3)

template void Empire::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void Empire::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void Empire::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void Empire::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <class Archive>
void EmpireManager::serialize(Archive& ar, const unsigned int version)
{
    if (Archive::is_loading::value) {
        Clear();    // clean up any existing dynamically allocated contents before replacing containers with deserialized data
    }

    std::map<std::pair<int, int>, DiplomaticMessage> messages;
    if (Archive::is_saving::value)
        GetDiplomaticMessagesToSerialize(messages, GetUniverse().EncodingEmpire());

    ar  & BOOST_SERIALIZATION_NVP(m_empire_map)
        & BOOST_SERIALIZATION_NVP(m_empire_diplomatic_statuses)
        & BOOST_SERIALIZATION_NVP(messages);

    if (Archive::is_loading::value)
        m_diplomatic_messages = std::move(messages);
}

template void EmpireManager::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void EmpireManager::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void EmpireManager::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void EmpireManager::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <class Archive>
void DiplomaticMessage::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_sender_empire)
        & BOOST_SERIALIZATION_NVP(m_recipient_empire)
        & BOOST_SERIALIZATION_NVP(m_type);
}

template void DiplomaticMessage::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void DiplomaticMessage::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void DiplomaticMessage::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void DiplomaticMessage::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <class Archive>
void SupplyManager::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_supply_starlane_traversals)
        & BOOST_SERIALIZATION_NVP(m_supply_starlane_obstructed_traversals)
        & BOOST_SERIALIZATION_NVP(m_fleet_supplyable_system_ids)
        & BOOST_SERIALIZATION_NVP(m_resource_supply_groups)
        & BOOST_SERIALIZATION_NVP(m_propagated_supply_ranges)
        & BOOST_SERIALIZATION_NVP(m_empire_propagated_supply_ranges)
        & BOOST_SERIALIZATION_NVP(m_propagated_supply_distances)
        & BOOST_SERIALIZATION_NVP(m_empire_propagated_supply_distances);
}

template void SupplyManager::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void SupplyManager::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void SupplyManager::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void SupplyManager::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);
