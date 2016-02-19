#include "Serialize.h"

#include "MultiplayerCommon.h"
#include "../universe/System.h"

#include "Serialize.ipp"


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
        & BOOST_SERIALIZATION_NVP(m_ai_aggr)
        & BOOST_SERIALIZATION_NVP(m_picked_shape)
        & BOOST_SERIALIZATION_NVP(m_picked_age)
        & BOOST_SERIALIZATION_NVP(m_picked_starlane_freq)
        & BOOST_SERIALIZATION_NVP(m_picked_planet_density)
        & BOOST_SERIALIZATION_NVP(m_picked_specials_freq)
        & BOOST_SERIALIZATION_NVP(m_picked_monster_freq)
        & BOOST_SERIALIZATION_NVP(m_picked_native_freq);
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
        & BOOST_SERIALIZATION_NVP(m_empire_name)
        & BOOST_SERIALIZATION_NVP(m_empire_color)
        & BOOST_SERIALIZATION_NVP(m_starting_species_name)
        & BOOST_SERIALIZATION_NVP(m_save_game_empire_id)
        & BOOST_SERIALIZATION_NVP(m_client_type);
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
        & BOOST_SERIALIZATION_NVP(m_save_game_empire_data);
}

template void MultiplayerLobbyData::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void MultiplayerLobbyData::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void MultiplayerLobbyData::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void MultiplayerLobbyData::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

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

