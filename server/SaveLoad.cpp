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
#include "../util/ScopedTimer.h"
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
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/thread.hpp>

namespace fs = boost::filesystem;

namespace {
    std::map<int, SaveGameEmpireData> CompileSaveGameEmpireData(const EmpireManager& empire_manager) {
        std::map<int, SaveGameEmpireData> retval;
        for (const std::map<int, Empire*>::value_type& entry : Empires())
            retval[entry.first] = SaveGameEmpireData(entry.first, entry.second->Name(), entry.second->PlayerName(), entry.second->Color());
        return retval;
    }

    void CompileSaveGamePreviewData(const ServerSaveGameData& server_save_game_data,
                                    const std::vector<PlayerSaveGameData>& player_save_game_data,
                                    const std::map<int, SaveGameEmpireData>& empire_save_game_data,
                                    SaveGamePreviewData& preview)
    {
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
            for (const PlayerSaveGameData& psgd : player_save_game_data) {
                if (psgd.m_client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER) {
                    if (player->m_client_type != Networking::CLIENT_TYPE_HUMAN_PLAYER &&
                       player->m_client_type != Networking::CLIENT_TYPE_HUMAN_OBSERVER &&
                       player->m_client_type != Networking::CLIENT_TYPE_HUMAN_MODERATOR)
                    {
                        player = &psgd;
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

int SaveGame(const std::string& filename, const ServerSaveGameData& server_save_game_data,
             const std::vector<PlayerSaveGameData>& player_save_game_data, const Universe& universe,
             const EmpireManager& empire_manager, const SpeciesManager& species_manager,
             const CombatLogManager& combat_log_manager, const GalaxySetupData& galaxy_setup_data,
             bool multiplayer)
{
    ScopedTimer timer("SaveGame: " + filename, true);

    bool use_binary = GetOptionsDB().Get<bool>("binary-serialization");
    DebugLogger() << "SaveGame(" << (use_binary ? "binary" : "zlib-xml") << ") filename: " << filename;
    GetUniverse().EncodingEmpire() = ALL_EMPIRES;

    DebugLogger() << "Compiling save empire and preview data";
    std::map<int, SaveGameEmpireData> empire_save_game_data = CompileSaveGameEmpireData(empire_manager);
    SaveGamePreviewData save_preview_data;
    CompileSaveGamePreviewData(server_save_game_data, player_save_game_data, empire_save_game_data, save_preview_data);


    // reinterpret save game data as header data for uncompressed header
    std::vector<PlayerSaveHeaderData> player_save_header_data;
    for (const PlayerSaveGameData& psgd : player_save_game_data)
    { player_save_header_data.push_back(psgd); }


    int bytes_written = 0;
    std::streampos pos_before_writing;


    try {
        fs::path path = FilenameToPath(filename);
        // A relative path should be relative to the save directory.
        if (path.is_relative()) {
            path = GetSaveDir() / path;
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
        pos_before_writing = ofs.tellp();

        if (use_binary) {
            DebugLogger() << "Creating binary oarchive";
            freeorion_bin_oarchive boa(ofs);
            boa << BOOST_SERIALIZATION_NVP(save_preview_data);
            boa << BOOST_SERIALIZATION_NVP(galaxy_setup_data);
            boa << BOOST_SERIALIZATION_NVP(server_save_game_data);
            boa << BOOST_SERIALIZATION_NVP(player_save_header_data);
            boa << BOOST_SERIALIZATION_NVP(empire_save_game_data);

            boa << BOOST_SERIALIZATION_NVP(player_save_game_data);
            boa << BOOST_SERIALIZATION_NVP(empire_manager);
            boa << BOOST_SERIALIZATION_NVP(species_manager);
            boa << BOOST_SERIALIZATION_NVP(combat_log_manager);
            Serialize(boa, universe);
            DebugLogger() << "Done serializing";

        } else {
            // Two-tier serialization:
            // main archive is uncompressed serialized header data first
            // then contains a string for compressed second archive
            // that contains the main gamestate info


            // allocate buffers for serialized gamestate
            DebugLogger() << "Allocating buffers for XML serialization...";
            std::string serial_str, compressed_str;
            try {
                DebugLogger() << "String Max Size: " << serial_str.max_size();
                std::string::size_type capacity = std::min(serial_str.max_size(),
                                                           static_cast<std::string::size_type>(std::pow(2.0, 29.0)));
                DebugLogger() << "Reserving Capacity:: " << capacity;
                serial_str.reserve(    capacity);
                compressed_str.reserve(std::pow(2.0, 26.0));
            } catch (...) {
                DebugLogger() << "Unable to preallocate full serialization buffers. Attempting serialization with dynamic buffer allocation.";
            }

            // wrap buffer string in iostream::stream to receive serialized data
            typedef boost::iostreams::back_insert_device<std::string> InsertDevice;
            InsertDevice serial_inserter(serial_str);
            boost::iostreams::stream<InsertDevice> s_sink(serial_inserter);

            // create archive with (preallocated) buffer...
            freeorion_xml_oarchive xoa(s_sink);
            // serialize main gamestate info
            xoa << BOOST_SERIALIZATION_NVP(player_save_game_data);
            xoa << BOOST_SERIALIZATION_NVP(empire_manager);
            xoa << BOOST_SERIALIZATION_NVP(species_manager);
            xoa << BOOST_SERIALIZATION_NVP(combat_log_manager);
            Serialize(xoa, universe);
            s_sink.flush();

            // wrap gamestate string in iostream::stream to extract serialized data
            typedef boost::iostreams::basic_array_source<char> SourceDevice;
            SourceDevice source(serial_str.data(), serial_str.size());
            boost::iostreams::stream<SourceDevice> s_source(source);

            // wrap compresed buffer string in iostream::streams to receive compressed string
            InsertDevice compressed_inserter(compressed_str);
            boost::iostreams::stream<InsertDevice> c_sink(compressed_inserter);

            // compression-filter gamestate into compressed string
            boost::iostreams::filtering_ostreambuf o;
            o.push(boost::iostreams::zlib_compressor());
            o.push(c_sink);
            boost::iostreams::copy(s_source, o);
            c_sink.flush();

            // write to save file: uncompressed header serialized data, with compressed main archive string at end...
            freeorion_xml_oarchive xoa2(ofs);
            // serialize uncompressed save header info
            xoa2 << BOOST_SERIALIZATION_NVP(save_preview_data);
            xoa2 << BOOST_SERIALIZATION_NVP(galaxy_setup_data);
            xoa2 << BOOST_SERIALIZATION_NVP(server_save_game_data);
            xoa2 << BOOST_SERIALIZATION_NVP(player_save_header_data);
            xoa2 << BOOST_SERIALIZATION_NVP(empire_save_game_data);
            // append compressed gamestate info
            xoa2 << BOOST_SERIALIZATION_NVP(compressed_str);
        }

        ofs.flush();
        bytes_written = ofs.tellp() - pos_before_writing;

    } catch (const std::exception& e) {
        ErrorLogger() << UserString("UNABLE_TO_WRITE_SAVE_FILE") << " SaveGame exception: " << ": " << e.what();
        throw e;
    }
    DebugLogger() << "SaveGame : Successfully wrote save file";

    return bytes_written;
}

void LoadGame(const std::string& filename, ServerSaveGameData& server_save_game_data,
              std::vector<PlayerSaveGameData>& player_save_game_data, Universe& universe,
              EmpireManager& empire_manager, SpeciesManager& species_manager,
              CombatLogManager& combat_log_manager, GalaxySetupData& galaxy_setup_data)
{
    //boost::this_thread::sleep_for(boost::chrono::seconds(1));

    ScopedTimer timer("LoadGame: " + filename, true);

    // player notifications
    if (ServerApp* server = ServerApp::GetApp())
        server->Networking().SendMessage(TurnProgressMessage(Message::LOADING_GAME));

    GetUniverse().EncodingEmpire() = ALL_EMPIRES;

    std::map<int, SaveGameEmpireData>   ignored_save_game_empire_data;
    SaveGamePreviewData                 ignored_save_preview_data;
    std::vector<PlayerSaveHeaderData>   ignored_player_save_header_data;

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
            DebugLogger() << "Reading binary iarchive";
            ia >> BOOST_SERIALIZATION_NVP(ignored_save_preview_data);
            ia >> BOOST_SERIALIZATION_NVP(galaxy_setup_data);
            ia >> BOOST_SERIALIZATION_NVP(server_save_game_data);
            ia >> BOOST_SERIALIZATION_NVP(ignored_player_save_header_data);
            ia >> BOOST_SERIALIZATION_NVP(ignored_save_game_empire_data);

            ia >> BOOST_SERIALIZATION_NVP(player_save_game_data);
            ia >> BOOST_SERIALIZATION_NVP(empire_manager);
            ia >> BOOST_SERIALIZATION_NVP(species_manager);
            ia >> BOOST_SERIALIZATION_NVP(combat_log_manager);
            Deserialize(ia, universe);
            DebugLogger() << "Done deserializing";
        } catch (...) {
            // if binary deserialization failed, try more-portable XML deserialization

            // reset to start of stream (attempted binary serialization will have consumed some input...)
            boost::iostreams::seek(ifs, 0, std::ios_base::beg);

            // allocate buffers for serialized gamestate
            DebugLogger() << "Allocating buffers for XML deserialization...";
            std::string serial_str, compressed_str;
            try {
                serial_str.reserve(    std::pow(2.0, 29.0));
                compressed_str.reserve(std::pow(2.0, 26.0));
            } catch (...) {
                DebugLogger() << "Unable to preallocate full deserialization buffers. Attempting deserialization with dynamic buffer allocation.";
            }

            // create archive with (preallocated) buffer...
            freeorion_xml_iarchive xia(ifs);
            // read from save file: uncompressed header serialized data, with compressed main archive string at end...
            // deserialize uncompressed save header info
            xia >> BOOST_SERIALIZATION_NVP(ignored_save_preview_data);
            xia >> BOOST_SERIALIZATION_NVP(galaxy_setup_data);
            xia >> BOOST_SERIALIZATION_NVP(server_save_game_data);
            xia >> BOOST_SERIALIZATION_NVP(ignored_player_save_header_data);
            xia >> BOOST_SERIALIZATION_NVP(ignored_save_game_empire_data);
            // extract compressed gamestate info
            xia >> BOOST_SERIALIZATION_NVP(compressed_str);

            // wrap compressed string in iostream::stream to extract compressed data
            typedef boost::iostreams::basic_array_source<char> SourceDevice;
            SourceDevice compressed_source(compressed_str.data(), compressed_str.size());
            boost::iostreams::stream<SourceDevice> c_source(compressed_source);

            // wrap uncompressed buffer string in iostream::stream to receive decompressed string
            typedef boost::iostreams::back_insert_device<std::string> InsertDevice;
            InsertDevice serial_inserter(serial_str);
            boost::iostreams::stream<InsertDevice> s_sink(serial_inserter);

            // set up filter to decompress data
            boost::iostreams::filtering_istreambuf i;
            i.push(boost::iostreams::zlib_decompressor());
            i.push(c_source);
            boost::iostreams::copy(i, s_sink);
            // The following line has been commented out because it caused an assertion in boost iostreams to fail
            // s_sink.flush();

            // wrap uncompressed buffer string in iostream::stream to extract decompressed string
            SourceDevice serial_source(serial_str.data(), serial_str.size());
            boost::iostreams::stream<SourceDevice> s_source(serial_source);

            // create archive with (preallocated) buffer...
            freeorion_xml_iarchive xia2(s_source);
            // deserialize main gamestate info
            xia2 >> BOOST_SERIALIZATION_NVP(player_save_game_data);
            xia2 >> BOOST_SERIALIZATION_NVP(empire_manager);
            xia2 >> BOOST_SERIALIZATION_NVP(species_manager);
            xia2 >> BOOST_SERIALIZATION_NVP(combat_log_manager);
            Deserialize(xia2, universe);
        }

    } catch (const std::exception& err) {
        ErrorLogger() << "LoadGame(...) failed!  Error: " << err.what();
        return;
    }
    DebugLogger() << "LoadGame : Successfully loaded save file";
}

void LoadGalaxySetupData(const std::string& filename, GalaxySetupData& galaxy_setup_data) {
    SaveGamePreviewData ignored_save_preview_data;

    ScopedTimer timer("LoadGalaxySetupData: " + filename, true);

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
            freeorion_xml_iarchive ia(ifs);

            ia >> BOOST_SERIALIZATION_NVP(ignored_save_preview_data);
            ia >> BOOST_SERIALIZATION_NVP(galaxy_setup_data);
        }
        // skipping additional deserialization which is not needed for this function

    } catch(const std::exception& err) {
        ErrorLogger() << "LoadGalaxySetupData(...) failed!  Error: " << err.what();
        return;
    }
}

void LoadPlayerSaveHeaderData(const std::string& filename, std::vector<PlayerSaveHeaderData>& player_save_header_data) {
    SaveGamePreviewData ignored_save_preview_data;
    ServerSaveGameData  ignored_server_save_game_data;
    GalaxySetupData     ignored_galaxy_setup_data;

    ScopedTimer timer("LoadPlayerSaveHeaderData: " + filename, true);

    try {
        DebugLogger() << "Reading player save game data from: " << filename;
        fs::path path = FilenameToPath(filename);
        fs::ifstream ifs(path, std::ios_base::binary);

        if (!ifs)
            throw std::runtime_error(UNABLE_TO_OPEN_FILE);

        try {
            // first attempt binary deserialziation
            DebugLogger() << "Attempting binary deserialization...";
            freeorion_bin_iarchive ia(ifs);

            ia >> BOOST_SERIALIZATION_NVP(ignored_save_preview_data);
            ia >> BOOST_SERIALIZATION_NVP(ignored_galaxy_setup_data);
            ia >> BOOST_SERIALIZATION_NVP(ignored_server_save_game_data);
            ia >> BOOST_SERIALIZATION_NVP(player_save_header_data);

        } catch (...) {
            // if binary deserialization failed, try more-portable XML deserialization
            DebugLogger() << "Trying again with XML deserialization...";

            // reset to start of stream (attempted binary serialization will have consumed some input...)
            boost::iostreams::seek(ifs, 0, std::ios_base::beg);
            freeorion_xml_iarchive ia(ifs);

            ia >> BOOST_SERIALIZATION_NVP(ignored_save_preview_data);
            ia >> BOOST_SERIALIZATION_NVP(ignored_galaxy_setup_data);
            ia >> BOOST_SERIALIZATION_NVP(ignored_server_save_game_data);
            ia >> BOOST_SERIALIZATION_NVP(player_save_header_data);
        }
        // skipping additional deserialization which is not needed for this function
        DebugLogger() << "Done reading player save game data...";
    } catch (const std::exception& e) {
        ErrorLogger() << UserString("UNABLE_TO_READ_SAVE_FILE") << " LoadPlayerSaveGameData exception: " << ": " << e.what();
        throw e;
    }
}

void LoadEmpireSaveGameData(const std::string& filename, std::map<int, SaveGameEmpireData>& empire_save_game_data) {
    SaveGamePreviewData                 ignored_save_preview_data;
    ServerSaveGameData                  ignored_server_save_game_data;
    std::vector<PlayerSaveHeaderData>   ignored_player_save_header_data;
    GalaxySetupData                     ignored_galaxy_setup_data;

    ScopedTimer timer("LoadEmpireSaveGameData: " + filename, true);

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
            ia >> BOOST_SERIALIZATION_NVP(ignored_player_save_header_data);
            ia >> BOOST_SERIALIZATION_NVP(empire_save_game_data);

        } catch (...) {
            // if binary deserialization failed, try more-portable XML deserialization

            // reset to start of stream (attempted binary serialization will have consumed some input...)
            boost::iostreams::seek(ifs, 0, std::ios_base::beg);
            freeorion_xml_iarchive ia(ifs);

            ia >> BOOST_SERIALIZATION_NVP(ignored_save_preview_data);
            ia >> BOOST_SERIALIZATION_NVP(ignored_galaxy_setup_data);
            ia >> BOOST_SERIALIZATION_NVP(ignored_server_save_game_data);
            ia >> BOOST_SERIALIZATION_NVP(ignored_player_save_header_data);
            ia >> BOOST_SERIALIZATION_NVP(empire_save_game_data);
        }
        // skipping additional deserialization which is not needed for this function

    } catch (const std::exception& e) {
        ErrorLogger() << UserString("UNABLE_TO_READ_SAVE_FILE") << " LoadEmpireSaveGameData exception: " << ": " << e.what();
        throw e;
    }
}
