#include "ServerApp.h"

#ifdef WITH_PYTHON
#  include <boost/python.hpp>
#  include <Python.h>
#  include <cstdio>

  extern "C" void initUniverse(void);
#endif

#ifdef __cplusplus
extern "C" // use C-linkage, as required by SDL
#endif
int main(int argc, char* argv[])
{
    
   ServerApp g_app(argc, argv);
#ifdef WITH_PYTHON

   Py_Initialize();
   initUniverse();
//   PyRun_SimpleString("import Universe\nprint dir(Universe)\nprint
//   \"Hallo\"");
   PyRun_InteractiveLoop(stdin, "Console");
   
   
   Py_Finalize();
   
#endif
   
   g_app(); // run app (intialization and main process loop)
   return 0;
}

