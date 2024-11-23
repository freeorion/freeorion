#include "Serialize.h"

#include "MultiplayerCommon.h"
#include "OptionsDB.h"
#include "OrderSet.h"
#include "Order.h"
#include "SaveGamePreviewUtils.h"
#include "Version.h"
#include "../universe/System.h"

#include "Serialize.ipp"

#include <boost/date_time/posix_time/time_serialize.hpp>
#include <boost/serialization/array.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

int& GlobalSerializationEncodingForEmpire() {
    static int s_encoding_empire = ALL_EMPIRES;
    return s_encoding_empire;
}

template <typename Archive>
void serialize(Archive& ar, FullPreview& fp, unsigned int const version)
{
    using namespace boost::serialization;

    ar & make_nvp("filename", fp.filename)
       & make_nvp("preview", fp.preview)
       & make_nvp("galaxy", fp.galaxy);
}

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, FullPreview&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, FullPreview&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, FullPreview&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, FullPreview&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, GalaxySetupData& obj, unsigned int const version)
{
    using namespace boost::serialization;

    if (Archive::is_saving::value && obj.encoding_empire != ALL_EMPIRES &&
        (!GetOptionsDB().Get<bool>("network.server.publish-seed")))
    {
        std::string dummy;
        ar  & make_nvp("m_seed", dummy);
    } else {
        ar  & make_nvp("m_seed", obj.seed);
    }

    ar  & make_nvp("m_size", obj.size)
        & make_nvp("m_shape", obj.shape)
        & make_nvp("m_age", obj.age)
        & make_nvp("m_starlane_freq", obj.starlane_freq)
        & make_nvp("m_planet_density", obj.planet_density)
        & make_nvp("m_specials_freq", obj.specials_freq)
        & make_nvp("m_monster_freq", obj.monster_freq)
        & make_nvp("m_native_freq", obj.native_freq)
        & make_nvp("m_ai_aggr", obj.ai_aggr);

    if (version >= 1) {
        ar & make_nvp("m_game_rules", obj.game_rules);
    }

    if (version >= 2) {
        ar & make_nvp("m_game_uid", obj.game_uid);
    } else {
        if constexpr (Archive::is_loading::value) {
            obj.game_uid = boost::uuids::to_string(boost::uuids::random_generator()());
        }
    }
}

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, GalaxySetupData&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, GalaxySetupData&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, GalaxySetupData&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, GalaxySetupData&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, SinglePlayerSetupData& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar  & make_nvp("GalaxySetupData", base_object<GalaxySetupData>(obj))
        & make_nvp("m_new_game", obj.new_game)
        & make_nvp("m_filename", obj.filename)
        & make_nvp("m_players", obj.players);
}

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, SinglePlayerSetupData&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, SinglePlayerSetupData&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, SinglePlayerSetupData&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, SinglePlayerSetupData&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, SaveGamePreviewData& obj, unsigned int const version)
{
    using namespace boost::serialization;

    if (version >= 2) {
        if constexpr (Archive::is_saving::value) {
            obj.freeorion_version = FreeOrionVersionString();
        }
        ar & make_nvp("description", obj.description)
           & make_nvp("freeorion_version", obj.freeorion_version);
        if (version >= 3) {
            ar & make_nvp("save_format_marker", obj.save_format_marker);
            if (version >= 4) {
                ar & make_nvp("uncompressed_text_size", obj.uncompressed_text_size)
                   & make_nvp("compressed_text_size", obj.compressed_text_size);
            }
        }
    }
    ar & make_nvp("magic_number", obj.magic_number)
       & make_nvp("main_player_name", obj.main_player_name);
    ar & make_nvp("main_player_empire_name", obj.main_player_empire_name);
    if (Archive::is_loading::value && version < 5) {
        CompatColor old_color;
        ar & make_nvp("main_player_empire_colour", old_color);
        obj.main_player_empire_colour = {{old_color.r, old_color.g, old_color.b, old_color.a}};
    } else {
        ar & make_nvp("main_player_empire_colour", obj.main_player_empire_colour);
    }
    ar & make_nvp("save_time", obj.save_time)
       & make_nvp("current_turn", obj.current_turn);
    if (version > 0) {
        ar & make_nvp("number_of_empires", obj.number_of_empires)
           & make_nvp("number_of_human_players", obj.number_of_human_players);
    }
}

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, SaveGamePreviewData&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, SaveGamePreviewData&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, SaveGamePreviewData&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, SaveGamePreviewData&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, SaveGameUIData& obj, unsigned int const version)
{
    using namespace boost::serialization;

    TraceLogger() << "SaveGameUIData::serialize " << (Archive::is_saving::value ? "saving" : "loading")
                  << " version " << version;
    ar  & make_nvp("map_top", obj.map_top)
        & make_nvp("map_left", obj.map_left)
        & make_nvp("map_zoom_steps_in", obj.map_zoom_steps_in)
        & make_nvp("fleets_exploring", obj.fleets_exploring)
        & make_nvp("obsolete_ui_event_count", obj.obsolete_ui_event_count);
    TraceLogger() << "SaveGameUIData::serialize processed obsolete UI event count";
    if (Archive::is_saving::value || version >= 3) {
        // serializing / deserializing boost::optional can cause problem, so
        // store instead in separate containers

        // std::vector<std::pair<int, boost::optional<std::pair<bool, int>>>> ordered_ship_design_ids_and_obsolete;
        std::vector<int> ordered_ship_design_ids;
        std::map<int, std::pair<bool, int>> ids_obsolete;

        if constexpr (Archive::is_saving::value) {
            // populate temp containers
            for (auto& id_pair_pair : obj.ordered_ship_design_ids_and_obsolete) {
                ordered_ship_design_ids.emplace_back(id_pair_pair.first);
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
            obj.ordered_ship_design_ids_and_obsolete.clear();
            for (int id : ordered_ship_design_ids) {
                auto it = ids_obsolete.find(id);
                auto opt_p_i = it == ids_obsolete.end() ?
                    boost::optional<std::pair<bool, int>>() :
                    boost::optional<std::pair<bool, int>>(it->second);
                obj.ordered_ship_design_ids_and_obsolete.emplace_back(id, opt_p_i);
            }
            TraceLogger() << "SaveGameUIData::serialize design data extracted";
        }
    } else {    // is_loading with version < 3
        // (attempt to) directly deserialize / load design ordering and obsolescence
        try {
            ar  & make_nvp("ordered_ship_design_ids_and_obsolete", obj.ordered_ship_design_ids_and_obsolete);
        } catch (const std::exception& e) {
            ErrorLogger() << "Deserializing ship design ids and obsoletes failed with '" << e.what() << "'. Skipping hull order and obsoletion, and obsolete ship parts.";
            return;
        }
    }
    ar  & make_nvp("ordered_ship_hull_and_obsolete", obj.ordered_ship_hull_and_obsolete);
    TraceLogger() << "SaveGameUIData::serialize ship hull processed";
    if (Archive::is_saving::value || version >= 4) {
        std::map<std::string, int> ordered_obsolete_ship_parts;

        if constexpr (Archive::is_saving::value) {
            // populate temp container
            ordered_obsolete_ship_parts = std::map<std::string, int>(obj.obsolete_ship_parts.begin(), obj.obsolete_ship_parts.end());
            // serialize into archive
            ar & BOOST_SERIALIZATION_NVP(ordered_obsolete_ship_parts);
        } else {
            // deserialize into temp container
            ar & BOOST_SERIALIZATION_NVP(ordered_obsolete_ship_parts);

            // extract from temp container
            obj.obsolete_ship_parts = std::unordered_map<std::string, int>(ordered_obsolete_ship_parts.begin(), ordered_obsolete_ship_parts.end());
        }
    } else {    // is_loading with version < 4
        try {
            ar  & make_nvp("obsolete_ship_parts", obj.obsolete_ship_parts);
        } catch (const std::exception& e) {
            ErrorLogger() << "Deserializing obsolete ship parts failed with '" << e.what() << "'.";
        }
    }
    TraceLogger() << "SaveGameUIData::serialize obsoleted ship parts processed " << obj.obsolete_ship_parts.size()
                  << " items. Bucket count " << obj.obsolete_ship_parts.bucket_count();
}

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, SaveGameUIData&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, SaveGameUIData&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, SaveGameUIData&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, SaveGameUIData&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, SaveGameEmpireData& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar  & make_nvp("m_empire_id", obj.empire_id)
        & make_nvp("m_empire_name", obj.empire_name)
        & make_nvp("m_player_name", obj.player_name);
    if (Archive::is_loading::value && version < 3) {
        CompatColor old_color;
        ar & make_nvp("m_color", old_color);
        obj.color = {{old_color.r, old_color.g, old_color.b, old_color.a}};
    } else {
        ar & make_nvp("m_color", obj.color);
    }
    if (version >= 1) {
        ar & make_nvp("m_authenticated", obj.authenticated);
    }
    if (version >= 2) {
        ar & make_nvp("m_eliminated", obj.eliminated);
        ar & make_nvp("m_won", obj.won);
    }
}

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, SaveGameEmpireData&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, SaveGameEmpireData&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, SaveGameEmpireData&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, SaveGameEmpireData&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, PlayerSaveHeaderData& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar  & make_nvp("m_name", obj.name)
        & make_nvp("m_empire_id", obj.empire_id)
        & make_nvp("m_client_type", obj.client_type);
}

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, PlayerSaveHeaderData&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, PlayerSaveHeaderData&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, PlayerSaveHeaderData&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, PlayerSaveHeaderData&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, PlayerSaveGameData& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar  & make_nvp("m_name", obj.name)
        & make_nvp("m_empire_id", obj.empire_id);

    if (Archive::is_loading::value && version < 3) {
        auto orders = std::make_shared<OrderSet>();
        auto ui_data = std::make_shared<SaveGameUIData>();
        ar  & make_nvp("m_orders", orders)
            & make_nvp("m_ui_data", ui_data);
        if (orders)
            obj.orders = std::move(*orders);
        if (ui_data)
            obj.ui_data = std::move(*ui_data);

    } else {
        ar  & make_nvp("m_orders", obj.orders)
            & make_nvp("m_ui_data", obj.ui_data);
    }

    ar  & make_nvp("m_save_state_string", obj.save_state_string)
        & make_nvp("m_client_type", obj.client_type);

    if (Archive::is_loading::value && version < 2) {
        bool ready = false;
        ar & make_nvp("m_ready", ready);
    }
}

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, PlayerSaveGameData&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, PlayerSaveGameData&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, PlayerSaveGameData&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, PlayerSaveGameData&, unsigned int const);


