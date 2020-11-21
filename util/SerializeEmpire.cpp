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
#include <boost/serialization/array.hpp>
#include <boost/serialization/version.hpp>
#include <boost/uuid/random_generator.hpp>


template <typename Archive>
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

template <typename Archive>
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

template <typename Archive>
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

template <typename Archive>
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

    if (Archive::is_saving::value) {
        // Serialization of uuid as a primitive doesn't work as expected from
        // the documentation.  This workaround instead serializes a string
        // representation.
        auto string_uuid = boost::uuids::to_string(uuid);
        ar & BOOST_SERIALIZATION_NVP(string_uuid);

     } else if (Archive::is_loading::value && version < 2) {
        // assign a random ID to this element so that future-issued orders can refer to it
        uuid = boost::uuids::random_generator()();

    } else {
        // convert string back into UUID
        std::string string_uuid;
        ar & BOOST_SERIALIZATION_NVP(string_uuid);

        try {
            uuid = boost::lexical_cast<boost::uuids::uuid>(string_uuid);
        } catch (const boost::bad_lexical_cast&) {
            uuid = boost::uuids::random_generator()();
        }
    }
}

BOOST_CLASS_VERSION(ProductionQueue::Element, 2)

template void ProductionQueue::Element::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void ProductionQueue::Element::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void ProductionQueue::Element::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void ProductionQueue::Element::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <typename Archive>
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

template <typename Archive>
void InfluenceQueue::Element::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(name)
        & BOOST_SERIALIZATION_NVP(empire_id)
        & BOOST_SERIALIZATION_NVP(allocated_ip)
        & BOOST_SERIALIZATION_NVP(paused);
}

template void InfluenceQueue::Element::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void InfluenceQueue::Element::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void InfluenceQueue::Element::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void InfluenceQueue::Element::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <class Archive>
void InfluenceQueue::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_queue)
        & BOOST_SERIALIZATION_NVP(m_projects_in_progress)
        & BOOST_SERIALIZATION_NVP(m_total_IPs_spent)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template void InfluenceQueue::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void InfluenceQueue::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void InfluenceQueue::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void InfluenceQueue::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <class Archive>
void Empire::PolicyAdoptionInfo::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(adoption_turn)
        & BOOST_SERIALIZATION_NVP(category)
        & BOOST_SERIALIZATION_NVP(slot_in_category);
}

