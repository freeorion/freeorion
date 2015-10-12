#include "SaveLoad.h"

#include "ServerApp.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../universe/Building.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/Planet.h"
#include "../universe/ShipDesign.h"
#include "../universe/System.h"
#include "../universe/Species.h"
#include "../util/Directories.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../util/Order.h"
#include "../util/OrderSet.h"
#include "../util/SaveGamePreviewUtils.h"
#include "../util/Serialize.h"
#include "../combat/CombatLogManager.h"

#include <GG/utf8/checked.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/thread.hpp>

namespace fs = boost::filesystem;

namespace {
    std::map<int, SaveGameEmpireData> CompileSaveGameEmpireData(const EmpireManager& empire_manager) {
        std::map<int, SaveGameEmpireData> retval;
        const EmpireManager& empires = Empires();
        for (EmpireManager::const_iterator it = empires.begin(); it != empires.end(); ++it)
            retval[it->first] = SaveGameEmpireData(it->first, it->second->Name(), it->second->PlayerName(), it->second->Color());
        return retval;
    }

    void CompileSaveGamePreviewData(const ServerSaveGameData& server_save_game_data,
                                    const std::vector<PlayerSaveGameData>& player_save_game_data,
                                    const std::map<int, SaveGameEmpireData>& empire_save_game_data,
                                    SaveGamePreviewData& preview)
    {
        typedef std::vector<PlayerSaveGameData>::const_iterator player_iterator;

        // First compile the non-player related data
        preview.current_turn = server_save_game_data.m_current_turn;
        preview.number_of_empires = empire_save_game_data.size();
        preview.save_time = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());

        if (player_save_game_data.empty()) {
            preview.main_player_name = UserString("NO_PLAYERS");
            preview.main_player_empire_name = UserString("NO_EMPIRE");

        } else {
            // Consider the first player the main player
            const PlayerSaveGameData* player = &(*player_save_game_data.begin());

            // If there are human players, the first of them should be the main player
            short humans = 0;
            for (player_iterator it = player_save_game_data.begin(); it != player_save_game_data.end(); ++it) {
                if (it->m_client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER) {
                    if (player->m_client_type != Networking::CLIENT_TYPE_HUMAN_PLAYER &&
                       player->m_client_type != Networking::CLIENT_TYPE_HUMAN_OBSERVER &&
                       player->m_client_type != Networking::CLIENT_TYPE_HUMAN_MODERATOR)
                    {
                        player = &(*it);
                    }
                    ++humans;
                }
            }

            preview.main_player_name = player->m_name;
            preview.number_of_human_players = humans;

            // Find the empire of the player, if it has one
            std::map<int, SaveGameEmpireData>::const_iterator empire = empire_save_game_data.find(player->m_empire_id);
            if (empire != empire_save_game_data.end()) {
                preview.main_player_empire_name = empire->second.m_empire_name;
                preview.main_player_empire_colour = empire->second.m_color;
            }
        }
    }

    const std::string UNABLE_TO_OPEN_FILE("Unable to open file");
}

