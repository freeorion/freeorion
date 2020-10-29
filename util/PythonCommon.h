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
    bool IsPythonRunning();

    bool Initialize();
};

#endif /* defined(__FreeOrion__Util__PythonCommon__) */

