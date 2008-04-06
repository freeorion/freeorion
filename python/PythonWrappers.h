#ifndef PYTHON_WRAPPERS
#define PYTHON_WRAPPERS

#include "PythonSetWrapper.h"

namespace FreeOrionPython {
    void WrapUniverseClasses();
    void WrapGameStateEnums();
    void WrapEmpire();

    void WrapLogger();
}

#endif
