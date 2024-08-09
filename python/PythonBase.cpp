#include "PythonBase.h"

#include "../util/Directories.h"
#include "../util/Logger.h"
#include "CommonWrappers.h"

#include <boost/filesystem.hpp>
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

PythonBase::~PythonBase()
{ Finalize(); }

bool PythonBase::Initialize() {
    if (!PythonCommon::Initialize()) {

#if defined(MS_WINDOWS)
        // forces stream encoding to UTF8, which will hopefully fix issues on windows with non-english locale settings
        const std::string ENCODING{"UTF-8"};    // specifying "C.UTF-8" causes Py_Initialize() call to fail for me -Geoff
        auto encoding_result = Py_SetStandardStreamEncoding(ENCODING.c_str(), ENCODING.c_str());
        DebugLogger() << "Python standard stream encoding set to: " << ENCODING << " with result: " << encoding_result;
#endif

        return false;
    }

    DebugLogger() << "Initializing C++ interfaces for Python";

    try {
        // get main namespace, needed to run other interpreted code
        py::object py_main = py::import("__main__");
        py::dict py_namespace = py::extract<py::dict>(py_main.attr("__dict__"));
        m_namespace = py_namespace;

        // add the directory containing common Python modules used by all Python scripts to Python sys.path
        AddToSysPath(GetPythonCommonDir());
    } catch (const py::error_already_set&) {
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
    } catch (const py::error_already_set&) {
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

bool PythonBase::InitCommonImports() {
    // allow the "freeorion_logger" C++ module to be imported within Python code
    if (PyImport_AppendInittab("freeorion_logger", &PyInit_freeorion_logger) == -1) {
        ErrorLogger() << "Unable to initialize freeorion_logger import";
        return false;
    }
    if (!InitImports()) {
        ErrorLogger() << "Unable to initialize imports";
        return false;
    }
    return true;
}

void PythonBase::Finalize() {
    if (Py_IsInitialized()) {
        // cleanup python objects before interpterer shutdown
        m_namespace = boost::none;
        if (m_python_module_error != nullptr) {
            (*m_python_module_error) = py::object();
            m_python_module_error = nullptr;
        }
        PythonCommon::Finalize();
        DebugLogger() << "Cleaned up FreeOrion Python interface";
    }
}

void PythonBase::SetCurrentDir(const fs::path& dir) {
    if (!fs::exists(dir)) {
        ErrorLogger() << "Tried setting current dir to non-existing dir: " << PathToString(dir);
        return;
    }
    py::str script = "import os\n"
        "os.chdir(r'";
    script += dir.native();
    script += "')\n"
        "print ('Python current directory set to', os.getcwd())";
    exec(script, *m_namespace, *m_namespace);
}

void PythonBase::AddToSysPath(const fs::path& dir) {
    if (!fs::exists(dir)) {
        ErrorLogger() << "Tried adding non-existing dir to sys.path: " << PathToString(dir);
        return;
    }
    py::str script = "import sys\n"
        "sys.path.append(r'";
    script += dir.native();
    script += "')";
    exec(script, *m_namespace, *m_namespace);
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
        catch (const py::error_already_set&) {
            HandleErrorAlreadySet();
            return err_list;
        }

        for (int i = 0; i < len(py_err_list); i++) {
            err_list.push_back(py::extract<std::string>(py_err_list[i]));
        }
    }

    return err_list;
}

fs::path GetPythonDir()
{ return GetResourceDir() / "python"; }

fs::path GetPythonCommonDir()
{ return GetPythonDir(); }
