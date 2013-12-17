// -*- C++ -*-
#ifndef _PythonUniverseGenerator_h_
#define _PythonUniverseGenerator_h_

#include "Universe.h"
#include "UniverseGenerator.h"
#include "../util/MultiplayerCommon.h"


// Initializes, runs and cleans up the Python universe generator
// interface to create a new game universe
// Serves as an interface to the server-side universe generator
// functions of the Universe class
void GenerateUniverse(GalaxySetupData&                      galaxy_setup_data,
                      const std::map<int, PlayerSetupData>& player_setup_data);


#endif // _PythonUniverseGenerator_h_