#include "CommonFramework.h"

#include "../util/Directories.h"
#include "../util/Logger.h"
#include "CommonWrappers.h"

#include <boost/filesystem.hpp>
#include <boost/python/tuple.hpp>
#include <boost/python/list.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/docstring_options.hpp>

using boost::python::object;
using boost::python::import;
using boost::python::error_already_set;
using boost::python::exec;
using boost::python::dict;
using boost::python::list;
using boost::python::extract;

namespace fs = boost::filesystem;

// Python module for logging functions
BOOST_PYTHON_MODULE(freeorion_logger) {
    boost::python::docstring_options doc_options(true, true, false);
    FreeOrionPython::WrapLogger();
}

PythonBase::PythonBase() :
#if defined(FREEORION_MACOSX)
    m_home_dir(""),
    m_program_name(""),
#endif
    m_python_module_error(nullptr)
{
#if defined(FREEORION_WIN32)
    m_home_dir[0] = '\0';
    m_program_name[0] = '\0';
#endif
}

PythonBase::~PythonBase() {
    Finalize();
}

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

    m_system_exit = import("exceptions").attr("SystemExit");
    try {
        // get main namespace, needed to run other interpreted code
        object py_main = import("__main__");
        m_namespace = extract<dict>(py_main.attr("__dict__"));

        // add the directory containing common Python modules used by all Python scripts to Python sys.path
        AddToSysPath(GetPythonCommonDir());

        // allow the "freeorion_logger" C++ module to be imported within Python code
        try {
            initfreeorion_logger();
        } catch (...) {
            ErrorLogger() << "Unable to initialize FreeOrion Python logging module";
            return false;
        }

        // Allow C++ modules implemented by derived classes to be imported
        // within Python code

        if (!InitModules()) {
            ErrorLogger() << "Unable to initialize FreeOrion Python modules";
            return false;
        }
    } catch (error_already_set& err) {
        HandleErrorAlreadySet();
        return false;
    }

    DebugLogger() << "FreeOrion Python interface successfully initialized!";
    return true;
}

bool PythonBase::IsPythonRunning()
{ return Py_IsInitialized(); }

void PythonBase::HandleErrorAlreadySet() {
    if (!Py_IsInitialized()) {
        ErrorLogger() << "Python interpreter not initialized and exception handler called.";
        return;
    }

    // Matches system exit
    if (PyErr_ExceptionMatches(m_system_exit.ptr()))
    {
        Finalize();
        ErrorLogger() << "Python interpreter exited with SystemExit(), sys.exit(), exit, quit or some other alias.";
        return;
    }

    PyErr_Print();
    return;
}

void PythonBase::Finalize() {
    if (Py_IsInitialized()) {
        Py_Finalize();
        DebugLogger() << "Cleaned up FreeOrion Python interface";
    }
}

void PythonBase::SetCurrentDir(const std::string dir) {
    if (!fs::exists(dir)) {
        ErrorLogger() << "Tried setting current dir to non-existing dir: " << dir;
        return;
    }
    std::string script = "import os\n"
    "os.chdir(r'" + dir + "')\n"
    "print 'Python current directory set to', os.getcwd()";
    exec(script.c_str(), m_namespace, m_namespace);
}

void PythonBase::AddToSysPath(const std::string dir) {
    if (!fs::exists(dir)) {
        ErrorLogger() << "Tried adding non-existing dir to sys.path: " << dir;
        return;
    }
    std::string script = "import sys\n"
        "sys.path.append(r'" + dir + "')";
    exec(script.c_str(), m_namespace, m_namespace);
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
            HandleErrorAlreadySet();
            return err_list;
        }

        for (int i = 0; i < len(py_err_list); i++) {
            err_list.push_back(extract<std::string>(py_err_list[i]));
        }
    }

    return err_list;
}

const std::string GetPythonDir()
{ return GetResourceDir().string() + "/python"; }

const std::string GetPythonCommonDir()
{ return GetPythonDir(); }
