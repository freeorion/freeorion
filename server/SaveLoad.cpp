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
#include "../util/base64_filter.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/Order.h"
#include "../util/OrderSet.h"
#include "../util/SaveGamePreviewUtils.h"
#include "../util/Serialize.h"
#include "../util/ScopedTimer.h"
#include "../combat/CombatLogManager.h"

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

#include <boost/serialization/shared_ptr.hpp>


namespace fs = boost::filesystem;

namespace {
    void CompileSaveGamePreviewData(const ServerSaveGameData& server_save_game_data,
                                    const std::vector<PlayerSaveGameData>& player_save_game_data,
                                    const std::map<int, SaveGameEmpireData>& empire_save_game_data,
                                    SaveGamePreviewData& preview)
    {
        // First compile the non-player related data
        preview.current_turn = server_save_game_data.m_current_turn;
        preview.number_of_empires = empire_save_game_data.size();
        preview.save_time = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());

        DebugLogger() << "CompileSaveGamePreviewData(...) player_save_game_data size: " << player_save_game_data.size();

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
            auto empire = empire_save_game_data.find(player->m_empire_id);
            if (empire != empire_save_game_data.end()) {
                preview.main_player_empire_name = empire->second.m_empire_name;
                preview.main_player_empire_colour = empire->second.m_color;
            }
        }
    }

    const std::string UNABLE_TO_OPEN_FILE("Unable to open file");

    const std::string XML_COMPRESSED_MARKER("zlib-xml");
    const std::string XML_COMPRESSED_BASE64_MARKER("zb64-xml");
    const std::string XML_DIRECT_MARKER("raw-xml");
    const std::string BINARY_MARKER("binary");
}

std::map<int, SaveGameEmpireData> CompileSaveGameEmpireData() {
    std::map<int, SaveGameEmpireData> retval;
    for (const auto& entry : Empires())
        retval[entry.first] = SaveGameEmpireData(entry.first, entry.second->Name(), entry.second->PlayerName(), entry.second->Color(), entry.second->IsAuthenticated(), entry.second->Eliminated(), entry.second->Won());
    return retval;
}

