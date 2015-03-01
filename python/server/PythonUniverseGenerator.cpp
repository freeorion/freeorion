#include "PythonUniverseGenerator.h"

#include "../../universe/ShipDesign.h"

#include "../../server/ServerApp.h"
#include "../../util/Directories.h"
#include "../../util/Logger.h"
#include "../../util/Random.h"
#include "../../util/OptionsDB.h"

#include "../PythonSetWrapper.h"
#include "../PythonWrappers.h"
#include "PythonFramework.h"
#include "PythonServerWrapper.h"

#include <vector>
#include <map>
#include <string>
#include <utility>

#include <boost/lexical_cast.hpp>
#include <boost/python.hpp>
#include <boost/python/list.hpp>
#include <boost/python/tuple.hpp>
#include <boost/python/extract.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>

#ifdef FREEORION_MACOSX
#include <sys/param.h>
#endif

using boost::python::class_;
using boost::python::def;
using boost::python::init;
using boost::python::no_init;
using boost::noncopyable;
using boost::python::return_value_policy;
using boost::python::copy_const_reference;
using boost::python::reference_existing_object;
using boost::python::return_by_value;
using boost::python::return_internal_reference;

using boost::python::object;
using boost::python::import;
using boost::python::error_already_set;
using boost::python::exec;
using boost::python::dict;
using boost::python::list;
using boost::python::tuple;
using boost::python::make_tuple;
using boost::python::extract;
using boost::python::len;


namespace {
    // Copy of player setup data last passed to GenerateUniverse
    std::map<int, PlayerSetupData> player_setup_data;

    // Returns the global PlayerSetupData map
    std::map<int, PlayerSetupData>& GetPlayerSetupDataQ()
    { return player_setup_data; }

    // Returns the global GalaxySetupData instance
    GalaxySetupData& GetGalaxySetupDataQ()
    { return ServerApp::GetApp()->GetGalaxySetupData(); }
}


// Python module providing the universe generator API
BOOST_PYTHON_MODULE(fo_universe_generator) {
    def("get_galaxy_setup_data",                GetGalaxySetupDataQ,            return_value_policy<reference_existing_object>());
    def("get_player_setup_data",                GetPlayerSetupDataQ,            return_value_policy<reference_existing_object>());

    FreeOrionPython::WrapGameStateEnums();
    FreeOrionPython::WrapGalaxySetupData();
    WrapServerAPI();
}


namespace {
    // Global reference to imported Python universe generator module
    static object s_python_module = object();

    // Prepares Python interpreter and environment
    bool PreparePythonEnvironment() {
        // Setup and run Python interpreter
        if (!PythonInit())
            return false;

        // Allow the "fo_universe_generator" C++ module to be imported within Python code
        try {
            initfo_universe_generator();
        }
        catch (...) {
            Logger().errorStream() << "Unable to initialize fo_universe_generator interface";
            return false;
        }

        // Set Python current work directory to directory containing
        // the universe generation Python scripts...
        std::string universe_generation_script_dir = GetResourceDir().string() + "/universe_generation";
        if (!PythonSetCurrentDir(universe_generation_script_dir))
            return false;
        // ...and also add it to Pythons sys.path to make sure Python will find our scripts
        if (!PythonAddToSysPath(universe_generation_script_dir))
            return false;

        try {
            // import universe generator script file
            s_python_module = import("universe_generator");
        }
        catch (error_already_set err) {
            Logger().errorStream() << "Unable to import universe generator script";
            PyErr_Print();
            return false;
        }

        return true;
    }

    // Wraps call to the Python universe generator error report function
    std::vector<std::string> PythonErrorReport() {
        std::vector<std::string> err_list;
        object f = s_python_module.attr("error_report");
        if (!f) {
            Logger().errorStream() << "Unable to call Python function error_report ";
            return err_list;
        }

        list py_err_list;
        try { py_err_list = extract<list>(f()); }
        catch (error_already_set err) {
            PyErr_Print();
            return err_list;
        }

        for(int i = 0; i < len(py_err_list); i++) {
            err_list.push_back(extract<std::string>(py_err_list[i]));
        }
        return err_list;
    }

    // Wraps call to the main Python universe generator function
    bool PythonCreateUniverse() {
        bool success;
        object f = s_python_module.attr("create_universe");
        if (!f) {
            Logger().errorStream() << "Unable to call Python function create_universe ";
            return false;
        }
        try { success = f(); }
        catch (error_already_set err) {
            success = false;
            PyErr_Print();
        }
        return success;
    }
}


void GenerateUniverse(const std::map<int, PlayerSetupData>& player_setup_data_) {
    Universe& universe = GetUniverse();

    player_setup_data = player_setup_data_;

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
        Logger().debugStream() << "CreateUniverse:: using clock for Seed:" << new_seed;
        seed = static_cast<unsigned int>(h);
        // store seed in galaxy setup data
        ServerApp::GetApp()->GetGalaxySetupData().m_seed = boost::lexical_cast<std::string>(seed);
    }
    Seed(seed);
    Logger().debugStream() << "GenerateUniverse with seed: " << seed;

    // Fire up Python
    PreparePythonEnvironment();

    // Reset the universe object for a new universe
    universe.ResetUniverse();
    // Add predefined ship designs to universe
    GetPredefinedShipDesignManager().AddShipDesignsToUniverse();
    // Initialize empire objects for each player
    InitEmpires(player_setup_data);
    // Call the main Python universe generator function
    if (!PythonCreateUniverse()) {
        ServerApp::GetApp()->Networking().SendMessage(ErrorMessage("SERVER_UNIVERSE_GENERATION_ERRORS", false));
    }

    // Stop and clean up Python interpreter
    PythonCleanup();
    // Release resources (TODO: is that really necessary?)
    s_python_module = object();

    Logger().debugStream() << "Applying first turn effects and updating meters";

    // Apply effects for 1st turn.
    universe.ApplyAllEffectsAndUpdateMeters();
    // Set active meters to targets or maxes after first meter effects application
    SetActiveMetersToTargetMaxCurrentValues(universe.Objects());
    universe.BackPropegateObjectMeters();
    Empires().BackPropegateMeters();

    Logger().debugStream() << "Re-applying first turn meter effects and updating meters";

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
        Logger().debugStream() << "!!!!!!!!!!!!!!!!!!! After setting active meters to targets";
        Logger().debugStream() << universe.Objects().Dump();
    }

    universe.UpdateEmpireObjectVisibilities();
}
