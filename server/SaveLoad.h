// -*- C++ -*-
#ifndef _SaveLoad_h_
#define _SaveLoad_h_

#include <vector>
#include <map>
#include <string>

struct PlayerSaveGameData;
struct ServerSaveGameData;
struct SaveGameEmpireData;
class Universe;
class EmpireManager;

/** Saves the provided data to savefile \a filename. */
void SaveGame(const std::string& filename,
              const ServerSaveGameData& server_save_game_data,
              const std::vector<PlayerSaveGameData>& player_save_game_data,
              const Universe& universe,
              const EmpireManager& empire_manager);

/** Loads the indicated data from savefile \a filename. */
void LoadGame(const std::string& filename,
              ServerSaveGameData& server_save_game_data,
              std::vector<PlayerSaveGameData>& player_save_game_data,
              Universe& universe,
              EmpireManager& empire_manager);

/** Loads from a savefile \a filename various non-gamestate information that is
  * needed when resuming a saved game.  This includes some player setup
  * information so that the server knows how many AI clients to start and which
  * empire(s) to assign to them and the human player, and also the set of
  * orders that the player has already issued during the current turn and the
  * state of the UI or AI state information that needs to be restored when
  * resuming a game. */
void LoadPlayerSaveGameData(const std::string& filename,
                            std::vector<PlayerSaveGameData>& player_save_game_data);

/** Loads from a savefile \a filename some basic empire information that is
  * useful when selecting which player will control which empire when reloading
  * a saved game: player name, empire name, and empire colour (and empire id). */
void LoadEmpireSaveGameData(const std::string& filename,
                            std::map<int, SaveGameEmpireData>& empire_save_game_data);

#endif