int SaveGame(const std::string& filename, const ServerSaveGameData& server_save_game_data,
             const std::vector<PlayerSaveGameData>& player_save_game_data, const Universe& universe,
             const EmpireManager& empire_manager, const SpeciesManager& species_manager,
             const CombatLogManager& combat_log_manager, const GalaxySetupData& galaxy_setup_data,
             bool multiplayer)
{
    SectionedScopedTimer timer("SaveGame");

    bool use_binary = GetOptionsDB().Get<bool>("save.format.binary.enabled");
    bool use_zlib_for_zml = GetOptionsDB().Get<bool>("save.format.xml.zlib.enabled");
    DebugLogger() << "SaveGame(" << (use_binary ? "binary" : (use_zlib_for_zml ? "zlib-xml" : "raw-xml")) << ") filename: " << filename;
    GetUniverse().EncodingEmpire() = ALL_EMPIRES;

    DebugLogger() << "Compiling save empire and preview data";
    timer.EnterSection("compiling data");
    std::map<int, SaveGameEmpireData> empire_save_game_data = CompileSaveGameEmpireData();
    SaveGamePreviewData save_preview_data;
    CompileSaveGamePreviewData(server_save_game_data, player_save_game_data,
                               empire_save_game_data, save_preview_data);


    // reinterpret save game data as header data for uncompressed header
    std::vector<PlayerSaveHeaderData> player_save_header_data;
    for (const PlayerSaveGameData& psgd : player_save_game_data)
    { player_save_header_data.push_back(psgd); }


    int bytes_written = 0;
    std::streampos pos_before_writing;


    try {
        timer.EnterSection("path management");
        fs::path path = FilenameToPath(filename);

        // A relative path should be relative to the save directory.
        if (path.is_relative()) {
            path = (multiplayer ? GetServerSaveDir() : GetSaveDir()) / path;
            DebugLogger() << "Made save path relative to save dir. Is now: " << path;
        }

        if (multiplayer) {
            // Make sure the path points into our save directory
            if (!IsInDir(GetServerSaveDir(), path.parent_path())) {
                WarnLogger() << "Path \"" << path << "\" is not in server save directory.";
                path = GetServerSaveDir() / path.filename();
                WarnLogger() << "Path changed to \"" << path << "\"";
            } else {
                try {
                    // ensure save directory exists
                    if (!exists(path.parent_path())) {
                        WarnLogger() << "Creating save directories " << path.parent_path().string();
                        boost::filesystem::create_directories(path.parent_path());
                    }
                } catch (const std::exception& e) {
                    ErrorLogger() << "Server unable to check / create save directory: " << e.what();
                }
            }
        }
        timer.EnterSection("");


        // set up output archive / stream for saving
        fs::ofstream ofs(path, std::ios_base::binary);
        if (!ofs)
            throw std::runtime_error(UNABLE_TO_OPEN_FILE);
        pos_before_writing = ofs.tellp();

        bool save_completed_as_xml = false;

        if (!use_binary) {
            if (use_zlib_for_zml) {
                // Attempt compressed XML serialization
                try {
                    timer.EnterSection("xml prep / allocation");
                    // Two-tier serialization:
                    // main archive is uncompressed serialized header data first
                    // then contains a string for compressed second archive
                    // that contains the main gamestate info
                    save_preview_data.SetBinary(false);
                    save_preview_data.save_format_marker = XML_COMPRESSED_BASE64_MARKER;

                    // allocate buffers for serialized gamestate
                    DebugLogger() << "Allocating buffers for XML serialization...";
                    std::string serial_str, compressed_str;
                    try {
                        DebugLogger() << "String Max Size: " << serial_str.max_size();
                        std::string::size_type capacity = std::min(serial_str.max_size(),
                            static_cast<std::string::size_type>(std::pow(2.0, 29.0)));
                        DebugLogger() << "Reserving Capacity:: " << capacity;
                        serial_str.reserve(capacity);
                        compressed_str.reserve(std::pow(2.0, 26.0));
                    }
                    catch (...) {
                        DebugLogger() << "Unable to preallocate full serialization buffers. Attempting serialization with dynamic buffer allocation.";
                    }

                    // wrap buffer string in iostream::stream to receive serialized data
                    typedef boost::iostreams::back_insert_device<std::string> InsertDevice;
                    InsertDevice serial_inserter(serial_str);
                    boost::iostreams::stream<InsertDevice> s_sink(serial_inserter);

                    timer.EnterSection("");
                    {
                        // create archive with (preallocated) buffer...
                        freeorion_xml_oarchive xoa(s_sink);
                        // serialize main gamestate info
                        timer.EnterSection("player data to xml");
                        xoa << BOOST_SERIALIZATION_NVP(player_save_game_data);
                        timer.EnterSection("empires to xml");
                        xoa << BOOST_SERIALIZATION_NVP(empire_manager);
                        timer.EnterSection("species to xml");
                        xoa << BOOST_SERIALIZATION_NVP(species_manager);
                        timer.EnterSection("combat logs to xml");
                        xoa << BOOST_SERIALIZATION_NVP(combat_log_manager);
                        timer.EnterSection("universe to xml");
                        Serialize(xoa, universe);
                        timer.EnterSection("");
                    }

                    s_sink.flush();

                    timer.EnterSection("compression");
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
                    o.push(boost::iostreams::base64_encoder());
                    o.push(c_sink);
                    boost::iostreams::copy(s_source, o);
                    c_sink.flush();

                    save_preview_data.uncompressed_text_size = serial_str.size();
                    save_preview_data.compressed_text_size = compressed_str.size();

                    timer.EnterSection("headers to xml");
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

                    timer.EnterSection("");
                    save_completed_as_xml = true;
                } catch (...) {
                    save_completed_as_xml = false;  // redundant, but here for clarity
                }
            } else {
                // Attempt direct-to-disk uncompressed XML serialization
                try {
                    save_preview_data.SetBinary(false);
                    save_preview_data.save_format_marker = XML_DIRECT_MARKER;

                    timer.EnterSection("headers to xml");
                    // create archive with (preallocated) buffer...
                    freeorion_xml_oarchive xoa(ofs);
                    xoa << BOOST_SERIALIZATION_NVP(save_preview_data);
                    xoa << BOOST_SERIALIZATION_NVP(galaxy_setup_data);
                    xoa << BOOST_SERIALIZATION_NVP(server_save_game_data);
                    xoa << BOOST_SERIALIZATION_NVP(player_save_header_data);
                    xoa << BOOST_SERIALIZATION_NVP(empire_save_game_data);

                    timer.EnterSection("gamestate to xml");
                    xoa << BOOST_SERIALIZATION_NVP(player_save_game_data);
                    xoa << BOOST_SERIALIZATION_NVP(empire_manager);
                    xoa << BOOST_SERIALIZATION_NVP(species_manager);
                    xoa << BOOST_SERIALIZATION_NVP(combat_log_manager);
                    Serialize(xoa, universe);

                    timer.EnterSection("");
                    save_completed_as_xml = true;
                } catch (...) {
                    save_completed_as_xml = false;  // redundant, but here for clarity
                }
            }
        }

        if (!save_completed_as_xml) {
            DebugLogger() << "Creating binary oarchive";
            save_preview_data.SetBinary(true);
            save_preview_data.save_format_marker = BINARY_MARKER;

            timer.EnterSection("headers to binary");
            freeorion_bin_oarchive boa(ofs);
            boa << BOOST_SERIALIZATION_NVP(save_preview_data);
            boa << BOOST_SERIALIZATION_NVP(galaxy_setup_data);
            boa << BOOST_SERIALIZATION_NVP(server_save_game_data);
            boa << BOOST_SERIALIZATION_NVP(player_save_header_data);
            boa << BOOST_SERIALIZATION_NVP(empire_save_game_data);

            timer.EnterSection("gamestate to binary");
            boa << BOOST_SERIALIZATION_NVP(player_save_game_data);
            boa << BOOST_SERIALIZATION_NVP(empire_manager);
            boa << BOOST_SERIALIZATION_NVP(species_manager);
            boa << BOOST_SERIALIZATION_NVP(combat_log_manager);
            Serialize(boa, universe);

            timer.EnterSection("");
            DebugLogger() << "Done serializing";
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
    ScopedTimer timer("LoadGame: " + filename, true);

    // player notifications
    if (ServerApp* server = ServerApp::GetApp())
        server->Networking().SendMessageAll(TurnProgressMessage(Message::LOADING_GAME));

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

        std::string signature(5, '\0');
        if (!ifs.read(&signature[0], 5))
            throw std::runtime_error(UNABLE_TO_OPEN_FILE);
        boost::iostreams::seek(ifs, 0, std::ios_base::beg);

        if (strncmp(signature.c_str(), "<?xml", 5)) {
            // XML file format signature not found; try as binary
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
        } else {
            // create archive with (preallocated) buffer...
            freeorion_xml_iarchive xia(ifs);
            DebugLogger() << "Reading XML iarchive";
            // read from save file: uncompressed header serialized data, with compressed main archive string at end...
            // deserialize uncompressed save header info
            xia >> BOOST_SERIALIZATION_NVP(ignored_save_preview_data);
            xia >> BOOST_SERIALIZATION_NVP(galaxy_setup_data);
            xia >> BOOST_SERIALIZATION_NVP(server_save_game_data);
            xia >> BOOST_SERIALIZATION_NVP(ignored_player_save_header_data);
            xia >> BOOST_SERIALIZATION_NVP(ignored_save_game_empire_data);


            if (ignored_save_preview_data.save_format_marker == XML_DIRECT_MARKER) {
                // deserialize directly from file / disk to gamestate

                xia >> BOOST_SERIALIZATION_NVP(player_save_game_data);
                xia >> BOOST_SERIALIZATION_NVP(empire_manager);
                xia >> BOOST_SERIALIZATION_NVP(species_manager);
                xia >> BOOST_SERIALIZATION_NVP(combat_log_manager);
                Deserialize(xia, universe);

            } else {
                // assume compressed XML
                if (BOOST_VERSION >= 106600 && ignored_save_preview_data.save_format_marker == XML_COMPRESSED_MARKER)
                    throw std::invalid_argument("Save Format Not Compatible with Boost Version " BOOST_LIB_VERSION);

                // allocate buffers for compressed and deceompressed serialized gamestate
                DebugLogger() << "Allocating buffers for XML deserialization...";
                std::string serial_str, compressed_str;
                try {
                    if (ignored_save_preview_data.uncompressed_text_size > 0) {
                        DebugLogger() << "Based on header info for uncompressed state string, attempting to reserve: " << ignored_save_preview_data.uncompressed_text_size << " bytes";
                        serial_str.reserve(ignored_save_preview_data.uncompressed_text_size);
                    } else {
                        serial_str.reserve(std::pow(2.0, 29.0));
                    }
                    if (ignored_save_preview_data.compressed_text_size > 0) {
                        DebugLogger() << "Based on header info for compressed state string, attempting to reserve: " << ignored_save_preview_data.compressed_text_size << " bytes";
                        compressed_str.reserve(ignored_save_preview_data.compressed_text_size);
                    } else {
                        compressed_str.reserve(std::pow(2.0, 26.0));
                    }
                } catch (...) {
                    DebugLogger() << "Unable to preallocate full deserialization buffers. Attempting deserialization with dynamic buffer allocation.";
                }

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
                if (ignored_save_preview_data.save_format_marker == XML_COMPRESSED_BASE64_MARKER)
                    i.push(boost::iostreams::base64_decoder());
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

        std::string signature(5, '\0');
        if (!ifs.read(&signature[0], 5))
            throw std::runtime_error(UNABLE_TO_OPEN_FILE);
        boost::iostreams::seek(ifs, 0, std::ios_base::beg);

        if (strncmp(signature.c_str(), "<?xml", 5)) {
            // XML file format signature not found; try as binary
            DebugLogger() << "Attempting binary deserialization...";
            freeorion_bin_iarchive ia(ifs);

            ia >> BOOST_SERIALIZATION_NVP(ignored_save_preview_data);
            ia >> BOOST_SERIALIZATION_NVP(galaxy_setup_data);

        } else {
            DebugLogger() << "Attempting XML deserialization...";
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

        std::string signature(5, '\0');
        if (!ifs.read(&signature[0], 5))
            throw std::runtime_error(UNABLE_TO_OPEN_FILE);
        boost::iostreams::seek(ifs, 0, std::ios_base::beg);

        if (strncmp(signature.c_str(), "<?xml", 5)) {
            // XML file format signature not found; try as binary
            DebugLogger() << "Attempting binary deserialization...";
            freeorion_bin_iarchive ia(ifs);

            ia >> BOOST_SERIALIZATION_NVP(ignored_save_preview_data);
            ia >> BOOST_SERIALIZATION_NVP(ignored_galaxy_setup_data);
            ia >> BOOST_SERIALIZATION_NVP(ignored_server_save_game_data);
            ia >> BOOST_SERIALIZATION_NVP(player_save_header_data);
        } else {
            DebugLogger() << "Attempting XML deserialization...";
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

void LoadEmpireSaveGameData(const std::string& filename,
                            std::map<int, SaveGameEmpireData>& empire_save_game_data,
                            std::vector<PlayerSaveHeaderData>& player_save_header_data,
                            GalaxySetupData& galaxy_setup_data,
                            int& current_turn)
{
    SaveGamePreviewData                 ignored_save_preview_data;
    ServerSaveGameData                  saved_server_save_game_data;
    GalaxySetupData                     saved_galaxy_setup_data;

    ScopedTimer timer("LoadEmpireSaveGameData: " + filename, true);

    try {
        fs::path path = FilenameToPath(filename);
        DebugLogger() << "LoadEmpireSaveGameData: filename: " << filename << " path:" << path;
        fs::ifstream ifs(path, std::ios_base::binary);

        if (!ifs)
            throw std::runtime_error(UNABLE_TO_OPEN_FILE);

        std::string signature(5, '\0');
        if (!ifs.read(&signature[0], 5))
            throw std::runtime_error(UNABLE_TO_OPEN_FILE);
        boost::iostreams::seek(ifs, 0, std::ios_base::beg);

        if (strncmp(signature.c_str(), "<?xml", 5)) {
            // XML file format signature not found; try as binary
            DebugLogger() << "Attempting binary deserialization...";
            freeorion_bin_iarchive ia(ifs);

            ia >> BOOST_SERIALIZATION_NVP(ignored_save_preview_data);
            ia >> BOOST_SERIALIZATION_NVP(saved_galaxy_setup_data);
            ia >> BOOST_SERIALIZATION_NVP(saved_server_save_game_data);
            ia >> BOOST_SERIALIZATION_NVP(player_save_header_data);
            ia >> BOOST_SERIALIZATION_NVP(empire_save_game_data);

        } else {
            DebugLogger() << "Attempting XML deserialization...";
            freeorion_xml_iarchive ia(ifs);

            ia >> BOOST_SERIALIZATION_NVP(ignored_save_preview_data);
            ia >> BOOST_SERIALIZATION_NVP(saved_galaxy_setup_data);
            ia >> BOOST_SERIALIZATION_NVP(saved_server_save_game_data);
            ia >> BOOST_SERIALIZATION_NVP(player_save_header_data);
            ia >> BOOST_SERIALIZATION_NVP(empire_save_game_data);
        }
        // skipping additional deserialization which is not needed for this function

    } catch (const std::exception& e) {
        ErrorLogger() << UserString("UNABLE_TO_READ_SAVE_FILE") << " LoadEmpireSaveGameData exception: " << ": " << e.what();
        throw e;
    }
    galaxy_setup_data.m_seed = saved_galaxy_setup_data.m_seed;
    galaxy_setup_data.m_size = saved_galaxy_setup_data.m_size;
    galaxy_setup_data.m_shape = saved_galaxy_setup_data.m_shape;
    galaxy_setup_data.m_age = saved_galaxy_setup_data.m_age;
    galaxy_setup_data.m_starlane_freq = saved_galaxy_setup_data.m_starlane_freq;
    galaxy_setup_data.m_planet_density = saved_galaxy_setup_data.m_planet_density;
    galaxy_setup_data.m_specials_freq = saved_galaxy_setup_data.m_specials_freq;
    galaxy_setup_data.m_monster_freq = saved_galaxy_setup_data.m_monster_freq;
    galaxy_setup_data.m_native_freq = saved_galaxy_setup_data.m_native_freq;
    galaxy_setup_data.m_ai_aggr = saved_galaxy_setup_data.m_ai_aggr;
    galaxy_setup_data.m_game_rules = saved_galaxy_setup_data.m_game_rules;
    galaxy_setup_data.m_game_uid = saved_galaxy_setup_data.m_game_uid;

    current_turn = saved_server_save_game_data.m_current_turn;
}