void SaveGame(const std::string& filename, const ServerSaveGameData& server_save_game_data,
              const std::vector<PlayerSaveGameData>& player_save_game_data, const Universe& universe,
              const EmpireManager& empire_manager, const SpeciesManager& species_manager,
              const CombatLogManager& combat_log_manager, const GalaxySetupData& galaxy_setup_data,
              bool multiplayer)
{
    DebugLogger() << "SaveGame:: filename: " << filename;
    GetUniverse().EncodingEmpire() = ALL_EMPIRES;

    std::map<int, SaveGameEmpireData> empire_save_game_data = CompileSaveGameEmpireData(empire_manager);
    SaveGamePreviewData save_preview_data;
    CompileSaveGamePreviewData(server_save_game_data, player_save_game_data, empire_save_game_data, save_preview_data);

    bool use_binary = GetOptionsDB().Get<bool>("binary-serialization");

    try {
        fs::path path = FilenameToPath(filename);
        // A relative path should be relative to the save directory.
        if (path.is_relative()) {
            path = GetSaveDir()/path;
            DebugLogger() << "Made save path relative to save dir. Is now: " << path;
        }

        if (multiplayer) {
            // Make sure the path points into our save directory
            if (!IsInside(path, GetSaveDir())) {
                path = GetSaveDir() / path.filename();
            }
        }

        // set up output archive / stream for saving
        fs::ofstream ofs(path, std::ios_base::binary);
        if (!ofs)
            throw std::runtime_error(UNABLE_TO_OPEN_FILE);

        if (use_binary) {
            freeorion_bin_oarchive boa(ofs);
            boa << BOOST_SERIALIZATION_NVP(save_preview_data);
            boa << BOOST_SERIALIZATION_NVP(galaxy_setup_data);
            boa << BOOST_SERIALIZATION_NVP(server_save_game_data);
            boa << BOOST_SERIALIZATION_NVP(player_save_game_data);
            boa << BOOST_SERIALIZATION_NVP(empire_save_game_data);
            boa << BOOST_SERIALIZATION_NVP(empire_manager);
            boa << BOOST_SERIALIZATION_NVP(species_manager);
            boa << BOOST_SERIALIZATION_NVP(combat_log_manager);
            Serialize(boa, universe);

        } else {
            // save as xml into stringstream
            std::stringstream ss;
            freeorion_xml_oarchive xoa(ss);
            xoa << BOOST_SERIALIZATION_NVP(save_preview_data);
            xoa << BOOST_SERIALIZATION_NVP(galaxy_setup_data);
            xoa << BOOST_SERIALIZATION_NVP(server_save_game_data);
            xoa << BOOST_SERIALIZATION_NVP(player_save_game_data);
            xoa << BOOST_SERIALIZATION_NVP(empire_save_game_data);
            xoa << BOOST_SERIALIZATION_NVP(empire_manager);
            xoa << BOOST_SERIALIZATION_NVP(species_manager);
            xoa << BOOST_SERIALIZATION_NVP(combat_log_manager);
            Serialize(xoa, universe);

            // set up filter to compress data before outputting to file
            boost::iostreams::filtering_ostreambuf o;
            o.push(boost::iostreams::zlib_compressor());
            o.push(ofs);

            // pass xml to compressed output file
            boost::iostreams::copy(std::istringstream(ss.str()), o);
        }


    } catch (const std::exception& e) {
        ErrorLogger() << UserString("UNABLE_TO_WRITE_SAVE_FILE") << " SaveGame exception: " << ": " << e.what();
        throw e;
    }
}

