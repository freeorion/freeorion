#ifndef __FreeOrion__PythonFramework__
#define __FreeOrion__PythonFramework__

#include <string>


// Initializes und runs the Python interpreter
// Prepares the Python environment
bool PythonInit();

// Executes a Python script
bool PythonExecScript(const std::string script);

// Sets Python current work directory
bool PythonSetCurrentDir(const std::string dir);

// Adds directory to Python sys.patch
bool PythonAddToSysPath(const std::string dir);

// Stops Python interpreter and releases its resources
void PythonCleanup();


#endif /* defined(__FreeOrion__PythonFramework__) */
