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


void SaveGame(const std::string& filename, const ServerSaveGameData& server_save_game_data, const std::vector<PlayerSaveGameData>& player_save_game_data, const Universe& universe)
{
    std::ofstream ofs(filename.c_str(), std::ios_base::binary);
    FREEORION_OARCHIVE_TYPE oa(ofs);
    oa << BOOST_SERIALIZATION_NVP(server_save_game_data);
    Universe::s_encoding_empire = ALL_EMPIRES;
    oa << BOOST_SERIALIZATION_NVP(player_save_game_data);
    Serialize(oa, universe);
}

void LoadGame(const std::string& filename, ServerSaveGameData& server_save_game_data, std::vector<PlayerSaveGameData>& player_save_game_data, Universe& universe)
{
    std::ifstream ifs(filename.c_str(), std::ios_base::binary);
    FREEORION_IARCHIVE_TYPE ia(ifs);
    ia >> BOOST_SERIALIZATION_NVP(server_save_game_data);
    Universe::s_encoding_empire = ALL_EMPIRES;
    ia >> BOOST_SERIALIZATION_NVP(player_save_game_data);
    Deserialize(ia, universe);
}
