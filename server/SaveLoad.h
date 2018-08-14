#ifndef _SaveLoad_h_
#define _SaveLoad_h_

#include <vector>
#include <map>
#include <string>

class CombatLogManager;
class EmpireManager;
class SpeciesManager;
class Universe;
struct GalaxySetupData;
struct PlayerSaveGameData;
struct PlayerSaveHeaderData;
struct SaveGameEmpireData;
struct ServerSaveGameData;

/** Saves the provided data to savefile \a filename. */
int SaveGame(const std::string& filename,
             const ServerSaveGameData& server_save_game_data,
             const std::vector<PlayerSaveGameData>& player_save_game_data,
             const Universe& universe,
             const EmpireManager& empire_manager,
             const SpeciesManager& species_manager,
             const CombatLogManager& combat_log_manager,
             const GalaxySetupData& galaxy_setup_data,
             bool multiplayer);

/** Loads the indicated data from savefile \a filename. */
void LoadGame(const std::string& filename,
              ServerSaveGameData& server_save_game_data,
              std::vector<PlayerSaveGameData>& player_save_game_data,
              Universe& universe,
              EmpireManager& empire_manager,
              SpeciesManager& species_manager,
              CombatLogManager& combat_log_manager,
              GalaxySetupData& galaxy_setup_data);

/** Loads from a savefile \a filename some basic info about players in the save
  * that is needed when resuming the game. */
void LoadPlayerSaveHeaderData(const std::string& filename,
                              std::vector<PlayerSaveHeaderData>& player_save_header_data);


/** Loads from a savefile \a filename some basic empire information that is
  * useful when selecting which player will control which empire when reloading
  * a saved game: player name, empire name, and empire colour (and empire id). */
void LoadEmpireSaveGameData(const std::string& filename,
                            std::map<int, SaveGameEmpireData>& empire_save_game_data,
                            std::vector<PlayerSaveHeaderData>& player_save_header_data);

#endif
