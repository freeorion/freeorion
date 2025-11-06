#include "PythonCommon.h"

#include <boost/algorithm/string/trim.hpp>

#include "../util/Directories.h"
#include "../util/Logger.h"

#ifdef FREEORION_WIN32
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#include <windows.h>
#endif

namespace py = boost::python;

namespace {
#if defined(FREEORION_MACOSX) || defined(FREEORION_WIN32) || defined(FREEORION_ANDROID)
wchar_t* GetFilePath(const std::filesystem::path& path)
{
#if defined(FREEORION_WIN32)
    const std::wstring_view native_path = path.native();
    wchar_t* py_path = (wchar_t*)PyMem_RawCalloc(native_path.size() + 1, sizeof(wchar_t));
    native_path.copy(py_path, native_path.size());
    py_path[native_path.size()] = L'\0';
    return py_path;
#else
    return Py_DecodeLocale(path.string().c_str(), nullptr);
#endif
}
#endif

auto GetLoggableString(const wchar_t* const original)
{
#if defined(FREEORION_WIN32)
    const std::wstring_view view(original);
    const size_t original_size = view.size(); // passing -1 would compute size for null terminated string, but would include the null
    int utf8_size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, original, original_size, nullptr, 0, nullptr, nullptr);
    std::string utf8_string(utf8_size, '\0');
    if (utf8_size > 0)
        WideCharToMultiByte(CP_UTF8, 0, original, original_size, utf8_string.data(), utf8_size, nullptr, nullptr);
    return utf8_string;
#else
    return original;
#endif
}

auto GetPythonExecutable() // should be the containing C++ binary / .exe file
{ return py::extract<std::string>(py::import("sys").attr("executable"))(); }
}

PythonCommon::~PythonCommon()
{ Finalize(); }

bool PythonCommon::IsPythonRunning() const
{ return Py_IsInitialized(); }

bool PythonCommon::Initialize() {
    DebugLogger() << "Initializing FreeOrion Python interface";

    try {
#if defined(FREEORION_MACOSX) || defined(FREEORION_WIN32) || defined(FREEORION_ANDROID)
        // There have been recurring issues on Windows and OSX to get FO to use the
        // Python framework shipped with the app (instead of falling back on the ones
        // provided by the system). These API calls have been added in an attempt to
        // solve the problems. Not sure if they are really required, but better safe
        // than sorry... ;)

        m_home_dir = GetFilePath(GetPythonHome());
        Py_SetPythonHome(m_home_dir);
        DebugLogger() << "Python home set to " << GetLoggableString(Py_GetPythonHome());

        m_program_name = GetFilePath(GetPythonHome() / "Python");
        Py_SetProgramName(m_program_name);
        DebugLogger() << "Python program name set to " << GetLoggableString(Py_GetProgramName());
#endif

#if defined(FREEORION_ANDROID)
        Py_NoSiteFlag = 1;
#endif
        if (!InitCommonImports()) {
            ErrorLogger() << "Unable to initialize imports";
            return false;
        }
        // initializes Python interpreter, allowing Python functions to be called from C++
        Py_Initialize();
        DebugLogger() << "Python initialized";
        DebugLogger() << "Python program: " << GetPythonExecutable();
        DebugLogger() << "Python version: " << Py_GetVersion();
        DebugLogger() << "Python prefix: " << GetLoggableString(Py_GetPrefix());
        DebugLogger() << "Python module search path: " << GetLoggableString(Py_GetPath());
    }
    catch (...) {
        ErrorLogger() << "Unable to initialize Python interpreter";
        return false;
    }

    return true;
}

bool PythonCommon::InitErrorHandler() {
    try {
        m_system_exit = py::import("builtins").attr("SystemExit");
        m_traceback_format_exception = py::import("traceback").attr("format_exception");
    } catch (const py::error_already_set&) {
        HandleErrorAlreadySet();
        ErrorLogger() << "Unable to initialize FreeOrion Python SystemExit";
        return false;
    } catch (...) {
        ErrorLogger() << "Unable to initialize FreeOrion Python SystemExit";
        return false;
    }

    return true;
}

bool PythonCommon::InitCommonImports()
{ return true; }

void PythonCommon::HandleErrorAlreadySet() {
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
    PyErr_Fetch(std::addressof(extype), std::addressof(value), std::addressof(traceback));
    PyErr_NormalizeException(std::addressof(extype), std::addressof(value), std::addressof(traceback));
    if (!extype) {
        ErrorLogger() << "Missing python exception type";
        return;
    }

    py::object o_extype(py::handle<>(py::borrowed(extype)));
    py::object o_value(py::handle<>(py::borrowed(value)));
    py::object o_traceback = (traceback != nullptr) ?
        py::object(py::handle<>(py::borrowed(traceback))) : py::object();

    py::object lines = m_traceback_format_exception(o_extype, o_value, o_traceback);
    for (int i = 0; i < len(lines); ++i) {
        std::string line = py::extract<std::string>(lines[i])();
        boost::algorithm::trim_right(line);
        ErrorLogger() << line;
    }
    return;
}

void PythonCommon::Finalize() {
    if (Py_IsInitialized()) {
        // cleanup python objects before interpterer shutdown
        m_system_exit = py::object();
        m_traceback_format_exception = py::object();
        try {
            // According to boost.python 1.69 docs python Py_Finalize must not be called
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
        } catch (...) {
            ErrorLogger() << "Caught unknown exception when cleaning up FreeOrion Python interface";
        }
    }
}

