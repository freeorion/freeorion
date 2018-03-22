#include "Serialize.h"

#include "MultiplayerCommon.h"
#include "../universe/System.h"

#include "Serialize.ipp"

#include <random>
#include <boost/date_time/posix_time/time_serialize.hpp>
#include <boost/format.hpp>

template <class Archive>
void GalaxySetupData::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_seed)
        & BOOST_SERIALIZATION_NVP(m_size)
        & BOOST_SERIALIZATION_NVP(m_shape)
        & BOOST_SERIALIZATION_NVP(m_age)
        & BOOST_SERIALIZATION_NVP(m_starlane_freq)
        & BOOST_SERIALIZATION_NVP(m_planet_density)
        & BOOST_SERIALIZATION_NVP(m_specials_freq)
        & BOOST_SERIALIZATION_NVP(m_monster_freq)
        & BOOST_SERIALIZATION_NVP(m_native_freq)
        & BOOST_SERIALIZATION_NVP(m_ai_aggr);

    if (version >= 1) {
        ar & BOOST_SERIALIZATION_NVP(m_game_rules);
    }

    if (version >= 2) {
        ar & BOOST_SERIALIZATION_NVP(m_game_uid);
    } else {
        if (Archive::is_loading::value) {
            std::default_random_engine generator;
            std::uniform_int_distribution<int> distribution(0,999);

            m_game_uid = m_seed + (boost::format("%03i") % distribution(generator)).str();
        }
    }
}

template void GalaxySetupData::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void GalaxySetupData::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void GalaxySetupData::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void GalaxySetupData::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <class Archive>
void SinglePlayerSetupData::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GalaxySetupData)
        & BOOST_SERIALIZATION_NVP(m_new_game)
        & BOOST_SERIALIZATION_NVP(m_filename)
        & BOOST_SERIALIZATION_NVP(m_players);
}

template void SinglePlayerSetupData::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void SinglePlayerSetupData::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void SinglePlayerSetupData::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void SinglePlayerSetupData::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <class Archive>
void SaveGameUIData::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(map_top)
        & BOOST_SERIALIZATION_NVP(map_left)
        & BOOST_SERIALIZATION_NVP(map_zoom_steps_in)
        & BOOST_SERIALIZATION_NVP(fleets_exploring);

    if (version >= 2) {
        ar & BOOST_SERIALIZATION_NVP(obsolete_ui_event_count);
        ar & BOOST_SERIALIZATION_NVP(ordered_ship_design_ids_and_obsolete);
        ar & BOOST_SERIALIZATION_NVP(ordered_ship_hull_and_obsolete);
        ar & BOOST_SERIALIZATION_NVP(obsolete_ship_parts);
    }

    // Only workarounds for old versions follow
    if (version >= 2)
        return;

    legacy_serialize(ar, version);
}

template <class Archive>
void SaveGameUIData::legacy_serialize(Archive& ar, const unsigned int version)
{
    if (!Archive::is_loading::value)
        return;

    if (version == 1) {
        obsolete_ui_event_count = 1;

        std::vector<std::pair<int, boost::optional<bool>>> dummy_ordered_ship_design_ids_and_obsolete;
        ar & boost::serialization::make_nvp(
            "ordered_ship_design_ids_and_obsolete", dummy_ordered_ship_design_ids_and_obsolete);

        std::vector<std::pair<std::string, bool>> dummy_ordered_ship_hull_and_obsolete;
        ar & boost::serialization::make_nvp(
            "ordered_ship_hull_and_obsolete", dummy_ordered_ship_hull_and_obsolete);
        ordered_ship_hull_and_obsolete.clear();
        ordered_ship_hull_and_obsolete.reserve(dummy_ordered_ship_hull_and_obsolete.size());
        for (auto name_and_obsolete : dummy_ordered_ship_hull_and_obsolete) {
            ordered_ship_hull_and_obsolete.push_back(
                {name_and_obsolete.first, {name_and_obsolete.second, ++obsolete_ui_event_count}});
        }

        std::unordered_set<std::string> dummy_obsolete_ship_parts;
        ar & boost::serialization::make_nvp(
            "obsolete_ship_parts", dummy_obsolete_ship_parts);
        obsolete_ship_parts.clear();
        obsolete_ship_parts.reserve(dummy_obsolete_ship_parts.size());
        for (auto part : dummy_obsolete_ship_parts) {
            obsolete_ship_parts.insert({part, ++obsolete_ui_event_count});
        }

        // Insert designs last to preserve version 1 behavior
        ordered_ship_design_ids_and_obsolete.clear();
        ordered_ship_design_ids_and_obsolete.reserve(dummy_ordered_ship_design_ids_and_obsolete.size());
        for (auto id_and_obsolete : dummy_ordered_ship_design_ids_and_obsolete) {
            ordered_ship_design_ids_and_obsolete.push_back(
                std::make_pair(id_and_obsolete.first,
                               (id_and_obsolete.second
                                ? boost::optional<std::pair<bool, int>>({true, ++obsolete_ui_event_count})
                                : boost::none)));
        }
    } else {
        std::vector<std::pair<int, bool>> dummy_ordered_ship_design_ids_and_obsolete;
        ar & boost::serialization::make_nvp(
            "ordered_ship_design_ids_and_obsolete", dummy_ordered_ship_design_ids_and_obsolete);

        ordered_ship_design_ids_and_obsolete.clear();
        ordered_ship_design_ids_and_obsolete.reserve(dummy_ordered_ship_design_ids_and_obsolete.size());
        for (auto id_and_obsolete : dummy_ordered_ship_design_ids_and_obsolete) {
            ordered_ship_design_ids_and_obsolete.push_back(
                std::make_pair(id_and_obsolete.first,
                               (id_and_obsolete.second
                                ? boost::optional<std::pair<bool, int>>({true, ++obsolete_ui_event_count})
                                : boost::none)));
        }
        ordered_ship_hull_and_obsolete.clear();
        obsolete_ship_parts.clear();
    }
}

