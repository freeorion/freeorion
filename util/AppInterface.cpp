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
    bool temp_source_bool = RecordSourceFile("$Id$");
}

const int INVALID_GAME_TURN = -1;
const int BEFORE_FIRST_TURN = -2;
const int IMPOSSIBLY_LARGE_TURN = 2 << 15;

EmpireManager& Empires()
{
#ifdef FREEORION_BUILD_SERVER
    return ServerApp::GetApp()->Empires();
#elif defined(FREEORION_BUILD_UTIL)
    static EmpireManager em;
    return em;
#else
    return ClientApp::GetApp()->Empires();
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
    return ClientApp::GetApp()->GetUniverse();
#endif
}

log4cpp::Category& Logger()
{
    return log4cpp::Category::getRoot();
}

int GetNewObjectID()
{
#ifdef FREEORION_BUILD_SERVER
    return GetUniverse().GenerateObjectID();
#elif defined(FREEORION_BUILD_UTIL)
    return UniverseObject::INVALID_OBJECT_ID;
#else
    return ClientApp::GetApp()->GetNewObjectID();
#endif
}

int CurrentTurn()
{
#ifdef FREEORION_BUILD_SERVER
    return ServerApp::GetApp()->CurrentTurn();
#elif defined(FREEORION_BUILD_UTIL)
    return INVALID_GAME_TURN;
#else
    return ClientApp::GetApp()->CurrentTurn();
#endif
}

DifficultyLevel CurrentDifficultyLevel()
{
#ifdef FREEORION_BUILD_SERVER
    return ServerApp::GetApp()->CurrentDifficultyLevel();
#elif defined(FREEORION_BUILD_UTIL)
    return INVALID_DIFFICULTY_LEVEL;
#else
    return ClientApp::GetApp()->CurrentDifficultyLevel();
#endif
}
