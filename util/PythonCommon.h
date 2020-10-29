#ifndef __FreeOrion__Util__PythonCommon__
#define __FreeOrion__Util__PythonCommon__

#include "Export.h"

class FO_COMMON_API PythonCommon {
public:
    PythonCommon();
    virtual ~PythonCommon();

    /**
       IsPythonRunning returns true is the python interpreter is
       initialized.  It is typically called after
       HandleErrorAlreadySet() to determine if the error caused the
       interpreter to shutdown.
     */
    bool         IsPythonRunning() const;

    bool         Initialize();        // initializes and runs the Python interpreter, prepares the Python environment

    virtual bool InitCommonImports(); // initializes Python imports

    void         Finalize();          // stops Python interpreter and releases its resources
private:
    // some helper objects needed to initialize and run the Python interface
#if defined(FREEORION_MACOSX) || defined(FREEORION_WIN32)
    wchar_t*                m_home_dir = nullptr;
    wchar_t*                m_program_name = nullptr;
#endif
};

#endif /* defined(__FreeOrion__Util__PythonCommon__) */

