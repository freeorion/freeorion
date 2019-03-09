#include "Serialize.h"

#include "MultiplayerCommon.h"
#include "OrderSet.h"
#include "Order.h"
#include "../universe/System.h"

#include "Serialize.ipp"

#include <boost/date_time/posix_time/time_serialize.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>


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
            m_game_uid = boost::uuids::to_string(boost::uuids::random_generator()());
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
        & BOOST_SERIALIZATION_NVP(fleets_exploring)
        & BOOST_SERIALIZATION_NVP(obsolete_ui_event_count)
        & BOOST_SERIALIZATION_NVP(ordered_ship_design_ids_and_obsolete)
        & BOOST_SERIALIZATION_NVP(ordered_ship_hull_and_obsolete)
        & BOOST_SERIALIZATION_NVP(obsolete_ship_parts);
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
    if (version >= 1) {
        ar & BOOST_SERIALIZATION_NVP(m_authenticated);
    }
}

template void SaveGameEmpireData::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void SaveGameEmpireData::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void SaveGameEmpireData::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void SaveGameEmpireData::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);


template <class Archive>
void PlayerSaveHeaderData::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_client_type);
}

template void PlayerSaveHeaderData::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void PlayerSaveHeaderData::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void PlayerSaveHeaderData::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void PlayerSaveHeaderData::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);


template <class Archive>
void PlayerSaveGameData::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_orders)
        & BOOST_SERIALIZATION_NVP(m_ui_data)
        & BOOST_SERIALIZATION_NVP(m_save_state_string)
        & BOOST_SERIALIZATION_NVP(m_client_type);
    if (version >= 1) {
        ar & BOOST_SERIALIZATION_NVP(m_ready);
    }
}

template void PlayerSaveGameData::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void PlayerSaveGameData::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void PlayerSaveGameData::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void PlayerSaveGameData::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);


template <class Archive>
void ServerSaveGameData::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_current_turn);
}

template void ServerSaveGameData::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void ServerSaveGameData::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void ServerSaveGameData::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void ServerSaveGameData::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);


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
    if (version >= 1) {
        ar & BOOST_SERIALIZATION_NVP(m_authenticated);
    }
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
