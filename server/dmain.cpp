#include "ServerApp.h"

#include "../util/OptionsDB.h"

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
    try {
        GetOptionsDB().AddFlag('h', "help", "Print this help message.");
        GetOptionsDB().SetFromCommandLine(argc, argv);
        if (GetOptionsDB().Get<bool>("help")) {
            GetOptionsDB().GetUsage(std::cerr);
            return 0;
        }
    } catch (const std::invalid_argument& e) {
        std::cerr << "main() caught exception(std::invalid_arg): " << e.what();
        GetOptionsDB().GetUsage(std::cerr);
        return 1;
    } catch (const std::runtime_error& e) {
        std::cerr << "main() caught exception(std::runtime_error): " << e.what();
        GetOptionsDB().GetUsage(std::cerr);
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "main() caught exception(std::exception): " << e.what();
        GetOptionsDB().GetUsage(std::cerr);
        return 1;
    }

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

