#include "CommonFramework.h"

#include "../util/Directories.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "CommonWrappers.h"

#include <boost/python/docstring_options.hpp>

using boost::python::object;
using boost::python::import;
using boost::python::error_already_set;
using boost::python::exec;
using boost::python::dict;
using boost::python::list;
using boost::python::extract;


// Python base package for freeorion
BOOST_PYTHON_MODULE(freeorion) {
    list module_path;
    module_path.append( (GetResourceDir() / GetOptionsDB().Get<std::string>("script-path")).string() );
    boost::python::scope().attr("__path__") = module_path;
}

// Python module for logging functions
BOOST_PYTHON_MODULE(freeorion_logger) {
    boost::python::docstring_options doc_options(true, true, false);
    FreeOrionPython::WrapLogger();
}

PythonBase::PythonBase() :
    m_python_interpreter_initialized(false),
#if defined(FREEORION_MACOSX)
    m_home_dir(""),
    m_program_name(""),
#endif
    m_python_module_error(NULL)
{}

bool PythonBase::Initialize()
{
    DebugLogger() << "Initializing FreeOrion Python interface";

    try {
#if defined(FREEORION_MACOSX) || defined(FREEORION_WIN32)
        // There have been recurring issues on Windows and OSX to get FO to use the
        // Python framework shipped with the app (instead of falling back on the ones
        // provided by the system). These API calls have been added in an attempt to
        // solve the problems. Not sure if they are really required, but better save
        // than sorry... ;)
        strcpy(m_home_dir, GetPythonHome().string().c_str());
        Py_SetPythonHome(m_home_dir);
        DebugLogger() << "Python home set to " << Py_GetPythonHome();
        strcpy(m_program_name, (GetPythonHome() / "Python").string().c_str());
        Py_SetProgramName(m_program_name);
        DebugLogger() << "Python program name set to " << Py_GetProgramFullPath();
#endif
        // initializes Python interpreter, allowing Python functions to be called from C++
        Py_Initialize();
        m_python_interpreter_initialized = true;
        DebugLogger() << "Python initialized";
        DebugLogger() << "Python version: " << Py_GetVersion();
        DebugLogger() << "Python prefix: " << Py_GetPrefix();
        DebugLogger() << "Python module search path: " << Py_GetPath();
    }
    catch (...) {
        ErrorLogger() << "Unable to initialize Python interpreter";
        return false;
    }

    DebugLogger() << "Initializing C++ interfaces for Python";

    try {
        // get main namespace, needed to run other interpreted code
        object py_main = import("__main__");
        m_namespace = extract<dict>(py_main.attr("__dict__"));
    }
    catch (error_already_set err) {
        ErrorLogger() << "Unable to set up main namespace in Python";
        PyErr_Print();
        return false;
    }

    // initialize "freeorion" package
    try {
        initfreeorion();
    } catch (...) {
        ErrorLogger() << "Unable to initialize FreeOrion base package";
        return false;
    }

    // allow the "freeorion_logger" C++ module to be imported within Python code
    try {
        initfreeorion_logger();
    } catch (...) {
        ErrorLogger() << "Unable to initialize FreeOrion Python logging module";
        return false;
    }

    // set up logging by redirecting stdout and stderr to exposed logging functions
    std::string script = "import sys\n"
    "import freeorion_logger\n"
    "class dbgLogger:\n"
    "  def write(self, msg):\n"
    "    freeorion_logger.log(msg)\n"
    "class errLogger:\n"
    "  def write(self, msg):\n"
    "    freeorion_logger.error(msg)\n"
    "sys.stdout = dbgLogger()\n"
    "sys.stderr = errLogger()\n"
    "print('Python stdout and stderr redirected')";
    if (!ExecScript(script)) {
        ErrorLogger() << "Unable to redirect Python stdout and stderr";
        return false;
    }

    // Allow C++ modules implemented by derived classes to be imported within Python code
    if (!InitModules()) {
        ErrorLogger() << "Unable to initialize FreeOrion Python modules";
        return false;
    }

    // add the directory containing common Python modules used by all Python scripts to Python sys.path
    if (!AddToSysPath(GetPythonCommonDir()))
        return false;

    DebugLogger() << "FreeOrion Python interface successfully initialized!";
    return true;
}

void PythonBase::Finalize() {
    if (m_python_interpreter_initialized) {
        Py_Finalize();
        m_python_interpreter_initialized = false;
    }
    DebugLogger() << "Cleaned up FreeOrion Python interface";
}

bool PythonBase::ExecScript(const std::string script) {
    try {
        object ignored = exec(script.c_str(), m_namespace, m_namespace);
    }
    catch (error_already_set err) {
        PyErr_Print();
        return false;
    }
    return true;
}

bool PythonBase::SetCurrentDir(const std::string dir) {
    std::string script = "import os\n"
    "os.chdir(r'" + dir + "')\n"
    "print 'Python current directory set to', os.getcwd()";
    if (!ExecScript(script)) {
        ErrorLogger() << "Unable to set Python current directory";
        return false;
    }
    return true;
}

bool PythonBase::AddToSysPath(const std::string dir) {
    std::string command = "sys.path.append(r'" + dir + "')";
    if (!ExecScript(command)) {
        ErrorLogger() << "Unable to add " << dir << " to sys.path";
        return false;
    }
    return true;
}

void PythonBase::SetErrorModule(object& module)
{ m_python_module_error = &module; }

std::vector<std::string> PythonBase::ErrorReport() {
    std::vector<std::string> err_list;

    if (m_python_module_error) {
        object f = m_python_module_error->attr("error_report");
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
    }

    return err_list;
}

const std::string GetPythonDir()
{ return GetResourceDir().string() + "/python"; }

const std::string GetPythonCommonDir()
{ return GetPythonDir(); }
