// -*- C++ -*-
#ifndef _SaveLoad_h_
#define _SaveLoad_h_

#include <vector>
#include <string>

class PlayerSaveGameData;
class ServerSaveGameData;
class Universe;

/** Saves the provided data to savefile \a filename. */
void SaveGame(const std::string& filename, const ServerSaveGameData& server_save_game_data,
              const std::vector<PlayerSaveGameData>& player_save_game_data, const Universe& universe);

/** Loads the indicated data from savefile \a filename. */
void LoadGame(const std::string& filename, ServerSaveGameData& server_save_game_data,
              std::vector<PlayerSaveGameData>& player_save_game_data, Universe& universe);

/** Loads player setup information and server state from a savefile \a filename
  * but does not load full gamestate. */
void LoadGamePlayerSetupData(const std::string& filename, ServerSaveGameData& server_save_game_data,
                             std::vector<PlayerSaveGameData>& player_save_game_data);

#endif
