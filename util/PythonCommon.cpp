#include "PythonCommon.h"

#include <boost/python.hpp>

#include "../util/Directories.h"
#include "../util/Logger.h"

PythonCommon::PythonCommon()
{ }

PythonCommon::~PythonCommon()
{ Finalize(); }

bool PythonCommon::IsPythonRunning() const
{ return Py_IsInitialized(); }

bool PythonCommon::Initialize() {
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
        if (!InitCommonImports()) {
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
    return true;
}

bool PythonCommon::InitCommonImports()
{ return true; }

void PythonCommon::Finalize() {
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
}

