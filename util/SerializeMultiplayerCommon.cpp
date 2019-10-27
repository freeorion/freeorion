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
    TraceLogger() << "SaveGameUIData::serialize " << (Archive::is_saving::value ? "saving" : "loading")
                  << " version " << version;
    ar  & BOOST_SERIALIZATION_NVP(map_top)
        & BOOST_SERIALIZATION_NVP(map_left)
        & BOOST_SERIALIZATION_NVP(map_zoom_steps_in)
        & BOOST_SERIALIZATION_NVP(fleets_exploring)
        & BOOST_SERIALIZATION_NVP(obsolete_ui_event_count);
    TraceLogger() << "SaveGameUIData::serialize processed obsolete UI event count";
    if (Archive::is_saving::value || version >= 3) {
        // serializing / deserializing boost::optional can cause problem, so
        // store instead in separate containers

        // std::vector<std::pair<int, boost::optional<std::pair<bool, int>>>> ordered_ship_design_ids_and_obsolete;
        std::vector<int> ordered_ship_design_ids;
        std::map<int, std::pair<bool, int>> ids_obsolete;

        if (Archive::is_saving::value) {
            // populate temp containers
            for (auto id_pair_pair : ordered_ship_design_ids_and_obsolete) {
                ordered_ship_design_ids.push_back(id_pair_pair.first);
                if (id_pair_pair.second)
                    ids_obsolete[id_pair_pair.first] = id_pair_pair.second.get();
            }
            // serialize into archive
            TraceLogger() << "SaveGameUIData::serialize design data into archive";
            ar  & BOOST_SERIALIZATION_NVP(ordered_ship_design_ids)
                & BOOST_SERIALIZATION_NVP(ids_obsolete);
            TraceLogger() << "SaveGameUIData::serialize design data into archive completed";
        } else {    // is_loading with version >= 3
            // deserialize into temp containers
            TraceLogger() << "SaveGameUIData::serialize design data from archive";
            ar  & BOOST_SERIALIZATION_NVP(ordered_ship_design_ids)
                & BOOST_SERIALIZATION_NVP(ids_obsolete);
            TraceLogger() << "SaveGameUIData::serialize design data from archive completed";

            // extract from temp containers into member storage with boost::optional
            ordered_ship_design_ids_and_obsolete.clear();
            for (int id : ordered_ship_design_ids) {
                auto it = ids_obsolete.find(id);
                auto opt_p_i = it == ids_obsolete.end() ?
                    boost::optional<std::pair<bool, int>>() :
                    boost::optional<std::pair<bool, int>>(it->second);
                ordered_ship_design_ids_and_obsolete.push_back(std::make_pair(id, opt_p_i));
            }
            TraceLogger() << "SaveGameUIData::serialize design data extracted";
        }
    } else {    // is_loading with version < 3
        // (attempt to) directly deserialize / load design ordering and obsolescence
        try {
            ar  & BOOST_SERIALIZATION_NVP(ordered_ship_design_ids_and_obsolete);
        } catch (...) {
            ErrorLogger() << "Deserializing ship design ids and obsoletes failed. Skipping hull order and obsoletion, and obsolete ship parts.";
            return;
        }
    }
    ar  & BOOST_SERIALIZATION_NVP(ordered_ship_hull_and_obsolete);
    TraceLogger() << "SaveGameUIData::serialize ship hull processed";
    if (Archive::is_saving::value || version >= 4) {
        std::map<std::string, int> ordered_obsolete_ship_parts;

        if (Archive::is_saving::value) {
            // populate temp container
            ordered_obsolete_ship_parts = std::move(std::map<std::string, int>(obsolete_ship_parts.begin(), obsolete_ship_parts.end()));
            // serialize into archive
            ar & BOOST_SERIALIZATION_NVP(ordered_obsolete_ship_parts);
        } else {
            // deserialize into temp container
            ar & BOOST_SERIALIZATION_NVP(ordered_obsolete_ship_parts);

            // extract from temp container
            obsolete_ship_parts = std::move(std::unordered_map<std::string, int>(ordered_obsolete_ship_parts.begin(), ordered_obsolete_ship_parts.end()));
        }
    } else {    // is_loading with version < 4
        try {
            ar  & BOOST_SERIALIZATION_NVP(obsolete_ship_parts);
        } catch (...) {
            ErrorLogger() << "Deserializing obsolete ship parts failed.";
        }
    }
    TraceLogger() << "SaveGameUIData::serialize obsoleted ship parts processed " << obsolete_ship_parts.size()
                  << " items. Bucket count " << obsolete_ship_parts.bucket_count();
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
    if (version >= 2) {
        ar & BOOST_SERIALIZATION_NVP(m_eliminated);
        ar & BOOST_SERIALIZATION_NVP(m_won);
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
    if (version >= 2) {
        ar & BOOST_SERIALIZATION_NVP(m_starting_team);
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
    if (version >= 1) {
        ar & BOOST_SERIALIZATION_NVP(m_save_game_current_turn);
    }
    if (version >= 2) {
        ar & BOOST_SERIALIZATION_NVP(m_in_game);
    }
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
