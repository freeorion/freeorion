#include "PythonUniverseGenerator.h"

#include "../../universe/Condition.h"
#include "../../universe/Species.h"
#include "../../universe/Special.h"
#include "../../universe/System.h"
#include "../../universe/Planet.h"
#include "../../universe/Building.h"
#include "../../universe/ShipDesign.h"
#include "../../universe/Fleet.h"
#include "../../universe/Ship.h"
#include "../../universe/Tech.h"

#include "../../server/ServerApp.h"
#include "../../util/Directories.h"
#include "../../util/Logger.h"
#include "../../util/Random.h"
#include "../../util/i18n.h"
#include "../../util/OptionsDB.h"
#include "../../parse/Parse.h"

#include "../../Empire/Empire.h"
#include "../../Empire/EmpireManager.h"

#include "../PythonSetWrapper.h"
#include "../PythonWrappers.h"
#include "PythonServerWrapper.h"

#include <vector>
#include <map>
#include <string>
#include <utility>

#include <boost/lexical_cast.hpp>
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
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

using boost::python::vector_indexing_suite;
using boost::python::map_indexing_suite;

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


// Python module for logging functions
BOOST_PYTHON_MODULE(fo_logger) {
    FreeOrionPython::WrapLogger();
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
    // Some helper objects needed to initialize and run the
    // Python interface. Don't know if or why they have to
    // be exactly this way, more or less copied them over
    // from the Python AI interface.
#ifdef FREEORION_MACOSX
    static char     s_python_home[MAXPATHLEN];
    static char     s_python_program_name[MAXPATHLEN];
#endif
    static dict     s_python_namespace = dict();
    static object   s_python_module = object();

    // Helper function for executing a Python script
    bool PythonExecScript(const std::string script) {
        try { object ignored = exec(script.c_str(), s_python_namespace, s_python_namespace); }
        catch (error_already_set err) {
            PyErr_Print();
            return false;
        }
        return true;
    }

    // Initializes und runs the Python interpreter
    // Prepares the Python environment
    void PythonInit() {
        Logger().debugStream() << "Initializing universe generator Python interface";

        try {
#ifdef FREEORION_MACOSX
            // There have been recurring issues on OSX to get FO to use the
            // Python framework shipped with the app (instead of falling back
            // on the ones provided by the system). These API calls have been
            // added in an attempt to solve the problems. Not sure if they
            // are really required, but better save than sorry.. ;)
            strcpy(s_python_home, GetPythonHome().string().c_str());
            Py_SetPythonHome(s_python_home);
            Logger().debugStream() << "Python home set to " << Py_GetPythonHome();
            strcpy(s_python_program_name, (GetPythonHome() / "Python").string().c_str());
            Py_SetProgramName(s_python_program_name);
            Logger().debugStream() << "Python program name set to " << Py_GetProgramFullPath();
#endif
            // initializes Python interpreter, allowing Python functions to be called from C++
            Py_Initialize();
            Logger().debugStream() << "Python initialized";
            Logger().debugStream() << "Python version: " << Py_GetVersion();
            Logger().debugStream() << "Python prefix: " << Py_GetPrefix();
            Logger().debugStream() << "Python module search path: " << Py_GetPath();
            Logger().debugStream() << "Initializing C++ interfaces for Python";
            initfo_logger();              // allows the "fo_logger" C++ module to be imported within Python code
            initfo_universe_generator();  // allows the "fo_universe_generator" C++ module to be imported within Python code
        }
        catch (...) {
            Logger().errorStream() << "Unable to initialize Python interpreter";
            return;
        }

        try {
            // get main namespace, needed to run other interpreted code
            object py_main = import("__main__");
            s_python_namespace = extract<dict>(py_main.attr("__dict__"));
        }
        catch (error_already_set err) {
            Logger().errorStream() << "Unable to set up main namespace in Python";
            PyErr_Print();
            return;
        }

        // set up logging by redirecting stdout and stderr to exposed logging functions
        std::string script = "import sys\n"
        "import fo_logger\n"
        "class dbgLogger:\n"
        "  def write(self, msg):\n"
        "    fo_logger.log(msg)\n"
        "class errLogger:\n"
        "  def write(self, msg):\n"
        "    fo_logger.error(msg)\n"
        "sys.stdout = dbgLogger()\n"
        "sys.stderr = errLogger()\n"
        "print ('Python stdout and stderr redirected')";
        if (!PythonExecScript(script)) {
            Logger().errorStream() << "Unable to redirect Python stdout and stderr";
            return;
        }

        // set Python current work directory to directory containing
        // the universe generation Python scripts
        std::string universe_generation_script_dir = GetResourceDir().string() + "/universe_generation";
        script = "import os\n"
        "os.chdir(r'" + universe_generation_script_dir + "')\n"
        "print 'Python current directory set to', os.getcwd()";
        if (!PythonExecScript(script)) {
            Logger().errorStream() << "Unable to set Python current directory";
            return;
        }

        // tell Python the path in which to locate universe generator script file
        std::string command = "sys.path.append(r'" + universe_generation_script_dir + "')";
        if (!PythonExecScript(command)) {
            Logger().errorStream() << "Unable to set universe generator script dir";
            return;
        }

        try {
            // import universe generator script file
            s_python_module = import("universe_generator");
        }
        catch (error_already_set err) {
            Logger().errorStream() << "Unable to import universe generator script";
            PyErr_Print();
            return;
        }

        Logger().debugStream() << "Python interface successfully initialized!";
    }

    void PythonCleanup() {
        // stops Python interpreter and release its resources
        Py_Finalize();
        s_python_namespace = dict();
        s_python_module = object();
        Logger().debugStream() << "Cleaned up universe generator Python interface";
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

    // Setup and run Python interpreter
    PythonInit();

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
