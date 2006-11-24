// -*- C++ -*-
#ifndef _SaveLoad_h_
#define _SaveLoad_h_

#include "../util/Serialize.h"


class PlayerSaveGameData;

/** Saves the provided data to savefile \a filename. */
void SaveGame(const std::string& filename, int current_turn, const std::vector<PlayerSaveGameData>& player_save_game_data, const Universe& universe);

/** Loads the indicated data from savefile \a filename. */
void LoadGame(const std::string& filename, int& current_turn, std::vector<PlayerSaveGameData>& player_save_game_data, Universe& universe);

#endif
