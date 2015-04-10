#include "PythonServerFramework.h"

#include "PythonServerWrapper.h"

#include "../../util/Directories.h"
#include "../../util/Logger.h"

#include "../PythonWrappers.h"

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/python.hpp>
#include <boost/python/list.hpp>
#include <boost/python/tuple.hpp>
#include <boost/python/extract.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>

#ifdef FREEORION_MACOSX
#include <sys/param.h>
#endif

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

namespace fs = boost::filesystem;

// Python module for logging functions
BOOST_PYTHON_MODULE(fo_logger) {
    FreeOrionPython::WrapLogger();
}

BOOST_PYTHON_MODULE(freeorion) {
    FreeOrionPython::WrapGameStateEnums();
    FreeOrionPython::WrapGalaxySetupData();
    WrapServerAPI();
}

namespace {
    // Some helper objects needed to initialize and run the
    // Python interface
#ifdef FREEORION_MACOSX
    static char     s_python_home[MAXPATHLEN];
    static char     s_python_program_name[MAXPATHLEN];
#endif
    static dict     s_python_namespace = dict();

    // Global reference to imported Python universe generator module
    static object s_python_module_universe_generator = object();

    // Global reference to imported Python turn events module
    static object s_python_module_turn_events = object();
}

const std::string GetPythonDir()
{ return GetResourceDir().string() + "/python"; }

const std::string GetPythonCommonDir()
{ return GetPythonDir(); }

const std::string GetPythonUniverseGeneratorDir()
{ return GetPythonDir() + "/universe_generation"; }

const std::string GetPythonTurnEventsDir()
{ return GetPythonDir() + "/turn_events"; }

bool PythonInit() {
    DebugLogger() << "Initializing server Python interface";

    try {
#ifdef FREEORION_MACOSX
        // There have been recurring issues on OSX to get FO to use the
        // Python framework shipped with the app (instead of falling back
        // on the ones provided by the system). These API calls have been
        // added in an attempt to solve the problems. Not sure if they
        // are really required, but better save than sorry.. ;)
        strcpy(s_python_home, GetPythonHome().string().c_str());
        Py_SetPythonHome(s_python_home);
        DebugLogger() << "Python home set to " << Py_GetPythonHome();
        strcpy(s_python_program_name, (GetPythonHome() / "Python").string().c_str());
        Py_SetProgramName(s_python_program_name);
        DebugLogger() << "Python program name set to " << Py_GetProgramFullPath();
#endif
        // initializes Python interpreter, allowing Python functions to be called from C++
        Py_Initialize();
        DebugLogger() << "Python initialized";
        DebugLogger() << "Python version: " << Py_GetVersion();
        DebugLogger() << "Python prefix: " << Py_GetPrefix();
        DebugLogger() << "Python module search path: " << Py_GetPath();
        DebugLogger() << "Initializing C++ interfaces for Python";
        initfo_logger();              // allows the "fo_logger" C++ module to be imported within Python code
    }
    catch (...) {
        ErrorLogger() << "Unable to initialize Python interpreter";
        return false;
    }

    // Allow the "freeorion" C++ module to be imported within Python code
    try {
        initfreeorion();
    }
    catch (...) {
        ErrorLogger() << "Unable to initialize freeorion interface";
        return false;
    }

    try {
        // get main namespace, needed to run other interpreted code
        object py_main = import("__main__");
        s_python_namespace = extract<dict>(py_main.attr("__dict__"));
    }
    catch (error_already_set err) {
        ErrorLogger() << "Unable to set up main namespace in Python";
        PyErr_Print();
        return false;
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
        ErrorLogger() << "Unable to redirect Python stdout and stderr";
        return false;
    }

    // Add the directory containing the Python scripts used by both
    // universe generation and turn events to Python sys.path
    if (!PythonAddToSysPath(GetPythonCommonDir()))
        return false;

    // Confirm existence of the directory containing the universe generation
    // Python scripts and add it to Pythons sys.path to make sure Python will
    // find our scripts
    if (!fs::exists(GetPythonUniverseGeneratorDir()) || !PythonAddToSysPath(GetPythonUniverseGeneratorDir()))
        return false;

    try {
        // import universe generator script file
        s_python_module_universe_generator = import("universe_generator");
    }
    catch (error_already_set err) {
        ErrorLogger() << "Unable to import universe generator script";
        PyErr_Print();
        return false;
    }

    // Confirm existence of the directory containing the turn event Python
    // scripts and add it to Pythons sys.path to make sure Python will find
    // our scripts
    if (!fs::exists(GetPythonTurnEventsDir()) || !PythonAddToSysPath(GetPythonTurnEventsDir()))
        return false;

    try {
        // import turn events script file
        s_python_module_turn_events = import("turn_events");
    }
    catch (error_already_set err) {
        ErrorLogger() << "Unable to import turn events script";
        PyErr_Print();
        return false;
    }

    DebugLogger() << "Server Python interface successfully initialized!";
    return true;
}

bool PythonExecScript(const std::string script) {
    try { object ignored = exec(script.c_str(), s_python_namespace, s_python_namespace); }
    catch (error_already_set err) {
        PyErr_Print();
        return false;
    }
    return true;
}

bool PythonSetCurrentDir(const std::string dir) {
    std::string script = "import os\n"
    "os.chdir(r'" + dir + "')\n"
    "print 'Python current directory set to', os.getcwd()";
    if (!PythonExecScript(script)) {
        ErrorLogger() << "Unable to set Python current directory";
        return false;
    }
    return true;
}

bool PythonAddToSysPath(const std::string dir) {
    std::string command = "sys.path.append(r'" + dir + "')";
    if (!PythonExecScript(command)) {
        ErrorLogger() << "Unable to set universe generator script dir";
        return false;
    }
    return true;
}

void PythonCleanup() {
    Py_Finalize();

    // TODO: are the three following lines really necessary?
    s_python_namespace = dict();
    s_python_module_turn_events = object();
    s_python_module_universe_generator = object();

    DebugLogger() << "Cleaned up server Python interface";
}

bool PythonExecuteTurnEvents() {
    bool success;
    object f = s_python_module_turn_events.attr("execute_turn_events");
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

std::vector<std::string> PythonErrorReport() {
    std::vector<std::string> err_list;
    object f = s_python_module_universe_generator.attr("error_report");
    if (!f) {
        ErrorLogger() << "Unable to call Python function error_report ";
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

bool PythonCreateUniverse(std::map<int, PlayerSetupData>& player_setup_data) {
    dict py_player_setup_data;
    bool success;
    
    for (std::map<int, PlayerSetupData>::iterator it = player_setup_data.begin(); it != player_setup_data.end(); ++it) {
        py_player_setup_data[it->first] = object(it->second);
    }

    object f = s_python_module_universe_generator.attr("create_universe");
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
