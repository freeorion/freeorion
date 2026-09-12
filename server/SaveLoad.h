#ifndef _SaveLoad_h_
#define _SaveLoad_h_

#include <filesystem>
#include <map>
#include <string>
#include <vector>

class CombatLogManager;
class EmpireManager;
class SpeciesManager;
class Universe;
struct GalaxySetupData;
struct PlayerSaveGameData;
struct PlayerSaveHeaderData;
struct SaveGameEmpireData;
struct ServerSaveGameData;

/** Prepared empire data for save game or lobby. */
std::map<int, SaveGameEmpireData> CompileSaveGameEmpireData(const EmpireManager& empires);

/** Saves the provided data to savefile \a filename. */
int SaveGame(std::filesystem::path path,
             const ServerSaveGameData& server_save_game_data,
             const std::vector<PlayerSaveGameData>& player_save_game_data,
             const Universe& universe,
             const EmpireManager& empire_manager,
             const SpeciesManager& species_manager,
             const CombatLogManager& combat_log_manager,
             GalaxySetupData galaxy_setup_data,
             bool multiplayer);
int SaveGame(auto, const ServerSaveGameData&, const std::vector<PlayerSaveGameData>&,
             const Universe&, const EmpireManager&, const SpeciesManager&,
             const CombatLogManager&, GalaxySetupData, bool) = delete;

/** Loads the indicated data from savefile \a filename. */
[[nodiscard]] bool LoadGame(const std::filesystem::path& path,
                            ServerSaveGameData& server_save_game_data,
                            std::vector<PlayerSaveGameData>& player_save_game_data,
                            Universe& universe,
                            EmpireManager& empire_manager,
                            SpeciesManager& species_manager,
                            CombatLogManager& combat_log_manager,
                            GalaxySetupData& galaxy_setup_data);

bool LoadGame(auto, ServerSaveGameData&, std::vector<PlayerSaveGameData>&, Universe&,
              EmpireManager&, SpeciesManager&, CombatLogManager&, GalaxySetupData&) = delete;

/** Loads from a savefile \a filename some basic info about players in the save
  * that is needed when resuming the game. */
void LoadPlayerSaveHeaderData(const std::string& filename,
                              std::vector<PlayerSaveHeaderData>& player_save_header_data);


/** Loads from a savefile \a file_path some basic empire information that is
  * useful when selecting which player will control which empire when reloading
  * a saved game: player name, empire name, and empire colour (and empire id).
  * Also loads galaxy setup data to show it in lobby window. */
void LoadEmpireSaveGameData(const std::filesystem::path& file_path,
                            std::map<int, SaveGameEmpireData>& empire_save_game_data,
                            std::vector<PlayerSaveHeaderData>& player_save_header_data,
                            GalaxySetupData& galaxy_setup_data,
                            int &current_turn);

#endif
