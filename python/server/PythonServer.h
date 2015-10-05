// -*- C++ -*-
#ifndef _PythonServer_h_
#define _PythonServer_h_

#include "PythonServerFramework.h"


// Initializes the server Python interface
bool PythonInit();

// Shuts down and cleans up the server Python interface
void PythonCleanup();

// Calls Python universe generator script.
// Supposed to be called to create a new universe so it can be used by content
// scripters to customize universe generation.
void GenerateUniverse(std::map<int, PlayerSetupData>& player_setup_data);

// Calls Python turn events script.
// Supposed to be called every turn so it can be used by content scripters to
// implement user customizable turn events.
void ExecuteScriptedTurnEvents();


#endif // _PythonServer_h_