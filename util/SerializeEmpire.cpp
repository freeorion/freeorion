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

    if (Archive::is_loading::value && version < 3) {
        to_be_removed = false;
    } else {
        ar  & BOOST_SERIALIZATION_NVP(to_be_removed);
    }

    if constexpr (Archive::is_saving::value) {
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

BOOST_CLASS_VERSION(ProductionQueue::Element, 3)

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
    ar  & BOOST_SERIALIZATION_NVP(m_color);
    ar  & BOOST_SERIALIZATION_NVP(m_capital_id)
        & BOOST_SERIALIZATION_NVP(m_source_id)
        & BOOST_SERIALIZATION_NVP(m_eliminated);
    if (Archive::is_loading::value && version < 13) {
        std::set<std::string> victories;
        ar  & boost::serialization::make_nvp("m_victories", victories);
        m_victories.clear();
        m_victories.insert(boost::container::ordered_unique_range, victories.begin(), victories.end());
    } else {
        ar  & BOOST_SERIALIZATION_NVP(m_victories);
    }


    auto encoding_empire = GlobalSerializationEncodingForEmpire();
    bool visible =
        (ALL_EMPIRES == encoding_empire) ||
        (m_id == encoding_empire); // TODO: GameRule for all empire info known to other empires
    bool allied_visible = visible;
    if constexpr (Archive::is_saving::value)
        allied_visible = allied_visible || Empires().GetDiplomaticStatus(m_id, GlobalSerializationEncodingForEmpire()) ==
            DiplomaticStatus::DIPLO_ALLIED; // TODO: pass in diplo status map?

    TraceLogger() << "serializing empire " << m_id << ": " << m_name;
    TraceLogger() << "encoding empire: " << encoding_empire;
    if constexpr (Archive::is_loading::value) {
        TraceLogger() << std::string(visible ? "visible" : "NOT visible");
    } else {
        TraceLogger() << std::string(visible ? "visible" : "NOT visible") << "  /  "
                      << std::string(allied_visible ? "allied visible" : "NOT allied visible");
    }

    if (Archive::is_loading::value && version < 11) {
        std::map<std::string, int> techs;
        ar  & boost::serialization::make_nvp("m_techs", techs);
        m_techs.insert(boost::container::ordered_unique_range, techs.begin(), techs.end());

    } else if (Archive::is_loading::value && version < 13) {
        std::map<std::string, int, std::less<>> techs;
        ar  & boost::serialization::make_nvp("m_techs", techs);
        m_techs.insert(boost::container::ordered_unique_range, techs.begin(), techs.end());

    } else {
        ar  & BOOST_SERIALIZATION_NVP(m_techs);
    }

    if (Archive::is_loading::value && version < 10) {
        std::map<std::string, PolicyAdoptionInfo> adopted_policies;
        std::map<std::string, PolicyAdoptionInfo> initial_adopted_policies;
        std::set<std::string>                     available_policies;

        ar  & boost::serialization::make_nvp("m_adopted_policies", adopted_policies);
        m_adopted_policies.clear();
        m_adopted_policies.insert(adopted_policies.begin(), adopted_policies.end());

        ar  & boost::serialization::make_nvp("m_initial_adopted_policies", initial_adopted_policies);
        m_initial_adopted_policies.clear();
        m_initial_adopted_policies.insert(initial_adopted_policies.begin(), initial_adopted_policies.end());

        ar  & boost::serialization::make_nvp("m_available_policies", available_policies);
        m_available_policies.clear();
        m_available_policies.insert(available_policies.begin(), available_policies.end());

    } else {
        ar  & BOOST_SERIALIZATION_NVP(m_adopted_policies)
            & BOOST_SERIALIZATION_NVP(m_initial_adopted_policies)
            & BOOST_SERIALIZATION_NVP(m_available_policies);
    }

    ar  & BOOST_SERIALIZATION_NVP(m_policy_adoption_total_duration);

    if (Archive::is_loading::value && version < 7) {
        const auto* app = IApp::GetApp();
        const auto current_turn = app ? app->CurrentTurn() : INVALID_GAME_TURN;

        m_policy_adoption_current_duration.clear();
        for (auto& [policy_name, adoption_info] : m_adopted_policies) {
            m_policy_adoption_current_duration[policy_name] =
                app ? (current_turn - adoption_info.adoption_turn) : 0;
        }
    } else {
        ar  & BOOST_SERIALIZATION_NVP(m_policy_adoption_current_duration);
    }

    if (Archive::is_loading::value && version < 11) {
        std::map<std::string, Meter> meters;
        ar  & boost::serialization::make_nvp("m_meters", meters);
        m_meters.insert(boost::container::ordered_unique_range, meters.begin(), meters.end());

    } else if (Archive::is_loading::value && version < 13) {
        std::vector<std::pair<std::string, Meter>> meters;
        ar  & boost::serialization::make_nvp("m_meters", meters);
        std::sort(meters.begin(), meters.end());
        m_meters.insert(std::make_move_iterator(meters.begin()), std::make_move_iterator(meters.end()));

    } else {
        ar  & BOOST_SERIALIZATION_NVP(m_meters);
    }

    if (Archive::is_saving::value && !allied_visible) {
        // don't send what other empires building and researching
        // and which building and ship parts are available to them
        ResearchQueue empty_research_queue(m_id);
        decltype(this->m_research_progress) empty_research_progress;
        ProductionQueue empty_production_queue(m_id);
        decltype(this->m_available_building_types) empty_string_set;
        static_assert(std::is_same_v<decltype(empty_string_set), decltype(this->m_available_ship_parts)>);
        static_assert(std::is_same_v<decltype(empty_string_set), decltype(this->m_available_ship_hulls)>);
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
            & BOOST_SERIALIZATION_NVP(m_influence_queue);

        if (Archive::is_loading::value && version < 13) {
            std::set<std::string> buf;
            ar  & boost::serialization::make_nvp("m_available_building_types", buf);
            m_available_building_types.insert(boost::container::ordered_unique_range, buf.begin(), buf.end());
            ar  & boost::serialization::make_nvp("m_available_part_types", buf);
            m_available_ship_parts.insert(boost::container::ordered_unique_range, buf.begin(), buf.end());
            ar  & boost::serialization::make_nvp("m_available_hull_types", buf);
            m_available_ship_hulls.insert(boost::container::ordered_unique_range, buf.begin(), buf.end());

        } else {
            ar  & BOOST_SERIALIZATION_NVP(m_available_building_types)
                & BOOST_SERIALIZATION_NVP(m_available_ship_parts)
                & BOOST_SERIALIZATION_NVP(m_available_ship_hulls);
        }
    }

    ar  & BOOST_SERIALIZATION_NVP(m_supply_system_ranges);
    ar  & BOOST_SERIALIZATION_NVP(m_supply_unobstructed_systems);
    ar  & BOOST_SERIALIZATION_NVP(m_preserved_system_exit_lanes);

    if (visible) {
        try {
            ar  & boost::serialization::make_nvp("m_ship_designs", m_known_ship_designs);
            ar  & BOOST_SERIALIZATION_NVP(m_sitrep_entries);
            if (Archive::is_loading::value && version < 12) {
                std::map<ResourceType, std::shared_ptr<ResourcePool>> scratch;
                ar  & boost::serialization::make_nvp("m_resource_pools", scratch);
                auto it = scratch.find(ResourceType::RE_INDUSTRY);
                if (it != scratch.end())
                    m_industry_pool = std::move(*it->second);
                it = scratch.find(ResourceType::RE_RESEARCH);
                if (it != scratch.end())
                    m_research_pool = std::move(*it->second);
                it = scratch.find(ResourceType::RE_INFLUENCE);
                if (it != scratch.end())
                    m_influence_pool = std::move(*it->second);

            } else {
                ar  & BOOST_SERIALIZATION_NVP(m_industry_pool)
                    & BOOST_SERIALIZATION_NVP(m_research_pool)
                    & BOOST_SERIALIZATION_NVP(m_influence_pool);
            }
            ar  & BOOST_SERIALIZATION_NVP(m_population_pool);

            if (Archive::is_loading::value && version < 8) {
                std::set<int> explored_system_ids;
                ar  & boost::serialization::make_nvp("m_explored_systems", explored_system_ids);
                m_explored_systems.clear();
                for (auto id : explored_system_ids)
                    m_explored_systems.emplace(id, 0);
            } else {
                ar  & BOOST_SERIALIZATION_NVP(m_explored_systems);
            }

            ar  & BOOST_SERIALIZATION_NVP(m_ship_names_used)
                & BOOST_SERIALIZATION_NVP(m_species_ships_owned)
                & BOOST_SERIALIZATION_NVP(m_ship_designs_owned)
                & boost::serialization::make_nvp("m_ship_part_types_owned", m_ship_parts_owned)
                & BOOST_SERIALIZATION_NVP(m_ship_part_class_owned)
                & BOOST_SERIALIZATION_NVP(m_species_colonies_owned)
                & BOOST_SERIALIZATION_NVP(m_outposts_owned)
                & BOOST_SERIALIZATION_NVP(m_building_types_owned);

            if (Archive::is_loading::value && version < 9) {
                m_ships_destroyed.clear();
            } else {
                ar  & BOOST_SERIALIZATION_NVP(m_ships_destroyed);
            }

            ar  & BOOST_SERIALIZATION_NVP(m_empire_ships_destroyed)
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
        } catch (const std::exception& e) {
            ErrorLogger() << "Empire::serialize failed to (de)serialize stuff for visible empire: " << e.what();
        }
    }

    ar  & BOOST_SERIALIZATION_NVP(m_authenticated);
    ar  & BOOST_SERIALIZATION_NVP(m_ready);
    ar  & BOOST_SERIALIZATION_NVP(m_auto_turn_count);

    if (Archive::is_loading::value && version < 13) {
        m_last_turn_received = INVALID_GAME_TURN;
    } else {
        ar  & BOOST_SERIALIZATION_NVP(m_last_turn_received);
    }

}