template void Empire::PolicyAdoptionInfo::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void Empire::PolicyAdoptionInfo::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void Empire::PolicyAdoptionInfo::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void Empire::PolicyAdoptionInfo::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <class Archive>
void Empire::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_id)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_player_name);
    if (Archive::is_loading::value && version < 5) {
        CompatColor old_color;
        ar & boost::serialization::make_nvp("m_color", old_color);
        m_color = {old_color.r, old_color.g, old_color.b, old_color.a};
    } else {
        ar & BOOST_SERIALIZATION_NVP(m_color);
    }
    ar  & BOOST_SERIALIZATION_NVP(m_capital_id)
        & BOOST_SERIALIZATION_NVP(m_source_id)
        & BOOST_SERIALIZATION_NVP(m_eliminated)
        & BOOST_SERIALIZATION_NVP(m_victories);


    auto encoding_empire = GlobalSerializationEncodingForEmpire();
    bool visible =
        (ALL_EMPIRES == encoding_empire) ||
        (m_id == encoding_empire); // TODO: GameRule for all objects visible
    bool allied_visible = visible ||
        Empires().GetDiplomaticStatus(m_id, GlobalSerializationEncodingForEmpire()) ==
            DiplomaticStatus::DIPLO_ALLIED;

    TraceLogger() << "serializing empire " << m_id << ": " << m_name;
    TraceLogger() << "encoding empire: " << encoding_empire;
    TraceLogger() << std::string(visible ? "visible" : "NOT visible") << "  /  "
                  << std::string(allied_visible ? "allied visible" : "NOT allied visible");

    if (Archive::is_loading::value && version < 1) {
        // adapt set to map
        std::set<std::string> temp_stringset;
        ar  & boost::serialization::make_nvp("m_techs", temp_stringset);
        m_techs.clear();
        for (auto& entry : temp_stringset)
            m_techs[entry] = BEFORE_FIRST_TURN;
    } else {
        ar  & BOOST_SERIALIZATION_NVP(m_techs);

        if (Archive::is_loading::value && version < 4) {
            m_adopted_policies.clear();
            m_initial_adopted_policies.clear();
            m_available_policies.clear();
            m_policy_adoption_total_duration.clear();
        } else {
            ar  & BOOST_SERIALIZATION_NVP(m_adopted_policies)
                & BOOST_SERIALIZATION_NVP(m_initial_adopted_policies)
                & BOOST_SERIALIZATION_NVP(m_available_policies)
                & BOOST_SERIALIZATION_NVP(m_policy_adoption_total_duration);
        }
    }

    ar  & BOOST_SERIALIZATION_NVP(m_meters);
    if (Archive::is_saving::value && !allied_visible) {
        // don't send what other empires building and researching
        // and which building and ship parts are available to them
        ResearchQueue empty_research_queue(m_id);
        std::map<std::string, float> empty_research_progress;
        ProductionQueue empty_production_queue(m_id);
        std::set<std::string> empty_string_set;
        InfluenceQueue empty_influence_queue(m_id);
        ar  & boost::serialization::make_nvp("m_research_queue", empty_research_queue)
            & boost::serialization::make_nvp("m_research_progress", empty_research_progress)
            & boost::serialization::make_nvp("m_production_queue", empty_production_queue)
            & boost::serialization::make_nvp("m_influence_queue", empty_influence_queue)
            & boost::serialization::make_nvp("m_available_building_types", empty_string_set)
            & boost::serialization::make_nvp("m_available_part_types", empty_string_set)
            & boost::serialization::make_nvp("m_available_hull_types", empty_string_set);
    } else {
        // processing all data on deserialization, saving to savegame,
        // or sending data to the current empire itself
        ar  & BOOST_SERIALIZATION_NVP(m_research_queue)
            & BOOST_SERIALIZATION_NVP(m_research_progress)
            & BOOST_SERIALIZATION_NVP(m_production_queue)
            & BOOST_SERIALIZATION_NVP(m_influence_queue)
            & BOOST_SERIALIZATION_NVP(m_available_building_types)
            & boost::serialization::make_nvp("m_available_part_types", m_available_ship_parts)
            & boost::serialization::make_nvp("m_available_hull_types", m_available_ship_hulls);
    }

    ar  & BOOST_SERIALIZATION_NVP(m_supply_system_ranges)
        & BOOST_SERIALIZATION_NVP(m_supply_unobstructed_systems)
        & BOOST_SERIALIZATION_NVP(m_preserved_system_exit_lanes);

    if (visible) {
        try {
        ar  & boost::serialization::make_nvp("m_ship_designs", m_known_ship_designs);
        ar  & BOOST_SERIALIZATION_NVP(m_sitrep_entries)
            & BOOST_SERIALIZATION_NVP(m_resource_pools)
            & BOOST_SERIALIZATION_NVP(m_population_pool)

            & BOOST_SERIALIZATION_NVP(m_explored_systems)
            & BOOST_SERIALIZATION_NVP(m_ship_names_used)

            & BOOST_SERIALIZATION_NVP(m_species_ships_owned)
            & BOOST_SERIALIZATION_NVP(m_ship_designs_owned)
            & boost::serialization::make_nvp("m_ship_part_types_owned", m_ship_parts_owned)
            & BOOST_SERIALIZATION_NVP(m_ship_part_class_owned)
            & BOOST_SERIALIZATION_NVP(m_species_colonies_owned)
            & BOOST_SERIALIZATION_NVP(m_outposts_owned)
            & BOOST_SERIALIZATION_NVP(m_building_types_owned)

            & BOOST_SERIALIZATION_NVP(m_empire_ships_destroyed)
            & BOOST_SERIALIZATION_NVP(m_ship_designs_destroyed)
            & BOOST_SERIALIZATION_NVP(m_species_ships_destroyed)
            & BOOST_SERIALIZATION_NVP(m_species_planets_invaded)

            & BOOST_SERIALIZATION_NVP(m_ship_designs_in_production)
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
        } catch (...) {
            ErrorLogger() << "Empire::serialize failed to (de)serialize stuff for visible empire";
        }
    }

    if (Archive::is_loading::value && version < 3) {
        m_authenticated = false;
    } else {
        ar  & BOOST_SERIALIZATION_NVP(m_authenticated);
    }

    if (Archive::is_loading::value && version < 4) {
        m_ready = false;
    } else {
        ar  & BOOST_SERIALIZATION_NVP(m_ready);
    }

    if (Archive::is_loading::value && version < 5) {
        m_auto_turn_count = 0;
    } else {
        ar  & BOOST_SERIALIZATION_NVP(m_auto_turn_count);
    }

    TraceLogger() << "DONE serializing empire " << m_id << ": " << m_name;
}

