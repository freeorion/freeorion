#ifndef __FreeOrion__Util__PythonCommon__
#define __FreeOrion__Util__PythonCommon__

#include "../util/boost_fix.h"
#include <boost/python.hpp>

#include "Export.h"

class FO_COMMON_API PythonCommon {
public:
    PythonCommon() = default;
    virtual ~PythonCommon();

    /** IsPythonRunning returns true is the python interpreter is
        initialized.  It is typically called after
        HandleErrorAlreadySet() to determine if the error caused the
        interpreter to shutdown. */
    bool IsPythonRunning() const;

    /** Handles boost::python::error_already_set.
        If the error is SystemExit the python interpreter is finalized
        and no longer available.

        Call PyErr_Print() if the exception is an error.

        HandleErrorAlreadySet is idempotent, calling it multiple times
        won't crash or hang the process. */
    void HandleErrorAlreadySet();

    bool Initialize();        // initializes and runs the Python interpreter, prepares the Python environment

    virtual bool InitCommonImports(); // initializes Python imports

    void Finalize();          // stops Python interpreter and releases its resources
private:
    // some helper objects needed to initialize and run the Python interface
#if defined(FREEORION_MACOSX) || defined(FREEORION_WIN32) || defined(FREEORION_ANDROID)
    wchar_t* m_home_dir = nullptr;
    wchar_t* m_program_name = nullptr;
#endif
    // A copy of the systemExit exception to compare with returned
    // exceptions.  It can't be created in the exception handler.
    boost::python::object m_system_exit;
    boost::python::object m_traceback_format_exception;
};

#endif /* defined(__FreeOrion__Util__PythonCommon__) */