void LoadGame(const std::string& filename, ServerSaveGameData& server_save_game_data,
              std::vector<PlayerSaveGameData>& player_save_game_data, Universe& universe,
              EmpireManager& empire_manager, SpeciesManager& species_manager,
              CombatLogManager& combat_log_manager, GalaxySetupData& galaxy_setup_data)
{
    //boost::this_thread::sleep(boost::posix_time::seconds(1));

    // player notifications
    if (ServerApp* server = ServerApp::GetApp())
        server->Networking().SendMessage(TurnProgressMessage(Message::LOADING_GAME));

    GetUniverse().EncodingEmpire() = ALL_EMPIRES;

    std::map<int, SaveGameEmpireData> ignored_save_game_empire_data;
    SaveGamePreviewData ignored_save_preview_data;

    empire_manager.Clear();
    universe.Clear();

    try {
        // set up input archive / stream for loading
        const fs::path path = FilenameToPath(filename);
        fs::ifstream ifs(path, std::ios_base::binary);
        if (!ifs)
            throw std::runtime_error(UNABLE_TO_OPEN_FILE);

        try {
            // first attempt binary deserialziation
            freeorion_bin_iarchive ia(ifs);

            DebugLogger() << "LoadGame : Passing Preview Data";
            ia >> BOOST_SERIALIZATION_NVP(ignored_save_preview_data);

            DebugLogger() << "LoadGame : Reading Galaxy Setup Data";
            ia >> BOOST_SERIALIZATION_NVP(galaxy_setup_data);

            DebugLogger() << "LoadGame : Reading Server Save Game Data";
            ia >> BOOST_SERIALIZATION_NVP(server_save_game_data);
            DebugLogger() << "LoadGame : Reading Player Save Game Data";
            ia >> BOOST_SERIALIZATION_NVP(player_save_game_data);

            DebugLogger() << "LoadGame : Reading Empire Save Game Data (Ignored)";
            ia >> BOOST_SERIALIZATION_NVP(ignored_save_game_empire_data);
            DebugLogger() << "LoadGame : Reading Empires Data";
            ia >> BOOST_SERIALIZATION_NVP(empire_manager);
            DebugLogger() << "LoadGame : Reading Species Data";
            ia >> BOOST_SERIALIZATION_NVP(species_manager);
            DebugLogger() << "LoadGame : Reading Combat Logs";
            ia >> BOOST_SERIALIZATION_NVP(combat_log_manager);
            DebugLogger() << "LoadGame : Reading Universe Data";
            Deserialize(ia, universe);

        } catch (...) {
            // if binary deserialization failed, try more-portable XML deserialization

            // reset to start of stream (attempted binary serialization will have consumed some input...)
            boost::iostreams::seek(ifs, 0, std::ios_base::beg);

            // set up filter to decompress data
            boost::iostreams::filtering_istreambuf i;
            i.push(boost::iostreams::zlib_decompressor());
            i.push(ifs);

            // pass decompressed xml into stringstream storage that the iarchve requires...
            std::stringstream ss;
            boost::iostreams::copy(i, ss);

            // extract xml data from stringstream
            freeorion_xml_iarchive ia(ss);

            DebugLogger() << "LoadGame : Passing Preview Data";
            ia >> BOOST_SERIALIZATION_NVP(ignored_save_preview_data);

            DebugLogger() << "LoadGame : Reading Galaxy Setup Data";
            ia >> BOOST_SERIALIZATION_NVP(galaxy_setup_data);

            DebugLogger() << "LoadGame : Reading Server Save Game Data";
            ia >> BOOST_SERIALIZATION_NVP(server_save_game_data);
            DebugLogger() << "LoadGame : Reading Player Save Game Data";
            ia >> BOOST_SERIALIZATION_NVP(player_save_game_data);

            DebugLogger() << "LoadGame : Reading Empire Save Game Data (Ignored)";
            ia >> BOOST_SERIALIZATION_NVP(ignored_save_game_empire_data);
            DebugLogger() << "LoadGame : Reading Empires Data";
            ia >> BOOST_SERIALIZATION_NVP(empire_manager);
            DebugLogger() << "LoadGame : Reading Species Data";
            ia >> BOOST_SERIALIZATION_NVP(species_manager);
            DebugLogger() << "LoadGame : Reading Combat Logs";
            ia >> BOOST_SERIALIZATION_NVP(combat_log_manager);
            DebugLogger() << "LoadGame : Reading Universe Data";
            Deserialize(ia, universe);
        }

    } catch (const std::exception& err) {
        ErrorLogger() << "LoadGame(...) failed!  Error: " << err.what();
        return;
    }
    DebugLogger() << "LoadGame : Done loading save file";
}

void LoadGalaxySetupData(const std::string& filename, GalaxySetupData& galaxy_setup_data) {
    SaveGamePreviewData ignored_save_preview_data;

    try {
        fs::path path = FilenameToPath(filename);
        fs::ifstream ifs(path, std::ios_base::binary);

        if (!ifs)
            throw std::runtime_error(UNABLE_TO_OPEN_FILE);

        try {
            // first attempt binary deserialziation
            freeorion_bin_iarchive ia(ifs);

            ia >> BOOST_SERIALIZATION_NVP(ignored_save_preview_data);
            ia >> BOOST_SERIALIZATION_NVP(galaxy_setup_data);

        } catch(...) {
            // if binary deserialization failed, try more-portable XML deserialization

            // reset to start of stream (attempted binary serialization will have consumed some input...)
            boost::iostreams::seek(ifs, 0, std::ios_base::beg);

            // set up filter to decompress data
            boost::iostreams::filtering_istreambuf i;
            i.push(boost::iostreams::zlib_decompressor());
            i.push(ifs);

            // pass decompressed xml into stringstream storage that the iarchve requires...
            std::stringstream ss;
            boost::iostreams::copy(i, ss);

            // extract xml data from stringstream
            freeorion_xml_iarchive ia(ss);

            ia >> BOOST_SERIALIZATION_NVP(ignored_save_preview_data);
            ia >> BOOST_SERIALIZATION_NVP(galaxy_setup_data);
        }
        // skipping additional deserialization which is not needed for this function

    } catch(const std::exception& err) {
        ErrorLogger() << "LoadGalaxySetupData(...) failed!  Error: " << err.what();
        return;
    }
}

