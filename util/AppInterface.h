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

/*
    Accessor for the App's empire manager
*/
EmpireManager& Empires();

/*
    Accessor for the App's universe object
*/
Universe& GetUniverse();
    
/*
    Accessor for the App's logger.  
    This should make everybody's lives easier
*/
log4cpp::Category& Logger();


    /* 
        TODO  -- add additional accessors here for app specific things
        that are needed for both the server and the client, but for which
        access will vary and requires an #ifdef
    */

inline std::pair<std::string, std::string> AppInterfaceRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _AppInterface_h_
