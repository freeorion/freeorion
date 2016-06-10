#include "ServerFramework.h"

#include "ServerWrapper.h"
#include "../CommonWrappers.h"

#include "../../util/Logger.h"
#include "../../universe/Universe.h"
#include "../../universe/UniverseGenerator.h"

#include <boost/filesystem.hpp>
#include <boost/python.hpp>
#include <boost/python/docstring_options.hpp>


#include <stdexcept>

using boost::python::object;
using boost::python::import;
using boost::python::error_already_set;
using boost::python::dict;

namespace fs = boost::filesystem;

BOOST_PYTHON_MODULE(freeorionserver) {
    boost::python::docstring_options doc_options(true, true, false);
    FreeOrionPython::WrapGameStateEnums();
    FreeOrionPython::WrapGalaxySetupData();
    FreeOrionPython::WrapEmpire();
    FreeOrionPython::WrapUniverseClasses();
    FreeOrionPython::WrapServer();
}

namespace {
}

bool PythonServer::InitModules() {
    DebugLogger() << "Initializing server Python modules";

    // Allow the "freeorionserver" C++ module to be imported within Python code
    try {
        initfreeorionserver();
    } catch (...) {
        ErrorLogger() << "Unable to initialize 'freeorionserver' server Python module";
        return false;
    }

    try {
        // import universe generator script file
        m_python_module_universe_generator = import("freeorion.universe_generation.universe_generator");
    }
    catch (error_already_set err) {
        ErrorLogger() << "Unable to import universe generator script";
        PyErr_Print();
        return false;
    }

    try {
        // import universe generator script file
        m_python_module_turn_events = import("freeorion.turn_events");
    }
    catch (error_already_set err) {
        ErrorLogger() << "Unable to import freeorion.turn_events script";
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
