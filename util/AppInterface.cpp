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


namespace {
    bool temp_header_bool = RecordHeaderFile(AppInterfaceRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}


EmpireManager& Empires()
{
#ifdef FREEORION_BUILD_SERVER
    return ServerApp::GetApp()->Empires();
#else
    return ClientApp::Empires();
#endif
}

Universe& GetUniverse()
{
#ifdef FREEORION_BUILD_SERVER
    return ServerApp::GetApp()->GetUniverse();
#else
    return ClientApp::GetUniverse();
#endif
}

log4cpp::Category& Logger()
{
#ifdef FREEORION_BUILD_SERVER
    return ServerApp::GetApp()->Logger();
#else
# ifdef FREEORION_BUILD_HUMAN
    return HumanClientApp::GetApp()->Logger();
# else
    return AIClientApp::GetApp()->Logger();
# endif
#endif
}

