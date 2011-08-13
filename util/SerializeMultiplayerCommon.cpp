#include "Serialize.h"

#include "MultiplayerCommon.h"
#include "../universe/System.h"

#include "Serialize.ipp"


template <class Archive>
void GalaxySetupData::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_size)
        & BOOST_SERIALIZATION_NVP(m_shape)
        & BOOST_SERIALIZATION_NVP(m_age)
        & BOOST_SERIALIZATION_NVP(m_starlane_freq)
        & BOOST_SERIALIZATION_NVP(m_planet_density)
        & BOOST_SERIALIZATION_NVP(m_specials_freq)
        & BOOST_SERIALIZATION_NVP(m_life_freq);
}

template void GalaxySetupData::serialize<FREEORION_OARCHIVE_TYPE>(FREEORION_OARCHIVE_TYPE&, const unsigned int);
template void GalaxySetupData::serialize<FREEORION_IARCHIVE_TYPE>(FREEORION_IARCHIVE_TYPE&, const unsigned int);

template <class Archive>
void SinglePlayerSetupData::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GalaxySetupData)
        & BOOST_SERIALIZATION_NVP(m_new_game)
        & BOOST_SERIALIZATION_NVP(m_filename)
        & BOOST_SERIALIZATION_NVP(m_players);
}

template void SinglePlayerSetupData::serialize<FREEORION_OARCHIVE_TYPE>(FREEORION_OARCHIVE_TYPE&, const unsigned int);
template void SinglePlayerSetupData::serialize<FREEORION_IARCHIVE_TYPE>(FREEORION_IARCHIVE_TYPE&, const unsigned int);

template <class Archive>
void SaveGameUIData::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(map_top)
        & BOOST_SERIALIZATION_NVP(map_left)
        & BOOST_SERIALIZATION_NVP(map_zoom_steps_in);
}

template void SaveGameUIData::serialize<FREEORION_OARCHIVE_TYPE>(FREEORION_OARCHIVE_TYPE&, const unsigned int);
template void SaveGameUIData::serialize<FREEORION_IARCHIVE_TYPE>(FREEORION_IARCHIVE_TYPE&, const unsigned int);

template <class Archive>
void SaveGameEmpireData::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_empire_name)
        & BOOST_SERIALIZATION_NVP(m_player_name)
        & BOOST_SERIALIZATION_NVP(m_color);
}

template void SaveGameEmpireData::serialize<FREEORION_OARCHIVE_TYPE>(FREEORION_OARCHIVE_TYPE&, const unsigned int);
template void SaveGameEmpireData::serialize<FREEORION_IARCHIVE_TYPE>(FREEORION_IARCHIVE_TYPE&, const unsigned int);

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

template void PlayerSetupData::serialize<FREEORION_OARCHIVE_TYPE>(FREEORION_OARCHIVE_TYPE&, const unsigned int);
template void PlayerSetupData::serialize<FREEORION_IARCHIVE_TYPE>(FREEORION_IARCHIVE_TYPE&, const unsigned int);

template <class Archive>
void MultiplayerLobbyData::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GalaxySetupData)
        & BOOST_SERIALIZATION_NVP(m_new_game)
        & BOOST_SERIALIZATION_NVP(m_save_file_index)
        & BOOST_SERIALIZATION_NVP(m_players)
        & BOOST_SERIALIZATION_NVP(m_save_games)
        & BOOST_SERIALIZATION_NVP(m_save_game_empire_data);
}

template void MultiplayerLobbyData::serialize<FREEORION_OARCHIVE_TYPE>(FREEORION_OARCHIVE_TYPE&, const unsigned int);
template void MultiplayerLobbyData::serialize<FREEORION_IARCHIVE_TYPE>(FREEORION_IARCHIVE_TYPE&, const unsigned int);

template <class Archive>
void PlayerInfo::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(name)
        & BOOST_SERIALIZATION_NVP(empire_id)
        & BOOST_SERIALIZATION_NVP(client_type)
        & BOOST_SERIALIZATION_NVP(host);
}

template void PlayerInfo::serialize<FREEORION_OARCHIVE_TYPE>(FREEORION_OARCHIVE_TYPE&, const unsigned int);
template void PlayerInfo::serialize<FREEORION_IARCHIVE_TYPE>(FREEORION_IARCHIVE_TYPE&, const unsigned int);

template <class Archive>
void CombatData::save(Archive & ar, const unsigned int version) const
{
    ar  & BOOST_SERIALIZATION_NVP(m_combat_turn_number)
        & BOOST_SERIALIZATION_NVP(m_system);
    Serialize(ar, m_combat_universe);
    Serialize(ar, m_pathing_engine);
}

template <class Archive>
void CombatData::load(Archive & ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_combat_turn_number)
        & BOOST_SERIALIZATION_NVP(m_system);
    Deserialize(ar, m_combat_universe);
    PathingEngine::s_combat_universe = &m_combat_universe;
    Deserialize(ar, m_pathing_engine);
    PathingEngine::s_combat_universe = 0;
}

template void CombatData::save<FREEORION_OARCHIVE_TYPE>(FREEORION_OARCHIVE_TYPE&, const unsigned int) const;
template void CombatData::load<FREEORION_IARCHIVE_TYPE>(FREEORION_IARCHIVE_TYPE&, const unsigned int);

template <class Archive>
void CombatSetupRegion::serialize(Archive & ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_type)
        & BOOST_SERIALIZATION_NVP(m_radius_begin)
        & BOOST_SERIALIZATION_NVP(m_radius_end)
        & BOOST_SERIALIZATION_NVP(m_centroid)
        & BOOST_SERIALIZATION_NVP(m_radial_axis)
        & BOOST_SERIALIZATION_NVP(m_tangent_axis)
        & BOOST_SERIALIZATION_NVP(m_theta_begin)
        & BOOST_SERIALIZATION_NVP(m_theta_end);
}

template void CombatSetupRegion::serialize<FREEORION_OARCHIVE_TYPE>(FREEORION_OARCHIVE_TYPE&, const unsigned int);
template void CombatSetupRegion::serialize<FREEORION_IARCHIVE_TYPE>(FREEORION_IARCHIVE_TYPE&, const unsigned int);

template <class Archive>
void CombatSetupGroup::serialize(Archive & ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_ships)
        & BOOST_SERIALIZATION_NVP(m_regions)
        & BOOST_SERIALIZATION_NVP(m_allow);
}

template void CombatSetupGroup::serialize<FREEORION_OARCHIVE_TYPE>(FREEORION_OARCHIVE_TYPE&, const unsigned int);
template void CombatSetupGroup::serialize<FREEORION_IARCHIVE_TYPE>(FREEORION_IARCHIVE_TYPE&, const unsigned int);