void LoadPlayerSaveGameData(const std::string& filename, std::vector<PlayerSaveGameData>& player_save_game_data)
{
    SaveGamePreviewData ignored_save_preview_data;
    ServerSaveGameData  ignored_server_save_game_data;
    GalaxySetupData     ignored_galaxy_setup_data;

    try {
        fs::path path = FilenameToPath(filename);
        fs::ifstream ifs(path, std::ios_base::binary);

        if (!ifs)
            throw std::runtime_error(UNABLE_TO_OPEN_FILE);

        try {
            // first attempt binary deserialziation
            freeorion_bin_iarchive ia(ifs);

            ia >> BOOST_SERIALIZATION_NVP(ignored_save_preview_data);
            ia >> BOOST_SERIALIZATION_NVP(ignored_galaxy_setup_data);
            ia >> BOOST_SERIALIZATION_NVP(ignored_server_save_game_data);
            ia >> BOOST_SERIALIZATION_NVP(player_save_game_data);

        } catch (...) {
            // if binary deserialization failed, try more-portable XML deserialization

            // reset to start of stream (attempted binary serialization will have consumed some input...)
            boost::iostreams::seek(ifs, 0, std::ios_base::beg);

            // set up filter to decompress data
            boost::iostreams::filtering_istreambuf i;
            i.push(boost::iostreams::zlib_decompressor());
            i.push(ifs);

            // pass decompressed xml into stringstream storage that the iarchve requires...
            std::stringstream ss;
            boost::iostreams::copy(i, ss);

            // extract xml data from stringstream
            freeorion_xml_iarchive ia(ss);

            ia >> BOOST_SERIALIZATION_NVP(ignored_save_preview_data);
            ia >> BOOST_SERIALIZATION_NVP(ignored_galaxy_setup_data);
            ia >> BOOST_SERIALIZATION_NVP(ignored_server_save_game_data);
            ia >> BOOST_SERIALIZATION_NVP(player_save_game_data);
        }
        // skipping additional deserialization which is not needed for this function

    } catch (const std::exception& e) {
        ErrorLogger() << UserString("UNABLE_TO_READ_SAVE_FILE") << " LoadPlayerSaveGameData exception: " << ": " << e.what();
        throw e;
    }
}

void LoadEmpireSaveGameData(const std::string& filename, std::map<int, SaveGameEmpireData>& empire_save_game_data)
{
    SaveGamePreviewData             ignored_save_preview_data;
    ServerSaveGameData              ignored_server_save_game_data;
    std::vector<PlayerSaveGameData> ignored_player_save_game_data;
    GalaxySetupData                 ignored_galaxy_setup_data;

    try {
        fs::path path = FilenameToPath(filename);
        DebugLogger() << "LoadEmpireSaveGameData: filename: " << filename << " path:" << path;
        fs::ifstream ifs(path, std::ios_base::binary);

        if (!ifs)
            throw std::runtime_error(UNABLE_TO_OPEN_FILE);

        try {
            // first attempt binary deserialziation
            freeorion_bin_iarchive ia(ifs);

            ia >> BOOST_SERIALIZATION_NVP(ignored_save_preview_data);
            ia >> BOOST_SERIALIZATION_NVP(ignored_galaxy_setup_data);
            ia >> BOOST_SERIALIZATION_NVP(ignored_server_save_game_data);
            ia >> BOOST_SERIALIZATION_NVP(ignored_player_save_game_data);
            ia >> BOOST_SERIALIZATION_NVP(empire_save_game_data);

        } catch (...) {
            // if binary deserialization failed, try more-portable XML deserialization

            // reset to start of stream (attempted binary serialization will have consumed some input...)
            boost::iostreams::seek(ifs, 0, std::ios_base::beg);

            // set up filter to decompress data
            boost::iostreams::filtering_istreambuf i;
            i.push(boost::iostreams::zlib_decompressor());
            i.push(ifs);

            // pass decompressed xml into stringstream storage that the iarchve requires...
            std::stringstream ss;
            boost::iostreams::copy(i, ss);

            // extract xml data from stringstream
            freeorion_xml_iarchive ia(ss);

            ia >> BOOST_SERIALIZATION_NVP(ignored_save_preview_data);
            ia >> BOOST_SERIALIZATION_NVP(ignored_galaxy_setup_data);
            ia >> BOOST_SERIALIZATION_NVP(ignored_server_save_game_data);
            ia >> BOOST_SERIALIZATION_NVP(ignored_player_save_game_data);
            ia >> BOOST_SERIALIZATION_NVP(empire_save_game_data);
        }
        // skipping additional deserialization which is not needed for this function

    } catch (const std::exception& e) {
        ErrorLogger() << UserString("UNABLE_TO_READ_SAVE_FILE") << " LoadEmpireSaveGameData exception: " << ": " << e.what();
        throw e;
    }
}
