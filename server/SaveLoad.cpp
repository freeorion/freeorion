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
#include "../util/OrderSet.h"
#include "../util/Serialize.h"

#include <boost/serialization/deque.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/vector.hpp>

#include <fstream>

namespace {
    std::map<int, SaveGameEmpireData> CompileSaveGameEmpireData(const EmpireManager& empire_manager) {
        std::map<int, SaveGameEmpireData> retval;
        const EmpireManager& empires = Empires();
        for (EmpireManager::const_iterator it = empires.begin(); it != empires.end(); ++it)
            retval[it->first] = SaveGameEmpireData(it->first, it->second->Name(), it->second->PlayerName(), it->second->Color());
        return retval;
    }
    const std::string UNABLE_TO_OPEN_FILE("Unable to open file");
}

void SaveGame(const std::string& filename, const ServerSaveGameData& server_save_game_data,
              const std::vector<PlayerSaveGameData>& player_save_game_data,
              const Universe& universe, const EmpireManager& empire_manager,
              const SpeciesManager& species_manager)
{
    Universe::s_encoding_empire = ALL_EMPIRES;

    std::map<int, SaveGameEmpireData> empire_save_game_data = CompileSaveGameEmpireData(empire_manager);

    try {
        std::ofstream ofs(filename.c_str(), std::ios_base::binary);
        if (!ofs)
            throw std::runtime_error(UNABLE_TO_OPEN_FILE);
        FREEORION_OARCHIVE_TYPE oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(server_save_game_data);
        oa << BOOST_SERIALIZATION_NVP(player_save_game_data);
        oa << BOOST_SERIALIZATION_NVP(empire_save_game_data);
        oa << BOOST_SERIALIZATION_NVP(empire_manager);
        oa << BOOST_SERIALIZATION_NVP(species_manager);
        Serialize(oa, universe);
    } catch (const std::exception& e) {
        Logger().errorStream() << UserString("UNABLE_TO_WRITE_SAVE_FILE") << " SaveGame exception: " << ": " << e.what();
        throw e;
    }
}

void LoadGame(const std::string& filename, ServerSaveGameData& server_save_game_data,
              std::vector<PlayerSaveGameData>& player_save_game_data,
              Universe& universe, EmpireManager& empire_manager, SpeciesManager& species_manager)
{
    Universe::s_encoding_empire = ALL_EMPIRES;

    std::map<int, SaveGameEmpireData> ignored_save_game_empire_data;

    empire_manager.Clear();
    universe.Clear();

    try {
        std::ifstream ifs(filename.c_str(), std::ios_base::binary);
        if (!ifs)
            throw std::runtime_error(UNABLE_TO_OPEN_FILE);
        FREEORION_IARCHIVE_TYPE ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(server_save_game_data);
        ia >> BOOST_SERIALIZATION_NVP(player_save_game_data);
        ia >> BOOST_SERIALIZATION_NVP(ignored_save_game_empire_data);
        ia >> BOOST_SERIALIZATION_NVP(empire_manager);
        ia >> BOOST_SERIALIZATION_NVP(species_manager);
        Deserialize(ia, universe);
    } catch (const std::exception& e) {
        Logger().errorStream() << UserString("UNABLE_TO_READ_SAVE_FILE") << " LoadGame exception: " << ": " << e.what();
        throw e;
    }
}

void LoadPlayerSaveGameData(const std::string& filename, std::vector<PlayerSaveGameData>& player_save_game_data)
{
    ServerSaveGameData ignored_server_save_game_data;

    try {
        std::ifstream ifs(filename.c_str(), std::ios_base::binary);
        if (!ifs)
            throw std::runtime_error(UNABLE_TO_OPEN_FILE);
        FREEORION_IARCHIVE_TYPE ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(ignored_server_save_game_data);
        ia >> BOOST_SERIALIZATION_NVP(player_save_game_data);
        // skipping additional deserialization which is not needed for this function
    } catch (const std::exception& e) {
        Logger().errorStream() << UserString("UNABLE_TO_READ_SAVE_FILE") << " LoadPlayerSaveGameData exception: " << ": " << e.what();
        throw e;
    }
}

void LoadEmpireSaveGameData(const std::string& filename, std::map<int, SaveGameEmpireData>& empire_save_game_data)
{
    ServerSaveGameData              ignored_server_save_game_data;
    std::vector<PlayerSaveGameData> ignored_player_save_game_data;

    try {
        std::ifstream ifs(filename.c_str(), std::ios_base::binary);
        if (!ifs)
            throw std::runtime_error(UNABLE_TO_OPEN_FILE);
        FREEORION_IARCHIVE_TYPE ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(ignored_server_save_game_data);
        ia >> BOOST_SERIALIZATION_NVP(ignored_player_save_game_data);
        ia >> BOOST_SERIALIZATION_NVP(empire_save_game_data);
        // skipping additional deserialization which is not needed for this function
    } catch (const std::exception& e) {
        Logger().errorStream() << UserString("UNABLE_TO_READ_SAVE_FILE") << " LoadEmpireSaveGameData exception: " << ": " << e.what();
        throw e;
    }
}