template void SaveGameUIData::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void SaveGameUIData::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void SaveGameUIData::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void SaveGameUIData::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <class Archive>
void SaveGameEmpireData::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_empire_name)
        & BOOST_SERIALIZATION_NVP(m_player_name)
        & BOOST_SERIALIZATION_NVP(m_color);
}

template void SaveGameEmpireData::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void SaveGameEmpireData::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void SaveGameEmpireData::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void SaveGameEmpireData::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <class Archive>
void PlayerSetupData::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_player_name)
        & BOOST_SERIALIZATION_NVP(m_player_id)
        & BOOST_SERIALIZATION_NVP(m_empire_name)
        & BOOST_SERIALIZATION_NVP(m_empire_color)
        & BOOST_SERIALIZATION_NVP(m_starting_species_name)
        & BOOST_SERIALIZATION_NVP(m_save_game_empire_id)
        & BOOST_SERIALIZATION_NVP(m_client_type)
        & BOOST_SERIALIZATION_NVP(m_player_ready);
}

template void PlayerSetupData::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void PlayerSetupData::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void PlayerSetupData::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void PlayerSetupData::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <class Archive>
void MultiplayerLobbyData::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GalaxySetupData)
        & BOOST_SERIALIZATION_NVP(m_new_game)
        & BOOST_SERIALIZATION_NVP(m_players)
        & BOOST_SERIALIZATION_NVP(m_save_game)
        & BOOST_SERIALIZATION_NVP(m_save_game_empire_data)
        & BOOST_SERIALIZATION_NVP(m_any_can_edit)
        & BOOST_SERIALIZATION_NVP(m_start_locked)
        & BOOST_SERIALIZATION_NVP(m_start_lock_cause);
}

template void MultiplayerLobbyData::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void MultiplayerLobbyData::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void MultiplayerLobbyData::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void MultiplayerLobbyData::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <class Archive>
void ChatHistoryEntity::serialize(Archive& ar, const unsigned int version)
{
    if (version < 1) {
        ar  & BOOST_SERIALIZATION_NVP(m_timestamp)
            & BOOST_SERIALIZATION_NVP(m_player_name)
            & BOOST_SERIALIZATION_NVP(m_text);
    } else {
        ar  & BOOST_SERIALIZATION_NVP(m_text)
            & BOOST_SERIALIZATION_NVP(m_player_name)
            & BOOST_SERIALIZATION_NVP(m_text_color)
            & BOOST_SERIALIZATION_NVP(m_timestamp);
    }
}

template void ChatHistoryEntity::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void ChatHistoryEntity::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void ChatHistoryEntity::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void ChatHistoryEntity::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

template <class Archive>
void PlayerInfo::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(name)
        & BOOST_SERIALIZATION_NVP(empire_id)
        & BOOST_SERIALIZATION_NVP(client_type)
        & BOOST_SERIALIZATION_NVP(host);
}

template void PlayerInfo::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void PlayerInfo::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void PlayerInfo::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void PlayerInfo::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