BOOST_CLASS_VERSION(Empire, 13)

template void Empire::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void Empire::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void Empire::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void Empire::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);


namespace {
    std::pair<int, int> DiploKey(int id1, int ind2)
    { return std::pair(std::max(id1, ind2), std::min(id1, ind2)); }
}

template <typename Archive>
void serialize(Archive& ar, EmpireManager& em, unsigned int const version)
{
    using boost::serialization::make_nvp;

    TraceLogger() << "Serializing EmpireManager encoding empire: " << GlobalSerializationEncodingForEmpire();

    if constexpr (Archive::is_loading::value)
        em.Clear();    // clean up any existing dynamically allocated contents before replacing containers with deserialized data

    std::map<std::pair<int, int>, DiplomaticMessage> messages;
    if constexpr (Archive::is_saving::value)
        em.GetDiplomaticMessagesToSerialize(messages, GlobalSerializationEncodingForEmpire());

    TraceLogger() << "EmpireManager version : " << version;
    if (Archive::is_loading::value && version < 1) {
        std::map<int, Empire*> empire_raw_ptr_map;
        ar  & make_nvp("m_empire_map", empire_raw_ptr_map);
        for (const auto& [empire_id, empire_ptr] : empire_raw_ptr_map)
            em.m_empire_map[empire_id] = std::shared_ptr<Empire>(empire_ptr);

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

    if constexpr (Archive::is_loading::value) {
        for (auto& [empire_id, empire_ptr] : em.m_empire_map) {
            em.m_const_empire_map.emplace(empire_id, empire_ptr);
            em.m_empire_ids.push_back(empire_id);
        }
        std::sort(em.m_empire_ids.begin(), em.m_empire_ids.end());

        em.RefreshCapitalIDs();

        em.m_diplomatic_messages = std::move(messages);

        // erase invalid empire diplomatic statuses
        std::vector<std::pair<int, int>> to_erase;
        for (auto& [ids, diplo_status] : em.m_empire_diplomatic_statuses) {
            (void)diplo_status; // quiet unused warning
            const auto& [e1, e2] = ids;
            if (!em.m_empire_map.contains(e1) || !em.m_empire_map.contains(e2)) {
                to_erase.emplace_back(e1, e2);
                ErrorLogger() << "Erased invalid diplomatic status between empires " << e1 << " and " << e2;
            }
        }
        for (auto p : to_erase)
            em.m_empire_diplomatic_statuses.erase(p);

        // add missing empire diplomatic statuses
        for (const auto& [e1_id, e1_ptr] : em.m_empire_map) {
            (void)e1_ptr; // quiet warning
            for (const auto& [e2_id, e2_ptr] : em.m_empire_map) {
                (void)e2_ptr; // quiet warning
                if (e1_id >= e2_id)
                    continue;
                auto dk = DiploKey(e1_id, e2_id);
                if (!em.m_empire_diplomatic_statuses.contains(dk)) {
                    em.m_empire_diplomatic_statuses[dk] = DiplomaticStatus::DIPLO_WAR;
                    ErrorLogger() << "Added missing diplomatic status (default WAR) between empires "
                                  << e1_id << " and " << e2_id;
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
