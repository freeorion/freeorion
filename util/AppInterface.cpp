#include "AppInterface.h"

#ifdef FREEORION_BUILD_SERVER
# include "../server/ServerApp.h"
#else // a client build
# ifdef FREEORION_BUILD_HUMAN
#  include "../client/human/HumanClientApp.h"
# else
#  include "../client/AI/AIClientApp.h"
# endif
#endif

#include "../util/MultiplayerCommon.h"


namespace {
    bool temp_header_bool = RecordHeaderFile(AppInterfaceRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}


EmpireManager& Empires()
{
#ifdef FREEORION_BUILD_SERVER
    return ServerApp::GetApp()->Empires();
#elif defined(FREEORION_BUILD_UTIL)
    static EmpireManager em;
    return em;
#else
    return ClientApp::Empires();
#endif
}

Universe& GetUniverse()
{
#ifdef FREEORION_BUILD_SERVER
    return ServerApp::GetApp()->GetUniverse();
#elif defined(FREEORION_BUILD_UTIL)
    static Universe u;
    return u;
#else
    return ClientApp::GetUniverse();
#endif
}

log4cpp::Category& Logger()
{
    return log4cpp::Category::getRoot();
}

