#include "AIClientApp.h"


#ifdef __cplusplus
extern "C" // use C-linkage, as required by SDL
#endif
int main(int argc, char* argv[])
{
   AIClientApp g_app(argc, argv);
   
   g_app(); // run app (intialization and main process loop)
   return 0;
}

