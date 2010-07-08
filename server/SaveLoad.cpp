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
}

void SaveGame(const std::string& filename, const ServerSaveGameData& server_save_game_data,
              const std::vector<PlayerSaveGameData>& player_save_game_data,
              const Universe& universe, const EmpireManager& empire_manager)
{
    Universe::s_encoding_empire = ALL_EMPIRES;

    std::map<int, SaveGameEmpireData> empire_save_game_data = CompileSaveGameEmpireData(empire_manager);

    std::ofstream ofs(filename.c_str(), std::ios_base::binary);
    FREEORION_OARCHIVE_TYPE oa(ofs);
    oa << BOOST_SERIALIZATION_NVP(server_save_game_data);
    oa << BOOST_SERIALIZATION_NVP(player_save_game_data);
    oa << BOOST_SERIALIZATION_NVP(empire_save_game_data);
    oa << BOOST_SERIALIZATION_NVP(empire_manager);
    Serialize(oa, universe);
}

void LoadGame(const std::string& filename, ServerSaveGameData& server_save_game_data,
              std::vector<PlayerSaveGameData>& player_save_game_data,
              Universe& universe, EmpireManager& empire_manager)
{
    Universe::s_encoding_empire = ALL_EMPIRES;

    std::map<int, SaveGameEmpireData> ignored_save_game_empire_data;

    empire_manager.Clear();
    universe.Clear();

    try {
        std::ifstream ifs(filename.c_str(), std::ios_base::binary);
        FREEORION_IARCHIVE_TYPE ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(server_save_game_data);
        ia >> BOOST_SERIALIZATION_NVP(player_save_game_data);
        ia >> BOOST_SERIALIZATION_NVP(ignored_save_game_empire_data);
        ia >> BOOST_SERIALIZATION_NVP(empire_manager);
        Deserialize(ia, universe);
    } catch (const boost::archive::archive_exception err) {
        Logger().errorStream() << "LoadGame(...) failed!";
        throw err;
    }
}

void LoadPlayerSaveGameData(const std::string& filename, std::vector<PlayerSaveGameData>& player_save_game_data)
{
    ServerSaveGameData ignored_server_save_game_data;

    try {
        std::ifstream ifs(filename.c_str(), std::ios_base::binary);
        FREEORION_IARCHIVE_TYPE ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(ignored_server_save_game_data);
        ia >> BOOST_SERIALIZATION_NVP(player_save_game_data);
        // skipping additional deserialization which is not needed for this function
    } catch (const boost::archive::archive_exception err) {
        Logger().errorStream() << "LoadPlayerSaveGameData(...) failed!";
        throw err;
    }
}

void LoadEmpireSaveGameData(const std::string& filename, std::map<int, SaveGameEmpireData>& empire_save_game_data)
{
    ServerSaveGameData              ignored_server_save_game_data;
    std::vector<PlayerSaveGameData> ignored_player_save_game_data;

    try {
        std::ifstream ifs(filename.c_str(), std::ios_base::binary);
        FREEORION_IARCHIVE_TYPE ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(ignored_server_save_game_data);
        ia >> BOOST_SERIALIZATION_NVP(ignored_player_save_game_data);
        ia >> BOOST_SERIALIZATION_NVP(empire_save_game_data);
        // skipping additional deserialization which is not needed for this function
    } catch (const boost::archive::archive_exception err) {
        Logger().errorStream() << "LoadEmpireSaveGameData(...) failed!";
        throw err;
    }
}