template<typename Archive>
void serialize(Archive& ar, PreviewInformation& pi, unsigned int const version)
{
    using namespace boost::serialization;

    ar & make_nvp("subdirectories", pi.subdirectories)
       & make_nvp("folder", pi.folder)
       & make_nvp("previews", pi.previews);
}

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, PreviewInformation&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, PreviewInformation&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, PreviewInformation&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, PreviewInformation&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, ServerSaveGameData& obj, unsigned int const version)
{
    ar  & boost::serialization::make_nvp("m_current_turn", obj.current_turn);
}

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, ServerSaveGameData&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, ServerSaveGameData&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, ServerSaveGameData&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, ServerSaveGameData&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, PlayerSetupData& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar  & make_nvp("m_player_name", obj.player_name)
        & make_nvp("m_player_id", obj.player_id)
        & make_nvp("m_empire_name", obj.empire_name)
        & make_nvp("m_empire_color", obj.empire_color)
        & make_nvp("m_starting_species_name", obj.starting_species_name)
        & make_nvp("m_save_game_empire_id", obj.save_game_empire_id)
        & make_nvp("m_client_type", obj.client_type)
        & make_nvp("m_player_ready", obj.player_ready);
    if (version >= 1) {
        ar & make_nvp("m_authenticated", obj.authenticated);
    }
    if (version >= 2) {
        ar & make_nvp("m_starting_team", obj.starting_team);
    }
}

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, PlayerSetupData&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, PlayerSetupData&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, PlayerSetupData&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, PlayerSetupData&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, MultiplayerLobbyData& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar  & make_nvp("GalaxySetupData", base_object<GalaxySetupData>(obj))
        & make_nvp("m_new_game", obj.new_game)
        & make_nvp("m_players", obj.players)
        & make_nvp("m_save_game", obj.save_game)
        & make_nvp("m_save_game_empire_data", obj.save_game_empire_data)
        & make_nvp("m_any_can_edit", obj.any_can_edit)
        & make_nvp("m_start_locked", obj.start_locked)
        & make_nvp("m_start_lock_cause", obj.start_lock_cause);
    if (version >= 1) {
        ar & make_nvp("m_save_game_current_turn", obj.save_game_current_turn);
    }
    if (version >= 2) {
        ar & make_nvp("m_in_game", obj.in_game);
    }
}

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, MultiplayerLobbyData&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, MultiplayerLobbyData&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, MultiplayerLobbyData&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, MultiplayerLobbyData&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, ChatHistoryEntity& obj, unsigned int const version)
{
    using namespace boost::serialization;

    if (version < 1) {
        ar  & make_nvp("m_timestamp", obj.timestamp)
            & make_nvp("m_player_name", obj.player_name)
            & make_nvp("m_text", obj.text);
    } else {
        ar  & make_nvp("m_text", obj.text)
            & make_nvp("m_player_name", obj.player_name)
            & make_nvp("m_text_color", obj.text_color)
            & make_nvp("m_timestamp", obj.timestamp);
    }
}

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, ChatHistoryEntity&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, ChatHistoryEntity&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, ChatHistoryEntity&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, ChatHistoryEntity&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, PlayerInfo& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar  & make_nvp("name", obj.name)
        & make_nvp("empire_id", obj.empire_id)
        & make_nvp("client_type", obj.client_type)
        & make_nvp("host", obj.host);
}

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, PlayerInfo&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, PlayerInfo&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, PlayerInfo&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, PlayerInfo&, unsigned int const);
