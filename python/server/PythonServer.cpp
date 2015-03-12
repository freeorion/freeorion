#include "PythonServer.h"

#include "../../universe/ShipDesign.h"
#include "../../server/ServerApp.h"
#include "../../util/Logger.h"
#include "../../util/Random.h"
#include "../../util/OptionsDB.h"

#include "PythonServerFramework.h"

#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>


void GenerateUniverse(std::map<int, PlayerSetupData>& player_setup_data) {
    Universe& universe = GetUniverse();

    // Initialize RNG with provided seed to get reproducible universes
    int seed = 0;
    try {
        seed = boost::lexical_cast<unsigned int>(GetGalaxySetupData().m_seed);
    } catch (...) {
        try {
            boost::hash<std::string> string_hash;
            std::size_t h = string_hash(GetGalaxySetupData().m_seed);
            seed = static_cast<unsigned int>(h);
        } catch (...) {}
    }
    if (GetGalaxySetupData().m_seed.empty()) {
        //ClockSeed();
        // replicate ClockSeed code here so can log the seed used
        boost::posix_time::ptime ltime = boost::posix_time::microsec_clock::local_time();
        std::string new_seed = boost::posix_time::to_simple_string(ltime);
        boost::hash<std::string> string_hash;
        std::size_t h = string_hash(new_seed);
        DebugLogger() << "CreateUniverse:: using clock for Seed:" << new_seed;
        seed = static_cast<unsigned int>(h);
        // store seed in galaxy setup data
        ServerApp::GetApp()->GetGalaxySetupData().m_seed = boost::lexical_cast<std::string>(seed);
    }
    Seed(seed);
    DebugLogger() << "GenerateUniverse with seed: " << seed;

    // Reset the universe object for a new universe
    universe.ResetUniverse();
    // Add predefined ship designs to universe
    GetPredefinedShipDesignManager().AddShipDesignsToUniverse();
    // Initialize empire objects for each player
    InitEmpires(player_setup_data);
    // Set Python current work directory to directory containing
    // the universe generation Python scripts
    PythonSetCurrentDir(GetPythonUniverseGeneratorDir());
    // Call the main Python universe generator function
    if (!PythonCreateUniverse(player_setup_data)) {
        ServerApp::GetApp()->Networking().SendMessage(ErrorMessage("SERVER_UNIVERSE_GENERATION_ERRORS", false));
    }

    DebugLogger() << "Applying first turn effects and updating meters";

    // Apply effects for 1st turn.
    universe.ApplyAllEffectsAndUpdateMeters();
    // Set active meters to targets or maxes after first meter effects application
    SetActiveMetersToTargetMaxCurrentValues(universe.Objects());
    universe.BackPropegateObjectMeters();
    Empires().BackPropegateMeters();

    DebugLogger() << "Re-applying first turn meter effects and updating meters";

    // Re-apply meter effects, so that results depending on meter values can be
    // re-checked after initial setting of those meter values
    universe.ApplyMeterEffectsAndUpdateMeters();
    // Re-set active meters to targets after re-application of effects
    SetActiveMetersToTargetMaxCurrentValues(universe.Objects());
    // Set the population of unowned planets to a random fraction of their target values.
    SetNativePopulationValues(universe.Objects());

    universe.BackPropegateObjectMeters();
    Empires().BackPropegateMeters();

    if (GetOptionsDB().Get<bool>("verbose-logging")) {
        DebugLogger() << "!!!!!!!!!!!!!!!!!!! After setting active meters to targets";
        DebugLogger() << universe.Objects().Dump();
    }

    universe.UpdateEmpireObjectVisibilities();
}

void ExecuteScriptedTurnEvents() {
    PythonSetCurrentDir(GetPythonTurnEventsDir());
    // Call the main Python turn events function
    if (!PythonExecuteTurnEvents()) {
        ServerApp::GetApp()->Networking().SendMessage(ErrorMessage("SERVER_TURN_EVENTS_ERRORS", false));
    }
}