BOOST_CLASS_VERSION(Empire, 6)

template void Empire::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void Empire::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void Empire::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void Empire::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);


namespace {
    std::pair<int, int> DiploKey(int id1, int ind2)
    { return std::make_pair(std::max(id1, ind2), std::min(id1, ind2)); }
}

template <typename Archive>
void serialize(Archive& ar, EmpireManager& em, unsigned int const version)
{
    using boost::serialization::make_nvp;

    TraceLogger() << "Serializing EmpireManager encoding empire: " << GlobalSerializationEncodingForEmpire();

    if (Archive::is_loading::value) {
        em.Clear();    // clean up any existing dynamically allocated contents before replacing containers with deserialized data
    }

    std::map<std::pair<int, int>, DiplomaticMessage> messages;
    if (Archive::is_saving::value)
        em.GetDiplomaticMessagesToSerialize(messages, GlobalSerializationEncodingForEmpire());

    TraceLogger() << "EmpireManager version : " << version;
    if (Archive::is_loading::value && version < 1) {
        std::map<int, Empire*> empire_raw_ptr_map;
        ar  & make_nvp("m_empire_map", empire_raw_ptr_map);
        TraceLogger() << "EmpireManager deserialized " << empire_raw_ptr_map.size() << " raw pointer empires:";
        for (const auto& entry : empire_raw_ptr_map)
            TraceLogger() << entry.second->Name() << " (" << entry.first << ")";

        for (const auto& entry : empire_raw_ptr_map)
            em.m_empire_map[entry.first] = std::shared_ptr<Empire>(entry.second);
        TraceLogger() << "EmpireManager put raw pointers into shared_ptr";

    } else if (Archive::is_loading::value && version < 2) {
        ar  & make_nvp("m_empire_map", em.m_empire_map);
        TraceLogger() << "EmpireManager serialized " << em.m_empire_map.size() << " empires";
        ar  & make_nvp("m_empire_diplomatic_statuses", em.m_empire_diplomatic_statuses);

    } else {
        ar  & make_nvp("m_empire_diplomatic_statuses", em.m_empire_diplomatic_statuses);
        ar  & make_nvp("m_empire_map", em.m_empire_map);
        TraceLogger() << "EmpireManager serialized " << em.m_empire_map.size() << " empires";
    }
    ar  & BOOST_SERIALIZATION_NVP(messages);

    if (Archive::is_loading::value) {
        for (const auto& entry : em.m_empire_map)
            em.m_const_empire_map.emplace(entry.first, entry.second);

        em.m_diplomatic_messages = std::move(messages);

        // erase invalid empire diplomatic statuses
        std::vector<std::pair<int, int>> to_erase;
        for (auto r : em.m_empire_diplomatic_statuses) {
            const auto& e1 = r.first.first;
            const auto& e2 = r.first.second;
            if (em.m_empire_map.count(e1) < 1 || em.m_empire_map.count(e2) < 1) {
                to_erase.emplace_back(e1, e2);
                ErrorLogger() << "Erased invalid diplomatic status between empires " << e1 << " and " << e2;
            }
        }
        for (auto p : to_erase)
            em.m_empire_diplomatic_statuses.erase(p);

        // add missing empire diplomatic statuses
        for (const auto& e1 : em.m_empire_map) {
            for (const auto& e2 : em.m_empire_map) {
                if (e1.first >= e2.first)
                    continue;
                auto dk = DiploKey(e1.first, e2.first);
                if (em.m_empire_diplomatic_statuses.count(dk) < 1) {
                    em.m_empire_diplomatic_statuses[dk] = DiplomaticStatus::DIPLO_WAR;
                    ErrorLogger() << "Added missing diplomatic status (default WAR) between empires " << e1.first << " and " << e2.first;
                }
            }
        }
    }
}

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, EmpireManager&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, EmpireManager&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, EmpireManager&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, EmpireManager&, unsigned int const);


template <typename Archive>
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

template <typename Archive>
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
