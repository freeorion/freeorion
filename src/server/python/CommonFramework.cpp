#include "CommonFramework.h"

#include "../../common/util/Directories.h"
#include "../../common/util/Logger.h"
#include "CommonWrappers.h"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/python/tuple.hpp>
#include <boost/python/list.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/docstring_options.hpp>

namespace fs = boost::filesystem;
namespace py = boost::python;

// Python module for logging functions
BOOST_PYTHON_MODULE(freeorion_logger) {
    boost::python::docstring_options doc_options(true, true, false);
    FreeOrionPython::WrapLogger();
}

PythonBase::PythonBase()
{}

PythonBase::~PythonBase()
{ Finalize(); }

bool PythonBase::Initialize() {
    DebugLogger() << "Initializing FreeOrion Python interface";

    try {
#if defined(FREEORION_MACOSX) || defined(FREEORION_WIN32)
        // There have been recurring issues on Windows and OSX to get FO to use the
        // Python framework shipped with the app (instead of falling back on the ones
        // provided by the system). These API calls have been added in an attempt to
        // solve the problems. Not sure if they are really required, but better save
        // than sorry... ;)
        m_home_dir = Py_DecodeLocale(GetPythonHome().string().c_str(), nullptr);
        Py_SetPythonHome(m_home_dir);
        DebugLogger() << "Python home set to " << Py_GetPythonHome();
        m_program_name = Py_DecodeLocale((GetPythonHome() / "Python").string().c_str(), nullptr);
        Py_SetProgramName(m_program_name);
        DebugLogger() << "Python program name set to " << Py_GetProgramFullPath();
#endif
        // allow the "freeorion_logger" C++ module to be imported within Python code
        if (PyImport_AppendInittab("freeorion_logger", &PyInit_freeorion_logger) == -1) {
            ErrorLogger() << "Unable to initialize freeorion_logger import";
            return false;
        }
        if (!InitImports()) {
            ErrorLogger() << "Unable to initialize imports";
            return false;
        }
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

    try {
        m_system_exit = py::import("builtins").attr("SystemExit");
        // get main namespace, needed to run other interpreted code
        py::object py_main = py::import("__main__");
        py::dict py_namespace = py::extract<py::dict>(py_main.attr("__dict__"));
        m_namespace = py_namespace;

        // add the directory containing common Python modules used by all Python scripts to Python sys.path
        AddToSysPath(GetPythonCommonDir());
    } catch (const py::error_already_set& err) {
        HandleErrorAlreadySet();
        ErrorLogger() << "Unable to initialize FreeOrion Python namespace and set path";
        return false;
    } catch (...) {
        ErrorLogger() << "Unable to initialize FreeOrion Python namespace and set path";
        return false;
    }

    try {
        // Allow C++ modules implemented by derived classes to be imported
        // within Python code
        if (!InitModules()) {
            ErrorLogger() << "Unable to initialize FreeOrion Python modules";
            return false;
        }
    } catch (const py::error_already_set& err) {
        HandleErrorAlreadySet();
        ErrorLogger() << "Unable to initialize FreeOrion Python modules (exception caught)";
        return false;
    } catch (...) {
        ErrorLogger() << "Unable to initialize FreeOrion Python modules (exception caught)";
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

    PyObject *extype, *value, *traceback;
    PyErr_Fetch(&extype, &value, &traceback);
    PyErr_NormalizeException(&extype, &value, &traceback);
    if (extype == nullptr) {
        ErrorLogger() << "Missing python exception type";
        return;
    }

    py::object o_extype(py::handle<>(py::borrowed(extype)));
    py::object o_value(py::handle<>(py::borrowed(value)));
    py::object o_traceback = traceback != nullptr ? py::object(py::handle<>(py::borrowed(traceback))) : py::object();

    py::object mod_traceback = py::import("traceback");
    py::object lines = mod_traceback.attr("format_exception")(o_extype, o_value, o_traceback);
    for (int i = 0; i < len(lines); ++i) {
        std::string line = py::extract<std::string>(lines[i])();
        boost::algorithm::trim_right(line);
        ErrorLogger() << line;
    }

    return;
}

void PythonBase::Finalize() {
    if (Py_IsInitialized()) {
        // cleanup python objects before interpterer shutdown
        m_system_exit = py::object();
        m_namespace = boost::none;
        if (m_python_module_error != nullptr) {
            (*m_python_module_error) = py::object();
            m_python_module_error = nullptr;
        }
        try {
            Py_Finalize();
#if defined(FREEORION_MACOSX) || defined(FREEORION_WIN32)
            if (m_home_dir != nullptr) {
                PyMem_RawFree(m_home_dir);
                m_home_dir = nullptr;
            }
            if (m_program_name != nullptr) {
                PyMem_RawFree(m_program_name);
                m_program_name = nullptr;
            }
#endif
        } catch (const std::exception& e) {
            ErrorLogger() << "Caught exception when cleaning up FreeOrion Python interface: " << e.what();
            return;
        }
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
    "print ('Python current directory set to', os.getcwd())";
    exec(script.c_str(), *m_namespace, *m_namespace);
}

void PythonBase::AddToSysPath(const std::string dir) {
    if (!fs::exists(dir)) {
        ErrorLogger() << "Tried adding non-existing dir to sys.path: " << dir;
        return;
    }
    std::string script = "import sys\n"
        "sys.path.append(r'" + dir + "')";
    exec(script.c_str(), *m_namespace, *m_namespace);
}

void PythonBase::SetErrorModule(py::object& module)
{ m_python_module_error = &module; }

std::vector<std::string> PythonBase::ErrorReport() {
    std::vector<std::string> err_list;

    if (m_python_module_error) {
        py::object f = m_python_module_error->attr("error_report");
        if (!f) {
            ErrorLogger() << "Unable to call Python function error_report ";
            return err_list;
        }

        py::list py_err_list;
        try { py_err_list = py::extract<py::list>(f()); }
        catch (const py::error_already_set& err) {
            HandleErrorAlreadySet();
            return err_list;
        }

        for (int i = 0; i < len(py_err_list); i++) {
            err_list.push_back(py::extract<std::string>(py_err_list[i]));
        }
    }

    return err_list;
}

const std::string GetPythonDir()
{ return GetResourceDir().string() + "/python"; }

const std::string GetPythonCommonDir()
{ return GetPythonDir(); }
