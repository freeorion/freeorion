#include "PythonServerFramework.h"

#include "PythonServerWrapper.h"
#include "../PythonWrappers.h"

#include "../../util/Logger.h"
#include "../../universe/Universe.h"
#include "../../universe/UniverseGenerator.h"

#include <boost/filesystem.hpp>
#include <boost/python.hpp>

#include <stdexcept>

using boost::python::object;
using boost::python::import;
using boost::python::error_already_set;
using boost::python::dict;

namespace fs = boost::filesystem;

BOOST_PYTHON_MODULE(freeorion) {
    FreeOrionPython::WrapGameStateEnums();
    FreeOrionPython::WrapGalaxySetupData();
    WrapServerAPI();
}

namespace {
}

bool PythonServer::InitModules() {
    DebugLogger() << "Initializing server Python modules";

    // Allow the "freeorion" C++ module to be imported within Python code
    try {
        initfreeorion();
    } catch (...) {
        ErrorLogger() << "Unable to initialize 'freeorion' server Python module";
        return false;
    }

    // Confirm existence of the directory containing the universe generation
    // Python scripts and add it to Pythons sys.path to make sure Python will
    // find our scripts
    if (!fs::exists(GetPythonUniverseGeneratorDir())) {
        ErrorLogger() << "Can't find folder containing universe generation scripts";
        return false;
    }
    if (!AddToSysPath(GetPythonUniverseGeneratorDir())) {
        ErrorLogger() << "Can't add folder containing universe generation scripts to sys.path";
        return false;
    }

    try {
        // import universe generator script file
        m_python_module_universe_generator = import("universe_generator");
    }
    catch (error_already_set err) {
        ErrorLogger() << "Unable to import universe generator script";
        PyErr_Print();
        return false;
    }

    // Confirm existence of the directory containing the turn event Python
    // scripts and add it to Pythons sys.path to make sure Python will find
    // our scripts
    if (!fs::exists(GetPythonTurnEventsDir())) {
        ErrorLogger() << "Can't find folder containing turn events scripts";
        return false;
    }
    if (!AddToSysPath(GetPythonTurnEventsDir())) {
        ErrorLogger() << "Can't add folder containing turn events scripts to sys.path";
        return false;
    }

    try {
        // import universe generator script file
        m_python_module_turn_events = import("turn_events");
    }
    catch (error_already_set err) {
        ErrorLogger() << "Unable to import turn events script";
        PyErr_Print();
        return false;
    }

    DebugLogger() << "Server Python modules successfully initialized!";
    return true;
}

bool PythonServer::CreateUniverse(std::map<int, PlayerSetupData>& player_setup_data) {
    dict py_player_setup_data;
    bool success;

    // the universe generator module should contain an "error_report" function,
    // so set the ErrorReport member function to use it
    SetErrorModule(m_python_module_universe_generator);

    for (std::map<int, PlayerSetupData>::iterator it = player_setup_data.begin(); it != player_setup_data.end(); ++it) {
        py_player_setup_data[it->first] = object(it->second);
    }

    object f = m_python_module_universe_generator.attr("create_universe");
    if (!f) {
        ErrorLogger() << "Unable to call Python function create_universe ";
        return false;
    }

    try { success = f(py_player_setup_data); }
    catch (error_already_set err) {
        success = false;
        PyErr_Print();
    }
    return success;
}

bool PythonServer::ExecuteTurnEvents() {
    bool success;
    object f = m_python_module_turn_events.attr("execute_turn_events");
    if (!f) {
        ErrorLogger() << "Unable to call Python function execute_turn_events ";
        return false;
    }
    try { success = f(); }
    catch (error_already_set err) {
        success = false;
        PyErr_Print();
    }
    return success;
}

const std::string GetPythonUniverseGeneratorDir()
{ return GetPythonDir() + "/universe_generation"; }

const std::string GetPythonTurnEventsDir()
{ return GetPythonDir() + "/turn_events"; }
