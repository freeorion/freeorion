// -*- C++ -*-
#ifndef _AppInterface_h_
#define _AppInterface_h_

#ifndef _EmpireManager_h_
#include "../Empire/EmpireManager.h"
#endif

#ifndef _Universe_h_
#include "../universe/Universe.h"
#endif

#include <log4cpp/Category.hh>

/** Accessor for the App's empire manager */
EmpireManager& Empires();

/** Accessor for the App's universe object */
Universe& GetUniverse();
    
/** Accessor for the App's logger */
log4cpp::Category& Logger();

/** Returns a new object ID from the server */
int GetNewObjectID();

/* add additional accessors here for app specific things
   that are needed for both the server and the client, but for which
   access will vary and requires an #ifdef */

inline std::string AppInterfaceRevision()
{return "$Id$";}

#endif // _AppInterface_h_
