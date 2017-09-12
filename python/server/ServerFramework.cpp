#include "ServerFramework.h"

#include "ServerWrapper.h"
#include "../SetWrapper.h"
#include "../CommonWrappers.h"

#include "../../util/Logger.h"
#include "../../universe/Universe.h"
#include "../../universe/UniverseGenerator.h"

#include <boost/filesystem.hpp>
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include <boost/python/docstring_options.hpp>


#include <stdexcept>

using boost::python::object;
using boost::python::class_;
using boost::python::import;
using boost::python::error_already_set;
using boost::python::dict;
using boost::python::vector_indexing_suite;
using boost::python::map_indexing_suite;

namespace fs = boost::filesystem;

BOOST_PYTHON_MODULE(freeorion) {
    boost::python::docstring_options doc_options(true, true, false);

    FreeOrionPython::WrapGameStateEnums();
    FreeOrionPython::WrapGalaxySetupData();
    FreeOrionPython::WrapEmpire();
    FreeOrionPython::WrapUniverseClasses();
    FreeOrionPython::WrapServer();

    // STL Containers
    class_<std::vector<int>>("IntVec")
        .def(vector_indexing_suite<std::vector<int>>())
    ;
    class_<std::vector<std::string>>("StringVec")
        .def(vector_indexing_suite<std::vector<std::string>>())
    ;

    class_<std::map<int, bool>>("IntBoolMap")
        .def(map_indexing_suite<std::map<int, bool>>())
    ;

    FreeOrionPython::SetWrapper<int>::Wrap("IntSet");
    FreeOrionPython::SetWrapper<std::string>::Wrap("StringSet");
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
    AddToSysPath(GetPythonUniverseGeneratorDir());

    // Confirm existence of the directory containing the turn event Python
    // scripts and add it to Pythons sys.path to make sure Python will find
    // our scripts
    if (!fs::exists(GetPythonTurnEventsDir())) {
        ErrorLogger() << "Can't find folder containing turn events scripts";
        return false;
    }
    AddToSysPath(GetPythonTurnEventsDir());

    // import universe generator script file
    m_python_module_turn_events = import("turn_events");

    // Confirm existence of the directory containing the auth Python scripts
    // and add it to Pythons sys.path to make sure Python will find our scripts
    if (!fs::exists(GetPythonAuthDir())) {
        ErrorLogger() << "Can't find folder containing auth scripts";
        return false;
    }
    AddToSysPath(GetPythonAuthDir());

    // import auth script file
    m_python_module_auth = import("auth");

    // Save AuthProvider instance in auth module's namespace
    m_python_module_auth.attr("__dict__")["auth_provider"] = m_python_module_auth.attr("AuthProvider")();

    DebugLogger() << "Server Python modules successfully initialized!";
    return true;
}

bool PythonServer::IsRequireAuth(const std::string& player_name, bool &result) const {
    boost::python::object auth_provider = m_python_module_auth.attr("__dict__")["auth_provider"];
    if (!auth_provider) {
        ErrorLogger() << "Unable to get Python object auth_provider";
        return false;
    }
    boost::python::object f = auth_provider.attr("is_require_auth");
    if (!f) {
        ErrorLogger() << "Unable to call Python method is_require_auth";
        return false;
    }
    boost::python::object r = f(player_name);
    result = boost::python::extract<bool>(r);
    return true;
}

bool PythonServer::IsSuccessAuth(const std::string& player_name, const std::string& auth, bool &result) const {
    boost::python::object auth_provider = m_python_module_auth.attr("__dict__")["auth_provider"];
    if (!auth_provider) {
        ErrorLogger() << "Unable to get Python object auth_provider";
        return false;
    }
    boost::python::object f = auth_provider.attr("is_success_auth");
    if (!f) {
        ErrorLogger() << "Unable to call Python method is_success_auth";
        return false;
    }
    boost::python::object r = f(player_name, auth);
    result = boost::python::extract<bool>(r);
    return true;
}

bool PythonServer::CreateUniverse(std::map<int, PlayerSetupData>& player_setup_data) {
    // Confirm existence of the directory containing the universe generation
    // Python scripts and add it to Pythons sys.path to make sure Python will
    // find our scripts
    if (!fs::exists(GetPythonUniverseGeneratorDir())) {
        ErrorLogger() << "Can't find folder containing universe generation scripts";
        return false;
    }
    AddToSysPath(GetPythonUniverseGeneratorDir());

    // import universe generator script file
    m_python_module_universe_generator = import("universe_generator");

    dict py_player_setup_data;

    // the universe generator module should contain an "error_report" function,
    // so set the ErrorReport member function to use it
    SetErrorModule(m_python_module_universe_generator);

    for (auto& psd : player_setup_data) {
        py_player_setup_data[psd.first] = object(psd.second);
    }

    object f = m_python_module_universe_generator.attr("create_universe");
    if (!f) {
        ErrorLogger() << "Unable to call Python function create_universe ";
        return false;
    }

    return f(py_player_setup_data);
}

bool PythonServer::ExecuteTurnEvents() {
    object f = m_python_module_turn_events.attr("execute_turn_events");
    if (!f) {
        ErrorLogger() << "Unable to call Python function execute_turn_events ";
        return false;
    }
    return f();
}

const std::string GetPythonUniverseGeneratorDir()
{ return GetPythonDir() + "/universe_generation"; }

const std::string GetPythonTurnEventsDir()
{ return GetPythonDir() + "/turn_events"; }

const std::string GetPythonAuthDir()
{ return GetPythonDir() + "/auth"; }
