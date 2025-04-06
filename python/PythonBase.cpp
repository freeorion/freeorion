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

namespace {
    template<typename T>
    py::object PathToPythonString(const std::basic_string<T>& filename) {
        PyObject *py_filename;
        if constexpr (std::is_same_v<T, wchar_t>) {
            py_filename = PyUnicode_FromWideChar(filename.c_str(), filename.size());
        } else {
            py_filename = PyUnicode_FromStringAndSize(filename.c_str(), filename.size());
        }
        if (!py_filename) {
            ErrorLogger() << "Failed to convert filename to Python str";
            py::throw_error_already_set();
        }
        return py::object{py::handle<>(py_filename)};
    }

    template<typename T>
    py::object PathToPythonBytes(const std::basic_string<T>& filename) {
        PyObject *py_filename = PyBytes_FromStringAndSize(reinterpret_cast<const char*>(filename.c_str()), filename.size() * sizeof(T));
        if (!py_filename) {
            ErrorLogger() << "Failed to convert filename to Python bytes";
            py::throw_error_already_set();
        }
        return py::object{py::handle<>(py_filename)};
    }
}

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

    if (!InitErrorHandler()) {
        ErrorLogger() << "Python error handler isn't initialized!";
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
    py::object py_os = py::import("os");
    py_os.attr("chdir")(PathToPythonBytes(dir.native()));
    InfoLogger() << "Python current directory set to " << py::extract<std::string>(py::str(py_os.attr("getcwd")()))();
}

void PythonBase::AddToSysPath(const fs::path& dir) {
    if (!fs::exists(dir)) {
        ErrorLogger() << "Tried adding non-existing dir to sys.path: " << PathToString(dir);
        return;
    }
    py::object py_sys = py::import("sys");
    py_sys.attr("path").attr("append")(PathToPythonString(dir.native()));
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
