#include <boost/test/unit_test.hpp>
#include <boost/test/tools/assertion_result.hpp>

#include "combat/CombatLogManager.h"
#include "Empire/EmpireManager.h"
#include "server/SaveLoad.h"
#include "universe/Species.h"
#include "universe/Universe.h"
#include "util/Directories.h"
#include "util/MultiplayerCommon.h"

BOOST_AUTO_TEST_SUITE(EmptySave)

BOOST_AUTO_TEST_CASE(empty_save) {
    InitDirs(boost::unit_test::framework::master_test_suite().argv[0], true);

    ServerSaveGameData server_save_game_data;
    std::vector<PlayerSaveGameData> player_save_game_data;
    Universe universe;
    EmpireManager empire_manager;
    SpeciesManager species_manager;
    CombatLogManager combat_log_manager;
    GalaxySetupData galaxy_setup_data;

    std::string filename = "test-save";

    SaveGame(filename,
             server_save_game_data,
             player_save_game_data,
             universe,
             empire_manager,
             species_manager,
             combat_log_manager,
             galaxy_setup_data,
             false);

    ServerSaveGameData server_save_game_data2;
    std::vector<PlayerSaveGameData> player_save_game_data2;
    Universe universe2;
    EmpireManager empire_manager2;
    SpeciesManager species_manager2;
    CombatLogManager combat_log_manager2;
    GalaxySetupData galaxy_setup_data2;

    auto path = FilenameToPath(filename);
    path = GetSaveDir() / path;

    BOOST_CHECK(LoadGame(PathToString(path),
             server_save_game_data2,
             player_save_game_data2,
             universe2,
             empire_manager2,
             species_manager2,
             combat_log_manager2,
             galaxy_setup_data2));
}

struct check_env {
    boost::test_tools::assertion_result operator()(boost::unit_test::test_unit_id) {
        const char* filename = std::getenv("FO_LOAD_GAME_PATH");
        if (filename != nullptr && filename[0] != '\0')
            return true;

        boost::test_tools::assertion_result ans(false);
        ans.message() << "path FO_LOAD_GAME_PATH wasn't set";
        return ans;
    }
};

/**
 * Load saved game from path FO_LOAD_GAME_PATH
 */

BOOST_AUTO_TEST_CASE(env_save, *boost::unit_test::precondition(check_env())) {
    const char* filename = std::getenv("FO_LOAD_GAME_PATH");

    ServerSaveGameData server_save_game_data2;
    std::vector<PlayerSaveGameData> player_save_game_data2;
    Universe universe2;
    EmpireManager empire_manager2;
    SpeciesManager species_manager2;
    CombatLogManager combat_log_manager2;
    GalaxySetupData galaxy_setup_data2;

    BOOST_CHECK(LoadGame(filename,
             server_save_game_data2,
             player_save_game_data2,
             universe2,
             empire_manager2,
             species_manager2,
             combat_log_manager2,
             galaxy_setup_data2));
}

BOOST_AUTO_TEST_SUITE_END()

