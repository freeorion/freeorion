#ifndef __FreeOrion__PythonServerFramework__
#define __FreeOrion__PythonServerFramework__

#include "../../universe/Universe.h"
#include "../../universe/UniverseGenerator.h"
#include "../../util/MultiplayerCommon.h"

#include <vector>
#include <map>
#include <string>
#include <utility>


// Returns parent folder for all server-side Python scripts
const std::string GetPythonDir();

// Returns folder containing Python modules used by both universe
// generation and turn events
const std::string GetPythonCommonDir();

// Returns folder containing the Python universe generator scripts
const std::string GetPythonUniverseGeneratorDir();

// Returns folder containing the Python turn events scripts
const std::string GetPythonTurnEventsDir();

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

// Wraps call to the Python universe generator error report function
std::vector<std::string> PythonErrorReport();

// Wraps call to the main Python universe generator function
bool PythonCreateUniverse(std::map<int, PlayerSetupData>& player_setup_data);

// Wraps call to the main Python turn events function
bool PythonExecuteTurnEvents();


#endif /* defined(__FreeOrion__PythonServerFramework__) */
